#include "param_package.h"

namespace bvh {
	namespace utils {

		ParamPackage::ParamPackage(PluginInterface* _plgif, ErrorProc* _error_proc, ConfigManager* _cfg_manager) : 
		plgif(_plgif), error_proc(_error_proc), cfg_manager(_cfg_manager) {
			;
		}

		ParamPackage::~ParamPackage() {
			;
		}
	}
}

