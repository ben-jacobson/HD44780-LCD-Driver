#include "HD4470_HAL.h"


// constructor for using with a Port
HDD4470_HAL::HDD4470_HAL(PortOut* _data_pins_port, PinName _register_select_pin, PinName _read_write_pin, PinName _enable_pin, bool initialize_with_default)
:   register_select_pin(_register_select_pin), 
    read_write_pin(_read_write_pin), 
    enable_pin(_enable_pin)
{
    // set the flag to make sure we are using the port
    this->data_pins.data_port = _data_pins_port;
    using_bus = false;  // I really wish C++ had an easy way to determine which member variable in a union is being used. As far as I know the only way is to set your own flag. Lame..

    // just for DRY
    this->initialize_display(initialize_with_default);
}

// constructor for using with a Bus
HDD4470_HAL::HDD4470_HAL(BusOut* _data_pins_bus, PinName _register_select_pin, PinName _read_write_pin, PinName _enable_pin, bool initialize_with_default)
:   register_select_pin(_register_select_pin), 
    read_write_pin(_read_write_pin), 
    enable_pin(_enable_pin)
{
    // set the flag to state we are using the bus
    using_bus = true; 
    this->data_pins.data_bus = _data_pins_bus;

    this->initialize_display(initialize_with_default); 
}

void HDD4470_HAL::initialize_display(bool initialize_with_default) {
    // initialise LCD screen values
    this->register_selected = this->INSTRUCTION_REGISTER;  
    this->enable_pin = 0; // init as low so that it doesn't init anything left over from the buffer. 
    this->register_select_pin = this->register_selected;  // start with instruction register
    this->read_write_pin = 0;  // we set this low = write mode, since we rarely read from the screen. You could also just ground this pin. 

    if (initialize_with_default == true)  // pass a false through the class constructor to override this, then you can use your own initialize methods using send_instruction
    {
        // clear the display, then set the function mode
        this->clear();       
        this->send_data(this->instr_fn_set, this->INSTRUCTION_REGISTER);
        ThisThread::sleep_for(this->CLEAR_DELAY_MS);            

        // then turn the display mode, and set an entry mode. user may want to overwrite these next two lines of code with their own. copy the above 5 lines, then set initialize_with_default = false in the constructor
        this->send_data(instr_display_on_no_cursor_blink, this->INSTRUCTION_REGISTER);
        this->send_data(instr_entry_mode, this->INSTRUCTION_REGISTER);    
    }
    this->cursor_x = 0;
    this->cursor_y = 0;
    this->set_cursor_position(this->cursor_x, this->cursor_y);
    this->screen_wrap = true;  // true by default, user can simply change public boolean value as needed.     
}

void HDD4470_HAL::clear(void) {
    this->send_data(this->instr_clear_disp, this->INSTRUCTION_REGISTER);   // set to true, because we want the function to select the instruction register
    ThisThread::sleep_for(this->CLEAR_DELAY_MS);        
}

inline void HDD4470_HAL::new_line(void)  {
    this->cursor_y++;
    this->cursor_x = 0;
    this->set_cursor_position(this->cursor_x, this->cursor_y);
}

size_t HDD4470_HAL::writeChar(uint8_t c)
{
    this->set_cursor_position(this->cursor_x, this->cursor_y);

    if (c == '\n') {
        this->new_line();
    }
    else if (c == '\r') {
        this->cursor_x = 0;
    }
    else {
        // drawChar(cursor_x, cursor_y, c, textcolor, textbgcolor, textsize); // replace with write_character and set_cursor_position() functions
        this->send_data(c, this->DATA_REGISTER);                  
        this->cursor_x++;

        if (this->screen_wrap && (this->cursor_x > (this->display_width))) {
            this->new_line();
        }
    }
    return 1; // overwriting writeChar function from Stream, it required a return value which we didn't really need. 
}

void HDD4470_HAL::set_cursor_position(uint8_t x, uint8_t y) { // update the display cursor position, and also in mcu memory. 
    this->cursor_x = x; 
    this->cursor_y = y;

    if (this->cursor_y == 0) {// first line                     
        this->send_data((FIRST_LINE + this->cursor_x), this->INSTRUCTION_REGISTER);                      
    }
    if (this->cursor_y >= this->display_height - 1)  {
        this->send_data((SECOND_LINE + this->cursor_x), this->INSTRUCTION_REGISTER);  
    }
}

void HDD4470_HAL::send_data(uint8_t instr, uint8_t register_sel) {
    if (register_sel == this->DATA_REGISTER)
    {
        this->select_data_register();              
    }
    else // for this->INSTRUCTION REGISTER, and also just incase someone writes something stupid. 
    {
        this->select_instruction_register();
    }            

    if (!using_bus) {
        *data_pins.data_port = instr; // set entire port to match instr, LSBF
    }
    else {
        *data_pins.data_bus = instr;
    }

    this->pulse_enable();
}

inline void HDD4470_HAL::select_instruction_register(void) {   
    if (this->register_selected != this->INSTRUCTION_REGISTER) 
    {
        this->register_selected = this->INSTRUCTION_REGISTER;              
        this->register_select_pin = this->register_selected;          
    }
}

inline void HDD4470_HAL::select_data_register(void) {       
    if (this->register_selected != this->DATA_REGISTER) 
    {   
        this->register_selected = this->DATA_REGISTER;      
        this->register_select_pin = this->register_selected;          
    }
}      

inline void HDD4470_HAL::pulse_enable(void) {
    // the LCD screen works on a falling edge, because we start low to avoid displaying junk data, we'll start by pulsing it high first.           
    this->enable_pin = 1;
    ThisThread::sleep_for(this->INSTRUCTION_DELAY_MS);     
                    
    this->enable_pin = 0;
    ThisThread::sleep_for(this->INSTRUCTION_DELAY_MS);                      
}      