#!/bin/bash
set -x # verbose

export ARCH=arm
SCRIPT_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
cd $SCRIPT_DIR

sudo apt-get install -y --force-yes build-essential xutils-dev git autoconf libtool pkg-config curl libgnutls-dev libvorbis-dev libxslt-dev 

cd test_vpu
./make.sh $@
cd $SCRIPT_DIR

cd test_hwcomposer
./make.sh $@
cd $SCRIPT_DIR

cd simple_framework
./make.sh $@
cd $SCRIPT_DIR

cd arm-examples
./make.sh $@
cd $SCRIPT_DIR

if [ $(echo $@|grep -w "clean"|wc -l) != "0" ]; then
    rm -rf Mindroid.cpp 
else
if ! [ -e Mindroid.cpp ]; then
  git clone https://github.com/esrlabs/Mindroid.cpp.git
  cd Mindroid.cpp
  git checkout 0e0371fc764168660018a8e8643a4640fbad5ae7
  sed 's/-D__ARMv6_CPU_ARCH__ -fPIC -O2/-fPIC -O2 -Wa,-mimplicit-it=thumb/g' Makefile.RPi > Makefile.ARM
  make -f Makefile.ARM
  sudo cp libmindroid.so /usr/local/lib
fi
fi
cd $SCRIPT_DIR

if ! [ -x /usr/bin/avconv ]; then
  sudo apt-get install libav-tools
fi

cd ffmpeg
./make.sh $@
cd ..

cd recorder
./make.sh $@
cd $SCRIPT_DIR

cd player
./make.sh $@
cd $SCRIPT_DIR

echo "now downloading some movies"
echo "press ctrl-c to abort"

cd media
./make.sh
cd $SCRIPT_DIR

exit
