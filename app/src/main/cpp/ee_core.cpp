#include "ee_core.hpp"
#include <cstring>
#include <limits>

namespace p3fes {
EeCore::EeCore() { reset(); }

void EeCore::reset() {
    s_ = {};
    s_.pc = 0xBFC00000u;
    s_.next_pc = s_.pc + 4;
    error_ = "";
}

bool EeCore::read8(std::uint32_t a, std::uint8_t& v) const {
    a &= 0x1fffffffu;
    if (a >= kRamBytes) return false;
    v = ram_[a]; return true;
}
bool EeCore::read16(std::uint32_t a, std::uint16_t& v) const {
    if (a & 1u) return false;
    a &= 0x1fffffffu;
    if (a > kRamBytes - 2) return false;
    std::memcpy(&v, ram_.data() + a, 2); return true;
}
bool EeCore::read32(std::uint32_t a, std::uint32_t& v) const {
    if (a & 3u) return false;
    a &= 0x1fffffffu;
    if (a > kRamBytes - 4) return false;
    std::memcpy(&v, ram_.data() + a, 4); return true;
}
bool EeCore::write8(std::uint32_t a, std::uint8_t v) {
    a &= 0x1fffffffu;
    if (a >= kRamBytes) return false;
    ram_[a] = v; return true;
}
bool EeCore::write16(std::uint32_t a, std::uint16_t v) {
    if (a & 1u) return false;
    a &= 0x1fffffffu;
    if (a > kRamBytes - 2) return false;
    std::memcpy(ram_.data() + a, &v, 2); return true;
}
bool EeCore::write32(std::uint32_t a, std::uint32_t v) {
    if (a & 3u) return false;
    a &= 0x1fffffffu;
    if (a > kRamBytes - 4) return false;
    std::memcpy(ram_.data() + a, &v, 4); return true;
}

bool EeCore::load_words(std::uint32_t a, const std::uint32_t* words, std::size_t n) {
    if (!words || (a & 3u) || n > kRamBytes / 4 || a > 0x1fffffffu ||
        std::uint64_t(a & 0x1fffffffu) + n * 4 > kRamBytes) {
        error_ = "invalid or out-of-range ELF/test load"; return false;
    }
    for (std::size_t i = 0; i < n; ++i)
        if (!write32(a + std::uint32_t(i * 4), words[i])) {
            error_ = "RAM write fault"; return false;
        }
    s_.pc = a; s_.next_pc = a + 4; error_ = "";
    return true;
}

bool EeCore::step() {
    std::uint32_t ins{};
    if (!read32(s_.pc, ins)) { error_ = "instruction fetch/alignment fault"; return false; }
    const std::uint32_t at = s_.pc;
    s_.pc = s_.next_pc; s_.next_pc += 4; // branches execute one delay-slot instruction
    const unsigned op = ins >> 26, rs = (ins >> 21) & 31, rt = (ins >> 16) & 31;
    const unsigned rd = (ins >> 11) & 31, sh = (ins >> 6) & 31, fn = ins & 63;
    const std::int32_t simm = static_cast<std::int16_t>(ins);
    const std::uint32_t uimm = ins & 0xffffu;
    const std::uint64_t a = s_.gpr[rs], b = s_.gpr[rt];
    const auto branch = [&](bool take) {
        if (take) s_.next_pc = at + 4u + (static_cast<std::uint32_t>(simm) << 2);
    };
    std::uint32_t v32{};
    std::uint16_t v16{};
    std::uint8_t v8{};
    switch (op) {
    case 0: // SPECIAL
        switch (fn) {
        case 0x00: s_.gpr[rd] = b << sh; break;
        case 0x02: s_.gpr[rd] = b >> sh; break;
        case 0x03: s_.gpr[rd] = static_cast<std::uint64_t>(static_cast<std::int64_t>(b) >> sh); break;
        case 0x04: s_.gpr[rd] = b << (a & 63); break;
        case 0x06: s_.gpr[rd] = b >> (a & 63); break;
        case 0x07: s_.gpr[rd] = static_cast<std::uint64_t>(static_cast<std::int64_t>(b) >> (a & 63)); break;
        case 0x08: s_.next_pc = static_cast<std::uint32_t>(a); break;
        case 0x09: s_.gpr[rd ? rd : 31] = s_.pc + 4; s_.next_pc = static_cast<std::uint32_t>(a); break;
        case 0x0a: if (!rt) s_.gpr[rd] = a; break;
        case 0x0b: if (rt && a) s_.gpr[rd] = a; break;
        case 0x10: s_.gpr[rd] = s_.hi; break;
        case 0x11: s_.hi = b; break;
        case 0x12: s_.gpr[rd] = s_.lo; break;
        case 0x13: s_.lo = b; break;
        case 0x18: { const __int128 p = static_cast<__int128>(static_cast<std::int64_t>(a)) * static_cast<std::int64_t>(b); s_.lo = static_cast<std::uint64_t>(p); s_.hi = static_cast<std::uint64_t>(p >> 64); break; }
        case 0x19: { const unsigned __int128 p = static_cast<unsigned __int128>(a) * b; s_.lo = static_cast<std::uint64_t>(p); s_.hi = static_cast<std::uint64_t>(p >> 64); break; }
        case 0x1a: if (static_cast<std::int64_t>(b) != 0) { s_.lo = static_cast<std::uint64_t>(static_cast<std::int64_t>(a) / static_cast<std::int64_t>(b)); s_.hi = static_cast<std::uint64_t>(static_cast<std::int64_t>(a) % static_cast<std::int64_t>(b)); } else { s_.lo = static_cast<std::int64_t>(a) < 0 ? 1u : ~0ull; s_.hi = a; } break;
        case 0x1b: if (b) { s_.lo = a / b; s_.hi = a % b; } else { s_.lo = ~0ull; s_.hi = a; } break;
        case 0x20: case 0x21: s_.gpr[rd] = a + b; break;
        case 0x22: case 0x23: s_.gpr[rd] = a - b; break;
        case 0x24: s_.gpr[rd] = a & b; break;
        case 0x25: s_.gpr[rd] = a | b; break;
        case 0x26: s_.gpr[rd] = a ^ b; break;
        case 0x27: s_.gpr[rd] = ~(a | b); break;
        case 0x2a: s_.gpr[rd] = static_cast<std::int64_t>(a) < static_cast<std::int64_t>(b); break;
        case 0x2b: s_.gpr[rd] = a < b; break;
        default: error_ = "unsupported SPECIAL/MMI instruction"; return false;
        } break;
    case 2: case 3: { const std::uint32_t target = ((at + 4) & 0xf0000000u) | ((ins & 0x03ffffffu) << 2); if (op == 3) s_.gpr[31] = at + 8; s_.next_pc = target; break; }
    case 4: branch(a == b); break;
    case 5: branch(a != b); break;
    case 6: branch(static_cast<std::int64_t>(a) <= 0); break;
    case 7: branch(static_cast<std::int64_t>(a) > 0); break;
    case 8: case 9: s_.gpr[rt] = a + static_cast<std::int64_t>(simm); break;
    case 10: s_.gpr[rt] = static_cast<std::int64_t>(a) < simm; break;
    case 11: s_.gpr[rt] = a < static_cast<std::uint64_t>(static_cast<std::int64_t>(simm)); break;
    case 12: s_.gpr[rt] = a & uimm; break;
    case 13: s_.gpr[rt] = a | uimm; break;
    case 14: s_.gpr[rt] = a ^ uimm; break;
    case 15: s_.gpr[rt] = std::uint64_t(uimm) << 16; break;
    case 20: branch(a == b); break; // BEQL, approximation pending precise annul semantics
    case 21: branch(a != b); break;
    case 32: if (!read8(static_cast<std::uint32_t>(a + simm), v8)) { error_="LB fault"; return false; } s_.gpr[rt]=static_cast<std::int8_t>(v8); break;
    case 33: if (!read16(static_cast<std::uint32_t>(a + simm), v16)) { error_="LH fault"; return false; } s_.gpr[rt]=static_cast<std::int16_t>(v16); break;
    case 34: case 38: error_="LWL/LWR not implemented"; return false;
    case 35: if (!read32(static_cast<std::uint32_t>(a + simm), v32)) { error_="LW fault"; return false; } s_.gpr[rt]=static_cast<std::int32_t>(v32); break;
    case 36: if (!read8(static_cast<std::uint32_t>(a + simm), v8)) { error_="LBU fault"; return false; } s_.gpr[rt]=v8; break;
    case 37: if (!read16(static_cast<std::uint32_t>(a + simm), v16)) { error_="LHU fault"; return false; } s_.gpr[rt]=v16; break;
    case 40: if (!write8(static_cast<std::uint32_t>(a + simm), static_cast<std::uint8_t>(b))) { error_="SB fault"; return false; } break;
    case 41: if (!write16(static_cast<std::uint32_t>(a + simm), static_cast<std::uint16_t>(b))) { error_="SH fault"; return false; } break;
    case 42: case 46: error_="SWL/SWR not implemented"; return false;
    case 43: if (!write32(static_cast<std::uint32_t>(a + simm), static_cast<std::uint32_t>(b))) { error_="SW fault"; return false; } break;
    default: error_="unsupported opcode (COP0/EE-specific ops pending)"; return false;
    }
    s_.gpr[0] = 0; ++s_.cycles; error_ = ""; return true;
}
}
