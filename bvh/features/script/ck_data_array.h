#pragma once

#include "../../../stdafx.h"
#include "../../utils/param_package.h"
#include <string>

namespace bvh {
	namespace features {
		namespace script {
			namespace ck_data_array {

				void ImportCsv(utils::ParamPackage* pkg);
				void ExportCsv(utils::ParamPackage* pkg);
				void Clean(utils::ParamPackage* pkg);

				::CKDataArray* getEditingDataArray(PluginInterface* plgif);
			}
		}
	}
}
