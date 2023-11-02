/*
The MIT License (MIT)

Copyright (c) 2012, 2013 <dhbaird@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include "DGErrorHandling.h"

// profiler group
DG_TRC_GROUP_DEF( easywsclient );

#ifdef _WIN32
#if defined( _MSC_VER ) && !defined( _CRT_SECURE_NO_WARNINGS )
#define _CRT_SECURE_NO_WARNINGS  // _CRT_SECURE_NO_WARNINGS for sscanf errors in MSVC2013 Express
#endif
#pragma warning(disable : 4244 4267)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <fcntl.h>
#pragma comment( lib, "ws2_32" )
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#ifndef _SOCKET_T_DEFINED
typedef SOCKET socket_t;
#define _SOCKET_T_DEFINED
#endif
#ifndef snprintf
#define snprintf _snprintf_s
#endif
#if _MSC_VER >= 1600
// vs2010 or later
#include <stdint.h>
#else
typedef __int8 int8_t;
typedef unsigned __int8 uint8_t;
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#endif
#define socketerrno               WSAGetLastError()
#define SOCKET_EAGAIN_EINPROGRESS WSAEINPROGRESS
#define SOCKET_EWOULDBLOCK        WSAEWOULDBLOCK
#else
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#ifndef _SOCKET_T_DEFINED
typedef int socket_t;
#define _SOCKET_T_DEFINED
#endif
#ifndef INVALID_SOCKET
#define INVALID_SOCKET ( -1 )
#endif
#ifndef SOCKET_ERROR
#define SOCKET_ERROR ( -1 )
#endif
#define closesocket( s ) ::close( s )
#include <errno.h>
#define socketerrno               errno
#define SOCKET_EAGAIN_EINPROGRESS EAGAIN
#define SOCKET_EWOULDBLOCK        EWOULDBLOCK
#endif

#include <string>
#include <vector>

#include "easywsclient.hpp"

using easywsclient::BytesCallback_Imp;
using easywsclient::Callback_Imp;

namespace
{  // private module-only namespace

socket_t hostname_connect( const std::string &hostname, int port )
{
	DG_TRC_BLOCK( easywsclient, hostname_connect, DGTrace::lvlDetailed, "hostname %s:%d", hostname.c_str(), port );

	struct addrinfo hints;
	struct addrinfo *result;
	struct addrinfo *p;
	int ret;
	socket_t sockfd = INVALID_SOCKET;
	char sport[ 16 ];
	memset( &hints, 0, sizeof( hints ) );
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	snprintf( sport, 16, "%d", port );
	if( ( ret = getaddrinfo( hostname.c_str(), sport, &hints, &result ) ) != 0 )
	{
		return INVALID_SOCKET;
	}

	DG_TRC_POINT( easywsclient, hostname_connect : getaddrinfo, DGTrace::lvlFull );

	for( p = result; p != NULL; p = p->ai_next )
	{
		DG_TRC_BLOCK( easywsclient, hostname_connect : connect_loop, DGTrace::lvlFull );

		sockfd = socket( p->ai_family, p->ai_socktype, p->ai_protocol );
		if( sockfd == INVALID_SOCKET )
		{
			continue;
		}
		if( connect( sockfd, p->ai_addr, p->ai_addrlen ) != SOCKET_ERROR )
		{
			break;
		}
		closesocket( sockfd );
		sockfd = INVALID_SOCKET;
	}
	freeaddrinfo( result );
	return sockfd;
}

class _DummyWebSocket: public easywsclient::WebSocket
{
public:
	bool poll( int timeout )
	{
		return true;
	}
	void send( const std::string &message )
	{}
	void sendBinary( const std::string &message )
	{}
	void sendBinary( const std::vector< uint8_t > &message )
	{}
	void sendBinary( const std::vector< char > &message )
	{}
	void sendPing()
	{}
	void close()
	{}
	readyStateValues getReadyState() const
	{
		return CLOSED;
	}
	void _dispatch( Callback_Imp &callable )
	{}
	void _dispatchBinary( BytesCallback_Imp &callable )
	{}
};

class _RealWebSocket: public easywsclient::WebSocket
{
public:
	// http://tools.ietf.org/html/rfc6455#section-5.2  Base Framing Protocol
	//
	//  0                   1                   2                   3
	//  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	// +-+-+-+-+-------+-+-------------+-------------------------------+
	// |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
	// |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
	// |N|V|V|V|       |S|             |   (if payload len==126/127)   |
	// | |1|2|3|       |K|             |                               |
	// +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
	// |     Extended payload length continued, if payload len == 127  |
	// + - - - - - - - - - - - - - - - +-------------------------------+
	// |                               |Masking-key, if MASK set to 1  |
	// +-------------------------------+-------------------------------+
	// | Masking-key (continued)       |          Payload Data         |
	// +-------------------------------- - - - - - - - - - - - - - - - +
	// :                     Payload Data continued ...                :
	// + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
	// |                     Payload Data continued ...                |
	// +---------------------------------------------------------------+
	struct wsheader_type
	{
		unsigned header_size;
		bool fin;
		bool mask;
		enum opcode_type
		{
			CONTINUATION = 0x0,
			TEXT_FRAME = 0x1,
			BINARY_FRAME = 0x2,
			CLOSE = 8,
			PING = 9,
			PONG = 0xa,
		} opcode;
		int N0;
		uint64_t N;
		uint8_t masking_key[ 4 ];
	};

	std::vector< uint8_t > rxbuf_;
	std::vector< uint8_t > txbuf_;
	std::vector< uint8_t > receivedData_;

	const socket_t sockfd_;
	readyStateValues readyState_;
	bool useMask_;
	bool isRxBad_;

	_RealWebSocket( socket_t sockfd, bool useMask ) : sockfd_( sockfd ), readyState_( OPEN ), useMask_( useMask ), isRxBad_( false )
	{}

	readyStateValues getReadyState() const
	{
		return readyState_;
	}

	bool poll( int timeout )
	{
		DG_TRC_BLOCK( easywsclient, poll, DGTrace::lvlDetailed, "timeout %d", timeout );

		std::string error;
		bool socket_ready = false;

		// timeout in milliseconds
		if( readyState_ == CLOSED )
			return socket_ready;

		if( timeout != 0 )
		{
			fd_set rfds;
			fd_set wfds;
			const auto abs_timeout = std::abs( timeout );
			timeval tv = { abs_timeout / 1000, ( abs_timeout % 1000 ) * 1000 };
			FD_ZERO( &rfds );
			FD_ZERO( &wfds );
			FD_SET( sockfd_, &rfds );
			if( txbuf_.size() )
			{
				FD_SET( sockfd_, &wfds );
			}
			socket_ready = select( sockfd_ + 1, &rfds, &wfds, 0, &tv ) > 0;

			DG_TRC_POINT_MSG( easywsclient, poll : select, DGTrace::lvlFull, "socket_ready %d", (int)socket_ready );
		}

		if( timeout < 0 )
			return socket_ready;  // wait only

		while( true )
		{
			// FD_ISSET(0, &rfds) will be true
			int N = rxbuf_.size();
			int ret;
			const int frame_size = 1500;  // jumbo frame size limit
			rxbuf_.resize( N + frame_size );
			ret = recv( sockfd_, (char *)&rxbuf_[ 0 ] + N, frame_size, 0 );
			DG_TRC_POINT_MSG( easywsclient, poll : recv, DGTrace::lvlFull, "size %d", ret );
			const auto sock_ret = socketerrno;
			if( ret < 0 && ( sock_ret == SOCKET_EWOULDBLOCK || sock_ret == SOCKET_EAGAIN_EINPROGRESS ) )
			{
				rxbuf_.resize( N );
				break;
			}
			else if( ret <= 0 )
			{
				rxbuf_.resize( N );
				closesocket( sockfd_ );
				readyState_ = CLOSED;
				error = "WebSocket: connection error on receive: " + std::to_string( sock_ret );
				break;
			}
			else
			{
				rxbuf_.resize( N + ret );
			}
		}
		
		while( txbuf_.size() )
		{
			int ret = ::send( sockfd_, (char *)&txbuf_[ 0 ], txbuf_.size(), 0 );
			DG_TRC_POINT_MSG( easywsclient, poll : send, DGTrace::lvlFull, "size %d", ret );

			const auto sock_ret = socketerrno;
			if( ret < 0 && ( sock_ret == SOCKET_EWOULDBLOCK || sock_ret == SOCKET_EAGAIN_EINPROGRESS ) )
			{
				break;
			}
			else if( ret <= 0 )
			{
				closesocket( sockfd_ );
				readyState_ = CLOSED;
				error = "WebSocket: connection error on send: " + std::to_string( sock_ret );
				break;
			}
			else
			{
				txbuf_.erase( txbuf_.begin(), txbuf_.begin() + ret );
			}			
		}		

		if( !txbuf_.size() && readyState_ == CLOSING )
		{
			closesocket( sockfd_ );
			readyState_ = CLOSED;
		}

		if( !error.empty() )
			DG_ERROR( error, ErrOperationFailed );

		return socket_ready;
	}

	// Callable must have signature: void(const std::string & message).
	// Should work with C functions, C++ functors, and C++11 std::function and
	// lambda:
	// template<class Callable>
	// void dispatch(Callable callable)
	virtual void _dispatch( Callback_Imp &callable )
	{
		struct CallbackAdapter: public BytesCallback_Imp
		// Adapt void(const std::string<uint8_t>&) to void(const std::string&)
		{
			Callback_Imp &callable;
			CallbackAdapter( Callback_Imp &callable ) : callable( callable )
			{}
			void operator()( const std::vector< uint8_t > &message )
			{
				std::string stringMessage( message.begin(), message.end() );
				callable( stringMessage );
			}
		};
		CallbackAdapter bytesCallback( callable );
		_dispatchBinary( bytesCallback );
	}

	virtual void _dispatchBinary( BytesCallback_Imp &callable )
	{
		DG_TRC_BLOCK( easywsclient, _dispatchBinary, DGTrace::lvlDetailed );

		std::string error;

		if( isRxBad_ )
		{
			return;
		}
		while( true )
		{
			wsheader_type ws;
			if( rxbuf_.size() < 2 )
			{
				return; /* Need at least 2 */
			}
			const uint8_t *data = (uint8_t *)&rxbuf_[ 0 ];  // peek, but don't consume
			ws.fin = ( data[ 0 ] & 0x80 ) == 0x80;
			ws.opcode = ( wsheader_type::opcode_type )( data[ 0 ] & 0x0f );
			ws.mask = ( data[ 1 ] & 0x80 ) == 0x80;
			ws.N0 = ( data[ 1 ] & 0x7f );
			ws.header_size = 2 + ( ws.N0 == 126 ? 2 : 0 ) + ( ws.N0 == 127 ? 8 : 0 ) + ( ws.mask ? 4 : 0 );
			if( rxbuf_.size() < ws.header_size )
			{
				return; /* Need: ws.header_size - rxbuf_.size() */
			}
			int i = 0;
			if( ws.N0 < 126 )
			{
				ws.N = ws.N0;
				i = 2;
			}
			else if( ws.N0 == 126 )
			{
				ws.N = 0;
				ws.N |= ( (uint64_t)data[ 2 ] ) << 8;
				ws.N |= ( (uint64_t)data[ 3 ] ) << 0;
				i = 4;
			}
			else if( ws.N0 == 127 )
			{
				ws.N = 0;
				ws.N |= ( (uint64_t)data[ 2 ] ) << 56;
				ws.N |= ( (uint64_t)data[ 3 ] ) << 48;
				ws.N |= ( (uint64_t)data[ 4 ] ) << 40;
				ws.N |= ( (uint64_t)data[ 5 ] ) << 32;
				ws.N |= ( (uint64_t)data[ 6 ] ) << 24;
				ws.N |= ( (uint64_t)data[ 7 ] ) << 16;
				ws.N |= ( (uint64_t)data[ 8 ] ) << 8;
				ws.N |= ( (uint64_t)data[ 9 ] ) << 0;
				i = 10;
				if( ws.N & 0x8000000000000000ull )
				{
					// https://tools.ietf.org/html/rfc6455 writes the "the most
					// significant bit MUST be 0."
					//
					// We can't drop the frame, because (1) we don't we don't
					// know how much data to skip over to find the next header,
					// and (2) this would be an impractically long length, even
					// if it were valid. So just close() and return immediately
					// for now.
					isRxBad_ = true;
					error = "WebSocket: frame has invalid frame length";
					close();
					return;
				}
			}
			if( ws.mask )
			{
				ws.masking_key[ 0 ] = ( (uint8_t)data[ i + 0 ] ) << 0;
				ws.masking_key[ 1 ] = ( (uint8_t)data[ i + 1 ] ) << 0;
				ws.masking_key[ 2 ] = ( (uint8_t)data[ i + 2 ] ) << 0;
				ws.masking_key[ 3 ] = ( (uint8_t)data[ i + 3 ] ) << 0;
			}
			else
			{
				ws.masking_key[ 0 ] = 0;
				ws.masking_key[ 1 ] = 0;
				ws.masking_key[ 2 ] = 0;
				ws.masking_key[ 3 ] = 0;
			}

			// Note: The checks above should hopefully ensure this addition
			//       cannot overflow:
			if( rxbuf_.size() < ws.header_size + ws.N )
			{
				return; /* Need: ws.header_size+ws.N - rxbuf_.size() */
			}

			// We got a whole message, now do something with it:
			if( ws.opcode == wsheader_type::TEXT_FRAME || ws.opcode == wsheader_type::BINARY_FRAME || ws.opcode == wsheader_type::CONTINUATION )
			{
				if( ws.mask )
				{
					for( size_t i = 0; i != ws.N; ++i )
					{
						rxbuf_[ i + ws.header_size ] ^= ws.masking_key[ i & 0x3 ];
					}
				}

				receivedData_.resize( receivedData_.size() + (size_t)ws.N );
				std::memcpy( &receivedData_[ receivedData_.size() - (size_t)ws.N ], &rxbuf_[ ws.header_size ], (size_t)ws.N );
				if( ws.fin )
				{
					callable( receivedData_ );
					receivedData_.clear();
				}
			}
			else if( ws.opcode == wsheader_type::PING )
			{
				if( ws.mask )
				{
					for( size_t i = 0; i != ws.N; ++i )
					{
						rxbuf_[ i + ws.header_size ] ^= ws.masking_key[ i & 0x3 ];
					}
				}
				std::string data( rxbuf_.begin() + ws.header_size, rxbuf_.begin() + ws.header_size + (size_t)ws.N );
				sendData( wsheader_type::PONG, data.size(), data.c_str() );
			}
			else if( ws.opcode == wsheader_type::PONG )
			{}
			else if( ws.opcode == wsheader_type::CLOSE )
			{
				close();
			}
			else
			{
				error = "WebSocket: got unexpected WebSocket message";
				close();
			}

			rxbuf_.erase( rxbuf_.begin(), rxbuf_.begin() + ws.header_size + (size_t)ws.N );
		}
		if( !error.empty() )
			DG_ERROR( error, ErrOperationFailed );
	}

	void sendPing()
	{
		std::string empty;
		sendData( wsheader_type::PING, empty.size(), empty.c_str() );
	}

	void send( const std::string &message )
	{
		sendData( wsheader_type::TEXT_FRAME, message.size(), message.c_str() );
	}

	void sendBinary( const std::string &message )
	{
		sendData( wsheader_type::BINARY_FRAME, message.size(), message.c_str() );
	}

	void sendBinary( const std::vector< uint8_t > &message )
	{
		sendData( wsheader_type::BINARY_FRAME, message.size(), message.data() );
	}

	void sendBinary( const std::vector< char > &message )
	{
		sendData( wsheader_type::BINARY_FRAME, message.size(), message.data() );
	}

	template< class Element >
	void sendData( wsheader_type::opcode_type type, uint64_t message_size, Element *message_begin )
	{
		static_assert( sizeof( Element ) == 1 );
		DG_TRC_BLOCK( easywsclient, sendData, DGTrace::lvlDetailed, "type %d, size %zu", type, (size_t)message_size );

		// TODO:
		// Masking key should (must) be derived from a high quality random
		// number generator, to mitigate attacks on non-WebSocket friendly
		// middleware:
		const uint8_t masking_key[ 4 ] = { 0x12, 0x34, 0x56, 0x78 };

		if( readyState_ == CLOSING || readyState_ == CLOSED )
			return;

		std::vector< uint8_t > header;
		header.assign( 2 + ( message_size >= 126 ? 2 : 0 ) + ( message_size >= 65536 ? 6 : 0 ) + ( useMask_ ? 4 : 0 ), 0 );
		header[ 0 ] = 0x80 | type;
		if( message_size < 126 )
		{
			header[ 1 ] = ( message_size & 0xff ) | ( useMask_ ? 0x80 : 0 );
			if( useMask_ )
			{
				header[ 2 ] = masking_key[ 0 ];
				header[ 3 ] = masking_key[ 1 ];
				header[ 4 ] = masking_key[ 2 ];
				header[ 5 ] = masking_key[ 3 ];
			}
		}
		else if( message_size < 65536 )
		{
			header[ 1 ] = 126 | ( useMask_ ? 0x80 : 0 );
			header[ 2 ] = ( message_size >> 8 ) & 0xff;
			header[ 3 ] = ( message_size >> 0 ) & 0xff;
			if( useMask_ )
			{
				header[ 4 ] = masking_key[ 0 ];
				header[ 5 ] = masking_key[ 1 ];
				header[ 6 ] = masking_key[ 2 ];
				header[ 7 ] = masking_key[ 3 ];
			}
		}
		else
		{
			header[ 1 ] = 127 | ( useMask_ ? 0x80 : 0 );
			header[ 2 ] = ( message_size >> 56 ) & 0xff;
			header[ 3 ] = ( message_size >> 48 ) & 0xff;
			header[ 4 ] = ( message_size >> 40 ) & 0xff;
			header[ 5 ] = ( message_size >> 32 ) & 0xff;
			header[ 6 ] = ( message_size >> 24 ) & 0xff;
			header[ 7 ] = ( message_size >> 16 ) & 0xff;
			header[ 8 ] = ( message_size >> 8 ) & 0xff;
			header[ 9 ] = ( message_size >> 0 ) & 0xff;
			if( useMask_ )
			{
				header[ 10 ] = masking_key[ 0 ];
				header[ 11 ] = masking_key[ 1 ];
				header[ 12 ] = masking_key[ 2 ];
				header[ 13 ] = masking_key[ 3 ];
			}
		}
		// N.B. - txbuf_ will keep growing until it can be transmitted over the socket:
		txbuf_.resize( txbuf_.size() + header.size() + message_size );
		std::memcpy( &txbuf_[ txbuf_.size() - message_size - header.size() ], header.data(), header.size() );
		std::memcpy( &txbuf_[ txbuf_.size() - message_size ], message_begin, message_size );

		if( useMask_ )
		{
			size_t message_offset = txbuf_.size() - message_size;
			for( size_t i = 0; i != message_size; ++i )
			{
				txbuf_[ message_offset + i ] ^= masking_key[ i & 0x3 ];
			}
		}
	}

	void close()
	{
		DG_TRC_BLOCK( easywsclient, close, DGTrace::lvlDetailed );
		if( readyState_ == CLOSING || readyState_ == CLOSED )
		{
			return;
		}
		readyState_ = CLOSING;
		uint8_t closeFrame[ 6 ] = { 0x88, 0x80, 0x00, 0x00, 0x00, 0x00 };  // last 4 bytes are a masking key
		std::vector< uint8_t > header( closeFrame, closeFrame + 6 );
		txbuf_.insert( txbuf_.end(), header.begin(), header.end() );
	}
};

