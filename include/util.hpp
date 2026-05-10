#ifndef UTIL_HPP
#define UTIL_HPP

// STL
#include <iostream>
#include <string>

namespace util {

	// Constants
	constexpr auto INFO_PREFIX = L"[INFO] ";
	constexpr auto OK_PREFIX   = L"[ OK ] ";
	constexpr auto WARN_PREFIX = L"[WARN] ";
	constexpr auto FAIL_PREFIX = L"[FAIL] ";

	inline void Info(const std::wstring &msg)  { std::wcout << INFO_PREFIX << msg << L'\n'; }
	inline void OK(const std::wstring &msg)    { std::wcout << OK_PREFIX   << msg << L'\n'; }
	inline void Warn(const std::wstring &msg)  { std::wcout << WARN_PREFIX << msg << L'\n'; }
	inline void Error(const std::wstring &msg) { std::wcerr << FAIL_PREFIX << msg << L'\n'; }

	inline void Header() {
		std::wcout << L"----------------------------------------\n"
				<< L" Large Address Aware Patcher\n"
				<< L"----------------------------------------\n\n";
	}
} // namespace util

#endif // UTIL_HPP