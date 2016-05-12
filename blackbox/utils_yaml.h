#pragma once
#include <yaml-cpp/yaml.h>
#include <bbstring.h>
#include <codecvt.h>

namespace YAML {

	template<>
	struct convert<bbstring>
	{
		static Node encode (bbstring const & rhs)
		{
			Node node;
			try {
				std::string str;
				bb::codecvt_utf16_utf8(rhs, str);
				node.push_back(str);
			}
			catch (std::exception const & e)
			{
				return node;
			}
			return node;
		}

		static bool decode (Node const & node, bbstring & rhs)
		{
			try
			{
				bb::codecvt_utf8_utf16(node.as<std::string>(), rhs);
			}
			catch (std::exception const & e)
			{
				return false;
			}
			return true;
		}
	};
}



