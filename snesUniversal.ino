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
//#include <Keyboard.h>
//#include "LiquidCrystal_I2C.h"
//#include "SNES.h"
//#include "Joystick.h"


/* =============================================================================
		SNES CONTROLLER CONFIG
   ============================================================================= */ 

// pins for SNES controller
#define SNES_CLOCK 7
#define SNES_LATCH 8
#define SNES_DATA  9

// number of buttons (todo add to SNES library)
#define NUM_BUTTONS 12

// SNES controller object
//SNESController snes(SNES_CLOCK, SNES_DATA, SNES_LATCH);

// array of SNES buttons. array indeces correspond to gamepad buttons
// this is also the order in which bindings are written in the text file
//const long SNES_BUTTONS[] = { SNES_B_BUTTON, SNES_A_BUTTON, SNES_Y_BUTTON,
//						  SNES_X_BUTTON, SNES_L_BUTTON, SNES_R_BUTTON,
//						  SNES_SELECT_BUTTON, SNES_START_BUTTON, SNES_DPAD_DOWN,
//						  SNES_DPAD_UP, SNES_DPAD_LEFT, SNES_DPAD_RIGHT };

// 2-dimensional array of kay mappings, read from the text file
// modifier keys are just weird ascii characters, as used by Keyboard.press()
char** mappings;

// number of configurations
unsigned int configCount;

// gamepad object
//Joystick_ gamepad(JOYSTICK_DEFAULT_REPORT_ID,JOYSTICK_TYPE_GAMEPAD,
//  12, 0,                 // Button Count, Hat Switch Count
//  false, false, false,   // no X, Y, or Z axis
//  false, false, false,   // No Rx, Ry, or Rz
//  false, false,          // No rudder or throttle
//  false, false, false);  // No accelerator, brake, or steering


/* =============================================================================
		SD CARD READER CONFIG
   ============================================================================= */ 

// SS pin for SD card reader SPI connection
#define SPI_SS 10

// path to the config file of the SD card
#define CONFIG_FILE_PATH "snesco~1.txt"

// macro to skip a Windows line ending
#define skipLineEnding() char* c = new char[2]; configFile.read(c, 2); delete c;


/* =============================================================================
		DISPLAY CONFIG
   ============================================================================= */ 

// size of LCD display, in characters
#define LCD_ROWS 2
#define LCD_COLS 16

// LCD display object
//LiquidCrystal_I2C lcd(0x27, LCD_COLS, LCD_ROWS); // what is 0x27 supposed to be


/* =============================================================================
		METHODS AND SUCH
   ============================================================================= */ 

/*
 * Initializes all peripherals and USB stuff
 */
void initHardware() {
	Serial.begin(9600); // Serial
	while (!Serial) {}
	SD.begin(SPI_SS);	// SD card
//	snes.initialize();	// SNES controller
//	gamepad.begin();	// USB gamepad
//	Keyboard.begin();	// keyboard
//	lcd.init();			// LCD display
//	lcd.backlight();
}


/* 
 * Loads mappings from the config file on the SD card
 */
void loadMappings() {
	
	// open config file
	File configFile = SD.open(CONFIG_FILE_PATH);
	
	// number of configurations is first line
	configCount = configFile.read() - '0'; Serial.println(configCount);
	mappings = new char*[configCount];
	skipLineEnding();

	for (int i = 0; i < configCount; i++) {
		// next lines are mappings, with nothing in between
		mappings[i] = new char[NUM_BUTTONS];
		configFile.read(mappings[i], NUM_BUTTONS);
		skipLineEnding();
	}
}

void setup() {
	initHardware();
	loadMappings();

	for (int i = 0; i < configCount; i++) {
		Serial.println(mappings[i]);
	}
}

void loop() {
	
}
