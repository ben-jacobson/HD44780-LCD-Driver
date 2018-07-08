class LCD_Driver  
{
/* 
 *  Specifically designed for use with the Adafruit LCD 16x2 screen which uses the HD44780 driver chip. 
 *  Uses a shift register to output data to the data lines
 *  Certain delay times are needed for error free operation, and there are a lot of basic functions to help get the device up and running
 *  
 *  TODO:
 *    - change the send_data and send_instruction to check the value of the register, change if needed
 *    
 */
public:
    const uint8_t display_width = 16; // this is a 16x2 screen
    const uint8_t display_height = 2; 

      LCD_Driver(uint8_t register_select_pin, uint8_t read_write_pin, uint8_t enable_pin, uint8_t ds_pin, uint8_t sh_cp, uint8_t st_cp, bool initialize_with_default = true) // with shift register, DS = data pin, SH_CP = shift clock, ST_CP = storage register clock
      {
          this->ds_pin = ds_pin;
          this->sh_cp = sh_cp;
          this->st_cp = st_cp;
          pinMode(this->ds_pin, OUTPUT);
          pinMode(this->sh_cp, OUTPUT);
          pinMode(this->st_cp, OUTPUT);

          // initialize shift register
          this->register_select_pin = register_select_pin;
          this->read_write_pin = read_write_pin;  
          this->enable_pin = enable_pin; 
          pinMode(this->register_select_pin, OUTPUT);
          pinMode(this->read_write_pin, OUTPUT);
          pinMode(this->enable_pin, OUTPUT);

          // if the MCU is reset, this doesn't clear the shift register. shift out all zero's to reset
          this->shiftout_data_lines(B00000000);

          // initialise LCD screen values
          this->register_selected = this->INSTRUCTION_REGISTER;                     
          digitalWrite(this->enable_pin, LOW); // init as low so that it doesn't init anything left over from the buffer. 
          digitalWrite(this->register_select_pin, register_selected); // start with instruction register
          digitalWrite(this->read_write_pin, LOW); // we set this low = write mode, since we rarely write to the screen. Alternatively, just ground this pin. 

            if (initialize_with_default == true)  // pass a false through the class constructor to override this, then you can use your own initialize methods using send_instruction
            {
                this->initialize_display_default();
            }
      }
      void clear(void)
      {
            this->send_data(this->instr_clear_disp, this->INSTRUCTION_REGISTER);   // set to true, because we want the function to select the instruction register  
            delayMicroseconds(this->clear_delay_us); 
      }

      void print_screen(String message)  // print will take into account the number of characters on the display, and attempt to wrap lines
      {
      //char c;
      short int message_length = message.length();
      short int max_message_length = this->display_width * this->display_height;

          if (message_length >= max_message_length) // ie larger that can fit on screen
          {
              message = message.substring(0, max_message_length); // truncate text to max len
          }

        this->clear(); // clear the display
                
            for (short int i = 0; i < message_length; i++) // will cut off message if needs to
            {                                                   
                this->send_data(byte(message[i]), this->DATA_REGISTER);

                    if (i == this->display_width - 1)
                    {
                        this->set_cursor_position(0, this->display_height - 1);   
                    }                    
            }
      }

      void write_line(String message, int8_t x, int8_t y) // will not clear screen, will write the line at the position you want
      {
      short int message_length = message.length();

          this->set_cursor_position(x, y);
              
              for (short int i = 0; i < message_length; i++) 
              {                                                   
                  this->send_data(byte(message[i]), this->DATA_REGISTER);                  
              }                  
      }
      
      void write_character(char c, bool manual_set_reg = false) // converts the individual char to uint8_t or byte representing the appropriate ASCII symbol and sends data to screen. Note this only supports the first 128 characters of ASCII, for the rest, please see byte codes in the datasheet
      {
            if (uint8_t(c) <= 128) // this will only for the standard symbols. For all other symbols, will need to manually write them using the byte codes of the LCD screen.
            {
                // note that a standard ASCII forward slash doesn't exist on the LCD screen, instead it is a ¥ yen symbol. 
                // ~ symbol is a -> symbol, and whatever is ASCII 127 is a <-
                this->send_data(byte(c), this->DATA_REGISTER);      
            }
      }  
      void set_cursor_position(uint8_t x, uint8_t y)
      {
      byte first_line = B10000000; 
      byte second_line = B11000000;

              if (x >= 0)
              {
                  if (x >= this->display_width) // if you go out of x bounds, bring it back to width
                  {
                      x = this->display_width - 1;
                  }
                  if (y == 0) // first line
                  {                    
                      this->send_data((first_line + byte(x)), this->INSTRUCTION_REGISTER);                      
                  }
                  if (y >= this->display_height - 1) // second line and above become second line. if you go out of y bounds, bring it back to width
                  {
                      this->send_data((second_line + byte(x)), this->INSTRUCTION_REGISTER);  
                  }
              }
              // else will just ignore the values

            //delayMicroseconds(this->clear_delay_us);
      }
      
private:   
    const short int MCU_latency_us = 4; // it takes 4us for digitalWrite to complete it's function, when you need exact timing, use this to offset the timing you want. 
  
    const short int clear_delay_us = 1550; // clear display and return home instructions take 1.53ms to execute
    const short int instruction_delay_us = 45; // most instructions take 45us to execute, we'll force the MCU to wait

    // Private constants of commonly used instructions for initializing the delay. 
    const byte instr_clear_disp         = B00000001; // clears the display entirely. 
    const byte instr_return_home        = B00000010; // returns the cursor to the home position
    const byte instr_fn_set             = B00111000; // Function set, 8 bit mode, 2 lines, 5×8 font.
    const byte instr_move_cursor_left   = B00010000; // move cursor left
    const byte instr_move_cursor_right  = B00010100; // move cursor right
  
    uint8_t register_select_pin;  // pins used for the LCD screen. We don't need 8 bits for these pin numbers, this is the smallest we can get. 
    uint8_t read_write_pin; 
    uint8_t enable_pin; 
    
    uint8_t ds_pin;   // pins used for the shift register. to save pins, we use a shift register to pulse out the 8 bits of screen data
    uint8_t sh_cp; 
    uint8_t st_cp;

    const uint8_t INSTRUCTION_REGISTER = LOW;
    const uint8_t DATA_REGISTER = HIGH;  
    uint8_t register_selected; 

      void initialize_display_default(void)  // just some default values, the user can choose to do this themselves if they want
      {
      byte disp_on    = B00001111; // Display ON, Cursor On, Cursor Blinking.
      byte entry_mode = B00000110; // Entry Mode, Increment cursor position, No display shift. change to B00000100 to disable automatic cursor increment

            // clear the display, then set the function mode
            this->send_data(this->instr_clear_disp, this->INSTRUCTION_REGISTER);
            delayMicroseconds(this->clear_delay_us);                               
            this->send_data(this->instr_fn_set, this->INSTRUCTION_REGISTER);
            delayMicroseconds(this->clear_delay_us);            

            // then turn the display mode, and set an entry mode. user may want to overwrite these next two lines of code with their own. copy the above 5 lines, then set initialize_with_default = false in the constructor
            this->send_data(disp_on, this->INSTRUCTION_REGISTER);
            this->send_data(entry_mode, this->INSTRUCTION_REGISTER);           
      }

      void send_data(byte instr, uint8_t register_sel)
      {
            if (register_sel == this->DATA_REGISTER)
            {
                this->select_data_register();              
            }
            else // for this->INSTRUCTION REGISTER, and also just incase someone writes something stupid. 
            {
                this->select_instruction_register();
            }

          // debug mode
          //Serial.read();
          //Serial.println("Instruction - reg: " + String(register_sel) + ", data: " + String(instr));
            
          this->shiftout_data_lines(instr);     
          this->pulse_enable();
      }
      
      void select_instruction_register(void)
      {   
          if (this->register_selected != this->INSTRUCTION_REGISTER) 
          {
              this->register_selected = this->INSTRUCTION_REGISTER;
              digitalWrite(this->register_select_pin, this->register_selected);
          }
      }
      
      void select_data_register(void)
      {       
          if (this->register_selected != this->DATA_REGISTER) 
          {   
              this->register_selected = this->DATA_REGISTER;      
              digitalWrite(this->register_select_pin, this->register_selected);      
          }
      }      
      
      void pulse_enable(void) 
      {
          // the LCD screen works on a falling edge, because we start low to avoid displaying junk data, we'll start by pulsing it high first. 
          digitalWrite(this->enable_pin, HIGH);
          delayMicroseconds(this->instruction_delay_us);     
                  
          digitalWrite(this->enable_pin, LOW);
          delayMicroseconds(this->instruction_delay_us);                      
      }
      
      void shiftout_data_lines(byte byte_data) // offset to make a 10us delay
      {
          digitalWrite(this->st_cp, LOW);  // set latch low
          shiftOut(this->ds_pin, this->sh_cp, MSBFIRST, byte_data); 
          digitalWrite(this->st_cp, HIGH); // set latch high           
          //digitalWrite(this->ds_pin, LOW);  // useful for cleaning up signals, but has no impact on circuit, best to avoid the time-cost of this routine
          delayMicroseconds((10 - this->MCU_latency_us));  //delay 10us
      } 
      
};

