LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := boost-date-time
LOCAL_SRC_FILES := main.cpp

LOCAL_LDLIBS    := -llog -landroid
LOCAL_LDLIBS    += -lboost_date_time-gcc-mt-1_49 -lgnustl_static

LOCAL_STATIC_LIBRARIES := android_native_app_glue

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
