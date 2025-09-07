/*
  SUSI / RCN-600 / S-9.4.1 implementation library
  (including RCN-602 / S-9.4.3 support)
  This library is heavily inspired by https://github.com/TheFidax/Rcn600/
  At minimum trying to keep same interface.
  Main change is, that this one is developed for little more actual processor CH32V003. (with theoretical upgrade to other CH32V.. family, and some STM32.. family)
  
  Created by Jindra Fucik / https://www.fucik.name
  
  Main concept: all SUSI stream is two wire SPI communication (half duplex). This version utilize SPI hardware for receive bytes. On top, it utilize Timer1 in monostable mode. It means, that measure gap from last falling edge and if exceeds 7 ms, it reset receiver for synchronization. It means, main CPU program is not disturbed by receiving bits and bytes. 

*/

#ifndef SUSI2_h
#define SUSI2_h

#include <Arduino.h>                                                                                                        // Library for typical Arduino IDE functions
// #include <stdint.h>                                                                                                         // Type library for 'uintX_t' types

#include "DataHeaders/SUSI_DATA_TYPE.h"                                                                                     // Symbolic Types for CallBack Functions
#include "DataHeaders/SUSI_FN_BIT.h"                                                                                        // bit for the control of the Digital Functions
#include "DataHeaders/SUSI_AUX_BIT.h"                                                                                       // bit for AUX control

/* Mandatory CVs */
#define	ADDRESS_CV                  0                                                                                       // identifies the CV containing the Slave Module address
#define	FIRST_CV                    897                                                                                     // identifies the first CV of the SUSI modules -> from 897 to 1023
#define	MANUFACTER_ID               13                                                                                      // identifies the constructor of the SUSI module: 13 from NMRA regulation : https://www.nmra.org/sites/default/files/appendix_a2c_s-9.2.2.pdf
#define	SUSI_VER                    13                                                                                      // identifies the protocol version SUSI: 1.3

/* Slave Module Addresses */
#define DEFAULT_SLAVE_NUMBER        1                                                                                       // identifies the SUSI Slave address: default 1
#define MAX_ADDRESS_VALUE           3                                                                                       // Maximum number of SUSI modules that can be connected to the decoder: 3


/* Acquisition Buffer */
// amount of packets in queue
#define BUFFER_SIZE 5

#define SPI_MISO PC7    // not used
#define SPI_MOSI PC6    // SUSI data
#define SPI_SCK PC5     // SUSI clock
#define SPI_CS PC4     // not used


union PacketT                                                               // one packet - do not forget, we are running on 32 bit processor, then uint32 is basic unit!
{
  struct {uint8_t cmnd; uint8_t arg1; uint8_t arg2; uint8_t used; } B;      // three bytes represent packet + slot status = 32 bits
  uint32_t W;                                                               // common name, good for example for clearing all, etc.
};

class SUSI2 {
    private:
        uint8_t	_slaveAddress;                                              // identifies the slave number on the SUSI bus (values from 1 to 3)

        PacketT MyBuffer[BUFFER_SIZE];                                      // received packet buffer
        uint8_t BufferR, BufferW;                                           // Buffer read and write position (cyclic buffer)
        uint8_t CV_Index;                                                   // in actual version CVs 900, 901, 940, 941, 980, 981 are mandatory indexed
        uint8_t LowBinary;                                           // save variable for 16 bit functions, that coming in two packets
        uint8_t WaitHighBinary;                                      // indicate what packet is expected next

    private:
        /*
        *   initClass() Initialize the Class and set the pins to which the Rcn600 Bus is connected to 'INPUT'
        *   Input:
        *       - None
        *   Returns:
        *       - None
        */
        void initClass(void);
        /*
        *   IsValidCV(CV_Value) Check, if requested CV number is valid for selected slave
        *   Input:
        *       - CV_Value
        *   Returns:
        *       - True/False
        */
        bool IsValidCV(uint8_t CV_Value);
        /*
        *   initSPI() Initialize SPI hardware
        *   Input:
        *       - none
        *   Returns:
        *       - none
        */
        void initSPI(void);
        /*
        *   initTimer1() Initialize Timer1 hardware
        *   Input:
        *       - none
        *   Returns:
        *       - none
        */
        void initTimer1(void);
        /*
        *   SendACK() Send ACK pulse
        *   Input:
        *       - none
        *   Returns:
        *       - none
        */
        void SendACK(void);

