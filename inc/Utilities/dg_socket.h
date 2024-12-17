///////////////////////////////////////////////////////////////////////////////
/// \file dg_socket.h
/// \brief DG socket support: classes for client-server communications over TCP/IP
///
///
/// Copyright DeGirum Corporation 2021
/// All rights reserved

#pragma once

#include <exception>
#include <string>
#include "Utilities/dg_tensor_structs.h"

#include <asio.hpp>

namespace DG
{

namespace main_protocol
{

/// Socket object type
using socket_t = asio::ip::tcp::socket;

/// I/O context object type
using io_context_t = asio::io_context;

/// Protocol callback type. The callback is called as soon as server reply size is available.
using callback_t = std::function< void( void ) >;

// Header size, just a four byte int
const int HEADER_SIZE = sizeof( uint32_t ) / sizeof( char );

// codes for supported commands
namespace commands
{
constexpr const char *STREAM = "stream";
constexpr const char *MODEL_ZOO = "modelzoo";
constexpr const char *SLEEP = "sleep";
constexpr const char *SHUTDOWN = "shutdown";
constexpr const char *LABEL_DICT = "label_dictionary";
constexpr const char *SYSTEM_INFO = "system_info";
constexpr const char *TRACE_MANAGE = "trace_manage";
constexpr const char *ZOO_MANAGE = "zoo_manage";
constexpr const char *DEV_CTRL = "dev_ctrl";
}  // namespace commands

/// Error handling for asio errors (defined as macro to preserve file location info in error messages)
/// \param[in] ec - asio error code
/// \param[in] ignore_errors - if true, ignore errors and return 0 on error
/// \return false if error is serious, true otherwise
#define throw_exception_if_error_is_serious( ec, ignore_errors )                                           \
	( ( ( ec ) && ( ec ) != asio::error::eof ) ? ( ( ignore_errors ) ? false                               \
																	 : ( DG::ErrorHandling::errorAdd(      \
																			 __FILE__,                     \
																			 TOSTRING( __LINE__ ),         \
																			 FUNCTION_NAME,                \
																			 DG::ErrorType::RUNTIME_ERROR, \
																			 DG::ErrSystem,                \
																			 ( ec ).message() ),           \
																		 false ) )                         \
											   : true )

/// Run executor in I/O context to handle asynchronous operations
/// \param[in] io_context - I/O context object to run
/// \param[in] timeout_ms - running timeout in ms; 0 to run until completion of all tasks
/// \return number of completed tasks
inline size_t run_async( io_context_t &io_context, size_t timeout_ms = 0 )
{
	// If ASIO executor was stopped, we need to restart it
	if( io_context.stopped() )
		io_context.restart();

	// Run ASIO executor
	if( timeout_ms > 0 )
		return io_context.run_for( std::chrono::milliseconds( timeout_ms ) );
	else
		return io_context.run();
}

/// Stop executor
/// \param[in] io_context - I/O context object to run
inline void stop( io_context_t &io_context )
{
	io_context.stop();
}

/// Open socket and connect to server
/// \param[in,out] io_context - execution context
/// \param[in] ip - server domain name or IP address string
/// \param[in] port - server TCP port number
/// \param[in] timeout_s - intended timeout in seconds
/// \return socket object with established connection to server
inline socket_t
socket_connect( io_context_t &io_context, const std::string &ip, int port, size_t timeout_s, int retries = 3 )
{
	asio::error_code error;
	asio::ip::tcp::resolver resolver( io_context );
	asio::ip::tcp::resolver::results_type endpoints = resolver
														  .resolve( asio::ip::tcp::v4(), ip, std::to_string( port ) );
	socket_t ret( io_context );

	for( int attempt = 0; attempt < retries; attempt++ )
	{
		error = asio::error_code();  // clear error code

		// Start the asynchronous operation itself. The lambda that is used as a
		// callback will update the error variable when the operation completes.
		asio::async_connect(
			ret,
			endpoints,
			[ &error ]( const asio::error_code &result_error, const asio::ip::tcp::endpoint & /*result_endpoint*/ ) {
				error = result_error;
			} );

		// Run the operation until it completes, or until the timeout.
		run_async( io_context, timeout_s * 1000 );

		// If the asynchronous operation completed successfully then the io_context
		// would have been stopped due to running out of work. If it was not
		// stopped, then the io_context::run_for call must have timed out.
		if( !io_context.stopped() )
		{
			// Close the socket to cancel the outstanding asynchronous operation.
			ret.close();

			// Run the io_context again until the operation completes.
			io_context.run();
		}

		// Restart the io_context to leave it pristine
		io_context.restart();

		// Determine whether a connection was successfully established.
		if( !error )
			break;
	}

	// Report final error
	if( error )
		DG_ERROR(
			DG_FORMAT(
				"Error connecting to " << ip << ":" << port << " after " << retries << " retries with timeout "
									   << timeout_s << " s: " << error.message() ),
			ErrSystem );

	ret.set_option( asio::ip::tcp::no_delay( true ) );
	return ret;
}

/// Close given socket
inline void socket_close( socket_t &socket )
{
	socket.shutdown( socket_t::shutdown_both );
	socket.close();
}

/// Read all incoming data to buffer, synchronously
/// \param[in] socket - socket to use. Must be connected
/// \param[out] response_buffer - buffer for response. Will be resized as needed
/// \param[in] ignore_errors - if true, ignore errors and return 0 on error
/// \return number of read bytes
template< typename T = char >
size_t read( socket_t &socket, std::vector< T > &response_buffer, bool ignore_errors = false )
{
	asio::error_code error;
	size_t bytes_read = 0;
	uint32_t big_endian_size = 0;
	char *size_buffer = reinterpret_cast< char * >( &big_endian_size );
	size_t packet_size = 0;

	// Read first 4 bytes to get message length
	bytes_read = asio::read( socket, asio::buffer( size_buffer, HEADER_SIZE ), error );

	if( bytes_read == 0 )
		return 0;
	else if( bytes_read < HEADER_SIZE )
		DG_ERROR(
			"Fail to read incoming packet length from socket " + socket.remote_endpoint().address().to_string(),
			ErrOperationFailed );
	if( !throw_exception_if_error_is_serious( error, ignore_errors ) )
		return 0;

	// Use the length to allocate buffer
	packet_size = ntohl( big_endian_size );
	response_buffer.resize( packet_size );

	// Complete read
	bytes_read = asio::read( socket, asio::buffer( response_buffer ), error );
	if( !throw_exception_if_error_is_serious( error, ignore_errors ) )
		return 0;

	return bytes_read;
}

/// Read all incoming data to buffer, synchronously
/// \param[in] socket - socket to use. Must be connected
/// \param[out] response_buffer - pointer to tensor buffer for response. Will be resized as needed
/// \param[in] ignore_errors - if true, ignore errors and return 0 on error
/// \return number of read bytes
inline size_t read( socket_t &socket, BasicTensor *response_buffer, bool ignore_errors = false )
{
	asio::error_code error;
	size_t bytes_read = 0;
	uint32_t big_endian_size = 0;
	char *size_buffer = reinterpret_cast< char * >( &big_endian_size );
	size_t packet_size = 0;

	DG_ASSERT( response_buffer != nullptr );

	// Read first 4 bytes to get message length
	bytes_read = asio::read( socket, asio::buffer( size_buffer, HEADER_SIZE ), error );

	if( bytes_read == 0 )
		return 0;
	else if( bytes_read < HEADER_SIZE )
		DG_ERROR(
			"Fail to read incoming packet length from socket " + socket.remote_endpoint().address().to_string(),
			ErrOperationFailed );
	if( !throw_exception_if_error_is_serious( error, ignore_errors ) )
		return 0;

	// Use the length to allocate buffer
	packet_size = ntohl( big_endian_size );
	response_buffer->alloc< char >( 0, "", { packet_size } );

	// Complete read
	bytes_read = asio::read( socket, asio::buffer( response_buffer->data< char >(), packet_size ), error );
	if( !throw_exception_if_error_is_serious( error, ignore_errors ) )
		return 0;

	return bytes_read;
}

/// Write data to socket, synchronously
/// \param[in] socket - socket to use. Must be connected
/// \param[in] request_buffer - data to send
/// \param[in] packet_size - size of data to send, in bytes
/// \param[in] ignore_errors - if true, ignore errors and return 0 on error
/// \return number of written bytes
inline size_t write( socket_t &socket, const char *request_buffer, size_t packet_size, bool ignore_errors = false )
{
	asio::error_code error;
	size_t bytes_sent = 0;
	uint32_t big_endian_size = 0;
	const char *size_buffer = reinterpret_cast< const char * >( &big_endian_size );

	// Prepare a 4 byte packet to signal message length
	assert( int( packet_size ) < ( std::numeric_limits< int32_t >::max )() );
	big_endian_size = htonl( static_cast< uint32_t >( packet_size ) );

	// Signal message length
	asio::write( socket, asio::buffer( size_buffer, HEADER_SIZE ), error );
	if( !throw_exception_if_error_is_serious( error, ignore_errors ) )
		return 0;

	// Send message
	bytes_sent += asio::write( socket, asio::buffer( request_buffer, packet_size ), error );
	if( !throw_exception_if_error_is_serious( error, ignore_errors ) )
		return 0;
	return bytes_sent;
}

/// Asynchronously write data to socket.
/// run_async() should be running in some worker thread to process event loop.
/// \param[in] socket - socket to use. Must be connected
/// \param[in] request_buffer - data to send
/// \param[in] packet_size - size of data to send, in bytes
/// \return completion waiting function.
/// It returns true if all bytes were sent, false otherwise.
/// It accepts timeout in ms. Pass zero to query current state without waiting.
inline std::function< bool( size_t ) > write_async( socket_t &socket, const char *request_buffer, size_t packet_size )
{
	// Prepare a 4 byte packet to signal message length
	assert( int( packet_size ) < ( std::numeric_limits< int32_t >::max )() );

	/// Data structure which stores async. write execution context
	struct WriteContext
	{
		const size_t bytes_total;              //!< total bytes to transfer
		std::atomic< size_t > bytes_sent = 0;  //!< transferred bytes
		std::condition_variable cv;            //!< condition variable to wait for transfer completion
		uint32_t big_endian_size;              //!< header to transfer
		std::vector< char > data;              //!< data to transfer

		/// Constructor
		/// \param[in] request_buffer - data to send
		/// \param[in] packet_size - size of data to send, in bytes
		WriteContext( const char *request_buffer, size_t packet_size ) :
			big_endian_size( htonl( static_cast< uint32_t >( packet_size ) ) ),
			bytes_total( HEADER_SIZE + packet_size ), data( packet_size )
		{
			std::memcpy( data.data(), request_buffer, packet_size );
		}
	};

	// Create write context
	auto write_context = std::make_shared< WriteContext >( request_buffer, packet_size );

	// Write completion event handler
	auto write_handler = [ write_context ]( const asio::error_code &ec, std::size_t bytes_transferred ) {
		write_context->bytes_sent += bytes_transferred;
		// Notify waiter
		write_context->cv.notify_all();
		throw_exception_if_error_is_serious( ec, false );
	};

	// Prepare buffers
	std::vector< asio::const_buffer > bufs = {
		asio::buffer( (const char *)&write_context->big_endian_size, HEADER_SIZE ),  // header
		asio::buffer( write_context->data )                                          // body
	};

	// Start sending message
	asio::async_write( socket, bufs, write_handler );

	// Return a function, which effectively waits until all bytes will be sent
	// (assuming run_async() is running in some worker thread to process event loop)
	return [ write_context ]( size_t timeout_ms ) -> bool {
		if( timeout_ms == 0 )
			return write_context->bytes_sent == write_context->bytes_total;
		else
		{
			std::mutex mx;
			std::unique_lock< std::mutex > lk( mx );
			const bool ret = write_context->cv.wait_for( lk, std::chrono::milliseconds( timeout_ms ), [ & ] {
				return write_context->bytes_sent == write_context->bytes_total;
			} );
			return ret;
		}
	};
}

/// Initiate a reply read, the result of which will be supplied to a callback. Use after sending a frame.
/// This function just queues a read of four bytes to get the message length. Do not call more than once before handling
/// the reply! run_async() should be running in some worker thread to process event loop. \param[in] socket - socket to
/// use. Must be connected \param[in] read_size - pointer to result packet size. Must be preallocated \param[in]
/// async_result_callback - callback for reply
inline void initiate_read( socket_t &socket, uint32_t *read_size, callback_t async_result_callback )
{
	char *read_size_buffer = reinterpret_cast< char * >( read_size );

	// Begin waiting for read
	asio::async_read(
		socket,
		asio::buffer( read_size_buffer, HEADER_SIZE ),
		[ &socket, read_size, async_result_callback ]( const asio::error_code &error, size_t bytes_transferred ) {
			// Handle the error
			throw_exception_if_error_is_serious( error, false );

			// Convert size to host endianness
			*read_size = ntohl( *read_size );

			// Call real callback
			async_result_callback();
		} );
}

/// Handle reading a reply. Call after the callback from initiate_read has received packet length
/// \param[in] socket - socket to use. Must be connected
/// \param[in] async_result_callback - callback for reply
/// \param[in] read_size - packet size to read in bytes as returned from initiate_read()
template< typename T = char >
inline void handle_read( socket_t &socket, std::vector< T > &response_buffer, uint32_t read_size )
{
	asio::error_code error;

	// We now know size of incoming message, and can queue its read synchronously, since it should be received
	// imminently
	response_buffer.resize( read_size );
	asio::read( socket, asio::buffer( response_buffer ), error );

	throw_exception_if_error_is_serious( error, false );
}

}  // namespace main_protocol
}  // namespace DG

#undef throw_exception_if_error_is_serious
