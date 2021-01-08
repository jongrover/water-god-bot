writeRegister(0x30,MPR121_CONFIG3);

uint16_t pdr = readRegister8(MPR121_ELEPROX_L) + readRegister(MPR121_ELEPROX_H)*256;
uint16_t ebv = readRegister8(MPR121_PROXBASELINE) << 2;

bool proxTouch = (bool)(readRegister(MPR121_PROXSTATUS_H)&&0x10) >> 4;
bool OVCF = (bool)readRegister(MPR121_PROXSTATUS_H) >> 7;
writeRegister8((uint8_t)MPR121_PROXSTATUS_H,(uint8_t)0x80);

writeRegister8((uint8_t)MPR121_DEBOUNCE,(uint8_t)0x74);

writeRegister8((uint8_t)MPR121_PMHDR,(uint8_t)0xFF);
writeRegister8((uint8_t)MPR121_PNHDR,(uint8_t)0xFF);
writeRegister8((uint8_t)MPR121_PMHDF,(uint8_t)0x01);
writeRegister8((uint8_t)MPR121_PNHDAF,(uint8_t)0x01);
writeRegister8((uint8_t)MPR121_PNCLF,(uint8_t)0xFF);
writeRegister8((uint8_t)MPR121_PFLDF,(uint8_t)0xFF);