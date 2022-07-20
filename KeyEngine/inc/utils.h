#pragma once

#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <iostream>
#include <bitset>
#include <cstddef>
#include <cinttypes>
#include "assertions_console.h"

#define isOfTypeT( obj, T ) ( dynamic_cast<T*>( obj ) != nullptr ) ? true : false


namespace util
{

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

template<typename T>
void removeByBackSwap( std::vector<T>& v,
	std::size_t index )
{
	typename std::vector<T>::iterator itBback = v.back();
	std::swap( v[index],
		itBback );
	v.pop_back();
}

//===================================================
//	\function	shrinkCapacity
//	\brief  shrink vector's capacity to its size
//	\date	2022/04/01 20:51
template<typename T, class Alloc>
void shrinkCapacity( std::vector<T,Alloc>& v )
{
   std::vector<T,Alloc>( v.begin(),
	   v.end() ).swap( v );
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

namespace
{
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
}

std::vector<std::string> splitString( const std::string &s, const std::string &delim );
bool stringContains( std::string_view haystack, std::string_view needle );
std::string &capitalizeFirstLetter( std::string &str );
std::string capitalizeFirstLetter( const std::string &str );

template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
void printBinary( T val )
{
	std::bitset<32> bin{val};
	std::cout << bin;
}

std::tuple<int, int, int> timeToHms( float time );
std::tuple<int, int, int> secondsToHms( int totalSecs );


std::uintptr_t pointerToInt( void* p );
void* intToPointer( uintptr_t i );
void* addPointers( void* p1, void* p2 );

std::string operator+( const std::string_view& sv1, const std::string_view& sv2 );

// print a comma every 3 decimal places
template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
std::string getNumberString( T num )
{
	std::stringstream ss;
	ss.imbue( std::locale{""} );
	ss << std::fixed
		<< num;
	return ss.str();
}

std::string generateCaptcha( int len );

//  check whether the address is aligned to `alignment` boundary
bool isAligned( const volatile void* p, std::size_t alignment ) noexcept;
bool isAligned( std::uintptr_t pi, std::size_t alignment ) noexcept;
constexpr int is4ByteAligned( intptr_t *addr );

//===================================================
//	\function	alignForward
//	\brief  align pointer forward with given alignment
//	\date	2022/02/20 20:34
template<typename T>
T* alignForward( T* p,
	std::size_t alignment ) noexcept
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

std::uintptr_t alignForward( std::uintptr_t ip, std::size_t alignment ) noexcept;
// calculates alignment in bits supposedly
std::size_t calcAlignedSize( std::size_t size, std::size_t alignment );
// calculate padding bytes needed to align address p forward given the alignment
const std::size_t getForwardPadding( const std::size_t p, const std::size_t alignment );
const std::size_t getForwardPaddingWithHeader( const std::size_t p,
	const std::size_t alignment, const std::size_t headerSize );

template<typename T>
T* alignPtr( const T *ptr,
	const std::size_t alignment )
{
	const std::uintptr_t uintPtr = reinterpret_cast<std::uintptr_t>( ptr );
	const std::uintptr_t alignedUintPtr = ( uintPtr + ( alignment - 1 ) ) & ~( alignment - 1 );
	T* alignedPtr = reinterpret_cast<T*>( alignedUintPtr );
	ASSERT( isAligned( alignedPtr, alignment ), "Not aligned!" );
	return alignedPtr;
}

// TODO: doesn't work properly
void* alignedMalloc( std::size_t nBytes, std::size_t alignment );
void alignedFree( void *p ) noexcept;

// INTEL:
//void* _mm_malloc(int size, int align)
//void _mm_free(void *p)


}//namespace util