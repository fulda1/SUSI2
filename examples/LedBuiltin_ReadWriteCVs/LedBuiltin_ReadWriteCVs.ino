/*
*	This example shows the correct decoding of the SUSI interface:
*   -   By turning on the LED built into the board when Function 0 is active.
*   -   Allows reading/writing of CVs saved in the Microcontroller's EEPROM
*/

//#include <stdint.h>     // Library with types "uintX_t" (automatically included by Arduino)
#include <SUSI2.h>        // Include the library for SUSI management
#include <EEPROM.h>       // Library for managing the internal EEPROM - Note, CH32V003 have no eeprom, emulated version must be used
                          // Note: The CH32 EEPROM library uses the user storage area, allowing 26 bytes of emulated EEPROM on the CH32V003.
                          //       but SUSI can call up to 127 addresses + indexed area. Handle with care!

#define LED_BUILTIN PD6   // CH32003 nano boards can differ. My one have LED on PD6

//SUSI2 SUSI(2, 3);      // (CLK pin, DATA pin) this function is for compatiility only
SUSI2 SUSI;            // new version does not needed pin definition, as it use hardware receiver on PC5 and PC6 pins.


void notifySusiFunc(SUSI_FN_GROUP SUSI_FuncGrp, uint8_t SUSI_FuncState) {                                           // CallBack function that is invoked when a command for Functions is decoded
    switch (SUSI_FuncGrp) {                                                                                         // Choose which function group the command belongs to
        case SUSI_FN_0_4: {                                                                                         // Functions 0 to 4
            ((SUSI_FuncState & SUSI_FN_BIT_00) ? digitalWrite(LED_BUILTIN, HIGH) : digitalWrite(LED_BUILTIN, LOW)); // if Function 0 is active I turn on the LED_BUILTIN
            break;
        }
        default: {}
    }
}

uint8_t notifySusiCVRead(uint16_t CV) {                                                                             // CallBack function to read the value of a stored CV
    if (CV < EEPROM.length()) {return EEPROM.read(CV);} else {return 255;}                                          // Returns the value stored in EEPROM

}

uint8_t notifySusiCVWrite(uint16_t CV, uint8_t Value) {                                                             // CallBack function to write the value of a stored CV
                                                                                                                    // If the required value is different I update the EEPROM, otherwise I do not modify it so as not to damage it.
    if (CV < EEPROM.length()) {EEPROM.write(CV, Value); EEPROM.commit();}       // commit every write is not effective!

    if (CV < EEPROM.length()) {return EEPROM.read(CV);} else {return 255;}                                          // Return the new value of the EEPROM
}

void setup() {                                                                                                      // Setup Code
    pinMode(LED_BUILTIN, OUTPUT);                                                                                   // Set the pin to which the LED_BUILTIN is connected as output
    EEPROM.begin();                                                                                                 // init emulated eeprom
    SUSI.init();                                                                                                    // Start the library
}

void loop() {                                                                                                       // Code loop
    SUSI.process();                                                                                                 // Process the data acquired from the library as many times as possible
}
