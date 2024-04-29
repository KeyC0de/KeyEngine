#pragma once

#include <functional>
#include <memory>
#include "non_copyable.h"


//===================================================
//	\function	Operation
//	\brief  returns a ready-made callable with its arguments, packaged in a std::unique_ptr
//			It can be inserted in any container or used for whatever purpose
//			utilizes std::function for type erasure and std::bind for argument packing
//	\date	2020/11/05 3:17
class Operation
{
	using TF = std::function<void()>;
	TF m_f;
private:
	Operation( TF &&f );
public:
	~Operation() noexcept;
	Operation( Operation &&rhs ) noexcept;
	Operation& operator=( Operation &&rhs ) noexcept;

	//	\function	setup	||	\date	2022/08/28 21:19
	//	\brief	setup function pointers
	template <typename TFunction, typename... TArgs>
	static std::unique_ptr<Operation> setup( TFunction &&f,
		TArgs &&...args )
	{
		return std::make_unique<Operation>( std::bind( std::forward<TFunction>( f ), std::forward<TArgs>( args )... ) );
	}
	// TRet( *f )( std::forward<TArgs>( args )... );

	//	\function	setup	||	\date	2022/08/28 21:19
	//	\brief	setup member function pointers
	template <typename TMethod, typename T, typename ...TArgs>
	static std::unique_ptr<Operation> setup( TMethod &&mf,
		T &&obj,
		TArgs &&...args )
	{
		return std::unique_ptr<Operation>( new Operation{std::bind( std::forward<TMethod>( mf ), std::forward<T>( obj ), std::forward<TArgs>( args )... )} );
	}
	//( std::forward<T>( obj ).*mf )( std::forward<TArgs>( args )... ); -> TRet

	void operator()() const;
	void swap( Operation &rhs ) noexcept;
	void reset() noexcept;

	inline operator bool() const noexcept;
	inline constexpr bool operator==( const Operation &rhs ) const noexcept;
	inline constexpr bool operator!=( const Operation &rhs ) const noexcept;
	inline constexpr bool operator==( const Operation *rhs ) const noexcept;
	inline constexpr bool operator!=( const Operation *rhs ) const noexcept;
};