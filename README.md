# snesUniversal

A universal adapter to play modern PC games with a SNES controller. Mappings are loaded from `snesControls.txt` in the root directory of the SD card and using buttons, they can be cycled through.

## File Format

* First line is how many configs there are
* Next lines are strings of characters for mappings in the following order:
	* B
	* Y
	* Select
	* Start
	* Up
	* Down
	* Left
	* Right
	* A
	* X
	* L
	* R
* For modifier keys, use alt codes or a hex editor to add the specific bytes [as determined by those who wrote the keyboard library](https://www.arduino.cc/reference/en/language/functions/usb/keyboard/keyboardmodifiers/)
* The tilde ('~') means no mapping for that button
* File name is "snesControls.txt"
* Use Windows line endings (CRLF)

```
2
todo update
```

## Hardware Used

 * [Sparkfun Pro Micro](https://www.sparkfun.com/products/12587)
	 * an Arduino Leonardo or anything with an Atmega32u4 should work too (be wary of SPI and I2C pins though)
 * SNES controller
	 * I cut up an extension cable because I don't believe in mutilating genuine hardware
 * [Adafruit MicroSD Card Breakout Board](https://www.adafruit.com/product/254)
 * [SparkFun 16x2 Character LCD](https://www.sparkfun.com/products/9053)
 * [I2C display driver](https://www.amazon.com/HiLetgo-Interface-LCD1602-Address-Changeable/dp/B00VC2NEU8/)