#include "scs/Common/windows.h"

namespace scs {

	void SetConsoleEncoding() {
		#ifdef _WIN32
			SetConsoleOutputCP(65001);
		#endif
	}

}
