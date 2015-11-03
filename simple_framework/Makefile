# source files.
SRC = \
    src/EGLRuntime.cpp \
    src/ETCHeader.cpp \
    src/HDRImage.cpp \
    src/Matrix.cpp \
    src/Platform.cpp \
    src/Shader.cpp \
    src/Text.cpp \
    src/Texture.cpp     \
    src/Timer.cpp \
    src/models/CubeModel.cpp \
    src/models/PlaneModel.cpp \
    src/models/SphereModel.cpp \
    src/models/SuperEllipsoidModel.cpp \
    src/models/TorusModel.cpp \
    src/HWCPlatform.cpp

OBJ = $(SRC:.cpp=.o)
 
OUT = libsimple_framework2.a
 
# include directories
INCLUDES = -I. -I/usr/include -I/usr/local/include -I ./inc -I ./inc/models -I /usr/local/include/android -I /usr/local/include/hybris/hwcomposerwindow -I /usr/local/include/hybris/eglplatformcommon 
 
# C++ compiler flags (-g -O2 -Wall)
CCFLAGS = -g -DGLES_VERSION=2 -DDEBUG
 
# compiler
CCC = g++
 
# library paths
LIBS = -L/usr/local/lib -l hybris-common -l hybris-hwcomposerwindow -l hybris-eglplatformcommon -l EGL -l sync -l hardware -l GLESv2
 
# compile flags
LDFLAGS = -g
 
.SUFFIXES: .cpp
 
default: dep $(OUT)
 
.cpp.o:
	$(CCC) $(INCLUDES) $(CCFLAGS) -c $< -o $@
 
$(OUT): $(OBJ)
	ar rcs $(OUT) $(OBJ)
 
depend: dep
 
dep:
	makedepend -- $(CFLAGS) -- $(INCLUDES) $(SRC)
 
clean:
	rm -f $(OBJ) $(OUT) Makefile.bak
# DO NOT DELETE

