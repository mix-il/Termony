#!/bin/sh
set -e -x
export LC_CTYPE=C
export TOOL_HOME=/Applications/DevEco-Studio.app/Contents
export OHOS_SDK_HOME=$TOOL_HOME/sdk/default/openharmony
make -C build-hnp
