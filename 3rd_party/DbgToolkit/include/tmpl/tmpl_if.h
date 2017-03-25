#pragma once

template<bool C, class T, class E> struct tmpl_if_c_impl;

template<class T, class E> struct tmpl_if_c_impl<true, T, E>
{
	using type = T;
};

template<class T, class E> struct tmpl_if_c_impl<false, T, E>
{
	using type = E;
};

template<bool C, class T, class E>
using tmpl_if_c = typename tmpl_if_c_impl<C, T, E>::type;

template<class C, class T, class E>
using tmpl_if = typename tmpl_if_c_impl<C::value != 0, T, E>::type;

