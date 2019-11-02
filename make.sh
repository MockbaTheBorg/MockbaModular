#!/usr/bin/bash
RACK_DIR=/home/mockba/VCV/Rack-SDK make $1
strip plugin.dll
