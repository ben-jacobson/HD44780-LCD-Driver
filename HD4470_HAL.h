#ifndef HD4470_HAL
#define HD4470_HAL

#include "mbed.h"
#include "platform/Stream.h"

union data_bus_or_port {      // we give the user the choice of using a PortOut, or a BusOut to give flexibility over which pins are used for the data bus. 
    PortOut *data_port; 
    BusOut *data_bus;
};

class HDD4470_HAL  : public Stream {
/* 
*  Specifically designed for use with the Adafruit LCD 16x2 screen which uses the HD44780 driver chip. 
*  Certain delay times are needed for error free operation, and there are a lot of basic functions to help get the device up and running
*     
*/
    public:
    static constexpr uint8_t display_width = 16; 
    static constexpr uint8_t display_height = 2;

    bool screen_wrap; 

    /*
    *  Class constructor requires you to not only define the individual pins you want to use for RS, WR and EN, but also a complete 8 bit pin Port is used for the data port. 
    */
    HDD4470_HAL(PortOut* _data_pins_port, PinName _register_select_pin, PinName _read_write_pin, PinName _enable_pin, bool initialize_with_default = true);
    HDD4470_HAL(BusOut* _data_pins_bus, PinName _register_select_pin, PinName _read_write_pin, PinName _enable_pin, bool initialize_with_default = true);
    void initialize_display(bool initialize_with_default);
    void clear(void);

    // Stream implementation - provides printf() interface to write to display
    virtual int _putc(int value) { return writeChar(value); };
    virtual int _getc() { return -1; };

    inline void new_line(void);
    size_t writeChar(uint8_t c);
    void set_cursor_position(uint8_t x, uint8_t y); // update the display cursor position, and also in mcu memory. 

    private:   

    // IO pins
    //PortOut data_port;
    data_bus_or_port data_pins;  // a union for storing either a bus or a port for the data pins
    bool using_bus; 

    DigitalOut register_select_pin, read_write_pin, enable_pin; 
    uint8_t register_selected; 

    // Timing delays
    static constexpr Kernel::Clock::duration_u32 MCU_LATENCY_MS = 1ms; // it takes 4us for digitalWrite to complete it's function, when you need exact timing, use this to offset the timing you want.   
    static constexpr Kernel::Clock::duration_u32 CLEAR_DELAY_MS = 2ms; // clear display and return home instructions take 1.53ms to execute
    static constexpr Kernel::Clock::duration_u32 INSTRUCTION_DELAY_MS = 1ms; // most instructions take 45us to execute, we'll force the MCU to wait

    // Commonly used instructions for this device
    enum LCD_Instructions: uint8_t {
        instr_clear_disp                    = 0b00000001, // 0x01 clears the display entirely. 
        instr_return_home                   = 0b00000010, // 0x02 returns the cursor to the home position
        instr_display_on                    = 0b00001111, // 0x0F Display ON, Cursor On, Cursor Blinking.
        instr_display_on_no_cursor_blink    = 0b00001100, // 0x0F Display ON, Cursor Off, Cursor Not Blinking.
        instr_entry_mode                    = 0b00000110, // 0x06 Entry Mode, Increment cursor position, No display shift. change to B00000100 to disable automatic cursor increment
        instr_fn_set                        = 0b00111000, // 0x38 Function set, 8 bit mode, 2 lines, 5×8 font.
        instr_move_cursor_left              = 0b00010000, // 0x10 move cursor left
        instr_move_cursor_right             = 0b00010100  // 0x14 move cursor right        
    };

    static constexpr uint8_t INSTRUCTION_REGISTER = 0;
    static constexpr uint8_t DATA_REGISTER = 1;  

    // positions used for moving to start of line 
    static constexpr uint8_t FIRST_LINE =  0b10000000; 
    static constexpr uint8_t SECOND_LINE = 0b11000000;      

    // x and y values of cursor, when using set_cursor this updates it in memory, but also in the device. 
    uint8_t cursor_x, cursor_y;

    void send_data(uint8_t instr, uint8_t register_sel); 
    inline void select_instruction_register(void);   
    inline void select_data_register(void);   
    inline void pulse_enable(void);
};

#endif