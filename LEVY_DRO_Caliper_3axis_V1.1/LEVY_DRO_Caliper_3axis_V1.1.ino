// LEVY
// http://levysounddesign.blogspot.com/
//
// 3 AXIS CALIPER DRO FOR LATHE
// Hardware Version 1.0
// Software Version 1.1
// Created 10NOV20
// Updated 13JAN21

// DESCRIPTION:
// This program uses an Arduino Nano to read inexpensive Digital Calipers for use as a DRO.
// X Z Axis are displayed on an I2C OLED screen.
// A matrix button pad is used to input user data.

// BOARDS / MODULES:
// Arduino Nano (old bootloader)
// LEVY DRO PCB

// SOFTWARE VERSION CHANGELOG:
// 1.1 - 04JUL21 - tidied up comments
// 1.0 - 10NOV20 - Start of project

// TASKS ONGOING:
// nil

// NOTES:
// !!! all calipers must be connected, otherwise the device will hang at the boot screen
// (for testing purposes, comment out any calipers not connected)

// INCLUDE:
#include <Keypad.h>                                   //Keypad library for matrix keypad
#include <Adafruit_GFX.h>                             //Adafruit_GFX for oled screen
#include <Adafruit_SSD1306.h>                         //Adafruit_SSD1306 for oled screen

// CONSTANTS:
const int clockPinX = 10;                             //X Caliper Clock pin
const int dataPinX = 11;                              //X Caliper Data pin
const int clockPinY = 12;                             //Y Caliper Clock pin
const int dataPinY = 13;                              //Y Caliper Data pin
const int clockPinZ = 14;                             //Z Caliper Clock pin
const int dataPinZ = 15;                              //Z Caliper Data pin

const int SCREEN_WIDTH = 128;                         //OLED display width, in pixels
const int SCREEN_HEIGHT = 64;                         //OLED display height, in pixels
const int OLED_RESET = -1;                            //OLED Reset pin # (or -1 if sharing Arduino reset pin)
const int OLED_ADDR = 0x3C;                           //OLED address

const byte keyPadRows = 4;                            //Keypad matrix rows
const byte keyPadCols = 4;                            //Keypad matrix columns
char keys[keyPadRows][keyPadCols] = {                 //Keypad matrix character array
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte keyPadRowPins[keyPadRows] = {9, 8, 7, 6};        //Keypad matrix row pins
byte keyPadColPins[keyPadCols] = {5, 4, 3, 2};        //Keypad matrix column pins

// VARIABLES:
int positionAbsX = 0;                                 //holds X caliper absolute position
int positionAbsY = 0;                                 //holds Y caliper absolute position
int positionAbsZ = 0;                                 //holds Z caliper absolute position

int positionRelX = 0;                                 //holds X caliper relative position
int positionRelY = 0;                                 //holds Y caliper relative position
int positionRelZ = 0;                                 //holds Z caliper relative position

int positionDisplayX = 0;                             //holds X caliper screen display
int positionDisplayY = 0;                             //holds Y caliper screen display
int positionDisplayZ = 0;                             //holds Z caliper screen display

int caliperValue = 0;                                 //holds caliper value during serial communications
int caliperValueSign = 1;                             //holds sign of caliper value during serial communications

int userInput = 0;                                    //holds user data from keypad
bool userInputFlagX = false;                          //flags user input X
bool userInputFlagY = false;                          //flags user input Y
bool userInputFlagZ = false;                          //flags user input Z

unsigned long tempTime = 0;                           //holds time

// OBJECTS:
Keypad keypad = Keypad( makeKeymap(keys), keyPadRowPins, keyPadColPins, keyPadRows, keyPadCols );   //instantiate Keypad class from Keypad library as keypad - for matrix keypad
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);                           //instantiate Adafruit_SSD1306 class from Adafruit_SSD1306 as display - for OLED screen

// SETUP:
void setup() {
  //Serial.begin(115200);                             //DEBUG ONLY start serial communication

  pinMode(clockPinX, INPUT);                          //clockPinX Digital Pin INPUT
  pinMode(dataPinX, INPUT);                           //dataPinX  Digital Pin INPUT
  pinMode(clockPinY, INPUT);                          //clockPinY Digital Pin INPUT
  pinMode(dataPinY, INPUT);                           //dataPinY  Digital Pin INPUT
  pinMode(clockPinZ, INPUT);                          //clockPinZ Digital Pin INPUT
  pinMode(dataPinZ, INPUT);                           //dataPinZ  Digital Pin INPUT

  keypad.setDebounceTime(100);                        //keypad debounce time

  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);     //start OLED display
  display.clearDisplay();                             //clear display buffer
  display.setTextSize(4);                             //text size
  display.setTextColor(WHITE);                        //text colour
  display.setCursor(0, 0);                            //Start draw display at top-left corner of screen
  display.println("LEVY");                            //draw text
  display.println("DRO");                             //draw text
  display.display();                                  //send display buffer to screen
  delay(1000);                                        //wait for 1s
}

