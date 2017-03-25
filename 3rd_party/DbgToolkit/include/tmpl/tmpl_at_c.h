#pragma once

template<class L, std::size_t I> struct tmpl_at_c_impl;

template<class L, std::size_t N>
using tmpl_at_c = typename tmpl_at_c_impl<L, N>::type;

template<class L, class N>
using tmpl_at = typename tmpl_at_c_impl<L, N::value>::type;

template<template<class...> class L, class FirstT, class... RestT>
struct tmpl_at_c_impl<L<FirstT, RestT...>, 0>
{
  using type = FirstT;
};

template<template<class...> class L, class FirstT, class... RestT, std::size_t N>
struct tmpl_at_c_impl<L<FirstT, RestT...>, N>
{
  using type = tmpl_at_c<L<RestT...>, N - 1>;
};

