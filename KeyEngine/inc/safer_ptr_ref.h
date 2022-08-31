#pragma once

#include "assertions_console.h"


//=============================================================
//	\class	SaferPtr
//
//	\author	KeyC0de
//	\date	2022/08/28 12:41
//
//	\brief	non-owing ptr class, with just better semantics and type safety
//			similar to std::observer_ptr, or std::weak_ptr
//=============================================================
template<typename T>
class SaferPtr
{
protected:
	T *m_obj;
public:
	// non-const versions are NOT implicitly generated
	//SaferPtr( SaferPtr<T> &rhs ) = default;
	//SaferPtr& operator=( SaferPtr<T> &rhs ) = default;

	inline SaferPtr( T *rhs )
		:
		m_obj(rhs)
	{

	}

	inline SaferPtr& operator=( T *rhs )
	{
		m_obj = rhs;
		return *this;
	}

	inline const T*	operator->() const noexcept
	{
		return m_obj;
	}

	inline T* operator->() noexcept
	{
		return m_obj;
	}

	inline const T*	get() const noexcept
	{
		return m_obj;
	}

	inline T* get() noexcept
	{
		return m_obj;
	}

	inline const T&	operator*() const noexcept
	{
		return *m_obj;
	}

	inline T& operator*() noexcept
	{
		return *m_obj;
	}
};

template<typename T>
class SaferRef
{
	T &m_obj;
public:
	inline SaferRef( T &rhs )
		:
		m_obj{rhs}
	{
		ASSERT( ( &m_obj ) != nullptr, "Supplied object is null!" );
	}

	inline SaferRef( SaferRef<T> &rhs )
		:
		m_obj{rhs.m_obj}
	{

	}

	SaferRef<T>& operator=( const SaferRef<T> &rhs ) = delete;

	//===================================================
	//	\function	cctor
	//	\brief  explicit move constructor inhibits implicit move constructor, so we define it
	//	\date	2022/08/27 13:13
	inline SaferRef( SaferRef<T> &&rhs )
		:
		m_obj{rhs.m_obj}
	{

	}

	SaferRef<T>& operator=( const SaferRef<T> &&rhs ) = delete;

	inline T& operator()() noexcept
	{
		return m_obj;
	}

	inline const T operator()() const noexcept
	{
		return m_obj;
	}
};