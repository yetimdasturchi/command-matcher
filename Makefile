COLOUR_GREEN=\033[0;32m
COLOUR_RED=\033[0;31m
COLOUR_MAGENTA=\033[0;35m
COLOUR_BLUE=\033[0;34m
END_COLOUR=\033[0m

OUT_DIR := out
SRC_DIR := jni
SOURCES := $(wildcard $(SRC_DIR)/*.c)
OBJECTS := $(patsubst $(SRC_DIR)/%.c, $(OUT_DIR)/%.o, $(SOURCES))
HEADERS := $(wildcard $(SRC_DIR)/*.h)

NDK_BUILD := $(ANDROID_NDK_HOME)/ndk-build
APP_PLATFORM := android-29
JNI_DIR := jni

all: linux

linux: pre clean library build

android:
	@echo "Building for Android..."
	$(NDK_BUILD) -C $(JNI_DIR) APP_PLATFORM=$(APP_PLATFORM)

pre:
	@echo "$(COLOUR_MAGENTA)Configuring...$(END_COLOUR)"

$(OUT_DIR):
	@echo "$(COLOUR_BLUE)Creating output directory...$(END_COLOUR)"
	@mkdir -p $(OUT_DIR)

clean:
	@echo "Cleaning up..."
	@rm -rf $(OUT_DIR)/*.o $(OUT_DIR)/*.so $(OUT_DIR)/test
	@rm -rf libs obj

$(OUT_DIR)/libmanulu.so: $(OBJECTS)
	@gcc -fvisibility=hidden -shared -g -o $@ $^

$(OUT_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS)
	@gcc -fvisibility=hidden -g -o $@ -c -fpic $<

library: $(OUT_DIR)/libmanulu.so

build: library
	@echo "$(COLOUR_MAGENTA)Building...$(END_COLOUR)"
	@gcc -o $(OUT_DIR)/test test.c -L$(OUT_DIR) -lmanulu -lm -pipe

test: build
	@echo "$(COLOUR_BLUE)Running tests...$(END_COLOUR)"
	@./$(OUT_DIR)/test

.PHONY: all linux android pre clean library build test
