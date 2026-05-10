#ifndef PATCHER_HPP
#define PATCHER_HPP

// STL
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <utility>

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
	constexpr WORD LAA_FLAG = IMAGE_FILE_LARGE_ADDRESS_AWARE;

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

	inline auto GetNTHeaders(std::vector<std::byte> &data) -> IMAGE_NT_HEADERS32 * {
		if (data.size() < sizeof(IMAGE_DOS_HEADER)) {
			return nullptr;
		}

		auto *dos = reinterpret_cast<IMAGE_DOS_HEADER *>(data.data());

		if (dos->e_magic != IMAGE_DOS_SIGNATURE) {
			return nullptr;
		}

		auto *image_nt = reinterpret_cast<IMAGE_NT_HEADERS32 *>(data.data() + dos->e_lfanew);

		if (image_nt->Signature != IMAGE_NT_SIGNATURE) {
			return nullptr;
		}

		return image_nt;
	}

	inline auto ReadFile(const std::wstring &path, std::vector<std::byte> &outData) -> Result {
		if (!std::filesystem::exists(path)) {
			return Result::FileNotFound;
		}

		std::ifstream file(path, std::ios::binary);

		if (!file) {
			return Result::ReadFailed;
		}

		file.seekg(0, std::ios::end);

		const auto size = file.tellg();

		file.seekg(0, std::ios::beg);

		outData.resize(static_cast<size_t>(size));

		file.read(reinterpret_cast<char *>(outData.data()), static_cast<std::streamsize>(size));

		return Result::Success;
	}

	inline auto Check(const std::wstring &path, Info &outInfo) -> Result {
		std::vector<std::byte> data;

		auto result = ReadFile(path, data);

		if (result != Result::Success) {
			return result;
		}

		auto *image_nt = GetNTHeaders(data);

		if (image_nt == nullptr) {
			return Result::InvalidPE;
		}

		outInfo.is32Bit = image_nt->FileHeader.Machine == IMAGE_FILE_MACHINE_I386;

		outInfo.characteristics = image_nt->FileHeader.Characteristics;

		outInfo.laaEnabled = (outInfo.characteristics & LAA_FLAG) != 0;

		return Result::Success;
	}

	inline auto Set(const std::wstring &path, bool enable, bool backup = true) -> Result {
		std::vector<std::byte> data;

		auto result = ReadFile(path, data);

		if (result != Result::Success) {
			return result;
		}

		auto *image_nt = GetNTHeaders(data);

		if (image_nt == nullptr) {
			return Result::InvalidPE;
		}

		if (image_nt->FileHeader.Machine != IMAGE_FILE_MACHINE_I386) {
			return Result::Not32Bit;
		}

		auto &chars = image_nt->FileHeader.Characteristics;

		const bool alreadyEnabled = (chars & LAA_FLAG) != 0;

		if (enable && alreadyEnabled) {
			return Result::AlreadyEnabled;
		}

		if (!enable && !alreadyEnabled) {
			return Result::AlreadyDisabled;
		}

		if (enable) {
			chars |= LAA_FLAG;
		} else {
			chars &= ~LAA_FLAG;
		}

		if (backup) {
			std::filesystem::copy_file(path, path + L".bak", std::filesystem::copy_options::overwrite_existing);
		}

		std::ofstream file(path, std::ios::binary | std::ios::trunc);

		if (!file) {
			return Result::WriteFailed;
		}

		file.write(reinterpret_cast<const char *>(data.data()), static_cast<std::streamsize>(data.size()));

		return Result::Success;
	}

	inline auto ResultToString(Result result) -> std::wstring {
		switch (result) {
		case Result::Success:
			return L"Success";

		case Result::FileNotFound:
			return L"File not found";

		case Result::ReadFailed:
			return L"Failed to read file";

		case Result::WriteFailed:
			return L"Failed to write file";

		case Result::InvalidPE:
			return L"Invalid PE executable";

		case Result::Not32Bit:
			return L"Executable is not 32-bit";

		case Result::AlreadyEnabled:
			return L"LAA already enabled";

		case Result::AlreadyDisabled:
			return L"LAA already disabled";

		default:
			std::unreachable();
		}
	}
} // namespace laa

#endif // PATCHER_HPP