#pragma once

#include <iostream>
#include "key_type_traits.h"


#if defined _MSC_VER || defined _WIN32 || defined _WIN64
// __declspec( restrict ): indicates that a symbol is not aliased in the current scope, this semantic is propagated by the compiler
#	define restricted __declspec( restrict )
// __declspec( noalias ): declares that the function does not modify memory outside the first level of indirection from the function's parameters. That is, the parameters are the only reference to the outside world the function has.
#	define noaliasing __declspec( noalias )
#elif defined __unix__ || defined __unix || defined __APPLE__ && defined __MACH__
#	define restricted __restrict__
#	define noaliasing __restrict__
#endif

#define FORCE_CRASH	int *var = nullptr;\
	*var = 0xDEADBEEF;
#define IS_OF_TYPE( obj, T )				( dynamic_cast<T*>( obj ) != nullptr ) ? true : false
#define SAFE_CALL( obj, function )			{ if ( obj ) { obj.function; } }
#define SAFE_CALL_POINTER( obj, function )	{ if ( obj ) { obj->function; } }

///===================================================
/// \macro	ALIAS_FUNCTION
/// \brief	optimal way of renaming/aliasing a function to another - can be inline & no unnecessary copies
/// \date	2022/08/29 13:33
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
	private:
// use it like so: `CLASS_NAMER( MyClassName );` as the first statement of your class

#define ASSERT_VALID_OBJECT_POINTER(ptr, ExpectedType) \
	if (ptr != nullptr && dynamic_cast<ExpectedType*>(ptr) == nullptr)\
	{\
		__debugbreak();\
	}///===ASSERT_VALID_OBJECT_POINTER_POTENTIALLY_NULL

#define ASSERT_VALID_OBJECT_POINTER_NEVER_NULL(ptr, ExpectedType) \
	if (ptr == nullptr || (ptr != nullptr && dynamic_cast<ExpectedType*>(ptr) == nullptr))\
	{\
		__debugbreak();\
	}

template<typename T>
constexpr typename std::remove_reference<T>::type makePrValue( T &&val )
{
	return val;
}

// checks whether an expression e can be evaluated at compile-time as a constant expression
#define IS_CONSTEXPR(e)	noexcept( makePrValue(e) )
// eg.
// constexpr int a = 10;
// int b = 20;
// constexpr int constexprFunc(int x) { return x * 2; }
// static_assert(IS_CONSTEXPR(a), "a is not constexpr!");					// This will pass
// static_assert(!IS_CONSTEXPR(b), "b is not constexpr!");					// This will pass because b is not constexpr
// static_assert(constexprFunc(5) == 10, "constexprFunc is not constexpr");	// This will pass


template<typename T, typename TDeleter>
bool operator==( const std::unique_ptr<T, TDeleter> &lhs,
	const T *rhs )
{
	return lhs.get() == rhs;
}

template<typename T, typename TDeleter>
bool operator!=( const std::unique_ptr<T, TDeleter> &lhs,
	const T *rhs )
{
	return !( lhs == rhs );
}