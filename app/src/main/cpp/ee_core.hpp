#pragma once
#include <array>
#include <cstdint>
#include <string>

namespace p3fes {
// First runtime layer: deterministic EE state and instruction stepping.
// This is deliberately not advertised as a complete PS2 CPU or game runner.
struct EeState {
    std::array<std::uint64_t, 32> gpr{};
    std::uint64_t hi{}, lo{};
    std::uint32_t pc{0xBFC00000u};
    std::uint32_t next_pc{0xBFC00004u};
    std::uint64_t cycles{};
};
class EeCore {
public:
    static constexpr std::size_t kRamBytes = 32u * 1024u * 1024u;
    EeCore();
    bool reset();
    bool load_words(std::uint32_t address, const std::uint32_t* words, std::size_t count);
    bool step();
    const EeState& state() const { return state_; }
    const std::string& last_error() const { return error_; }
private:
    bool read32(std::uint32_t address, std::uint32_t& value) const;
    bool write32(std::uint32_t address, std::uint32_t value);
    EeState state_{};
    std::array<std::uint8_t, kRamBytes> ram_{};
    std::string error_;
};
}
