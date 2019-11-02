#!/usr/bin/bash
unset PYTHONHOME
unset PYTHONPATH
rm -rf tmp
mkdir tmp
../Rack-SDK/helper.py createmodule Feidah res/Feidah.svg tmp/Feidah.cpp
../Rack-SDK/helper.py createmodule Dividah res/Dividah.svg tmp/Dividah.cpp
../Rack-SDK/helper.py createmodule UDPClockMaster res/UDPClockMaster.svg tmp/UDPClockMaster.cpp
../Rack-SDK/helper.py createmodule UDPClockSlave res/UDPClockSlave.svg tmp/UDPClockSlave.cpp