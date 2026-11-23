# Native engine status / roadmap
Architecture follows the native recompilation + host runtime pattern used by Brandon's Android projects, but PS2 FES requires a distinct MIPS R5900/Emotion Engine runtime.

Implemented starter: Android NDK/CMake target, EE state and 32 MiB RAM model, small MIPS instruction stepping subset, JNI probe.

Not implemented and required for boot/play: full R5900/MMI/FPU and timing/exceptions; game ELF loader/static recompiler; PS2 IOP/services; GS graphics renderer; VU0/VU1; SPU2 audio; CD/DVD and game filesystem; movie decoding; controller/touch, memory cards and lifecycle integration.

This is a foundation slice, not a playable engine or finished APK. No copyrighted game files are bundled.