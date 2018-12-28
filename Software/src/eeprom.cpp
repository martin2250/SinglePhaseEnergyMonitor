#include "Arduino.h"
#include <Wire.h>

#define EEPROM_ADDRESS 0x50

void eeprom_write(uint16_t address, int32_t value)
{
	address *= 4;

	Wire.beginTransmission(EEPROM_ADDRESS);
	Wire.write((address >> 8) & 0x0F);
	Wire.write(address & 0xFF);

	uint32_t value_unsigned = (uint32_t)value;
	Wire.write((value_unsigned >> 24) & 0xFF);
	Wire.write((value_unsigned >> 16) & 0xFF);
	Wire.write((value_unsigned >> 8) & 0xFF);
	Wire.write(value_unsigned & 0xFF);

	Wire.endTransmission();

	delay(5);
}

void eeprom_write(uint16_t address, uint8_t *buffer, uint8_t length)
{
	address *= 4;

	if(length > 30)
		length = 30;

	Wire.beginTransmission(EEPROM_ADDRESS);
	Wire.write((address >> 8) & 0x0F);
	Wire.write(address & 0xFF);

	while(length--)
		Wire.write(*(buffer++));

	Wire.endTransmission();

	delay(5);
}

int32_t eeprom_read(uint16_t address)
{
	address *= 4;

	Wire.beginTransmission(EEPROM_ADDRESS);
	Wire.write((address >> 8) & 0x0F);
	Wire.write(address & 0xFF);

	Wire.endTransmission(false);
	Wire.requestFrom((uint8_t)EEPROM_ADDRESS, (uint8_t)4);

	uint32_t value_unsigned = 0;

	while(Wire.available())
		value_unsigned = (value_unsigned << 8) | Wire.read();

	return (int32_t)value_unsigned;
}

void eeprom_read(uint16_t address, uint8_t *buffer, uint8_t length)
{
	address *= 4;

	if(length > 30)
		length = 30;

	Wire.beginTransmission(EEPROM_ADDRESS);
	Wire.write((address >> 8) & 0x0F);
	Wire.write(address & 0xFF);

	Wire.endTransmission(false);
	Wire.requestFrom((uint8_t)EEPROM_ADDRESS, length);

	while(Wire.available())
		*(buffer++) = Wire.read();
}
