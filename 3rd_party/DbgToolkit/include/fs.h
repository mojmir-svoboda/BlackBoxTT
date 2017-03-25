#pragma once
#if _MSC_VER < 1900
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;
#else
#include <filesystem>
namespace fs = std::tr2::sys;
#endif

