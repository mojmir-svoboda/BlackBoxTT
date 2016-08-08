#pragma once
#include "SchemeConfig.h"
#include <memory>

struct s7_scheme;

namespace bb {
  
  struct Scheme
  {
		SchemeConfig m_config;
		s7_scheme * m_scheme;

		Scheme ();
		~Scheme ();
		bool Init (SchemeConfig const & cfg);
		bool Done ();

		bool Eval (char const * str, char * resp, size_t resp_sz);
  };
}
