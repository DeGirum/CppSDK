//////////////////////////////////////////////////////////////////////
/// \file  dg_file_utilities.h
/// \brief DG file handling utility functions
///
/// Copyright 2025 DeGirum Corporation
///
/// This file contains implementation of various helper functions
/// for file handling
///

#ifndef DG_FILE_UTILITIES_H_
#define DG_FILE_UTILITIES_H_

#ifdef _MSC_VER
	#include <stdlib.h>
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#include <psapi.h>
#else
	#include <dlfcn.h>
	#include <pwd.h>
	#include <string.h>
	#include <sys/file.h>
	#include <sys/types.h>
	#include <unistd.h>
	#ifdef __APPLE__
		#include <libgen.h>
		#include <mach-o/dyld.h>
		#include <mach/mach.h>
	#else
extern char *program_invocation_short_name;
extern char *program_invocation_name;
	#endif
#endif

#include <sys/stat.h>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <random>
#include <regex>
#include <string>
#include <thread>
#include <vector>

namespace DG
{
class FileHelper
{
public:
	/// Load text file to string buffer
	/// \param[in] path - file path
	/// \param[in] is_binary - treat file as binary
	/// \return string with file contents
	inline static std::string file2string( const std::string &path, bool is_binary = false );

	/// Save string buffer to text file
	/// \param[in] path - file path
	/// \param[in] str - string buffer to save
	inline static void string2file( const std::string &path, const std::string &str );

	/// Load binary file to vector buffer
	/// \param[in] path - file path
	/// \return vector with file contents
	template< typename T >
	static std::vector< T > file2vector( const std::string &path );

	/// Save vector buffer to binary file
	/// \param[in] path - file path
	/// \param[in] buf - vector buffer to save
	template< typename T >
	static void vector2file( const std::string &path, const std::vector< T > &buf );

	// Save stringview buffer to binary file
	template< typename CharT, typename Traits >
	static void stringview2file( const std::string &path, std::basic_string_view< CharT, Traits > buf );

	/// Split file path into directory, file name, and extension
	/// \param[in] fullpath - full file path
	/// \param[out] path_ret - directory part of full path including trailing slash (optional, can be null)
	/// \param[out] name_ret - file name (optional, can be null)
	/// \param[out] ext_ret - extension w/o leading dot or "" if no dot found (optional, can be null)
	static void
	path_split( const std::string &fullpath, std::string *path_ret, std::string *name_ret, std::string *ext_ret )
	{
		std::filesystem::path path( fullpath );

		if( path_ret != nullptr )
		{
			*path_ret = path.parent_path().string();
			if( path_ret->length() > 0 && path_ret->back() != '/' )
				*path_ret += '/';
		}

		if( name_ret != nullptr )
			*name_ret = path.stem().string();

		if( ext_ret != nullptr )
		{
			*ext_ret = path.extension().string();
			if( ext_ret->length() > 0 && ext_ret->front() == '.' )
				*ext_ret = ext_ret->substr( 1 );
		}
	}

	/// Touch file, updating its last write time to current time
	/// \param[in] fname - file name to touch
	/// \return true if file was touched, false if file does not exist
	static bool touch( const std::string &fname )
	{
		if( std::filesystem::exists( fname ) )
		{
			std::filesystem::file_time_type file_time_now = decltype( file_time_now )::clock::now();
			std::filesystem::last_write_time( fname, file_time_now );
			return true;
		}
		return false;
	}

	/// Check if given file exist
	/// \param[in] fname - file name to check
	/// \return true if file exist
	static bool fexist( const std::string &fname )
	{
		return std::filesystem::exists( fname );
	}

	/// Check if given directory exist
	/// \param[in] dir_name - directory name to check
	/// \return true if directory exist
	static bool dir_exist( const std::string &dir_name )
	{
		return std::filesystem::exists( dir_name );
	}

	/// Get file size
	/// \param[in] fname - file name
	/// \return file size in bytes or -1 if file does not exist
	static size_t fsize( const std::string &fname )
	{
		if( std::filesystem::exists( fname ) )
			return std::filesystem::file_size( fname );
		return -1;
	}

	/// Return the size in bytes of directory contents
	/// \param[in] directory - directory path
	/// \return size of all files located inside directory
	static size_t dir_size( const std::string &directory )
	{
		size_t size{ 0 };
		for( const auto &entry : std::filesystem::recursive_directory_iterator(
				 directory,
				 std::filesystem::directory_options::skip_permission_denied ) )
		{
			if( entry.is_regular_file() && !entry.is_symlink() )
				size += entry.file_size();
		}
		return size;
	}

