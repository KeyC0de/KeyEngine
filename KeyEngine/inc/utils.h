#pragma once

#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <bitset>
#include <cstddef>
#include <cinttypes>
#include "assertions_console.h"
#include "key_traits.h"

#define isOfTypeT( obj, T ) ( dynamic_cast<T*>( obj ) != nullptr ) ? true : false
#define SAFE_CALL( obj, function )			{ if ( obj ) { obj.function; } }
#define SAFE_CALL_POINTER( obj, function )	{ if ( obj ) { obj->function; } }

//===================================================
//	\macro	ALIAS_FUNCTION
//	\brief  optimal way of renaming/aliasing a function to another - can be inline & no unnecessary copies
//	\date	2022/08/29 13:33
#define ALIAS_FUNCTION( originalFunctionName, aliasedFunctionName) \
	template <typename... TArgs>\
	inline auto aliasedFunctionName( TArgs&&... args ) -> decltype( originalFunctionName( std::forward<TArgs>( args )... ) )\
	{\
		return  originalFunctionName( std::forward<TArgs>( args )... );\
	}


namespace util
{

template<typename T>
void safeDelete( T*& p )
{
	if ( p )
	{
		delete p;
	}
	p = nullptr;
}

template<typename T, typename = std::enable_if_t<is_pointer_wrapper_v<T>>>
void safeDelete( T& pSm )
{
	if ( pSm )
	{
		pSm = nullptr;
	}
}

template<typename T>
T sum( std::initializer_list<T> lst )
{
	T total = (T)0;
	for ( auto i : lst )
	{
		total += i;
	}
	return total;
}

//===================================================
//	\function	tokenizeQuotedString
//	\brief  converts a string input into a vector of strings
//			separation into vector element "tokens" is based on spaces or quotes '
//	\date	2021/01/12 12:54
std::vector<std::string> tokenizeQuotedString( const std::string &input );
//===================================================
//	\function	s2ws
//	\brief	convert from strings/chars to wide strings/wchar_ts
//				or std::wstring( s.begin(), s.end() );
//	\date	2020/12/30 20:38
std::wstring s2ws( const std::string &narrow );
//===================================================
//	\function	ws2s
//	\brief	convert wide strings/wchar_ts to strings/chars
//	\date	2020/12/30 20:38
std::string ws2s( const std::wstring &wide );

std::vector<std::string> splitString( const std::string &s, const std::string &delim );
bool stringContains( std::string_view haystack, std::string_view needle );
std::string& capitalizeFirstLetter( std::string &str );
std::string capitalizeFirstLetter( const std::string &str );

//===================================================
//	\function	trimL
//	\brief  trim from start (in place)
//	\date	2022/07/29 21:12
void trimL( std::string &s );
//===================================================
//	\function	trimR
//	\brief  trim from end (in place)
//	\date	2022/07/29 21:13
void trimR( std::string &s );
//===================================================
//	\function	trim
//	\brief  trim from both ends (in place)
//	\date	2022/07/29 21:13
void trim( std::string &s );
//===================================================
//	\function	trimCopy
//	\brief  trim from both ends (copying)
//	\date	2022/07/29 21:13
std::string trimCopy( std::string s );
//===================================================
//	\function	trimLCopy
//	\brief  trim from start (copying)
//	\date	2022/07/29 21:13
std::string trimLCopy( std::string s );
//===================================================
//	\function	trimRCopy
//	\brief  trim from end (copying)
//	\date	2022/07/29 21:14
std::string trimRCopy( std::string s );

template<typename T>
std::string toString( const T &t )
{
	std::stringstream ss;
	ss << t;
	return ss.str();
}


template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
void printBinary( const T val )
{
	std::bitset<32> bin{val};
	std::cout << bin;
}

std::tuple<int, int, int> timeToHms( const float time );
std::tuple<int, int, int> secondsToHms( const int totalSecs );
//===================================================
//	\function	secondsToTimeT
//	\brief	convert seconds to time_t
//			Although not defined, time_t is implementation defined
//			It is almost always an integral value holding the number of seconds (not counting leap seconds) since 00:00, Jan 1 1970 UTC, corresponding to POSIX time.
//	\date	2022/07/28 22:35
inline time_t secondsToTimeT( const int s );
//===================================================
//	\function	timeTtoSeconds
//	\brief  convert time_t to seconds
//			time_t can be acquired as if by means of time(nullptr)
//	\date	2022/07/28 22:32
long int timeTtoSeconds( const time_t t );

std::uintptr_t pointerToInt( const void *p );
void* intToPointer( const uintptr_t i );
void* addPointers( const void *p1, const void *p2 );

std::string operator+( const std::string_view &sv1, const std::string_view &sv2 );

// print a comma every 3 decimal places
template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
std::string getNumberString( const T num )
{
	std::stringstream ss;
	ss.imbue( std::locale{""} );
	ss << std::fixed
		<< num;
	return ss.str();
}

std::string generateCaptcha( int len );

//===================================================
//	\function	isAligned
//	\brief  check whether the address is aligned to `alignment` boundary
//	\date	2022/08/30 9:40
bool isAligned( const volatile void *p, const std::size_t alignment ) noexcept;
bool isAligned( const std::uintptr_t pi, const std::size_t alignment ) noexcept;
constexpr const int is4ByteAligned( const intptr_t *addr );

//===================================================
//	\function	alignForward
//	\brief  align pointer forward with given alignment
//	\date	2022/02/20 20:34
template<typename T>
const T* alignForward( const T *p,
	const std::size_t alignment ) noexcept
{
	if ( alignment == 0 )
	{
		return p;
	}
	const std::uintptr_t ip = reinterpret_cast<std::uintptr_t>( p );
	if ( ip % alignment == 0 )
	{
		return p;
	}
	return reinterpret_cast<T*>( ( ip + ( alignment - 1 ) ) & ~( alignment - 1 ) );
	// or: (ip + alignment - 1) / alignment * alignment;
}

const std::uintptr_t alignForward( const std::uintptr_t ip, const std::size_t alignment ) noexcept;
//===================================================
//	\function	calcAlignedSize
//	\brief  calculates alignment in bits supposedly
//	\date	2022/08/30 9:39
const std::size_t calcAlignedSize( const std::size_t size, const std::size_t alignment );
//===================================================
//	\function	getForwardPadding
//	\brief  calculate padding bytes needed to align address p forward given the alignment
//	\date	2022/08/30 9:40
const std::size_t getForwardPadding( const std::size_t p, const std::size_t alignment );
const std::size_t getForwardPaddingWithHeader( const std::size_t p, const std::size_t alignment, const std::size_t headerSize );

template<typename T>
T* alignPtr( const T *ptr,
	const std::size_t alignment )
{
	const std::uintptr_t uintPtr = reinterpret_cast<std::uintptr_t>( ptr );
	const std::uintptr_t alignedUintPtr = ( uintPtr + ( alignment - 1 ) ) & ~( alignment - 1 );
	T *alignedPtr = reinterpret_cast<T*>( alignedUintPtr );
	ASSERT( isAligned( alignedPtr, alignment ), "Not aligned!" );
	return alignedPtr;
}

// #FIXME: doesn't work properly
void* alignedMalloc( const std::size_t nBytes, const std::size_t alignment );
void alignedFree( void *p ) noexcept;

// INTEL:
//void *_mm_malloc(int size, int align)
//void _mm_free(void *p)

#pragma warning( disable : 4312 )
inline unsigned int volatile& readMEM( unsigned int memoryAddress )
{
	return *reinterpret_cast<unsigned int volatile*>( memoryAddress );
}
#pragma warning( default : 4312 )

#pragma warning( disable : 4312 )
inline unsigned long long int volatile& readMEM( unsigned long long int memoryAddress )
{
	return *reinterpret_cast<unsigned long long int volatile*>( memoryAddress );
}
#pragma warning( default : 4312 )


}//namespace util