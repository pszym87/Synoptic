# Introduction

Synoptic is an _embedded_ project programmed in the **C language**. The project hardware consists of NUCLEO64-F476RG board, EEPROM 24LC01 memory, TFT 1.8 inch LCD screen SP35S, UART-USB PL2303 converter and barometric sensor LPS25HB.

There are several parts to this project, including: wiring hardware components to the breadboard, developing low level libraries for peripheral devices, coding higher level functionalities. 

## Features:

Synoptic offers basic interaction between system components and the final user. At present, it is equipped with the following functionalities:

- debugging and programming the device through CLI (Command-Line Interface)
- performing real-time measurements and calculations of: temperature, relative and absolute atmospheric pressure, altitude at location
- raw readings are transformed into image and text output and then displayed on the LCD screen
- memory read and write on request  
- programmable alarm; when triggered the device performs measurements and writes them into the memory
- RTC for time stamp generation and setting the alarm time
- Watchdog

Microcontroller uses **I2C** protocol to communicate with EEPROM and LPS25HB (they share SDA and SCL lines), **SPI** for TFT display and **UART** for debugging.

## Circuit Diagram and Connections

Connections between hardware modules are presented in the picture below:

![Untitled](https://user-images.githubusercontent.com/106841261/179467229-f1d3b543-00c1-4221-a203-384294190a52.png)

## Higher Level Functionality

__Subprogram 1: ‘Live mode’__
Program reads current temperature and atmospheric pressure, then calculates an altitude. Data are shown on LCD display with basic graphics.
__Subprogram 2: ‘History mode’__
Program reads measurement history stored in EEPROM and presents it on the display. 
__Continuous program__
Independently from selected mode system can:
- trigger an alarm in interruption mode. Which writes current measurement and timestamp to EEPROM
- handle console input/output (printing and reading to the console is realized in the interruption & DMA mode)
Synoptic is programmable through CLI (Command-Line Interface).

### CLI instructions

Below are the list of instructions with specified format which allows to control the device:

### Memory management

![memory](https://user-images.githubusercontent.com/106841261/181515029-b98d9a48-06e9-48ab-839e-06e28e17329d.png)

The capacity of 24LC01 is 128B. 35B of it are reserved for storing measurement history. This segment starts at address 0x00 and ends at 0x22.

Single record consists of 7B of information. 1B each for: day, month, year, hours, minutes, integer part of temperature, decimal part of temperature.

Reserved memory allows for storage of up to 5 records. After that, the oldest record will be overwritten and so on (i.e. after writing to 0x22 memory pointer moves to 0x00)

The system determines where to write next record by reading the memory pointer, which is stored in a memory cell at address 0x7f. It points to memory location to write the first byte of the next record.

### Debugging

During the system initialization phase, the value of various flags are printed on the console. The watchdog flag indicates a reset caused by unexpected system crash. Other flags are related to peripherals initialization. 

Instructions `printmem`, `memr`, `refhis` and `save` make up for the rest of the debugging functionality. 

## Demonstration pic
![demo](https://user-images.githubusercontent.com/106841261/179476986-7a420d2d-3704-40a7-828b-27d68e177792.jpg)
