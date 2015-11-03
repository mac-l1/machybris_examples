#!/bin/bash
set -x

if [ $(echo $@|grep -w "clean"|wc -l) != "0" ]; then
  rm -rf h265player h264player
  exit
fi

#h265player (using kodi 15.2 static ffmpeg libs that support hevc/h265)
g++ -DTARGET_RK32 -DDEBUG -DEGL_EGLEXT_PROTOTYPES -I ../simple_framework/inc/ -I /usr/local/hybris/include/ -I /usr/local/hybris/include/hybris/hwcomposerwindow/ -I /usr/local/hybris/include/hybris/eglplatformcommon/ -I /usr/local/hybris/include/android/ -I ../Mindroid.cpp -I ../ffmpeg/ffmpeg-install/include/ -L ../ffmpeg/ffmpeg-install/lib/ -L ../Mindroid.cpp -L /usr/local/hybris/lib/ -o h265player h265player.cpp hwc_copybit.cpp fbtools.c -lmindroid -lpthread ../simple_framework/libsimple_framework2.a -lEGL -lGLESv2 -lvpu -lhybris-common -lhybris-hwcomposerwindow -lhybris-eglplatformcommon -lsync -lhardware ../ffmpeg/ffmpeg-install/lib/libavformat.a ../ffmpeg/ffmpeg-install/lib/libavcodec.a ../ffmpeg/ffmpeg-install/lib/libavutil.a ../ffmpeg/ffmpeg-install/lib/libswscale.a ../ffmpeg/ffmpeg-install/lib/libswresample.a $(pkg-config  --static --libs-only-l --silence-errors gnutls) $(pkg-config --static --libs-only-l --silence-errors vorbisenc) $(PKG_CONFIG_PATH=../ffmpeg/ffmpeg-install/lib/pkgconfig pkg-config --static --libs-only-l --silence-errors libavcodec)

#h264player (using kodi 15.2 static ffmpeg libs that support hevc/h265)
g++ -DTARGET_RK32 -DDEBUG -DEGL_EGLEXT_PROTOTYPES -I ../simple_framework/inc/ -I /usr/local/hybris/include/ -I /usr/local/hybris/include/hybris/hwcomposerwindow/ -I /usr/local/hybris/include/hybris/eglplatformcommon/ -I /usr/local/hybris/include/android/ -I ../Mindroid.cpp -I ../ffmpeg/ffmpeg-install/include/ -L ../ffmpeg/ffmpeg-install/lib/ -L ../Mindroid.cpp -L /usr/local/hybris/lib/ -o h264player h264player.cpp hwc_copybit.cpp fbtools.c -lmindroid -lpthread ../simple_framework/libsimple_framework2.a -lEGL -lGLESv2 -lvpu -lhybris-common -lhybris-hwcomposerwindow -lhybris-eglplatformcommon -lsync -lhardware ../ffmpeg/ffmpeg-install/lib/libavformat.a ../ffmpeg/ffmpeg-install/lib/libavcodec.a ../ffmpeg/ffmpeg-install/lib/libavutil.a ../ffmpeg/ffmpeg-install/lib/libswscale.a ../ffmpeg/ffmpeg-install/lib/libswresample.a $(pkg-config  --static --libs-only-l --silence-errors gnutls) $(pkg-config --static --libs-only-l --silence-errors vorbisenc) $(PKG_CONFIG_PATH=../ffmpeg/ffmpeg-install/lib/pkgconfig pkg-config --static --libs-only-l --silence-errors libavcodec)

#h264player (using default distributions ffmpeg shared libs)
#g++  -DTARGET_RK32 -DDEBUG -DEGL_EGLEXT_PROTOTYPES -I ../simple_framework/inc/ -I /usr/local/hybris/include/ -I /usr/local/hybris/include/hybris/hwcomposerwindow/ -I /usr/local/hybris/include/hybris/eglplatformcommon/ -I /usr/local/hybris/include/android/ -I ../Mindroid.cpp -L ../Mindroid.cpp -L /usr/local/hybris/lib/ -o h264player h264player.cpp hwc_copybit.cpp fbtools.c -lmindroid -lpthread ../simple_framework/libsimple_framework2.a -lEGL -lGLESv2 -lvpu -lhybris-common -lhybris-hwcomposerwindow -lhybris-eglplatformcommon -lsync -lhardware -lavformat -lavcodec -lavutil -lswscale  

cd ../media
if ! [ -e sintel_trailer-1080p.mp4 ]; then 
  wget https://download.blender.org/durian/trailer/sintel_trailer-1080p.mp4
fi

if ! [ -e bbb_1080p_c.ts ]; then
  wget http://www.elecard.com/assets/files/other/clips/bbb_1080p_c.ts
fi
cd ../player
