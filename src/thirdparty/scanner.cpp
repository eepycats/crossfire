//===========================================================================//
//
// Author: NULLderef
// Purpose: Portal 2: Multiplayer Mod server plugin
// 
//===========================================================================//

#include "scanner.hpp"

#include <Windows.h>
#include <immintrin.h>
#include <stdexcept>
#include <sstream>

namespace Memory {
#ifndef _WIN32
	inline void __cpuidex(int cpuid[4], int function, int subleaf) {
		asm volatile("cpuid"
			: "=a" (cpuid[0]),
			"=b" (cpuid[1]),
			"=c" (cpuid[2]),
			"=d" (cpuid[3])
			: "0" (function), "2" (subleaf)
		);
	}
#endif // _WIN32

	enum MaskState : uint8_t {
		MASK_EMPTY = 0x00,
		MASK_FULL = 0xFF,
	};

	class ScanData {
	public:
		ScanData(std::string patternString) : locatorFirst(0), locatorLast(0) {
			std::istringstream patternStream(patternString);
			std::string patternByte;

			while (patternStream >> patternByte) {
				if (patternByte == "?" || patternByte == "??") {
					this->pattern.push_back(0x00);
					this->mask.push_back(MASK_EMPTY);
				}
				else {
					this->pattern.push_back(static_cast<uint8_t>(std::stoul(patternByte, nullptr, 16)));
					this->mask.push_back(MASK_FULL);
				}
			}

			for (size_t first = 0; first < this->mask.size() - 1; first++) {
				if (this->mask[first] == MASK_FULL) {
					this->locatorFirst = first;
					break;
				}
			}

			for (size_t last = this->mask.size() - 1; last > 0; last--) {
				if (this->mask[last] == MASK_FULL) {
					this->locatorLast = last;
					break;
				}
			}

			if (this->mask[this->locatorFirst] == MASK_EMPTY || this->mask[this->locatorLast] == MASK_EMPTY) {
				throw std::runtime_error("Unable to find locating bytes (mask may be too loose)");
			}
		}

		std::vector<uint8_t> pattern;
		std::vector<uint8_t> mask;
		size_t locatorFirst;
		size_t locatorLast;
	};

	// implementation: AVX

	class AVXScanner : public ScannerImplementation {
	public:
		uintptr_t Scan(std::span<uint8_t> region, std::string patternString, int offset) {
			ScanData scanData(patternString);

			const __m256i locatorFirstMask = _mm256_set1_epi8(static_cast<uint8_t>(scanData.pattern[scanData.locatorFirst]));
			const __m256i locatorLastMask = _mm256_set1_epi8(static_cast<uint8_t>(scanData.pattern[scanData.locatorLast]));

			for (size_t blockOffset = 0; blockOffset < region.size() - scanData.pattern.size() - sizeof(__m256i); blockOffset += sizeof(__m256i)) {
				const __m256i scanFirstBlock = _mm256_loadu_si256(reinterpret_cast<__m256i*>(&region[blockOffset + scanData.locatorFirst]));
				const __m256i scanLastBlock = _mm256_loadu_si256(reinterpret_cast<__m256i*>(&region[blockOffset + scanData.locatorLast]));

				const uint32_t comparedMask = _mm256_movemask_epi8(_mm256_and_si256(
					_mm256_cmpeq_epi8(scanFirstBlock, locatorFirstMask),
					_mm256_cmpeq_epi8(scanLastBlock, locatorLastMask)
				));

				if (comparedMask != 0) {
					// WANING: this for loop needs the count of *bits* and not *bytes*
					for (size_t bitPosition = 0; bitPosition < sizeof(comparedMask) * 8; bitPosition++) {
						if ((comparedMask & (1 << bitPosition)) != 0) {
							if (this->InnerCompare(std::span(&region[blockOffset + bitPosition], scanData.pattern.size()), scanData)) {
								return reinterpret_cast<uintptr_t>(&region[blockOffset + bitPosition + offset]);
							}
						}
					}
				}

			}

			throw std::runtime_error("Unable to find signature");
		}

