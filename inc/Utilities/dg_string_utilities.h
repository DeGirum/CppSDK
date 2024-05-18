//////////////////////////////////////////////////////////////////////
/// \file  dg_string_utilities.h
/// \brief DG string handling utility functions
///
/// Copyright 2024 DeGirum Corporation
///
/// This file contains implementation of various helper functions
/// for string handling
///

#ifndef DG_STRING_UTILITIES_H_
#define DG_STRING_UTILITIES_H_

#include <algorithm>
#include <string>
#include <vector>

namespace DG
{

namespace Strings
{

/// Find substring in a string with case insensitive comparison
/// \param[in] str - string to search in
/// \param[in] substr - substring to search for
/// \return true if substring is found
inline bool strSearchCI( const std::string &str, const std::string &substr )
{
	auto cmp = []( char ch1, char ch2 ) {
		return std::toupper( ch1 ) == std::toupper( ch2 );
	};
	auto it = std::search( str.begin(), str.end(), substr.begin(), substr.end(), cmp );
	return it != str.end();
}

/// Compare strings ignoring case
/// \param[in] str1 - first string
/// \param[in] str2 - second string
/// \return true if strings are equal ignoring case
inline bool strCompareCI( const std::string &str1, const std::string &str2 )
{
	auto cmp = []( char ch1, char ch2 ) {
		return std::toupper( ch1 ) == std::toupper( ch2 );
	};
	return str1.size() == str2.size() && std::equal( str1.begin(), str1.end(), str2.begin(), cmp );
}

/// Convert to uppercase
/// \param[in] str - input string
/// \return uppercase string
inline std::string toUpper( const std::string &str )
{
	std::string result = str;
	std::transform( result.begin(), result.end(), result.begin(), ::toupper );
	return result;
}

/// Convert to lowercase
/// \param[in] str - input string
/// \return lowercase string
inline std::string toLower( const std::string &str )
{
	std::string result = str;
	std::transform( result.begin(), result.end(), result.begin(), ::tolower );
	return result;
}

/// Split string into words by given delimiters
/// \param[in] input - input string
/// \param[in] delimiters - delimiters string
/// \return vector of words
inline std::vector< std::string > stringSplit( const std::string &input, const std::string &delimiters )
{
	std::vector< std::string > words;
	size_t start = 0, end;

	while( ( end = input.find_first_of( delimiters, start ) ) != std::string::npos )
	{
		if( end != start )
			words.push_back( input.substr( start, end - start ) );
		start = end + 1;
	}

	if( start < input.length() )
		words.push_back( input.substr( start ) );

	return words;
}

}  // namespace Strings

}  // namespace DG

#endif  // DG_STRING_UTILITIES_H_
