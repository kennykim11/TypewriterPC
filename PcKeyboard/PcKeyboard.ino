/*
  PcKeyboard
  Kenny Kim
  2018-2019
  
  This is a Teensyduino program to help the keyboard of an IBM Wheelwriter 1000 function as a USB keyboard.
  The Wheelwriter keyboard has 2 ribbon cables coming out of it - one with 14 pins and one with 8 pins. They act as a row/column system and here is the mapping for the keys.
  +--------------------------------------------------------------------------------------------------------------------------------------------------+
  |   0         1        2         3         4         5         6         7         8         9         10         11         12         13         |
  | 0 L_Control                                                  G         H                             '          Enter      Down Arr              |
  | 1           Alt                A         S         D         F         J         K         L         ;                     Reloc                 |
  | 2           Mar Rel            Escape                        5         6         +                   -          Backspace  Page Up               |
  | 3                              1         2         3         4         7         8         9         0                                           |
  | 4 LMar      Tab      Q         W         E         R         U         I         O         P                               Page Down             |
  | 5           RMar                                             T         Y         ]         [                                                     |
  | 6           CapsLock R_Shift   Z         X         C         V         M         ,         .                    Up Arr                           |
  | 7 Space     Delete   L_Shift                                 B         N                             /          Left Arr   Right Arr  GUI        |
  +--------------------------------------------------------------------------------------------------------------------------------------------------+

  The main loop is a double for loop, where each output "row" pin is by default set to high voltage (5V) and one at a time is set to low voltage (0v). Then each input "column" pin
  (connected to a pullup resistor) checks to see if the voltage is low. If so, that means that the specific key is being pressed. Each input pin is actually represented by a Bounce 
  object, which is a downloadable Arduino library that filters out key bounces.

  There are seven keyboard layouts defined in KeyboardConfig.h:
  . Computer - for normal comuter usage
    . Regular - model an ANSI-101 keyboard as much as possible
    . Function - replaces some keys for F keys and mouse controls
    . Media - replaces some keys for number pad and multimedia keys
    . Macro - just send serial codes for each key
  . Literal - models the original keyboard as much as possible
  . Music - emulates a keyboard and outputs tones on up to 10 active buzzers

  Each layout is a 2D array of unsigned ints because the Teensyduino keys are actually just namespaces for unisgned ints. Special functions are also stored
  as unsigned ints with this format where the first digit designates the type of special function:
  . Mouse ccontrol = 1????
  . Send certain Key Combo now = 2????
  . Send Alt code = 3????
  . Send Serial = 4????
  The special functions are identified in the loop and handled by helper functions.
*/



// === IMPORTS ===
#include "KeyboardConfig.h" //contains a single 3D array of unsigned ints that map configuration+row+column to key
#include <Bounce2.h> //used to make sure a key bounc is not counted as two presses



// === CONSTANTS ===
//pins
const int columnPins[14] = {PIN_D1, PIN_D2, PIN_D3, PIN_D4, PIN_D5, PIN_D6, PIN_D7, PIN_E0, PIN_C1, PIN_C2, PIN_C3, PIN_C4, PIN_C5, PIN_C6};
Bounce COLUMNS[14] = {}; //Set in setup
const int ROWS[8] = {PIN_F0, PIN_F1, PIN_F2, PIN_F3, PIN_F4, PIN_F5, PIN_F6, PIN_F7};


const String ALTCODES[8] = {"0176", "171", "0177", "172", "0253", "0179", "21", "20"}; //alt codes
void (usb_keyboard_class::*registerFuncs[6])(unsigned char) = {&usb_keyboard_class::set_key1, &usb_keyboard_class::set_key2, &usb_keyboard_class::set_key3, &usb_keyboard_class::set_key4, &usb_keyboard_class::set_key5, &usb_keyboard_class::set_key6}; //"set_key" array
int checkPin = PIN_B6; //The check pin - bridge to GND for keyboard to function



