#!/bin/bash

#gst-launch-1.0 filesrc location=$1 ! qtdemux  ! h264parse ! video/x-h264, stream-format=byte-stream, alignment=nal, parsed=true ! filesink location=$1.h264
avconv -i $1 -c:v copy -bsf:v h264_mp4toannexb -an $1.h264
