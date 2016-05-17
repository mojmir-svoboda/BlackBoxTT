#pragma once
#include "Command.h"

namespace bb {

  enum class E_CommandType : unsigned
  {
    e_bb32wm,
  };

  struct Command_bb32wm : Command
  {
    unsigned m_bb32wm;

		Command_bb32wm (unsigned wm) : m_bb32wm(wm) {}
		virtual ~Command_bb32wm () { }
		virtual E_CommandType GetType () const override { return E_CommandType::e_bb32wm; }
  };
};
