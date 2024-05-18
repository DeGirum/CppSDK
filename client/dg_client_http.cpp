
///////////////////////////////////////////////////////////////////////////////
/// \file dg_client_http.cpp
/// \brief Class, which implements client-side functionality of
///  DG client-server system based on HTTP+WebSockets protocol
///
/// Copyright DeGirum Corporation 2023
///
/// This file contains implementation of DG::Client class:
/// which handles HTTP+WebSockets protocol of DG client-server system
///

#include "dg_client_http.h"
#include "Utilities/dg_time_utilities.h"
#include "Utilities/easywsclient.hpp"

/// profiler group
DG_TRC_GROUP_DEF( AIClientHttp );

namespace DG
{

///
/// WebSocket client wrapper with worker thread
///
class WebSocketClient
{
public:
	/// Callback type.
	/// Argument: received text message
	using callback_t = std::function< void( const std::vector< uint8_t > & ) >;

	/// Constructor
	/// \param[in] url WebSocket URL
	WebSocketClient( const std::string &url ) :
		m_url( url ), m_ews_client( easywsclient::WebSocket::from_url_no_mask( url ) )
	{}

	/// Destructor
	~WebSocketClient()
	{
		if( m_ews_client != nullptr )
		{
			m_ews_client->close();
			if( m_worker.valid() )
			{
				try
				{
					m_worker.get();
				}
				catch( std::exception & )  // ignore all errors in destructor
				{}
			}
		}
	}

	/// Set received data dispatch callback
	/// \param[in] callback - callback function
	void callbackSet( callback_t callback )
	{
		DG_TRC_BLOCK( AIClientHttp, callbackSet, DGTrace::lvlFull );
		std::lock_guard< std::mutex > lk( m_mx );
		m_callback = callback;
		if( !m_worker.valid() )
		{
			DG_TRC_BLOCK( AIClientHttp, callbackSet : start_worker, DGTrace::lvlFull );
			m_worker = std::async( std::launch::async, &WebSocketClient::workerThread, this );
		}
	}

	/// Check worker thread for errors. Throw exception if any.
	void errorCheck()
	{
		DG_TRC_BLOCK( AIClientHttp, errorCheck, DGTrace::lvlFull );

		// if the worker thread is running, it will run until destructor is called;
		// if it becomes ready before that, it means it is finished with exception;
		// we call .get() to fire that exception
		if( m_worker.valid() && m_worker.wait_for( std::chrono::seconds( 0 ) ) == std::future_status::ready )
			m_worker.get();
	}

	/// Send binary message
	/// \param[in] data - binary data to send
	void binarySend( const std::vector< char > &data )
	{
		DG_TRC_BLOCK( AIClientHttp, binarySend, DGTrace::lvlDetailed );
		std::lock_guard< std::mutex > lk( m_mx );
		m_ews_client->sendBinary( data );  // buffer data
		m_ews_client->poll( 0 );           // send immediately
	}

	/// Send text message and receive reply
	/// \param[in] data - text to send
	/// \param[in] timeout_ms - timeout in milliseconds
	/// \return received text message
	std::string textSendReceive( const std::string &data, int timeout_ms )
	{
		DG_TRC_BLOCK( AIClientHttp, textSendReceive, DGTrace::lvlDetailed );
		std::lock_guard< std::mutex > lk( m_mx );

		std::string received_msg;
		auto dispather = [ &received_msg ]( const std::string &msg ) {
			received_msg = msg;
		};

		const int poll_interval_ms = std::max( 1, timeout_ms / 10 );
		auto poller = [ & ]() {
			if( m_ews_client->poll( poll_interval_ms ) )
				m_ews_client->dispatch( dispather );
			return !received_msg.empty();
		};

		m_ews_client->send( data );  // buffer data

		if( !DG::pollingWaitFor( poller, timeout_ms ) )
			DG_ERROR(
				DG_FORMAT( "Timeout " << timeout_ms << " ms communicating with WebSocket server at " << m_url ),
				ErrTimeout );
		return received_msg;
	}

