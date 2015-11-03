#!/bin/bash
set -x

if [ $(echo $@|grep -w "clean"|wc -l) != "0" ]; then
  rm -rf test_vpu
  exit
fi

gcc -o test_vpu test_vpu.c -lvpu
