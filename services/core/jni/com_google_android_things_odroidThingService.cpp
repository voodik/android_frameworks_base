/*
 *    Copyright (c) 2019 Sangchul Go <luke.go@hardkernel.com>
 *
 *    OdroidThings is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Lesser General Public License as
 *    published by the Free Software Foundation, either version 3 of the
 *    License, or (at your option) any later version.
 *
 *    OdroidThings is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with OdroidThings.
 *    If not, see <http://www.gnu.org/licenses/>.
 */

#define LOG_TAG "OdroidThings-JNI"

#include <nativehelper/JNIHelp.h>

#include <hardware/hardware.h>

#include <utils/Log.h>
#include "core_jni_helpers.h"

#include <vector>
#include <map>

#include <vendor/hardkernel/hardware/odroidthings/1.0/IOdroidThings.h>
#include <vendor/hardkernel/hardware/odroidthings/1.0/IOdroidThingsGpioCallback.h>
#if defined(__LP64__)
#define THINGS_PATH "/system/lib64/hw/odroidThings.so"
#else
#define THINGS_PATH "/system/lib/hw/odroidThings.so"
#endif

namespace android {

using android::sp;

using android::hardware::Return;
using android::hardware::Void;
using android::hardware::hidl_vec;

using IOdroidThings = vendor::hardkernel::hardware::odroidthings::V1_0::IOdroidThings;
using IOdroidThingsGpioCallback = vendor::hardkernel::hardware::odroidthings::V1_0::IOdroidThingsGpioCallback;
using direction_t = vendor::hardkernel::hardware::odroidthings::V1_0::Direction;
using vendor::hardkernel::hardware::odroidthings::V1_0::Result;

class OdroidThingHal {
private:
    static sp<IOdroidThings> sOdroidThings;

    OdroidThingHal() {}

public:
    static void disassociate() {
        sOdroidThings = nullptr;
    }

