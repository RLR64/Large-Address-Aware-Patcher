#ifndef PATCHER_HPP
#define PATCHER_HPP

// STL
#include <string>
 
// Windows
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#ifdef _WIN64
    #define _AMD64_
#else
    #define _X86_
#endif

#include <minwindef.h>
#include <winnt.h>

namespace laa {

	inline constexpr WORD LAA_FLAG = IMAGE_FILE_LARGE_ADDRESS_AWARE;

	enum class Result {
		Success,
		FileNotFound,
		ReadFailed,
		WriteFailed,
		InvalidPE,
		Not32Bit,
		AlreadyEnabled,
		AlreadyDisabled
	};

	struct Info {
		bool is32Bit{};
		bool laaEnabled{};
		WORD characteristics{};
	};

	auto check(const std::wstring &path, Info &outInfo) -> Result;
	auto set(const std::wstring &path, bool enable, bool backup = true) -> Result;
	auto resultToString(Result result) -> std::wstring;

} // namespace laa

#endif // PATCHER_HPP