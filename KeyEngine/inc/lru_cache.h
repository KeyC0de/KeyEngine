#pragma once

#include <list>
#include <unordered_map>
#include <iostream>


//=============================================================
//	\class	LRUCache
//	\author	KeyC0de
//	\date	2022/02/22 23:34
//	\brief	A data structure which can store a max amount of key-value pairs
//			the list stores the data
//			to find them fast a hash table stores references to each value in the cache
//=============================================================
template<typename T>
class LRUCache
{
	std::list<T> m_list;
	std::unordered_map<T, typename std::list<T>::const_iterator> m_map;
	int m_maxSize;
public:
	LRUCache( int maxSize )
		:
		m_maxSize{maxSize}
	{

	}

	//===================================================
	//	\function	find
	//	\brief	refers element x with in the LRU cache
	//	\date	2022/07/30 21:08
	void find( T x )
	{
		if ( m_map.find( x ) == m_map.end() )
		{
			// not present in cache
			if ( m_list.size() == m_maxSize )
			{
				// cache is full so delete least recently used element
				T last = m_list.back();
				m_list.pop_back();
				m_map.erase( last );
			}
		}
		else
		{
			m_list.erase( m_map[x] );
		}

		// update reference
		m_list.push_front( x );
		m_map[x] = m_list.cbegin();
	}

	void display()
	{
		for ( auto it = m_list.cbegin(); it != m_list.cend(); ++it )
		{
			std::cout << ( *it ) << " ";
		}
		std::cout << std::endl;
	}
};