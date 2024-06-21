//////////////////////////////////////////////////////////////////////////////
/// \file dg_device_types.h
/// \brief N2X device types
///
/// Copyright 2024 DeGirum Corporation
///
/// This file contains declaration of N2X device types enumerator and related functions
///

#ifndef DG_DEVICE_TYPES_H_
#define DG_DEVICE_TYPES_H_

#include <string>

namespace DG
{
	/// List of supported N2X device types
	// _( id, label, index, description )
	#define DG_DEVICE_TYPE_LIST \
		_( CPU,			"CPU",			0,	"processing on computer" ) \
		_( SIMULATOR,	"SIMULATOR",	1,	"DG Verilog simulator of Orca" ) \
		_( FPGA,		"FPGA",			2,	"DG FPGA simulator (aka 'small' FPGA)" ) \
		_( BIG_FPGA,	"BIG_FPGA",		3,	"DG advanced FPGA simulator (aka 'big' FPGA)" ) \
		_( ORCA,		"ORCA",			4,	"DG Orca-based accelerator" ) \
		_( DEV_DUMMY,	"DUMMY",		5,	"dummy device (for unit tests)" ) \
		_( BIG_FPGA_1P1,"BIG_FPGA_1P1",	11,	"DG advanced FPGA simulator for ORCA1.1 (aka 'big' FPGA)" ) \
		_( ORCA_1P1,	"ORCA1",		12,	"DG Orca 1.1 based accelerator" )


enum DEVICE_TYPES : int
{
	DEV_TYPE_MIN = 0,  //!< minimum type index
#define _( id, label, index, description ) id = index,
	DG_DEVICE_TYPE_LIST
#undef _
		DEV_TYPE_CNT,  //!< # of different types
};

/// Deduce device type by device name
/// \param[in] dev_name - device name string
/// \result - device type ID
static inline DEVICE_TYPES deviceTypeFromName( const std::string &dev_name )
{
#define _( id, label, index, description ) \
	if( dev_name == label )                \
		return id;
	DG_DEVICE_TYPE_LIST
#undef _
	return DEV_TYPE_CNT;
}

/// Get device name by device type
/// \param[in] dev_id - device type ID
/// \result - device name
static inline std::string deviceNameFromType( DEVICE_TYPES dev_id )
{
	switch( dev_id )
	{
#define _( id, label, index, description ) \
	case id:                               \
		return label;
		DG_DEVICE_TYPE_LIST
#undef _
	default:
		return "";
	}
}

}  // namespace DG

#endif  // DG_DEVICE_TYPES_H_
