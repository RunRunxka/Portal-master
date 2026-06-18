//
// Created by fuqiuluo on 2024/10/15.
//
#include <dobby.h>
#include <unistd.h>
#include <sys/time.h>
#include "sensor_hook.h"
#include "logging.h"
#include "elf_util.h"
#include "dobby_hook.h"

#define LIBSF_PATH \"/system/lib64/libsensorservice.so\"

extern bool enableSensorHook;

// Step mock state
bool enableStepMock = false;
int stepFrequency = 100;
int64_t stepBaseCount = 0;
int64_t stepMockStartNanos = 0;
int64_t lastStepDetectorNanos = 0;

// _ZN7android16SensorEventQueue5writeERKNS_2spINS_7BitTubeEEEPK12ASensorEventm
OriginalSensorEventQueueWriteType OriginalSensorEventQueueWrite = nullptr;

OriginalConvertToSensorEventType OriginalConvertToSensorEvent = nullptr;

static inline int64_t getTimeNanos() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

int64_t SensorEventQueueWrite(void *tube, void *events, int64_t numEvents) {
    if (enableSensorHook) {
        LOGD(\"SensorEventQueueWrite called\");
    }
    return OriginalSensorEventQueueWrite(tube, events, numEvents);
}

void ConvertToSensorEvent(void *src, void *dst) {
    if (enableSensorHook) {
        auto a = *(int32_t *)((char*)src + 4);
        auto b = *(int32_t *)((char*)src + 8);
        auto c = *(int64_t *)((char*)src + 16);

        *(int64_t *)((char*)dst + 16) = 0LL;
        *(int32_t *)((char*)dst + 24) = 0;
        *(int64_t *)((char*)dst) = c;
        *(int32_t *)((char*)dst + 8) = a;
        *(int32_t *)((char*)dst + 12) = b;
        *(int8_t *)((char*)dst + 28) = b;

        if (b == 18) {
            // TYPE_STEP_DETECTOR: fire step events at configured frequency
            if (enableStepMock) {
                int64_t now = getTimeNanos();
                int64_t stepIntervalNanos = 60000000000LL / stepFrequency;
                if (now - lastStepDetectorNanos >= stepIntervalNanos) {
                    *(float *)((char*)dst + 16) = 1.0f;
                    lastStepDetectorNanos = now;
                } else {
                    *(float *)((char*)dst + 16) = 0.0f;
                }
            } else {
                *(float *)((char*)dst + 16) = -1.0;
            }
        } else if (b == 19) {
            // TYPE_STEP_COUNTER: monotonic increasing cumulative count
            if (enableStepMock) {
                int64_t now = getTimeNanos();
                int64_t elapsedNanos = now - stepMockStartNanos;
                int64_t stepsToAdd = elapsedNanos * stepFrequency / 60000000000LL;
                *(int64_t *)((char*)dst + 16) = stepBaseCount + stepsToAdd;
            } else {
                *(int64_t *)((char*)dst + 16) = -1;
            }
        } else {
            *(float *)((char*)dst + 16) = -1.0;
            *(float *)((char*)dst + 24) = -1.0;
            *(int8_t *)((char*)dst + 28) = *(int8_t *)((char*)src + 36);
        }
    } else {
        OriginalConvertToSensorEvent(src, dst);
    }

    if (enableSensorHook) {
        LOGD(\"ConvertToSensorEvent called\");
    }
}

void doSensorHook() {
    SandHook::ElfImg sensorService(LIBSF_PATH);

    if (!sensorService.isValid()) {
        LOGE(\"failed to load libsensorservice\");
        return;
    }

    auto sensorWrite = sensorService.getSymbolAddress<void*>(\"_ZN7android16SensorEventQueue5writeERKNS_2spINS_7BitTubeEEEPK12ASensorEventm\");
    if (sensorWrite == nullptr) {
        sensorWrite = sensorService.getSymbolAddress<void*>(\"_ZN7android16SensorEventQueue5writeERKNS_2spINS_7BitTubeEEEPK12ASensorEventj\");
    }

    auto convertToSensorEvent = sensorService.getSymbolAddress<void*>(\"_ZN7android8hardware7sensors4V1_014implementation20convertToSensorEventERKNS2_5EventEP15sensors_event_t\");

    LOGD(\"Dobby SensorEventQueue::write found at %p\", sensorWrite);
    LOGD(\"Dobby convertToSensorEvent found at %p\", convertToSensorEvent);

    if (sensorWrite != nullptr) {
        OriginalSensorEventQueueWrite = (OriginalSensorEventQueueWriteType)InlineHook(sensorWrite, (void *)SensorEventQueueWrite);
    }

    if (convertToSensorEvent != nullptr) {
        OriginalConvertToSensorEvent = (OriginalConvertToSensorEventType)InlineHook(convertToSensorEvent, (void *)ConvertToSensorEvent);
    }
}