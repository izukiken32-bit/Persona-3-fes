#include "ee_core.hpp"
#include <cstring>

namespace p3fes {
EeCore::EeCore() { reset(); }
bool EeCore::reset() {
    state_ = EeState{};
    error_.clear();
    return true;
}
bool EeCore::read32(std::uint32_t address, std::uint32_t& value) const {
    // EE RAM mirror: physical 0x00000000..0x01ffffff, cached/uncached aliases.
    const auto physical = address & 0x1fffffffu;
    if (physical > kRamBytes - sizeof(value)) return false;
    std::memcpy(&value, ram_.data() + physical, sizeof(value));
    return true;
}
bool EeCore::write32(std::uint32_t address, std::uint32_t value) {
    const auto physical = address & 0x1fffffffu;
    if (physical > kRamBytes - sizeof(value)) return false;
    std::memcpy(ram_.data() + physical, &value, sizeof(value));
    return true;
}
bool EeCore::load_words(std::uint32_t address, const std::uint32_t* words, std::size_t count) {
    if (!words || (address & 3u) || count > kRamBytes / 4) {
        error_ = "invalid word buffer or unaligned address"; return false;
    }
    for (std::size_t i=0; i<count; ++i)
        if (!write32(address + static_cast<std::uint32_t>(i*4), words[i])) {
            error_ = "word buffer exceeds EE RAM"; return false;
        }
    state_.pc = address; state_.next_pc = address + 4;
    return true;
}
bool EeCore::step() {
    std::uint32_t op{};
    if (!read32(state_.pc, op)) { error_ = "instruction fetch outside EE RAM"; return false; }
    const auto old_pc = state_.pc;
    state_.pc = state_.next_pc; state_.next_pc += 4;
    const auto rs=(op>>21)&31u, rt=(op>>16)&31u, rd=(op>>11)&31u;
    const auto imm=static_cast<std::int16_t>(op&0xffffu);
    switch (op>>26) {
      case 0: // SPECIAL
        switch (op&63u) {
          case 0x00: state_.gpr[rd]=state_.gpr[rt]<<(op>>6&31u); break; // SLL
          case 0x02: state_.gpr[rd]=state_.gpr[rt]>>(op>>6&31u); break; // SRL
          case 0x08: state_.next_pc=static_cast<std::uint32_t>(state_.gpr[rs]); break; // JR
          case 0x21: state_.gpr[rd]=state_.gpr[rs]+state_.gpr[rt]; break; // ADDU
          case 0x23: state_.gpr[rd]=state_.gpr[rs]-state_.gpr[rt]; break; // SUBU
          case 0x24: state_.gpr[rd]=state_.gpr[rs]&state_.gpr[rt]; break;
          case 0x25: state_.gpr[rd]=state_.gpr[rs]|state_.gpr[rt]; break;
          case 0x2b: state_.gpr[rd]=(state_.gpr[rs]<state_.gpr[rt]); break;
          default: error_="unsupported EE SPECIAL opcode"; return false;
        } break;
      case 0x09: state_.gpr[rt]=state_.gpr[rs]+imm; break; // ADDIU
      case 0x0c: state_.gpr[rt]=state_.gpr[rs]&(op&0xffffu); break; // ANDI
      case 0x0d: state_.gpr[rt]=state_.gpr[rs]|(op&0xffffu); break; // ORI
      case 0x0f: state_.gpr[rt]=std::uint64_t(op&0xffffu)<<16; break; // LUI
      case 0x23: { std::uint32_t v; if(!read32(static_cast<std::uint32_t>(state_.gpr[rs]+imm),v)){error_="LW address fault";return false;} state_.gpr[rt]=static_cast<std::int32_t>(v); break; }
      case 0x2b: if(!write32(static_cast<std::uint32_t>(state_.gpr[rs]+imm),static_cast<std::uint32_t>(state_.gpr[rt]))){error_="SW address fault";return false;} break;
      case 0x04: if(state_.gpr[rs]==state_.gpr[rt]) state_.next_pc=old_pc+4+(std::int32_t(imm)<<2); break;
      case 0x05: if(state_.gpr[rs]!=state_.gpr[rt]) state_.next_pc=old_pc+4+(std::int32_t(imm)<<2); break;
      default: error_="unsupported EE primary opcode"; return false;
    }
    state_.gpr[0]=0; ++state_.cycles; return true;
}
}
