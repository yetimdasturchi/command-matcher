LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := manulu
LOCAL_SRC_FILES := cJSON.c \
                   macros.c \
                   main.c \
                   metaphone.c \
                   re.c \
                   similarity.c \
                   sqlite3.c \
                   strtotime.c \
                   utils.c \
                   jaro.c

LOCAL_CFLAGS := -DFOR_DROID -fvisibility=hidden
LOCAL_C_INCLUDES := $(LOCAL_PATH)
LOCAL_LDLIBS := -llog

include $(BUILD_SHARED_LIBRARY)