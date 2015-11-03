#!/bin/bash
set -x

if [ $(echo $@|grep -w "clean"|wc -l) != "0" ]; then
  rm -rf h264recorder
  exit
fi

#h264recorder
g++ -DTARGET_RK32 -I ../Mindroid.cpp -L ../Mindroid.cpp -L /usr/local/hybris/lib/ -I ../ffmpeg/ffmpeg-install/include -L ../ffmpeg/ffmpeg-install/lib/ -o h264recorder h264recorder.cpp hwc_copybit.cpp fbtools.c Timer.cpp -lmindroid -lpthread -lvpu ../ffmpeg/ffmpeg-install/lib/libavformat.a ../ffmpeg/ffmpeg-install/lib/libavcodec.a ../ffmpeg/ffmpeg-install/lib/libavutil.a ../ffmpeg/ffmpeg-install/lib/libswscale.a ../ffmpeg/ffmpeg-install/lib/libswresample.a $(pkg-config  --static --libs-only-l --silence-errors gnutls) $(pkg-config --static --libs-only-l --silence-errors vorbisenc) $(PKG_CONFIG_PATH=../ffmpeg/ffmpeg-install/lib/pkgconfig pkg-config --static --libs-only-l --silence-errors libavcodec)