    static sp<IOdroidThings> associate() {
        if (sOdroidThings == nullptr) {
            sOdroidThings = IOdroidThings::getService();

            if (sOdroidThings == nullptr) {
                ALOGE("Unable to get IOdroidThings interface.");
            }
        }
        return sOdroidThings;
    }
};

sp<IOdroidThings> OdroidThingHal::sOdroidThings = nullptr;

static void init(JNIEnv *env, jobject obj) {
    sp<IOdroidThings> hal = OdroidThingHal::associate();
}

static jobject getPinName(JNIEnv *env, jobject obj) {

    jclass clsArrayList = env->FindClass("java/util/ArrayList");
    jmethodID mCreator = env->GetMethodID(clsArrayList, "<init>", "()V");
    jmethodID mAdd = env->GetMethodID(clsArrayList, "add", "(Ljava/lang/Object;)Z");
    jobject resultArray = env->NewObject(clsArrayList, mCreator);

    sp<IOdroidThings> hal = OdroidThingHal::associate();

    hal->getPinNameList(
            [&](const auto &pinNameList) {
            const size_t count = pinNameList.size();
            for (size_t i=0; i<count; i++) {
            std::string pin = pinNameList[i];
            jstring pinName = env->NewStringUTF(pin.c_str());
            env->CallBooleanMethod(resultArray, mAdd, pinName);
            env->DeleteLocalRef(pinName);
            }
            });

    return resultArray;
}

static jobject getListOf(JNIEnv *env, jobject obj, jint mode) {

    jclass clsArrayList = env->FindClass("java/util/ArrayList");
    jmethodID mCreator = env->GetMethodID(clsArrayList, "<init>", "()V");
    jmethodID mAdd = env->GetMethodID(clsArrayList, "add", "(Ljava/lang/Object;)Z");
    jobject resultArray = env->NewObject(clsArrayList, mCreator);

    sp<IOdroidThings> hal = OdroidThingHal::associate();

    hal->getListOf(
            mode,
            [&](const auto &pinList) {
            const size_t count = pinList.size();
            for (size_t i=0; i<count; i++) {
                std::string pin = pinList[i];
                jstring pinName = env->NewStringUTF(pin.c_str());
                env->CallBooleanMethod(resultArray, mAdd, pinName);
                env->DeleteLocalRef(pinName);
            }
            });

    return resultArray;
}
static void setGpioDirection(JNIEnv *env, jobject obj, jint pin, jint direction) {
    sp<IOdroidThings> hal = OdroidThingHal::associate();
    hal->setDirection(pin, (direction_t) direction);
}

static void setGpioValue(JNIEnv *env, jobject obj, jint pin, jboolean value) {
    sp<IOdroidThings> hal = OdroidThingHal::associate();
    hal->gpio_setValue(pin, (value == JNI_TRUE));
}

static jboolean getGpioValue(JNIEnv *env, jobject obj, jint pin) {
    sp<IOdroidThings> hal = OdroidThingHal::associate();
    bool result = hal->gpio_getValue(pin);
    return (result? JNI_TRUE: JNI_FALSE);
}

static void setGpioActiveType(JNIEnv *env, jobject obj, jint pin, jint activeType) {
    sp<IOdroidThings> hal = OdroidThingHal::associate();
    hal->gpio_setActiveType(pin, activeType);
}

static void setEdgeTriggerType(JNIEnv *env, jobject obj, jint pin, jint edgeTriggerType) {
    sp<IOdroidThings> hal = OdroidThingHal::associate();
    hal->gpio_setEdgeTriggerType(pin, edgeTriggerType);
}

class Callback : public IOdroidThingsGpioCallback {
    private:
        JavaVM* jvm;
        JNIEnv *env;
        int pin;
        jclass thingsManagerClass;
        jmethodID cb;
    public:
        Callback();
        ~Callback();
        Callback(JNIEnv*, int);
        Return<void> doCallback() override;
};

Callback::Callback() {
    this->env = NULL;
    this->pin= -1;
}
Callback::~Callback() {
    this->env = NULL;
    this->jvm = NULL;
    this->pin = -1;
}

Callback::Callback(JNIEnv *env, int pin) {
    this->env = env;
    this->pin = pin;
    env->GetJavaVM(&jvm);
    jclass localClass = (*env).FindClass("com/google/android/things/odroid/OdroidThingsManager");
    thingsManagerClass = (jclass) (*env).NewGlobalRef(localClass);
    cb = (*env).GetStaticMethodID(thingsManagerClass, "doCallback", "(I)V");
}

Return<void> Callback::doCallback() {
    (*jvm).AttachCurrentThread(&env, NULL);
    (*env).CallStaticVoidMethod(thingsManagerClass, cb, pin);

    return Void();
}

static void registerCallback(JNIEnv *env, jobject obj, jint pin) {
    sp<IOdroidThings> hal = OdroidThingHal::associate();
    sp<IOdroidThingsGpioCallback> callback = new Callback(env, pin);
    hal->gpio_registerCallback(pin, callback);
}

static void unregisterCallback(JNIEnv *env, jobject obj, jint pin) {
    sp<IOdroidThings> hal = OdroidThingHal::associate();
    hal->gpio_unregisterCallback(pin);
}

static void openPwm(JNIEnv *env, jobject obj, jint pin) {
    sp<IOdroidThings> hal = OdroidThingHal::associate();
    hal->pwm_open(pin);
}

static void closePwm(JNIEnv *env, jobject obj, jint pin) {
    sp<IOdroidThings> hal = OdroidThingHal::associate();
    hal->pwm_close(pin);
}

static jboolean setPwmEnable(JNIEnv *env, jobject obj, jint pin, jboolean enabled) {
    sp<IOdroidThings> hal = OdroidThingHal::associate();
    bool result = hal->pwm_setEnable(pin, (enabled == JNI_TRUE));
    return (result? JNI_TRUE:JNI_FALSE);
}

static jboolean setDutyCycle(JNIEnv *env, jobject obj, jint pin, jdouble cycle_rate) {
    sp<IOdroidThings> hal = OdroidThingHal::associate();
    bool result = hal->pwm_setDutyCycle(pin, cycle_rate);
    return (result? JNI_TRUE:JNI_FALSE);
}

static jboolean setFrequency(JNIEnv *env, jobject obj, jint pin, jdouble frequency_hz) {
    sp<IOdroidThings> hal = OdroidThingHal::associate();
    bool result = hal->pwm_setFrequency(pin, frequency_hz);
    return (result? JNI_TRUE:JNI_FALSE);
}

static void openI2c(JNIEnv *env, jobject obj, jint nameIdx, jint address, jint idx) {
    sp<IOdroidThings> hal = OdroidThingHal::associate();
    hal->i2c_open(nameIdx, address, idx);
}

static void closeI2c(JNIEnv *env, jobject obj, jint idx) {
    sp<IOdroidThings> hal = OdroidThingHal::associate();
    hal->i2c_close(idx);
}

static jbyteArray readI2cRegBuffer(JNIEnv *env, jobject obj, jint idx, jint reg, jint length) {
    sp<IOdroidThings> hal = OdroidThingHal::associate();
    hidl_vec<uint8_t> buffer;
    uint8_t *retBuffer = NULL;
    jbyteArray result;

    Return<void> ret = hal->i2c_readRegBuffer(idx, reg, length,
        [&] (Result ret, hidl_vec<uint8_t> result) {
            if (ret == Result::OK) buffer = result;
        });

    if (ret.isOk()) {
        retBuffer = new uint8_t[length];
        for (int i=0; i<length; i++)
            retBuffer[i] = buffer[i];

        result = env->NewByteArray(length);
        env->SetByteArrayRegion(result, 0, length, (jbyte *)retBuffer);
        delete[] retBuffer;
    } else {
        result = env->NewByteArray(0);
    }

    return result;
}

static jboolean writeI2cRegBuffer(JNIEnv *env, jobject obj, jint idx, jint reg, jbyteArray buffer, jint length) {
    sp<IOdroidThings> hal = OdroidThingHal::associate();
    Result ret;
    hidl_vec<uint8_t> writeBuffer(length);

    uint8_t *data = (uint8_t *)env->GetByteArrayElements(buffer, NULL);
    for(int i=0; i<length; i++)
        writeBuffer[i] = data[i];
    env->ReleaseByteArrayElements(buffer, (jbyte *)data, JNI_ABORT);

    ret = hal->i2c_writeRegBuffer(idx, reg, writeBuffer, length);

    return (ret == Result::OK)?JNI_TRUE:JNI_FALSE;
}

static const JNINativeMethod sManagerMethods[] = {
    /* name, signature, funcPtr */
    {"_init",
        "()V",
        reinterpret_cast<void *>(init)},
    {"_getPinName",
        "()Ljava/util/ArrayList;",
        reinterpret_cast<void*>(getPinName)},
    {"_getListOf",
        "(I)Ljava/util/ArrayList;",
        reinterpret_cast<void*>(getListOf)},
};

static const JNINativeMethod sGpioMethods[] = {
    {"_setGpioDirection",
        "(II)V",
        reinterpret_cast<void *>(setGpioDirection)},
    {"_setGpioValue",
        "(IZ)V",
        reinterpret_cast<void *>(setGpioValue)},
    {"_getGpioValue",
        "(I)Z",
        reinterpret_cast<void *>(getGpioValue)},
    {"_setGpioActiveType",
        "(II)V",
        reinterpret_cast<void *>(setGpioActiveType)},
    {"_setEdgeTriggerType",
        "(II)V",
        reinterpret_cast<void *>(setEdgeTriggerType)},
    {"_registerCallback",
        "(I)V",
        reinterpret_cast<void *>(registerCallback)},
    {"_unregisterCallback",
        "(I)V",
        reinterpret_cast<void *>(unregisterCallback)},

};

static const JNINativeMethod sPwmMethods[] = {
    {"_openPwm",
        "(I)V",
        reinterpret_cast<void *>(openPwm)},
    {"_closePwm",
        "(I)V",
        reinterpret_cast<void *>(closePwm)},
    {"_setPwmEnabled",
        "(IZ)Z",
        reinterpret_cast<void *>(setPwmEnable)},
    {"_setDutyCycle",
        "(ID)Z",
        reinterpret_cast<void *>(setDutyCycle)},
    {"_setFrequency",
        "(ID)Z",
        reinterpret_cast<void *>(setFrequency)},
};

static const JNINativeMethod sI2cMethods[] = {
    {"_open",
        "(III)V",
        reinterpret_cast<void *>(openI2c)},
    {"_close",
        "(I)V",
        reinterpret_cast<void *>(closeI2c)},
    {"_readRegBuffer",
        "(III)[B",
        reinterpret_cast<void *>(readI2cRegBuffer)},
    {"_writeRegBuffer",
        "(II[BI)Z",
        reinterpret_cast<void *>(writeI2cRegBuffer)},
};

int register_google_android_things_odroid(JNIEnv* env) {
    ALOGD("load odroid things server jni ");
    jniRegisterNativeMethods(
            env,
            "com/google/android/things/odroid/OdroidThingsManager",
            sManagerMethods,
            NELEM(sManagerMethods));
    jniRegisterNativeMethods(
            env,
            "com/google/android/things/odroid/OdroidGpio",
            sGpioMethods,
            NELEM(sGpioMethods));
    jniRegisterNativeMethods(
            env,
            "com/google/android/things/odroid/OdroidPwm",
            sPwmMethods,
            NELEM(sPwmMethods));
    return jniRegisterNativeMethods(
            env,
            "com/google/android/things/odroid/OdroidI2c",
            sI2cMethods,
            NELEM(sI2cMethods));
}
} // namespace android
