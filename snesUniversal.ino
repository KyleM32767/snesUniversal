/*
 * snesUniversal.ino
 * 
 * Arduino sketch for a universal adapter to use an SNES controller to play PC games. Uses the Arduino Joystick Library
 * to make it a generic USB gamepad, but if it just doesn't work (which does happen sometimes), it can instead send
 * game-specific keyboard inputs, which are read from a text file on an SD card. A 16x2 character LCD display shows the
 * current configuration.
 * 
 * HARDWARE AND PINOUTS: todo
 * 
 * CUSTOM LIBRARIES USED:
 *  - Arduino SNES Controller Library 
 *    - https://github.com/KyleM32767/Arduino-SNES-Controller-Library
 *  - Arduino Joystick Library 
 *    - https://github.com/MHeironimus/ArduinoJoystickLibrary
 *  - LiquidCrystal_I2C
 *    - https://github.com/johnrickman/LiquidCrystal_I2C
 * 
 * Author: Kyle M
 * Created 27 Apr 2021
 */

#include <SPI.h>
#include <SD.h>
#include <Keyboard.h>
#include "LiquidCrystal_I2C.h"
#include "SNES.h"
#include "Joystick.h"


/* =============================================================================
		SNES CONTROLLER CONFIG
   ============================================================================= */ 

// pins for SNES controller
#define SNES_CLOCK 7
#define SNES_LATCH 8
#define SNES_DATA  9

// SNES controller object
SNESController snes(SNES_CLOCK, SNES_DATA, SNES_LATCH);

// gamepad object
Joystick_ gamepad(JOYSTICK_DEFAULT_REPORT_ID,JOYSTICK_TYPE_GAMEPAD,
  12, 0,                 // Button Count, Hat Switch Count
  false, false, false,   // no X, Y, or Z axis
  false, false, false,   // No Rx, Ry, or Rz
  false, false,          // No rudder or throttle
  false, false, false);  // No accelerator, brake, or steering


/* =============================================================================
		SD CARD READER CONFIG
   ============================================================================= */ 

// SS pin for SD card reader SPI connection
#define SPI_SS 10

// path to the config file of the SD card
#define CONFIG_FILE_PATH "snesControls.txt"

// the config file
File configFile;


/* =============================================================================
		DISPLAY CONFIG
   ============================================================================= */ 

// size of LCD display, in characters
#define LCD_ROWS 2
#define LCD_COLS 16

// LCD display object
LiquidCrystal_I2C lcd(0x27, LCD_COLS, LCD_ROWS); // what is 0x27 supposed to be


/* =============================================================================
		METHODS AND SUCH
   ============================================================================= */ 

/*
 * Initializes all peripherals and USB stuff
 */
void initHardware() {
	SD.begin(SPI_SS);						// SD card
	configFile = SD.open(CONFIG_FILE_PATH);
	snes.initialize();						// SNES controller
//	gamepad.begin();						// USB gamepad
	Keyboard.begin();						// keyboard
	lcd.init();								// LCD display
	lcd.backlight();
}

void setup() {
	Serial.begin(9600);
	initHardware();
}

void loop() {
	
}