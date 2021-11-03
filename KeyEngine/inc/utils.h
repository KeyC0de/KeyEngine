#pragma once

#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <iostream>
#include <bitset>

#define isOfTypeT( obj, T ) ( dynamic_cast<T*>( obj ) != nullptr ) ? true : false


namespace util
{

template<typename T>
void removeByBackSwap( std::vector<T>& v,
		std::size_t index )
{
	typename std::vector<T>::iterator pback = v.back();
	std::swap( v[index], pback );
	v.pop_back();
}
template<typename T>
void removeByBackSwap( std::vector<T>& v,
	const T& element )
{
	v.erase( std::find( v.begin(),
		v.end(),
		&element ) );
}

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
std::vector<std::string> tokenizeQuotedString( const std::string& input );
//===================================================
//	\function	s2ws
//	\brief	convert from strings/chars to wide strings/wchar_ts
//				or std::wstring( s.begin(), s.end() );
//	\date	2020/12/30 20:38
std::wstring s2ws( const std::string& narrow );
//===================================================
//	\function	ws2s
//	\brief	convert wide strings/wchar_ts to strings/chars
//	\date	2020/12/30 20:38
std::string ws2s( const std::wstring& wide );

namespace
{
template<class Iter>
void splitString_impl( const std::string& s,
	const std::string& delim,
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

std::vector<std::string> splitString( const std::string& s, const std::string& delim );

bool stringContains( std::string_view haystack, std::string_view needle );

std::string& capitalizeFirstLetter( std::string& str );
std::string&& capitalizeFirstLetter( std::string&& str );

template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
void printBinary( T val )
{
	std::bitset<32> bin{val};
	std::cout << bin;
}

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


}//util