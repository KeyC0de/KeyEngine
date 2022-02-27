#include "utils.h"
#include <locale>
#include <codecvt>
#include <sstream>
#include <iomanip>
#include <cctype>


namespace util
{


std::vector<std::string> tokenizeQuotedString( const std::string& input )
{
	std::istringstream stream;
	stream.str( input );
	std::vector<std::string> tokens;
	std::string token;

	while ( stream >> std::quoted( token ) )
	{
		tokens.push_back( std::move( token ) );
	}
	return tokens;
}

std::wstring s2ws( const std::string& s )
{
	try
	{
		return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>{}.from_bytes( s );
	}
	catch( std::range_error& e )
	{
		(void)e;
		size_t length = s.length();
		std::wstring result;
		result.reserve( length );
		for ( size_t i = 0; i < length; ++i )
		{
			result.push_back( s[i] & 0xFF );
		}
		return result;
	}
}

std::string ws2s( const std::wstring& ws )
{
	try
	{
		return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>{}.to_bytes( ws );
	}
	catch( std::range_error& e )
	{
		(void)e;
		size_t length = ws.length();
		std::string result;
		result.reserve( length );
		for ( size_t i = 0; i < length; ++i )
		{
			result.push_back( ws[i] & 0xFF );
		}
		return result;
	}
}

std::vector<std::string> splitString( const std::string& s,
	const std::string& delim )
{
	std::vector<std::string> strings;
	splitString_impl( s,
		delim,
		std::back_inserter( strings ) );
	return strings;
}

bool stringContains( std::string_view haystack,
	std::string_view needle )
{
	return std::search( haystack.begin(),
		haystack.end(),
		needle.begin(),
		needle.end() ) != haystack.end();
}

std::string& capitalizeFirstLetter( std::string& str )
{
	str[0] = toupper( str[0] );
	return str;
}

std::string&& capitalizeFirstLetter( std::string&& str )
{
	str[0] = toupper( str[0] );
	return std::move( str );
}


std::uintptr_t pointerToInt( void* p )
{
	return reinterpret_cast<std::uintptr_t>( p );
}

void* intToPointer( uintptr_t i )
{
	return reinterpret_cast<void*>( i );
}

void* addPointers( void* p1,
	void* p2 )
{
	return intToPointer( pointerToInt( p1 ) + pointerToInt( p2 ) );
}

std::string operator+( const std::string_view& sv1,
	const std::string_view& sv2 )
{
	return std::string{sv1} + std::string{sv2};
}

std::string generateCaptcha( int len )
{
	const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	int charsetSize = sizeof charset / sizeof charset[0];
	//std::cout << charsetSize << '\n';
	std::string captcha;
	while ( len-- )
	{
		captcha.push_back( charset[rand() % charsetSize] );
	}
	return captcha;
}


}//util