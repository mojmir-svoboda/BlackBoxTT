#pragma once

#ifdef blackbox_EXPORTS
#	define BB_API __declspec(dllexport)
#else
#	define BB_API __declspec(dllimport)
#endif


