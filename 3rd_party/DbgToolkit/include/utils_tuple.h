#pragma once
template<int... Is> struct seq { };
template<int N, int... Is> struct gen_seq : gen_seq<N - 1, N - 1, Is...> { };
template<int... Is> struct gen_seq<0, Is...> : seq<Is...> { };

template<int N, class T, class F>
void apply_nth (T & t, F f)
{
  f(std::get<N>(t));
}

template<class T, class F, int... Is>
void apply (T & t, int index, F f, seq<Is...>)
{
  using apply_fn_type = void (T &, F);
  static apply_fn_type const * const funcs[] = { &apply_nth<Is, T, F>... };
  apply_fn_type const * const nth = funcs[index];
  nth(t, f);
}

template<class T, class F>
void apply (T & t, int index, F f)
{
  apply(t, index, f, gen_seq<std::tuple_size<T>::value>{ });
}