	/// Check if given path is absolute path
	/// \return true if it is absolute
	static bool is_abs_path( const std::string &path )
	{
		if( path.empty() )
			return false;

#ifdef _MSC_VER
		return path.find( ':' ) != std::string::npos || path[ 0 ] == '/' || path[ 0 ] == '\\';
#else
		return path[ 0 ] == '/';
#endif
	}

	/// Make path with trailing slash
	/// \param[in] path - input path
	/// \return path with trailing slash
	static std::string path_with_slash( const std::string &path )
	{
		if( path.empty() )
			return "";

#ifdef _MSC_VER
		const bool path_ends_with_slash = path.back() == '/' || path.back() == '\\';
#else
		const bool path_ends_with_slash = path.back() == '/';
#endif
		if( !path_ends_with_slash )
			return std::filesystem::path( path + '/' ).generic_string();
		return std::filesystem::path( path ).generic_string();
	}

	/// Prepend given input path with given root path, if the input path is relative,
	/// otherwise just return it. In both cases returned path is affixed with trailing slash.
	/// \param[in] path - input path
	/// \param[in] root_path - root path
	/// \return path if path is absolute, otherwise return root_path + path
	static std::string abs_path( const std::string &path, const std::string &root_path )
	{
		std::string name_ret;
		path_split( path, nullptr, &name_ret, nullptr );

		std::string ret = is_abs_path( path ) ? path : path_with_slash( root_path ) + path;
		return name_ret.empty() ? path_with_slash( ret ) : ret;
	}

	/// Get path and filename of current executable module.
	/// \param[out] path_ret - directory part of full path including trailing slash (optional, can be null)
	/// \param[out] name_ret - file name (optional, can be null)
	/// \param[in] for_top_module - when true the path to executable file of the current process is returned;
	/// when false, the path to the module, where this function is linked into, is returned
	/// (for Linux it is always the executable, but for Windows it can be DLL).
	static void module_path( std::string *path_ret, std::string *name_ret = nullptr, bool for_top_module = true )
	{
		std::string fullpath;

#ifdef _MSC_VER
		HMODULE hModule = NULL;
		if( !for_top_module )  // get module handle of DLL, where this function is linked into
			GetModuleHandleEx(
				GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT | GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
				(LPCTSTR)module_path,
				&hModule );
		CHAR my_filepath[ _MAX_PATH ];
		GetModuleFileNameA( hModule, my_filepath, _MAX_PATH );
		fullpath = my_filepath;
		std::replace( fullpath.begin(), fullpath.end(), '\\', '/' );

#elif defined( __APPLE__ )
		if( !for_top_module )
		{
			Dl_info info;
			if( dladdr( (void *)&module_path, &info ) )
				fullpath = std::string( info.dli_fname );
		}
		if( fullpath.empty() )
		{
			char app_path[ PATH_MAX ];
			uint32_t size = PATH_MAX;
			if( 0 == _NSGetExecutablePath( app_path, &size ) )
				fullpath = std::filesystem::canonical( app_path ).string();
		}

#else  // GCC
		if( !for_top_module )
		{
			Dl_info info;
			if( dladdr( (void *)&module_path, &info ) )
				fullpath = std::string( info.dli_fname );
		}
		if( fullpath.empty() )
			try
			{
				fullpath = std::filesystem::weakly_canonical( program_invocation_name ).string();
			}
			catch( std::exception & )
			{
				fullpath = program_invocation_name;
			}
#endif

		path_split( fullpath, path_ret, name_ret, nullptr );
	}

	/// Check for existence of given directory and create it with write access rights if it does not exist
	/// \param[in] dir_name - directory name to create
	/// \return true if directory was creates
	static bool dir_create_if_not_exist( const std::string &dir_name )
	{
		if( !dir_exist( dir_name ) )
		{
			std::filesystem::create_directories( dir_name );
			std::filesystem::permissions(
				dir_name,
				std::filesystem::perms::owner_all | std::filesystem::perms::group_all |
					std::filesystem::perms::others_all );
			return true;
		}
		return false;
	}

