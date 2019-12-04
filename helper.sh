#!/usr/bin/bash
unset PYTHONHOME
unset PYTHONPATH
rm -rf tmp
mkdir tmp
../Rack-SDK/helper.py createmodule $1 res/$1.svg tmp/$1.cpp