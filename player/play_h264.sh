#!/bin/bash
#set -x

BIN=`pwd`/h264player
if [ $(echo $@|grep -w "fd"|wc -l) != "0" ]; then DISP=fd; fi
if [ $(echo $@|grep -w "fbo"|wc -l) != "0" ]; then DISP=fbo; fi
if [ $(echo $@|grep -w "roto"|wc -l) != "0" ]; then DISP=roto; fi
if [ -z "$DISP" ]; then FILE=`pwd`/$1; DISP=roto; else FILE=`pwd`/$2; fi
if [ -z "$2" ]; then FILE=sintel_trailer-1080p.mp4; fi

export DISPLAY=:0.0 || true
sudo su -c "EGL_PLATFORM=hwcomposer $BIN $DISP $FILE"
xrefresh || true
