#pragma once

#include "../../stdafx.h"
#include <string>

namespace bvh {
	namespace utils {
		namespace string_helper {

			// Inline functions should be implemented in header
			// Implements references
			// https://www.boost.org/doc/libs/1_39_0/boost/algorithm/string/predicate.hpp
			inline bool StdstringStartsWith(const std::string& source, const char* needle) {
				auto it = source.begin();
				const char* pit = needle;

				for (;
					it != source.end() && *pit != '\0';
					++it, ++pit) {
					if (*it != *pit) return false;
				}

				return (*pit == '\0');
			}
			inline bool StdstringStartsWith(const std::string& source, const std::string& needle) {
				return StdstringStartsWith(source, needle.c_str());
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
