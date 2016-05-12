#pragma once
#include <bbstring.h>

namespace bb {

  struct PluginConfig
  {
		bbstring m_name;  /// display name as in the menu
		bbstring m_path;  /// as in plugins.rc

		bool m_enabled;
		bool m_isSlit;
		bool m_inSlit;

		PluginConfig ()
			: m_enabled(false)
      , m_isSlit(false)
			, m_inSlit(false)
		{ }
  };

}
