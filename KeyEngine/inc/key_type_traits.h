#pragma once

#include <type_traits>


template<typename T>
struct is_string
	: public std::disjunction<
		std::is_same<char *, std::decay_t<T>>,
		std::is_same<const char *, std::decay_t<T>>,
		std::is_same<std::string, std::decay_t<T>>
	>
{};

template<typename T>
constexpr bool is_string_v = is_string<T>::value;

template<typename T>
using begin_function = decltype( std::declval<T>().begin() );

template<typename T>
using end_function = decltype( std::declval<T>().end() );

template<typename T>
using size_function = decltype( std::declval<T>().size() );

template<typename T>
using reserve_function = decltype( std::declval<T>().reserve( unsigned{} ) );

template<typename T>
using push_back_function = decltype( std::declval<T>().push_back( std::declval<typename T::value_type>() ) );

template<typename T>
using value_type = typename T::value_type;

template<typename T>
using iterator = typename T::iterator;

template<typename T>
using function_call_operator = decltype( &T::operator() ); // match any operator() with arbitrary arity and parameter types

template<typename T>
using eq_operator = decltype( std::declval<T>() == std::declval<T>() );

template<typename T>
using deref_operator = decltype( std::declval<T>().operator*() );

template<typename T>
using arrow_operator = decltype( std::declval<T>().operator->() );

template<typename T>
using get_function = std::is_pointer<decltype( std::declval<T>().get() )>;

// Note: it's possible that an iterator doesn't define one of this traits
// and iterator_traits are still valid in that case (std::iterator_traits is SFINAE-friendly)
// but for most cases this check is what we want, if needed we can add exceptions
template<typename T>
using all_iterator_traits = std::conjunction<typename std::iterator_traits<T>::difference_type,
	typename std::iterator_traits<T>::value_type,
	typename std::iterator_traits<T>::pointer,
	typename std::iterator_traits<T>::reference,
	typename std::iterator_traits<T>::iterator_category>;

template<typename T, template<typename> class ExpressionTemplate, typename = std::void_t<>>
struct implements
	: std::false_type
{};

template<typename T, template<typename> class ExpressionTemplate>
struct implements<T, ExpressionTemplate, std::void_t<ExpressionTemplate<T>>>
	: std::true_type
{};

template<typename T, template<typename> class ExpressionTemplate>
constexpr bool implements_v = implements<T, ExpressionTemplate>::value;

template<typename T>
using pointer_wrapper_traits = std::conjunction<deref_operator<T>, arrow_operator<T>, get_function<T>>;

template<typename T>
constexpr bool is_iterator_v = implements_v<T, all_iterator_traits>;

template<typename T>
constexpr bool is_container_v = implements_v<T, begin_function> && implements_v<T, end_function> && implements_v<T, size_function> && implements_v<T, T::value_type>;

template<typename T>
constexpr bool is_pointer_wrapper_v = implements_v<T, pointer_wrapper_traits>;

// check if some type T is a template type
template<typename T>
struct is_template
	: std::false_type
{};

// This will not work if T is a type that mix & matches non-type with typed template arguments (eg. std::array)
template<template<typename... Types> class T, typename... Types>
struct is_template<T<Types...>>
	: std::true_type
{};

// For 1 type + 1 non-type (eg std::array)
template<template<typename Type, auto NonType> class T, typename Type, auto NonType>
struct is_template<T<Type, NonType>>
	: std::true_type
{};

// A few other combos for completion
template<template<auto NonType, typename Type> class T, auto NonType, typename Type>
struct is_template<T<NonType, Type>>
	: std::true_type
{};

template<template<typename Type1, typename Type2, auto NonType> class T, typename Type1, typename Type2, auto NonType>
struct is_template<T<Type1, Type2, NonType>>
	: std::true_type
{};

template<template<auto NonType1, auto NonType2, typename Type> class T, auto NonType1, auto NonType2, typename Type>
struct is_template<T<NonType1, NonType2, Type>>
	: std::true_type
{};

template<template<typename Type1, auto NonType1, typename Type2> class T, typename Type1, auto NonType1, typename Type2>
struct is_template<T<Type1, NonType1, Type2>>
	: std::true_type
{};

template<template<auto NonType, typename Type1, typename Type2> class T, auto NonType, typename Type1, typename Type2>
struct is_template<T<NonType, Type1, Type2>>
	: std::true_type
{};

template<template<typename Type1, typename Type2, auto NonType1, auto NonType2> class T, typename Type1, typename Type2, auto NonType1, auto NonType2>
struct is_template<T<Type1, Type2, NonType1, NonType2>>
	: std::true_type
{};

template<typename T>
constexpr bool is_template_v = is_template<T>::value;

// check if some type T is an instance of a specific class template
template<typename TInstance, template<typename...> class T>
struct is_instance_of
	: public std::false_type
{};

template<template<typename...> class T, typename... TArgs>
struct is_instance_of<T<TArgs...>, T>
	: public std::true_type
{};

template<typename, typename, typename = std::void_t<>>
struct is_stream_insertable
	: std::false_type
{};

template<typename Stream, typename T>
struct is_stream_insertable<Stream, T>
	: public decltype( std::declval<Stream&>() << std::declval<T>(), std::true_type() )
{};

template<typename Stream, typename T>
constexpr bool is_stream_insertable_v = is_stream_insertable<Stream, T>::value;
