LOCAL_PATH := $(call my-dir)

MC_PATH := ../../../../../src/client
CMN_PATH := ../../../../../src/common

include $(CLEAR_VARS)

LOCAL_MODULE := main

SDL_PATH := ../SDL2

LOCAL_CFLAGS := -DON_ANDROID -DUSE_SDL2 -DUSE_SDL2_TTF

LOCAL_C_INCLUDES := $(LOCAL_PATH)/$(SDL_PATH)/include

# Add your application source files here...
LOCAL_SRC_FILES := $(MC_PATH)/main-sdl2.c $(MC_PATH)/client.c \
		$(MC_PATH)/sdl-font.c $(MC_PATH)/sdl-sound.c \
		$(MC_PATH)/c-birth.c $(MC_PATH)/c-cmd.c $(MC_PATH)/c-files.c \
		$(MC_PATH)/c-init.c $(MC_PATH)/c-inven.c $(MC_PATH)/c-spell.c \
		$(MC_PATH)/c-store.c $(MC_PATH)/c-tables.c $(MC_PATH)/c-util.c \
		$(MC_PATH)/c-xtra1.c $(MC_PATH)/c-xtra2.c $(MC_PATH)/c-variable.c \
		$(MC_PATH)/c-cmd0.c \
		$(MC_PATH)/net-client.c \
		$(MC_PATH)/set_focus.c $(MC_PATH)/ui.c \
		$(MC_PATH)/z-term.c \
		$(MC_PATH)/lupng/lupng.c $(MC_PATH)/lupng/miniz.c \
		$(MC_PATH)/grafmode.c \
		$(CMN_PATH)/base64encode.c $(CMN_PATH)/sha1.c \
		$(CMN_PATH)/z-file.c \
		$(CMN_PATH)/z-form.c $(CMN_PATH)/z-rand.c $(CMN_PATH)/z-util.c \
		$(CMN_PATH)/z-virt.c $(CMN_PATH)/md5.c $(CMN_PATH)/net-basics.c \
		$(CMN_PATH)/net-imps.c $(CMN_PATH)/net-pack.c \
		$(CMN_PATH)/parser.c $(CMN_PATH)/z-bitflag.c


LOCAL_SHARED_LIBRARIES := SDL2 SDL2_ttf

LOCAL_LDLIBS := -lGLESv1_CM -lGLESv2 -llog

include $(BUILD_SHARED_LIBRARY)
