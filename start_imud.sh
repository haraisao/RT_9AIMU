#!/bin/bash

SCRIPT_DIR=$(cd $(dirname $0);pwd)
CMD="${SCRIPT_DIR}/build/imud -d $*"

sudo ${CMD}
#sudo ${SCRIPT_DIR}/build/imud -d $*
