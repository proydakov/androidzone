LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := modbus-tcp
LOCAL_SRC_FILES := main.cpp modbus_tcp_server.h modbus_tcp_client.h modbus_logger.h

LOCAL_LDLIBS    := -llog -landroid
LOCAL_LDLIBS    += -lmodbus
LOCAL_LDLIBS    += -lboost_thread-gcc-mt-1_49 -lgnustl_static

LOCAL_STATIC_LIBRARIES := android_native_app_glue

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
