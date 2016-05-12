#pragma once

/*template <class T>
constexpr size_t array_count(T * t)
{
return sizeof(t) / sizeof(*t);
}*/
#ifndef array_count
# define array_count(s) (sizeof(s) / sizeof(*s))
#endif


