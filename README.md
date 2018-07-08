# HD44780 LCD Driver Wrapper Code

Ben Jacobson - September 2017

Arduino code for use with Ada Fruit HD44780-LCD-Driver - 
Datasheet: https://www.sparkfun.com/datasheets/LCD/HD44780.pdf
Product: https://www.adafruit.com/product/181

The code does two things:
1. The "LCD_Driver" class is a simple interface with which you can write to the LCD screen, eg. lcd-screen.write_line("Hello World", 0, 0);. This code is probably the most useful for people wishing to adopt this into their own projects. 
2. The "screen_interface" class gives you the ability to set up menus and scroll left/right, up/down etc. This part of the code is a bit more specific so less likely to be useful to you without augmentation. This part of the code might be missing a few features because this code was part of a bigger project, at the time I had everything I needed so did not continue any further with this.  

I wrote this code when I was working on a hardware project - it was a circuit board with an ATMega328P hooked up to an IR encoder. The device would let you type in Hex code and have an IR Blaster circuit transmit it out. Essentially creating a field-programmable remote control for IT/AV professionals. Code specific to that project hasn't been included.
