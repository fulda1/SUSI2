/*
*   This example allows you to see the correct translation of SUSI messages:
*   -   Print the received decoded commands on the screen
*   -   Allows reading/writing of CVs saved in the Microcontroller's EEPROM
*/
// note: as it contains lot of texts, it is recomentded to enable LTO optimalization.

//#include <stdint.h>     // Library with types "uintX_t" (automatically included by Arduino)
#include <SUSI2.h>        // Include the library for SUSI management
#include <EEPROM.h>       // Library for managing the internal EEPROM - Note, CH32V003 have no eeprom, emulated version must be used
                          // Note: The CH32 EEPROM library uses the user storage area, allowing 26 bytes of emulated EEPROM on the CH32V003.
                          //       but SUSI can call up to 127 addresses + indexed area. Handle with care!

//SUSI2 SUSI(2, 3);      // (CLK pin, DATA pin) this function is for compatiility only
SUSI2 SUSI;            // new version does not needed pin definition, as it use hardware receiver on PC5 and PC6 pins.

// Uncomment the #define below to print the Digital Functions status
#define NOTIFY_SUSI_FUNC
#ifdef  NOTIFY_SUSI_FUNC
void notifySusiFunc(SUSI_FN_GROUP SUSI_FuncGrp, uint8_t SUSI_FuncState) {
    Serial.print("notifySusiFunc: ");

    switch (SUSI_FuncGrp) {
    case SUSI_FN_0_4:
        Serial.print(" FN 0: ");
        Serial.print((SUSI_FuncState & SUSI_FN_BIT_00) ? "1  " : "0  ");
        Serial.print("FN 1-4: ");
        Serial.print((SUSI_FuncState & SUSI_FN_BIT_01) ? "1  " : "0  ");
        Serial.print((SUSI_FuncState & SUSI_FN_BIT_02) ? "1  " : "0  ");
        Serial.print((SUSI_FuncState & SUSI_FN_BIT_03) ? "1  " : "0  ");
        Serial.println((SUSI_FuncState & SUSI_FN_BIT_04) ? "1  " : "0  ");
        break;

    case SUSI_FN_5_12:
        Serial.print(" FN 5-12: ");
        Serial.print((SUSI_FuncState & SUSI_FN_BIT_05) ? "1  " : "0  ");
        Serial.print((SUSI_FuncState & SUSI_FN_BIT_06) ? "1  " : "0  ");
        Serial.print((SUSI_FuncState & SUSI_FN_BIT_07) ? "1  " : "0  ");
        Serial.print((SUSI_FuncState & SUSI_FN_BIT_08) ? "1  " : "0  ");
        Serial.print((SUSI_FuncState & SUSI_FN_BIT_09) ? "1  " : "0  ");
        Serial.print((SUSI_FuncState & SUSI_FN_BIT_10) ? "1  " : "0  ");
        Serial.print((SUSI_FuncState & SUSI_FN_BIT_11) ? "1  " : "0  ");
        Serial.println((SUSI_FuncState & SUSI_FN_BIT_12) ? "1  " : "0  ");
        break;

    case SUSI_FN_13_20:
        Serial.print(" FN 13-20: ");
        Serial.print((SUSI_FuncState & SUSI_FN_BIT_13) ? "1  " : "0  ");
        Serial.print((SUSI_FuncState & SUSI_FN_BIT_14) ? "1  " : "0  ");
        Serial.print((SUSI_FuncState & SUSI_FN_BIT_15) ? "1  " : "0  ");
        Serial.print((SUSI_FuncState & SUSI_FN_BIT_16) ? "1  " : "0  ");
        Serial.print((SUSI_FuncState & SUSI_FN_BIT_17) ? "1  " : "0  ");
        Serial.print((SUSI_FuncState & SUSI_FN_BIT_18) ? "1  " : "0  ");
        Serial.print((SUSI_FuncState & SUSI_FN_BIT_19) ? "1  " : "0  ");
        Serial.println((SUSI_FuncState & SUSI_FN_BIT_20) ? "1  " : "0  ");
        break;

    case SUSI_FN_21_28:
        Serial.print(" FN 21-28: ");
        Serial.print((SUSI_FuncState & SUSI_FN_BIT_21) ? "1  " : "0  ");
        Serial.print((SUSI_FuncState & SUSI_FN_BIT_22) ? "1  " : "0  ");
        Serial.print((SUSI_FuncState & SUSI_FN_BIT_23) ? "1  " : "0  ");
        Serial.print((SUSI_FuncState & SUSI_FN_BIT_24) ? "1  " : "0  ");
        Serial.print((SUSI_FuncState & SUSI_FN_BIT_25) ? "1  " : "0  ");
        Serial.print((SUSI_FuncState & SUSI_FN_BIT_26) ? "1  " : "0  ");
        Serial.print((SUSI_FuncState & SUSI_FN_BIT_27) ? "1  " : "0  ");
        Serial.println((SUSI_FuncState & SUSI_FN_BIT_28) ? "1  " : "0  ");
        break;

    case SUSI_FN_29_36:
        Serial.print(" FN 29-36: ");
        Serial.print((SUSI_FuncState & SUSI_FN_BIT_29) ? "1  " : "0  ");
        Serial.print((SUSI_FuncState & SUSI_FN_BIT_30) ? "1  " : "0  ");
        Serial.print((SUSI_FuncState & SUSI_FN_BIT_31) ? "1  " : "0  ");
        Serial.print((SUSI_FuncState & SUSI_FN_BIT_32) ? "1  " : "0  ");
        Serial.print((SUSI_FuncState & SUSI_FN_BIT_33) ? "1  " : "0  ");
        Serial.print((SUSI_FuncState & SUSI_FN_BIT_34) ? "1  " : "0  ");
        Serial.print((SUSI_FuncState & SUSI_FN_BIT_35) ? "1  " : "0  ");
        Serial.println((SUSI_FuncState & SUSI_FN_BIT_36) ? "1  " : "0  ");
        break;

    case SUSI_FN_37_44:
        Serial.print(" FN 37-44: ");
        Serial.print((SUSI_FuncState & SUSI_FN_BIT_37) ? "1  " : "0  ");
        Serial.print((SUSI_FuncState & SUSI_FN_BIT_38) ? "1  " : "0  ");
        Serial.print((SUSI_FuncState & SUSI_FN_BIT_39) ? "1  " : "0  ");
        Serial.print((SUSI_FuncState & SUSI_FN_BIT_40) ? "1  " : "0  ");
        Serial.print((SUSI_FuncState & SUSI_FN_BIT_41) ? "1  " : "0  ");
        Serial.print((SUSI_FuncState & SUSI_FN_BIT_42) ? "1  " : "0  ");
        Serial.print((SUSI_FuncState & SUSI_FN_BIT_43) ? "1  " : "0  ");
        Serial.println((SUSI_FuncState & SUSI_FN_BIT_44) ? "1  " : "0  ");
        break;

    case SUSI_FN_45_52:
        Serial.print(" FN 45-52: ");
        Serial.print((SUSI_FuncState & SUSI_FN_BIT_45) ? "1  " : "0  ");
        Serial.print((SUSI_FuncState & SUSI_FN_BIT_46) ? "1  " : "0  ");
        Serial.print((SUSI_FuncState & SUSI_FN_BIT_47) ? "1  " : "0  ");
        Serial.print((SUSI_FuncState & SUSI_FN_BIT_48) ? "1  " : "0  ");
        Serial.print((SUSI_FuncState & SUSI_FN_BIT_49) ? "1  " : "0  ");
        Serial.print((SUSI_FuncState & SUSI_FN_BIT_50) ? "1  " : "0  ");
        Serial.print((SUSI_FuncState & SUSI_FN_BIT_51) ? "1  " : "0  ");
        Serial.println((SUSI_FuncState & SUSI_FN_BIT_52) ? "1  " : "0  ");
        break;

    case SUSI_FN_53_60:
        Serial.print(" FN 53-60: ");
        Serial.print((SUSI_FuncState & SUSI_FN_BIT_53) ? "1  " : "0  ");
        Serial.print((SUSI_FuncState & SUSI_FN_BIT_54) ? "1  " : "0  ");
        Serial.print((SUSI_FuncState & SUSI_FN_BIT_55) ? "1  " : "0  ");
        Serial.print((SUSI_FuncState & SUSI_FN_BIT_56) ? "1  " : "0  ");
        Serial.print((SUSI_FuncState & SUSI_FN_BIT_57) ? "1  " : "0  ");
        Serial.print((SUSI_FuncState & SUSI_FN_BIT_58) ? "1  " : "0  ");
        Serial.print((SUSI_FuncState & SUSI_FN_BIT_59) ? "1  " : "0  ");
        Serial.println((SUSI_FuncState & SUSI_FN_BIT_60) ? "1  " : "0  ");
        break;

    case SUSI_FN_61_68:
        Serial.print(" FN 61-68: ");
        Serial.print((SUSI_FuncState & SUSI_FN_BIT_61) ? "1  " : "0  ");
        Serial.print((SUSI_FuncState & SUSI_FN_BIT_62) ? "1  " : "0  ");
        Serial.print((SUSI_FuncState & SUSI_FN_BIT_63) ? "1  " : "0  ");
        Serial.print((SUSI_FuncState & SUSI_FN_BIT_64) ? "1  " : "0  ");
        Serial.print((SUSI_FuncState & SUSI_FN_BIT_65) ? "1  " : "0  ");
        Serial.print((SUSI_FuncState & SUSI_FN_BIT_66) ? "1  " : "0  ");
        Serial.print((SUSI_FuncState & SUSI_FN_BIT_67) ? "1  " : "0  ");
        Serial.println((SUSI_FuncState & SUSI_FN_BIT_68) ? "1  " : "0  ");
        break;
    }
}
#endif

