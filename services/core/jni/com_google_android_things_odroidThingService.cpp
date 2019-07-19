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
#include <hardware/odroidThings.h>

#include <utils/Log.h>
#include "core_jni_helpers.h"

#include <vector>
#include <map>
#include <dlfcn.h>

namespace android {
things_module_t* thingsModule;
things_device_t* thingsDevice;

static std::vector<pin_t> pinList;

static void init(JNIEnv *env, jobject obj) {
    if (!thingsModule) {
        // TODO: Apply hw_get_module
        void *handle = dlopen("/system/lib/hw/odroidThings.so", RTLD_NOW);
        if ( handle == NULL) {
            ALOGE("module load err");
        }
        const char *sym = HAL_MODULE_INFO_SYM_AS_STR;
        thingsModule = (things_module_t*) dlsym(handle, sym);
    }
    if (thingsModule) {
        thingsModule->init();
        thingsModule->common.methods->open((const hw_module_t *) thingsModule, NULL,
                (struct hw_device_t **)&thingsDevice);
        pinList = thingsModule->getPinList();
    }
}

static jobject getPinName(JNIEnv *env, jobject obj) {

    jclass clsArrayList = env->FindClass("java/util/ArrayList");
    jmethodID mCreator = env->GetMethodID(clsArrayList, "<init>", "()V");
    jmethodID mAdd = env->GetMethodID(clsArrayList, "add", "(Ljava/lang/Object;)Z");
    jobject resultArray = env->NewObject(clsArrayList, mCreator);

    for (pin_t pin: pinList) {
        jstring pinName = env->NewStringUTF(pin.name.c_str());
        env->CallBooleanMethod(resultArray, mAdd, pinName);
        env->DeleteLocalRef(pinName);
    }

    return resultArray;
}

static jintArray getAvailables(JNIEnv *env, jobject obj) {
    jintArray intArray = env->NewIntArray(PIN_MAX);
    jint *element = env->GetIntArrayElements(intArray, nullptr);
    int i=0;
    for (pin_t pin: pinList) {
        element[i] = pin.availableModes;
        i++;
    }
    env->ReleaseIntArrayElements(intArray, element, 0);

    return intArray;
}

static void setGpioDirection(JNIEnv *env, jobject obj, jint pin, jint direction) {
    thingsDevice->gpio_ops.setDirection(pin, direction);
}

static void setGpioValue(JNIEnv *env, jobject obj, jint pin, jboolean value) {
    thingsDevice->gpio_ops.setValue(pin, (value == JNI_TRUE));
}
static jboolean getGpioValue(JNIEnv *env, jobject obj, jint pin) {
    return (thingsDevice->gpio_ops.getValue(pin)? JNI_TRUE:JNI_FALSE);
}

static void setGpioActiveType(JNIEnv *env, jobject obj, jint pin, jint activeType) {
    thingsDevice->gpio_ops.setActiveType(pin, activeType);
}

static void setEdgeTriggerType(JNIEnv *env, jobject obj, jint pin, jint edgeTriggerType) {
    thingsDevice->gpio_ops.setEdgeTriggerType(pin, edgeTriggerType);
}

class Callback {
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
        void doCallback();
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

void Callback::doCallback() {
    (*jvm).AttachCurrentThread(&env, NULL);
    (*env).CallStaticVoidMethod(thingsManagerClass, cb, pin);
}

std::map<int, Callback> callbackList;
// TODO: Replace it to class!
static void callback0() {callbackList[0].doCallback();}
static void callback1() {callbackList[1].doCallback();}
static void callback2() {callbackList[2].doCallback();}
static void callback3() {callbackList[3].doCallback();}
static void callback4() {callbackList[4].doCallback();}
static void callback5() {callbackList[5].doCallback();}
static void callback6() {callbackList[6].doCallback();}
static void callback7() {callbackList[7].doCallback();}
static void callback8() {callbackList[8].doCallback();}
static void callback9() {callbackList[9].doCallback();}
static void callback10() {callbackList[10].doCallback();}
static void callback11() {callbackList[11].doCallback();}
static void callback12() {callbackList[12].doCallback();}
static void callback13() {callbackList[13].doCallback();}
static void callback14() {callbackList[14].doCallback();}
static void callback15() {callbackList[15].doCallback();}
static void callback16() {callbackList[16].doCallback();}
static void callback17() {callbackList[17].doCallback();}
static void callback18() {callbackList[18].doCallback();}
static void callback19() {callbackList[19].doCallback();}
static void callback20() {callbackList[20].doCallback();}
static void callback21() {callbackList[21].doCallback();}
static void callback22() {callbackList[22].doCallback();}
static void callback23() {callbackList[23].doCallback();}
static void callback24() {callbackList[24].doCallback();}
static void callback25() {callbackList[25].doCallback();}
static void callback26() {callbackList[26].doCallback();}
static void callback27() {callbackList[27].doCallback();}
static void callback28() {callbackList[28].doCallback();}
static void callback29() {callbackList[29].doCallback();}
static void callback30() {callbackList[30].doCallback();}
static void callback31() {callbackList[31].doCallback();}

static void registerCallback(JNIEnv *env, jobject obj, jint pin) {
    callbackList.insert(std::pair<int, Callback>(pin,Callback(env, pin)));
    function_t callback;
    switch (pin) {
        case 0: callback = &callback0; break;
        case 1: callback = &callback1; break;
        case 2: callback = &callback2; break;
        case 3: callback = &callback3; break;
        case 4: callback = &callback4; break;
        case 5: callback = &callback5; break;
        case 6: callback = &callback6; break;
        case 7: callback = &callback7; break;
        case 8: callback = &callback8; break;
        case 9: callback = &callback9; break;
        case 10: callback = &callback10; break;
        case 11: callback = &callback11; break;
        case 12: callback = &callback12; break;
        case 13: callback = &callback13; break;
        case 14: callback = &callback14; break;
        case 15: callback = &callback15; break;
        case 16: callback = &callback16; break;
        case 17: callback = &callback17; break;
        case 18: callback = &callback18; break;
        case 19: callback = &callback19; break;
        case 20: callback = &callback20; break;
        case 21: callback = &callback21; break;
        case 22: callback = &callback22; break;
        case 23: callback = &callback23; break;
        case 24: callback = &callback24; break;
        case 25: callback = &callback25; break;
        case 26: callback = &callback26; break;
        case 27: callback = &callback27; break;
        case 28: callback = &callback28; break;
        case 29: callback = &callback29; break;
        case 30: callback = &callback30; break;
        case 31: callback = &callback31; break;
    }
    thingsDevice->gpio_ops.registerCallback(pin, callback);
}

static void unregisterCallback(JNIEnv *env, jobject obj, jint pin) {
    std::map<int, Callback>::iterator it;
    it = callbackList.find(pin);
    if (it != callbackList.end()) {
        thingsDevice->gpio_ops.unregisterCallback(pin);
        callbackList.erase(it);
    }
}

static const JNINativeMethod sManagerMethods[] = {
    /* name, signature, funcPtr */
    {"_init",
        "()V",
        reinterpret_cast<void *>(init)},
    {"_getPinName",
        "()Ljava/util/ArrayList;",
        reinterpret_cast<void*>(getPinName)},
    {"_getPinAvailables",
        "()[I",
        reinterpret_cast<void*>(getAvailables)},
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
int register_google_android_things_odroid(JNIEnv* env) {
    ALOGD("load odroid things server jni ");
    jniRegisterNativeMethods(
            env,
            "com/google/android/things/odroid/OdroidThingsManager",
            sManagerMethods,
            NELEM(sManagerMethods));
    return jniRegisterNativeMethods(
            env,
            "com/google/android/things/odroid/OdroidGpio",
            sGpioMethods,
            NELEM(sGpioMethods));
}
} // namespace android