struct screen_element // fixed screen elements, we update the values of these to make it easy to draw everything
{
    bool selectable; // can you put the cursor over it? 
    uint8_t x; // value range from 0-15, only requires 2 bits of data to store
    uint8_t y; // value range from 0-1, only requires 1 bit of data to store. Why aren't there 1, 2 and 4 bit data types for stuff like this? I read that bool's are 1 byte length. Wut? 
    String contents; // store the content of the data, to any length that you like 
};
 
class screen_interface // I originally wanted to use inheritance for this, but it proved to be too tedious setting up, 
{
  
public:   

  screen_interface(uint8_t register_select_pin, uint8_t read_write_pin, uint8_t enable_pin, uint8_t ds_pin, uint8_t sh_cp, uint8_t st_cp, bool initialize_with_default = true)
  {
  char left_arrow = 127; // technically ascii value ~, but on this screen is a left arrow.            
  char right_arrow = 126;          
  short int stored_hex_values = 8; // we want to track and store 8 hex elements
  short int spacer = 0;
      
      interface_lcd = new LCD_Driver(register_select_pin, read_write_pin, enable_pin, ds_pin, sh_cp, st_cp);

      // set up the screen elements
      screen_elements = new screen_element[this->max_screen_elements];  
      
      this->create_screen_element(0, 0, "IR:", false); 
      this->create_screen_element(4, 0, "DEFAULT", true);  

      this->left_arrow_index = this->create_screen_element(15, 0, String(left_arrow), true); // right arrow. 

        for (uint8_t i = 0; i < stored_hex_values; i++)
        {
            //Serial.println(String(i + spacer));
            this->create_screen_element(i + spacer, 1, "F", true); // create the screen elements that store the hex values, default value F  
            
                if (i % 2 == 1) // every 3rd position is a gap. To be honest, I have no idea if this works for really long patterns.. Doesn't matter. 
                {
                    spacer++;               
                }            
        }   

      this->right_arrow_index = this->create_screen_element(15, 1, String(right_arrow), true); // right arrow.  
   
      // set up the hex value screen elements
      this->current_element_selected = 1; // this is the first hex value selected.       
  }

