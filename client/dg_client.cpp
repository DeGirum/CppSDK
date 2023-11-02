///////////////////////////////////////////////////////////////////////////////
/// \file dg_client.cpp
/// \brief Class, which implements client-side functionality of
///  DG client-server system
///
/// Copyright DeGirum Corporation 2023
///
/// This file contains implementation of DG::Client class:
/// which handles protocols of DG client-server system
///

#include "dg_client.h"
#include "dg_client_asio.h"
#include "dg_client_http.h"

//
// Class factory. Creates client object based on selected protocol.
// [in] server_type - server protocol type
// [in] server_address - server address
// [in] connection_timeout_ms - connection timeout in milliseconds
// [in] inference_timeout_ms - AI server inference timeout in milliseconds
// return - pointer to client object
//
DG::Client::ClientPtr DG::Client::create(
	const std::string &server_address,
	size_t connection_timeout_ms,
	size_t inference_timeout_ms )
{
	ClientPtr client;

	const auto addr = ServerAddress::fromHostname( server_address );

	switch( addr.server_type )
	{
	case ServerType::ASIO:
		client = std::make_shared< ClientAsio >( addr, connection_timeout_ms, inference_timeout_ms );
		break;
	case ServerType::HTTP:
		client = std::make_shared< ClientHttp >( addr, connection_timeout_ms, inference_timeout_ms );
		break;
	default:
		assert( 0 );
	}
	return client;
}