    public:
        /*
        *   SUSI2() Class Constructor
        *   Input:
        *       - none
        *   Returns:
        *       - None
        */
        SUSI2();
        /*
        *   SUSI2() Class Constructor (for compatibility only)
        *   Input:
        *       - the pin to which the 'Clock' line is connected - ignored
        *       - the pin to which the 'Data' line is connected - ignored
        *   Returns:
        *       - None
        */
        SUSI2(uint8_t CLK_pin, uint8_t DATA_pin);
        /*
        *   ~SUSI2() Class Destructor
        *   Input:
        *       - None
        *   Returns:
        *       - None
        */
        ~SUSI2(void);
        /*
        *   init() Initialize the library, using the notifySusiCVread methods
        *   Input:
        *       - None
        *   Returns:
        *       - None
        */
        void init(void);
        /*
        *   init() Initialize the library by passing the Slave address
        *   Input:
        *       - Slave address: 1, 2, 3
        *   Returns:
        *       - None
        */
        void init(uint8_t SlaveAddress);
        /*
        *   process() It should be invoked as much as possible: decoding the raw messages acquired
        *   Input:
        *       - None
        *   Returns:
        *       - -1	INVALID MESSAGE in queue appeared (all valid decoded)
        *       -  0	No Messages in Decoding Queue
*       -  1	VALID MESSAGEs decoded from queue (all available)
        */
        int8_t process(void);
        /*
        *   AddToQueue() It must public for visibility. Is used by interrupt handler to add data to object
        *   Input:
        *       - Object to add to queue
        *   Returns:
        *       - None
        */
        void AddToQueue(PacketT ReceivedData);

};



/* RCN-602 / S-9.4.3 - CV mapping:
CV-Name                     |  CV#  |  CV#  |  CV#  | Comment
                            |Module1|Module2|Module3|
----------------------------+-------+-------+--------+----------
SUSI Module #               |          897           | Fixed meaning
----------------------------+-------+-------+--------+----------
SUSI CV-Banking	            |          898           | volatile, outdated variant
----------------------------+-------+-------+--------+----------
Manufacturer-specific       |          899           | has been already used
----------------------------+-------+-------+--------+----------
Manufacturer identification | 900.0 | 940.0 | 980.0  | only in Bank 0; fixed
----------------------------+-------+-------+--------+----------
Hardware identification     | 900.1 | 940.1 | 980.1  | only in Bank 1; fixed
----------------------------+-------+-------+--------+----------
Manufacturer ID 2           |       |       |        | only in bank 254; reserved for
or Alternative              |900.254|940.254|980.254 | alternative manufacturer identification
Manufacturer ID             |       |       |        | or extended NMRA manufacturer
                            |       |       |        | identification
----------------------------+-------+-------+--------+----------
Version number              | 901.0 | 941.0 | 981.0  | only in Bank 0; fixed
----------------------------+-------+-------+--------+----------
Subversion number           | 901.1 | 941.1 | 981.1  | only in Bank 1; fixed
----------------------------+-------+-------+--------+----------
SUSI version                |901.254|941.254|981.254 | only in Bank 254;
                            |       |       |        | supported SUSI version
----------------------------+-------+-------+--------+----------
Manufacturer specific       |902-939|942-979|982-1019|
----------------------------+-------+-------+--------+----------
Status bits                 |                        | Bits 0-3 = WAIT, SLOW
                            |         1020           | HOLD & STOP
                            |                        | Bits 4-7 reserved.
----------------------------+-------+-------+--------+----------
SUSI CV Banking             |         1021           | Non-volatile,
                            |                        | recommended variant
----------------------------+-------+-------+--------+----------
reserved                    |         1022           |
----------------------------+-------+-------+--------+----------
reserved                    |         1023           |
----------------------------+-------+-------+--------+----------
reserved                    |         1024           |
----------------------------+-------+-------+--------+----------
*/

