/*
 * snesUniversal.ino
 * 
 * Arduino sketch for a universal adapter to use an SNES controller to play PC games. Uses the Arduino Joystick Library
 * to make it a generic USB gamepad, but if it just doesn't work (which does happen sometimes), it can instead send
 * game-specific keyboard inputs, which are read from a text file on an SD card. A 16x2 character LCD display shows the
 * current configuration.
 * 
 * Author: Kyle M
 * Created 27 Apr 2021
 * 
 * 
 * pinout for SNES controller is as follows:
 * -----------------------------\
 * |  O   O   O   O | O   O   O  )
 * -----------------------------/
 *    1   2   3   4   5   6   7
 * 
 * 
 * HARDWARE USED:
 *  - Sparkfun Pro Micro
 *    - https://www.sparkfun.com/products/12587
 *    - an Arduino Leonardo or anything with an Atmega32u4 should work too (be wary of SPI and I2C pins though)
 *  - SNES controller
 *    - I cut up an extension cable because I don't believe in mutilating genuine hardware
 *  - Adafruit MicroSD Card Breakout Board
 *    - https://www.adafruit.com/product/254
 *  - SparkFun 16x2 Character LCD
 *    - https://www.sparkfun.com/products/9053
 *  - I2C display driver
 *    - https://www.amazon.com/HiLetgo-Interface-LCD1602-Address-Changeable/dp/B00VC2NEU8/
 * 
 * PINOUTS:
 *  - SNES Clock (pin 2)	=>	7
 *  - SNES Latch (pin 3)	=>	8
 *  - SNES Data (pin 4)		=>	9
 *  - microSD card CS pin	=>	10
 *  - up pushbutton			=>	4
 *  - down pushbutton		=>	5
 * 
 * CUSTOM LIBRARIES USED:
 *  - Arduino SNES Controller Library 
 *    - https://github.com/KyleM32767/Arduino-SNES-Controller-Library
 *  - Arduino Joystick Library (not used yet)
 *    - https://github.com/MHeironimus/ArduinoJoystickLibrary
 *  - LiquidCrystal_I2C
 *    - https://github.com/johnrickman/LiquidCrystal_I2C
 */

#include <SPI.h>
#include <SD.h>
#include <Keyboard.h>
#include <Wire.h>
#include "LiquidCrystal_I2C.h"
#include "SNES.h"
//#include "Joystick.h" // this isn't in yet but I might do it someday


/* =============================================================================
		SNES CONTROLLER CONFIG
   ============================================================================= */ 

// pins for SNES controller
#define SNES_CLOCK 7
#define SNES_LATCH 8
#define SNES_DATA  9

// SNES controller object
SNESController snes(SNES_CLOCK, SNES_DATA, SNES_LATCH);

// array of SNES buttons. array indeces correspond to gamepad buttons
// this is also the order in which bindings are written in the text file
//const long SNES_BUTTONS[] = { SNES_B_BUTTON, SNES_A_BUTTON, SNES_Y_BUTTON,
//						  SNES_X_BUTTON, SNES_L_BUTTON, SNES_R_BUTTON,
//						  SNES_SELECT_BUTTON, SNES_START_BUTTON, SNES_DPAD_DOWN,
//						  SNES_DPAD_UP, SNES_DPAD_LEFT, SNES_DPAD_RIGHT };

// 2-dimensional array of kay mappings, read from the text file
// modifier keys are just weird ascii characters, as used by Keyboard.press()
unsigned char** mappings;

// the index of the current mapping in use
unsigned int mapIndex = 0;

// number of configurations
unsigned int configCount;

// macro for the current mapping being used
#define currentMap mappings[mapIndex]

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
#define CONFIG_FILE_PATH "SNESCO~1.txt"

// macro to skip a Windows line ending
#define skipLineEnding() char* c = new char[2]; configFile.read(c, 2); delete c;


/* =============================================================================
		DISPLAY CONFIG
   ============================================================================= */ 

// size of LCD display, in characters
#define LCD_ROWS 2
#define LCD_COLS 16

// LCD display object
LiquidCrystal_I2C lcd(0x27, LCD_COLS, LCD_ROWS); // what is 0x27 supposed to be


/* =============================================================================
		MISCELLANEOUS CONFIG
   ============================================================================= */ 

// the buttons to change the config
#define UP_BTN   4
#define DOWN_BTN 5


/* =============================================================================
		METHODS AND SUCH
   ============================================================================= */

/*
 * Initializes all peripherals and USB stuff
 */
void initHardware() {
	Serial.begin(9600); // Serial
//	while (!Serial) {}
	SD.begin(SPI_SS);	// SD card
	snes.initialize();	// SNES controller
//	gamepad.begin();	// USB gamepad
	Keyboard.begin();	// keyboard
	lcd.init();			// LCD display
	lcd.backlight();
	pinMode(UP_BTN, INPUT_PULLUP);
	pinMode(DOWN_BTN, INPUT_PULLUP);
}


/* 
 * Loads mappings from the config file on the SD card
 */
void loadMappings() {
	
	// open config file
	File configFile = SD.open(CONFIG_FILE_PATH);
	
	// number of configurations is first line
	configCount = configFile.read() - '0';
	mappings = new char*[configCount]; Serial.println(configCount);
	skipLineEnding();

	for (int i = 0; i < configCount; i++) {
		// next lines are mappings, with nothing in between
		mappings[i] = new char[SNES_NUM_BUTTONS];
		configFile.read(mappings[i], SNES_NUM_BUTTONS);
		skipLineEnding();
	}
}


/*
 * Updates the LCD display
 */
void updateLCD() {
	// first row: config number
	lcd.setCursor(0,0);
	lcd.clear();
	lcd.print("Config No. ");lcd.print(mapIndex);
}


void setup() {
	initHardware();
	loadMappings();
	updateLCD();
}

void loop() {
	// get data from controller
	snes.update();
	uint16_t inputs = snes.getData();

	// press/release the respective keys
	for (int i = 0; i < SNES_NUM_BUTTONS; i++) {
		// capital X signifies no mapping
		if (currentMap[i] != '~') {
			if (inputs & (1 << i))
				Keyboard.press(currentMap[i]);
			else
				Keyboard.release(currentMap[i]);
		}		
	}

	// pool for buttons to change mapping
	if (digitalRead(UP_BTN) == LOW) {
		if (++mapIndex == configCount)
			mapIndex = 0;
		updateLCD();
	} else if (digitalRead(DOWN_BTN) == LOW) {
		if (mapIndex-- == 0)
			mapIndex = configCount - 1;
		updateLCD();
	}

	// wait for button to be released
	while (digitalRead(DOWN_BTN) == LOW || digitalRead(UP_BTN) == LOW) {}
}