// Uncomment the #define below to print when a binary state function is called (short and long version)
#define NOTIFY_SUSI_BINARY_STATE
#ifdef  NOTIFY_SUSI_BINARY_STATE
void notifySusiBinaryState(uint8_t Command, uint8_t CommandState) {
    Serial.print("notifySusiBinaryState: ");
    Serial.print(Command);
    Serial.print(" ; State: ");
    Serial.println(CommandState);
};
void notifySusiBinaryStateL(uint16_t Command, uint8_t CommandState) {
    Serial.print("notifySusiBinaryStateL (long): ");
    Serial.print(Command);
    Serial.print(" ; State: ");
    Serial.println(CommandState);
};
#endif

// Uncomment the #define below to print the status of the AUXs controlled directly from the Master
#define NOTIFY_SUSI_AUX
#ifdef  NOTIFY_SUSI_AUX
void notifySusiAux(SUSI_AUX_GROUP SUSI_auxGrp, uint8_t SUSI_AuxState) {
    Serial.print("notifySusiAux: ");

    switch (SUSI_auxGrp) {
    case SUSI_AUX_1_8:
        Serial.print(" AUX 1-8: ");
        Serial.print((SUSI_AuxState & SUSI_AUX_BIT_01) ? "1  " : "0  ");
        Serial.print((SUSI_AuxState & SUSI_AUX_BIT_02) ? "1  " : "0  ");
        Serial.print((SUSI_AuxState & SUSI_AUX_BIT_03) ? "1  " : "0  ");
        Serial.print((SUSI_AuxState & SUSI_AUX_BIT_04) ? "1  " : "0  ");
        Serial.print((SUSI_AuxState & SUSI_AUX_BIT_05) ? "1  " : "0  ");
        Serial.print((SUSI_AuxState & SUSI_AUX_BIT_06) ? "1  " : "0  ");
        Serial.print((SUSI_AuxState & SUSI_AUX_BIT_07) ? "1  " : "0  ");
        Serial.println((SUSI_AuxState & SUSI_AUX_BIT_08) ? "1  " : "0  ");
        break;

    case SUSI_AUX_9_16:
        Serial.print(" AUX 9-16: ");
        Serial.print((SUSI_AuxState & SUSI_AUX_BIT_09) ? "1  " : "0  ");
        Serial.print((SUSI_AuxState & SUSI_AUX_BIT_10) ? "1  " : "0  ");
        Serial.print((SUSI_AuxState & SUSI_AUX_BIT_11) ? "1  " : "0  ");
        Serial.print((SUSI_AuxState & SUSI_AUX_BIT_12) ? "1  " : "0  ");
        Serial.print((SUSI_AuxState & SUSI_AUX_BIT_13) ? "1  " : "0  ");
        Serial.print((SUSI_AuxState & SUSI_AUX_BIT_14) ? "1  " : "0  ");
        Serial.print((SUSI_AuxState & SUSI_AUX_BIT_15) ? "1  " : "0  ");
        Serial.println((SUSI_AuxState & SUSI_AUX_BIT_16) ? "1  " : "0  ");
        break;

    case SUSI_AUX_17_24:
        Serial.print(" AUX 17-24: ");
        Serial.print((SUSI_AuxState & SUSI_AUX_BIT_17) ? "1  " : "0  ");
        Serial.print((SUSI_AuxState & SUSI_AUX_BIT_18) ? "1  " : "0  ");
        Serial.print((SUSI_AuxState & SUSI_AUX_BIT_19) ? "1  " : "0  ");
        Serial.print((SUSI_AuxState & SUSI_AUX_BIT_20) ? "1  " : "0  ");
        Serial.print((SUSI_AuxState & SUSI_AUX_BIT_21) ? "1  " : "0  ");
        Serial.print((SUSI_AuxState & SUSI_AUX_BIT_22) ? "1  " : "0  ");
        Serial.print((SUSI_AuxState & SUSI_AUX_BIT_23) ? "1  " : "0  ");
        Serial.println((SUSI_AuxState & SUSI_AUX_BIT_24) ? "1  " : "0  ");
        break;

    case SUSI_AUX_25_32:
        Serial.print(" AUX 25-32: ");
        Serial.print((SUSI_AuxState & SUSI_AUX_BIT_25) ? "1  " : "0  ");
        Serial.print((SUSI_AuxState & SUSI_AUX_BIT_26) ? "1  " : "0  ");
        Serial.print((SUSI_AuxState & SUSI_AUX_BIT_27) ? "1  " : "0  ");
        Serial.print((SUSI_AuxState & SUSI_AUX_BIT_28) ? "1  " : "0  ");
        Serial.print((SUSI_AuxState & SUSI_AUX_BIT_29) ? "1  " : "0  ");
        Serial.print((SUSI_AuxState & SUSI_AUX_BIT_30) ? "1  " : "0  ");
        Serial.print((SUSI_AuxState & SUSI_AUX_BIT_31) ? "1  " : "0  ");
        Serial.println((SUSI_AuxState & SUSI_AUX_BIT_32) ? "1  " : "0  ");
        break;
    }
}
#endif