	/// Create uniquely-named subdirectory in system temporary directory
	/// \param[in] max_tries - maximum number of attempts to create a directory
	/// \return path to the created directory
	static std::filesystem::path create_temp_subdir( size_t max_tries = 10 )
	{
		auto tmp_dir = std::filesystem::temp_directory_path();
		std::random_device dev;
		std::mt19937 prng( dev() );
		std::uniform_int_distribution< uint64_t > rand( 0 );
		std::filesystem::path path;
		for( auto it = 0; it < max_tries; it++ )
		{
			path = tmp_dir / std::to_string( rand( prng ) );
			if( std::filesystem::create_directory( path ) )
				return path;
		}
		throw std::runtime_error( "could not find non-existing directory" );
	}

	/// Get path to user home directory (with trailing slash).
	static std::string home_dir()
	{
#if defined( WIN32 ) || defined( _WIN32 )
		const char *home_path = std::getenv( "USERPROFILE" );

#elif defined( __linux__ ) || defined( __APPLE__ )
		const char *home_path = std::getenv( "HOME" );
		if( home_path == nullptr )
		{
			struct passwd *pwd = getpwuid( getuid() );
			if( pwd != nullptr )
				home_path = pwd->pw_dir;
		}
#endif
		if( home_path == nullptr )
			return "";

		return path_with_slash( home_path );
	}

	/// Get path to DeGirum-specific application data directory: the directory which applications can use to write data
	/// to (with trailing slash). If no such directory existed before, it will be created. For Linux it is
	/// $HOME/.local/share/DeGirum For Windows it is %APPDATA%\DeGirum For MacOS it is $HOME/Library/Application
	/// Support/DeGirum
	static std::string appdata_dg_dir()
	{
		auto appdata_path{ std::filesystem::temp_directory_path() / "DeGirum" };  // temp path is used as a fallback

#if defined( WIN32 ) || defined( _WIN32 )
		{
			const char *profile_path = std::getenv( "APPDATA" );
			if( profile_path != nullptr )
				appdata_path = std::filesystem::path( profile_path ) / "DeGirum";
		}
#elif defined( __linux__ ) || defined( __APPLE__ )
		{
			std::filesystem::path home_path( home_dir() );
			if( !home_path.empty() )
	#if defined( __linux__ )
				appdata_path = home_path / ".local/share/DeGirum";
	#elif defined( __APPLE__ )
				appdata_path = home_path / "Library/Application Support/DeGirum";
	#endif
		}
#endif

		const std::string ret = appdata_path.generic_string();
		dir_create_if_not_exist( ret );
		return path_with_slash( ret );
	}

	/// Count files matching given wildcard pattern in given directory
	/// \param[in] path - directory path
	/// \param[in] wildcard - typical filesystem wildcard pattern to match containing '*' and '?' characters
	/// \return number of files matching the pattern
	static int count_files_matching_wildcard( const std::string &path, const std::string &wildcard )
	{
		int count = 0;

		// Convert the wildcard pattern into a regular expression
		std::string regex_pattern = "^" + std::regex_replace( wildcard, std::regex( "\\*" ), ".*" );
		regex_pattern = std::regex_replace( regex_pattern, std::regex( "\\?" ), "." );
		std::regex pattern( regex_pattern );

		// Iterate recursively over the directory
		for( const auto &entry : std::filesystem::recursive_directory_iterator(
				 path,
				 std::filesystem::directory_options::skip_permission_denied ) )
		{
			if( !entry.is_directory() )
			{
				// Check if the file name matches the pattern
				if( std::regex_match( entry.path().filename().string(), pattern ) )
					count++;
			}
		}
		return count;
	}

	/// Get available space in given directory
	/// \param[in] dir - directory path
	/// \return available space in bytes or -1 if failed to get the information
	static size_t space_available( const std::string &dir )
	{
		try
		{
			auto spaceInfo = std::filesystem::space( dir );
			return spaceInfo.available;
		}
		catch( const std::exception & )
		{
			return size_t( -1 );
		}
	}

	/// Set current working directory to current executable location directory
	/// \return original current working directory
	static std::string cwd2exe()
	{
		std::string exe_path;
		module_path( &exe_path );
		const auto cwd = std::filesystem::current_path();
		std::filesystem::current_path( exe_path );
		return cwd.string();
	}

