//////////////////////////////////////////////////////////////////////
/// \file  dg_model_parameters.h
/// \brief DG centralized handling of JSON model parameters
///
/// This file implements classes for centralized handling of JSON model parameters.
/// Each AI model in DG framework is accompanied with JSON configuration file,
/// which defines all model configuration parameters.
/// Class list:
///    ModelParamsReadAccess - read-only model parameters accessor
///    ModelParamsWriteAccess - read/write model parameters accessor
///    ModelParams - model parameters collection with access type defined by the template parameter
///

// Copyright DeGirum Corporation 2024
// All rights reserved

//
// ****  ATTENTION!!! ****
//
// This file is used to build client documentation using Doxygen.
// Please clearly describe all public entities declared in this file,
// using full and grammatically correct sentences, avoiding acronyms,
// not omitting articles.
//

#ifndef DG_MODEL_PARAMETERS_H_
#define DG_MODEL_PARAMETERS_H_

#include <string>
#include <type_traits>
#include "dg_json_helpers.h"

namespace DG
{

/// The most current version of JSON model configuration, supported by this version of software.
/// Increment it each time you change any parameter definition or add/remove any parameter
const int MODEL_PARAMS_CURRENT_VERSION = 11;

/// The minimum compatible version of JSON model configuration, still supported by this version of software.
/// Increase it when the software is modified such a way, that it stops supporting older JSON model configuration
/// versions.
const int MODEL_PARAMS_MIN_COMPATIBLE_VERSION = 1;

/// Model parameters section descriptor
struct ModelParamsSection
{
	const char *label;  //!< section name string
	bool is_scalar;     //!< scalar vs vector flag: vector sections contain more than one element
};

/// JSON file top-level sections
constexpr ModelParamsSection SECT_TOP = { "", true };                      //!< top-level section
constexpr ModelParamsSection SECT_DEVICE = { "DEVICE", true };             //!< device parameters section
constexpr ModelParamsSection SECT_PRE_PROCESS = { "PRE_PROCESS", false };  //!< pre-processing parameters section
constexpr ModelParamsSection SECT_MODEL_PARAMETERS = { "MODEL_PARAMETERS", true };  //!< model parameters section
constexpr ModelParamsSection SECT_POST_PROCESS = { "POST_PROCESS", true };  //!< post-processing parameters section
constexpr ModelParamsSection SECT_INTERNAL = { "INTERNAL", true };          //!< internal parameters section

using vec_dbl = std::vector< double >;       //!< vector of doubles
using vec_flt = std::vector< float >;        //!< vector of floats
using vec_int = std::vector< int >;          //!< vector of integers
using vec_str = std::vector< std::string >;  //!< vector of strings
using vec_sz = std::vector< size_t >;        //!< vector of size_t
using map_str_vec = std::map< std::string, std::vector< int32_t > >;
using vec3d_int = std::vector< std::vector< std::vector< int > > >;  //!< 3D vector of integers

// List of all configuration parameters
#include "dg_model_parameters.inc"

/// \brief ModelParamsReadAccess is read-only accessor to model parameters.
///
/// This class provides programmatic type-safe read access to model parameters defined in JSON model configuration.
/// It keeps non-owning const reference to underlying JSON array.
/// For each model parameter it provides getter method, which name matches the parameter name as it appears in JSON
/// array.
class ModelParamsReadAccess
{
public:
	/// Constructor. Attaches model parameter read-only accessor to JSON array
	/// \param[in] config - JSON array with model configuration
	ModelParamsReadAccess( const json &config ) : m_cfg_ro( config )
	{}

