#pragma once
#include <vector>
#include <cstdint>
#include <span>
#include <Windows.h>
#include <Psapi.h>
#include "scanner.hpp"

namespace memdetail {
	template<size_t N>
	struct StringLiteral {
		constexpr StringLiteral(const char(&str)[N]) {
			std::copy_n(str, N, value);
		}

		char value[N];
	};

}
namespace mem {
	class byte_patch {
	public:
		byte_patch();
		byte_patch(uintptr_t modbase, std::vector<uint8_t> patch);
		~byte_patch();
	private:
		uintptr_t modbase = 0;
		std::vector<uint8_t> original{};
	};

	template <size_t idx, typename T, typename... Args>
	__forceinline T vcall(void* instance, Args... args)
	{
		using fn_t = T(__thiscall*)(void*, Args...);

		auto fn = (*reinterpret_cast<fn_t**>(instance))[idx];
		return fn(instance, args...);
	}
	inline std::span<std::uint8_t> entire_module(std::uintptr_t base) {
		MODULEINFO mi{};
		GetModuleInformation(GetCurrentProcess(), reinterpret_cast<HMODULE>(base), &mi, sizeof(mi));
		return std::span<std::uint8_t>(reinterpret_cast<std::uint8_t*>(base), mi.SizeOfImage);
	}

	inline std::span<std::uint8_t> module(std::string_view name) {
		MODULEINFO mi{};
		auto base = GetModuleHandleA(name.data());
		GetModuleInformation(GetCurrentProcess(), base, &mi, sizeof(mi));
		return std::span<std::uint8_t>(reinterpret_cast<std::uint8_t*>(base), mi.SizeOfImage);
	}

	using signature_scanner = Memory::Scanner;

	template <memdetail::StringLiteral module>
	std::uintptr_t rva(std::uintptr_t rva) {
		static HMODULE lib = GetModuleHandleA(module.value);
		auto ptr = reinterpret_cast<std::uint8_t*>(lib);
		ptr += rva;
		return reinterpret_cast<std::uintptr_t>(ptr);
	}

	template <memdetail::StringLiteral module, memdetail::StringLiteral exportname>
	std::uintptr_t modexport() {
		HMODULE lib = GetModuleHandleA(module.value);
		return reinterpret_cast<std::uintptr_t>(GetProcAddress(lib, exportname.value));
	}

	void patch_bytes(uintptr_t base, std::span<uint8_t> patch);
}
