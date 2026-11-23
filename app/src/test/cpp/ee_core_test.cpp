#include "ee_core.hpp"
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
    return 0;
}
