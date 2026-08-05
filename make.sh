#!/usr/bin/bash
RACK_DIR=/home/mockba/Coding/VCV/Rack-SDK make $1
[ -f plugin.dll ] && strip plugin.dll
[ -f plugin.so ] && strip plugin.so

