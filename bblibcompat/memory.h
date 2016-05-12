#pragma once

#ifdef BBOPT_MEMCHECK
# define m_alloc(s) _m_alloc(s,__FILE__,__LINE__)
# define c_alloc(s) _c_alloc(s,__FILE__,__LINE__)
# define m_realloc(p,s) _m_realloc(p,s,__FILE__,__LINE__)
# define m_free(p) _m_free(p,__FILE__,__LINE__)
#else
# define m_alloc(n) malloc(n)
# define c_alloc(n) calloc(1,n)
# define m_free(v) free(v)
# define m_realloc(p,s) realloc(p,s)
# define m_alloc_check_leaks(title)
# define m_alloc_check_memory()
# define m_alloc_size() 0
#endif /* BBOPT_MEMCHECK */

template <typename T>
T * c_new ()
{
  return static_cast<T * >(c_alloc(sizeof(T)));
}
//#define c_new(t) (t*)c_alloc(sizeof(t))
#define c_del(v) m_free(v)
