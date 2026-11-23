#include "ee_core.hpp"
#include "ps2_elf_loader.hpp"
#include <array>
#include <cstring>
#include <cassert>
#include <cstdint>

int main() {
    p3fes::EeCore cpu;
    // ADDIU t0,zero,42; ADDIU t1,zero,10; ADDU t2,t0,t1;
    // SW t2,0x100(zero); LW t3,0x100(zero); BEQ t3,t2,1;
    // ADDIU t4,zero,1 (delay slot); ADDIU t4,zero,2 (branch target)
    const std::uint32_t program[] = {
        0x2408002a, 0x2409000a, 0x01095021, 0xac0a0100,
        0x8c0b0100, 0x116a0001, 0x240c0001, 0x240c0002
    };
    assert(cpu.load_words(0x1000, program, sizeof(program)/sizeof(program[0])));
    for (int i=0;i<8;i++) assert(cpu.step());
    const auto& s=cpu.state();
    assert(s.gpr[8]==42 && s.gpr[9]==10 && s.gpr[10]==52 && s.gpr[11]==52);
    assert(s.gpr[12]==2); // delay slot and taken branch are both observed
    assert(s.cycles==8);
    // Minimal ELF32 MIPS PT_LOAD fixture validates segment and entry setup.
    std::array<std::uint8_t, 88> elf{};
    const auto put16=[&](std::size_t o,std::uint16_t v){elf[o]=v&0xff;elf[o+1]=(v>>8)&0xff;};
    const auto put32=[&](std::size_t o,std::uint32_t v){for(int j=0;j<4;++j)elf[o+j]=(v>>(j*8))&0xff;};
    elf[0]=0x7f; elf[1]='E'; elf[2]='L'; elf[3]='F'; elf[4]=1; elf[5]=1;
    put16(16,2); put16(18,8); put32(24,0x1000); put32(28,52);
    put16(40,52); put16(42,32); put16(44,1);
    put32(52,1); put32(56,84); put32(60,0x1000); put32(64,0x1000);
    put32(68,4); put32(72,8); put32(76,5); put32(80,4);
    put32(84,0x24080007); // addiu t0,zero,7
    p3fes::EeCore elfCpu;
    const auto result=p3fes::Ps2ElfLoader::load(elfCpu,elf);
    assert(result.ok && result.entry==0x1000 && result.loaded_segments==1);
    assert(elfCpu.state().pc==0x1000);
    assert(elfCpu.step() && elfCpu.state().gpr[8]==7);
    assert(!p3fes::Ps2ElfLoader::load(elfCpu,std::span<const std::uint8_t>{}).ok);
    return 0;
}
