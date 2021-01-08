/*!
 *  @file MPR121PROX.h
 *
 *  This is a header file to be used in companion
 *  with Adafruit_MPR121.h to add or redefine registers used
 *  by the MPR121 while in the Proximity or Area Detect mode.
 *
 *  Jim Grover
 *
 *  BSD license, all text above must be included in any redistribution
 */
 
/*!
 *  New or redefined device register map
 */
enum {
  MPR121_PROXSTATUS_L = 0x00,  // Redefined
  MPR121_PROXSTATUS_H = 0x01,  // Redefined
  
  MPR121_ELEPROX_L = 0x1C,     // New
  MPR121_ELEPROX_H = 0x1D,     // New
  MPR121_PROXBASELINE = 0X2A,  // New
  
  MPR121_ELEPROX_MHDR = 0x36,  // New
  MPR121_ELEPROX_NHDAR = 0x37, // New
  MPR121_ELEPROX_NCLR = 0x38,  // New
  MPR121_ELEPROX_FDLR = 0x39,  // New
  MPR121_ELEPROX_MHDF = 0x3A,  // New
  MPR121_ELEPROX_NHDAF = 0x3B, // New
  MPR121_ELEPROX_NCLF = 0x3C,  // New
  MPR121_ELEPROX_FLDF = 0x3D,  // New
  MPR121_ELEPROX_NHDAT = 0x3E, // New
  MPR121_ELEPROX_NCLT = 0x3F,  // New
  MPR121_ELEPROX_FDLT = 0x40,  // New
  
  MPR121_PROXTOUCH = 0x59,     // New
  MPR121_PROXRELEASE = 0x5A,   // New
  
// MPR121_DEBOUNCE = 0x5B,        Defined in Adafruit_MPR121.h
// MPR121_CONFIG1 = 0x5C,         Defined in Adafruit_MPR121.h
// MPR121_CONFIG2 = 0x5D,         Defined in Adafruit_MPR121.h
  MPR121_CONFIG3 = 0x5E        // New
  };