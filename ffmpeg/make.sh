#!/bin/bash
set -x

if [ $(echo $@|grep -w "clean"|wc -l) != "0" ]; then
  rm -rf ffmpeg-2.6.4-Isengard* ffmpeg-install 
  exit
fi

if [ ! -d "ffmpeg-install" ]; then
  CFLAGS="-mfloat-abi=hard -mfpu=neon -ftree-vectorize -mvectorize-with-neon-quad -ffast-math -O3 -pipe -fstack-protector" ./autobuild.sh -r --disable-optimizations --arch=armv7
fi

