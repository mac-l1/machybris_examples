#!/bin/bash
set -x

if [ $(echo $@|grep -w "clean"|wc -l) != "0" ]; then
  rm -rf ffmpeg-2.6.4-Isengard* ffmpeg-install 
  exit
fi

if [ ! -d "ffmpeg-install" ]; then
  if [ ! -x "$(command -v curl)" ]; then sudo apt-get install curl; fi
  if [ ! -x "$(command -v lsb_release)" ]; then sudo apt-get install lsb-release; fi
  LSB_CODE=`lsb_release -c|awk -F: '{print $NF}'|tr -d '[[:space:]]'`
  curl -O http://mac-l1.com/ffmpeg/ffmpeg-install-2.6.4-Isengard-${LSB_CODE}.zip
  if [ -e ffmpeg-install-2.6.4-Isengard-${LSB_CODE}.zip ]; then
    unzip ffmpeg-install-2.6.4-Isengard-${LSB_CODE}.zip
  else
    CFLAGS="-mfloat-abi=hard -mfpu=neon -ftree-vectorize -mvectorize-with-neon-quad -ffast-math -O3 -pipe -fstack-protector" ./autobuild.sh -r --disable-optimizations --arch=armv7
  fi
fi