	/// Compose WebSocket URL from parts
	/// \param[in] host - hostname
	/// \param[in] port - IP port
	/// \param[in] route - URL route suffix
	/// \return full URL
	static std::string urlCompose( const std::string &host, int port, const std::string &route )
	{
		return "ws://" + host + ":" + std::to_string( port ) + route;
	}

private:
	std::string m_url;                                        //!< WebSocket URL
	std::shared_ptr< easywsclient::WebSocket > m_ews_client;  //!< WebSocket client pointer
	std::future< void > m_worker;                             //!< worker thread future
	std::mutex m_mx;                                          //!< mutex to protect access to m_ews_client
	callback_t m_callback = nullptr;                          //!< callback function

	/// Worker thread
	/// \param[in] me - pointer to self
	static void workerThread( WebSocketClient *me )
	{
		DG_TRC_BLOCK( AIClientHttp, workerThread, DGTrace::lvlFull );
		const int poll_interval_ms = 50;

		while( me->m_ews_client->getReadyState() != easywsclient::WebSocket::CLOSED )
		{
			DG_TRC_POINT( AIClientHttp, workerThread : loop, DGTrace::lvlFull );

			// wait for socket to be ready
			if( me->m_ews_client->poll( -poll_interval_ms ) )  // negative timeout means just wait
			{
				DG_TRC_POINT( AIClientHttp, workerThread : poll1, DGTrace::lvlFull );

				std::unique_lock< std::mutex > lk( me->m_mx );

				// do send/receive
				me->m_ews_client->poll( 0 );

				DG_TRC_POINT( AIClientHttp, workerThread : poll2, DGTrace::lvlFull );

				// dispatch received messages
				if( me->m_callback )
				{
					auto dispather = [ me, &lk ]( const std::vector< uint8_t > &message ) {
						lk.unlock();
						me->m_callback( message );
						lk.lock();
					};
					me->m_ews_client->dispatchBinary( dispather );
				}
			}
		}
	}
};

///////////////////////////////////////////////////////////////////////////////
// ClientHttp class implementation

//
// Constructor. Sets up active server using address.
// [in] server_address - server address
// [in] connection_timeout_ms - connection timeout in milliseconds
// [in] inference_timeout_ms - AI server inference timeout in milliseconds
//
ClientHttp::ClientHttp(
	const ServerAddress &server_address,
	size_t connection_timeout_ms,
	size_t inference_timeout_ms ) :
	m_server_address( server_address ),
	m_connection_timeout_ms( connection_timeout_ms ), m_inference_timeout_ms( inference_timeout_ms ),
	m_ws_client( nullptr ), m_async_result_callback( nullptr ), m_frame_queue_depth( 0 ),
	m_http_client( server_address )
{
	DG_TRC_BLOCK( AIClientHttp, constructor, DGTrace::lvlBasic );

	m_http_client.set_keep_alive( true );         // keep connection alive
	m_http_client.set_address_family( AF_INET );  // IPv4 only
	m_http_client.set_connection_timeout( std::chrono::milliseconds( m_connection_timeout_ms ) );
	m_http_client.set_read_timeout( std::chrono::milliseconds( m_connection_timeout_ms ) );
	m_http_client.set_write_timeout( std::chrono::milliseconds( m_connection_timeout_ms ) );
}

// Destructor
ClientHttp::~ClientHttp()
{
	DG_TRC_BLOCK( AIClientHttp, destructor, DGTrace::lvlBasic );
	try
	{
		dataEnd();
		closeStream();
	}
	catch( ... )
	{}
}

//
// Check HTTP result for errors
// [in] result - HTTP result to check
// [in] path - URL path string
//
void ClientHttp::checkHttpResult( const httplib::Result &result, const std::string &path )
{
	auto prefix = [ & ]() {
		return DG_FORMAT(
			"Error sending HTTP request '" << path << "' to " << std::string( m_server_address ) << ": " );
	};
	if( result.error() != httplib::Error::Success )
		DG_ERROR( DG_FORMAT( prefix() + httplib::to_string( result.error() ) ), ErrOperationFailed );
	else if( result->status >= 300 || result->status < 200 )
		DG_ERROR(
			DG_FORMAT( prefix() << result->reason << "(" << result->status << ") " << result->body ),
			ErrOperationFailed );
}

//
// Send shutdown request to AI server
//
void ClientHttp::shutdown()
{
	DG_TRC_BLOCK( AIClientHttp, shutdown, DGTrace::lvlBasic );
	httpRequest< POST >( "/v1/sleep/0" );  // first ping the server so we will get exception if it is not running
	try
	{
		httpRequest< POST >( "/v1/shutdown" );  // then send shutdown command and ignore exceptions
												// (server may stop before reply is sent)
	}
	catch( ... )
	{}
}

//
// Get model label dictionary
// [in] model_name specifies the AI model.
// return JSON object containing model label dictionary
//
json ClientHttp::labelDictionary( const std::string &model_name )
{
	DG_TRC_BLOCK( AIClientHttp, labelDictionary, DGTrace::lvlBasic );
	auto result = httpRequest< GET >( "/v1/label_dictionary/" + model_name );
	return DG_JSON_PARSE( result->body );
}

//
// Get list of models in all model zoos of all active servers
// [out] modelzoo_list - array of models in model zoos
//
void ClientHttp::modelzooListGet( std::vector< DG::ModelInfo > &modelzoo_list )
{
	DG_TRC_BLOCK( AIClientHttp, modelzooListGet, DGTrace::lvlBasic );
	auto result = httpRequest< GET >( "/v1/modelzoo" );
	auto model_map = DG_JSON_PARSE( result->body );

	modelzoo_list.clear();
	modelzoo_list.reserve( model_map.size() );
	for( const auto &model : model_map.items() )
	{
		const auto model_name = model.key();
		DG::ModelParamsWriter mparams( model.value() );
		DG::ModelInfo mi;

		mi.name = model_name;
		mi.extended_params = mparams;
		modelzoo_list.push_back( mi );
	}
}

//
// Return host system information dictionary
//
json ClientHttp::systemInfo()
{
	DG_TRC_BLOCK( AIClientHttp, systemInfo, DGTrace::lvlBasic );
	auto result = httpRequest< GET >( "/v1/system_info" );
	return DG_JSON_PARSE( result->body );
}

//
// AI server tracing facility management
// [in] req - management request
// return results of management request completion (request-specific)
//
json ClientHttp::traceManage( const json &req )
{
	DG_TRC_BLOCK( AIClientHttp, traceManage, DGTrace::lvlBasic );
	auto result = httpRequest< POST >( "/v1/trace_manage", req.dump(), "application/json" );
	return DG_JSON_PARSE( result->body );
}

//
// AI server model zoo management
// [in] req - management request
// return results of management request completion (request-specific)
//
json ClientHttp::modelZooManage( const json &req )
{
	DG_TRC_BLOCK( AIClientHttp, modelZooManage, DGTrace::lvlBasic );
	auto result = httpRequest< POST >( "/v1/zoo_manage", req.dump(), "application/json" );
	return DG_JSON_PARSE( result->body );
}

//
// Ping server with an instantaneous command
// [in] sleep_ms - optional server-side sleep time in milliseconds
// [in] ignore_errors - when true, ignore all errors and return false, otherwise throw exception
// return true if no error occurred during the ping
//
bool ClientHttp::ping( double sleep_ms, bool ignore_errors )
{
	DG_TRC_BLOCK( AIClientHttp, ping, DGTrace::lvlBasic );

	try
	{
		httpRequest< POST >( "/v1/sleep/" + std::to_string( sleep_ms ) );
		return true;
	}
	catch( ... )
	{
		if( ignore_errors )
			return false;
		throw;
	}
}

//
// Creates and opens socket for stream of frames to be used by subsequent predict() calls
// [in] model_name - model name, which defines op destination
// [in] frame_queue_depth - the depth of internal frame queue
// [in] additional_model_parameters - additional model parameters in JSON format compatible with DG JSON model
// configuration
//
void ClientHttp::openStream(
	const std::string &model_name,
	size_t frame_queue_depth,
	const json &additional_model_parameters )
{
	DG_TRC_BLOCK( AIClientHttp, openStream, DGTrace::lvlBasic );
	m_frame_queue_depth = frame_queue_depth;

	if( m_ws_client != nullptr )
		closeStream();

	m_ws_client = new WebSocketClient(
		WebSocketClient::urlCompose( m_server_address.ip, m_server_address.port, "/v1/stream" ) );

	// configure connection
	json req = { { "name", model_name }, { "config", additional_model_parameters } };
	const json resp = DG::JsonHelper::jsonDeserializeStr(
		m_ws_client->textSendReceive( req.dump(), (int)m_connection_timeout_ms ) );
	DG::JsonHelper::errorCheck(
		resp,
		DG_FORMAT( "Error configuring model " << model_name << " on AI server " << std::string( m_server_address ) ) );

	// clear last error
	{
		std::lock_guard< std::mutex > lock( m_state );
		m_state.m_last_error = "";
	}

	// re-set callback, if it was set before opening stream
	resultObserve( m_async_result_callback );
}

//
// Close stream opened by openStream()
//
void ClientHttp::closeStream( void )
{
	DG_TRC_BLOCK( AIClientHttp, closeStream, DGTrace::lvlBasic );
	if( m_ws_client != nullptr )
	{
		delete m_ws_client;
		m_ws_client = nullptr;
	}
}

//
// Install prediction results observation callback
// [in] callback - user callback, which will be called as soon as prediction result is ready when using dataSend()
// methods
//
void ClientHttp::resultObserve( callback_t callback )
{
	DG_TRC_BLOCK( AIClientHttp, resultObserve, DGTrace::lvlBasic );

	m_async_result_callback = callback;

	auto callback_adapter = [ this ]( const std::vector< uint8_t > &raw_data ) {
		DG_TRC_BLOCK( AIClientHttp, callback_adapter, DGTrace::lvlDetailed );

		// parse result and check for error
		const json result = DG::JsonHelper::jsonDeserialize( raw_data );
		const std::string err_msg = DG::JsonHelper::errorCheck( result, "", false );

		std::unique_lock< std::mutex > lock( m_state );               // acquire runtime state lock
		std::string frame_info = m_state.m_frame_info_queue.front();  // get frame info
		const bool was_error = !m_state.m_last_error.empty();         // check if there was an error before

		// save last error
		if( !err_msg.empty() )
			m_state.m_last_error = err_msg;

		// invoke user callback; not under lock; catch and ignore all errors;
		// do it only for the first error - skip for consecutive errors to avoid race conditions
		// when callback is called in parallel with main thread after dataEnd() exits
		if( !was_error )
		{
			lock.unlock();
			try
			{
				m_async_result_callback( result, frame_info );
			}
			catch( ... )
			{}
			lock.lock();
		}

		// remove frame info from queue and notify result receiving thread;
		// do it after invoking user callback
		m_state.m_frame_info_queue.pop();
		m_waiter.notify_all();
	};

	if( m_ws_client != nullptr )
		m_ws_client->callbackSet(
			m_async_result_callback != nullptr ? callback_adapter : WebSocketClient::callback_t() );
}

//
// Wait until the number of outstanding frames becomes less or equal to the given value or until error is detected
// [in] outstanding_frames - number of outstanding frames to wait for
// [in] lock - acquired lock to use for waiting
// Throws error on timeout
// return true if no error occurred during the wait
//
bool ClientHttp::waitFor( size_t outstanding_frames, std::unique_lock< std::mutex > &lock )
{
	DG_TRC_BLOCK( AIClientHttp, waitFor, DGTrace::lvlDetailed );

	try
	{
		// wait until number of outstanding frames becomes less or equal to the given value
		for( auto cur_size = m_state.m_frame_info_queue.size();
			 cur_size > outstanding_frames && m_state.m_last_error.empty();
			 cur_size = m_state.m_frame_info_queue.size() )
		{
			if( !m_waiter.wait_for( lock, std::chrono::milliseconds( m_inference_timeout_ms ), [ & ] {
					m_ws_client->errorCheck();
					return m_state.m_frame_info_queue.size() < cur_size || !m_state.m_last_error.empty();
				} ) )
			{
				DG_ERROR(
					DG_FORMAT(
						"Timeout " << m_inference_timeout_ms << " ms waiting for inference completion on AI server '"
								   << std::string( m_server_address ) << " (current queue size is " << cur_size
								   << ")" ),
					ErrTimeout );
			}
		}
	}
	catch( std::exception &e )
	{
		m_state.m_last_error = e.what();  // preserve exception as last error
		throw;
	}
	return m_state.m_last_error.empty();
}

//
// Send given data frame for prediction. Prerequisites:
//   stream should be opened by openStream();
//   user callback to receive prediction results should be installed by resultObserve().
// On the very first frame the method launches result receiving thread. This thread asynchronously
// retrieves prediction results and for each result calls user callback installed by resultObserve().
// To terminate this thread dataEnd() should be called when no more data frames are expected.
// [in] data - array containing frame data
//
void ClientHttp::dataSend( const std::vector< std::vector< char > > &data, const std::string &frame_info )
{
	DG_TRC_BLOCK( AIClientHttp, dataSend, DGTrace::lvlDetailed );

	if( m_ws_client == nullptr )
		DG_ERROR( "dataSend: socket was not opened", ErrIncorrectAPIUse );

	if( m_async_result_callback == nullptr )
		DG_ERROR( "dataSend: observation callback is not installed", ErrIncorrectAPIUse );

	{
		// acquire runtime state lock
		std::unique_lock< std::mutex > lock( m_state );

		// wait until there is a space in outstanding frame queue
		if( !waitFor( m_frame_queue_depth - 1, lock ) )
			return;  // do not post new frames if error was detected

		// put frame info into the outstanding frame queue
		m_state.m_frame_info_queue.push( frame_info );
	}

	// send frame to the server
	for( const auto &d : data )
		m_ws_client->binarySend( d );
}

//
// Finalize the sequence of data frames. Should be called when no more data frames are
// expected to terminate result receiving thread started by dataSend().
//
void ClientHttp::dataEnd()
{
	DG_TRC_BLOCK( AIClientHttp, dataEnd, DGTrace::lvlBasic );

	std::unique_lock< std::mutex > lock( m_state );
	waitFor( 0, lock );
}

//
// Run prediction on given data frame. Stream should be opened by openStream()
// [in] data - array containing frame data
// [out] output - prediction result (JSON array)
//
void ClientHttp::predict( std::vector< std::vector< char > > &data, json &output )
{
	DG_TRC_BLOCK( AIClientHttp, predict::vector, DGTrace::lvlBasic );

	if( m_async_result_callback != nullptr )
		DG_ERROR(
			"cannot perform single-frame inference: client was configured for streaming inference",
			ErrIncorrectAPIUse );

	auto manage_callback = DG::RAII_Cleanup(
		[ & ]() { resultObserve( [ & ]( const json &result, const std::string &frame_info ) { output = result; } ); },
		[ & ]() { resultObserve( callback_t() ); } );

	dataSend( data, "" );
	dataEnd();

	if( !m_state.m_last_error.empty() )
		throw DGException( m_state.m_last_error );
}

}  // namespace DG