// PROGRAM MAIN LOOP:
void loop() {

  // READ X CALIPER
  while (digitalRead(clockPinX) == HIGH) {}           //if clock is LOW wait until it turns to HIGH
  tempTime = micros();
  while (digitalRead(clockPinX) == LOW) {}            //wait for the end of the HIGH pulse
  if ((micros() - tempTime) > 500) {                  //if HIGH pulse >500 micros, start of a new bit sequence
    caliperValueSign = 1;                             //assume positive value
    caliperValue = 0;                                 //set value = 0
    for (int i = 0; i < 23; i++) {                    //for 23 cycles
      while (digitalRead(clockPinX) == HIGH) {}       //wait until clock HIGH. First bit not needed
      while (digitalRead(clockPinX) == LOW) {}        //wait until clock LOW
      if (digitalRead(dataPinX) == LOW) {             //if dataPin LOW
        if (i < 20) {                                   //if i<20
          caliperValue |= 1 << i;                         //left shift a 1 to bit i of caliperValue
        }
        if (i == 20) {                                //if i==20
          caliperValueSign = -1;                        //value is negative. set caliperValueSign = -1
        }
      }
    }
    positionAbsX = caliperValue * caliperValueSign;   //calculate absolute position
    //Serial.println(positionAbsX);                   //DEBUG ONLY print result
  }


  // READ Y CALIPER
  while (digitalRead(clockPinY) == HIGH) {}           //if clock is LOW wait until it turns to HIGH
  tempTime = micros();
  while (digitalRead(clockPinY) == LOW) {}            //wait for the end of the HIGH pulse
  if ((micros() - tempTime) > 500) {                  //if HIGH pulse >500 micros, start of a new bit sequence
    caliperValueSign = 1;                             //assume positive value
    caliperValue = 0;                                 //set value = 0
    for (int i = 0; i < 23; i++) {                    //for 23 cycles
      while (digitalRead(clockPinY) == HIGH) {}       //wait until clock HIGH. First bit not needed
      while (digitalRead(clockPinY) == LOW) {}        //wait until clock LOW
      if (digitalRead(dataPinY) == LOW) {             //if dataPin LOW
        if (i < 20) {                                   //if i<20
          caliperValue |= 1 << i;                         //left shift a 1 to bit i of caliperValue
        }
        if (i == 20) {                                //if i==20
          caliperValueSign = -1;                        //value is negative. set caliperValueSign = -1
        }
      }
    }
    positionAbsY = caliperValue * caliperValueSign;   //calculate absolute position
    //Serial.println(positionAbsY);                   //DEBUG ONLY print result
  }

  // READ Z CALIPER
  while (digitalRead(clockPinZ) == HIGH) {}           //if clock is LOW wait until it turns to HIGH
  tempTime = micros();
  while (digitalRead(clockPinZ) == LOW) {}            //wait for the end of the HIGH pulse
  if ((micros() - tempTime) > 500) {                  //if HIGH pulse >500 micros, start of a new bit sequence
    caliperValueSign = 1;                             //assume positive value
    caliperValue = 0;                                 //set value = 0
    for (int i = 0; i < 23; i++) {                    //for 23 cZcles
      while (digitalRead(clockPinZ) == HIGH) {}       //wait until clock HIGH. First bit not needed
      while (digitalRead(clockPinZ) == LOW) {}        //wait until clock LOW
      if (digitalRead(dataPinZ) == LOW) {             //if dataPin LOW
        if (i < 20) {                                   //if i<20
          caliperValue |= 1 << i;                         //left shift a 1 to bit i of caliperValue
        }
        if (i == 20) {                                //if i==20
          caliperValueSign = -1;                        //value is negative. set caliperValueSign = -1
        }
      }
    }
    positionAbsZ = caliperValue * caliperValueSign;   //calculate absolute position
    //Serial.println(positionAbsZ);                   //DEBUG ONLZ print result
  }


  // READ KEYPAD
  int key = keypad.getKey();                          //read keypad
  if (key != NO_KEY) {                                //if any button pressed:

    if (userInputFlagX == false && userInputFlagY == false && userInputFlagZ == false) {    //if no flags true:
      if (key == 'A') userInputFlagX = true;            //if "A" (X) button pressed, set user input flag X
      if (key == 'B') userInputFlagY = true;            //if "B" (Y) button pressed, set user input flag Y
      if (key == 'C') userInputFlagZ = true;            //if "C" (Z) button pressed, set user input flag Z
    }

    if (userInputFlagX == true || userInputFlagY == true || userInputFlagZ == true) {       //if any flags true:
      if (key == '1') userInput = userInput * 10 + 1;     //if number button pressed, left shift, add number
      if (key == '2') userInput = userInput * 10 + 2;     //if number button pressed, left shift, add number
      if (key == '3') userInput = userInput * 10 + 3;     //if number button pressed, left shift, add number
      if (key == '4') userInput = userInput * 10 + 4;     //if number button pressed, left shift, add number
      if (key == '5') userInput = userInput * 10 + 5;     //if number button pressed, left shift, add number
      if (key == '6') userInput = userInput * 10 + 6;     //if number button pressed, left shift, add number
      if (key == '7') userInput = userInput * 10 + 7;     //if number button pressed, left shift, add number
      if (key == '8') userInput = userInput * 10 + 8;     //if number button pressed, left shift, add number
      if (key == '9') userInput = userInput * 10 + 9;     //if number button pressed, left shift, add number
      if (key == '0') userInput = userInput * 10 + 0;     //if number button pressed, left shift, add number
      if (key == '#') userInput = userInput * -1;         //multiply userInput by -1
    }

    if (key == 'D') {                                   //if "D" (enter) button pressed:
      if (userInputFlagX == true) {                       //if userInputFlagX is true:
        positionRelX = userInput - positionAbsX;            //calculate positionRelX
        userInput = 0;                                      //reset userInput
        userInputFlagX = false;                             //reset userInputFlagX
      }

      if (userInputFlagY == true) {                      //if userInputFlagY is true:
        positionRelY = userInput - positionAbsY;           //calculate positionRelY
        userInput = 0;                                     //reset userInput
        userInputFlagY = false;                            //reset userInputFlagY
      }

      if (userInputFlagZ == true) {                      //if userInputFlagZ is true:
        positionRelZ = userInput - positionAbsZ;           //calculate positionRelZ
        userInput = 0;                                     //reset userInput
        userInputFlagZ = false;                            //reset userInputFlagZ
      }

    }

    if (key == '*') {                                   //if "*" (clear) button pressed:
      userInput = 0;                                      //clear userInput
      userInputFlagX = false;                             //clear userInputFlag
      userInputFlagY = false;                             //clear userInputFlag
      userInputFlagZ = false;                             //clear userInputFlag
    }

    //Serial.println(userInput);                      //DEBUG ONLY print
  }

  // CALCULATE POSITION
  positionDisplayX = positionAbsX + positionRelX;     //calculate positionDisplay
  positionDisplayY = positionAbsY + positionRelY;     //calculate positionDisplay
  positionDisplayZ = positionAbsZ + positionRelZ;     //calculate positionDisplay

  // DISPLAY
  display.clearDisplay();                             //clear display buffer
  display.setTextSize(2);                             //text size
  display.setTextColor(WHITE);                        //text colour
  display.setCursor(0, 0);                            //Start draw display at top-left corner of screen

  if (userInputFlagX == true) {                      //if userInputFlag is true (axis selection)
    display.print("*X: ");                              //draw text
    display.println(userInput / 100.00);                //draw userInput in mm
  } else {                                            //if userInputFlag is false
    display.print("X: ");                               //draw text
    display.println(positionDisplayX / 100.00);         //draw position in mm
  }

  if (userInputFlagY == true) {                      //if userInputFlag is true (axis selection)
    display.print("*Y: ");                              //draw text
    display.println(userInput / 100.00);                //draw userInput in mm
  } else {                                            //if userInputFlag is false
    display.print("Y: ");                               //draw text
    display.println(positionDisplayY / 100.00);         //draw position in mm
  }

  if (userInputFlagZ == true) {                      //if userInputFlag is true (axis selection)
    display.print("*Z: ");                              //draw text
    display.println(userInput / 100.00);                //draw userInput in mm
  } else {                                            //if userInputFlag is false
    display.print("Z: ");                               //draw text
    display.println(positionDisplayZ / 100.00);         //draw position in mm
  }

  display.display();                                  //send display buffer to screen

}


// REFERENCE:
// Calipers tutorial - https://sites.google.com/site/marthalprojects/home/arduino/arduino-reads-digital-caliper
// Keypad tutorial - https://playground.arduino.cc/Code/Keypad/
// OLED Screen tutorial - https://randomnerdtutorials.com/guide-for-oled-display-with-arduino/