// Uncomment the #define below to print when a Trigger Pulse is called
#define NOTIFY_SUSI_TRIGGER_PULSE
#ifdef  NOTIFY_SUSI_TRIGGER_PULSE
void notifySusiTriggerPulse(uint8_t state) {
    Serial.print("notifySusiTriggerPulse: ");
    Serial.println((state & 1) ? "Pulse!  " : "Not Pulse  ");
};
#endif

// Uncomment the #define below to print the motor's current consumption
#define NOTIFY_SUSI_MOTOR_CURRENT
#ifdef  NOTIFY_SUSI_MOTOR_CURRENT
void notifySusiMotorCurrent(int8_t current) {
    Serial.print("notifySusiCurrentAbsorption: ");
    Serial.println(current, DEC);
};
#endif

// Uncomment the #define below to print the Requested Speed ​​data from the Control Panel to the Decoder
#define NOTIFY_SUSI_REQUEST_SPEED
#ifdef  NOTIFY_SUSI_REQUEST_SPEED
void notifySusiRequestSpeed(uint8_t Speed, SUSI_DIRECTION Dir) {
    Serial.print("notifySusiRequestSpeed: ");
    Serial.print(" Speed: ");
    Serial.print(Speed, DEC);
    Serial.print(" Dir: ");
    Serial.println((Dir == SUSI_DIR_FWD) ? "Forward" : "Reverse");
};
#endif