// === HELPER FUNCTIONS ===
// =Getters=
//getOneKey checks to see if a specific key is pressed
bool getOneKey(int row, int column){
  digitalWrite(ROWS[row], LOW);
  bool concl = !(COLUMNS[column].read());
  digitalWrite(ROWS[row], HIGH);
  return concl;
}

//getSwitch returns the status of the external selection switch
int getSwitch(){
  if (false) return 0; //Literal
  if (false) return 2; //Music
  return 1; //Pc
}

//getMode returns the specific keyboard to use, specifically which index of the configuration index
int getMode(){
  //Array = {LITERAL, LITERALSHIFT, LITERALCODE, STANDARD, FUNCTION, MEDIA, MUSIC, MACRO}
  int switchState = getSwitch();
  if (switchState == 0){ //Literal
    if (getOneKey(6, 2) || getOneKey(7, 2)) return 1; //Shift
    if (getOneKey(7, 13)) return 2; //Code
    return 0; //Normal
  }
  if (switchState == 2){ //Music
    return 6; //Music
  }
  else { //Pc
    if (getOneKey(2, 1)) return 4; //Function
    if (getOneKey(4, 0)) return 5; //Media
    if (getOneKey(5, 1)) return 7; //Macro
    return 3; //Standard
  }
}

// =Senders=
//sendAltCode sends a specific Alt code for Windows OS
void sendAltCode(String code){
  Keyboard.press(MODIFIERKEY_ALT);
  Keyboard.print(code);
  Keyboard.release(MODIFIERKEY_ALT);
}

//sendKeys sends a specific key or key combination
void sendKeys(unsigned int code){
  if      (code == 20001) Keyboard.print("[");
  else if (code == 20002) Keyboard.print(",");
  else if (code == 20003) Keyboard.print(".");
  else if (code == 20004) {
    Keyboard.press(MODIFIERKEY_CTRL);
    Keyboard.press(KEY_U);
    Keyboard.release(KEY_U);
    Keyboard.release(MODIFIERKEY_CTRL);
  }
  else if (code == 20005) {
    Keyboard.press(MODIFIERKEY_ALT);
    Keyboard.press(MODIFIERKEY_SHIFT);
    Keyboard.release(MODIFIERKEY_SHIFT);
    Keyboard.release(MODIFIERKEY_ALT);
  }
  else if (code == 20006) {
    Keyboard.press(MODIFIERKEY_CTRL);
    Keyboard.press(KEY_B);
    Keyboard.release(KEY_B);
    Keyboard.release(MODIFIERKEY_CTRL);
  }
  else if (code == 20009) {
    Keyboard.press(MODIFIERKEY_CTRL);
    Keyboard.press(KEY_LEFT);
    Keyboard.release(KEY_LEFT);
    Keyboard.release(MODIFIERKEY_CTRL);
  }
  else if (code == 20010) {
    Keyboard.press(MODIFIERKEY_CTRL);
    Keyboard.press(KEY_RIGHT);
    Keyboard.release(KEY_RIGHT);
    Keyboard.release(MODIFIERKEY_CTRL);
  }
}

// =Others=
//mouseControl changes mouseClick and mouseMovement state based on specific keycode
void mouseControl(unsigned int code, bool *mouseClick, int *mouseMovement){
  if      (code == 10001) Mouse.scroll(1);
  else if (code == 10002) Mouse.scroll(-1);
  else if (code == 10003) mouseClick[0] = true;
  else if (code == 10004) mouseClick[1] = true;
  else if (code == 10005) mouseClick[2] = true;
  else if (code == 10006) Mouse.scroll(5);
  else if (code == 10007) Mouse.scroll(-5);
  else if (code == 10008) mouseClick[1] -= 2;
  else if (code == 10009) mouseClick[1] += 2;
  else if (code == 10010) mouseClick[0] -= 2;
  else if (code == 10011) mouseClick[0] += 2;
  return;
}

