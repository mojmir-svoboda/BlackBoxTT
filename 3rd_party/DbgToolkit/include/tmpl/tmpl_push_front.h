#pragma once

template<class L, class T> struct tmpl_push_front_impl;

template<template<class...> class L, class... U, class T>
struct tmpl_push_front_impl<L<U...>, T>
{
  using type = L<T, U...>;
};

template<class L, class T>
using tmpl_push_front = typename tmpl_push_front_impl<L, T>::type;
