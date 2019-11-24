# Mockba Modular
 Mockba's modules for VCV Rack

This is a small collection of modules I have created out of pure necessity.<br>
I believe these modules might prove useful to others. Please feel free to use them.

![Alt text](./screenshot.png)

## Blank
A simple logo blank just because everyone else seems to have one.

## Feidah
Small fader (attenuator) module I use to control volume levels, etc.<br>
This is actually the first module I made, evah!

## Mixah
Small two signal mixer (adder) for voltages.

## Dividah
Simple clock divider based on inverse counting, so all the clock outputs will start in sync.

## Countah
Simple binary counter/divider, can be used as divider but the clock outputs won't start in sync.

## Selectah
Simple 4 to 1 selector, used to select waves from a 4 output oscillator.

## UDPClockMaster (Mastah)
Clock/Reset UDP broadcaster.<br>
This module receives a clock and a reset signals as inputs and then broadcasts them via UDP port 7000, to allow for synchronization of other VCV Rack instances running on different computers at the same network.<br>
Best results are achieved if using Ethernet, not Wifi.<br>
By default it broadcasts to 192.168.1.255. If a different network is needed, one would need to change this in a JSON file that is created on Rack's documents folder when the plugin is first loaded.<br>
From that moment on the plugin will always load the default IP address from the JSON file.<br>
The little status light illuminates red if there's any issue establishing the UDP socket.

## UDPClockSlave (Slavah)
Clock/Reset receiver.<br>
This module receives the UDP broadcasts on port 7000 and generates corresponding Clock and Reset signals.<br>
The idea here is to have one Slavah (only one) running on each VCV computer that is going to be synced to the master.<br>
The little status light illuminates red if there's any issue establishing the UDP socket.

# CZ Series
![Alt text](./screenshot2.png)

These modules recreate some of the classic Casio CZ series oscillators.

## Building
Just follow the VCV recommendations to build plugins.<br>
I have provided a premake5 lua setup I use, which creates a VSTudio solution/project to allow editing the source code on Visual Studio. Just double click on setup.cmd to create the solution or on clean.cmd to clean it up.<br>
The Visual Studio solution and project files are created on a VStudio sub folder.<br>
Build still needs to be done via Makefile on the msys2 shell (for Windows) or the respective methods for Linux and Mac. I have provided also a make.sh script I (have to) use to execute the make command.

## ToDo list for the UDP modules
. Add some offset adjust knob to Slavah to compensate for network delay.<br>

## Issues
Sometimes the Slave won't start when added, if this is the case then close the patch and open it again. This normally fixes the issue.<br>
I am still trying to figure out the reason for this.<br>
<br>
## Disclaimer
THESE MODULES ARE ALL WORK IN PROGRESS, EXPECT MISBEHAVIORS!<br>
Usage of these modules might cause issues like electricity spikes, cpu core meltdowns or planetary energy disruptions, be advised.<br>
Help, suggestions and of course donations are much welcome.<br>
