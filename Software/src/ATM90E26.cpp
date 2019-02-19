#include "Arduino.h"
#include <SPI.h>
#include "ATM90E26.h"
#include "settings.h"
#include "eeprom.h"

#define ATM90_CS_PIN 2

uint16_t read_register(uint8_t address)
{
	SPISettings settings(500000, MSBFIRST, SPI_MODE2);

	SPI.beginTransaction(settings);

	digitalWrite(ATM90_CS_PIN, LOW);
	delayMicroseconds(1);

	SPI.transfer(address | (1 << 7));       // R/W flags
	delayMicroseconds(4);
	uint16_t value = SPI.transfer16(0xFFFF);

	digitalWrite(ATM90_CS_PIN, HIGH);

	return value;
}

void write_register(uint8_t address, uint16_t value)
{
	SPISettings settings(500000, MSBFIRST, SPI_MODE2);

	SPI.beginTransaction(settings);

	digitalWrite(ATM90_CS_PIN, LOW);
	delayMicroseconds(1);

	SPI.transfer(address);
	delayMicroseconds(4);
	SPI.transfer16(value);

	digitalWrite(ATM90_CS_PIN, HIGH);
}

uint8_t atm90e26_lgain_register[5] = { 0x04, 0x00, 0x01, 0x02, 0x03 };
uint8_t atm90e26_lgain_factor[5] = { 1, 4, 8, 16, 24 };
double pga_correction_factor = 1;

void initATM90E26()
{
	pinMode(ATM90_CS_PIN, OUTPUT);

	write_register(SoftReset, 0x789A);   // Perform soft reset

	delay(10);

	uint16_t mmode0_val = 0;
	mmode0_val |= (atm90e26_lgain_register[setting_current_pga_gain.value]) << 13;  // Lgain
	mmode0_val |= (2) << 11;                                                        // Ngain
	mmode0_val |= (1) << 10;                                                        // LNSel
	mmode0_val |= (0) << 8;                                                         // DisHPF
	mmode0_val |= (3) << 4;                                                         // Zxcon

	write_register(CalStart, 0x5678);
	write_register(PLconstH, (((uint32_t)setting_pl_const_24.value) >> 16) & 0xFFFF);
	write_register(PLconstL, ((uint32_t)setting_pl_const_24.value) & 0xFFFF);
	write_register(MMode, mmode0_val);

	write_register(AdjStart, 0x5678);
	write_register(Ugain, (uint16_t)setting_voltage_gain.value);
	write_register(IgainL, (uint16_t)setting_current_gain.value);
	write_register(Uoffset, (uint16_t)setting_voltage_offset.value);
	write_register(IoffsetL, (uint16_t)setting_current_offset.value);
	write_register(PoffsetL, (uint16_t)setting_act_power_offset.value);
	write_register(QoffsetL, (uint16_t)setting_rct_power_offset.value);

	pga_correction_factor = (1.0 + setting_pga_gain_error[setting_current_pga_gain.value].value / 10000.0) / atm90e26_lgain_factor[setting_current_pga_gain.value];
}