  void draw_screen(void)
  {        
      // draw the screen by looping through the screen_elements 
      
          for (uint8_t i = 0; i < this->max_screen_elements; i++)    
          {
              interface_lcd->write_line(this->screen_elements[i].contents, this->screen_elements[i].x, this->screen_elements[i].y);
          }
      // set cursor position
      interface_lcd->set_cursor_position(screen_elements[this->current_element_selected].x, screen_elements[this->current_element_selected].y);             
  }

  void click_up(void) 
  {
      
  }

  void click_down(void)
  {

  }

  void click_left(void) 
  {
        for (short int i = 0; i < this->total_element_count + 1; i++) // originally we had a while(true) condition, but if someone comes along and uses this with no selectable elements, it would result in an infinite loop.  
        {                              
                if (this->current_element_selected - 1 < 0) // introduce a wrap around
                {
                    this->current_element_selected = this->total_element_count; // the very last element
                }

            this->current_element_selected--;     // go left    
                
                // skip unreadable elements
                if (this->screen_elements[this->current_element_selected].selectable == true) // for the sake of readable code. we could have just made this part ofthe loop, but it got super unreadable fast. 
                {
                    break;
                }                                                            
        }                 
  }

  void click_right(void)
  {
        for (short int i = 0; i < this->total_element_count + 1; i++) // originally we had a while(true) condition, but if someone comes along and uses this with no selectable elements, it would result in an infinite loop.  
        {   
            this->current_element_selected++;     // go right   
            
                if (this->current_element_selected >= this->total_element_count) // introduce a wrap around
                {
                    this->current_element_selected = 0; // the very first element
                }
                            
                // skip unreadable elements
                if (this->screen_elements[this->current_element_selected].selectable == true) // for the sake of readable code. we could have just made this part ofthe loop, but it got super unreadable fast. 
                {
                    break;
                }                              
        }            
  }
  
private:
  LCD_Driver *interface_lcd; // why we don't have the ability to do this without a pointer, is beyond me. Surely there is some deeply hidden downside to doing it this way?
  
