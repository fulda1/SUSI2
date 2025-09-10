/*
*	This example shows the correct decoding of the SUSI interface:
*   -   By turning on the LED built into the board when Function 0 is active.
*   -   By turning on two other output LEDs there function can be ptegrammed in CV#902 (FA) and CV#903 (FB)
*   -   Allows reading/writing of CVs saved in the Microcontroller's EEPROM
*/

//#include <stdint.h>     // Library with types "uintX_t" (automatically included by Arduino)
#include <SUSI2.h>        // Include the library for SUSI management
#include <EEPROM.h>       // Library for managing the internal EEPROM - Note, CH32V003 have no eeprom, emulated version must be used
                          // Note: The CH32 EEPROM library uses the user storage area, allowing 26 bytes of emulated EEPROM on the CH32V003.
                          //       but SUSI can call up to 127 addresses + indexed area. Handle with care!

#define LED_BUILTIN PD6   // CH32003 nano boards can differ. My one have LED on PD6
#define LED_FA PC0        // LED used as FA output
#define LED_FB PC1        // LED used as FB output
#define LED_FCf PC2       // LED used as FC forward output
#define LED_FCb PC3       // LED used as FC backward output


#define HW_VER 10         // Hardware version 1.0
#define VERSION_NO 1      // Version 1
#define SUBVERSION_NO 0   // SubVersion 0

//SUSI2 SUSI(2, 3);       // (CLK pin, DATA pin) this function is for compatibility only
SUSI2 SUSI;               // new version does not needed pin definition, as it use hardware receiver on PC5 and PC6 pins.

SUSI_FN_GROUP FAGroup, FBGroup, FCGroup;  // Function group for available functions FA, FB, FC
uint8_t FABit, FBBit, FCBit;     // Function mask for available functions FA, FB, FC
SUSI_DIRECTION RealDirection;    // real direction store (note, requested and real can differ as per acceleration/deceleration curve)

void notifySusiFunc(SUSI_FN_GROUP SUSI_FuncGrp, uint8_t SUSI_FuncState) {                                           // CallBack function that is invoked when a command for Functions is decoded
    switch (SUSI_FuncGrp) {                                                                                         // Choose which function group the command belongs to
        case SUSI_FN_0_4: {                                                                                         // Functions 0 to 4
            ((SUSI_FuncState & SUSI_FN_BIT_00) ? digitalWrite(LED_BUILTIN, HIGH) : digitalWrite(LED_BUILTIN, LOW)); // if Function 0 is active I turn on the LED_BUILTIN
            break;
        }
        default: {}
    }

      // simple function mapping to output
    if (SUSI_FuncGrp == FAGroup) {((SUSI_FuncState & FABit) ? digitalWrite(LED_FA, HIGH) : digitalWrite(LED_FA, LOW));}
    if (SUSI_FuncGrp == FBGroup) {((SUSI_FuncState & FBBit) ? digitalWrite(LED_FB, HIGH) : digitalWrite(LED_FB, LOW));}

      // Function mapping for direction depend functions
    if (SUSI_FuncGrp == FCGroup) {
      if (RealDirection == SUSI_DIR_FWD) { ((SUSI_FuncState & FCBit) ? digitalWrite(LED_FCf, HIGH) : digitalWrite(LED_FCf, LOW)); digitalWrite(LED_FCb, LOW);}
                                    else { ((SUSI_FuncState & FCBit) ? digitalWrite(LED_FCb, HIGH) : digitalWrite(LED_FCb, LOW)); digitalWrite(LED_FCf, LOW);}
    }
}

void notifySusiRealSpeed(uint8_t Speed, SUSI_DIRECTION Dir) {                                                       // CallBack function that is invoked when  the Actual Speed ​​and Direction are received
  RealDirection = Dir;                                                                                              // Save direction information for direction depend functions
  (void)(Speed);                                                                                                    // Have no usage for parameter "Speed", will mark it as "unused"
}