src/EGLRuntime.o: ./inc/EGLRuntime.h /usr/include/EGL/egl.h
src/EGLRuntime.o: /usr/include/EGL/eglplatform.h
src/EGLRuntime.o: /usr/include/KHR/khrplatform.h /usr/include/stdint.h
src/EGLRuntime.o: /usr/include/features.h /usr/include/stdc-predef.h
src/EGLRuntime.o: /usr/include/X11/Xlib.h /usr/include/X11/X.h
src/EGLRuntime.o: /usr/include/X11/Xfuncproto.h /usr/include/X11/Xosdefs.h
src/EGLRuntime.o: /usr/include/X11/Xutil.h /usr/include/X11/keysym.h
src/EGLRuntime.o: /usr/include/X11/keysymdef.h /usr/include/EGL/eglext.h
src/EGLRuntime.o: /usr/include/EGL/eglmesaext.h
src/EGLRuntime.o: /usr/include/EGL/eglextchromium.h ./inc/Platform.h
src/EGLRuntime.o: /usr/local/include/hybris/hwcomposerwindow/hwcomposer_window.h
src/EGLRuntime.o: /usr/local/include/hybris/eglplatformcommon/nativewindowbase.h
src/EGLRuntime.o: /usr/include/string.h /usr/include/xlocale.h
src/EGLRuntime.o: /usr/local/include/android/system/window.h
src/EGLRuntime.o: /usr/local/include/android/cutils/native_handle.h
src/EGLRuntime.o: /usr/include/errno.h /usr/include/limits.h
src/EGLRuntime.o: /usr/include/limits.h
src/EGLRuntime.o: /usr/local/include/android/sync/sync.h
src/EGLRuntime.o: /usr/local/include/android/system/graphics.h
src/EGLRuntime.o: /usr/include/unistd.h /usr/include/getopt.h
src/EGLRuntime.o: /usr/local/include/hybris/eglplatformcommon/support.h
src/EGLRuntime.o: /usr/include/assert.h /usr/include/linux/fb.h
src/EGLRuntime.o: /usr/include/linux/types.h /usr/include/linux/posix_types.h
src/EGLRuntime.o: /usr/include/linux/stddef.h /usr/include/linux/i2c.h
src/EGLRuntime.o: /usr/local/include/android/hardware/gralloc.h
src/EGLRuntime.o: /usr/local/include/android/hardware/hardware.h
src/EGLRuntime.o: /usr/local/include/android/hardware/fb.h
src/EGLRuntime.o: ./inc/VectorTypes.h
src/EGLRuntime.o: /usr/local/include/android/android-config.h
src/EGLRuntime.o: /usr/local/include/android/android-version.h
src/EGLRuntime.o: /usr/include/GLES2/gl2.h /usr/include/GLES2/gl2platform.h
src/EGLRuntime.o: /usr/include/stdio.h /usr/include/libio.h
src/EGLRuntime.o: /usr/include/_G_config.h /usr/include/wchar.h
src/EGLRuntime.o: /usr/include/math.h
src/EGLRuntime.o: /usr/local/include/android/hardware/hwcomposer.h
src/EGLRuntime.o: /usr/local/include/android/hardware/hwcomposer_defs.h
src/EGLRuntime.o: /usr/include/malloc.h
src/ETCHeader.o: ./inc/ETCHeader.h
src/HDRImage.o: ./inc/HDRImage.h ./inc/Platform.h
src/HDRImage.o: /usr/local/include/hybris/hwcomposerwindow/hwcomposer_window.h
src/HDRImage.o: /usr/local/include/hybris/eglplatformcommon/nativewindowbase.h
src/HDRImage.o: /usr/include/string.h /usr/include/features.h
src/HDRImage.o: /usr/include/stdc-predef.h /usr/include/xlocale.h
src/HDRImage.o: /usr/local/include/android/system/window.h
src/HDRImage.o: /usr/local/include/android/cutils/native_handle.h
src/HDRImage.o: /usr/include/errno.h /usr/include/limits.h
src/HDRImage.o: /usr/include/limits.h /usr/include/stdint.h
src/HDRImage.o: /usr/local/include/android/sync/sync.h
src/HDRImage.o: /usr/local/include/android/system/graphics.h
src/HDRImage.o: /usr/include/unistd.h /usr/include/getopt.h
src/HDRImage.o: /usr/include/EGL/egl.h /usr/include/EGL/eglplatform.h
src/HDRImage.o: /usr/include/KHR/khrplatform.h /usr/include/X11/Xlib.h
src/HDRImage.o: /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h
src/HDRImage.o: /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h
src/HDRImage.o: /usr/include/X11/keysym.h /usr/include/X11/keysymdef.h
src/HDRImage.o: /usr/local/include/hybris/eglplatformcommon/support.h
src/HDRImage.o: /usr/include/assert.h /usr/include/linux/fb.h
src/HDRImage.o: /usr/include/linux/types.h /usr/include/linux/posix_types.h
src/HDRImage.o: /usr/include/linux/stddef.h /usr/include/linux/i2c.h
src/HDRImage.o: /usr/local/include/android/hardware/gralloc.h
src/HDRImage.o: /usr/local/include/android/hardware/hardware.h
src/HDRImage.o: /usr/local/include/android/hardware/fb.h ./inc/EGLRuntime.h
src/HDRImage.o: /usr/include/EGL/eglext.h /usr/include/EGL/eglmesaext.h
src/HDRImage.o: /usr/include/EGL/eglextchromium.h ./inc/VectorTypes.h
src/Matrix.o: ./inc/Matrix.h ./inc/VectorTypes.h ./inc/Mathematics.h
src/Matrix.o: ./inc/Platform.h
src/Matrix.o: /usr/local/include/hybris/hwcomposerwindow/hwcomposer_window.h
src/Matrix.o: /usr/local/include/hybris/eglplatformcommon/nativewindowbase.h
src/Matrix.o: /usr/include/string.h /usr/include/features.h
src/Matrix.o: /usr/include/stdc-predef.h /usr/include/xlocale.h
src/Matrix.o: /usr/local/include/android/system/window.h
src/Matrix.o: /usr/local/include/android/cutils/native_handle.h
src/Matrix.o: /usr/include/errno.h /usr/include/limits.h
src/Matrix.o: /usr/include/limits.h /usr/include/stdint.h
src/Matrix.o: /usr/local/include/android/sync/sync.h
src/Matrix.o: /usr/local/include/android/system/graphics.h
src/Matrix.o: /usr/include/unistd.h /usr/include/getopt.h
src/Matrix.o: /usr/include/EGL/egl.h /usr/include/EGL/eglplatform.h
src/Matrix.o: /usr/include/KHR/khrplatform.h /usr/include/X11/Xlib.h
src/Matrix.o: /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h
src/Matrix.o: /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h
src/Matrix.o: /usr/include/X11/keysym.h /usr/include/X11/keysymdef.h
src/Matrix.o: /usr/local/include/hybris/eglplatformcommon/support.h
src/Matrix.o: /usr/include/assert.h /usr/include/linux/fb.h
src/Matrix.o: /usr/include/linux/types.h /usr/include/linux/posix_types.h
src/Matrix.o: /usr/include/linux/stddef.h /usr/include/linux/i2c.h
src/Matrix.o: /usr/local/include/android/hardware/gralloc.h
src/Matrix.o: /usr/local/include/android/hardware/hardware.h
src/Matrix.o: /usr/local/include/android/hardware/fb.h ./inc/EGLRuntime.h
src/Matrix.o: /usr/include/EGL/eglext.h /usr/include/EGL/eglmesaext.h
src/Matrix.o: /usr/include/EGL/eglextchromium.h
src/Platform.o: ./inc/Platform.h
src/Platform.o: /usr/local/include/hybris/hwcomposerwindow/hwcomposer_window.h
src/Platform.o: /usr/local/include/hybris/eglplatformcommon/nativewindowbase.h
src/Platform.o: /usr/include/string.h /usr/include/features.h
src/Platform.o: /usr/include/stdc-predef.h /usr/include/xlocale.h
src/Platform.o: /usr/local/include/android/system/window.h
src/Platform.o: /usr/local/include/android/cutils/native_handle.h
src/Platform.o: /usr/include/errno.h /usr/include/limits.h
src/Platform.o: /usr/include/limits.h /usr/include/stdint.h
src/Platform.o: /usr/local/include/android/sync/sync.h
src/Platform.o: /usr/local/include/android/system/graphics.h
src/Platform.o: /usr/include/unistd.h /usr/include/getopt.h
src/Platform.o: /usr/include/EGL/egl.h /usr/include/EGL/eglplatform.h
src/Platform.o: /usr/include/KHR/khrplatform.h /usr/include/X11/Xlib.h
src/Platform.o: /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h
src/Platform.o: /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h
src/Platform.o: /usr/include/X11/keysym.h /usr/include/X11/keysymdef.h
src/Platform.o: /usr/local/include/hybris/eglplatformcommon/support.h
src/Platform.o: /usr/include/assert.h /usr/include/linux/fb.h
src/Platform.o: /usr/include/linux/types.h /usr/include/linux/posix_types.h
src/Platform.o: /usr/include/linux/stddef.h /usr/include/linux/i2c.h
src/Platform.o: /usr/local/include/android/hardware/gralloc.h
src/Platform.o: /usr/local/include/android/hardware/hardware.h
src/Platform.o: /usr/local/include/android/hardware/fb.h ./inc/EGLRuntime.h
src/Platform.o: /usr/include/EGL/eglext.h /usr/include/EGL/eglmesaext.h
src/Platform.o: /usr/include/EGL/eglextchromium.h ./inc/VectorTypes.h
src/Shader.o: ./inc/Shader.h ./inc/Platform.h
src/Shader.o: /usr/local/include/hybris/hwcomposerwindow/hwcomposer_window.h
src/Shader.o: /usr/local/include/hybris/eglplatformcommon/nativewindowbase.h
src/Shader.o: /usr/include/string.h /usr/include/features.h
src/Shader.o: /usr/include/stdc-predef.h /usr/include/xlocale.h
src/Shader.o: /usr/local/include/android/system/window.h
src/Shader.o: /usr/local/include/android/cutils/native_handle.h
src/Shader.o: /usr/include/errno.h /usr/include/limits.h
src/Shader.o: /usr/include/limits.h /usr/include/stdint.h
src/Shader.o: /usr/local/include/android/sync/sync.h
src/Shader.o: /usr/local/include/android/system/graphics.h
src/Shader.o: /usr/include/unistd.h /usr/include/getopt.h
src/Shader.o: /usr/include/EGL/egl.h /usr/include/EGL/eglplatform.h
src/Shader.o: /usr/include/KHR/khrplatform.h /usr/include/X11/Xlib.h
src/Shader.o: /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h
src/Shader.o: /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h
src/Shader.o: /usr/include/X11/keysym.h /usr/include/X11/keysymdef.h
src/Shader.o: /usr/local/include/hybris/eglplatformcommon/support.h
src/Shader.o: /usr/include/assert.h /usr/include/linux/fb.h
src/Shader.o: /usr/include/linux/types.h /usr/include/linux/posix_types.h
src/Shader.o: /usr/include/linux/stddef.h /usr/include/linux/i2c.h
src/Shader.o: /usr/local/include/android/hardware/gralloc.h
src/Shader.o: /usr/local/include/android/hardware/hardware.h
src/Shader.o: /usr/local/include/android/hardware/fb.h ./inc/EGLRuntime.h
src/Shader.o: /usr/include/EGL/eglext.h /usr/include/EGL/eglmesaext.h
src/Shader.o: /usr/include/EGL/eglextchromium.h ./inc/VectorTypes.h
src/Text.o: ./inc/Text.h ./inc/Matrix.h ./inc/VectorTypes.h ./inc/Texture.h
src/Text.o: ./inc/ETCHeader.h ./inc/Shader.h ./inc/Platform.h
src/Text.o: /usr/local/include/hybris/hwcomposerwindow/hwcomposer_window.h
src/Text.o: /usr/local/include/hybris/eglplatformcommon/nativewindowbase.h
src/Text.o: /usr/include/string.h /usr/include/features.h
src/Text.o: /usr/include/stdc-predef.h /usr/include/xlocale.h
src/Text.o: /usr/local/include/android/system/window.h
src/Text.o: /usr/local/include/android/cutils/native_handle.h
src/Text.o: /usr/include/errno.h /usr/include/limits.h /usr/include/limits.h
src/Text.o: /usr/include/stdint.h /usr/local/include/android/sync/sync.h
src/Text.o: /usr/local/include/android/system/graphics.h
src/Text.o: /usr/include/unistd.h /usr/include/getopt.h
src/Text.o: /usr/include/EGL/egl.h /usr/include/EGL/eglplatform.h
src/Text.o: /usr/include/KHR/khrplatform.h /usr/include/X11/Xlib.h
src/Text.o: /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h
src/Text.o: /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h
src/Text.o: /usr/include/X11/keysym.h /usr/include/X11/keysymdef.h
src/Text.o: /usr/local/include/hybris/eglplatformcommon/support.h
src/Text.o: /usr/include/assert.h /usr/include/linux/fb.h
src/Text.o: /usr/include/linux/types.h /usr/include/linux/posix_types.h
src/Text.o: /usr/include/linux/stddef.h /usr/include/linux/i2c.h
src/Text.o: /usr/local/include/android/hardware/gralloc.h
src/Text.o: /usr/local/include/android/hardware/hardware.h
src/Text.o: /usr/local/include/android/hardware/fb.h ./inc/EGLRuntime.h
src/Text.o: /usr/include/EGL/eglext.h /usr/include/EGL/eglmesaext.h
src/Text.o: /usr/include/EGL/eglextchromium.h
src/Texture.o: ./inc/Texture.h ./inc/ETCHeader.h ./inc/Platform.h
src/Texture.o: /usr/local/include/hybris/hwcomposerwindow/hwcomposer_window.h
src/Texture.o: /usr/local/include/hybris/eglplatformcommon/nativewindowbase.h
src/Texture.o: /usr/include/string.h /usr/include/features.h
src/Texture.o: /usr/include/stdc-predef.h /usr/include/xlocale.h
src/Texture.o: /usr/local/include/android/system/window.h
src/Texture.o: /usr/local/include/android/cutils/native_handle.h
src/Texture.o: /usr/include/errno.h /usr/include/limits.h
src/Texture.o: /usr/include/limits.h /usr/include/stdint.h
src/Texture.o: /usr/local/include/android/sync/sync.h
src/Texture.o: /usr/local/include/android/system/graphics.h
src/Texture.o: /usr/include/unistd.h /usr/include/getopt.h
src/Texture.o: /usr/include/EGL/egl.h /usr/include/EGL/eglplatform.h
src/Texture.o: /usr/include/KHR/khrplatform.h /usr/include/X11/Xlib.h
src/Texture.o: /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h
src/Texture.o: /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h
src/Texture.o: /usr/include/X11/keysym.h /usr/include/X11/keysymdef.h
src/Texture.o: /usr/local/include/hybris/eglplatformcommon/support.h
src/Texture.o: /usr/include/assert.h /usr/include/linux/fb.h
src/Texture.o: /usr/include/linux/types.h /usr/include/linux/posix_types.h
src/Texture.o: /usr/include/linux/stddef.h /usr/include/linux/i2c.h
src/Texture.o: /usr/local/include/android/hardware/gralloc.h
src/Texture.o: /usr/local/include/android/hardware/hardware.h
src/Texture.o: /usr/local/include/android/hardware/fb.h ./inc/EGLRuntime.h
src/Texture.o: /usr/include/EGL/eglext.h /usr/include/EGL/eglmesaext.h
src/Texture.o: /usr/include/EGL/eglextchromium.h ./inc/VectorTypes.h
src/Timer.o: ./inc/Timer.h
src/models/CubeModel.o: ./inc/models/CubeModel.h ./inc/VectorTypes.h
src/models/CubeModel.o: ./inc/Platform.h
src/models/CubeModel.o: /usr/local/include/hybris/hwcomposerwindow/hwcomposer_window.h
src/models/CubeModel.o: /usr/local/include/hybris/eglplatformcommon/nativewindowbase.h
src/models/CubeModel.o: /usr/include/string.h /usr/include/features.h
src/models/CubeModel.o: /usr/include/stdc-predef.h /usr/include/xlocale.h
src/models/CubeModel.o: /usr/local/include/android/system/window.h
src/models/CubeModel.o: /usr/local/include/android/cutils/native_handle.h
src/models/CubeModel.o: /usr/include/errno.h /usr/include/limits.h
src/models/CubeModel.o: /usr/include/limits.h /usr/include/stdint.h
src/models/CubeModel.o: /usr/local/include/android/sync/sync.h
src/models/CubeModel.o: /usr/local/include/android/system/graphics.h
src/models/CubeModel.o: /usr/include/unistd.h /usr/include/getopt.h
src/models/CubeModel.o: /usr/include/EGL/egl.h /usr/include/EGL/eglplatform.h
src/models/CubeModel.o: /usr/include/KHR/khrplatform.h
src/models/CubeModel.o: /usr/include/X11/Xlib.h /usr/include/X11/X.h
src/models/CubeModel.o: /usr/include/X11/Xfuncproto.h
src/models/CubeModel.o: /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h
src/models/CubeModel.o: /usr/include/X11/keysym.h
src/models/CubeModel.o: /usr/include/X11/keysymdef.h
src/models/CubeModel.o: /usr/local/include/hybris/eglplatformcommon/support.h
src/models/CubeModel.o: /usr/include/assert.h /usr/include/linux/fb.h
src/models/CubeModel.o: /usr/include/linux/types.h
src/models/CubeModel.o: /usr/include/linux/posix_types.h
src/models/CubeModel.o: /usr/include/linux/stddef.h /usr/include/linux/i2c.h
src/models/CubeModel.o: /usr/local/include/android/hardware/gralloc.h
src/models/CubeModel.o: /usr/local/include/android/hardware/hardware.h
src/models/CubeModel.o: /usr/local/include/android/hardware/fb.h
src/models/CubeModel.o: ./inc/EGLRuntime.h /usr/include/EGL/eglext.h
src/models/CubeModel.o: /usr/include/EGL/eglmesaext.h
src/models/CubeModel.o: /usr/include/EGL/eglextchromium.h
src/models/PlaneModel.o: ./inc/models/PlaneModel.h ./inc/VectorTypes.h
src/models/PlaneModel.o: ./inc/Matrix.h ./inc/Mathematics.h ./inc/Platform.h
src/models/PlaneModel.o: /usr/local/include/hybris/hwcomposerwindow/hwcomposer_window.h
src/models/PlaneModel.o: /usr/local/include/hybris/eglplatformcommon/nativewindowbase.h
src/models/PlaneModel.o: /usr/include/string.h /usr/include/features.h
src/models/PlaneModel.o: /usr/include/stdc-predef.h /usr/include/xlocale.h
src/models/PlaneModel.o: /usr/local/include/android/system/window.h
src/models/PlaneModel.o: /usr/local/include/android/cutils/native_handle.h
src/models/PlaneModel.o: /usr/include/errno.h /usr/include/limits.h
src/models/PlaneModel.o: /usr/include/limits.h /usr/include/stdint.h
src/models/PlaneModel.o: /usr/local/include/android/sync/sync.h
src/models/PlaneModel.o: /usr/local/include/android/system/graphics.h
src/models/PlaneModel.o: /usr/include/unistd.h /usr/include/getopt.h
src/models/PlaneModel.o: /usr/include/EGL/egl.h
src/models/PlaneModel.o: /usr/include/EGL/eglplatform.h
src/models/PlaneModel.o: /usr/include/KHR/khrplatform.h
src/models/PlaneModel.o: /usr/include/X11/Xlib.h /usr/include/X11/X.h
src/models/PlaneModel.o: /usr/include/X11/Xfuncproto.h
src/models/PlaneModel.o: /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h
src/models/PlaneModel.o: /usr/include/X11/keysym.h
src/models/PlaneModel.o: /usr/include/X11/keysymdef.h
src/models/PlaneModel.o: /usr/local/include/hybris/eglplatformcommon/support.h
src/models/PlaneModel.o: /usr/include/assert.h /usr/include/linux/fb.h
src/models/PlaneModel.o: /usr/include/linux/types.h
src/models/PlaneModel.o: /usr/include/linux/posix_types.h
src/models/PlaneModel.o: /usr/include/linux/stddef.h /usr/include/linux/i2c.h
src/models/PlaneModel.o: /usr/local/include/android/hardware/gralloc.h
src/models/PlaneModel.o: /usr/local/include/android/hardware/hardware.h
src/models/PlaneModel.o: /usr/local/include/android/hardware/fb.h
src/models/PlaneModel.o: ./inc/EGLRuntime.h /usr/include/EGL/eglext.h
src/models/PlaneModel.o: /usr/include/EGL/eglmesaext.h
src/models/PlaneModel.o: /usr/include/EGL/eglextchromium.h
src/models/SphereModel.o: ./inc/models/SphereModel.h ./inc/VectorTypes.h
src/models/SphereModel.o: ./inc/Platform.h
src/models/SphereModel.o: /usr/local/include/hybris/hwcomposerwindow/hwcomposer_window.h
src/models/SphereModel.o: /usr/local/include/hybris/eglplatformcommon/nativewindowbase.h
src/models/SphereModel.o: /usr/include/string.h /usr/include/features.h
src/models/SphereModel.o: /usr/include/stdc-predef.h /usr/include/xlocale.h
src/models/SphereModel.o: /usr/local/include/android/system/window.h
src/models/SphereModel.o: /usr/local/include/android/cutils/native_handle.h
src/models/SphereModel.o: /usr/include/errno.h /usr/include/limits.h
src/models/SphereModel.o: /usr/include/limits.h /usr/include/stdint.h
src/models/SphereModel.o: /usr/local/include/android/sync/sync.h
src/models/SphereModel.o: /usr/local/include/android/system/graphics.h
src/models/SphereModel.o: /usr/include/unistd.h /usr/include/getopt.h
src/models/SphereModel.o: /usr/include/EGL/egl.h
src/models/SphereModel.o: /usr/include/EGL/eglplatform.h
src/models/SphereModel.o: /usr/include/KHR/khrplatform.h
src/models/SphereModel.o: /usr/include/X11/Xlib.h /usr/include/X11/X.h
src/models/SphereModel.o: /usr/include/X11/Xfuncproto.h
src/models/SphereModel.o: /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h
src/models/SphereModel.o: /usr/include/X11/keysym.h
src/models/SphereModel.o: /usr/include/X11/keysymdef.h
src/models/SphereModel.o: /usr/local/include/hybris/eglplatformcommon/support.h
src/models/SphereModel.o: /usr/include/assert.h /usr/include/linux/fb.h
src/models/SphereModel.o: /usr/include/linux/types.h
src/models/SphereModel.o: /usr/include/linux/posix_types.h
src/models/SphereModel.o: /usr/include/linux/stddef.h
src/models/SphereModel.o: /usr/include/linux/i2c.h
src/models/SphereModel.o: /usr/local/include/android/hardware/gralloc.h
src/models/SphereModel.o: /usr/local/include/android/hardware/hardware.h
src/models/SphereModel.o: /usr/local/include/android/hardware/fb.h
src/models/SphereModel.o: ./inc/EGLRuntime.h /usr/include/EGL/eglext.h
src/models/SphereModel.o: /usr/include/EGL/eglmesaext.h
src/models/SphereModel.o: /usr/include/EGL/eglextchromium.h
src/models/SphereModel.o: ./inc/Mathematics.h
src/models/SuperEllipsoidModel.o: ./inc/models/SuperEllipsoidModel.h
src/models/SuperEllipsoidModel.o: ./inc/VectorTypes.h ./inc/Platform.h
src/models/SuperEllipsoidModel.o: /usr/local/include/hybris/hwcomposerwindow/hwcomposer_window.h
src/models/SuperEllipsoidModel.o: /usr/local/include/hybris/eglplatformcommon/nativewindowbase.h
src/models/SuperEllipsoidModel.o: /usr/include/string.h
src/models/SuperEllipsoidModel.o: /usr/include/features.h
src/models/SuperEllipsoidModel.o: /usr/include/stdc-predef.h
src/models/SuperEllipsoidModel.o: /usr/include/xlocale.h
src/models/SuperEllipsoidModel.o: /usr/local/include/android/system/window.h
src/models/SuperEllipsoidModel.o: /usr/local/include/android/cutils/native_handle.h
src/models/SuperEllipsoidModel.o: /usr/include/errno.h /usr/include/limits.h
src/models/SuperEllipsoidModel.o: /usr/include/limits.h /usr/include/stdint.h
src/models/SuperEllipsoidModel.o: /usr/local/include/android/sync/sync.h
src/models/SuperEllipsoidModel.o: /usr/local/include/android/system/graphics.h
src/models/SuperEllipsoidModel.o: /usr/include/unistd.h /usr/include/getopt.h
src/models/SuperEllipsoidModel.o: /usr/include/EGL/egl.h
src/models/SuperEllipsoidModel.o: /usr/include/EGL/eglplatform.h
src/models/SuperEllipsoidModel.o: /usr/include/KHR/khrplatform.h
src/models/SuperEllipsoidModel.o: /usr/include/X11/Xlib.h
src/models/SuperEllipsoidModel.o: /usr/include/X11/X.h
src/models/SuperEllipsoidModel.o: /usr/include/X11/Xfuncproto.h
src/models/SuperEllipsoidModel.o: /usr/include/X11/Xosdefs.h
src/models/SuperEllipsoidModel.o: /usr/include/X11/Xutil.h
src/models/SuperEllipsoidModel.o: /usr/include/X11/keysym.h
src/models/SuperEllipsoidModel.o: /usr/include/X11/keysymdef.h
src/models/SuperEllipsoidModel.o: /usr/local/include/hybris/eglplatformcommon/support.h
src/models/SuperEllipsoidModel.o: /usr/include/assert.h
src/models/SuperEllipsoidModel.o: /usr/include/linux/fb.h
src/models/SuperEllipsoidModel.o: /usr/include/linux/types.h
src/models/SuperEllipsoidModel.o: /usr/include/linux/posix_types.h
src/models/SuperEllipsoidModel.o: /usr/include/linux/stddef.h
src/models/SuperEllipsoidModel.o: /usr/include/linux/i2c.h
src/models/SuperEllipsoidModel.o: /usr/local/include/android/hardware/gralloc.h
src/models/SuperEllipsoidModel.o: /usr/local/include/android/hardware/hardware.h
src/models/SuperEllipsoidModel.o: /usr/local/include/android/hardware/fb.h
src/models/SuperEllipsoidModel.o: ./inc/EGLRuntime.h
src/models/SuperEllipsoidModel.o: /usr/include/EGL/eglext.h
src/models/SuperEllipsoidModel.o: /usr/include/EGL/eglmesaext.h
src/models/SuperEllipsoidModel.o: /usr/include/EGL/eglextchromium.h
src/models/SuperEllipsoidModel.o: ./inc/Mathematics.h
src/models/TorusModel.o: ./inc/models/TorusModel.h ./inc/VectorTypes.h
src/models/TorusModel.o: ./inc/Platform.h
src/models/TorusModel.o: /usr/local/include/hybris/hwcomposerwindow/hwcomposer_window.h
src/models/TorusModel.o: /usr/local/include/hybris/eglplatformcommon/nativewindowbase.h
src/models/TorusModel.o: /usr/include/string.h /usr/include/features.h
src/models/TorusModel.o: /usr/include/stdc-predef.h /usr/include/xlocale.h
src/models/TorusModel.o: /usr/local/include/android/system/window.h
src/models/TorusModel.o: /usr/local/include/android/cutils/native_handle.h
src/models/TorusModel.o: /usr/include/errno.h /usr/include/limits.h
src/models/TorusModel.o: /usr/include/limits.h /usr/include/stdint.h
src/models/TorusModel.o: /usr/local/include/android/sync/sync.h
src/models/TorusModel.o: /usr/local/include/android/system/graphics.h
src/models/TorusModel.o: /usr/include/unistd.h /usr/include/getopt.h
src/models/TorusModel.o: /usr/include/EGL/egl.h
src/models/TorusModel.o: /usr/include/EGL/eglplatform.h
src/models/TorusModel.o: /usr/include/KHR/khrplatform.h
src/models/TorusModel.o: /usr/include/X11/Xlib.h /usr/include/X11/X.h
src/models/TorusModel.o: /usr/include/X11/Xfuncproto.h
src/models/TorusModel.o: /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h
src/models/TorusModel.o: /usr/include/X11/keysym.h
src/models/TorusModel.o: /usr/include/X11/keysymdef.h
src/models/TorusModel.o: /usr/local/include/hybris/eglplatformcommon/support.h
src/models/TorusModel.o: /usr/include/assert.h /usr/include/linux/fb.h
src/models/TorusModel.o: /usr/include/linux/types.h
src/models/TorusModel.o: /usr/include/linux/posix_types.h
src/models/TorusModel.o: /usr/include/linux/stddef.h /usr/include/linux/i2c.h
src/models/TorusModel.o: /usr/local/include/android/hardware/gralloc.h
src/models/TorusModel.o: /usr/local/include/android/hardware/hardware.h
src/models/TorusModel.o: /usr/local/include/android/hardware/fb.h
src/models/TorusModel.o: ./inc/EGLRuntime.h /usr/include/EGL/eglext.h
src/models/TorusModel.o: /usr/include/EGL/eglmesaext.h
src/models/TorusModel.o: /usr/include/EGL/eglextchromium.h
src/models/TorusModel.o: ./inc/Mathematics.h
src/HWCPlatform.o: ./inc/HWCPlatform.h ./inc/Platform.h
src/HWCPlatform.o: /usr/local/include/hybris/hwcomposerwindow/hwcomposer_window.h
src/HWCPlatform.o: /usr/local/include/hybris/eglplatformcommon/nativewindowbase.h
src/HWCPlatform.o: /usr/include/string.h /usr/include/features.h
src/HWCPlatform.o: /usr/include/stdc-predef.h /usr/include/xlocale.h
src/HWCPlatform.o: /usr/local/include/android/system/window.h
src/HWCPlatform.o: /usr/local/include/android/cutils/native_handle.h
src/HWCPlatform.o: /usr/include/errno.h /usr/include/limits.h
src/HWCPlatform.o: /usr/include/limits.h /usr/include/stdint.h
src/HWCPlatform.o: /usr/local/include/android/sync/sync.h
src/HWCPlatform.o: /usr/local/include/android/system/graphics.h
src/HWCPlatform.o: /usr/include/unistd.h /usr/include/getopt.h
src/HWCPlatform.o: /usr/include/EGL/egl.h /usr/include/EGL/eglplatform.h
src/HWCPlatform.o: /usr/include/KHR/khrplatform.h /usr/include/X11/Xlib.h
src/HWCPlatform.o: /usr/include/X11/X.h /usr/include/X11/Xfuncproto.h
src/HWCPlatform.o: /usr/include/X11/Xosdefs.h /usr/include/X11/Xutil.h
src/HWCPlatform.o: /usr/include/X11/keysym.h /usr/include/X11/keysymdef.h
src/HWCPlatform.o: /usr/local/include/hybris/eglplatformcommon/support.h
src/HWCPlatform.o: /usr/include/assert.h /usr/include/linux/fb.h
src/HWCPlatform.o: /usr/include/linux/types.h
src/HWCPlatform.o: /usr/include/linux/posix_types.h
src/HWCPlatform.o: /usr/include/linux/stddef.h /usr/include/linux/i2c.h
src/HWCPlatform.o: /usr/local/include/android/hardware/gralloc.h
src/HWCPlatform.o: /usr/local/include/android/hardware/hardware.h
src/HWCPlatform.o: /usr/local/include/android/hardware/fb.h
src/HWCPlatform.o: ./inc/EGLRuntime.h /usr/include/EGL/eglext.h
src/HWCPlatform.o: /usr/include/EGL/eglmesaext.h
src/HWCPlatform.o: /usr/include/EGL/eglextchromium.h ./inc/VectorTypes.h
