# P3FES native engine roadmap

This repository is being developed toward the native-recompilation/runtime pattern used by Brandon's Android ports. P3FES is a PS2 title, so its target runtime is not the Switch runtime used by those projects.

## Added in this foundation commit
- Android NDK/CMake shared-library target
- EE CPU state, 32 MiB RAM model, RAM aliases and a small instruction-stepper
- JNI entry point to verify that the native runtime loads in Android

## Required before a game can boot
1. Complete EE/R5900 + MMI/FPU semantics, exception/interrupt and timing model
2. ELF/SLUS loader and static-recompilation pipeline for the USA P3FES executable
3. PS2 IOP subsystem and device services
4. GS renderer (Vulkan/OpenGL ES), texture/framebuffer handling
5. VU0/VU1 execution and microprogram support
6. SPU2 audio, CD/DVD and file-system services, ADPCM/ADX and movie playback
7. Android touch/controller mapping, save cards, suspend/resume and device profiling

The current CPU core is an instructional bring-up slice, not a compatibility-complete emulator. It cannot boot Persona 3 FES yet. The APK contains no game data; users supply their own legally obtained files locally.
