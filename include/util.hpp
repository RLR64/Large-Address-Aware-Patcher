#ifndef UTIL_HPP
#define UTIL_HPP

// STL
#include <string>

namespace util {

	inline constexpr auto INFO_PREFIX = L"[INFO] ";
	inline constexpr auto OK_PREFIX   = L"[ OK ] ";
	inline constexpr auto WARN_PREFIX = L"[WARN] ";
	inline constexpr auto FAIL_PREFIX = L"[FAIL] ";

	void Info(const std::wstring &msg);
	void OK(const std::wstring &msg);
	void Warn();
	void Error();
	void Header();

} // namespace util

#endif // UTIL_HPP