#include <jni.h>
#include "ee_core.hpp"
#include <memory>
namespace { std::unique_ptr<p3fes::EeCore> core; }
extern "C" JNIEXPORT jstring JNICALL
Java_com_izukiken_p3fes_MainActivity_nativeRuntimeStatus(JNIEnv* env, jobject) {
    if (!core) core=std::make_unique<p3fes::EeCore>();
    const char* status="P3FES native runtime: EE core foundation loaded; PS2 devices/GS/VU/SPU and game loader are not yet implemented.";
    return env->NewStringUTF(status);
}
