#include "Arduino.h"
#include <Wire.h>

#define EEPROM_ADDRESS 0x50

#if BUFFER_LENGTH < 34
#error I2C BUFFER TOO SMALL, change BUFFER_LENGTH to at least 34 in /libraries/Wire/Wire.h
#endif

bool eeprom_write(uint16_t address, uint8_t length, uint8_t *buffer)
{
	address *= 4;

	if (length > 32)
		length = 32;

	Wire.beginTransmission(EEPROM_ADDRESS);
	Wire.write((address >> 8) & 0x0F);
	Wire.write(address & 0xFF);

	while (length--)
		Wire.write(*(buffer++));

	if (Wire.endTransmission())
		return false;

	delay(5);

	return true;
}

bool eeprom_read(uint16_t address, uint8_t length, uint8_t *buffer)
{
	address *= 4;

	if (length > 32)
		length = 32;

	Wire.beginTransmission(EEPROM_ADDRESS);
	Wire.write((address >> 8) & 0x0F);
	Wire.write(address & 0xFF);

	if (Wire.endTransmission(false))
		return false;

	if (Wire.requestFrom((uint8_t)EEPROM_ADDRESS, length) != length)
		return false;

	while (Wire.available())
		*(buffer++) = Wire.read();

	return true;
}
