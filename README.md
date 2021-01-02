# HD44780 ARM Mbed LCD Driver Code

Ben Jacobson - January 2021

ARM Mbed code for use with Ada Fruit HD44780-LCD-Driver. Developed this on an STM32F303K8, but should theoretically work on just about any ARM Cortex based MCU. 

Datasheet: https://www.sparkfun.com/datasheets/LCD/HD44780.pdf
Product: https://www.adafruit.com/product/181

I wrote this code when I was working on a hardware project in Sept 2017 - it was a circuit board with an ATMega328P hooked up to an IR encoder. The device would let you type in Hex code and have an IR Blaster circuit transmit it out. Essentially creating a field-programmable remote control for IT/AV professionals. Code specific to that project hasn't been included. A few years later I needed this code for an STM32 project, so I ported it over and made some improvements. Feel free to check out previous versions of this to see the Arduino version of the code. 

Usage: See main.cpp for example of how to use. 

The HDD4470_HAL class is a simple interface with which you can write to the LCD screen. It inherits from the Stream class which allows you to use printf like statements just like you would anywhere else in C.




