#pragma once
#include "Command.h"
#include <platform_win.h>
#include <memory>
#include <bbstring.h>
#include <bblib/codecvt.h>
struct DecodedCommand;

namespace bb {

  enum class E_CommandType : unsigned
  {
    e_bbcmd,
		//e_bb32wm_ack,
  };

	struct Command_bbcmd : Command
  {
    bbstring m_bbcmd;

		Command_bbcmd (char const * b, size_t ln)
		{
			size_t const sz = bb::codecvt_utf8_utf16_dst_size(b, ln);
			wchar_t * const bbcmd_u16 = static_cast<wchar_t *>(alloca(sz * sizeof(wchar_t)));
			size_t const bbcmd_u16_ln = bb::codecvt_utf8_utf16(b, ln, bbcmd_u16, sz);
			m_bbcmd = std::move(bbstring(bbcmd_u16, bbcmd_u16_ln));
		}
		virtual ~Command_bbcmd () { }
		virtual E_CommandType GetType () const override { return E_CommandType::e_bbcmd; }
  };

//   struct Command_bb32wm : Command
//   {
//     unsigned m_bb32wm;
// 
// 		Command_bb32wm (unsigned wm) : m_bb32wm(wm) {}
// 		virtual ~Command_bb32wm () { }
// 		virtual E_CommandType GetType () const override { return E_CommandType::e_bb32wm; }
//   };
// 
// 	struct Command_bb32wm_ack : Command
// 	{
// 		HANDLE m_hwnd;
// 
// 		Command_bb32wm_ack (HANDLE h) : m_hwnd(h) { }
// 		virtual ~Command_bb32wm_ack () { }
// 		virtual E_CommandType GetType () const override { return E_CommandType::e_bb32wm_ack; }
// 
// 		virtual size_t Encode (char * buff, size_t buffsz) override;
// 	};

	std::unique_ptr<Command> mkCommand (DecodedCommand const & cmd);
}
