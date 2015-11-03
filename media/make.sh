#!/bin/bash
set -x

# h264 1080p
if ! [ -e sintel_trailer-1080p.mp4 ]; then
  wget https://download.blender.org/durian/trailer/sintel_trailer-1080p.mp4
fi

# 264 1080p
if ! [ -e big_buck_bunny_1080p_h264.mov ]; then
  wget http://download.blender.org/peach/bigbuckbunny_movies/big_buck_bunny_1080p_h264.mov
fi

# hevc/h265 1080p
if ! [ -e bbb_1080p_c.ts ]; then
  wget http://www.elecard.com/assets/files/other/clips/bbb_1080p_c.ts
fi

# hevc/h265 1080p
if ! [ -e elephants-dream-1080-cfg02.mkv ]; then
  wget http://www.libde265.org/hevc-bitstreams/elephants-dream-1080-cfg02.mkv
fi

# hevc/h265 1080p
if ! [ -e sintel-1080-cfg02.mkv ]; then
  wget http://www.libde265.org/hevc-bitstreams/sintel-1080-cfg02.mkv
fi 