// Uncomment the #define below to print actual speed data.
#define NOTIFY_SUSI_REAL_SPEED
#ifdef  NOTIFY_SUSI_REAL_SPEED
void notifySusiRealSpeed(uint8_t Speed, SUSI_DIRECTION Dir) {
    Serial.print("notifySusiRealSpeed: ");
    Serial.print(" Speed: ");
    Serial.print(Speed, DEC);
    Serial.print(" Dir: ");
    Serial.println((Dir == SUSI_DIR_FWD) ? "Forward" : "Reverse");
};
#endif

// Uncomment the #define below to print the motor Load
#define NOTIFY_SUSI_MOTOR_LOAD
#ifdef  NOTIFY_SUSI_MOTOR_LOAD
void notifySusiMotorLoad(int8_t load) {
    Serial.print("notifySusiMotorLoad: ");
    Serial.println(load, DEC);
};
#endif

// Uncomment the #define below to print the status of analog functions
#define NOTIFY_SUSI_ANALOG_FUNCTION
#ifdef  NOTIFY_SUSI_ANALOG_FUNCTION
void notifySusiAnalogFunction(SUSI_AN_GROUP SUSI_AnalogGrp, uint8_t SUSI_AnalogState) {
    Serial.print("notifySusiAnalog: ");
    switch (SUSI_AnalogGrp) {
    case SUSI_AN_FN_1:
        Serial.print(" Analog function 1 : ");
        Serial.println(SUSI_AnalogState);
        break;

    case SUSI_AN_FN_2:
        Serial.print(" Analog function 2 : ");
        Serial.println(SUSI_AnalogState);
        break;

    case SUSI_AN_FN_3:
        Serial.print(" Analog function 3 : ");
        Serial.println(SUSI_AnalogState);
        break;

    case SUSI_AN_FN_4:
        Serial.print(" Analog function 4 : ");
        Serial.println(SUSI_AnalogState);
        break;

    case SUSI_AN_FN_5:
        Serial.print(" Analog function 5 : ");
        Serial.println(SUSI_AnalogState);
        break;

    case SUSI_AN_FN_6:
        Serial.print(" Analog function 6 : ");
        Serial.println(SUSI_AnalogState);
        break;

    case SUSI_AN_FN_7:
        Serial.print(" Analog function 7 : ");
        Serial.println(SUSI_AnalogState);
        break;

    case SUSI_AN_FN_8:
        Serial.print(" Analog function 8 : ");
        Serial.println(SUSI_AnalogState);
        break;
    }
}
#endif

