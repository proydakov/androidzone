LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := std-streams
LOCAL_SRC_FILES := main.cpp

LOCAL_LDLIBS    := -llog -landroid
LOCAL_LDLIBS    += -lgnustl_static

LOCAL_STATIC_LIBRARIES := android_native_app_glue

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
