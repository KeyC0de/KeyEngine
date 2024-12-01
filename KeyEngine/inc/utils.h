#pragma once

#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <set>
#include <bitset>
#include <cstddef>
#include <cinttypes>
#include <future>
#include <algorithm>	// algorithms
#include <numeric>		// algorithms
#include <iterator>		// algorithms
#include <regex>		// algorithms
#include <execution>	// algorithms
#include "assertions_console.h"
#include "key_type_traits.h"
#include "utils_global.h"


namespace util
{

uint64_t combineUnsignedInt32to64( const unsigned int high32Bit, const unsigned int low32Bit );

template<typename T>
void safeDelete( T *&p )
{
	if ( p )
	{
		delete p;
	}
	p = nullptr;
}

template<typename T>
T sum( std::initializer_list<T> lst )
{
	T total = (T)0;
	for ( const auto &i : lst )
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

/// \brief	converts a string input into a vector of strings
/// \brief	separation into vector element "tokens" is based on spaces or quotes '
std::vector<std::string> tokenizeQuotedString( const std::string &input );
/// \brief	convert from strings/chars to wide strings/wchar_ts, or std::wstring( s.begin(), s.end() );
std::wstring s2ws( const std::string &narrow );
/// \brief	convert wide strings/wchar_ts to strings/chars
std::string ws2s( const std::wstring &wide );

void removeSubstring( std::string &str, const std::string &substring );
/// \brief	trim nChars from the start of the string
std::string trimStringFromStart( const std::string &str, const int nChars );
std::string trimStringFromEnd( const std::string &str, const int nChars );
void trimStringFromStartInPlace( std ::string &str, const int nChars );
void trimStringFromEndInPlace( std::string &str, const int nChars );
/// \brief	example:	splitDelimitedString( sea_horde_faction_keys, ";" );
std::vector<std::string> splitDelimitedString( const std::string &str, const char delimiter );
std::string assembleDelimitedString( const std::vector<std::string> &delimitedString, const char delimiter );
/// \brief	removes all occurrences of str from delimited_string
void removeFromDelimitedString( std::vector<std::string> &delimitedString, const char delimiter, const std::string &str );
bool stringContains( std::string_view haystack, std::string_view needle );
std::string& capitalizeFirstLetter( std::string &str );
std::string capitalizeFirstLetter( const std::string &str );
std::string intToStr( int integer ) noexcept;
int stringToInt( const std::string &str ) noexcept;

/// \brief	trimSpaces from start (in place)
void trimSpacesLeft( std::string &s );
/// \brief	trimSpaces from end (in place)
void trimSpacesRight( std::string &s );
/// \brief	trimSpaces from both ends (in place)
void trimSpaces( std::string &s );
/// \brief	trimSpaces from both ends (copying)
std::string trimSpacesCopy( std::string s );
/// \brief	trimSpaces from start (copying)
std::string trimSpacesLCopy( std::string s );
/// \brief	trimSpaces from end (copying)
std::string trimSpacesRightCopy( std::string s );

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

/// \brief	convert seconds to time_t
/// \brief	Although not defined, time_t is implementation defined, it is almost always an integral value holding the number of seconds (not counting leap seconds) since 00:00, Jan 1 1970 UTC, corresponding to POSIX time.
time_t secondsToTimeT( const int s );
/// \brief	convert time_t to seconds --- time_t can be acquired as if by means of time(nullptr)
long int timeTtoSeconds( const time_t t );

std::uintptr_t pointerToInt( const void *p );
void* intToPointer( const uintptr_t i );
void* addPointers( const void *p1, const void *p2 );

std::string operator+( const std::string_view &sv1, const std::string_view &sv2 );

/// \brief	print a comma every 3 decimal places
template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
std::string getNumberString( const T num )
{
	std::stringstream ss;
	ss.imbue( std::locale{""} );
	ss << std::fixed << num;
	return ss.str();
}

std::string generateCaptcha( int len );

unsigned char mapToByte( const float value );
unsigned char mapToByte( const double value );

/// \brief	check whether the address is aligned to `alignment` boundary
bool isAligned( const volatile void *p, const std::size_t alignment ) noexcept;
bool isAligned( const std::uintptr_t pi, const std::size_t alignment ) noexcept;
constexpr const int is4ByteAligned( const intptr_t *addr );

/// \brief	align pointer forward with given alignment
template<typename T>
T* alignForward( const T *p,
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

std::uintptr_t alignForward( const std::uintptr_t ip, const std::size_t alignment ) noexcept;
/// \brief	calculates alignment in bits supposedly
std::size_t calcAlignedSize( const std::size_t size, const std::size_t alignment );
/// \brief	calculate padding bytes needed to align address p forward given the alignment
std::size_t getForwardPadding( const std::size_t p, const std::size_t alignment );
std::size_t getForwardPaddingWithHeader( const std::size_t p, const std::size_t alignment, const std::size_t headerSize );

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

/// \brief	if you allocate an object on the heap, it will be given a unique address within that process; nothing else will be assigned that address until you free the object
[[nodiscard]] inline restricted noaliasing void* getUniqueMemory( const std::size_t bytes )
{
	return std::malloc(bytes);
}

template<typename T>
T& deconst( const T &obj )
{
	static_assert( !std::is_const_v<T>, "Cannot remove const from a const object!" );
	return const_cast<T&>( obj );
}

template<typename T>
T* deconst( const T *obj )
{
	static_assert( !std::is_const_v<T>, "Cannot remove const from a const object!" );
	return const_cast<T*>( obj );
}

/// \brief ALGORITHM RELATED FUNCTIONS
////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename TContainer, typename TP>
decltype(auto) pointerToIterator( TContainer &c,
	TP pElem )
{
	return std::find_if( c.begin(), c.end(), [&pElem] ( const TP &p )
		{
			return pElem == std::addressof( *p );
		} );
}

/// \brief	removeByBackSwap overloads remove an element from a container without causing iterator invalidation
template <typename TContainer>
void removeByBackSwap( TContainer &container,
	typename TContainer::size_type index )
{
	container[index] = std::move( container.back() );
	container.pop_back();
}

template <typename TContainer>
void removeByBackSwap( TContainer &container,
	typename TContainer::iterator iter )
{
	*iter = std::move( container.back() );
	container.pop_back();
}

template<typename TContainer, class T>
void removeByBackSwap( TContainer &c,
	T p )
{
	static_assert( !std::is_same_v<decltype(std::declval<TContainer>().back()), void>, "TContainer must have a back method for `removeByBackSwap` to work!" );
	static_assert( std::is_pointer_v<T>, "T must be pointer type!" );

	typename TContainer::iterator iter = pointerToIterator( c, p );
	if ( iter == c.end() )
	{
		return;
	}
	*iter = std::move( c.back() );
	c.pop_back();
}

template<typename TContainer, typename TPredicate, typename = std::enable_if_t<std::is_function_v<TPredicate>>>
void removeByBackSwap( TContainer &c,
	TPredicate &&predicate )
{
	const auto newEnd = std::remove_if( c.begin(), c.end(), std::forward<TPredicate>( predicate ) );
	c.erase( newEnd, c.end() );
}

/// \brief	shrink vector's capacity to its size
template<typename T, class Alloc = std::allocator<T>>
void shrinkCapacity( std::vector<T, Alloc>& v )
{
	std::vector<T, Alloc>( v.begin(), v.end() ).swap( v );
}

/// \brief std::vector<int> src = {1, 2, 3, 4, 5};
/// \brief std::vector<int> dest;
/// \brief moveIf( src.begin(), src.end(), std::back_inserter(dest), [](int x) { return x % 2 != 0; } );
template<class TContainer, class InputIt, class OutputIt, class TPredicate>
void moveIf( InputIt srcFirst,
	InputIt srcLast,
	std::back_insert_iterator<TContainer> destContainer,
	TPredicate &&predicate )
{
	static_assert( std::is_same_v<decltype(std::declval<TContainer>().push_back(std::declval<typename TContainer::value_type>())), void>, "`moveIf` uses `std::back_inserter` which can only be used to append at the end of containers that have the ::push_back method defined! Containers like std::vector, std::list, std::deque." );
	std::copy_if( std::move_iterator( srcFirst ), std::move_iterator( srcLast ), destContainer, std::forward<TPredicate>( predicate ) );
}

template<typename TContainer>
void printContainer( const TContainer &cont,
	const char *delimiter = " " )
{
	std::copy( cont.begin(), cont.end(), std::ostream_iterator<typename TContainer::value_type>( std::cout, delimiter ) );
}

void regexSearch( const std::regex &pattern );

template<typename TContainer, typename TFunc>
void doForAll( TContainer &c,
	TFunc &&f )
{
	for ( auto &i : c )
	{
		std::forward<TFunc>( f )( i );
	}
}

template<typename TContainer, typename TFunc>
void doForAll( const TContainer &c,
	TFunc &&f )
{
	for ( const auto &i : c )
	{
		std::forward<TFunc>( f )( i );
	}
}

template<class TContainer, class TFilterFunction>
auto filterContainer( TContainer &c,
	TFilterFunction &&f )
{
	static_assert( std::is_same_v<decltype(c.begin()), decltype(c.end())>, "c must be a container type!" );

	using ValueType = typename TContainer::value_type;
	std::vector<ValueType> result;
	result.reserve(c.size());

	// copy elements satisfying the filter predicate
	return std::copy_if( std::execution::par, c.begin(), c.end(), std::back_inserter(result), std::forward<TFilterFunction>(f) );
}

template<typename TContainer>
typename TContainer::iterator erase( TContainer &container,
	typename TContainer::const_reference elementToErase )
{
	return container.erase( std::remove( std::begin( container ), std::end( container ), elementToErase ), std::end( container ) );
}

template<typename TContainer, typename TPredicate>
typename TContainer::iterator eraseIf( TContainer &container,
	TPredicate &&predicate )
{
	typename TContainer::iterator oldEndIter = std::end( container );
	typename TContainer::iterator newEndIter = std::remove_if( std::begin( container ), oldEndIter, std::forward<TPredicate>( predicate ) );

	return container.erase( newEndIter, oldEndIter );
}

/// \brief	test if at least N elements of an iterator range match a predicate
/// \brief	earlies out and returns once the required amount of elements have been matched
template<typename InputIt, typename TPredicate>
bool atLeastNOfRange( InputIt first,
	InputIt last,
	size_t n,
	TPredicate &&predicate )
{
	size_t count = 0;
	for ( ; first != last; ++first )
	{
		if ( std::forward<TPredicate>( predicate )(*first) )
		{
			++count;
			if (count >= n)
			{
				return true; // early exit
			}
		}
	}
	return false;
}

template<typename TContainer, typename T>
unsigned indexOf( const TContainer &container,
	const T &val )
{
	const auto begin = std::begin( container );
	const auto end = std::end( container );

	return std::distance( begin, std::find( begin, end, val ) );
}

template<typename TContainer, typename T>
bool containerContains( const TContainer &container,
	const T &val )
{
	const auto cend = std::cend( container );
	return std::find( std::cbegin( container ), cend, val ) != cend;
}

template<typename TContainer, typename T, typename TInserter>
bool insertUnique( const TContainer &container,
	T &&val,
	TInserter &&inserter )
{
	if ( !containerContains( container, val ) )
	{
		inserter( std::forward<T>( val ) );
		return true;
	}
	return false;
}
ALIAS_FUNCTION( insertUnique, emplaceBackUnique );

template <typename TContainer, typename TComparator = std::equal_to<typename TContainer::value_type>>
bool hasDuplicates( const TContainer &container, const TComparator &comparator = {} )
{
	for ( typename TContainer::const_iterator it = container.begin(); it != container.end(); ++it )
	{
		auto isDuplicate = [&comparator, &lhs = *it] (const auto &rhs)
			{
				return comparator(lhs, rhs);
			};

		if ( std::any_of(std::next(it), container.end(), isDuplicate) )
		{
			return true; // duplicate found
		}
	}
	return false;
}

template<typename TContainer>
std::set<typename TContainer::value_type> uniquefy( const TContainer &container )
{
	std::set<typename TContainer::value_type> seen;
	for ( const auto &in : container )
	{
		seen.insert( in );
	}
	return seen;
}


}//namespace util