# Mockba Modular
 Mockba's modules for VCV Rack

This is a small collection of modules I have created out of pure necessity.<br>
I believe these modules might prove useful to others. Please feel free to use them.

![Alt text](./screenshot.png)

## Feidah
Small fader (attenuator) module I use to control volume levels, etc.<br>
This is actually the first module I made, evah!

## Dividah
Simple clock divider based on inverse counting, so all the clock outputs will start in sync.

## Countah
Simple binary counter/divider, can be used as divider but the clock outputs won't start in sync.

## UDPClockMaster (Mastah)
Clock/Reset UDP broadcaster.<br>
This module receives a clock and a reset signals as inputs and then broadcasts them via UDP port 7000, to allow for synchronization of other VCV Rack instances running on different computers at the same network.<br>
Best results are achieved if using Ethernet, not Wifi.<br>
Right now it broadcasts to 192.168.1.255 only, one would need to change this in the source code and recompile if they want to use a different IP broadcast address.<br>
The little status light illuminates red if there's any issue establishing the UDP socket.

## UDPClockSlave (Slavah)
Clock/Reset receiver.<br>
This module receives the UDP broadcasts on port 7000 and generates corresponding Clock and Reset signals.<br>
The idea here is to have one Slavah (only one) running on each VCV computer that is going to be synced to the master.<br>
The little status light illuminates red if there's any issue establishing the UDP socket.

## Building
Just follow the VCV recommendations to build plugins.<br>
I have provided a premake5 lua setup I use, which creates a VSTudio solution/project to allow editing the source code on Visual Studio. Just double click on setup.cmd to create the solution or on clean.cmd to clean it up.<br>
The Visual Studio solution and project files are created on a VStudio sub folder.<br>
Build still needs to be done via Makefile on the msys2 shell (for Windows) or the respective methods for Linux and Mac. I have provided also a make.sh script I (have to) use to execute the make command.

## ToDo list for the UDP modules
. Add some offset adjust knob to Slavah to compensate for network delay.<br>
. Add an option to set the broadcast IP address of Mastah manually.

## Issues
Sometimes the Slave won't start when added, if this is the case then close the patch and open it again. This normally fixes the issue.<br>
I am still trying to figure out the reason for this.<br>
<br>
## Disclaimer
THESE MODULES ARE ALL WORK IN PROGRESS, EXPECT MISBEHAVIORS!<br>
Usage of these modules might cause issues like electricity spikes, cpu core meltdowns or planetary energy disruptions, be advised.<br>
Help, suggestions and of course donations are much welcome.<br>
