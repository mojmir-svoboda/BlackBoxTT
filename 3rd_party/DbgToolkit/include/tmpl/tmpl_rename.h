#pragma once
template<class A, template<class...> class B>
struct tmpl_rename_impl;

template<template<class...> class A, class... T, template<class...> class B>
struct tmpl_rename_impl<A<T...>, B>
{
  using type = B<T...>;
};

template<class A, template<class...> class B>
using tmpl_rename = typename tmpl_rename_impl<A, B>::type;

