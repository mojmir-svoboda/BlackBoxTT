#pragma once
#include <yaml-cpp/yaml.h>
#include <bbstring.h>
#include <codecvt.h>
#include <bblib/logging.h>

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
				TRACE_MSG(LL_ERROR, CTX_CONFIG, "YAML string encode exception: %s", e.what());
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
				TRACE_MSG(LL_ERROR, CTX_CONFIG, "YAML string decode exception: %s", e.what());
				return false;
			}
			return true;
		}
	};

// 	template<class T>
// 	struct convert<std::vector<std::unique_ptr<T>>>
// 	{
// 		static Node encode (std::vector<std::unique_ptr<T>> const & rhs)
// 		{
// 			Node node(NodeType::Sequence);
// 			for (typename std::vector<std::unique_ptr<T>>::const_iterator it = rhs.begin(); it != rhs.end(); ++it)
// 				node.push_back(**it);
// 			return node;
// 		}
// 
// 		static bool decode (Node const & node, std::vector<std::unique_ptr<T>> & rhs)
// 		{
// 			if (!node.IsSequence())
// 				return false;
// 
// 			rhs.clear();
// 			for (typename std::vector<std::unique_ptr<T>>::const_iterator it = node.begin(); it != node.end(); ++it)
// 			{
// 				T tmp = it->as<T>();
// 				std::unique_ptr<T> unique_tmp(new T(std::move(tmp)));
// 				rhs.push_back(std::move(unique_tmp));
// 			}
// 			return true;
// 		}
// 	};

}



