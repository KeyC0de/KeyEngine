#pragma once

#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <bitset>
#include <cstddef>
#include <cinttypes>
#include <future>
#include "assertions_console.h"
#include "key_traits.h"

/*
__declspec(restrict) declares that the return value of a function points to memory that is not aliased. That is, the memory returned by the function is guaranteed to not be accessible through any other pointer in the program.

__declspec(noalias) declares that the function does not modify memory outside the first level of indirection from the function's parameters. That is, the parameters are the only reference to the outside world the function has.

Both of them are just performance hints to the compiler.
*/
#if defined _MSC_VER || defined _WIN32 || defined _WIN64
#	define restricted __declspec( restrict )
#	define noaliasing __declspec( noalias )
#	define compiler_hint __declspec( restrict ) __declspec( noalias )
#elif defined __unix__ || defined __unix || defined __APPLE__ && defined __MACH__
#	define restricted __restrict__
#	define noaliasing __restrict__
#	define hint __restrict__
#endif

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

#define ENUM_STR( e )				(#e)
#define ENUM_WSTR( e )				( L ## (#e) )
#define PRINT_ENUM( e )				std::printf( "'%s'", (#e) );
#define PRINTW_ENUM( e )			std::wprintf( L"'%s'", (#e) );

#define CLASS_NAMER( className ) private: \
		static inline constexpr const char *s_className = #className ; \
	public: \
		static constexpr const char* getClassName() noexcept \
		{ \
			return s_className; \
		} \
	private: \
// use it like so: `CLASS_NAMER( MyClassName );` as the first statement of your class


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

template<typename T>
bool isFutureReady( const std::future<T> &fu )
{
	return fu.wait_for( std::chrono::seconds( 0 ) ) == std::future_status::ready;
}

//	\function	tokenizeQuotedString	||	\date	2021/01/12 12:54
//	\brief	converts a string input into a vector of strings
//			separation into vector element "tokens" is based on spaces or quotes '
std::vector<std::string> tokenizeQuotedString( const std::string &input );
//	\function	s2ws	||	\date	2020/12/30 20:38
//	\brief	convert from strings/chars to wide strings/wchar_ts, or std::wstring( s.begin(), s.end() );
std::wstring s2ws( const std::string &narrow );
//	\function	ws2s	||	\date	2020/12/30 20:38
//	\brief	convert wide strings/wchar_ts to strings/chars
std::string ws2s( const std::wstring &wide );

std::vector<std::string> splitString( const std::string &s, const std::string &delim );
bool stringContains( std::string_view haystack, std::string_view needle );
std::string& capitalizeFirstLetter( std::string &str );
std::string capitalizeFirstLetter( const std::string &str );

//	\function	trimL	||	\date	2022/07/29 21:12
//	\brief	trim from start (in place)
void trimL( std::string &s );
//	\function	trimR	||	\date	2022/07/29 21:13
//	\brief	trim from end (in place)
void trimR( std::string &s );
//	\function	trim	||	\date	2022/07/29 21:13
//	\brief	trim from both ends (in place)
void trim( std::string &s );
//	\function	trimCopy	||	\date	2022/07/29 21:13
//	\brief	trim from both ends (copying)
std::string trimCopy( std::string s );
//	\function	trimLCopy	||	\date	2022/07/29 21:13
//	\brief	trim from start (copying)
std::string trimLCopy( std::string s );
//	\function	trimRCopy	||	\date	2022/07/29 21:14
//	\brief	trim from end (copying)
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

//	\function	secondsToTimeT	||	\date	2022/07/28 22:35
//	\brief	convert seconds to time_t
//			Although not defined, time_t is implementation defined, it is almost always an integral value holding the number of seconds (not counting leap seconds) since 00:00, Jan 1 1970 UTC, corresponding to POSIX time.
inline time_t secondsToTimeT( const int s );
//	\function	timeTtoSeconds	||	\date	2022/07/28 22:32
//	\brief	convert time_t to seconds --- time_t can be acquired as if by means of time(nullptr)
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

//	\function	isAligned	||	\date	2022/08/30 9:40
//	\brief	check whether the address is aligned to `alignment` boundary
bool isAligned( const volatile void *p, const std::size_t alignment ) noexcept;
bool isAligned( const std::uintptr_t pi, const std::size_t alignment ) noexcept;
constexpr const int is4ByteAligned( const intptr_t *addr );

//	\function	alignForward	||	\date	2022/02/20 20:34
//	\brief	align pointer forward with given alignment
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
//	\function	calcAlignedSize	||	\date	2022/08/30 9:39
//	\brief	calculates alignment in bits supposedly
const std::size_t calcAlignedSize( const std::size_t size, const std::size_t alignment );
//	\function	getForwardPadding	||	\date	2022/08/30 9:40
//	\brief	calculate padding bytes needed to align address p forward given the alignment
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

inline compiler_hint std::size_t* getUniqueMemory()
{
	return reinterpret_cast<std::size_t*>( new std::size_t{0} );
}


}//namespace util