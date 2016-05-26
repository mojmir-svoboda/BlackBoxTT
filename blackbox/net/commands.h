#pragma once
#include "Command.h"
#include <platform_win.h>
#include <memory>
struct DecodedCommand;

namespace bb {

  enum class E_CommandType : unsigned
  {
    e_bb32wm,
		e_bb32wm_ack,
  };

  struct Command_bb32wm : Command
  {
    unsigned m_bb32wm;

		Command_bb32wm (unsigned wm) : m_bb32wm(wm) {}
		virtual ~Command_bb32wm () { }
		virtual E_CommandType GetType () const override { return E_CommandType::e_bb32wm; }
  };

	struct Command_bb32wm_ack : Command
	{
		HANDLE m_hwnd;

		Command_bb32wm_ack (HANDLE h) : m_hwnd(h) { }
		virtual ~Command_bb32wm_ack () { }
		virtual E_CommandType GetType () const override { return E_CommandType::e_bb32wm_ack; }

		virtual size_t Encode (char * buff, size_t buffsz) override;
	};

	std::unique_ptr<Command> mkCommand (DecodedCommand const & cmd);
}
