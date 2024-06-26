//////////////////////////////////////////////////////////////////////
/// \file dg_core_client.cpp
/// \brief DG Core client command line utility
///
/// Copyright 2021 DeGirum Corporation
///
/// This file contains implementation of AI client command line utility.
/// This utility is used to communicate with DeGirum AI Server to 
/// perform inference tasks
///
/// This is fully functional utility with flexible command line options,
/// which allow performing the following tasks:
///
/// 1. Print the list of all AI models available on given AI server
///    Usage: dg_core_client --ip {server address} --list
/// 2. Run the AI inference of the given model on given AI server and on given list of input frames
///    Usage: dg_core_client --ip {server address} --model {model name} --out {result file} {frame file 1} .. {frame file N}
/// 3. Send shutdown packet to AI server (works only for servers on local loopback address)
///    Usage: dg_core_client --ip {server address} --shutdown
///

#include <iostream>
#include <fstream>
#include "DglibInterface/dg_model_api.h"
#include "client/dg_client.h"
#include "Utilities/dg_cmdline_parser.h"
#include "Utilities/dg_file_utilities.h"

// Command line arguments
#define CMD_IPADDR		"ip"		//!< server IP address
#define CMD_MODEL		"model"		//!< name of ML model to run
#define CMD_OUT			"out"		//!< name of output file
#define CMD_SHUTDOWN	"shutdown"	//!< shutdown server
#define CMD_LIST		"list"		//!< list available models

/// Main entry point
int main( int argc, char **argv )
{
	// parse command line
	DG::InputParser cmd_args( argc, argv );

	//
	// handle --help command
	//
	if( cmd_args.cmdOptionExists( "help" ) || cmd_args.cmdOptionExists( "h" ) )
	{
		std::cout <<
			"\nPerform inference tasks on remote DG Core TCP server\n\n"
			"Parameters:\n"
			"  -" CMD_IPADDR " <IP address:port> - IP address of the server to work with (default 127.0.0.1)\n"
			"  -" CMD_MODEL " <model name> - name of ML model from model zoo to run\n"
			"  -" CMD_OUT " <output file> - name of output file to save results (default - print to console)\n"
			"  -" CMD_LIST " - print list of available models\n"
			"  -" CMD_SHUTDOWN " - shutdown server\n"
			"  <files> - space-separated list of files to run inference on\n\n";
		return 0;
	}

	try
	{
		// extract command line arguments
		const std::string server_ip = cmd_args.getCmdOption( CMD_IPADDR, "127.0.0.1" );
		const std::string model_name = cmd_args.getCmdOption( CMD_MODEL, "" );
		const std::string out_file = cmd_args.getCmdOption( CMD_OUT, "" );
		const std::vector< std::string > files = cmd_args.getNonOptions();
		const bool do_shutdown = cmd_args.cmdOptionExists( CMD_SHUTDOWN );
		const bool do_list = cmd_args.cmdOptionExists( CMD_LIST );
		
		//
		// handle --shutdown command
		//
		if( do_shutdown )
		{
			DG::shutdown( server_ip );
			std::cout << "Shutdown " << server_ip << "\n";
			return 0;
		}

		//
		// handle --list command:
		// get list of models in model zoo
		//
		std::vector< DG::ModelInfo > modelzoo_list;
		DG::modelzooListGet( server_ip, modelzoo_list );

		if( do_list )
		{
			std::cout << "\nAvailable models:\n\n";
			for( auto m : modelzoo_list )
				std::cout << m.name << "\n";
			return 0;
		}

		// validate parameters
		if( model_name == "" )
			throw std::runtime_error( "Model name is not specified" );

		if( files.size() == 0 )
			throw std::runtime_error( "No input files specified" );

		// find model in the model zoo by model name substring
		auto model_id = DG::modelFind( server_ip, { model_name } );
		if( model_id.name.empty() )
			throw std::runtime_error( "Model '" + model_name + "' is not found in model zoo" );

		//
		// handle 'run inference' command
		//
		std::cout <<
			"\n\nRunning inference\n"
			"  Server: " << server_ip << "\n"
			"  Model: " << model_id.name << "\n";

		// create AI model instance
		DG::AIModel model( server_ip, model_id.name );

		// iterate over all input files
		for( size_t fi = 0; fi < files.size(); fi++ )
		{
			std::cout << "File: " << files[ fi ] << "...";

			// send frame for inference
			DG::json resp;
			std::vector< std::vector< char > > frame = { DG::FileHelper::file2vector< char >( files[ fi ] ) };
			model.predict( frame, resp );

			// check for errors
			const std::string err = DG::errorCheck( resp );
			if( !err.empty() )
				std::cout << "Error detected during inference:\n" << err << "\n";

			// save inference result
			const std::string resp_str = resp.dump();
			if( out_file == "" )
				std::cout << files[ fi ] << "\n\n" << resp_str << "\n";
			else
				std::ofstream( out_file, std::ios_base::out | (fi == 0 ? std::ios_base::trunc : std::ios_base::app) ).write(
					resp_str.c_str(), resp_str.size() );
		}

	}
	catch( std::exception &e )
	{
		std::cout << e.what() << "\n";
		return -1;
	}
	catch( ... )
	{
		std::cout << "Unhandled exception\n";
		return -1;
	}

	return 0;
}