#if defined (__cplusplus)
extern "C" {                                                                                                                // External functions, implementable at the user's discretion
#endif
        /*
        *   notifySusiRawMessage() It is invoked whenever there is a message (2 Bytes) to be decode. It is NOT invoked for CVs Manipulation Messages. It displays the Raw message: NOT DECODED.
        *   Input:
        *       - The First Byte of the Message (Command)
        *       - The Second Byte of the Message (Argument)
        *   Returns:
        *       - None
        */
        extern	void notifySusiRawMessage(uint8_t firstByte, uint8_t secondByte) __attribute__((weak));
        /*
        *   notifySusiRawMessage3b() It is invoked whenever CVs Manipulation Messages (3 Bytes) to be decode. It displays the Raw message: NOT DECODED.
        *   Input:
        *       - The First Byte of the Message (Command)
        *       - The Second Byte of the Message (Argument)
        *   Returns:
        *       - None
        */
        extern	void notifySusiRawMessage3b(uint8_t firstByte, uint8_t secondByte, uint8_t thirdByte) __attribute__((weak));
        /*
        *   notifySusiFunc() It is invoked when: data is received from the Master on a group of digital functions
        *   Input:
        *       - the decoded Functions group
        *       - the status of the function group
        *   Returns:
        *       - None
        */
        extern	void notifySusiFunc(SUSI_FN_GROUP SUSI_FuncGrp, uint8_t SUSI_FuncState) __attribute__((weak));
        /*
        *   notifySusiBinaryState() it is invoked when: data is received from the Master on the status of a specific function
        *   Input:
        *       - the function number (from 1 to 127)
        *       - the state of the Function (active = 1, inactive = 0)
        *   Returns:
        *       - None
        */
        extern  void notifySusiBinaryState(uint8_t Command, uint8_t CommandState) __attribute__((weak));
        /*
        *   notifySusiBinaryStateL() it is invoked when: data is received from the Master on the status of a specific function - long version 
        *   Input:
        *       - the function number (from 0 to 32767; 0=broadcast)
        *       - the state of the Function (active = 1, inactive = 0)
        *   Returns:
        *       - None
        */
        extern  void notifySusiBinaryStateL(uint16_t Command, uint8_t CommandState) __attribute__((weak));
        /*
        *   notifySusiAux() it is invoked when: data is received from the Master on the status of a specific AUX
        *   Input:
        *       - the AUX number
        *       - the output state (active = 1, inactive = 0)
        *   Returns:
        *       - None
        */
        extern  void notifySusiAux(SUSI_AUX_GROUP SUSI_auxGrp, uint8_t SUSI_AuxState) __attribute__((weak));
        /*
        *   notifySusiTriggerPulse() it is invoked when: the Trigger (or pulsation) command for any steam puffs is received from the Master
        *   Input:
        *       - Trigger/Pulse command status (1 = trigger active, rest is reserved)
        *   Returns:
        *       - None
        */
        extern  void notifySusiTriggerPulse(uint8_t state) __attribute__((weak));
        /*
        *   notifySusiMotorCurrent() it is invoked when: data on the current absorption by the motor is received from the Master
        *   Input:
        *       - Current Draw: -128 to +127 (already converted from original 2's Complement)
        *   Returns:
        *       - None
        */
        extern  void notifySusiMotorCurrent(int8_t current) __attribute__((weak));
        /*
        *   notifySusiRequestSpeed() it is invoked when: the data on Speed and Direction requested by the Control Unit are received from the Master
        *   Input:
        *       - the speed (128 steps) required
        *       - the direction requested
        *   Returns:
        *       - None
        */
        extern  void notifySusiRequestSpeed(uint8_t Speed, SUSI_DIRECTION Dir) __attribute__ ((weak));
        /*
        *   notifySusiDCCSpeed() it is invoked when: the data on Speed and Direction requested by the Control Unit are received from the Master, This value is only normalized from 14 or 28 speed steps to 127 speed steps if necessary.
        *   Input:
        *       - the speed (128 steps) required
        *       - the direction requested
        *   Returns:
        *       - None
        */
        extern  void notifySusiDCCSpeed(uint8_t Speed, SUSI_DIRECTION Dir) __attribute__ ((weak));
        /*
        *   notifySusiRealSpeed() It is invoked when: data on the real Speed and Direction are received from the Master
        *   Input:
        *       - the real speed (128 steps)
        *       - the real direction
        *   Returns:
        *       - None
        */
        extern  void notifySusiRealSpeed(uint8_t Speed, SUSI_DIRECTION Dir) __attribute__ ((weak));
        /*
        *   notifySusiMotorLoad() it is invoked when: data on the Engine load is received from the Master
        *   Input:
        *       - Engine Load: -128 to +127 (already converted from original 2's Complement)
        *   Returns:
        *       - None
        */
        extern	void notifySusiMotorLoad(int8_t load) __attribute__((weak));
        /*
        *   notifySusiAnalogFunction() It is invoked when: receiving data from the Master on a group of analog functions
        *   Input:
        *       - the decoded Analog group
        *       - the state of the group
        *   Returns:
        *       - None
        */
        extern  void notifySusiAnalogFunction(SUSI_AN_GROUP SUSI_AnalogGrp, uint8_t SUSI_AnalogState) __attribute__((weak));
        /*
        *   notifySusiAnalogDirectCommand() It is invoked when: data is received from the Master direct commands for analog operation
        *   Input:
        *       - the analog function number: 1 or 8
        *       - the analog value
        *   Returns:
        *       - None
        */
        extern  void notifySusiAnalogDirectCommand(uint8_t aunctionNumber, uint8_t Value) __attribute__((weak));
        /*
        *   notifySusiNoOperation() It is invoked when: the "no operation" command is received, it is mainly used for testing purposes
        *   Input:
        *       - the command argument (can be anything)
        *   Returns:
        *       - None
        */
        extern  void notifySusiNoOperation(uint8_t commandArgument) __attribute__((weak));
        /*
        *   notifySusiMasterAddress() it is invoked when: the digital address of the Master is received
        *   Input:
        *       - the Master's Digital Address
        *   Returns:
        *       - None
        */
        extern	void notifySusiMasterAddress(uint16_t MasterAddress) __attribute__((weak));
        /*
        *   notifySusiControlModule() It is invoked when: the command on the module control is received
        *   Input:
        *       - bytes containing the form control
        *   Returns:
        *       - None
        */
        extern	void notifySusiControllModule(uint8_t ModuleControll) __attribute__((weak));
        /*
        *   notifySusiUnknownMessage() It is invoked whenever there is a unknownn message (2 Bytes) to be decode.
        *   Input:
        *       - The First Byte of the Message (Command)
        *       - The Second Byte of the Message (Argument)
        *   Returns:
        *       - None
        */
        extern	void notifySusiUnknownMessage(uint8_t firstByte, uint8_t secondByte) __attribute__((weak));

    

        /* CV MANIPULATION METHODS */

        /*
        *   notifySusiCVRead() It is invoked when: reading a CV is requested
        *   Input:
        *       - the CV number to read - relative to base value 897! (0=CV897, 1=CV898, ... 127=CV1024)
        *       - the CV index - in this version at minimum CVs 900 (3), 901 (4), 940 (43), 941 (44), 980 (83), 981 (84) are mandatory indexed
        *   Returns:
        *       - returns the value of the read CV
        */
        extern uint8_t notifySusiCVRead(uint8_t CV, uint8_t CVindex) __attribute__((weak));
        /*
        *   notifySusiCVWrite() it is invoked when: writing a CV is required.
        *   Input:
        *       - the number of the requested CV
        *       - the CV index - in this version at minimum CVs 900 (3), 901 (4), 940 (43), 941 (44), 980 (83), 981 (84) are mandatory indexed
        *       - the New Value of CV
        *   Returns:
        *       - the value read (post write) at the requested position
        */
        extern uint8_t notifySusiCVWrite(uint8_t CV, uint8_t CVindex, uint8_t Value) __attribute__((weak));
        /* RESET CVs, the same method as the NmraDcc Library is used:
        * 
        *  notifyCVResetFactoryDefault() Called when CVs must be reset. This is called when CVs must be reset to their factory defaults.
        *   Inputs:
        *       - value used to write to CV8 (usually write CV8=8 means reset all CVs to factory defaults)
        *   Returns:
        *       - None
        */
        extern void notifyCVResetFactoryDefault(uint8_t Value) __attribute__((weak));
#if defined (__cplusplus)
}
#endif

#endif
