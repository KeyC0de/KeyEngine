#pragma once

#include <memory>
#include "non_copyable.h"


///=============================================================
/// \class	ObjectPool
/// \author	Nikos Lazaridis (KeyC0de)
/// \date	3-Oct-19
/// \brief	Pool Allocator
///=============================================================
template<typename T>
class ObjectPool final
	: public NonCopyable
{
	union Object
	{
		std::aligned_storage_t<sizeof( T ), alignof( T )> m_storage;
		Object *m_pNext;
	};

	std::unique_ptr<Object[]> m_pool;
	Object *m_pNextFree;
	std::size_t m_nObjs;
public:
	using value_type = T;
	using pointer = T*;

	/// \brief	ctor creates the pool given the amount of objects it will hold
	explicit ObjectPool( const std::size_t nObjs )
		:
		m_pool{std::make_unique<Object[]>( nObjs )},
		m_pNextFree{nullptr},
		m_nObjs(nObjs)
	{
		for ( int i = 1; i < nObjs; ++i )
		{
			m_pool[i - 1].m_pNext = &m_pool[i];
		}

		m_pNextFree = &m_pool[0];
	}

	~ObjectPool() noexcept = default;

	ObjectPool( ObjectPool &&rhs ) noexcept
		:
		m_pool{std::move( rhs.m_pool )},
		m_pNextFree{rhs.m_pNextFree},
		m_nObjs{rhs.getSize()}
	{
		rhs.m_pNextFree = nullptr;
	}

	ObjectPool& operator=( ObjectPool &&rhs ) noexcept
	{
		ObjectPool tmp{std::move( rhs )};
		std::swap( *this, tmp );
		return *this;
	}

	template <typename U>
	struct rebind
	{
		using otherAllocator = ObjectPool<U>;
	};

	T* address( T &r ) const noexcept
	{
		return &r;
	}

	const T* address( const T &r ) const noexcept
	{
		return &r;
	}

	// add object to the list
	[[nodiscard]]
	T* allocate()
	{
		if ( m_pNextFree == nullptr )
		{
			throw std::bad_alloc{};
		}

		const auto currentObj = m_pNextFree;
		m_pNextFree = currentObj->m_pNext;

		return reinterpret_cast<T*>( &currentObj->m_storage );
	}

	// remove object from the list
	void deallocate( T *p,
		[[maybe_unused]] std::size_t count = 0 ) noexcept
	{
		const auto o = reinterpret_cast<Object*>( p );
		o->m_pNext = m_pNextFree;
		m_pNextFree = o;
	}

	// pass ctor args
	template<typename... TArgs>
	[[nodiscard]]
	T* construct( TArgs... args )
	{
		return new ( allocate() ) T{std::forward<TArgs>( args )...};
	}

	void destroy( T *p ) noexcept
	{
		if ( p == nullptr )
		{
			return;
		}

		p->~T();
		deallocate( p );
	}

	std::size_t getSize() noexcept
	{
		return m_nObjs;
	}
};

template <class T, class Other>
bool operator==( const ObjectPool<T>& lhs,
	const ObjectPool<Other>& rhs ) noexcept
{
	return lhs.m_pool == rhs.m_pool;
}

template <class T, class Other>
bool operator!=( const ObjectPool<T>& lhs,
	const ObjectPool<Other>& rhs ) noexcept
{
	return lhs.m_pool != rhs.m_pool;
}