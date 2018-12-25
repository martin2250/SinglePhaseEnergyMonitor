void eeprom_write(uint16_t address, int32_t value);
int32_t eeprom_read(uint16_t address);
void eeprom_write(uint16_t address, uint8_t *buffer, uint8_t length);
void eeprom_read(uint16_t address, uint8_t *buffer, uint8_t length);