	//! @cond Doxygen_Suppress

/// Getters with defaults: method name is parameter name
/// \param[in] value - default value to return if parameter is missing (applies only for non-mandatory parameters)
/// \param[in] idx - array index inside section object
#define _( name, section, c_type, default_val, mandatory, runtime, visible, fallback )                               \
	template< typename T = c_type >                                                                                  \
	T name( const c_type &value = default_val, size_t idx = 0 ) const                                                \
	{                                                                                                                \
		return T( paramGet<                                                                                          \
				  c_type >( section.label, #name, mandatory, value, idx, &ModelParamsReadAccess::fallback##_get ) ); \
	}
	DG_MODEL_PARAMS_LIST
#undef _

/// Getters without defaults: method name is parameter name with "_get" suffix
/// \param[in] idx - array index inside section object
#define _( name, section, c_type, default_val, mandatory, runtime, visible, fallback ) \
	c_type name##_get( size_t idx = 0 ) const                                          \
	{                                                                                  \
		return paramGet< c_type >(                                                     \
			section.label,                                                             \
			#name,                                                                     \
			mandatory,                                                                 \
			default_val,                                                               \
			idx,                                                                       \
			&ModelParamsReadAccess::fallback##_get );                                  \
	}
	DG_MODEL_PARAMS_LIST
#undef _

/// Checking for existence: method name is parameter name with "_exist" suffix
/// \param[in] idx - array index inside section object
#define _( name, section, c_type, default_val, mandatory, runtime, visible, fallback )            \
	bool name##_exist( size_t idx = 0 ) const                                                     \
	{                                                                                             \
		return paramExist( section.label, #name, idx, &ModelParamsReadAccess::fallback##_exist ); \
	}
	DG_MODEL_PARAMS_LIST
#undef _

/// Get parent section name
#define _( name, section, c_type, default_val, mandatory, runtime, visible, fallback ) \
	std::string name##_section() const                                                 \
	{                                                                                  \
		return section.label;                                                          \
	}
	DG_MODEL_PARAMS_LIST
#undef _

	//! @endcond

	/// Access to underlying JSON array
	/// \return constant non-owning reference to JSON array with model configuration
	const json &jsonGet() const
	{
		return m_cfg_ro;
	}

	/// Get size of underlying JSON sub-array for particular section
	/// \param[in] section - section name to access (one of SECT_xxx constants)
	/// \return section sub-array size in elements
	size_t sectionSizeGet( const std::string &section ) const
	{
		return section.empty() || !m_cfg_ro.contains( section ) ? 1 : m_cfg_ro[ section ].size();
	}

	/// Access to underlying JSON sub-array for particular section
	/// \param[in] section - section name to access (one of SECT_xxx constants)
	/// \param[in] idx - array index inside section object
	/// \return constant non-owning reference to JSON array with particular model section configuration
	const json &sectionGet( const std::string &section, size_t idx = 0 ) const
	{
		return section.empty() ? m_cfg_ro : m_cfg_ro[ section ][ idx ];
	}

	/// Compute the hash of the parameter values in a given section
	/// \param[in] section - section name to compute hash for
	/// \param[in] idx - array index inside section object
	/// \return std::hash for a given section
	size_t sectionHashGet( const std::string &section, size_t idx = 0 ) const
	{
		return std::hash< std::string >()( sectionGet( section, idx ).dump() );
	}

	/// Access to underlying JSON array
	/// \return constant non-owning reference to JSON array with model configuration
	operator const json &() const
	{
		return m_cfg_ro;
	}

	/// Access to underlying JSON array as string
	/// \return JSON text with model configuration
	operator std::string() const
	{
		return m_cfg_ro.dump();
	}

	/// Function to return the full JSON of model parameters
	/// This returns the model parameter JSON merged with a JSON
	/// of default values for all the parameters that aren't listed
	json fullJsonGet() const
	{
		json resultJson = m_cfg_ro;

#define _( name, section, c_type, default_val, mandatory, runtime, visible, fallback )                           \
	if( runtime )                                                                                                \
	{                                                                                                            \
		if( section.label[ 0 ] == 0 )                                                                            \
		{                                                                                                        \
			if( !resultJson.contains( #name ) )                                                                  \
				resultJson[ #name ] = name##_exist() ? name##_get() : default_val;                               \
		}                                                                                                        \
		else                                                                                                     \
		{                                                                                                        \
			if( !resultJson.contains( section.label ) )                                                          \
				resultJson[ section.label ] = json::array();                                                     \
																												 \
			if( resultJson[ section.label ].is_array() )                                                         \
			{                                                                                                    \
				for( size_t idx = 0; idx < resultJson[ section.label ].size(); idx++ )                           \
				{                                                                                                \
					auto &item = resultJson[ section.label ][ idx ];                                             \
					if( !item.contains( #name ) )                                                                \
						item[ #name ] = name##_exist( idx ) ? name##_get( idx ) : default_val;                   \
				}                                                                                                \
			}                                                                                                    \
			else if( resultJson[ section.label ].is_object() && !resultJson[ section.label ].contains( #name ) ) \
			{                                                                                                    \
				resultJson[ section.label ][ #name ] = name##_exist() ? name##_get() : default_val;              \
			}                                                                                                    \
		}                                                                                                        \
	}
		DG_MODEL_PARAMS_LIST
#undef _

		return resultJson;
	}

#if defined( FRAMEWORK_PATH ) && !defined( DG_CLIENT_BUILD )
	/// Check version from model configuration against current software version and minimum compatible software version
	/// \param[in] do_throw - when true, exception will be thrown in case of version mismatch,
	/// otherwise error message string will be returned
	/// \return error message string, if version mismatch is detected
	std::string versionCheck( bool do_throw ) const
	{
		const std::string model_name = ModelPath_exist() ? std::filesystem::path( ModelPath() ).stem().string()
														 : "unknown";
		const int config_version = ConfigVersion();
		std::string error_msg;

		if( config_version < MODEL_PARAMS_MIN_COMPATIBLE_VERSION )
		{
			error_msg = "The version " + std::to_string( config_version ) + " of '" + model_name +
				"' model parameter collection is TOO OLD and is not supported by the current version of DeGirum "
				"framework software. " +
				"The model version should be at least " + std::to_string( MODEL_PARAMS_MIN_COMPATIBLE_VERSION ) +
				". Please upgrade your model zoo";
		}
		else if( config_version > MODEL_PARAMS_CURRENT_VERSION )
		{
			error_msg = "The version " + std::to_string( config_version ) + " of '" + model_name +
				"' model parameter collection is NEWER than the current version " +
				std::to_string( MODEL_PARAMS_CURRENT_VERSION ) +
				" of DeGirum framework software. Please upgrade your installation of DeGirum framework software";
		}

		if( do_throw && !error_msg.empty() )
			DG_ERROR( error_msg, ErrNotSupportedVersion );
		else
			return error_msg;
	}
#endif

	/// Stub for empty getter fallback
	template< typename T >
	T None_get( size_t idx = 0 ) const
	{
		return T();
	}

	/// Stub for empty getter fallback
	template< typename T >
	T None( size_t idx = 0 ) const
	{
		return T();
	}

	/// Stub for empty existence checker fallback
	bool None_exist( size_t idx = 0 ) const
	{
		return false;
	}

protected:
	const json &m_cfg_ro;  //!< non-owning reference to JSON array with model configuration

	/// Get parameter from JSON array
	/// \tparam T - parameter type
	/// \param[in] section - top section name; if empty, key is taken from topmost level
	/// \param[in] key - key of key-value pair
	/// \param[in] is_mandatory - if parameter mandatory
	/// \param[in] default_value - value to return, if no such key is found
	/// \param[in] idx - array index inside section object
	/// \param[in] fallback - fallback parameter getter
	/// \return parameter value
	template< typename T >
	T paramGet(
		const char *section,
		const char *key,
		bool is_mandatory,
		const T &default_value,
		size_t idx,
		T ( ModelParamsReadAccess::*fallback )( size_t ) const ) const
	{
		if( paramExist( section, key, idx, &ModelParamsReadAccess::None_exist ) ||
			fallback == &ModelParamsReadAccess::None_get< T > )
		{
			if( is_mandatory )
				return jsonGetMandatoryValue< T >( m_cfg_ro, section, (int)idx, key );
			else
				return jsonGetOptionalValue< T >( m_cfg_ro, section, (int)idx, key, default_value );
		}
		else
			return ( ( *this ).*fallback )( idx );
	}

	/// Check parameter for existence
	/// \tparam T - parameter type
	/// \param[in] section - top section name; if empty, key is taken from topmost level
	/// \param[in] key - key of key-value pair
	/// \param[in] idx - array index inside section object
	/// \param[in] fallback - existence checker for fallback parameter
	/// \return parameter value
	bool paramExist(
		const char *section,
		const char *key,
		size_t idx,
		bool ( ModelParamsReadAccess::*fallback )( size_t ) const ) const
	{
		const bool exists = jsonKeyExist( m_cfg_ro, section, (int)idx, key );
		if( !exists && fallback != &ModelParamsReadAccess::None_exist )
			return ( ( *this ).*fallback )( idx );
		return exists;
	}

public:

	/// Helper method: get model input size
	size_t modelInputSizeGet() const
	{
		return sectionSizeGet( SECT_PRE_PROCESS.label );
	}

	/// Helper method: get model input shape
	/// \param[in] inp_idx - input index
	/// \param[in] expected_size - expected size of the shape vector, 0 if not checked
	/// \return model input shape vector
	std::vector< size_t > modelShapeGet( size_t inp_idx, size_t expected_size )
	{
		if( InputShape_exist( inp_idx ) && InputShape_get( inp_idx ).size() > 0 )
		{
			// InputShape has priority over InputN, InputH, InputW, InputC
			auto shape = InputShape_get( inp_idx );
			if( expected_size != 0 && shape.size() != expected_size )
				DG_ERROR(
					DG_FORMAT(
						"The input shape parameter InputShape for input #"
						<< inp_idx << " must have " << expected_size << " elements, while it has " << shape.size() ),
					ErrBadParameter );
			return shape;
		}
		else
		{
			if( expected_size != 0 )
			{
				// when expected_size is set, we use it to fill the shape vector
				// and put N/H/W/C values in the right places
				std::vector< size_t > shape( expected_size, 1 );
				if( InputN_get( inp_idx ) > 0 && expected_size >= 1 )
					shape[ 0 ] = InputN_get( inp_idx );
				if( InputH_get( inp_idx ) > 0 && expected_size >= 2 )
					shape[ 1 ] = InputH_get( inp_idx );
				if( InputW_get( inp_idx ) > 0 && expected_size >= 3 )
					shape[ 2 ] = InputW_get( inp_idx );
				if( InputC_get( inp_idx ) > 0 && expected_size >= 4 )
					shape[ 3 ] = InputC_get( inp_idx );
				return shape;
			}
			else
			{
				// when expected_size is not set, we fill the shape vector
				// with all the values that are defined
				std::vector< size_t > shape;
				if( InputN_exist( inp_idx ) && InputN_get( inp_idx ) > 0 )
					shape.push_back( (size_t)InputN_get( inp_idx ) );
				if( InputH_exist( inp_idx ) && InputH_get( inp_idx ) > 0 )
					shape.push_back( (size_t)InputH_get( inp_idx ) );
				if( InputW_exist( inp_idx ) && InputW_get( inp_idx ) > 0 )
					shape.push_back( (size_t)InputW_get( inp_idx ) );
				if( InputC_exist( inp_idx ) && InputC_get( inp_idx ) > 0 )
					shape.push_back( (size_t)InputC_get( inp_idx ) );
				return shape;
			}
		}
	}
};

/// \brief ModelParamsWriteAccess is read/write accessor to model parameters.
///
/// This class provides programmatic type-safe read and write access to model parameters defined in JSON model
/// configuration. It keeps non-owning non-const reference to underlying JSON array. For each model parameter it
/// provides both getter and setter methods. Getter method name matches the parameter name as it appears in JSON array.
/// Setter method name is constructed from the parameter name by appending _set suffix. For example, InputImgFmt_set()
class ModelParamsWriteAccess: public ModelParamsReadAccess
{
public:
	/// Constructor. Attaches model parameter read-write accessor to JSON array
	/// \param[in] config - JSON array with model configuration
	ModelParamsWriteAccess( json &config ) : ModelParamsReadAccess( config ), m_cfg_rw( config ), m_dirty( false )
	{}

	//! @cond Doxygen_Suppress

/// Setters: method name is parameter name with "_set" suffix
/// \param[in] value - parameter value to set
/// \param[in] idx - array index inside section object
#define _( name, section, c_type, default_val, mandatory, runtime, visible, fallback ) \
	ModelParamsWriteAccess &name##_set( const c_type &value, size_t idx = 0 )          \
	{                                                                                  \
		return paramSet< c_type >( section.label, #name, value, idx );                 \
	}
	DG_MODEL_PARAMS_LIST
#undef _

	//! @endcond

	/// Merge given model configuration JSON array with self
	/// Note: only runtime parameters will be merged
	/// \param[in] config - model configuration JSON array to be merged
	ModelParamsWriteAccess &merge( const json &config )
	{
		ModelParamsReadAccess patch( config );

		//! @cond Doxygen_Suppress
		//!
#define _( name, section, c_type, default_val, mandatory, runtime, visible, fallback )        \
	if( runtime && patch.name##_exist() )                                                     \
	{                                                                                         \
		if( section.label[ 0 ] == 0 || section.is_scalar )                                    \
			name##_set( patch.name() );                                                       \
		else                                                                                  \
		{                                                                                     \
			auto lbl = section.label;                                                         \
			const size_t sz = std::min( patch.sectionSizeGet( lbl ), sectionSizeGet( lbl ) ); \
			for( size_t idx = 0; idx < sz; idx++ )                                            \
				name##_set( patch.name##_get( idx ), idx );                                   \
		}                                                                                     \
	}
		DG_MODEL_PARAMS_LIST
#undef _
		//! @endcond
		return *this;
	}  // namespace DG

	/// Check if dirty: at least one of the parameters was changed
	bool is_dirty() const
	{
		return m_dirty;
	}

	/// Set dirty flag
	/// \param[in] state - dirty flag value to set
	void set_dirty( bool state )
	{
		m_dirty = state;
	}

protected:
	json &m_cfg_rw;  //!< non-owning reference to JSON array with model configuration
	bool m_dirty;    //!< 'some parameter was changed' flag

	/// Set parameter to JSON array
	/// \tparam T - parameter type
	/// \param[in] section - top section name; if empty, key is placed in topmost level
	/// \param[in] key - key of key-value pair
	/// \param[in] value - value to set
	/// \param[in] idx - array index inside section object
	template< typename T >
	ModelParamsWriteAccess &paramSet( const char *section, const char *key, const T &value, size_t idx )
	{
		auto &param = ( section == nullptr || *section == '\0' ) ? m_cfg_rw : m_cfg_rw[ section ][ idx ];
		if( !param.is_object() || !param.contains( key ) )
		{
			param[ key ] = value;
			set_dirty( true );
		}
		else
		{
			typename std::remove_cv_t< std::remove_reference_t< decltype( value ) > > val;
			auto &rec = param[ key ];
			rec.get_to( val );
			if( !rec.is_primitive() || rec.is_number_float() || val != value )
			{
				rec = value;
				set_dirty( true );
			}
		}
		return *this;
	}
};

/// \brief ModelParams is model parameters collection with user-defined access rights.
///
/// This class provides programmatic type-safe access to model parameters defined in JSON model configuration
/// while owning that JSON array.
/// Access (read or write) is defined by the Base template parameter, which is used as the base class.
/// \tparam Base - ModelParamsReadAccess-derived base class: it can be ModelParamsReadAccess or ModelParamsWriteAccess
template<
	class Base = ModelParamsReadAccess,
	std::enable_if_t< std::is_base_of_v< ModelParamsReadAccess, Base >, bool > = false >
class ModelParams: public Base
{
public:
	/// Constructor. Creates model parameter collection by parsing JSON text in character array
	/// \param[in] json_text - JSON text with model parameters. It will be parsed and stored in internal JSON array.
	ModelParams( const char *json_text = "{}" ) : Base( m_cfg )
	{
		m_cfg = DG_JSON_PARSE( json_text );
		DG_ERROR_TRUE( m_cfg.is_object() ) << "ModelParams must be initialized with string containing json object";
	}

	/// Constructor. Creates model parameter collection by parsing JSON text in std::string
	/// \param[in] json_text - JSON text with model parameters. It will be parsed and stored in internal JSON array.
	explicit ModelParams( const std::string &json_text ) : Base( m_cfg )
	{
		m_cfg = DG_JSON_PARSE( json_text );
		DG_ERROR_TRUE( m_cfg.is_object() ) << "ModelParams must be initialized with string containing json object";
	}

	/// Constructor. Creates model parameter collection from given JSON array
	/// \param[in] json_cfg - JSON array with model parameters. It will be copied into internal JSON array.
	explicit ModelParams( const json &json_cfg ) : m_cfg( json_cfg ), Base( m_cfg )
	{}

	/// Constructor with move semantic. Creates model parameter collection from given JSON array
	/// \param[in] json_cfg - JSON array with model parameters. It will be moved into internal JSON array.
	explicit ModelParams( json &&json_cfg ) : m_cfg( std::move( json_cfg ) ), Base( m_cfg )
	{}

	/// Copy constructor
	ModelParams( ModelParams const &rhs ) : Base( m_cfg ), m_cfg( rhs.m_cfg )
	{}

	/// Copy assignment
	ModelParams &operator=( ModelParams const &rhs )
	{
		m_cfg = rhs.m_cfg;
		return *this;
	}

	/// Move constructor
	ModelParams( ModelParams &&rhs ) : Base( m_cfg ), m_cfg( std::move( rhs.m_cfg ) )
	{}

	/// Move assignment
	ModelParams &operator=( ModelParams &&rhs )
	{
		m_cfg = std::move( rhs.m_cfg );
		return *this;
	}

private:
	json m_cfg;  //!< JSON array with model configuration
};

/// ModelParamsWriter is ModelParams template instantiation with write access
using ModelParamsWriter = ModelParams< ModelParamsWriteAccess >;

}  // namespace DG

#endif  // DG_MODEL_PARAMETERS_H_
