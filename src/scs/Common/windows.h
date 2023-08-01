#pragma once
#pragma execution_character_set("utf-8")

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
#endif

namespace scs {
	void SetConsoleEncoding();
}
