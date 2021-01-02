#include "mbed.h"
#include "HD4470_HAL.h"

int main() {
/*
    D5-D12 = Data bus D0-D7
    D2 = Register Select
    D3 = Read/Write
    D4 = Enable
*/ 
// PortOut LCD_display_data_bus(PortA); if you want to use a Port, do this
BusOut LCD_display_data_bus(D5, D6, D7, D8, D9, D10, D11, D12); // Otherwise, do this. 
HDD4470_HAL LCD_Display(&LCD_display_data_bus, D2, D3, D4);

    LCD_Display.set_cursor_position(3, 0); // update the display cursor position, and also in mcu memory. 
    int test_num = 5; 
    LCD_Display.printf("Hello World\r\n!!! %d", test_num);

    while (true) {
        // Do nothing
    }
}

