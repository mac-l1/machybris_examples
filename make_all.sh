#!/bin/bash
set -x # verbose
SCRIPT_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

# init bin dir for output
cd $SCRIPT_DIR
mkdir -p bin

cd test_vpu
./make.sh
cp -a test_vpu $SCRIPT_DIR/bin
cd $SCRIPT_DIR

exit
