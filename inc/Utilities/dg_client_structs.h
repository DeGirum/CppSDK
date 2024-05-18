//////////////////////////////////////////////////////////////////////////////
/// \file dg_client_structs.h
/// \brief DG client API data types
///
/// This file contains declarations of various data types
/// of DG client API:
/// - device types
/// - runtime agent types
/// - model parameters, etc.
///

// Copyright DeGirum Corporation 2021
// All rights reserved

//
// ****  ATTENTION!!! ****
//
// This file is used to build client documentation using Doxygen.
// Please clearly describe all public entities declared in this file,
// using full and grammatically correct sentences, avoiding acronyms,
// not omitting articles.
//

#ifndef DG_CLIENT_STRUCTS_H_
#define DG_CLIENT_STRUCTS_H_

#include <stdlib.h>
#include <string>
#include <vector>
#include "Utilities/dg_model_parameters.h"

namespace DG
{
/// client-server protocol version tag
constexpr const char *PROTOCOL_VERSION_TAG = "VERSION";

/// minimum compatible client-server protocol version
const int MIN_COMPATIBLE_PROTOCOL_VERSION = 4;

/// current client-server protocol version
const int CURRENT_PROTOCOL_VERSION = 4;

/// Default TCP port of AI server
const int DEFAULT_PORT = 8778;

/// Default connection timeout, ms
const int DEFAULT_CONNECTION_TIMEOUT_MS = 10000;

/// Default inference timeout, ms
const int DEFAULT_INFERENCE_TIMEOUT_MS = 180000;

/// Default frame queue depth
const int DEFAULT_FRAME_QUEUE_DEPTH = 8;

/// Server protocol type
enum class ServerType
{
	Unknown,  //!< Not set
	ASIO,     //!< proprietary TCP socket server protocol
	HTTP,     //!< HTTP server protocol
};

/// ServerAddress is the server address structure.
/// It keeps AI server TCP/IP address, port, and server protocol type.
struct ServerAddress
{
	/// Constructor
	/// \param[in] ip - server domain name or IP address
	/// \param[in] port - server TCP port
	/// \param[in] server_type - server protocol type
	ServerAddress( const std::string &ip, int port, ServerType server_type ) :
		ip( ip ), port( port ), server_type( server_type )
	{}

	/// Construct server address from hostname
	/// \param[in] hostname - server domain name or IP address with optional port suffix and protocol prefix (http:// or
	/// asio://) \return server address object
	static ServerAddress fromHostname( const std::string &hostname )
	{
		std::string pure_hostname = hostname;

		// deduce server protocol type
		ServerType server_type = ServerType::ASIO;

		const std::vector< std::pair< std::string, ServerType > > prefixes =
			{ { "http://", ServerType::HTTP }, { "asio://", ServerType::ASIO } };

		for( const auto &prefix : prefixes )
		{
			if( const auto delim = pure_hostname.find( prefix.first ); delim != std::string::npos )
			{
				server_type = prefix.second;
				pure_hostname = pure_hostname.substr( delim + prefix.first.length() );
				break;
			}
		}

		// deduce TCP port
		if( const auto delim = pure_hostname.rfind( ":" ); delim != std::string::npos )
			return ServerAddress(
				pure_hostname.substr( 0, delim ),
				std::atoi( pure_hostname.substr( delim + 1 ).c_str() ),
				server_type );

		return ServerAddress( pure_hostname, DEFAULT_PORT, server_type );
	}

	/// Check server address validity
	/// \return true if server address is valid
	const bool is_valid() const
	{
		return !ip.empty();
	}

	/// Convert server address to string
	operator std::string() const
	{
		return ( server_type == ServerType::HTTP ? "http://" : "" ) + ip + ":" + std::to_string( port );
	}

	std::string ip;                                //!< server domain name or IP address string
	int port = DEFAULT_PORT;                       //!< server TCP port number
	ServerType server_type = ServerType::Unknown;  //!< server protocol type
};

/// ModelInfo is the model identification structure. It keeps AI model key attributes.
typedef struct ModelInfo
{
	std::string name;                       //!< model string name
	DG::ModelParamsWriter extended_params;  //!< extended model parameters
} ModelInfo;

/// Prepare client-server response message json from input json
/// \param in: server response json
/// \return client response json
static json messagePrepareJson( const json &in )
{
	DG_ERROR_TRUE( in.is_object() );
	if( in.contains( PROTOCOL_VERSION_TAG ) )
		return in;
	auto out = in;
	out[ PROTOCOL_VERSION_TAG ] = DG::CURRENT_PROTOCOL_VERSION;
	return out;
}

/// prepare client-server response message string from input json
/// \param in: server response json
/// \return client response string
static std::string messagePrepare( const json &in )
{
	return messagePrepareJson( in ).dump();
}

}  // namespace DG

#endif
