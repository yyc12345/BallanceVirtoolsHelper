#pragma once

#include "../../stdafx.h"
#include "error_proc.h"
#include "config_manager.h"
#include <string>

namespace bvh {
	namespace utils {
		class ParamPackage {
		public:
			PluginInterface* plgif;
			ErrorProc* error_proc;
			config_manager* cfg_manager;

			ParamPackage(PluginInterface* _plgif, ErrorProc* _error_proc, config_manager* _cfg_manager);
			~ParamPackage();
		};
	}
}