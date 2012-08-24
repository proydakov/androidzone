LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := gles-engine

LOCAL_SRC_FILES := main.cpp macro.h
LOCAL_SRC_FILES += message_logger.h message_logger.cpp
LOCAL_SRC_FILES += egl_engine.h egl_engine.cpp
LOCAL_SRC_FILES += gles_engine.h gles_engine.cpp
LOCAL_SRC_FILES += fps_calculator.h fps_calculator.cpp

LOCAL_LDLIBS := -llog -landroid 
LOCAL_LDLIBS += -lEGL -lGLESv1_CM
LOCAL_LDLIBS += -lboost_thread-gcc-mt-1_49
LOCAL_LDLIBS += -lboost_date_time-gcc-mt-1_49
LOCAL_LDLIBS += -lgnustl_static

LOCAL_STATIC_LIBRARIES := android_native_app_glue

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
