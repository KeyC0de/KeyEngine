#pragma once

#include <algorithm>
#include <numeric>
#include <iterator>
#include <vector>
#include <regex>
#include <iostream>
#include <key_traits.h>


namespace util
{

template<typename T>
void removeByBackSwap( std::vector<T> &v,
	std::size_t index )
{
	typename std::vector<T>::iterator itEnd = v.back();
	std::swap( v[index],
		itEnd );
	v.pop_back();
}

template<typename T>
void removeByBackSwap( std::vector<T> &v,
	typename std::vector<T>::iterator it )
{
	typename std::vector<T>::iterator itEnd = v.back();
	std::swap( it,
		itEnd );
	v.pop_back();
}

template<typename Container, typename Predicate>
void removeByBackSwap( Container &c,
	Predicate pred )
{
	const auto newEnd = std::remove_if( c.begin(),
		c.end(),
		pred );
	c.erase( newEnd,
		c.end() );
}

//	\function	shrinkCapacity	||	\date	2022/04/01 20:51
//	\brief	shrink vector's capacity to its size
template<typename T, class Alloc = std::allocator<T>>
void shrinkCapacity( std::vector<T, Alloc>& v )
{
	std::vector<T, Alloc>( v.begin(),
		v.end() ).swap( v );
}

//	\function	sizeAsInt	||	\date	2022/07/29 13:36
//	\brief	When using int loop indexes, use size_as_int(container) instead of container.size() in order
//			to document the inherent assumption that the size of the container can be represented by an int.
template<typename TContainer>
constexpr int sizeAsInt( const TContainer &c )
{
	const auto /*typename TContainer::size_type*/ size = c.size();
	static_assert( size <= static_cast<std::size_t>( std::numeric_limits<int>::max() ) );
	return static_cast<int>( size );
}

template<typename Container, typename Func>
decltype(auto) forAll( Container &c,
	Func f )
{
	for ( auto &i : c )
	{
		f( i );
	}
}

template<typename Container, typename TP>
decltype(auto) pointerToIterator( Container &c,
	TP pItem )
{
	return std::find_if( c.begin(),
		c.end(),
		[&pItem] ( const TP &p )
		{
			return pItem == *p;
		} );
}

template<class InputIt, class OutputIt, class TPredicate>
void move_if( InputIt srcFirst,
	InputIt srcLast,
	OutputIt *destFirst,
	TPredicate predicate )
{
	std::copy_if( std::make_move_iterator( srcFirst ),
		std::make_move_iterator( srcLast ),
		std::back_inserter( *destFirst ),
		predicate );
}

//	\function	splitMovePartition	||	\date	2022/07/29 21:03
//	\brief  like partition_move - puts the second group of an std::partition to another container removing them from the source container
template<class Container, class TPredicate>
void splitMovePartition( Container &src,
	Container &dest,
	TPredicate p )
{
	auto newEnd = std::partition_copy( std::make_move_iterator( src.begin() ),
		std::make_move_iterator( src.end() ),
		src.begin(),
		std::back_inserter( dest ),
		p );
	src.erase( newEnd.first,
		src.end() );
}

template<typename Container>
void printContainer( const Container &cont,
	const char *delimiter = " " )
{
	std::copy( cont.begin(),
		cont.end(),
		std::ostream_iterator<Container::value_type>( std::cout, delimiter ) );
}

void regexSearch( const std::regex &pattern );

//Performs an erase on an unordered range. Swap item with last element and erase.
template<typename TContainer, typename TIt>
void unordered_erase( TContainer& collection, TIt erase_itr )
{
	TIt last_elem_itr = std::prev( std::end( collection ) );

	if ( last_elem_itr != erase_itr )
	{
		*erase_itr = std::move( *last_elem_itr );
	}

	collection.erase( last_elem_itr );
}

template<typename TContainer, typename T_VALUE>
bool find_and_unordered_erase( TContainer& collection, const T_VALUE& value )
{
	using TIt = typename TContainer::iterator;

	const TIt erase_itr( find( collection, value ) );

	if ( erase_itr != std::end( collection ) )
	{
		unordered_erase( collection, erase_itr );
		return true;
	}

	return false;
}

template<typename TContainer, typename TPredicate>
bool find_if_and_unordered_erase( TContainer& collection, TPredicate&& predicate )
{
	using TIt = typename TContainer::iterator;

	const TIt erase_itr( find_if( collection, std::forward<TPredicate>( predicate ) ) );

	if ( erase_itr != std::end( collection ) )
	{
		unordered_erase( collection, erase_itr );
		return true;
	}

	return false;
}

//Performs an extract on an unordered range. Move out value, swap with last element, erase and return the moved out value.
template<typename TContainer, typename TIt>
decltype( auto ) unordered_extract( TContainer& collection, TIt erase_itr )
{
	using T = typename TContainer::value_type;

	TIt last_elem_itr = std::prev( std::end( collection ) );

	T result( std::move( *erase_itr ) );

	if ( last_elem_itr != erase_itr )
	{
		*erase_itr = std::move( *last_elem_itr );
	}

	collection.erase( last_elem_itr );

	return result;
}

template<typename TContainer, typename T>
typename TContainer::const_iterator find( const TContainer& collection,
	const T& elem )
{
	return std::find( std::cbegin( collection ),
		std::cend( collection ),
		elem );
}

template<typename TContainer, typename T>
typename TContainer::iterator find( TContainer& collection,
	const T& elem )
{
	return std::find( std::begin( collection ),
		std::end( collection ),
		elem );
}

template<typename TContainer, typename T>
decltype( auto ) try_find( const TContainer& collection,
	const T& elem )
{
	const TContainer::const_iterator cend_itr = std::cend( collection );
	const TContainer::const_iterator search_itr = std::find( std::cbegin( collection ),
		cend_itr,
		elem );

	using T = typename TContainer::value_type;

	if constexpr ( std::is_pointer_v<T> )
	{
		return search_itr != cend_itr ? *search_itr : nullptr;
	}
	else if constexpr ( is_pointer_wrapper_v<T> )
	{
		return search_itr != cend_itr ? search_itr->get() : nullptr;
	}
	else
	{
		return search_itr != cend_itr ? static_cast<const T*>( &*search_itr ) : nullptr;
	}
}

template<typename TContainer, typename T>
decltype( auto ) try_find( TContainer& collection,
	const T& elem )
{
	auto result = try_find( const_cast<const TContainer&>( collection ), elem );
	return const_cast<std::remove_const_t<std::remove_pointer_t<decltype( result )>>*>( result );
}

template<typename TContainer>
typename TContainer::iterator remove( TContainer& collection,
	typename TContainer::const_reference element_to_remove )
{
	return std::remove( std::begin( collection ),
		std::end( collection ),
		element_to_remove );
}

template<typename TContainer>
typename TContainer::iterator erase( TContainer& collection,
	typename TContainer::const_reference element_to_erase )
{
	const TContainer::iterator end_itr = std::end( collection );
	return collection.erase( std::remove( std::begin( collection ), end_itr, element_to_erase ),
		end_itr );
}

template<typename TContainer, typename TPredicate>
typename TContainer::const_iterator find_if( const TContainer& collection,
	TPredicate&& predicate )
{
	return std::find_if( std::cbegin( collection ),
		std::cend( collection ),
		std::forward<TPredicate>( predicate ) );
}

template<typename TContainer, typename TPredicate>
typename TContainer::iterator find_if( TContainer& collection,
	TPredicate&& predicate )
{
	return std::find_if( std::begin( collection ),
		std::end( collection ),
		std::forward<TPredicate>( predicate ) );
}

template<typename TContainer, typename TPredicate>
decltype( auto ) try_find_if( const TContainer& collection, TPredicate&& predicate )
{
	const auto cend_itr = std::cend( collection );
	const auto search_itr = std::find_if( std::cbegin( collection ),
		cend_itr,
		std::forward<TPredicate>( predicate ) );

	using T = std::remove_reference_t<decltype( *search_itr )>;

	if constexpr ( std::is_pointer_v<T> )
	{
		return search_itr != cend_itr ?
			*search_itr :
			nullptr;
	}
	else if constexpr ( is_pointer_wrapper_v<T> )
	{
		return search_itr != cend_itr ?
			search_itr->get() :
			nullptr;
	}
	else
	{
		return search_itr != cend_itr ?
			static_cast<const T*>( &*search_itr )
			: nullptr;
	}
}

template<typename TContainer, typename TPredicate>
decltype( auto ) try_find_if( TContainer& collection,
	TPredicate&& predicate )
{
	auto result = try_find_if( const_cast<const TContainer&>( collection ),
		std::forward<TPredicate>( predicate ) );
	return const_cast<std::remove_const_t<std::remove_pointer_t<decltype( result )>>*>( result );
}

template<typename TContainer, typename TPredicate>
typename TContainer::iterator erase_if( TContainer& collection,
	TPredicate&& predicate )
{
	const TContainer::iterator old_end_itr = std::end( collection );
	const TContainer::iterator new_end_itr = std::remove_if( std::begin( collection ),
		old_end_itr,
		std::forward<TPredicate>( predicate ) );

	return collection.erase( new_end_itr,
		old_end_itr );
}

template<typename TContainer, typename T>
bool collection_contains( const TContainer& collection,
	const T& val )
{
	const auto cend = std::cend( collection );
	return std::find( std::cbegin( collection ), cend, val ) != cend;
}

template<typename TContainer, typename TPredicate>
bool any_of( const TContainer& collection,
	TPredicate&& predicate )
{
	return std::any_of( std::cbegin( collection ),
		std::cend( collection ),
		std::forward<TPredicate>( predicate ) );
}

//	\function	at_least_n_of_range	\date	2022/08/28 23:30
//	\brief	test if at least N elements of an iterator range match a predicate
//			earlies out and returns once the required amount of elements have been matched
template<typename TIt, typename TPredicate>
bool at_least_n_of_range( TIt begin,
	TIt end,
	size_t n,
	TPredicate &&predicate )
{
	using T = decltype( *begin );

	return n == 0u
		|| std::any_of(
			begin,
			end,
			[&n, &predicate] ( const T& element )
			{
				n -= predicate( element ) ? 1u : 0u;
				return n == 0u;
			} );
}

template<typename TContainer, typename TPredicate>
bool at_least_n_of_range( const TContainer &collection,
	size_t n,
	TPredicate &&predicate )
{
	return at_least_n_of_range( std::cbegin( collection ),
		std::cend( collection ),
		n,
		std::forward<TPredicate>( predicate ) );
}

template<typename TContainer, typename T>
unsigned index_of( const TContainer &collection,
	const T &val )
{
	const auto begin = std::begin( collection );
	const auto end = std::end( collection );

	return std::distance( begin,
		std::find( begin, end, val ) );
}

template<typename TContainer, typename T, typename TInserter>
bool insert_unique( const TContainer &collection,
	T &&val,
	TInserter &&inserter )
{
	if ( !collection_contains( collection, val ) )
	{
		inserter( std::forward<T>( val ) );
		return true;
	}
	return false;
}

//This could be implemented in terms of insert_unique, but given how simple it is, it doesn't seem worth the lambda overhead
template<typename TContainer, typename T>
bool emplace_back_unique( TContainer &collection,
	T &&val )
{
	if ( !collection_contains( collection, val ) )
	{
		collection.emplace_back( std::forward<T>( val ) );
		return true;
	}
	return false;
}

template<typename TContainer, typename TPredicate, typename... TArgs>
bool emplace_back_if_none_of( TContainer &collection,
	TPredicate &&predicate,
	TArgs&&... args )
{
	if ( none_of( collection, std::forward<TPredicate>( predicate ) ) )
	{
		collection.emplace_back( std::forward<TArgs>( args )... );
		return true;
	}
	return false;
}

template<typename TContainer, typename TPredicate, typename... TArgs>
typename TContainer::iterator emplace_back_unique_and_return( TContainer &collection,
	TPredicate &&predicate,
	TArgs&&... args )
{
	typename TContainer::value_type* result = try_find_if( collection,
		std::forward<TPredicate>( predicate ) );
	if ( result == nullptr )
	{
		return &collection.emplace_back( std::forward<TArgs>( args )... );

	}
	return result;
}

template<typename TContainer, typename TPredicate>
typename TContainer::iterator partition( TContainer &collection,
	TPredicate &&predicate )
{
	return std::partition( std::begin( collection ),
		std::end( collection ),
		std::forward<TPredicate>( predicate ) );
}

template<typename TContainer, typename TPredicate>
void move_erase_if( TContainer &src,
	TContainer &dest,
	TPredicate &&predicate )
{
	erase_if( src,
		[&dest, &predicate] ( typename TContainer::reference element )
		{
			if ( predicate( element ) )
			{
				dest.emplace_back( std::move( element ) );
				return true;
			}
			return false;
		} );
}

template<typename TSrcCollection, typename TDestCollection, typename TF>
void transform_emplace_back( TSrcCollection &src,
	TDestCollection &dest,
	TF &&func )
{
	for ( typename TSrcCollection::reference element : src )
	{
		dest.emplace_back( func( element ) );
	}
}

template<typename TSrcCollection, typename TDestCollection, typename TF>
void transform_emplace_back( const TSrcCollection &src,
	TDestCollection &dest,
	TF &&func )
{
	for ( typename TSrcCollection::const_reference element : src )
	{
		dest.emplace_back( func( element ) );
	}
}

template<typename TSrcCollection, typename TDestCollection, typename TPredicate, typename TF>
void transform_emplace_back_if( const TSrcCollection &src,
	TDestCollection &dest,
	TPredicate &&predicate,
	TF &&func )
{
	for ( typename TSrcCollection::const_reference element : src )
	{
		if ( predicate( element ) )
		{
			dest.emplace_back( func( element ) );
		}
	}
}

template<typename TSrcCollection, typename TDestCollection, typename TPredicate, typename TF>
void transform_emplace_back_if( TSrcCollection &src,
	TDestCollection &dest,
	TPredicate &&predicate,
	TF &&func )
{
	for ( typename TSrcCollection::reference element : src )
	{
		if ( predicate( element ) )
		{
			dest.emplace_back( func( element ) );
		}
	}
}

template<typename TContainer, typename TF>
void transform_emplace_back_clear( TContainer& src, TContainer& dest, TF&& func )
{
	transform_emplace_back( src,
		dest,
		std::forward<TF>( func ) );
	src.clear();
}

template<typename TContainer, typename TComparator>
bool hasDuplicates( const TContainer &collection, const TComparator &comparator )
{
	using TIt = typename TContainer::const_iterator;
	using T = typename TContainer::value_type;

	bool is_unique = true;

	for ( TIt end = std::end( collection ), itr1 = std::begin( collection ); is_unique && itr1 != end; itr1 = std::next( itr1 ) )
	{
		//If the type contained in the collection is a pointer or a number we pass everything by value
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
bool hasDuplicates( const TContainer &collection )
{
	return hasDuplicates( collection,
		std::equal_to() );
}


}//namespace util