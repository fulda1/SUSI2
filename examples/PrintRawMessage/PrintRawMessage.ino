/*
*	This example allows you to see raw form of all SUSI messages:
*   -   Print the received bytes on the screen (including those for CVs management)
*   -   Allows reading/writing of CVs saved in the Microcontroller's EEPROM
*/

//#include <stdint.h>     // Library with types "uintX_t" (automatically included by Arduino)
#include <SUSI2.h>        // Include the library for SUSI management
#include <EEPROM.h>       // Library for managing the internal EEPROM - Note, CH32V003 have no eeprom, emulated version must be used
                          // Note: The CH32 EEPROM library uses the user storage area, allowing 26 bytes of emulated EEPROM on the CH32V003.
                          //       but SUSI can call up to 127 addresses + indexed area. Handle with care!

//SUSI2 SUSI(2, 3);      // (CLK pin, DATA pin) il pin di Clock DEVE ESSERE di tipo interrupt, il pin Data puo' essere in pin qualsiasi: compresi gli analogici
SUSI2 SUSI;      // (CLK pin, DATA pin) il pin di Clock DEVE ESSERE di tipo interrupt, il pin Data puo' essere in pin qualsiasi: compresi gli analogici

void notifySusiRawMessage(uint8_t firstByte, uint8_t secondByte) {                                                  // CallBack function invoked when a message is waiting for decoding
    Serial.print("notifySusiRawMessage : ");

    Serial.print(firstByte, BIN);                                                                                   // Print the received bytes in binary format
    Serial.print(" - "); 
    Serial.print(secondByte, BIN); 

    Serial.print(" ( ");                                                                                            // Print the hex value of the received bytes
    Serial.print(firstByte < 16 ? "0" : "");
    Serial.print(firstByte, HEX); 
    Serial.print(" - "); 
    Serial.print(secondByte < 16 ? "0" : "");
    Serial.print(secondByte, HEX); 
    Serial.println(" )");
}

void notifySusiRawMessage3b(uint8_t firstByte, uint8_t secondByte, uint8_t thirdByte) {                             // CallBack function invoked when a 3 byte message is waiting for decoding
    Serial.print("notifySusiRawMessage (3 byte): ");

    Serial.print(firstByte, BIN);                                                                                   // Print the received bytes in binary format
    Serial.print(" | "); 
    Serial.print(secondByte, BIN); 
    Serial.print(" | "); 
    Serial.print(thirdByte, BIN); 

    Serial.print(" ( ");                                                                                            // Print the hex value of the received bytes
    Serial.print(firstByte < 16 ? "0" : "");
    Serial.print(firstByte, HEX); 
    Serial.print(" | "); 
    Serial.print(secondByte < 16 ? "0" : "");
    Serial.print(secondByte, HEX); 
    Serial.print(" | "); 
    Serial.print(thirdByte < 16 ? "0" : "");
    Serial.print(thirdByte, HEX); 
    Serial.println(" )");
}


uint8_t notifySusiCVRead(uint8_t CV, uint8_t CVindex) {                                                             // CallBack function to read the value of a stored CV
    if (CV < EEPROM.length()) {return EEPROM.read(CV);} else {return 255;}                                          // Returns the value stored in EEPROM

}

uint8_t notifySusiCVWrite(uint16_t CV, uint8_t CVindex, uint8_t Value) {                                            // CallBack function to write the value of a stored CV
                                                                                                                    // If the required value is different I update the EEPROM, otherwise I do not modify it so as not to damage it.
    if (CV < EEPROM.length()) {EEPROM.write(CV, Value); EEPROM.commit();}       // commit every write is not effective!

    if (CV < EEPROM.length()) {return EEPROM.read(CV);} else {return 255;}                                          // Return the new value of the EEPROM
}

void setup() {                                                                                                      // Setup Code
    Serial.begin(115200);                                                                                           // Starting Serial Communication
    while (!Serial) {}                                                                                              // Waiting for serial communication to be available

    Serial.println("SUSI Print Raw Messages:");                                                                     // Welcome message

    EEPROM.begin();                                                                                                 // init emulated eeprom
    SUSI.init();                                                                                                    // library initialisation
}

void loop() {                                                                                                       // Code loop
    SUSI.process();                                                                                                 // Process the data acquired from the library as many times as possible
}
