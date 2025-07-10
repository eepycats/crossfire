//===========================================================================//
//
// Author: NULLderef
// Purpose: Portal 2: Multiplayer Mod server plugin
// 
//===========================================================================//

#ifndef SCANNER_HPP
#define SCANNER_HPP

#include <span>
#include <string>
#include <vector>
#include <memory>

namespace Memory {
	class ScannerImplementation {
	public:
		virtual uintptr_t Scan(std::span<uint8_t> region, std::string pattern, int offset) = 0;
		virtual std::vector<uintptr_t> ScanMultiple(std::span<uint8_t> region, std::string pattern, int offset) = 0;
	};

	class Scanner {
	public:
		template<typename T = uintptr_t> static T scan(std::span<uint8_t> region, std::string pattern, int offset = 0) {
			return reinterpret_cast<T>(Scanner::Implementation().get()->Scan(region, pattern, offset));
		}

		static uintptr_t scan(std::span<uint8_t> region, std::string pattern, int offset = 0) {
			return Scanner::Implementation().get()->Scan(region, pattern, offset);
		}

		static std::vector<uintptr_t> scan_multiple(std::span<uint8_t> region, std::string pattern, int offset = 0) {
			return Scanner::Implementation().get()->ScanMultiple(region, pattern, offset);
		}

	private:
		static std::unique_ptr<ScannerImplementation>& Implementation();
	};
};

#endif // SCANNER_HPP
