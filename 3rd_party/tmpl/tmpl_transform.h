#pragma once

template<template<class...> class F, class L>
struct tmpl_transform_impl;

template<template<class...> class F, class L>
using tmpl_transform = typename tmpl_transform_impl<F, L>::type;

template<template<class...> class F, template<class...> class L, class... T>
struct tmpl_transform_impl<F, L<T...>>
{
  using type = L<F<T>...>;
};