uint8_t notifySusiCVRead(uint8_t CV, uint8_t CVindex) {                                                             // CallBack function to read the value of a stored CV

    // all CVs are stored as for device #1, once acting as device #2 or #3, change to #1
    if ((CV>42) && (CV<83)) { return notifySusiCVRead(CV - 40, CVindex); }                                          // CVs for device 2 are same as for device 1
    if ((CV>82) && (CV<123)) { return notifySusiCVRead(CV - 80, CVindex); }                                         // CVs for device 3 are same as for device 1
    
    // Mandatory CVs
    if (CV == 0) {return EEPROM.read(0);}                                                                           // 0 = CV #897 module address
    // List of supported read only (fixed value) CVs
    if ((CV == 3) && (CVindex == 0)) {return MANUFACTER_ID;}                                                        // 3 = CV #900.0 Manufacturer identification / only in Bank 0
    if ((CV == 3) && (CVindex == 0)) {return HW_VER;}                                                               // 3 = CV #900.1 Hardware identification / only in Bank 1
    //if ((CV == 3) && (CVindex == 254)) {return MyAlternativeID;}                                                  // 3 = CV #900.254 Alternative Manufacturer ID / only in Bank 254
    if ((CV == 4) && (CVindex == 0)) {return VERSION_NO;}                                                           // 4 = CV #901.0 Version number / only in Bank 0
    if ((CV == 4) && (CVindex == 0)) {return SUBVERSION_NO;}                                                        // 4 = CV #901.1 Subversion number / only in Bank 1
    if ((CV == 4) && (CVindex == 254)) {return SUSI_VER;}                                                           // 4 = CV #901.254 SUSI version / only in Bank 254

    // Manufacturer managed CVs
    if (CV == 5) {return EEPROM.read(1);}                                                                           // 5 = CV #902 Manufacturer specific / Output #1(A) function in my case
    if (CV == 6) {return EEPROM.read(2);}                                                                           // 5 = CV #903 Manufacturer specific / Output #2(B) function in my case
    if (CV == 7) {return EEPROM.read(3);}                                                                           // 6 = CV #904 Manufacturer specific / Output #3(C) function in my case

    return 255;                                                                                                     // no other CVs supported
}

uint8_t notifySusiCVWrite(uint8_t CV, uint8_t CVindex, uint8_t Value) {                                             // CallBack function to write the value of a stored CV

    // all CVs are stored as for device #1, once acting as device #2 or #3, change to #1
    if ((CV>42) && (CV<83)) { return notifySusiCVWrite(CV - 40, CVindex, Value); }                                  // CVs for device 2 are same as for device 1
    if ((CV>82) && (CV<123)) { return notifySusiCVWrite(CV - 80, CVindex, Value); }                                 // CVs for device 3 are same as for device 1
    
    // Mandatory CVs
    if (CV == 0) {EEPROM.write(1, Value); EEPROM.commit(); ReadMyCVs(); return EEPROM.read(0);}                     // 0 = CV #897 module address
    // List of supported CVs READ ONLY -> return value without any action
    if ((CV == 3) && (CVindex == 0)) {return MANUFACTER_ID;}                                                        // 3 = CV #900.0 Manufacturer identification / only in Bank 0
    if ((CV == 3) && (CVindex == 0)) {return HW_VER;}                                                               // 3 = CV #900.1 Hardware identification / only in Bank 1
    //if ((CV == 3) && (CVindex == 254)) {return MyAlternativeID;}                                                  // 3 = CV #900.254 Alternative Manufacturer ID / only in Bank 254
    if ((CV == 4) && (CVindex == 0)) {return VERSION_NO;}                                                           // 4 = CV #901.0 Version number / only in Bank 0
    if ((CV == 4) && (CVindex == 0)) {return SUBVERSION_NO;}                                                        // 4 = CV #901.1 Subversion number / only in Bank 1
    if ((CV == 4) && (CVindex == 254)) {return SUSI_VER;}                                                           // 4 = CV #901.254 SUSI version / only in Bank 254

    // Manufacturer managed CVs
    if (CV == 5) {EEPROM.write(1, Value); EEPROM.commit(); ReadMyCVs(); return EEPROM.read(1);}                    // 5 = CV #902 Manufacturer specific / Output #1(A) function in my case
    if (CV == 6) {EEPROM.write(2, Value); EEPROM.commit(); ReadMyCVs(); return EEPROM.read(2);}                    // 6 = CV #903 Manufacturer specific / Output #2(B) function in my case
    if (CV == 7) {EEPROM.write(3, Value); EEPROM.commit(); ReadMyCVs(); return EEPROM.read(3);}                    // 7 = CV #904 Manufacturer specific / Output #3(C) function in my case
	                                                                                        // commit every write is not effective!
																							
    return 255;                                                                                                     // no other CVs supported
}

