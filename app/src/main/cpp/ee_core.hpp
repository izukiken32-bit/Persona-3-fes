#pragma once
#include <array>
#include <cstddef>
#include <cstdint>

namespace p3fes {
struct EeState {
    std::array<std::uint64_t, 32> gpr{};
    std::uint64_t hi{}, lo{}, cycles{};
    std::uint32_t pc{0xBFC00000u}, next_pc{0xBFC00004u};
};
class EeCore {
public:
    static constexpr std::size_t kRamBytes = 32u * 1024u * 1024u;
    EeCore();
    void reset();
    bool load_words(std::uint32_t address, const std::uint32_t* words, std::size_t count);\n    bool load_bytes(std::uint32_t address, const std::uint8_t* bytes, std::size_t count);
    bool step();
    const EeState& state() const { return s_; }
    const char* error() const { return error_; }
private:
    bool read8(std::uint32_t, std::uint8_t&) const;
    bool read16(std::uint32_t, std::uint16_t&) const;
    bool read32(std::uint32_t, std::uint32_t&) const;
    bool write8(std::uint32_t, std::uint8_t);
    bool write16(std::uint32_t, std::uint16_t);
    bool write32(std::uint32_t, std::uint32_t);
    EeState s_{};
    std::array<std::uint8_t, kRamBytes> ram_{};
    const char* error_ = "";
};
}
