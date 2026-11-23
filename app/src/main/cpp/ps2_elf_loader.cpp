#include "ps2_elf_loader.hpp"
#include <algorithm>
#include <array>
#include <cstring>

namespace p3fes {
namespace {
bool range(std::size_t offset, std::size_t size, std::size_t total) {
    return offset <= total && size <= total - offset;
}
std::uint16_t u16(const std::uint8_t* p) {
    return std::uint16_t(p[0]) | (std::uint16_t(p[1]) << 8);
}
std::uint32_t u32(const std::uint8_t* p) {
    return std::uint32_t(p[0]) | (std::uint32_t(p[1]) << 8) |
           (std::uint32_t(p[2]) << 16) | (std::uint32_t(p[3]) << 24);
}
}
ElfLoadResult Ps2ElfLoader::load(EeCore& core, std::span<const std::uint8_t> image) {
    auto fail=[](const char* message){ return ElfLoadResult{false,0,0,message}; };
    if (image.size() < 52 || image[0] != 0x7f || image[1] != 'E' ||
        image[2] != 'L' || image[3] != 'F') return fail("not an ELF image");
    if (image[4] != 1 || image[5] != 1) return fail("expected ELF32 little-endian");
    if (u16(image.data()+18) != 8) return fail("ELF machine is not MIPS");
    const auto type=u16(image.data()+16);
    if (type != 2 && type != 3) return fail("ELF is not executable/shared object");
    const std::uint32_t entry=u32(image.data()+24), phoff=u32(image.data()+28);
    const std::uint16_t phsize=u16(image.data()+42), phnum=u16(image.data()+44);
    if (phsize < 32 || !range(phoff,std::size_t(phsize)*phnum,image.size()))
        return fail("invalid ELF program-header table");
    std::uint32_t loaded=0;
    for (std::uint16_t i=0;i<phnum;++i) {
        const auto* ph=image.data()+phoff+std::size_t(i)*phsize;
        if (u32(ph) != 1) continue; // PT_LOAD only
        const std::uint32_t offset=u32(ph+4), vaddr=u32(ph+8), paddr=u32(ph+12);
        const std::uint32_t filesz=u32(ph+16), memsz=u32(ph+20);
        if (filesz > memsz || !range(offset,filesz,image.size()))
            return fail("invalid ELF load segment range");
        const std::uint32_t destination=(vaddr ? vaddr : paddr) & 0x1fffffffu;
        if (std::uint64_t(destination)+memsz > EeCore::kRamBytes)
            return fail("ELF segment exceeds 32 MiB EE RAM");
        if (!core.load_bytes(destination,image.data()+offset,filesz))
            return fail(core.error());
        std::array<std::uint8_t,4096> zero{};
        std::uint32_t remaining=memsz-filesz, at=destination+filesz;
        while (remaining) {
            const auto n=std::min<std::uint32_t>(remaining,zero.size());
            if (!core.load_bytes(at,zero.data(),n)) return fail(core.error());
            at+=n; remaining-=n;
        }
        ++loaded;
    }
    if (!loaded) return fail("ELF contains no loadable segments");
    return {true,entry,loaded,"ELF segments loaded; PS2 system boot services are still required"};
}
}
