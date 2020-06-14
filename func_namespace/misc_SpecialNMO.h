#if !defined(_YYCDLL_MISC_SPECIALNMO_H__IMPORTED_)
#define _YYCDLL_MISC_SPECIALNMO_H__IMPORTED_

#include "../stdafx.h"
#include <filesystem>

namespace func_namespace {
	namespace misc {
		namespace SpecialNMO {
			BOOL SaveSpecialNMO(PluginInterface* plgif);

			void getNmoFile(std::filesystem::path* str);
		}
	}
}

#endif