void ReadMyCVs(void) {                                                                                              // this function reads CV values and transform them to working variables
  uint8_t MemValue;                                                                                                 // Temporary value
  MemValue = EEPROM.read(0);                                                                                        // just check, if module number is 1, 2, or 3.
  if ((MemValue < 1) || (MemValue > MAX_ADDRESS_VALUE)) {EEPROM.write(0, DEFAULT_SLAVE_NUMBER); EEPROM.commit(); }  // No, then set default

  MemValue = EEPROM.read(1);                                                                                        // Output #1(A) function in my case
  if (MemValue > 68) {MemValue=0; EEPROM.write(1, MemValue); EEPROM.commit();}                                      // Check, if output is 0 to 68 - if not, set 0 as default
  FAGroup = (MemValue + 3) / 8;                                                                                     // this will fill gap of 3 missing functions in group 1
  if (MemValue == 0) {
    FABit = SUSI_FN_BIT_00;
  } else if (MemValue <= 4) {
    FABit = 1 << (MemValue - 1);
  } else {
    MemValue += 3;
    FABit = 1 << (MemValue % 8);
  }

  MemValue = EEPROM.read(2);                                                                                        // Output #2(B) function in my case
  if (MemValue > 68) {MemValue=0; EEPROM.write(2, MemValue); EEPROM.commit();}                                      // Check, if output is 0 to 68 - if not, set 0 as default
  FBGroup = (MemValue + 3) / 8;                                                                                     // this will fill gap of 3 missing functions in group 1
  if (MemValue == 0) {
    FBBit = SUSI_FN_BIT_00;
  } else if (MemValue <= 4) {
    FBBit = 1 << (MemValue - 1);
  } else {
    MemValue += 3;
    FBBit = 1 << (MemValue % 8);
  }

  MemValue = EEPROM.read(3);                                                                                        // Output #3(C) function in my case
  if (MemValue > 68) {MemValue=0; EEPROM.write(3, MemValue); EEPROM.commit();}                                      // Check, if output is 0 to 68 - if not, set 0 as default
  FCGroup = (MemValue + 3) / 8;                                                                                     // this will fill gap of 3 missing functions in group 1
  if (MemValue == 0) {
    FCBit = SUSI_FN_BIT_00;
  } else if (MemValue <= 4) {
    FCBit = 1 << (MemValue - 1);
  } else {
    MemValue += 3;
    FCBit = 1 << (MemValue % 8);
  }

}

void setup() {                                                                                                      // Setup Code
    pinMode(LED_BUILTIN, OUTPUT);                                                                                   // Set the pin to which the LED_BUILTIN is connected as output
    pinMode(LED_FA, OUTPUT);                                                                                        // Set the pin to which the LED_FA is connected as output
    pinMode(LED_FB, OUTPUT);                                                                                        // Set the pin to which the LED_FB is connected as output
    pinMode(LED_FCf, OUTPUT);                                                                                        // Set the pin to which the LED_FB is connected as output
    pinMode(LED_FCb, OUTPUT);                                                                                        // Set the pin to which the LED_FB is connected as output
    EEPROM.begin();                                                                                                 // init emulated eeprom
	  ReadMyCVs();                                                                                                    // Read local CVs to working variables
    SUSI.init();                                                                                                    // Start the library
}

void loop() {                                                                                                       // Code loop
    SUSI.process();                                                                                                 // Process the data acquired from the library as many times as possible
}
