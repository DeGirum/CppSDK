///////////////////////////////////////////////////////////////////////////////
/// \file dg_client_http.h
/// \brief Class, which implements client-side functionality of
///  DG client-server system based on HTTP+WebSockets protocol
///
/// Copyright DeGirum Corporation 2023
///
/// This file contains declaration of DG::Client class:
/// which handles HTTP+WebSockets protocol of DG client-server system
///

#ifndef DG_CLIENT_HTTP_H_
#define DG_CLIENT_HTTP_H_

#include <mutex>
#include <queue>
#include "dg_client.h"
#include "dg_socket.h"
#include "httplib.h"

namespace DG
{

/// Client-side protocol handler of DG client-server system
/// based on HTTP+WebSockets protocol
class ClientHttp: public Client
{
public:
	/// Constructor. Sets up active server using address.
	/// \param[in] server_address - server address
	/// \param[in] connection_timeout_ms - connection timeout in milliseconds
	/// \param[in] inference_timeout_ms - AI server inference timeout in milliseconds
	ClientHttp(
		const ServerAddress &server_address,
		size_t connection_timeout_ms = DEFAULT_CONNECTION_TIMEOUT_MS,
		size_t inference_timeout_ms = DEFAULT_INFERENCE_TIMEOUT_MS );

	ClientHttp( const Client &s ) = delete;
	ClientHttp &operator=( const ClientHttp & ) = delete;

	/// Destructor
	~ClientHttp();

	/// Get list of models in all model zoos of all active servers
	/// \param[out] modelzoo_list - array of models in model zoos
	void modelzooListGet( std::vector< DG::ModelInfo > &modelzoo_list ) override;

	/// Return host system information dictionary
	json systemInfo() override;

	/// AI server tracing facility management
	/// \param[in] req - management request
	/// \return results of management request completion (request-specific)
	json traceManage( const json &req ) override;

	/// AI server model zoo management
	/// \param[in] req - management request
	/// \return results of management request completion (request-specific)
	json modelZooManage( const json &req ) override;

	/// Ping server with an instantaneous command
	/// \param[in] sleep_ms - optional server-side sleep time in milliseconds
	/// \param[in] ignore_errors - when true, ignore all errors and return false, otherwise throw exception
	/// \return true if no error occurred during the ping
	bool ping( double sleep_ms = 0, bool ignore_errors = true ) override;

	/// Creates and opens socket for stream of frames to be used by subsequent predict() calls
	/// \param[in] model_name - model name, which defines op destination
	/// \param[in] frame_queue_depth - the depth of internal frame queue
	/// \param[in] additional_model_parameters - additional model parameters in JSON format compatible with DG JSON
	/// model configuration
	void openStream(
		const std::string &model_name,
		size_t frame_queue_depth,
		const json &additional_model_parameters = {} ) override;

	/// Send shutdown request to AI server
	void shutdown() override;

	/// Get model label dictionary
	/// \param[in] model_name specifies the AI model.
	/// \return JSON object containing model label dictionary
	json labelDictionary( const std::string &model_name ) override;

	//
	// Synchronous prediction API
	//

	/// Run prediction on given data frame. Stream should be opened by openStream()
	/// \param[in] data - array containing frame data
	/// \param[out] output - prediction result (JSON array)
	void predict( std::vector< std::vector< char > > &data, json &output ) override;

	//
	// Asynchronous prediction API
	//

	/// Install prediction results observation callback
	/// \param[in] callback - user callback, which will be called as soon as prediction result is ready when using
	/// dataSend() methods
	void resultObserve( callback_t callback ) override;

	/// Send given data frame for prediction. Prerequisites:
	///   stream should be opened by openStream();
	///   user callback to receive prediction results should be installed by resultObserve().
	/// On the very first frame the method launches result receiving thread. This thread asynchronously
	/// retrieves prediction results and for each result calls user callback installed by resultObserve().
	/// To terminate this thread dataEnd() should be called when no more data frames are expected.
	/// \param[in] data - array containing frame data
	/// \param[in] frame_info - optional frame information string to be passed to the callback along the frame result
	void dataSend( const std::vector< std::vector< char > > &data, const std::string &frame_info = "" ) override;

	/// Finalize the sequence of data frames. Should be called when no more data frames are
	/// expected to terminate result receiving thread started by dataSend().
	/// NOTE: will wait until all outstanding results are received
	void dataEnd() override;

	/// Get # of outstanding inference results scheduled so far
	int outstandingResultsCountGet() override
	{
		std::lock_guard< std::mutex > lock( m_state );
		return (int)m_state.m_frame_info_queue.size();
	}

	/// If ever during consecutive calls to predict() methods server reported run-time error, then
	/// this method will return error message string, otherwise it returns empty string.
	std::string lastError() override
	{
		std::lock_guard< std::mutex > lock( m_state );
		return m_state.m_last_error;
	}

private:
	// setup parameters:
	DG::ServerAddress m_server_address;  //!< address of active server
	size_t m_frame_queue_depth;          //!< depth of frame queue
	size_t m_connection_timeout_ms;      //!< socket connection timeout, in milliseconds
	size_t m_inference_timeout_ms;       //!< AI server inference timeout, in milliseconds
	callback_t m_async_result_callback;  //!< asynchronous inference result callback

	// internal objects:
	httplib::Client m_http_client;       //!< HTTP client
	class WebSocketClient *m_ws_client;  //!< WebSocket client

	/// Asynchronous prediction runtime state structure
	struct State: public std::mutex
	{
		std::queue< std::string > m_frame_info_queue;  //!< frame info queue
		std::string m_last_error;                      //!< last prediction error (or empty)
	} m_state;                                         //!< runtime state object

	std::condition_variable m_waiter;  //!< condition variable for result receiving thread synchronization

	/// Check HTTP result for errors
	/// \param[in] result - HTTP result to check
	/// \param[in] path - URL path string
	void checkHttpResult( const httplib::Result &result, const std::string &path );

	/// HTTP request types
	enum REQ
	{
		POST,  //!< POST
		GET    //!< GET
	};

	/// Post HTTP request and check HTTP result for errors
	/// \param[in] path - path string
	/// \param[in] body - optional body string
	/// \param[in] content_type - optional content type string
	template< REQ req >
	httplib::Result
	httpRequest( const std::string &path, const std::string &body = "", const std::string &content_type = "" )
	{
		httplib::Result result;
		switch( req )
		{
		case POST:
			result = m_http_client.Post( path, body, content_type );
			break;
		case GET:
		default:
			result = m_http_client.Get( path );
			break;
		}
		checkHttpResult( result, path );
		return result;
	}

	/// Wait until the number of outstanding frames becomes or equal to the given value or until error is detected
	/// \param[in] outstanding_frames - number of outstanding frames to wait for
	/// \param[in] lock - acquired lock to use for waiting
	/// Throws error on timeout
	// \return true if no error occurred during the wait
	bool waitFor( size_t outstanding_frames, std::unique_lock< std::mutex > &lock );

	/// Close stream opened by openStream()
	void closeStream();
};
}  // namespace DG

#endif  // DG_CLIENT_HTTP_H_
