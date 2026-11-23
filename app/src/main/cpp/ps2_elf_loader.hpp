#pragma once
#include "ee_core.hpp"
#include <cstddef>
#include <cstdint>
#include <span>

namespace p3fes {
struct ElfLoadResult {
    bool ok{};
    std::uint32_t entry{};
    std::uint32_t loaded_segments{};
    const char* error{};
};
class Ps2ElfLoader {
public:
    // Loads ELF32 little-endian MIPS PT_LOAD segments into EE RAM.
    // This is a bare executable loader; it does not initialize the PS2 kernel/IOP.
    static ElfLoadResult load(EeCore& core, std::span<const std::uint8_t> image);
};
}
