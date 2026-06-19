#include <dobby.h>
#include <jni.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include "sensor_hook.h"

bool enableSensorHook = false;

static inline int64_t getTimeNanos() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM* vm, void* reserved) {
    JNIEnv* env;
    if (vm->GetEnv((void**) &env, JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }

    doSensorHook();

    return JNI_VERSION_1_6;
}

extern "C"
JNIEXPORT void JNICALL
Java_moe_fuqiuluo_dobby_Dobby_setStatus(JNIEnv *env, jobject thiz, jboolean status) {
    enableSensorHook = status;
}

extern "C"
JNIEXPORT void JNICALL
Java_moe_fuqiuluo_dobby_Dobby_setStepMockStatus(JNIEnv *env, jobject thiz, jboolean status, jint frequency, jlong baseCount) {
    enableStepMock = status;
    if (status) {
        stepFrequency = frequency > 0 ? frequency : 100;
        stepBaseCount = baseCount;
        stepMockStartNanos = getTimeNanos();
        lastStepDetectorNanos = stepMockStartNanos;
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_moe_fuqiuluo_dobby_Dobby_setStepCount(JNIEnv *env, jobject thiz, jlong count) {
    stepBaseCount = count;
    stepMockStartNanos = getTimeNanos();
}

extern "C"
JNIEXPORT void JNICALL
Java_moe_fuqiuluo_dobby_Dobby_setStepFrequency(JNIEnv *env, jobject thiz, jint frequency) {
    if (frequency > 0) {
        stepFrequency = frequency;
    }
}
