LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := boost-socket
LOCAL_SRC_FILES := async_tcp_server.h main.cpp

LOCAL_LDLIBS    := -llog -landroid
LOCAL_LDLIBS    += -lboost_thread-gcc-mt-1_49
LOCAL_LDLIBS    += -lboost_system-gcc-mt-1_49
LOCAL_LDLIBS    += -lboost_iostreams-gcc-mt-1_49
LOCAL_LDLIBS    += -lboost_date_time-gcc-mt-1_49
LOCAL_LDLIBS    += -lgnustl_static

LOCAL_STATIC_LIBRARIES := android_native_app_glue

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
