#include "mem.hpp"
#include "windows.h"
#include <print>
#include <global.hpp>

namespace mem {
	using namespace global;
	byte_patch::byte_patch() {}; // do nothing

	byte_patch::byte_patch(uintptr_t base, std::vector<uint8_t> patch) {

		modbase = base;
		if (!modbase) return;


		std::string msg = std::format("byte_patch: patching {} bytes at {:x}\n", patch.size(), modbase);
		ConColorMsg({183, 255, 250, 255}, msg.c_str());

		DWORD prot;
		if (!VirtualProtect(reinterpret_cast<LPVOID>(base), patch.size(), PAGE_EXECUTE_READWRITE, &prot)) return;

		original.resize(patch.size());
		std::memcpy(original.data(), reinterpret_cast<void*>(base), patch.size());

		std::memcpy(reinterpret_cast<void*>(base), patch.data(), patch.size());

		if (!VirtualProtect(reinterpret_cast<void*>(base), patch.size(), prot, &prot)) return;

	}

	byte_patch::~byte_patch() {
		DWORD prot;

		if (!modbase || original.size() == 0) return;
		if (IsBadReadPtr(reinterpret_cast<void*>(modbase), original.size())) return;
		VirtualProtect(reinterpret_cast<LPVOID>(modbase), original.size(), PAGE_EXECUTE_READWRITE, &prot);
		std::memcpy(reinterpret_cast<LPVOID>(modbase), original.data(), original.size());
		VirtualProtect(reinterpret_cast<LPVOID>(modbase), original.size(), prot, &prot);
	}

	void patch_bytes(uintptr_t base, std::span<uint8_t> patch){
		uintptr_t modbase = base;
		if (!modbase) return;

		std::string msg = std::format("patch_bytes: patching {} bytes at {:x}\n", patch.size(), modbase);
		ConColorMsg({183, 255, 250, 255}, msg.c_str());
		DWORD prot;
		if (!VirtualProtect(reinterpret_cast<LPVOID>(base), patch.size(), PAGE_EXECUTE_READWRITE, &prot)) 
			return;

		std::memcpy(reinterpret_cast<void*>(base), patch.data(), patch.size());

		if (!VirtualProtect(reinterpret_cast<void*>(base), patch.size(), prot, &prot)) return;

	}

}