
# Configure Android SDK/NDK here.

ANDROID_SDK_VERSION := 23
ANDROID_SDK_BUILD_TOOLS_VERSION := 23.0.1
ANDROID_GCC_VERSION := 4.9

ANDROID_SDK_ROOT := $(CHROMIUM_ROOT)/third_party/android_tools/sdk
ANDROID_NDK_ROOT := $(CHROMIUM_ROOT)/third_party/android_tools/ndk

ANDROID_SDK := $(ANDROID_SDK_ROOT)/platforms/android-$(ANDROID_SDK_VERSION)
ANDROID_SDK_TOOLS := $(ANDROID_SDK_ROOT)/build-tools/$(ANDROID_SDK_BUILD_TOOLS_VERSION)


ANDROID_NDK_PREBUILT.x86 = $(ANDROID_NDK_ROOT)/prebuilt/android-x86
ANDROID_TOOLCHAIN.x86 = $(ANDROID_NDK_ROOT)/toolchains/x86-$(ANDROID_GCC_VERSION)/prebuilt/linux-x86_64/bin
TRIPLET.x86 = i686-linux-android
CROSS.x86 = $(TRIPLET.x86)-

ANDROID_NDK_PREBUILT.x86_64 = $(ANDROID_NDK_ROOT)/prebuilt/android-x86_64
ANDROID_TOOLCHAIN.x86_64 = $(ANDROID_NDK_ROOT)/toolchains/x86_64-$(ANDROID_GCC_VERSION)/prebuilt/linux-x86_64/bin
TRIPLET.x86_64 = x86_64-linux-android
CROSS.x86_64 = $(TRIPLET.x86_64)-

ANDROID_NDK_PREBUILT.armv5 = $(ANDROID_NDK_ROOT)/prebuilt/android-arm
ANDROID_TOOLCHAIN.armv5 = $(ANDROID_NDK_ROOT)/toolchains/arm-linux-androideabi-$(ANDROID_GCC_VERSION)/prebuilt/linux-x86_64/bin
TRIPLET.armv5 = arm-linux-androideabi
CROSS.armv5 = $(TRIPLET.armv5)-

ANDROID_NDK_PREBUILT.armv7 = $(ANDROID_NDK_PREBUILT.armv5)
ANDROID_TOOLCHAIN.armv7 = $(ANDROID_TOOLCHAIN.armv5)
TRIPLET.armv7 = $(TRIPLET.armv5)
CROSS.armv7 = $(CROSS.armv5)

ANDROID_NDK_PREBUILT.armv8 = $(ANDROID_NDK_ROOT)/prebuilt/android-arm64
ANDROID_TOOLCHAIN.armv8 = $(ANDROID_NDK_ROOT)/toolchains/aarch64-linux-android-$(ANDROID_GCC_VERSION)/prebuilt/linux-x86_64/bin
TRIPLET.armv8 = aarch64-linux-android
CROSS.armv8 = $(TRIPLET.armv8)-

COMPILER_PATH = $(ANDROID_TOOLCHAIN.$(TARGET_ARCH))
STRIP = $(COMPILER_PATH)/$(CROSS.$(TARGET_ARCH))strip

SDK_JELLY_BEAN := 16
SDK_MARSHMALLOW := 23