	/// Lock file, wrapped by ofstream object
	/// \param[in] fileStream - filestream object to lock
	/// \return true if file is locked, false if not (meaning that it is already locked by somebody else)
	static bool lockFileStreamUnderlyingFileHandle( std::ofstream &fileStream )
	{
#ifndef _MSC_VER
		/// Wrapper class to access protected member
		class LockingFileBuf: public std::filebuf
		{
		public:
			/// Constructor
			/// \param[in] buf - pointer to rdbuf to lock
			LockingFileBuf( std::filebuf &&buf ) : std::filebuf( std::move( buf ) )
			{}

			/// Lock rdbuf
			bool lock()
			{
	#ifndef __APPLE__
				return flock( _M_file.fd(), LOCK_EX | LOCK_NB ) != 0;
	#else
				return true;
	#endif
			}
		};

		LockingFileBuf lock_buf( std::move( *fileStream.rdbuf() ) );
		const bool ret = lock_buf.lock();
		*fileStream.rdbuf() = std::move( lock_buf );
		return ret;
#else
		return true;  // not needed in Windows
#endif
	}

	/// Get a path to a file with given file name, located in given directory,
	/// which is not currently opened by any other process. If such file is already
	/// open by another process, add numeric suffix to the file name.
	/// If such file existed before, rename it to .bak
	/// Full filename will be in the following format:
	/// [dir]/[file name].[number].[file extension]
	///
	/// \param[in] dir - directory to deal with
	/// \param[in] file_name - desired file name with extension (path component, if any, is ignored)
	/// \return full path to such file
	/// NOTE: implementation is not protected against race conditions!
	static std::string notUsedFileInDirBackupAndGet( const std::string &dir, const std::string &file_name )
	{
		const std::string file_stem = std::filesystem::path( file_name ).stem().string();      // name w/o extension
		const std::string file_ext = std::filesystem::path( file_name ).extension().string();  // extension with dot
		const std::string path_prefix = path_with_slash( dir ) + file_stem;

		dir_create_if_not_exist( dir );
		for( int idx = 0; idx < 100 /*we need some limit anyway*/; idx++ )
		{
			const std::string try_filename_no_ext = path_prefix + ( idx == 0 ? "" : "." + std::to_string( idx ) );
			const std::string try_filename = try_filename_no_ext + file_ext;

			if( !fexist( try_filename ) )  // file not exist: just use it
				return try_filename;

#ifndef _MSC_VER
			// check file lock
			{
				const int fd = open( try_filename.c_str(), O_RDONLY );
				if( fd == -1 )
					continue;  // cannot open file
				const bool locked = flock( fd, LOCK_EX | LOCK_NB ) == 0;
				if( locked )
					flock( fd, LOCK_UN );
				close( fd );
				if( !locked )
					continue;  // cannot lock: file is most likely opened by another process, try next one
			}
#endif

			// try to rename to .bak
			std::error_code io_err;
			std::filesystem::rename( try_filename, try_filename_no_ext + ".bak", io_err );
			if( io_err )
				continue;  // cannot rename: file is most likely opened by another process, try next one

			return try_filename;
		}

		// when all fails (unlikely), just return the original file name
		return path_prefix + file_ext;
	}

	/// Get upper limit for the number of virtual CPU devices available in the system.
	/// By default, report half of the number of threads available.
	/// If DG_CPU_LIMIT_CORES env.var. is defined, report it's value.
	static inline int systemVCPULimitGet()
	{
		const int host_cpu_limit = std::thread::hardware_concurrency() / 2;
		const char *cpu_limit_env = getenv( "DG_CPU_LIMIT_CORES" );
		if( cpu_limit_env != nullptr && *cpu_limit_env != '\0' )
			return std::min( host_cpu_limit, std::max( 2, std::stoi( cpu_limit_env ) ) );
		else
			return std::max( 1, host_cpu_limit );
	}

	/// Get upper limit for the physical system memory in bytes
	/// If DG_MEMORY_LIMIT_BYTES env.var. is defined, report it's value.
	static inline size_t systemRAMLimitGet()
	{
		size_t total_memory = 0;

#ifdef _WIN32
		{
			MEMORYSTATUSEX status;
			status.dwLength = sizeof( status );
			if( GlobalMemoryStatusEx( &status ) )
				total_memory = status.ullTotalPhys;
			else
				total_memory = 1 << 30;  // some reasonable default
		}
#elif defined( __linux__ ) || defined( __APPLE__ )
		{
			long pages = sysconf( _SC_PHYS_PAGES );
			long page_size = sysconf( _SC_PAGE_SIZE );
			total_memory = pages * page_size;
		}
#else
	#error "Unsupported platform"
#endif

		const char *memory_limit = getenv( "DG_MEMORY_LIMIT_BYTES" );
		if( memory_limit != nullptr && *memory_limit != '\0' )
		{
			char *endptr = 0;
			size_t total_memory_env = size_t( std::strtoull( memory_limit, &endptr, 10 ) );
			switch( *endptr )
			{
			case 'k':
			case 'K':
				total_memory_env *= 1 << 10;
				break;
			case 'm':
			case 'M':
				total_memory_env *= 1 << 20;
				break;
			case 'g':
			case 'G':
				total_memory_env *= 1 << 30;
				break;
			}
			total_memory = std::min( total_memory, std::max( size_t{ 1 << 30 }, total_memory_env ) );
		}

		return total_memory;
	}

