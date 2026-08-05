#!/usr/bin/bash
[ -f .gitmodules ] && git submodule update --init --recursive
RACK_DIR=/home/mockba/Coding/VCV/Rack-SDK make EXTRA_CXXFLAGS=-Wno-deprecated-declarations $1
[ -f plugin.dll ] && strip plugin.dll
[ -f plugin.so ] && strip plugin.so
