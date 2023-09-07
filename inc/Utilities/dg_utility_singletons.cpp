//////////////////////////////////////////////////////////////////////
/// \file  dg_utility_singletons.cpp
/// \brief dg_utilities library singletons
///
/// Copyright 2023 DeGirum Corporation
///
/// This file contains all singletons used in dg_utilities library
///

#include "DGErrorHandling.h"
#include "DGLog.h"
#include "dg_tracing_facility.h"

//
// Get global tracing facility object and optionally substitute it with another object provided as a parameter
// [in] substitute - pointer to an object to substitute global tracing facility object with
// Special values:
//  - nullptr - do not substitute
//  - DG_TRC_ORIGINAL_FACILITY - reset to original
// return reference to current (either original or substituted) tracing facility object
//
extern "C" DG_EXPORT_API DGTrace::TracingFacility *DGTrace::manageTracingFacility( TracingFacility *substitute )
{
	static TracingFacility instance;
	static TracingFacility *instance_substitute = nullptr;

	// return what it is set right now
	TracingFacility *ret = instance_substitute == nullptr ? &instance : instance_substitute;

	// set substitute if requested
	if( substitute != nullptr )
	{
		if( substitute == DG_TRC_ORIGINAL_FACILITY )
			instance_substitute = nullptr;
		else
			instance_substitute = substitute;
	}
	return ret;
}

//
// Get system logger singleton
//
DG::FileLogger &DG::FileLogger::get_FileLogger()
{
	static FileLogger instance;
	return instance;
}

//
// Get global collection of registered errors
//
DG::ErrorHandling::ErrorCollection &DG::ErrorHandling::get_error_collection()
{
	static ErrorCollection instance;
	return instance;
}
