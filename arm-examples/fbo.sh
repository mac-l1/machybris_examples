#!/bin/bash
BIN=`pwd`/framebufferobject
export DISPLAY=:0.0 || true
sudo su -c "EGL_PLATFORM=hwcomposer $BIN; chvt 7"
xrefresh || true
