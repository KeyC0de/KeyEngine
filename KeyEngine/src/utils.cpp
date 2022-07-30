#include "utils.h"
#include <algorithm>
#include <chrono>
#include <locale>
#include <codecvt>
#include <sstream>
#include <iomanip>
#include <cctype>


namespace util
{

std::vector<std::string> tokenizeQuotedString( const std::string &input )
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

template<class Iter>
void splitString_impl( const std::string &s,
	const std::string &delim,
	Iter out )
{
	if ( delim.empty() )
	{
		*out++ = s;
	}
	else
	{
		size_t a = 0;
		size_t b = s.find( delim );
		for ( ; b != std::string::npos;
			a = b + delim.length(),
			b = s.find( delim, a ) )
		{
			*out++ = std::move( s.substr( a,
				b - a ) );
		}
		*out++ = std::move( s.substr( a,
			s.length() - a ) );
	}
}

std::vector<std::string> splitString( const std::string &s,
	const std::string &delim )
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

std::string &capitalizeFirstLetter( std::string &str )
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

void trimL( std::string &s )
{
	s.erase( s.begin(),
		std::find_if( s.begin(),
			s.end(),
			[] ( unsigned char ch )
			{
				return !std::isspace( ch );
			} )
	);
}

void trimR( std::string &s )
{
	s.erase( std::find_if( s.rbegin(),
		s.rend(),
		[] ( unsigned char ch )
		{
			return !std::isspace( ch );
		} ).base(),
			s.end() );
}

void trim( std::string &s )
{
	trimL( s );
	trimR( s );
}

inline std::string trimCopy( std::string s )
{
	trim( s );
	return s;
}

inline std::string trimLCopy( std::string s )
{
	trimL( s );
	return s;
}

inline std::string trimRCopy( std::string s )
{
	trimR( s );
	return s;
}


std::tuple<int, int, int> timeToHms( float time )
{
	int hours = (int) time;
	float fminutes = ( time - hours ) * 60;
	int minutes = (int) fminutes;
	float fseconds = ( fminutes - minutes ) * 60;
	int seconds = (int) fseconds;
	return {hours, minutes, seconds};
}

std::tuple<int, int, int> secondsToHms( int totalSecs )
{
	int hours = totalSecs / 3600;
	int rest = totalSecs % 3600;
	int minutes = rest / 60;
	int seconds = rest % 60;
	return {hours, minutes, seconds};
}

//===================================================
//	\function	secondsToTimeT
//	\brief	convert seconds to time_t
//			Although not defined, time_t is implementation defined
//			It is almost always an integral value holding the number of seconds (not counting leap seconds) since 00:00, Jan 1 1970 UTC, corresponding to POSIX time.
//	\date	2022/07/28 22:35
inline time_t secondsToTimeT( int s )
{
	return std::chrono::system_clock::to_time_t( std::chrono::system_clock::time_point( std::chrono::duration_cast<std::chrono::seconds>( std::chrono::duration<int>( s ) ) ) );
}

//===================================================
//	\function	timeTtoSeconds
//	\brief  convert time_t to seconds
//			time_t can be acquired as if by means of time(nullptr)
//	\date	2022/07/28 22:32
long int timeTtoSeconds( time_t t )
{
	return static_cast<long int>( t );
}


std::uintptr_t pointerToInt( void *p )
{
	return reinterpret_cast<std::uintptr_t>( p );
}

void* intToPointer( uintptr_t i )
{
	return reinterpret_cast<void*>( i );
}

void* addPointers( void *p1,
	void *p2 )
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

bool isAligned( const volatile void *p,
	std::size_t alignment ) noexcept
{
	return ( reinterpret_cast<std::uintptr_t>( p ) % alignment ) == 0;
}

bool isAligned( std::uintptr_t pi,
	std::size_t alignment ) noexcept
{
	return ( pi % alignment ) == 0;
}

#pragma warning( push, 0 )
constexpr int is4ByteAligned( intptr_t *addr )
{
	if ( ( (intptr_t)addr & 0x3 ) == 0 )
	{ // changing int *to int
		return 1;
	}
	return 0;
}
#pragma warning( pop )

std::uintptr_t alignForward( std::uintptr_t ip,
	std::size_t alignment ) noexcept
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

// calculates alignment in bits supposedly
std::size_t calcAlignedSize( std::size_t size,
	std::size_t alignment )
{
	return size + ( size % ( alignment / 8 ) );
}

// calculate padding bytes needed to align address p forward given the alignment
const std::size_t getForwardPadding( const std::size_t p,
	const std::size_t alignment )
{
	const std::size_t mult = ( p / alignment ) + 1;
	const std::size_t alignedAddress = mult * alignment;
	const std::size_t padding = alignedAddress - p;
	return padding;
}

const std::size_t getForwardPaddingWithHeader( const std::size_t p,
	const std::size_t alignment,
	const std::size_t headerSize )
{
	std::size_t padding = getForwardPadding( p,
		alignment );
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

void* alignedMalloc( std::size_t nBytes,
	std::size_t alignment )
{
	// allocate `nBytes` + `nBytesForAlignment` required given requested `alignment` value
	// store malloced address in `pMem`
	// compute aligned address `pAlignedMem` by adding the `bytesForAdjustment` to malloced `pMem` address
	ASSERT( false, "Never should have come here!" );
	void *pMem = nullptr;
	std::size_t nBytesForAlignment = alignment - 1 + sizeof(void*);
	if ( ( pMem = static_cast<void*>( ::operator new( nBytes + nBytesForAlignment ) ) ) == nullptr )
	{
		return nullptr;
	}
	// round-up / align address forward
	std::size_t bytesForAdjustment = ( nBytesForAlignment ) & ~( alignment - 1 );
	void** pAlignedMem = reinterpret_cast<void**>( reinterpret_cast<std::size_t>( pMem )
		+ bytesForAdjustment );
	pAlignedMem[-1] = pMem;
	return pAlignedMem;
}

void alignedFree( void *p ) noexcept
{
	ASSERT( false, "Never should have come here!" );
	::operator delete( static_cast<void**>( p )[-1] );
}


}//namespace util