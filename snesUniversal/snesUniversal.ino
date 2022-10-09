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
#include "Joystick.h" // this isn't in yet but I might do it someday


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

// array of game names for display
char** names;

// the index of the current mapping in use
unsigned int mapIndex = 0;

// number of configurations
unsigned int configCount;

// macro for the current mapping being used
#define currentMap mappings[mapIndex]

// gamepad object
Joystick_ gamepad(0x03,JOYSTICK_TYPE_GAMEPAD,
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
#define CONFIG_FILE_PATH "SNESCO~1.txt"
#define NAME_FILE_PATH   "names.txt"

// macro to skip a Windows line ending
#define skipLineEnding(f) {char* c = new char[2]; f.read(c, 2); delete c;}


/* =============================================================================
		SPECIAL KEYS MAPPING
   ============================================================================= */ 

// special keys are hard to type in the text files, so use these characters instead
#define CHAR_F1        '!'
#define CHAR_F2        '@'
#define CHAR_F3        '#'
#define CHAR_F4        '$'
#define CHAR_F5        '%'
#define CHAR_F6        '^'
#define CHAR_F7        '&'
#define CHAR_F8        '*'
#define CHAR_F9        '('
#define CHAR_F10       ')'
#define CHAR_F11       '_'
#define CHAR_F12       '+'
#define CHAR_ESC       'E'
#define CHAR_SHIFT     'S'
#define CHAR_CTRL      'C'
#define CHAR_ALT       'A'
#define CHAR_TAB       'T'
#define CHAR_ENTER     'N'
#define CHAR_BACKSPACE 'B'
#define CHAR_UP        'U'
#define CHAR_DOWN      'D'
#define CHAR_LEFT      'L'
#define CHAR_RIGHT     'R'


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
	//while (!Serial) {}
	SD.begin(SPI_SS);	// SD card
	snes.initialize();	// SNES controller
	gamepad.begin();	// HID gamepad
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
	File nameFile = SD.open(NAME_FILE_PATH);
	
	// number of configurations is first line
	configCount = configFile.read() - '0';
	mappings = new char*[configCount];
	names = new char*[configCount + 1];
	skipLineEnding(configFile);

	for (int i = 0; i < configCount; i++) {

		// next lines are mappings, with nothing in between
		mappings[i] = new char[SNES_NUM_BUTTONS];

		// corresponding name is on line of name file
		names[i] = new char[LCD_COLS];
		
		// read lines from config/name file and set mappings
		configFile.read(mappings[i], SNES_NUM_BUTTONS);
		nameFile.read(names[i], LCD_COLS);
		convertSpecialKeys(mappings[i]);
		
		// skip line endings
		skipLineEnding(configFile);
		skipLineEnding(nameFile);
	}

	names[configCount] = "USB Gamepad";
}


/*
 * Updates the LCD display
 */
void updateLCD() {
	// first row: config number
	lcd.setCursor(0,0);
	lcd.clear();
	lcd.print("Config No. ");
	lcd.print(mapIndex);

	// second row: name
	lcd.setCursor(0,1);
	lcd.print(names[mapIndex]);
}


/* 
 * Sets a button to the given state
 * 
 * args:
 *   btn   = index of button (0-SNES_NUM_BUTTONS)
 *   state = true if button is pressed, false otherwise
 */
void setButton(int btn, bool state) {
	if (mapIndex == configCount)
		gamepad.setButton(btn, state);
	else if (currentMap[btn] != '~') // tilde signifies no mapping
		state ? Keyboard.press(currentMap[btn]) : Keyboard.release(currentMap[btn]);
}


void convertSpecialKeys(char* mappings) {

	// parse through mappings with a giant switch statement
	for (int i = 0; i < SNES_NUM_BUTTONS; i++){
		switch (mappings[i]) {
		
		case CHAR_ALT:
			mappings[i] = KEY_LEFT_ALT;
			break;
		
		case CHAR_BACKSPACE:
			mappings[i] = KEY_BACKSPACE;
			break;
		
		case CHAR_CTRL:
			mappings[i] = KEY_LEFT_CTRL;
			break;
		
		case CHAR_ENTER:
			mappings[i] = KEY_RETURN;
			break;
		
		case CHAR_ESC:
			mappings[i] = KEY_ESC;
			break;
		
		case CHAR_SHIFT:
			mappings[i] = KEY_LEFT_SHIFT;
			break;
		
		case CHAR_TAB:
			mappings[i] = KEY_TAB;
			break;
		
		case CHAR_UP:
			mappings[i] = KEY_UP_ARROW;
			break;
		
		case CHAR_DOWN:
			mappings[i] = KEY_DOWN_ARROW;
			break;
		
		case CHAR_LEFT:
			mappings[i] = KEY_LEFT_ARROW;
			break;
		
		case CHAR_RIGHT:
			mappings[i] = KEY_RIGHT_ARROW;
			break;
		
		case CHAR_F1:
			mappings[i] = KEY_F1;
			break;
		
		case CHAR_F2:
			mappings[i] = KEY_F2;
			break;
		
		case CHAR_F3:
			mappings[i] = KEY_F3;
			break;
		
		case CHAR_F4:
			mappings[i] = KEY_F4;
			break;
		
		case CHAR_F5:
			mappings[i] = KEY_F5;
			break;
		
		case CHAR_F6:
			mappings[i] = KEY_F6;
			break;
		
		case CHAR_F7:
			mappings[i] = KEY_F7;
			break;
		
		case CHAR_F8:
			mappings[i] = KEY_F8;
			break;
		
		case CHAR_F9:
			mappings[i] = KEY_F9;
			break;
		
		case CHAR_F10:
			mappings[i] = KEY_F10;
			break;
		
		case CHAR_F11:
			mappings[i] = KEY_F11;
			break;
		
		case CHAR_F12:
			mappings[i] = KEY_F12;
			break;
		}
	}

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
		setButton(i, inputs & (1 << i));
	}

	// pool for buttons to change mapping
	if (digitalRead(UP_BTN) == LOW) {
		if (++mapIndex == configCount + 1)
			mapIndex = 0;
		updateLCD();
	} else if (digitalRead(DOWN_BTN) == LOW) {
		if (mapIndex-- == 0)
			mapIndex = configCount;
		updateLCD();
	}

	// wait for button to be released
	while (digitalRead(DOWN_BTN) == LOW || digitalRead(UP_BTN) == LOW) {}
}
