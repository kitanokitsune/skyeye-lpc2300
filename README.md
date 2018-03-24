# skyeye-lpc2300
**SKYEYE Simulator for LPC2300**

## What is this
This is a branch of [SkyEye ARM simulator](https://sourceforge.net/projects/skyeye/). Base SkyEye version is [1.2.6_rc1](https://sourceforge.net/projects/skyeye/files/skyeye/skyeye-1.2.6_rc1/).

The aim of this project is to support NXP's LPC23xx, especially **CQ-FRK-NXP-ARM** board. Now the following functions are implemented:

* UART(0/1/2/3) function and Rx/Tx interrupt
* TIMER(0/1/2/3) function and MR0/1/2/3 interrupt

## How to build
### For Linux
just type **./configure** and **make**.
### For MS-Windows
Simply use **skyeye.exe** in **bin/** directory.
Or, if you want to build yourself, you need MinGW/MSYS environment. Type the following commands on MSYS shell.

* **./configure --enable-lcd=no LDFLAGS="-static -static-libstdc++ -static-libgcc -s"**
* **make**

## Known Issue
I confirmed KEIL's sample __*Interrupt-Driven UART I/O for Philips LPC23xx*__ works fine on the simulator. But some programs have problems. I guess the cause is related to CPSR/SPSR mode switching.
