#if !defined(_YYCDLL_SCRIPT_CKDATAARRAY_H__IMPORTED_)
#define _YYCDLL_SCRIPT_CKDATAARRAY_H__IMPORTED_

#include "../stdafx.h"
#include <string>

namespace func_namespace {
	namespace script {
		namespace CKDataArray {

			BOOL ImportCsv(PluginInterface* plgif);
			BOOL ExportCsv(PluginInterface* plgif);
			BOOL Clean(PluginInterface* plgif);

			::CKDataArray* getEditingDataArray(PluginInterface* plgif);
			void getCsvFile(std::string* str, BOOL isOpen);
		}
	}
}

#endif