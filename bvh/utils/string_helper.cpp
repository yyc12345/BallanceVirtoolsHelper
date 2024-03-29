#include "string_helper.h"
#include <cstdarg>
#include <stdexcept>

namespace bvh {
	namespace utils {
		namespace string_helper {

			BOOL Stdstring2Uint(std::string* strl, UINT* number) {
				return (sscanf(strl->c_str(), "%u", number) > 0);
			}

			void Uint2Stdstring(std::string* strl, UINT number) {
				StdstringPrintf(strl, "%u", number);
			}

			void StdstringPrintf(std::string* strl, const char* format, ...) {
				va_list argptr;
				va_start(argptr, format);
				StdstringVPrintf(strl, format, argptr);
				va_end(argptr);
			}
			void StdstringVPrintf(std::string* strl, const char* format, va_list argptr) {
				int count = _vsnprintf(NULL, 0, format, argptr);
				count++;

				strl->resize(count);
				(*strl)[count - 1] = '\0';
				int write_result = _vsnprintf(strl->data(), count, format, argptr);

				if (write_result < 0 || write_result >= count) throw new std::length_error("Invalid write_result in _vsnprintf.");
			}

			void StdwstringPrintf(std::wstring* strl, const wchar_t* format, ...) {
				va_list argptr;
				va_start(argptr, format);
				int count = _vsnwprintf(NULL, 0, format, argptr);
				count++;
				va_end(argptr);

				strl->resize(count);
				(*strl)[count - 1] = L'\0';
				va_start(argptr, format);
				int write_result = _vsnwprintf(strl->data(), count, format, argptr);
				va_end(argptr);

				if (write_result < 0 || write_result >= count) throw new std::length_error("Invalid write_result in _vsnwprintf.");
			}


			void ConvertEncoding(std::string* orig, std::string* dest, UINT origCP, UINT destCP) {
				std::wstring wscache;

				if (origCP == destCP) {
					// do not need any convertion
					(*dest) = orig->c_str();
					return;
				}

				if (!String2Wstring(orig, &wscache, origCP)) {
					// fail to conv, fallback
					(*dest) = orig->c_str();
					return;
				}

				if (!Wstring2String(&wscache, dest, destCP)) {
					// fail to conv, fallback
					(*dest) = orig->c_str();
					return;
				}
			}

			BOOL String2Wstring(std::string* orig, std::wstring* dest, UINT cp) {
				int wcount, write_result;

				// convert to WCHAR
				wcount = MultiByteToWideChar(cp, 0, orig->c_str(), orig->size(), NULL, 0);
				if (wcount <= 0) return FALSE;
				dest->resize(wcount);
				write_result = MultiByteToWideChar(cp, 0, orig->c_str(), orig->size(), dest->data(), wcount);
				if (write_result <= 0) return FALSE;

				return TRUE;
			}

			BOOL Wstring2String(std::wstring* orig, std::string* dest, UINT cp) {
				int count, write_result;

				//converter to CHAR
				count = WideCharToMultiByte(cp, 0, orig->c_str(), orig->size(), NULL, 0, NULL, NULL);
				if (count <= 0) return FALSE;
				dest->resize(count);
				write_result = WideCharToMultiByte(cp, 0, orig->c_str(), orig->size(), dest->data(), count, NULL, NULL);
				if (write_result <= 0) return FALSE;

				return TRUE;
			}

		}
	}
}

