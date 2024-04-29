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

#if defined _MSC_VER || defined _WIN32 || defined _WIN64
#	define restricted __declspec( restrict )	// indicates that a symbol is not aliased in the current scope, this semantic is propagated by the compiler
#	define noaliasing __declspec( noalias )		// declares that the function does not modify memory outside the first level of indirection from the function's parameters. That is, the parameters are the only reference to the outside world the function has.
#elif defined __unix__ || defined __unix || defined __APPLE__ && defined __MACH__
#	define restricted __restrict__
#	define noaliasing __restrict__
#endif

#define isOfType( obj, T ) ( dynamic_cast<T*>( obj ) != nullptr ) ? true : false
#define SAFE_CALL( obj, function )			{ if ( obj ) { obj.function; } }
#define SAFE_CALL_POINTER( obj, function )	{ if ( obj ) { obj->function; } }

//===================================================
//	\macro	ALIAS_FUNCTION
//	\brief  optimal way of renaming/aliasing a function to another - can be inline & no unnecessary copies
//	\date	2022/08/29 13:33
#define ALIAS_FUNCTION( originalFunctionName, aliasedFunctionName ) \
	template <typename... TArgs>\
	inline auto aliasedFunctionName( TArgs&&... args ) -> decltype( originalFunctionName( std::forward<TArgs>( args )... ) )\
	{\
		return  originalFunctionName( std::forward<TArgs>( args )... );\
	}

#define ENUM_STR( e )				(#e)
#define ENUM_WSTR( e )				( L ## (#e) )
#define PRINT_ENUM( e )				std::printf( "'%s'", (#e) );
#define PRINTW_ENUM( e )			std::wprintf( L"'%s'", (#e) );

#define CLASS_NAMER( className ) private:\
		static inline constexpr const char *s_className = #className ;\
	public:\
		static constexpr const char* getClassName() noexcept\
		{\
			return s_className;\
		}\
	private:\
// use it like so: `CLASS_NAMER( MyClassName );` as the first statement of your class


template<typename T>
constexpr typename std::remove_reference<T>::type makePrValue( T &&val )
{
	return val;
}

#define IS_CONSTEXPR(e)	noexcept( makePrValue(e) )


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
void safeDelete( T &pSm )
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
std::string intToStr( int integer ) noexcept;
int stringToInt( const std::string &str ) noexcept;

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
time_t secondsToTimeT( const int s );
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

unsigned char mapToByte( double value );

//	\function	isAligned	||	\date	2022/08/30 9:40
//	\brief	check whether the address is aligned to `alignment` boundary
bool isAligned( const volatile void *p, const std::size_t alignment ) noexcept;
bool isAligned( const std::uintptr_t pi, const std::size_t alignment ) noexcept;
constexpr const int is4ByteAligned( const intptr_t *addr );

//	\function	alignForward	||	\date	2022/02/20 20:34
//	\brief	align pointer forward with given alignment
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
//	\function	calcAlignedSize	||	\date	2022/08/30 9:39
//	\brief	calculates alignment in bits supposedly
std::size_t calcAlignedSize( const std::size_t size, const std::size_t alignment );
//	\function	getForwardPadding	||	\date	2022/08/30 9:40
//	\brief	calculate padding bytes needed to align address p forward given the alignment
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

// if you allocate an object on the heap, it will be given a unique address within that process; nothing else will be assigned that address until you free the object
[[nodiscard]] inline restricted noaliasing void* getUniqueMemory( const std::size_t bytes )
{
	return std::malloc(bytes);
}

template<typename T>
T& deconst( const T &obj )
{
	return const_cast<T&>( obj );
}

template<typename T>
T* deconst( const T *obj )
{
	return const_cast<T*>( obj );
}


////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
// ALGORITHMS
////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T>
void removeByBackSwap( std::vector<T> &v,
	std::size_t index )
{
	typename std::vector<T>::iterator itEnd = v.back();
	std::swap( v[index], itEnd );
	v.pop_back();
}

template<typename T>
void removeByBackSwap( std::vector<T> &v,
	typename std::vector<T>::iterator it )
{
	typename std::vector<T>::iterator itEnd = v.back();
	std::swap( it, itEnd );
	v.pop_back();
}

template<typename TContainer, typename Predicate>
void removeByBackSwap( TContainer &c,
	Predicate pred )
{
	const auto newEnd = std::remove_if( c.begin(), c.end(), pred );
	c.erase( newEnd, c.end() );
}

//	\function	shrinkCapacity	||	\date	2022/04/01 20:51
//	\brief	shrink vector's capacity to its size
template<typename T, class Alloc = std::allocator<T>>
void shrinkCapacity( std::vector<T, Alloc>& v )
{
	std::vector<T, Alloc>( v.begin(), v.end() ).swap( v );
}

template<typename TContainer, typename TP>
decltype(auto) pointerToIterator( TContainer &c,
	TP pElem )
{
	return std::find_if( c.begin(), c.end(), [&pElem] ( const TP &p )
		{
			return pElem == std::addressof(*p);
		} );
}

// note that there is std::move(inIt1, inIt2, outIt);
template<class InputIt, class OutputIt, class TPredicate>
void moveIf( InputIt srcFirst,
	InputIt srcLast,
	OutputIt *destFirst,
	TPredicate predicate )
{
	std::copy_if( std::move_iterator( srcFirst ), std::move_iterator( srcLast ), std::back_inserter( *destFirst ), predicate );
}