// Uncomment the #define below to print 'Direct Analog' commands
#define NOTIFY_SUSI_ANALOG_DIRECT_COMMAND
#ifdef  NOTIFY_SUSI_ANALOG_DIRECT_COMMAND
void notifySusiAnalogDirectCommand(uint8_t commandNumber, uint8_t Command) {
    Serial.print("notifySusiAnalogDirectCommand: ");
    Serial.print(" Command Number: ");
    Serial.print(commandNumber);
    Serial.print(" Command: ");
    Serial.println(Command);
};
#endif

// Uncomment the #define below to print 'No Operation' commands - this is for testing purposes only.
#define NOTIFY_SUSI_NO_OPERATION
#ifdef  NOTIFY_SUSI_NO_OPERATION
void notifySusiNoOperation(uint8_t commandArgument) {
    Serial.print("notifySusiNoOperation: ");
    Serial.print(" commandArgument: ");
    Serial.println(commandArgument,DEC);
};
#endif

// Uncomment the #define below to print the Master decoder address
#define NOTIFY_SUSI_MASTER_ADDRESS
#ifdef  NOTIFY_SUSI_MASTER_ADDRESS
void notifySusiMasterAddress(uint16_t MasterAddress) {
    Serial.print("notifySusiMasterAddress: ");
    Serial.println(MasterAddress);
};
#endif

