///////////////////////////////////////////////////////////////////////////////
/// \file dg_client.h
/// \brief Base interface for client-side functionality of
///  DG client-server system
///
/// Copyright DeGirum Corporation 2023
///
/// This file contains declaration of DG::Client class:
/// base interface which handles protocol of DG client-server system
///

#ifndef DG_CLIENT_H_
#define DG_CLIENT_H_

#include "Utilities/dg_client_structs.h"
#include "Utilities/dg_json_helpers.h"

namespace DG
{

/// Client-side protocol handler of DG client-server system.
/// Base interface and class factory.
class Client
{
public:
	/// Alias for shared pointer to Client object
	using ClientPtr = std::shared_ptr< Client >;

	/// Short alias for nlohmann::json
	using json = nlohmann::json;

	/// Class factory. Creates client object based on selected protocol.
	/// \param[in] server_address - server address in a format "[http://]host:port"
	/// \param[in] connection_timeout_ms - connection timeout in milliseconds
	/// \param[in] inference_timeout_ms - AI server inference timeout in milliseconds
	/// \return - pointer to client object
	static ClientPtr create(
		const std::string &server_address,
		size_t connection_timeout_ms = DEFAULT_CONNECTION_TIMEOUT_MS,
		size_t inference_timeout_ms = DEFAULT_INFERENCE_TIMEOUT_MS );

	/// Destructor
	virtual ~Client()
	{}

	/// Get list of models in all model zoos of all active servers
	/// \param[out] modelzoo_list - array of models in model zoos
	virtual void modelzooListGet( std::vector< DG::ModelInfo > &modelzoo_list ) = 0;

	/// Return host system information dictionary
	virtual json systemInfo() = 0;

	/// AI server tracing facility management
	/// \param[in] req - management request
	/// \return results of management request completion (request-specific)
	virtual json traceManage( const json &req ) = 0;

	/// AI server model zoo management
	/// \param[in] req - management request
	/// \return results of management request completion (request-specific)
	virtual json modelZooManage( const json &req ) = 0;

	/// Ping server with an instantaneous command
	/// \param[in] sleep_ms - optional server-side sleep time in milliseconds
	/// \param[in] ignore_errors - when true, ignore all errors and return false, otherwise throw exception
	/// \return true if no error occurred during the ping
	virtual bool ping( double sleep_ms = 0, bool ignore_errors = true ) = 0;

	/// 'stream' op handler: creates and opens socket for stream of frames to be used by subsequent predict() calls
	/// \param[in] model_name - model name, which defines op destination
	/// \param[in] frame_queue_depth - the depth of internal frame queue
	/// \param[in] additional_model_parameters - additional model parameters in JSON format compatible with DG JSON
	/// model configuration
	virtual void openStream(
		const std::string &model_name,
		size_t frame_queue_depth,
		const json &additional_model_parameters = {} ) = 0;

	/// Send shutdown request to AI server
	virtual void shutdown() = 0;

	/// Get model label dictionary
	/// \param[in] model_name specifies the AI model.
	/// \return JSON object containing model label dictionary
	virtual json labelDictionary( const std::string &model_name ) = 0;

	//
	// Synchronous prediction API
	//

	/// Run prediction on given data frame. Stream should be opened by openStream()
	/// \param[in] data - array containing frame data
	/// \param[out] output - prediction result (JSON array)
	virtual void predict( std::vector< std::vector< char > > &data, json &output ) = 0;

	//
	// Asynchronous prediction API
	//

	/// User callback type. The callback is called as soon as prediction result is ready.
	/// Consecutive prediction result (JSON array) is passed as the first argument.
	/// Corresponding frame info string (provided to dataSend() call) is passed as the second argument.
	using callback_t = std::function< void( const json &, const std::string & ) >;

	/// Install prediction results observation callback
	/// \param[in] callback - user callback, which will be called as soon as prediction result is ready when using
	/// dataSend() methods
	virtual void resultObserve( callback_t callback ) = 0;

	/// Send given data frame for prediction. Prerequisites:
	///   stream should be opened by openStream();
	///   user callback to receive prediction results should be installed by resultObserve().
	/// On the very first frame the method launches result receiving thread. This thread asynchronously
	/// retrieves prediction results and for each result calls user callback installed by resultObserve().
	/// To terminate this thread dataEnd() should be called when no more data frames are expected.
	/// \param[in] data - array containing frame data
	/// \param[in] frame_info - optional frame information string to be passed to the callback along the frame result
	virtual void dataSend( const std::vector< std::vector< char > > &data, const std::string &frame_info = "" ) = 0;

	/// Finalize the sequence of data frames. Should be called when no more data frames are
	/// expected to terminate result receiving thread started by dataSend().
	/// NOTE: will wait until all outstanding results are received
	virtual void dataEnd() = 0;

	/// Get # of outstanding inference results scheduled so far
	virtual int outstandingResultsCountGet() = 0;

	/// If ever during consecutive calls to predict() methods server reported run-time error, then
	/// this method will return error message string, otherwise it returns empty string.
	virtual std::string lastError() = 0;
};
}  // namespace DG

#endif  // DG_CLIENT_H_
