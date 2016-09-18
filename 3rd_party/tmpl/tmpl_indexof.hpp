#pragma once

template<class L, class V> struct tmpl_indexof_impl;

template<class L, class V>
using tmpl_indexof = typename tmpl_indexof_impl<L, V>::type;

template<template<class...> class L, class V>
struct tmpl_indexof_impl<L<>, V>
{
	using type = std::integral_constant<size_t, 0>;
};

template<template<class...> class L, class... T, class V>
struct tmpl_indexof_impl<L<V, T...>, V>
{
	using type = std::integral_constant<size_t, 0>;
};

template<template<class...> class L, class T1, class... T, class V>
struct tmpl_indexof_impl<L<T1, T...>, V>
{
	using type = std::integral_constant<size_t, 1 + tmpl_indexof<L<T...>, V>::value>;
};
