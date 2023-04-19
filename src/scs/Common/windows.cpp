#pragma execution_character_set("utf-8")

#ifdef _WIN32
#include <windows.h>
#endif

namespace scs {

	void SetConsoleEncoding() {
	#ifdef _WIN32
			SetConsoleOutputCP(65001);
	#endif
	}

}
