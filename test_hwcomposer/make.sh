#!/bin/bash
set -x

if [ $(echo $@|grep -w "clean"|wc -l) != "0" ]; then
  rm -rf test_hwcomposer
  exit
fi

g++ -I /usr/local/include/android/ -I /usr/local/include/hybris/hwcomposerwindow/ -I /usr/local/include/hybris/eglplatformcommon/ -L /usr/local/lib/ -o test_hwcomposer test_hwcomposer.cpp -l hybris-common -l hybris-hwcomposerwindow -l hybris-eglplatformcommon -l EGL -l sync -l hardware -l GLESv2
