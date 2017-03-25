#pragma once
#include <utils_tuple.h>

template<int N, class T>
QVariant get_nth (T & t)
{
  return std::get<N>(t);
}

template<class T, int... Is>
QVariant tuple_get_nth (T & t, int index, seq<Is...>)
{
  using get_fn_prototype = QVariant (T &);
  static get_fn_prototype * const funcs[] = { &get_nth<Is, T>... };
  get_fn_prototype * const nth = funcs[index];
  return nth(t);
}

template<class T>
QVariant tuple_get_nth (T & t, int index)
{
  return tuple_get_nth(t, index, gen_seq<std::tuple_size<T>::value>{ });
}