	/// Get currently used memory in bytes
	static inline size_t systemRAMUsedGet()
	{
		size_t current_memory = 0;

#ifdef _WIN32

		PROCESS_MEMORY_COUNTERS_EX counters;
		GetProcessMemoryInfo( GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS *)&counters, sizeof( counters ) );
		current_memory = counters.PrivateUsage;

#elif defined( __linux__ )

		FILE *file = fopen( "/proc/self/status", "r" );
		char line[ 128 ];

		if( file == nullptr )
			return 0;

		while( fgets( line, 128, file ) != nullptr )
		{
			if( strncmp( line, "VmRSS:", 6 ) == 0 )
			{
				int success = sscanf( line, "VmRSS: %zu", &current_memory );
				break;
			}
		}
		fclose( file );
		current_memory *= 1024;

#elif defined( __APPLE__ )

		kern_return_t success;
		mach_task_basic_info_data_t info;
		mach_msg_type_number_t count = MACH_TASK_BASIC_INFO_COUNT;

		success = task_info( mach_task_self(), MACH_TASK_BASIC_INFO, (task_info_t)&info, &count );
		if( success != KERN_SUCCESS || count != MACH_TASK_BASIC_INFO_COUNT )
			current_memory = FileHelper::systemRAMLimitGet();
		else
			current_memory = info.resident_size;

#endif

		return current_memory;
	}
};
}  // namespace DG

#include "DGErrorHandling.h"

// Load text file to string buffer (implementation)
inline std::string DG::FileHelper::file2string( const std::string &path, bool is_binary )
{
	std::ifstream fin( path.c_str(), is_binary ? ( std::ios::in | std::ios::binary ) : std::ios::in );
	if( fin.fail() )
		DG_ERROR( "Error reading file " + path, ErrFileReadFailed );
	return std::string( std::istreambuf_iterator< char >( fin ), std::istreambuf_iterator< char >() );
}

// Save string buffer to text file (implementation)
inline void DG::FileHelper::string2file( const std::string &path, const std::string &str )
{
	std::ofstream fout( path.c_str() );
	if( fout.fail() )
		DG_ERROR( "Error writing file " + path, ErrFileWriteFailed );
	fout.write( str.c_str(), str.length() );
}

// Load binary file to vector buffer (implementation)
template< typename T >
std::vector< T > DG::FileHelper::file2vector( const std::string &path )
{
	std::ifstream fin( path, std::ios_base::in | std::ios_base::binary );
	if( fin.fail() )
		DG_ERROR( "Error reading file " + path, ErrFileReadFailed );

	// get file size
	fin.seekg( 0, fin.end );
	auto f_length = fin.tellg();
	fin.seekg( 0, fin.beg );

	std::vector< T > buf( f_length / sizeof( T ) );
	fin.read( (char *)buf.data(), f_length );
	return buf;
}

// Save vector buffer to binary file (implementation)
template< typename T >
void DG::FileHelper::vector2file( const std::string &path, const std::vector< T > &buf )
{
	std::ofstream fout( path.c_str(), std::ios_base::out | std::ios_base::binary );
	if( fout.fail() )
		DG_ERROR( "Error writing file " + path, ErrFileWriteFailed );
	fout.write( (const char *)buf.data(), buf.size() * sizeof( T ) );
}

// Save stringview buffer to binary file (implementation)
template< typename CharT, typename Traits >
void DG::FileHelper::stringview2file( const std::string &path, std::basic_string_view< CharT, Traits > buf )
{
	std::ofstream fout( path.c_str(), std::ios_base::out | std::ios_base::binary );
	if( fout.fail() )
		DG_ERROR( "Error writing file " + path, ErrFileWriteFailed );
	fout.write( (const char *)buf.data(), buf.size() * sizeof( CharT ) );
}

#endif  // DG_FILE_UTILITIES_H_