easywsclient::WebSocket::pointer from_url( const std::string &url, bool useMask, const std::string &origin )
{
	DG_TRC_BLOCK( easywsclient, from_url, DGTrace::lvlDetailed, "url %s", url.c_str() );

	char host[ 512 ];
	int port;
	char path[ 512 ];
	if( url.size() >= 512 )
	{
		DG_ERROR( "WebSocket: URL size limit exceeded", ErrBadParameter );
		return NULL;
	}
	if( origin.size() >= 200 )
	{
		DG_ERROR( "WebSocket: origin size limit exceeded", ErrBadParameter );
		return NULL;
	}

	if( sscanf( url.c_str(), "ws://%[^:/]:%d/%s", host, &port, path ) == 3 )
	{}
	else if( sscanf( url.c_str(), "ws://%[^:/]/%s", host, path ) == 2 )
	{
		port = 80;
	}
	else if( sscanf( url.c_str(), "ws://%[^:/]:%d", host, &port ) == 2 )
	{
		path[ 0 ] = '\0';
	}
	else if( sscanf( url.c_str(), "ws://%[^:/]", host ) == 1 )
	{
		port = 80;
		path[ 0 ] = '\0';
	}
	else
	{
		DG_ERROR( "WebSocket: could not parse WebSocket url: " + url, ErrBadParameter );
		return NULL;
	}

	socket_t sockfd = hostname_connect( host, port );
	if( sockfd == INVALID_SOCKET )
	{
		DG_ERROR( DG_FORMAT( "WebSocket: unable to connect to " << host << ":" << port ), ErrOperationFailed );
		return NULL;
	}
	{
		// XXX: this should be done non-blocking,
		char line[ 1024 ];
		int status;
		int i;
		snprintf( line, 1024, "GET /%s HTTP/1.1\r\n", path );
		::send( sockfd, line, strlen( line ), 0 );
		if( port == 80 )
		{
			snprintf( line, 1024, "Host: %s\r\n", host );
			::send( sockfd, line, strlen( line ), 0 );
		}
		else
		{
			snprintf( line, 1024, "Host: %s:%d\r\n", host, port );
			::send( sockfd, line, strlen( line ), 0 );
		}
		snprintf( line, 1024, "Upgrade: websocket\r\n" );
		::send( sockfd, line, strlen( line ), 0 );
		snprintf( line, 1024, "Connection: Upgrade\r\n" );
		::send( sockfd, line, strlen( line ), 0 );
		if( !origin.empty() )
		{
			snprintf( line, 1024, "Origin: %s\r\n", origin.c_str() );
			::send( sockfd, line, strlen( line ), 0 );
		}
		snprintf( line, 1024, "Sec-WebSocket-Key: x3JJHMbDL1EzLkh9GBhXDw==\r\n" );
		::send( sockfd, line, strlen( line ), 0 );
		snprintf( line, 1024, "Sec-WebSocket-Version: 13\r\n" );
		::send( sockfd, line, strlen( line ), 0 );
		snprintf( line, 1024, "\r\n" );
		::send( sockfd, line, strlen( line ), 0 );

		DG_TRC_POINT( easywsclient, from_url : send_done, DGTrace::lvlFull );

		for( i = 0; i < 2 || ( i < 1023 && line[ i - 2 ] != '\r' && line[ i - 1 ] != '\n' ); ++i )
		{
			if( recv( sockfd, line + i, 1, 0 ) == 0 )
			{
				return NULL;
			}
		}
		line[ i ] = 0;
		if( i == 1023 )
		{
			DG_ERROR( DG_FORMAT( "WebSocket: got invalid status line connecting to " << url ), ErrOperationFailed );
			return NULL;
		}
		if( sscanf( line, "HTTP/1.1 %d", &status ) != 1 || status != 101 )
		{
			DG_ERROR( DG_FORMAT( "WebSocket: got bad status connecting to " << url ), ErrOperationFailed );
			return NULL;
		}

		DG_TRC_POINT( easywsclient, from_url : recv_done, DGTrace::lvlFull );

		// TODO: verify response headers,
		while( true )
		{
			for( i = 0; i < 2 || ( i < 1023 && line[ i - 2 ] != '\r' && line[ i - 1 ] != '\n' ); ++i )
			{
				if( recv( sockfd, line + i, 1, 0 ) == 0 )
				{
					return NULL;
				}
			}
			if( line[ 0 ] == '\r' && line[ 1 ] == '\n' )
			{
				break;
			}
		}

		DG_TRC_POINT( easywsclient, from_url : verify_done, DGTrace::lvlFull );

	}
	int flag = 1;
	setsockopt( sockfd, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof( flag ) );  // Disable Nagle's algorithm
#ifdef _WIN32
	u_long on = 1;
	ioctlsocket( sockfd, FIONBIO, &on );
#else
	fcntl( sockfd, F_SETFL, O_NONBLOCK );
#endif

	return easywsclient::WebSocket::pointer( new _RealWebSocket( sockfd, useMask ) );
}

}  // namespace

namespace easywsclient
{

WebSocket::pointer WebSocket::create_dummy()
{
	static pointer dummy = pointer( new _DummyWebSocket );
	return dummy;
}

WebSocket::pointer WebSocket::from_url( const std::string &url, const std::string &origin )
{
	return ::from_url( url, true, origin );
}

WebSocket::pointer WebSocket::from_url_no_mask( const std::string &url, const std::string &origin )
{
	return ::from_url( url, false, origin );
}

}  // namespace easywsclient
