#!/bin/bash
set -x

if [ $(echo $@|grep -w "clean"|wc -l) != "0" ]; then 
  rm -rf rotozoom framebufferobject
  exit
fi

#rotozoom
g++ -DEGL_EGLEXT_PROTOTYPES -I ../simple_framework/inc/ -I /usr/local/hybris/include/ -I /usr/local/hybris/include/hybris/hwcomposerwindow/ -I /usr/local/hybris/include/hybris/eglplatformcommon/ -I /usr/local/hybris/include/android/ -I ../Mindroid.cpp -L ../Mindroid.cpp -L /usr/local/hybris/lib/ -I ../ffmpeg/ffmpeg-install/include -o rotozoom RotoZoom.cpp ../simple_framework/libsimple_framework2.a -lEGL -lGLESv2 -lhybris-common -lhybris-hwcomposerwindow -lhybris-eglplatformcommon -lsync -lhardware 

#framebufferobject
g++ -DEGL_EGLEXT_PROTOTYPES -I ../simple_framework/inc/ -I /usr/local/hybris/include/ -I /usr/local/hybris/include/hybris/hwcomposerwindow/ -I /usr/local/hybris/include/hybris/eglplatformcommon/ -I /usr/local/hybris/include/android/ -I ../Mindroid.cpp -L ../Mindroid.cpp -L /usr/local/hybris/lib/ -I ../ffmpeg/ffmpeg-install/include -o framebufferobject FrameBufferObject.cpp hwc_copybit.cpp ../simple_framework/libsimple_framework2.a -lEGL -lGLESv2 -lhybris-common -lhybris-hwcomposerwindow -lhybris-eglplatformcommon -lsync -lhardware 