//registerKey registers each key by calling calling set_key unless the buffer of six keys is full
int registerKey(int index, unsigned int key){
  if (index > 5) return 6;
  (Keyboard.*registerFuncs[index])(key);
  return index+1;
}

//clearKeys clears the buffer to be ready for nte next packet
void clearKeys(){
  for (int i = 0; i < 6; i++) (Keyboard.*registerFuncs[i])(0);
}



// === MAIN FUNCTIONS ===
// =Setup=
void setup() {
  Serial.begin(256000); //setup Serial
  Serial.println("Keyboard: Setting up");
  for (int row = 0; row < 8; row++){
    pinMode(ROWS[row], OUTPUT);
    digitalWrite(ROWS[row], HIGH); //rows will be by default high, set specific row to low when checking it
  }
  for (int column = 0; column < 14; column++){
    COLUMNS[column].attach(columnPins[column], INPUT_PULLUP); //attatch pin and pullup to Bounce object
    COLUMNS[column].interval(10); //set inveral to this many milliseconds in Bounce object
  }
  pinMode(checkPin, INPUT_PULLUP); //set up the check pin
}

// =Loop=
void loop() {
  if (!digitalRead(checkPin)){ //Only run if check pin is bridged with GND

    // =Set to defaults=
    bool mouseClick[3] = {0, 0, 0};
    int mouseMovement[2] = {0, 0};
    unsigned int modifiers = 0;
    int keyIndex = 0;
    clearKeys();

    int mode = getMode(); //get the keyboard config to use
    
    for (int row = 0; row < 8; row++){ //for each row
      digitalWrite(ROWS[row], LOW); //set the row to low voltage and check pressed keys
      
      for (int column = 0; column < 14; column++){ //for each column
        if (!COLUMNS[column].read()){ //if key is pressed
          
          unsigned int tempKey = LAYOUT[mode][row][column]; //temp key is the key being pressed
          Serial.print("Keyboard: Row=");
          Serial.print(row);
          Serial.print(", Column=");
          Serial.println(column);

          //if the config is literal shift and key is 0, do default instead
          if (mode == 1 && tempKey == 0){
            tempKey = LAYOUT[0][row][column];
            continue;
          }

          //if the key is a modifier, add it to the modifiers
          if (tempKey == MODIFIERKEY_CTRL || tempKey == MODIFIERKEY_SHIFT || tempKey == MODIFIERKEY_RIGHT_SHIFT || tempKey == MODIFIERKEY_ALT || tempKey == MODIFIERKEY_GUI){
            modifiers |= tempKey;
            continue;
          }


          // =Special Handling=
          if (tempKey < 10000) continue; //keyboard config keys are 00001, 00002, 00003
          if (tempKey >= 10000 && tempKey < 20000){ //mouse controls
            mouseControl(tempKey, mouseClick, mouseMovement);
            continue;
          }
          if (tempKey >= 20000 && tempKey < 30000){ //specific key
            sendKeys(tempKey);
            continue;
          }
          if (tempKey >= 30000 && tempKey < 40000){ //alt code
            sendAltCode(ALTCODES[tempKey - 30001]);
            continue;
          }
          if (tempKey >= 40000 && tempKey < 50000){ //serial code
            Serial.println(tempKey);
            continue;
          }
          
          keyIndex = registerKey(keyIndex, tempKey); //assuming the key is a normal key, register it to the packet

          //if the buffer is full skip the rest and send the packet
          if (keyIndex >= 6) {
            digitalWrite(ROWS[row], HIGH);
            goto sendPacket;
          }
        }
      }
      digitalWrite(ROWS[row], HIGH); //set the row to high and go to the next one
    }

    sendPacket: //send all of the keyboard and mouse packets
    Keyboard.set_modifier(modifiers);
    Keyboard.send_now();
    Mouse.set_buttons(mouseClick[0], mouseClick[1], mouseClick[2]);
    Mouse.move(mouseMovement[0], mouseMovement[1]);
  }
}
