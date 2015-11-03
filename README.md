# machybris_examples - (C) by mac-l1
examples of how to use machybris in your code - for sharing and study purposes;
these include demo's for androids EGL/GLES and rockchips VPU support in linux

**if you like this, please [donate]( https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=KKWC6YE6G5EZU&item_name=mac_l1&item_number=mac_l1)**

* **test_vpu**:   
simple encoder / decoder example for raw video bitstreams (eg h264, ...) to/from vpu frames (nv21) using vpu; with decode every x-th nv21 frame is converted to tga image file that you can display with eog; use gstreamer, ffmpeg or avconv to decode/ encode raw streams (eg .mov or h264 movie file to h264 video bit stream)

* **test_hwcomposer**:   
original test_hwcomposer of libhybris that shows 'the swirl'; use ./test_hwc to run

* **arm-examples**: rotozoom and framebufferobject   
ported RotoZoom and FrameBufferObject EGL/GLES examples from ARM that shows a rotating image (use ./roto.sh to run) and a rotating cube (use .fbo.sh to run) to demo some GLES magic

* **recorder**: h264recorder   
my h264recorder that captures the 1920x1020 framebuffer/screen into a raw h264 bytestream movie using rockchips vpu and rkon2 libs; use "./screenrecord_h264.sh mymovie.mp4" to run; i used this to record my [youtube demo](https://www.youtube.com/watch?v=PLHsnUpak5Q)

* **player**: h264player and h265player   
my h264player that plays h264 movies (only 1920x1080 format is hard-coded!) to either simple framebuffer/screen or using EGL/GLES for some rotating magic with the movie frames; it uses rockchips vpu and rkon2 libs and androids EGL/GLES lib; use "./play_h264.sh [fd|roto|fbo] [movie.name]" to play; i used this in my [youtube demos](http://freaktab.com/forum/tv-player-support/rk3288-devices/494410-accelerated-video-video-processor-vpu-running-on-linux-on-rk3288-firefly).
also added a h265player that plays hevc/h265 moviess (only 1920x1080 is supported!)

# cheers! - mac-l1
**and again, if you like this, please [donate]( https://www.paypal.com/cgi-bin/webscr?cmd=_donations&business=KKWC6YE6G5EZU&item_name=mac_l1&item_number=mac_l1)**