		std::vector<uintptr_t> ScanMultiple(std::span<uint8_t> region, std::string patternString, int offset) {
			ScanData scanData(patternString);

			std::vector<uintptr_t> matches;

			const __m256i locatorFirstMask = _mm256_set1_epi8(static_cast<uint8_t>(scanData.pattern[scanData.locatorFirst]));
			const __m256i locatorLastMask = _mm256_set1_epi8(static_cast<uint8_t>(scanData.pattern[scanData.locatorLast]));

			for (size_t blockOffset = 0; blockOffset < region.size() - scanData.pattern.size() - sizeof(__m256i); blockOffset += sizeof(__m256i)) {
				const __m256i scanFirstBlock = _mm256_loadu_si256(reinterpret_cast<__m256i*>(&region[blockOffset + scanData.locatorFirst]));
				const __m256i scanLastBlock = _mm256_loadu_si256(reinterpret_cast<__m256i*>(&region[blockOffset + scanData.locatorLast]));

				const uint32_t comparedMask = _mm256_movemask_epi8(_mm256_and_si256(
					_mm256_cmpeq_epi8(scanFirstBlock, locatorFirstMask),
					_mm256_cmpeq_epi8(scanLastBlock, locatorLastMask)
				));

				if (comparedMask != 0) {
					// WANING: this for loop needs the count of *bits* and not *bytes*
					for (size_t bitPosition = 0; bitPosition < sizeof(comparedMask) * 8; bitPosition++) {
						if ((comparedMask & (1 << bitPosition)) != 0) {
							if (this->InnerCompare(std::span(&region[blockOffset + bitPosition], scanData.pattern.size()), scanData)) {
								matches.push_back(reinterpret_cast<uintptr_t>(&region[blockOffset + bitPosition + offset]));
							}
						}
					}
				}
			}

			return matches;
		}

	private:
		inline bool InnerCompare(std::span<uint8_t> region, ScanData& scanData) {
			size_t lastAligned = (region.size() & ~(sizeof(__m256i) - 1));
			for (size_t offset = 0; offset < lastAligned; offset += sizeof(__m256i)) {
				if (
					~_mm256_movemask_epi8(
						_mm256_cmpeq_epi8(
							_mm256_loadu_si256(reinterpret_cast<__m256i*>(&region[offset])),
							_mm256_loadu_si256(reinterpret_cast<__m256i*>(&scanData.pattern[offset]))
						)
					)
					&
					_mm256_movemask_epi8(
						_mm256_loadu_si256(reinterpret_cast<__m256i*>(&scanData.mask[offset]))
					)
					) {
					return false;
				}
			}

			for (size_t offset = lastAligned; offset < region.size(); offset++) {
				if (region[offset] != scanData.pattern[offset] && scanData.mask[offset] == MASK_FULL) {
					return false;
				}
			}

			return true;
		}
	};

	// implementation: SSE

	class SSEScanner : public ScannerImplementation {
	public:
		uintptr_t Scan(std::span<uint8_t> region, std::string patternString, int offset) {
			ScanData scanData(patternString);

			const __m128i locatorFirstMask = _mm_set1_epi8(static_cast<uint8_t>(scanData.pattern[scanData.locatorFirst]));
			const __m128i locatorLastMask = _mm_set1_epi8(static_cast<uint8_t>(scanData.pattern[scanData.locatorLast]));

			for (size_t blockOffset = 0; blockOffset < region.size() - scanData.pattern.size() - sizeof(__m128i); blockOffset += sizeof(__m128i)) {
				const __m128i scanFirstBlock = _mm_loadu_si128(reinterpret_cast<__m128i*>(&region[blockOffset + scanData.locatorFirst]));
				const __m128i scanLastBlock = _mm_loadu_si128(reinterpret_cast<__m128i*>(&region[blockOffset + scanData.locatorLast]));

				const uint16_t comparedMask = _mm_movemask_epi8(_mm_and_si128(
					_mm_cmpeq_epi8(scanFirstBlock, locatorFirstMask),
					_mm_cmpeq_epi8(scanLastBlock, locatorLastMask)
				));

				if (comparedMask != 0) {
					// WANING: this for loop needs the count of *bits* and not *bytes*
					for (size_t bitPosition = 0; bitPosition < sizeof(comparedMask) * 8; bitPosition++) {
						if ((comparedMask & (1 << bitPosition)) != 0) {
							if (this->InnerCompare(std::span(&region[blockOffset + bitPosition], scanData.pattern.size()), scanData)) {
								return reinterpret_cast<uintptr_t>(&region[blockOffset + bitPosition + offset]);
							}
						}
					}
				}
			}

			throw std::runtime_error("Unable to find signature");
		}

