#!/bin/bash
#set -x

BIN=`pwd`/h265player
if [ $(echo $@|grep -w "fd"|wc -l) != "0" ]; then DISP=fd; fi
if [ $(echo $@|grep -w "fbo"|wc -l) != "0" ]; then DISP=fbo; fi
if [ $(echo $@|grep -w "roto"|wc -l) != "0" ]; then DISP=roto; fi
if [ $(grep -i RK3368 < /proc/cpuinfo|wc -l) != "0" ]; then
if [ -z "$DISP" ]; then FILE=`pwd`/$1; DISP=fd; else FILE=`pwd`/$2; fi
else
if [ -z "$DISP" ]; then FILE=`pwd`/$1; DISP=roto; else FILE=`pwd`/$2; fi
fi
if [ -z "$2" ]; then FILE=bbb_1080p_c.ts; fi

export DISPLAY=:0.0 || true
sudo su -c "EGL_PLATFORM=hwcomposer $BIN $DISP $FILE"
xrefresh || true
