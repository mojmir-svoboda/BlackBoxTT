#pragma once

//@NOTE: ehm, this is more like tmpl_sizeof

template<class L> struct tmpl_count_impl;

template<template<class...> class L, class... T>
struct tmpl_count_impl<L<T...>>
{
	using type = std::integral_constant<size_t, sizeof...(T)>;
};

template<class L>
using tmpl_count = typename tmpl_count_impl<L>::type;


