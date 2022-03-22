#pragma once

#include "../../../stdafx.h"
#include "../../utils/param_package.h"

namespace bvh {
	namespace features {
		namespace mapping {
			namespace grouping {

				void AutoGrouping(utils::ParamPackage* pkg);
				void groupIntoWithCreation(CKContext* ctx, CK3dObject* obj, const char* groupName);

				void GroupingChecker(utils::ParamPackage* pkg);

			}
		}
	}
}
