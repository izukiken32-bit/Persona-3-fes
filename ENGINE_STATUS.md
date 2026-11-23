# Native engine status / roadmap

The project is building an Android native runtime for Persona 3 FES, targeting ARM Mali Android GPUs. The architecture is inspired by native recompilation ports, but the PS2 Emotion Engine and its system hardware require a dedicated implementation.

## Implemented source

- Android launcher, settings persistence, NDK/CMake and GitHub Actions APK workflow.
- 32 MiB EE RAM model and register/PC/HI/LO/cycle state.
- Initial MIPS integer instruction interpreter: arithmetic/logical operations, branches and jumps with delay slots, multiply/divide, and basic byte/halfword/word loads and stores.
- Bounds-checked ELF32 little-endian MIPS program-header loader for PT_LOAD segments, including zero-filled BSS.

## Not playable yet

The loader is not connected to a complete game boot path. A PS2 ELF cannot run by itself: the EE needs the PS2 BIOS/kernel environment and game-specific boot setup. Remaining major work includes accurate R5900/MMI and FPU behavior, COP0 exceptions and timers, scratchpad and hardware memory map, EE static recompilation, IOP CPU and services, GS graphics and Mali/Vulkan/OpenGL backend, VU0/VU1, SPU2, CD/DVD filesystem, movie playback, input, memory cards, and game lifecycle integration.

This repository is still an early engine bring-up, **not a playable port or tested in-game APK**. The workflow produces a debug launcher build only when CI succeeds. It does not include copyrighted game files; users must supply their own legally obtained game dump.
