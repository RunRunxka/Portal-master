//
// Created by fuqiuluo on 2024/10/15.
//

#ifndef PORTAL_SENSOR_HOOK_H
#define PORTAL_SENSOR_HOOK_H

#include "android/sensor.h"
#include <cstdint>

// ssize_t SensorEventQueue::write(const sp<BitTube>& tube,
//        ASensorEvent const* events, size_t numEvents)
typedef int64_t (*OriginalSensorEventQueueWriteType)(void*, void*, int64_t);

// void convertToSensorEvent(const Event &src, sensors_event_t *dst);
typedef void (*OriginalConvertToSensorEventType)(void*, void*);

// Step mock state
extern bool enableStepMock;
extern int stepFrequency;
extern int64_t stepBaseCount;
extern int64_t stepMockStartNanos;
extern int64_t lastStepDetectorNanos;

void doSensorHook();

#endif //PORTAL_SENSOR_HOOK_H
