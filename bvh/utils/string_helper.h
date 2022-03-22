#pragma once

#include "../../stdafx.h"
#include <string>

namespace bvh {
	namespace utils {
		namespace string_helper {

			// Inline functions should be implemented in header
			// Implements references
			// https://github.com/AshampooSystems/boden/blob/master/framework/foundation/include/bdn/String.h
			// https://zh.cppreference.com/w/cpp/string/basic_string/find
			inline bool StdstringStartsWith(const std::string& source, const std::string& needle) {
				return source.length() >= needle.length() && source.find(needle) == 0;
			}
			inline bool StdstringStartsWith(const std::string& source, const char* needle) {
				return source.length() >= strlen(needle) && source.find(needle) == 0;
			}

			BOOL Stdstring2Uint(std::string* strl, UINT* number);
			void Uint2Stdstring(std::string* strl, UINT number);
			void StdstringPrintf(std::string* strl, const char* format, ...);
			void StdstringVPrintf(std::string* strl, const char* format, va_list argptr);
			void StdwstringPrintf(std::wstring* strl, const wchar_t* format, ...);

			void ConvertEncoding(std::string* orig, std::string* dest, UINT origCP, UINT destCP);
			BOOL String2Wstring(std::string* orig, std::wstring* dest, UINT cp);
			BOOL Wstring2String(std::wstring* orig, std::string* dest, UINT cp);
		}
	}
}
