// STL
#include <iostream>
#include <span>
#include <string>

// Local
#include "patcher.hpp"
#include "util.hpp"

auto wmain(int argc, wchar_t *argv[]) -> int {
	const auto args = std::span(argv, static_cast<std::size_t>(argc));
	util::Header();

	if (args.size() < 2) {
		std::wcout << L"Usage:\n\n"
		           << L"  laa.exe <exe>\n"
		           << L"  laa.exe <exe> --check\n"
		           << L"  laa.exe <exe> --clear\n"
		           << L"  laa.exe <exe> --no-backup\n";

		return 0;
	}

	const std::wstring path = args[1];

    bool checkOnly = false;
    bool clearFlag = false;
    bool backup = true;

	for (std::size_t i = 2; i < args.size(); ++i) {
		const std::wstring arg = args[i];

		if (arg == L"--check") {
			checkOnly = true;

		} else if (arg == L"--clear") {
			clearFlag = true;

		} else if (arg == L"--no-backup") {
			backup = false;
		}
	}

	laa::Info info{};

	auto result = laa::Check(path, info);

	if (result != laa::Result::Success) {
		util::Error(laa::ResultToString(result));
		return 1;
	}

	std::wcout << L"File      : " << path << L'\n' << L"Architecture : "
	           << (info.is32Bit ? L"32-bit" : L"64-bit / Other") << L'\n' << L"LAA Flag  : "
	           << (info.laaEnabled ? L"Enabled" : L"Disabled") << L'\n' << L"Characteristics : 0x" << std::hex
	           << info.characteristics << std::dec << L"\n\n";

	if (checkOnly) {
		return 0;
	}

	result = laa::Set(path, !clearFlag, backup);

	if (result != laa::Result::Success) {
		util::Error(laa::ResultToString(result));
		return 1;
	}

	if (clearFlag) {
		util::OK(L"LAA flag cleared successfully.");
	} else {
		util::OK(L"LAA flag enabled successfully.");
	}

	if (backup) {
		util::Info(L"Backup created (.bak)");
	}

	return 0;
}