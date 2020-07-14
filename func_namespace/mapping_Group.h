#if !defined(_YYCDLL_MAPPING_GROUP_H__IMPORTED_)
#define _YYCDLL_MAPPING_GROUP_H__IMPORTED_

#include "../stdafx.h"

namespace func_namespace {
	namespace mapping {
		namespace Group {

			BOOL AutoGrouping();
			void GroupIntoWithCreation(CKContext* ctx, CK3dObject* obj, const char* groupName);

			BOOL GroupingChecker();

		}
	}
}

#endif