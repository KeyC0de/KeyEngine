#include "utils.h"
#include <algorithm>
#include <vector>
#include <chrono>
#include <locale>
#include <codecvt>
#include <sstream>
#include <iomanip>
#include <cctype>


namespace util
{

uint64_t combineUnsignedInt32to64( const unsigned int high32Bit,
	const unsigned int low32Bit )
{
	return ( (uint64_t(high32Bit) << 32u) | uint64_t(low32Bit) );
}

std::vector<std::string> tokenizeQuotedString( const std::string &input )
{
	std::istringstream stream;
	stream.str( input );
	std::vector<std::string> tokens;
	std::string token;

	while ( stream >> std::quoted( token ) )
	{
		tokens.emplace_back( std::move( token ) );
	}
	return tokens;
}

std::wstring s2ws( const std::string &s )
{
	try
	{
		return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>{}.from_bytes( s );
	}
	catch( std::range_error &e )
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

std::string ws2s( const std::wstring &ws )
{
	try
	{
		return std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>{}.to_bytes( ws );
	}
	catch( std::range_error &e )
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

void removeSubstring( std::string &str,
	const std::string &substring )
{
	size_t pos;
	while ((pos = str.find(substring)) != std::string::npos)
	{
		str.erase(pos, substring.length());
	}
}

std::string trimStringFromStart( const std::string &str,
	const int nChars )
{
	std::string trimmed = str.substr( nChars, str.length() - nChars );
	return trimmed;
}

std::string trimStringFromEnd( const std::string &str,
	const int nChars )
{
	std::string trimmed = str.substr( str.length() - nChars );
	return trimmed;
}

void trimStringFromStartInPlace( std ::string &str,
	const int nChars )
{
	if ( nChars >= 0 && static_cast<std::size_t>(nChars) <= str.length() )
	{
		str.erase( 0, nChars );
	}
}

void trimStringFromEndInPlace( std::string &str,
	const int nChars )
{
	str.erase( str.length() - nChars );
}

std::vector<std::string> splitDelimitedString( const std::string& str,
	const char delimiter )
{
	std::vector<std::string> subStrings;
	std::size_t start = 0;
	for ( std::size_t pos = 0; pos < str.size(); ++pos )
	{
		if (str[pos] == delimiter)
		{
			if (pos - start > 1)
			{
				subStrings.push_back( str.substr( start, pos - start ) );
			}
			start = pos + 1;
		}
	}
 
	if ( start < str.size() )
	{
		subStrings.push_back( str.substr( start, str.size() - start ) );
	}
	return subStrings;
}

std::string assembleDelimitedString( const std::vector<std::string>& delimitedString,
	const char delimiter )
{
	std::string str;
	for (const auto& elem : delimitedString)
	{
		str += elem;
		str += delimiter;	// always add the delimiter, even after the last element
	}
	return str;
}

void removeFromDelimitedString( std::vector<std::string>& delimitedString,
	const char delimiter,
	const std::string& str )
{
	delimitedString.erase( std::remove(delimitedString.begin(), delimitedString.end(), str),
		delimitedString.end() );
}

bool stringContains( std::string_view haystack,
	std::string_view needle )
{
	return std::search( haystack.begin(), haystack.end(), needle.begin(), needle.end() ) != haystack.end();
}

std::string& capitalizeFirstLetter( std::string &str )
{
	str[0] = toupper( str[0] );
	return str;
}

std::string capitalizeFirstLetter( const std::string &str )
{
	std::string strCopy{str};
	strCopy[0] = toupper( strCopy[0] );
	return strCopy;
}

std::string intToStr( int integer ) noexcept
{
	char stringIntegers[11] = "0123456789";
	int i = 0, j = 0;
	std::string strInt;
	std::string tmp;

	// Checks whether the input integer is negative in order to store a '-', in [0] of the output string
	if ( integer < 0 )
	{
		strInt[0] = '-';
		j = 1;
		integer = -integer;
	}
	// extract digits of the input integer and store them, in opposite order (last to first digit), in temporary string
	while ( integer )
	{
		tmp[i] = stringIntegers[integer % 10];
		integer /= 10;
		++i;
	}
	// now store them in the right order
	for ( --i; i >= 0; ++j, --i )
	{
		strInt[j] = tmp[i];
	}
	strInt[j] = '\0';

	return strInt;
}

int stringToInt( const std::string &str ) noexcept
{
	int tmp;
	int result = 0;
	for ( int i = 0; str[i] >= '0' && str[i] <= '9'; ++i )
	{
		tmp = str[i] - '0';
		result = result * 10 + tmp;
	}
	return result;
}

void trimSpacesLeft( std::string &s )
{
	s.erase( s.begin(), std::find_if( s.begin(), s.end(),
		[] ( unsigned char ch )
		{
			return !std::isspace( ch );
		} ) );
}

void trimSpacesRight( std::string &s )
{
	s.erase( std::find_if( s.rbegin(), s.rend(),
		[] ( unsigned char ch )
		{
			return !std::isspace( ch );
		} ).base(), s.end() );
}

void trimSpaces( std::string &s )
{
	trimSpacesLeft( s );
	trimSpacesRight( s );
}

std::string trimSpacesCopy( std::string s )
{
	trimSpaces( s );
	return s;
}

std::string trimSpacesLCopy( std::string s )
{
	trimSpacesLeft( s );
	return s;
}

std::string trimSpacesRightCopy( std::string s )
{
	trimSpacesRight( s );
	return s;
}


std::tuple<int, int, int> timeToHms( const float time )
{
	int hours = (int) time;
	float fminutes = ( time - hours ) * 60;
	int minutes = (int) fminutes;
	float fseconds = ( fminutes - minutes ) * 60;
	int seconds = (int) fseconds;
	return {hours, minutes, seconds};
}

std::tuple<int, int, int> secondsToHms( const int totalSecs )
{
	int hours = totalSecs / 3600;
	int rest = totalSecs % 3600;
	int minutes = rest / 60;
	int seconds = rest % 60;
	return {hours, minutes, seconds};
}

time_t secondsToTimeT( const int s )
{
	return std::chrono::system_clock::to_time_t( std::chrono::system_clock::time_point( std::chrono::duration_cast<std::chrono::seconds>( std::chrono::duration<int>( s ) ) ) );
}

long int timeTtoSeconds( const time_t t )
{
	return static_cast<long int>( t );
}

std::uintptr_t pointerToInt( const void *p )
{
	return reinterpret_cast<std::uintptr_t>( p );
}

void* intToPointer( const uintptr_t i )
{
	return reinterpret_cast<void*>( i );
}

void* addPointers( const void *p1,
	const void *p2 )
{
	return intToPointer( pointerToInt( p1 ) + pointerToInt( p2 ) );
}

std::string operator+( const std::string_view &sv1,
	const std::string_view &sv2 )
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

unsigned char mapToByte( const float value )
{
	return static_cast<unsigned char>(floor( fmod( value, 255 ) ));
}

unsigned char mapToByte( const double value )
{
	return static_cast<unsigned char>(floor( fmod( value, 255 ) ));
}

bool isAligned( const volatile void *p,
	const std::size_t alignment ) noexcept
{
	return ( reinterpret_cast<std::uintptr_t>( p ) % alignment ) == 0;
}

bool isAligned( const std::uintptr_t pi,
	const std::size_t alignment ) noexcept
{
	return ( pi % alignment ) == 0;
}

#pragma warning( push, 0 )
constexpr const int is4ByteAligned( const intptr_t *addr )
{
	if ( ( (intptr_t)addr & 0x3 ) == 0 )
	{ // changing int *to int
		return 1;
	}
	return 0;
}
#pragma warning( pop )

std::uintptr_t alignForward( const std::uintptr_t ip,
	const std::size_t alignment ) noexcept
{
	if ( alignment == 0 )
	{
		return ip;
	}
	if ( ip % alignment == 0 )
	{
		return ip;
	}
	return ( ip + ( alignment - 1 ) ) & ~( alignment - 1 );
	// or: (ip + alignment - 1) / alignment * alignment;
}

std::size_t calcAlignedSize( const std::size_t size,
	const std::size_t alignment )
{
	return size + ( size % ( alignment / 8 ) );
}

std::size_t getForwardPadding( const std::size_t p,
	const std::size_t alignment )
{
	const std::size_t mult = ( p / alignment ) + 1;
	const std::size_t alignedAddress = mult * alignment;
	const std::size_t padding = alignedAddress - p;
	return padding;
}

std::size_t getForwardPaddingWithHeader( const std::size_t p,
	const std::size_t alignment,
	const std::size_t headerSize )
{
	std::size_t padding = getForwardPadding( p, alignment );
	std::size_t neededSpace = headerSize;

	if ( padding < neededSpace )
	{
		// Header does not fit - Calculate next aligned address that header fits
		neededSpace -= padding;

		// How many alignments I need to fit the header
		if ( neededSpace % alignment > 0 )
		{
			padding += alignment * ( 1 + ( neededSpace / alignment ) );
		}
		else
		{
			padding += alignment * ( neededSpace / alignment );
		}
	}
	return padding;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
// ALGORITHMS
////////////////////////////////////////////////////////////////////////////////////////////////////

void regexSearch( const std::regex &pattern )
{
	int lineNo = 0;
	for ( std::string line; std::getline( std::cin, line ); )
	{
		++lineNo;
		std::smatch matches;
		if ( std::regex_search( line, matches, pattern ) )
		{
			std::cout << lineNo
				<< ": "
				<< matches[0]
				<< '\n';
		}
	}
}


}//namespace util