//	\function	splitMovePartition	||	\date	2022/07/29 21:03
//	\brief  like partition_move - puts the second group of an std::partition to another container removing them from the source container
template<class Container, class TPredicate>
void splitMovePartition( Container &src,
	Container &dest,
	TPredicate p )
{
	auto newEnd = std::partition_copy( std::move_iterator( src.begin() ), std::move_iterator( src.end() ), src.begin(), std::back_inserter( dest ), p );
	src.erase( newEnd.first, src.end() );
}

template<typename TContainer>
void printContainer( const TContainer &cont,
	const char *delimiter = " " )
{
	std::copy( cont.begin(), cont.end(), std::ostream_iterator<typename TContainer::value_type>( std::cout, delimiter ) );
}

void regexSearch( const std::regex &pattern );

template<typename TContainer, typename Func>
decltype(auto) doForAll( TContainer &c,
	Func f )
{
	for ( auto &i : c )
	{
		f( i );
	}
}

template<class Container, class FilterFunction>
decltype(auto) filterContainer( Container &c,
	const FilterFunction&& f )
{
	static_assert( is_container_v<Container>, "c is not a container type!" );

	return std::transform( std::execution::par, c.begin(), c.end(), f );
}


template<typename TContainer>
typename TContainer::iterator erase( TContainer& container,
	typename TContainer::const_reference element_to_erase )
{
	return container.erase( std::remove( std::begin( container ), std::end( container ), element_to_erase ), std::end( container ) );
}

template<typename TContainer, typename TPredicate>
typename TContainer::iterator eraseIf( TContainer& container,
	TPredicate&& predicate )
{
	const TContainer::iterator old_end_itr = std::end( container );
	const TContainer::iterator new_end_itr = std::remove_if( std::begin( container ), old_end_itr, std::forward<TPredicate>( predicate ) );

	return container.erase( new_end_itr, old_end_itr );
}

template<typename TContainer, typename T>
bool containerContains( const TContainer& container,
	const T& val )
{
	const auto cend = std::cend( container );
	return std::find( std::cbegin( container ), cend, val ) != cend;
}

//	\function	atLeastNOfRange	\date	2022/08/28 23:30
//	\brief	test if at least N elements of an iterator range match a predicate
//			earlies out and returns once the required amount of elements have been matched
template<typename TIt, typename TPredicate>
bool atLeastNOfRange( TIt begin,
	TIt end,
	size_t n,
	TPredicate &&predicate )
{
	using T = decltype( *begin );

	return n == 0u || std::any_of( begin, end, [&n, &predicate] ( const T &element )
			{
				n -= predicate( element ) ? 1u : 0u;
				return n == 0u;
			} );
}

template<typename TContainer, typename TPredicate>
bool atLeastNOfRange( const TContainer &container,
	size_t n,
	TPredicate &&predicate )
{
	return atLeastNOfRange( std::cbegin( container ), std::cend( container ), n, std::forward<TPredicate>( predicate ) );
}

template<typename TContainer, typename T>
unsigned indexOf( const TContainer &container,
	const T &val )
{
	const auto begin = std::begin( container );
	const auto end = std::end( container );

	return std::distance( begin, std::find( begin, end, val ) );
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

template<typename TContainer, typename TPredicate, typename... TArgs>
typename TContainer::iterator emplaceBackUniqueAndReturnIt( TContainer &container,
	TPredicate &&predicate,
	TArgs&&... args )
{
	typename TContainer::value_type* result = tryFindIf( container,
		std::forward<TPredicate>( predicate ) );
	if ( result == nullptr )
	{
		return &container.emplace_back( std::forward<TArgs>( args )... );

	}
	return result;
}

template<typename TContainer, typename TComparator>
bool hasDuplicates( const TContainer &container,
	const TComparator &comparator )
{
	using TIt = typename TContainer::const_iterator;
	using T = typename TContainer::value_type;

	bool is_unique = true;

	for ( TIt end = std::end( container ), itr1 = std::begin( container ); is_unique && itr1 != end; itr1 = std::next( itr1 ) )
	{
		//If the type contained in the container is a pointer or a number we pass everything by value
		if constexpr ( std::is_pointer_v<T> || std::is_arithmetic_v<T> )
		{
			const auto uniqueness_predicate = [&comparator, lhs = *itr1]( const T rhs )
			{
				return comparator( lhs, rhs );
			};

			is_unique = std::none_of( std::next( itr1 ), end, uniqueness_predicate );
		}
		// Otherwise pass everything by const lval ref
		else
		{
			const auto uniqueness_predicate = [&comparator, &lhs = *itr1]( typename TContainer::const_reference rhs )
			{
				return comparator( lhs, rhs );
			};

			is_unique = std::none_of( std::next( itr1 ), end, uniqueness_predicate );
		}

	}

	return is_unique == false;
}

template<typename TContainer>
bool hasDuplicates( const TContainer &container )
{
	return hasDuplicates( container, std::equal_to() );
}

template<typename TContainer>
std::set<typename TContainer::value_type> uniquefy( const TContainer &container )
{
	std::set<typename TContainer::value_type> seen;
	for ( const typename TContainer::value_type &in : container )
	{
		auto result = seen.insert( in );
	}
	return seen;
}


}//namespace util