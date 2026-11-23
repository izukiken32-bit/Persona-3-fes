#include <jni.h>
#include "ee_core.hpp"
#include <memory>
static std::unique_ptr<p3fes::EeCore> core;
extern "C" JNIEXPORT jstring JNICALL Java_com_izukiken_p3fes_MainActivity_nativeRuntimeStatus(JNIEnv* e,jobject){if(!core)core=std::make_unique<p3fes::EeCore>();return e->NewStringUTF("EE CPU foundation loaded; incomplete runtime, game boot unavailable.");}