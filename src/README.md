# Library API

* [Mandatory Methods](#Mandatory-Methods)
* [CallBack Functions](#CallBack-Functions)
* [CVs manipulation](#CVs-manipulation)
* [Class Destructor](#Class-Destructor)
* [Data Types](#Data-Types)

------------

# Mandatory Methods
The following methods are **mandatory** for correct function of the library.

------------

```c
SUSI2();
```
Default constructor.<br/>
As libryry use hardware components, plus it is mandatory to have input pins 5V tolerant, *Clock* pin must be always pin **PC5**, and *Data* pin must be always pin **PC6**.

**OR**

```c
SUSI2(CLK_pin, DATA_pin);
```
Library declaration ~~in which to insert the pins to which the SUSI Bus is connected~~.<br/>
~~The CLK pin **must* be of type ***Interrupt***, the Data pin *can* be of any type (including analog)~~.<br/>
_This is for compatibility only. Pin numbers are ignored_

**OR**

```c
SUSI2(EXTERNAL_CLOCK, DATA_pin);
```
~~Library declaration for clock acquisition using PortChangeInterrupt~~.</br>
~~The Data pin *can* be any type (including analog)~~.</br>
_This is for compatibility only. Pin numbers are ignored_

------------

```c
void init(void);
```
***OR***
```c
void init(uint8_t SlaveAddress);
```
**It is necessary** to invoke it in the 'setup' code: it starts the interrupt handling and initializes the internal counters.

The method **without the parameter** uses, *if present*, the CVs reading method to determine the module address (*saved in CV 897 (0)*), if the method *is absent* it uses the **default address: 1**.<br/>
In case your processor requiere initialization of EEPROM *(for example emulated EEPROM on CH32V003)*, EEPROM initialization must perform **before** this library initialization.

The method **with parameter** *allows you to specify the address of the module*: **CAN HAVE VALUE**: 1, 2, 3.</br>
If the value is different, the default value of 1 will be used.

------------

```c
uint8_t process(void);
```
**It is necessary to invoke it as many times as possible** in the code 'loop': it decrypts the SUSI packet.
- Input:
  - Nothing
- Returns:
  -  -1 **Invalid Message** *(one of messages in queue was not recognized, `notifySusiUnknownMessage` callback can be used for notification)*
  -  0  No Messages in Decoding Queue
  -  1  **Valid Message(s)** *(queue contain one or more valir messages)*

------------

# CallBack Functions
The following CallBack functions are **optional** (defined as 'extern' to the library), and allow the user to define the behavior to adopt in case of a particular command.</br>

------------

```c
void notifySusiFunc(SUSI_FN_GROUP SUSI_FuncGrp, uint8_t SUSI_FuncState);
```
*notifySusiFunc()* is called when: receiving data from the Master on a group of digital functions:
* Input:
  - the decoded Functions group (see [Data Types](#Data-Types))
  - the status of the function group
* Returns:
  - Nothing

------------

```c
void notifySusiBinaryState(uint8_t Command, uint8_t CommandState);
```

**AND**

```c
void notifySusiBinaryStateL(uint16_t Command, uint8_t CommandState);
```

*notifySusiBinaryState()* it is invoked when: data is received from the Master on the status of a specific state function:
- Input:
  - the state function number (short version from 1 to 127; long version from 0 to 32767; 0 means broadcast for all)
  - the state of the function (active = 1, inactive = 0)
- Returns:
  - Nothing

------------

```c
void notifySusiAux(SUSI_AUX_GROUP SUSI_auxGrp, uint8_t SUSI_AuxState);
```
*notifySusiAux()* it is invoked when: data is received from the Master on the status of a specific AUX:
- Input:
  - the AUX number
  - the output state (active = 1, inactive = 0)
- Returns:
  - Nothing

------------

```c
void notifySusiTriggerPulse(uint8_t state);
```
notifySusiTriggerPulse() The command is used for synchronization of a steam impulse. It is sent once per steam pulse.
- Input:
  - Trigger/Pulse command status (1=steam pulse, rest is reserved)
- Returns:
  - Nothing

------------

```c
void notifySusiMotorCurrent(int8_t current);
```
*notifySusiMotorCurrent()* it is invoked when: data on the current consumed by the motor is received from the Master
- Input:
  - Current Draw: -128 to +127 (already converted from original 2's Complement; negative values mean regeneration as it is possible with modern electric locomotives)
- Returns:
  - Nothing

------------

```c
void notifySusiRequestSpeed(uint8_t Speed, SUSI_DIRECTION Dir);
```
*notifySusiRequestSpeed()* it is invoked when: the data on Requested Speed ​​and Direction requested by the Control Unit are received from the Master
- Input:
  - requested speed (128 steps)
  - requested direction
- Returns:
  - Nothing

------------

```c
void notifySusiRealSpeed(uint8_t Speed, SUSI_DIRECTION Dir);
```
*notifySusiRealSpeed()* It is invoked when: data on the Actual Speed ​​and Direction are received from the Master
- Input:
  - actual speed (128 steps)
  - actual direction
- Returns:
  - Nothing

------------

```c
void notifySusiDCCSpeed(uint8_t Speed, SUSI_DIRECTION Dir);
```
*notifySusiDCCSpeed()* It is invoked when: data on the DCC Speed ​​and Direction are received from the Master (Speed as it is presented on DCC without applying speed curves)
- Input:
  - DCC speed (128 steps)
  - DCC direction
- Returns:
  - Nothing

------------

```c
void notifySusiMotorLoad(int8_t load);
```
*notifySusiAnalogFunction()* it is invoked when: data on the motor load is received from the Master
- Input:
  - Engine Load: -128 to +127
- Returns:
  - Nothing

------------

```c
void notifySusiAnalogFunction(SUSI_AN_GROUP SUSI_AnalogGrp, uint8_t SUSI_AnalogState);
```
*notifySusiAnalogFunction()* It is invoked when: receiving data from the Master on analog functions
- Input:
  - the decoded Analog function (8 functions available)
  - the value of the function
- Returns:
  - Nothing

------------

```c
void notifySusiAnalogDirectCommand(uint8_t commandNumber, uint8_t Command);
```
*notifySusiAnalogDirectCommand()* It is invoked when: data is received from the Master direct commands for analog operation (Setting of basic functions in analog mode bypassing a function assignment.)
- Input:
  - the command number: 1 or 2
  - the command bits (Command 1: Bit 0: Sound on/off; Bit 1: Up/break; Bit 7: Reduced volume / Command 2: Bit 0: Front light; Bit 1: Rear light; Bit 2: Parking light)
- Returns:
  - Nothing

------------

```c
void notifySusiMasterAddress(uint16_t MasterAddress);
```
*notifySusiMasterAddress()* it is invoked when: the digital address of the Master is received
- Input:
  - the Master's prmary Digital Address
- Returns:
  - Nothing

------------

```c
void notifySusiControllModule(uint8_t ModuleControll);
```
*notifySusiControlModule()* It is invoked when: the command on the module control is received
- Input:
  - bytes containing the form control (Bit 0 = Buffer Control: 0 = Buffer off, 1 = Buffer on; Bit 1 = Reset function: 0 = set all functions to "Off", 1 = normal operation; If implemented, bits 0 and 1 must be set to 1 in the SUSI-Module after a reset.)
- Returns:
  - Nothing

------------

```c
void notifySusiNoOperation(uint8_t commandArgument);
```
*notifySusiNoOperation()* It is invoked when: The command does not cause any action in the SUSI-Module. The data can have any value.
- Input:
  - the command argument (can be anything)
- Returns:
  - Nothing

------------

**THE FOLLOWING CALL CAN BE USED FOR DEBUG OR TO EXTRAPOLATE THE *RAW* DATA ACQUIRED BY THE LIBRARY**

```c
void notifySusiRawMessage(uint8_t firstByte, uint8_t secondByte);
```

**AND**

```c
void notifySusiRawMessage3b(uint8_t firstByte, uint8_t secondByte, uint8_t thirdByte);
```

*notifySusiRawMessage()* It is invoked whenever there is a message (2 Bytes or 3 bytes) to be decode. Standard messages are 2 bytes, CV manipulation messages are 3 bytes.
* Input:
  - The First Byte of the Message (command)
  - The Second Byte of the Message (argument)
  - The Third Byte of the Message (value)
* Returns:
  - Nothing

------------

# CVs manipulation
The following functions are **optional** (defined as 'external' to the library), but they allow the library to communicate with the Master Decoder in the event of *Read/Write CVs*.</br>
The library **handles the ACK** that allows the decoder to know the outcome of the requested operation.

------------

```c
uint8_t uint8_t notifySusiCVRead(uint8_t CV, uint8_t CVindex);
```
*notifySusiCVRead()* It is invoked when: reading a CV is requested
- Input:
  - the CV number to read (CV numbers as they are valid for Slave -> CV#897 mean value 0; CV#898 mean value 1, .. CV#1024 mean value 127)
  - the CV Index (some CVs are mandatory indexed, for example 900(3), 901(4), 940(43), etc.)
- Returns:
  - readed value of the CV

------------

```c
uint8_t uint8_t notifySusiCVWrite(uint8_t CV, uint8_t CVindex, uint8_t Value);
```
*notifySusiCVWrite()* it is invoked when: writing a CV is required.
- Input:
  - the CV number to read (CV numbers as they are valid for Slave -> CV#897 mean value 0; CV#898 mean value 1, .. CV#1024 mean value 127)
  - the CV Index (some CVs are mandatory indexed, for example 900(3), 901(4), 940(43), etc.)
  - the value of CV to be written
- Returns:
  - readed (post-write) value of the CV to be written

------------

RESET CVs, viene utilizzato la *stessa funzione* della Libreria [NmraDcc](https://github.com/mrrwa/NmraDcc):</br>
```c
void void notifyCVResetFactoryDefault(uint8_t Value);
```
*notifyCVResetFactoryDefault()* Called when CVs must be reset. This is called when CVs must be reset to their factory defaults.
- Inputs:
  - the value used for reset (majority of decoders use value 8 for reset, but other can be used as well)
- Returns:
  - None

------------

# Class Destructor
It is possible to destroy the Class if it is no longer needed.
```c
~Rcn600(void);	
```

Resources will be deallocated, pins will be kept into **INPUT** state to prevent accidental corruption.
 
------------

# Data Types
The following data types are used by the library methods/functions, they are *symbolic types* defined via "#define" and serve to improve the readability of the code, they correspond to the *uint8_t* type</br>

```c
#define	SUSI_DIRECTION		uint8_t
...
#define	SUSI_FN_GROUP		uint8_t
...
#define	SUSI_AUX_GROUP		uint8_t
...
#define	SUSI_AN_GROUP		uint8_t
```

------------

```c
SUSI_DIRECTION
```
It *symbolically* identifies the direction transmitted by the Master Decoder: </br>
- SUSI_DIR_REV : Direction *reverse*
- SUSI_DIR_FWD : Direction *forward*

------------

```c
SUSI_FN_GROUP
```
It *symbolically* identifies the group of Digital Functions transmitted by the Master Decoder:</br>
- SUSI_FN_0_4 : Functions 0 to 4
- SUSI_FN_5_12 : Functions 5 to 12
- SUSI_FN_13_20 : Functions 13 to 20
- SUSI_FN_21_28 : Functions 21 to 28
- SUSI_FN_29_36 : Functions 29 to 36
- SUSI_FN_37_44 : Functions 37 to 44
- SUSI_FN_45_52 : Functions 45 to 52
- SUSI_FN_53_60 : Functions 53 to 60
- SUSI_FN_61_68 : Functions 61 to 68

------------

```c
SUSI_AUX_GROUP
```
It *symbolically* identifies the group of AUXs transmitted by the Master Decoder:</br>
- SUSI_AUX_1_8 : AUX 1 to 8
- SUSI_AUX_9_16 : AUX 9 to 16
- SUSI_AUX_17_24 : AUX 17 to 24
- SUSI_AUX_25_32 : AUX 25 to 32

------------

```c
SUSI_AN_FN_GROUP
```
It *symbolically* identifies the group of Analogue Functions transmitted by the Master Decoder:</br>
- SUSI_AN_FN_1 : Analog function 1
- SUSI_AN_FN_2 : Analog function 2
- SUSI_AN_FN_3 : Analog function 3
- SUSI_AN_FN_4 : Analog function 4
- SUSI_AN_FN_5 : Analog function 5
- SUSI_AN_FN_6 : Analog function 6
- SUSI_AN_FN_7 : Analog function 7
- SUSI_AN_FN_8 : Analog function 8

------------