  const short int max_screen_elements = 16;
  screen_element *screen_elements;
  
  short int total_element_count = 0;  
  short int current_element_selected = 0;

  short int left_arrow_index = 0;
  short int right_arrow_index = 0; 
  
      short int create_screen_element(uint8_t x, uint8_t y, String contents, bool selectable)
      {   
              if (this->total_element_count < this->max_screen_elements)
              {
                  // fill existing one 
                  this->screen_elements[this->total_element_count].x = x;
                  this->screen_elements[this->total_element_count].y = y;
                  this->screen_elements[this->total_element_count].contents = contents;
                  this->screen_elements[this->total_element_count].selectable = selectable;
                  this->total_element_count++;             // then move counter forward
              }
          return this->total_element_count - 1;  // returns the element number that it just filled up. 
      }            
};

uint8_t register_select_pin = 8;  // pins used for the LCD screen
uint8_t read_write_pin = 9; // on the circuit, either pull low or connect to a pin, which will set low.
uint8_t enable_pin = 10; 

uint8_t ds_pin = 2;   // pins used for the shift register. to save pins, we use a shift register to pulse out the 8 bits of screen data
uint8_t sh_cp = 3; 
uint8_t st_cp = 4;  

uint8_t up_button = 14; //pin 13 = A0 but used as a digital
uint8_t down_button = 15; 
uint8_t left_button = 16; 
uint8_t right_button = 17; 

screen_interface *controller_screen;
short int update_delay = 180; // without a delay, the screen moves too fast for the user to keep up. This is comfortable. 
bool has_screen_updated = false; 

void setup()
{  
    Serial.begin(9600);
    while(!Serial); // wait  until serial is ready

    pinMode(up_button, INPUT_PULLUP); // no need for pullup, we have our own
    pinMode(down_button, INPUT_PULLUP); 
    pinMode(left_button, INPUT_PULLUP);
    pinMode(right_button, INPUT_PULLUP); 
      
    controller_screen = new screen_interface(register_select_pin, read_write_pin, enable_pin, ds_pin, sh_cp, st_cp);
    has_screen_updated = true; // this forces the first run of the loop to draw it initially. 

    //controller_screen->build_instruction
}

void loop()
{
        if (has_screen_updated)
        {
            controller_screen->draw_screen();   
            has_screen_updated = false;             
            delay(update_delay); 
        }
  
        if (digitalRead(up_button) == LOW)
        {
            controller_screen->click_up();
            has_screen_updated = true;  
        }
        
        if (digitalRead(down_button) == LOW)
        {
            controller_screen->click_down();
            has_screen_updated = true;       
        }
        
        if (digitalRead(left_button) == LOW)
        {
            //Serial.println("Clicked Left");
            controller_screen->click_left();
            has_screen_updated = true;  
        }
        
        if (digitalRead(right_button) == LOW)
        {
            //Serial.println("Clicked Right");          
            controller_screen->click_right();
            has_screen_updated = true;         
        } 
             
}


  
/*LCD_Driver test_lcd(register_select_pin, read_write_pin, enable_pin, ds_pin, sh_cp, st_cp); 
String input; 

void setup() 
{

    Serial.println("Enter some text: ");   
    test_lcd.print_screen("testing one two three four five six");
}

void loop() 
{
    if (Serial.available())
    {
        input = Serial.readString();
        Serial.println("printing on LCD: " + input);
        test_lcd.print_screen(input);
        test_lcd.set_cursor_position(0, 1);
    }
}*/

