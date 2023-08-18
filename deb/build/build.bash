#!/bin/bash
SCRIPT_DIR=$(cd $(dirname $0); pwd)
cd $SCRIPT_DIR

git clone https://github.com/Kotakku/FT_SCServo_Debug_Qt.git
cd FT_SCServo_Debug_Qt/
qmake .
make