// Uncomment the #define below to print the Module Control commands.
#define NOTIFY_SUSI_MODULE_CONTROLL
#ifdef  NOTIFY_SUSI_MODULE_CONTROLL
void notifySusiControllModule(uint8_t ModuleControll) {
    Serial.print("notifySusiControllModule: ");
    Serial.println(ModuleControll);
};
#endif



/* Methods for CV Manipulation */

// Uncomment the #define below to show the CV Compare command
#define NOTIFY_SUSI_CV_READ
#ifdef  NOTIFY_SUSI_CV_READ
uint8_t notifySusiCVRead(uint8_t CV, uint8_t CVindex) {
    Serial.print("notifySusiCVRead: ");
    Serial.print(" CV: ");
    Serial.print(CV, DEC);
    Serial.print(" Index: ");
    Serial.print(CVindex, DEC);
    Serial.print(" Read Value: ");
    if (CV < EEPROM.length()) {Serial.println(EEPROM.read(CV), DEC);} else {Serial.println("out of EEPROM");}
    
    if (CV < EEPROM.length()) {return EEPROM.read(CV);} else {return 255;}
}
#endif

// Uncomment the #define below to show the CV Writing command
#define NOTIFY_SUSI_WRITE
#ifdef  NOTIFY_SUSI_WRITE
uint8_t notifySusiCVWrite(uint8_t CV, uint8_t CVindex, uint8_t Value) {
    Serial.print("notifySusiCVWrite: ");
    Serial.print(" CV: ");
    Serial.print(CV, DEC);
    Serial.print(" Index: ");
    Serial.print(CVindex, DEC);
    Serial.print(" Value to Write: ");
    Serial.print(Value, DEC);

    if (CV < EEPROM.length()) {EEPROM.write(CV, Value); EEPROM.commit();}       // commit every write is not effective!

    Serial.print(" New CV Value: ");
    if (CV < EEPROM.length()) {Serial.println(EEPROM.read(CV), DEC);} else {Serial.println("out of EEPROM");}

    if (CV < EEPROM.length()) {return EEPROM.read(CV);} else {return 255;}
    
}
 #endif

// Uncomment the #define below to show when a CV reset is required
#define NOTIFY_SUSI_CV_RESET
#ifdef  NOTIFY_SUSI_CV_RESET
void notifyCVResetFactoryDefault(uint8_t Value) {
    Serial.print("notifyCVResetFactoryDefault: ");
    Serial.println(Value);
}
#endif

// Uncomment the #define below to display when a unknown command is received.
#define NOTIFY_SUSI_UNKNOWN
#ifdef  NOTIFY_SUSI_UNKNOWN
void notifySusiUnknownMessage(uint8_t firstByte, uint8_t secondByte) {                                                  
    Serial.print("notifySusiUnknownMessage : ");

    Serial.print(" ( 0x");                                                                                            
    Serial.print(firstByte < 16 ? "0" : "");
    Serial.print(firstByte, HEX); 
    Serial.print(", 0x"); 
    Serial.print(secondByte < 16 ? "0" : "");
    Serial.print(secondByte, HEX); 
    Serial.println(" )");
}
#endif

void setup() {                                                                                                      // Setup Code
    Serial.begin(115200);                                                                                           // Starting Serial Communication
    while (!Serial) {}                                                                                              // Waiting for serial communication to be available

    Serial.println("SUSI Print Decoded Messages:");                                                                 // Welcome message

    EEPROM.begin();                                                                                                 // init emulated eeprom
    SUSI.init();                                                                                                    // library initialisation

}

void loop() {                                                                                                       // Code loop
    SUSI.process();                                                                                                 // Process the data acquired from the library as many times as possible
}
