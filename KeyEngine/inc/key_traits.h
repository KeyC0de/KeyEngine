#pragma once

#include <type_traits>


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
using all_iterator_traits = std::conjunction<
    typename std::iterator_traits<T>::difference_type,
    typename std::iterator_traits<T>::value_type,
    typename std::iterator_traits<T>::pointer,
    typename std::iterator_traits<T>::reference,
    typename std::iterator_traits<T>::iterator_category>;

template<typename T, template<typename> typename ExpressionTemplate, typename = std::void_t<>>
struct implements
    : std::false_type
{};

template<typename T, template<typename> typename ExpressionTemplate>
struct implements<T, ExpressionTemplate, std::void_t<ExpressionTemplate<T>>>
    : std::true_type
{};

template<typename T, template<typename> typename ExpressionTemplate>
constexpr bool implements_v = implements<T, ExpressionTemplate>::value;

template<typename T>
using pointer_wrapper_traits = std::conjunction<deref_operator<T>, arrow_operator<T>, get_function<T>>;

template<typename T>
constexpr bool is_iterator_v = implements_v<T, all_iterator_traits>;

template<typename T>
constexpr bool is_container_v = implements_v<T, begin_function> && implements_v<T, end_function> && implements_v<T, size_function> && implements_v<T, T::value_type>;

template<typename T>
constexpr bool is_pointer_wrapper_v = implements_v<T, pointer_wrapper_traits>;

// check if some type is an instance of a specific template
template<typename Instance_t, template<typename...> typename Template_k>
constexpr bool is_instance_of_v = false;

template<template<typename...> typename Template_k, typename... Template_type_parameters_t>
constexpr bool is_instance_of_v<Template_k<Template_type_parameters_t...>, Template_k> = true;
