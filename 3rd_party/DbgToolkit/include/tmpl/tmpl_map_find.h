#pragma once
template<class M, class K> struct tmpl_map_find_impl;

template<class M, class K>
using tmpl_map_find = typename tmpl_map_find_impl<M, K>::type;

template<template<class...> class M, class... T, class K>
struct tmpl_map_find_impl<M<T...>, K>
{
  struct U : std::identity<T>... { };

  template<template<class...> class L, class... U>
  static std::identity<L<K, U...>> f( std::identity<L<K, U...>>* );
  static std::identity<void> f( ... );

  using V = decltype( f((U*)0) );
  using type = typename V::type;
};