		std::vector<uintptr_t> ScanMultiple(std::span<uint8_t> region, std::string patternString, int offset) {
			ScanData scanData(patternString);

			std::vector<uintptr_t> matches;

			const __m128i locatorFirstMask = _mm_set1_epi8(static_cast<uint8_t>(scanData.pattern[scanData.locatorFirst]));
			const __m128i locatorLastMask = _mm_set1_epi8(static_cast<uint8_t>(scanData.pattern[scanData.locatorLast]));

			for (size_t blockOffset = 0; blockOffset < region.size() - scanData.pattern.size() - sizeof(__m128i); blockOffset += sizeof(__m128i)) {
				const __m128i scanFirstBlock = _mm_loadu_si128(reinterpret_cast<__m128i*>(&region[blockOffset + scanData.locatorFirst]));
				const __m128i scanLastBlock = _mm_loadu_si128(reinterpret_cast<__m128i*>(&region[blockOffset + scanData.locatorLast]));

				const uint16_t comparedMask = _mm_movemask_epi8(_mm_and_si128(
					_mm_cmpeq_epi8(scanFirstBlock, locatorFirstMask),
					_mm_cmpeq_epi8(scanLastBlock, locatorLastMask)
				));

				if (comparedMask != 0) {
					// WANING: this for loop needs the count of *bits* and not *bytes*
					for (size_t bitPosition = 0; bitPosition < sizeof(comparedMask) * 8; bitPosition++) {
						if ((comparedMask & (1 << bitPosition)) != 0) {
							if (this->InnerCompare(std::span(&region[blockOffset + bitPosition], scanData.pattern.size()), scanData)) {
								matches.push_back(reinterpret_cast<uintptr_t>(&region[blockOffset + bitPosition + offset]));
							}
						}
					}
				}
			}

			return matches;
		}

	private:
		inline bool InnerCompare(std::span<uint8_t> region, ScanData& scanData) {
			size_t lastAligned = (region.size() & ~(sizeof(__m128i) - 1));
			for (size_t offset = 0; offset < lastAligned; offset += sizeof(__m128i)) {
				if (
					~_mm_movemask_epi8(
						_mm_cmpeq_epi8(
							_mm_loadu_si128(reinterpret_cast<__m128i*>(&region[offset])),
							_mm_loadu_si128(reinterpret_cast<__m128i*>(&scanData.pattern[offset]))
						)
					)
					&
					_mm_movemask_epi8(
						_mm_loadu_si128(reinterpret_cast<__m128i*>(&scanData.mask[offset]))
					)
					) {
					return false;
				}
			}

			for (size_t offset = lastAligned; offset < region.size(); offset++) {
				if (region[offset] != scanData.pattern[offset] && scanData.mask[offset] == MASK_FULL) {
					return false;
				}
			}

			return true;
		}
	};

	// implementation: generic

	class GenericScanner : public ScannerImplementation {
	public:
		uintptr_t Scan(std::span<uint8_t> region, std::string patternString, int offset) {
			ScanData scanData(patternString);

			for (size_t blockOffset = 0; blockOffset < region.size() - scanData.pattern.size(); blockOffset++) {
				if (
					region[blockOffset + scanData.locatorFirst] == scanData.pattern[scanData.locatorFirst] &&
					region[blockOffset + scanData.locatorLast] == scanData.pattern[scanData.locatorLast]
					) {
					if (this->InnerCompare(std::span(&region[blockOffset], scanData.pattern.size()), scanData)) {
						return reinterpret_cast<uintptr_t>(&region[blockOffset + offset]);
					}
				}
			}

			throw std::runtime_error("Unable to find signature");
		}

		std::vector<uintptr_t> ScanMultiple(std::span<uint8_t> region, std::string patternString, int offset) {
			ScanData scanData(patternString);

			std::vector<uintptr_t> matches;

			for (size_t blockOffset = 0; blockOffset < region.size() - scanData.pattern.size(); blockOffset++) {
				if (
					region[blockOffset + scanData.locatorFirst] == scanData.pattern[scanData.locatorFirst] &&
					region[blockOffset + scanData.locatorLast] == scanData.pattern[scanData.locatorLast]
					) {
					if (this->InnerCompare(std::span(&region[blockOffset], scanData.pattern.size()), scanData)) {
						matches.push_back(reinterpret_cast<uintptr_t>(&region[blockOffset + offset]));
					}
				}
			}

			throw std::runtime_error("Unable to find signature");
		}

	private:
		inline bool InnerCompare(std::span<uint8_t> region, ScanData& scanData) {
			for (size_t offset = 0; offset < region.size(); offset++) {
				if (region[offset] != scanData.pattern[offset] && scanData.mask[offset] == MASK_FULL) {
					return false;
				}
			}

			return true;
		}
	};

	// common

	std::unique_ptr<ScannerImplementation>& Scanner::Implementation() {
		static std::unique_ptr<ScannerImplementation> implementation;

		if (implementation == nullptr) {
			int cpuid[4];

			__cpuidex(cpuid, 0, 0);

			int ncpuids = cpuid[0];
			if(ncpuids >= 7) {
				__cpuidex(cpuid, 7, 0);
				if (cpuid[1] & (1 << 5)) {
					implementation = std::make_unique<AVXScanner>();
				} else {
					implementation = std::make_unique<SSEScanner>();
				}
			} else {
				__cpuidex(cpuid, 1, 0);
				if(cpuid[3] & (1 << 26)) {
					implementation = std::make_unique<SSEScanner>();
				} else {
					implementation = std::make_unique<GenericScanner>();
				}
			}
		}

		return implementation;
	}
};
