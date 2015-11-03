#!/bin/bash
BIN=`pwd`/rotozoom
export DISPLAY=:0.0 || true
sudo su -c "EGL_PLATFORM=hwcomposer $BIN; chvt 7"
xrefresh || true
