#!/bin/bash
set -x

sudo rm -rf $1.h264 $1 
sudo ./h264recorder $1.h264
avconv -i $1.h264 -vcodec copy $1
sudo rm -rf $1.h264  
