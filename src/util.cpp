// STL
#include <iostream>
#include <string>

// Local
#include "util.hpp"

namespace util {

	void Info(const std::wstring &msg)  {
        std::wcout << INFO_PREFIX << msg << L'\n';
    }

    void OK(const std::wstring &msg) {
        std::wcout << OK_PREFIX << msg << L'\n';
    }

    [[maybe_unused]] static void Warn(const std::wstring &msg) {
        std::wcout << WARN_PREFIX << &msg << L'\n';
    }

    [[maybe_unused]] static void Error(const std::wstring &msg) {
        std::wcerr << FAIL_PREFIX << msg << L'\n';
    }

	void Header() {
		std::wcout << L"----------------------------------------\n"
		           << L" Large Address Aware Patcher\n"
		           << L"----------------------------------------\n\n";
	}
 
} // namespace util