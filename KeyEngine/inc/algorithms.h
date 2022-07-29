#pragma once

#include <algorithm>
#include <iterator>
#include <vector>
#include <regex>


namespace util
{

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
void shrinkCapacity( std::vector<T, Alloc>& v )
{
	std::vector<T, Alloc>( v.begin(),
		v.end() ).swap( v );
}

//===================================================
//	\function	sizeAsInt
//	\brief  When using int loop indexes, use size_as_int(container) instead of container.size() in order
//			to document the inherent assumption that the size of the container can be represented by an int.
//	\date	2022/07/29 13:36
template <typename TContainer>
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

template <class InputIt, class OutputIt, class UnaryPredicate>
void move_if( InputIt srcFirst,
	InputIt srcLast,
	OutputIt *destFirst,
	UnaryPredicate predicate )
{
	std::copy_if( std::make_move_iterator( srcFirst ),
		std::make_move_iterator( srcLast ),
		std::back_inserter( *destFirst ),
		predicate );
}

//===================================================
//	\function	splitMovePartition
//	\brief  like partition_move - puts the second group of an std::partition to another container removing them from the source container
//	\date	2022/07/29 21:03
template<class Container, class Predicate>
void splitMovePartition( Container &src,
	Container &dest,
	Predicate p )
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

}//util