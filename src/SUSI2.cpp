/*
  SUSI / RCN-600 / S-9.4.1 implementation library
  (including RCN-602 / S-9.4.3 support)
  This library is heavily inspired by https://github.com/TheFidax/Rcn600/
  At minimum trying to keep same interface.
  Main change is, that this one is developed for little more actual processor CH32V003. (with theoretical upgrade to other CH32V.. family, and some STM32.. family)
  
  Created by Jindra Fucik / https://www.fucik.name
  
  Main concept: all SUSI stream is two wire SPI communication (half duplex). This version utilize SPI hardware for receive bytes. On top, it utilize Timer1 in monostable mode. It means, that measure gap from last falling edge and if exceeds 7 ms, it reset receiver for synchronization. It means, main CPU program is not disturbed by receiving bits and bytes. 

*/

#include "SUSI2.h"                                                                                 // Header

SUSI2* pointerToSUSI;                                                         // Pointer to the SUSI Class
PacketT partial;                                                              // partially received packet - used in ISR routine
uint8_t ByteCount;                                                            // Counter of bytes in packet - used in ISR routine

/**********************************************************************************************************************/
/* Constructor and Destructor */

SUSI2::SUSI2(uint8_t CLK_pin, uint8_t DATA_pin) {                                                       // Class constructor
}                                                                                                       // This one is for compatibility only. Pin assignment is fixed for the hardware

SUSI2::SUSI2(void) {                                                                                    // Class constructor
}                                                                                                       // empty for now

SUSI2::~SUSI2(void) {                                                                                   // Class Destructor
  SPI_Cmd( SPI1, DISABLE );                                                                             // stop SPI receiver
  TIM_Cmd( TIM1, DISABLE );                                                                             // stop Timer1 functions
}

/**********************************************************************************************************************/
/* Initializing Library */

void SUSI2::initClass(void) {
  pointerToSUSI = this;                                                                             // I assign the pointer the address of the following class

  for (BufferR=0; BufferR<BUFFER_SIZE; BufferR++) {MyBuffer[BufferR].W = 0;};    // empty buffer
  ByteCount=0;          // Counter of bytes in packet
  BufferR=BufferW=0;    // Buffer read and write position
  partial.W=0;          // empty partially received
  initSPI();            // initialize SIP for receive
  initTimer1();         // initialize Timer1 for synchronization
}

void SUSI2::init(void) {
    if (notifySusiCVRead) {                                                                             // If CV storage system is present
        _slaveAddress = notifySusiCVRead(ADDRESS_CV,0);                                                   // I read the value stored in the CV of the address

        if ((_slaveAddress > MAX_ADDRESS_VALUE) || (_slaveAddress < 1)) {                               // If the address is greater than those allowed
            if (notifySusiCVWrite) {                                                                    // Check if it is possible to update the value with a correct one
                notifySusiCVWrite(ADDRESS_CV, 0, DEFAULT_SLAVE_NUMBER);                                    // I write the default address
            }
            _slaveAddress = DEFAULT_SLAVE_NUMBER;                                                       // I use the default address: 1
        }
    }	
    else {                                                                                              // If there is NO CV storage system
        _slaveAddress = DEFAULT_SLAVE_NUMBER;                                                           // I use the default address: 1
    }
    
    initClass();                                                                                        // I initialize the class and its components
}

void SUSI2::init(uint8_t SlaveAddress) {                                                                // Initialization with user-chosen address in code
    _slaveAddress = SlaveAddress;                                                                       // Except the address

    if ((_slaveAddress > MAX_ADDRESS_VALUE) || (_slaveAddress < 1)) {                                   // If the address is greater than those allowed
        _slaveAddress = DEFAULT_SLAVE_NUMBER;                                                           // I use the default address: 1
    }

    initClass();                                                                                        // I initialize the class and its components
}

/**********************************************************************************************************************/
/* Interrupts */
/* interrupts are not member of class, they are linked as static */

void SUSI2::AddToQueue(PacketT) {
  if (!MyBuffer[BufferW].B.used)                       // is buffer available?
  {
    MyBuffer[BufferW++] = partial;                     // yes, store data
    if (BufferW == BUFFER_SIZE) {BufferW = 0;}         // rotate cyrcular pointer
  }
}

// Interrupt functions must have "C" linkage!!!
#ifdef __cplusplus
extern "C" {
#endif

void SPI1_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
/*********************************************************************
 * @fn      SPI1_IRQHandler
 * @brief   This function handles SPI1 received byte.
 * @return  none
 */
void SPI1_IRQHandler(void)
{
  switch (ByteCount) {
    case 0 :
      partial.B.cmnd = SPI_I2S_ReceiveData( SPI1 );            // read data to command
      ByteCount++;
      break;
    case 1 :
      partial.B.arg1 = SPI_I2S_ReceiveData( SPI1 );            // read data to arg1
      if ( (partial.B.cmnd & 0xF0) == 0x70 )                   // is it 3 byte command? (CV manipulation)
        {ByteCount++;}                                         // yes, then read next byte
      else
        {
          ByteCount = 0;                                       // reset for next one
          partial.B.used = 1;                                  // mark as used
          pointerToSUSI->AddToQueue(partial);                  // add to my queue
          partial.W = 0;
        }
      break;
    case 2 :
      partial.B.arg2 = SPI_I2S_ReceiveData( SPI1 );            // read data to arg2 (must be programming command)
      ByteCount = 0;
      partial.B.used = 1;                                      // mark as used
      pointerToSUSI->AddToQueue(partial);                  // add to my queue
      partial.W = 0;
      break;
    default :                                 // some error???
      SPI_I2S_ReceiveData( SPI1 );            // read data to finish interrupt
      ByteCount = 0;                          // reset receiver
      break;
  }
}

#ifdef __cplusplus
}
#endif

// Interrupt functions must have "C" linkage!!!
#ifdef __cplusplus
extern "C" {
#endif

void TIM1_UP_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

/*********************************************************************
 * @fn      TIM1_UP_IRQHandler
 * @brief   This function handles TIM1 UP exception.
 * @return  none
 */
void TIM1_UP_IRQHandler(void)
{
    // Note: same IRQ handler is defined in HardwareTimer. To avoid conflict, disable: //#define TIM_MODULE_ENABLED
    //       in file C:\Users\<user>\AppData\Local\Arduino15\packages\WCH\hardware\ch32v\1.0.4\variants\CH32V00x\CH32V003F4\variant_CH32V003F4.h
    SPI1->CTLR1 |= SPI_NSSInternalSoft_Set ;        // initialize SPI receiver by pulse of SS bit (internal one)
    SPI1->CTLR1 &= SPI_NSSInternalSoft_Reset;       // bo back to active state
    TIM_ClearITPendingBit( TIM1, TIM_IT_Update );   // reset interrupt flag
    ByteCount=0;                                    // reset counter of bytes in packet
    partial.W=0;                                    // empty partially received data
}

#ifdef __cplusplus
}
#endif

/**********************************************************************************************************************/
/* Hardware inits */

void SUSI2::initSPI() {
  /*
  R16_SPI_CTLR1             // 0x40013000 SPI Control register1

              0000 0110 1100 0001 = 0x06C1
                                1: Data sampling starts from the second clock edge.
                               0: SCK is held low in idle state.
                              0: Configured as a slave device.
                          00 0: FHCLK /2;
                         1: Enable SPI.
                        1: LSB is transmitted first.
                      0: NSS is low.
                     1: Software control of the NSS pins.
                    1: Receive only, simplex mode.
                   0: Use 8-bit data length for sending and receiving.
                 0: Continue to send data from the data register.
                0: CRC calculation is disabled.
               0: Disable output, receive only.
              0: Selection of 2-line bi-directional mode.

 

R16_SPI_CTLR2             // 0x40013004 SPI Control register2

              0000 0000 0100 0000
                                0：Disable Rx buffer DMA.
                               0：Disable Tx buffer DMA.
                              0: Disable SS output in Master mode.
                           0 0 Reserved
                          0 Error interrupt disable.
                         1 RX buffer not empty interrupt enable.
                        0 Tx buffer empty interrupt disable.
              0000 0000 Reserved

 

R16_SPI_STATR              // 0x40013008 SPI Status register

              Read only - bit 0 = received byte

R16_SPI_DATAR             // 0x4001300C SPI Data register

              received data (read clears bit 0 of STATR)


SPI1->CTLR1 |= CTLR1_SPE_Set;

 */

    SPI_InitTypeDef SPI_InitStructure={0};

    RCC_APB2PeriphClockCmd(  RCC_APB2Periph_SPI1, ENABLE ); // do not forget clock

    pinMode(SPI_SCK,INPUT); // SUSI data
    pinMode(SPI_MOSI,INPUT); // SUSI clock
    //pinMode(SPI_MISO,GPIO_Mode_AF_PP);    // not used
    //pinMode(SPI_CS,INPUT);     // not used



#define SPI_FirstBit_LSB  ((uint16_t)0x0080)      // not defined in default header

// SPI parameters to fit SUSI requirements
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_RxOnly;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Slave;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_LSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init( SPI1, &SPI_InitStructure );

// Enable interrupt on interrupt controller
    NVIC_EnableIRQ(SPI1_IRQn);

// set interrupt for new packet received
    SPI_I2S_ITConfig( SPI1, SPI_I2S_IT_RXNE , ENABLE );     //RX buffer not empty interrupt enable bit. Used to generate an interrupt request when the RXNE flag is set. 

// Enable SPI
    SPI_Cmd( SPI1, ENABLE );


  //SPI_CTLR1 = 0x06C1;
}

void SUSI2::initTimer1() {     // Timer 1 in "slave" mode.

    // the trick is, that timer receive "reset" every falling edge of ETR pin, and ETR pin is shared with SUSI clock.
    // it mean, timer count 7 miliseconds from last click. After this it reset receiver.
    // It requiere good configuration of slave mode register. I did not found it in default HAL setup, then I decided to use direct hex values. Sorry

    //pinMode(SPI_SCK,INPUT); // SUSI data - already done in SPI

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE );   // enable clock for timer

//R16_TIM1_CTLR1  Control register 1
// 0000 0000 0000 0100 = 0x0004
//                   0 - Enables the counter. -> this is enabled afterwards by TIM_Cmd
//                  0 - 0: UEV is allowed. update (UEV) events are generated by any of the following events: -Counter overflow/underflow ..
//                 1 - 1: If an update interrupt is enabled, only an update interrupt is generated if the counter overflows/underflows. 
//                0 - 0: The counter does not stop when the next update event occurs.  --> that is questionable, I nave nultiple resets every 7 ms. It can be useful  can be only one.
//              0 - 0: the counter's counting mode is incremental. 
//            00 - 00: Edge-aligned mode. The counter counts up or down based on the direction bit (DIR). 
//           0 - 0: Auto Reload Value Register (ATRLR) is disabled. 
//        00 - 00: Tdts=Tck_int (no time divider 1:1)
//   00 00 - reserved
//  0 - 0: The capture value is the value of the actual counter 
// 0 - 0: Disable the indication function

// R16_TIM1_SMCFGR - Slave mode control register 
// 1000 0000 0111 0100 = 0x8074
//                 100 - 100: reset mode, where the rising edge of the trigger input (TRGI) will initialize the counter and generate a signal to update the registers. 
//                0 - reserved
//            111 - 111: External trigger input (ETRF). 
//           0 - 0: Does not function. 
//      0000 - 0000: No externally triggered filtering
//   00 - 00: Prescaler off. 
//  0 - 0: Disable external clock mode 2. 
// 1 - 1: Invert ETR, low or falling edge active; 

// Timing constant calculation:
// Timer update is generated, when repetation counter reach requested number of repetations.
// In reality prescaler and repetation counter starts with 0, it mean, we must increment them (+1) to be on usual mathematic.
// Calculation here is for 48 MHz clock only, no any clever automatic calculation.
// Calculation formula can be written like this:
// Prescaler * ATRLR * RepetitionCounter / SystemClock
// Do not forget, that prescaler is (PSC + 1) and RepetitionCounter is (RPTCR + 1)
//
// In our case: ((PSC + 1) * ATRLR * (RPTCR + 1)) / System clock
// ((1 + 1) * 56 000 * (2+1)) / 48 000 000 = (2 * 56 000 * 3) / 48 000 000 = 336 000 / 48 000 000 = 0,007 seconds = 7 ms.

    TIM1->CTLR1 = 0x0004;    // URS=1 interupt on overload...
    TIM1->SMCFGR = 0x8074;  // inverted trigger, no prescaler, no ETF, no MSM, Trigger Selection FS = external ETRF (7), SMS = reset mode (4)
    TIM1->PSC = 1;          // prescaler = 1 -> 48 MHz / (PSC+1) = 24 MHz / 56000 = 428.6 Hz / (RTPCR+1) = 142.8 Hz -> 7 ms.
    TIM1->RPTCR = 2;         // Repetition Counter (postscaler)  Updata_time = psc*arr*RepetitionCounter/system
    TIM1->ATRLR = 56000;
    TIM_Cmd( TIM1, ENABLE );

    TIM_ClearITPendingBit( TIM1, TIM_IT_Update );   // clear potential interrupt flag from the past

    NVIC_EnableIRQ(TIM1_UP_IRQn);                   // enable Timer 1 update unterrupt on controller

    TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);      // enable timer updating event in timer config

}

/**********************************************************************************************************************/
/* ACK pulse as hardware */
void SUSI2::SendACK() {
  pinMode(SPI_MOSI,OUTPUT_OD);  // change pin to output, with open drain
  digitalWrite(SPI_MOSI, LOW);  // set it to low
  delay(2);                     // 1.5 will be better
  pinMode(SPI_MOSI,INPUT);      // change pin back to input
  
}


/**********************************************************************************************************************/
/* Message processor */

int8_t SUSI2::process(void) {
  int8_t ResponseStatus = 0;
  if (MyBuffer[BufferR].B.used) {ResponseStatus = 1;}        // at minimum one in queue
  while (MyBuffer[BufferR].B.used)                           // are data in buffer available?
  {
    if (WaitHighBinary==1) {                                    // pair function 0x6F must follow 0x6E
      if (MyBuffer[BufferR].B.cmnd != 0x6F) {
        WaitHighBinary=0;                                    // does not follow, cancel
      }
    }
    if (WaitHighBinary==2) {                                    // pair function 0x5F must follow 0x5E
      if (MyBuffer[BufferR].B.cmnd != 0x5F) {
        WaitHighBinary=0;                                    // does not follow, cancel
      }
    }
    if ((notifySusiRawMessage) && ((MyBuffer[BufferR].B.cmnd & 0xF0) != 0x70)) {
        notifySusiRawMessage(MyBuffer[BufferR].B.cmnd, MyBuffer[BufferR].B.arg1);
    }
    if ((notifySusiRawMessage3b) && ((MyBuffer[BufferR].B.cmnd & 0xF0) == 0x70)) {
        notifySusiRawMessage3b(MyBuffer[BufferR].B.cmnd, MyBuffer[BufferR].B.arg1, MyBuffer[BufferR].B.arg2);
    }

    switch (MyBuffer[BufferR].B.cmnd) {
      case 0x60:
        /*Function group 1 : 0110-0000 (0x60 = 96) 0 0 0 F0 - F4 F3 F2 F1*/
        if (notifySusiFunc) {
          notifySusiFunc(SUSI_FN_0_4, MyBuffer[BufferR].B.arg1);
        }
        break;
      case 0x61:
        /*Function group 2 : 0110-0001 (0x61 = 97) F12 F11 F10 F9 - F8 F7 F6 F5*/
        if (notifySusiFunc) {
          notifySusiFunc(SUSI_FN_5_12, MyBuffer[BufferR].B.arg1);
        }
        break;
      case 0x62:
        /*Function group 3 : 0110-0010 (0x62 = 98) F20 F19 F18 F17 - F16 F15 F14 F13*/
        if (notifySusiFunc) {
          notifySusiFunc(SUSI_FN_13_20, MyBuffer[BufferR].B.arg1);
        }
        break;
      case 0x63:
        /*Function group 4 : 0110-0011 (0x63 = 99) F28 F27 F26 F25 - F24 F23 F22 F21*/
        if (notifySusiFunc) {
          notifySusiFunc(SUSI_FN_21_28, MyBuffer[BufferR].B.arg1);
        }
        break;
      case 0x64:
        /*Function group 5 : 0110-0100 (0x64 = 100) F36 F35 F34 F33 - F32 F31 F30 F29*/
        if (notifySusiFunc) {
          notifySusiFunc(SUSI_FN_29_36, MyBuffer[BufferR].B.arg1);
        }
        break;
      case 0x65:
        /*Function group 6 : 0110-0101 (0x65 = 101) F44 F43 F42 F41 - F40 F39 F38 F37*/
        if (notifySusiFunc) {
          notifySusiFunc(SUSI_FN_37_44, MyBuffer[BufferR].B.arg1);
        }
        break;
      case 0x66:
        /*Function group 7 : 0110-0110 (0x66 = 102) F52 F51 F50 F49 - F48 F47 F46 F45*/
        if (notifySusiFunc) {
          notifySusiFunc(SUSI_FN_45_52, MyBuffer[BufferR].B.arg1);
        }
        break;
      case 0x67:
        /*Function group 8 : 0110-0111 (0x67 = 103) F60 F59 F58 F57 - F56 F55 F54 F53*/
        if (notifySusiFunc) {
          notifySusiFunc(SUSI_FN_53_60, MyBuffer[BufferR].B.arg1);
        }
        break;
      case 0x68:
        /*Function group 9 : 0110-1000 (0x68 = 104) F68 F67 F66 F65 - F64 F63 F62 F61*/
        if (notifySusiFunc) {
          notifySusiFunc(SUSI_FN_61_68, MyBuffer[BufferR].B.arg1);
        }
        break;
      case 0x6D:
        /*Binary states short form : 0110-1101 (0x6D = 109) D L6 L5 L4 - L3 L2 L1 L0
            D = 0 means function L switched off, D = 1 switched on
            L = function number 1 ... 127
            L = 0 (broadcast) switches all functions 1 to 127 off (D = 0) or on (D = 1)*/
        if (notifySusiBinaryState) {
            if ((MyBuffer[BufferR].B.arg1 & 0x7F) == 0) {      // L = 0 ?
                // Broadcast to all functions
                if (MyBuffer[BufferR].B.arg1 & 0x80) {	// D = 0 ?? deactivate all
                    for (int i = 1; i < 128; i++) {
                        notifySusiBinaryState(i, 0);
                    }
                }
                else {				// D = 1 activate all
                    for (int i = 1; i < 128; i++) {
                        notifySusiBinaryState(i, 1);
                    }
                }
            }
            else {
                // Command for one function
                notifySusiBinaryState(MyBuffer[BufferR].B.arg1 & 0x7F, MyBuffer[BufferR].B.arg1 & 0x80);
                                       // ^^ Function number             ^^ Function state
            }
        }
        break;
      case 0x6E:  // && 0x6F
        /*Binary states long form low byte : 0110-1110 (0x6E = 110) D L6 L5 L4 - L3 L2 L1 L0
            The Binary states long form commands are always sent as a pair. This command is sent before
            the binary state long form high byte. If the two commands do not follow each other directly, they
            must be ignored.
            D = 0 means binary state L switched off, D = 1 "switched on"
            L = low-order bits of binary state number 1 ... 32767

            Binary states long form high byte : 0110-1111 (0x6F = 111) H7 H6 H5 H4 - H3 H2 H1 H0
            The Binary states long form commands are always sent as a pair. This command is sent after
            the binary state long form low byte. If the two commands do not follow each other directly, they must
            be ignored. Only this command leads to the execution of the complete command.
            H = high-order bits of the binary state number high 1 ... 32767
            H and L = 0 (broadcast) switches all 32767 available binary states off (D = 0) or on (D = 1)*/
        LowBinary = MyBuffer[BufferR].B.arg1;   // copy low byte for future
        WaitHighBinary = 1;                     // set waiting flag
        break;
      case 0x6F:  // && 0x6E
        if (WaitHighBinary == 1) {                   // only if previous one was 0x6E
          WaitHighBinary = 0;                   // remove waiting flag
          uint16_t BAddress = MyBuffer[BufferR].B.arg1;
          BAddress = BAddress << 7;
          BAddress |= (LowBinary & 0x7F);
          if (notifySusiBinaryStateL) {
            notifySusiBinaryStateL(BAddress, LowBinary & 0x80);
          }
        }
        break;
      case 0x40:
        /*Direct command 1 : 0100-0000 (0x40 = 64) X8 X7 X6 X5 - X4 X3 X2 X1
            The direct commands are used for direct control of outputs and other functions after
            interpreting the function (mapping) table in the Host. A bit = 1 means the corresponding output is
            switched on.*/
        if (notifySusiAux) {
          notifySusiFunc(SUSI_AUX_1_8, MyBuffer[BufferR].B.arg1);
        }
        break;
      case 0x41:
        /*Direct command 2 : 0100-0001 (0x41 = 65) X16 X15 X14 X13 - X12 X11 X10 X9 */
        if (notifySusiAux) {
          notifySusiFunc(SUSI_AUX_9_16, MyBuffer[BufferR].B.arg1);
        }
        break;
      case 0x42:
        /*Direct command 3 : 0100-0010 (0x42 = 66) X24 X23 X22 X21 – X20 X19 X18 X17 */
        if (notifySusiAux) {
          notifySusiFunc(SUSI_AUX_17_24, MyBuffer[BufferR].B.arg1);
        }
        break;
      case 0x43:
        /*Direct command 4 : 0100-0011 (0x43 = 67) X32 X31 X30 X29 - X28 X27 X26 X25 */
        if (notifySusiAux) {
          notifySusiFunc(SUSI_AUX_25_32, MyBuffer[BufferR].B.arg1);
        }
        break;
      case 0x21:
        /*Trigger pulse : 0010-0001 (0x21 = 33) 0 0 0 0 - 0 0 0 1 
          The command is used for synchronization of a steam impulse. It is sent once per steam pulse. 
          Bits 1 to 7 are reserved for future applications.*/
        if (notifySusiTriggerPulse) {
          notifySusiTriggerPulse(MyBuffer[BufferR].B.arg1);
        }
        break;
      case 0x23:
        /*Current : 0010-0011 (0x23 = 35) S7 S6 S5 S4 - S3 S2 S1 S0
          Current consumed by the motor. The value has a range from -128 to 127, is transmitted in 2's 
          complement and is calibrated by a manufacturer specific CV in the locomotive decoder. Negative 
          values mean regeneration as it is possible with modern electric locomotives. */
        if (notifySusiMotorCurrent) {
          notifySusiMotorCurrent(static_cast<int8_t>(MyBuffer[BufferR].B.arg1));
        }
        break;
      case 0x24:
      case 0x50:
        /*Locomotive actual speed step : 0010-0100 (0x24 = 36) R G6 G5 G4 - G3 G2 G1 G0
          The speed step and direction correspond to the real state of the motor. The transmitted G value 
          is the Vmax of the model normalized to 0…127. G = 0 means the locomotive is stationary, G = 1 ... 
          127 is the normalized speed, R = direction of travel with R = 0 for reverse and R = 1 for forward. 
          This and the following command are not recommended for new implementations. SUSI-Modules 
          should evaluate commands 0x50 to 0x52 if possible. Hosts that use deviating and/or different 
          implementations for commands 0x24 and 0x25 for compatibility with existing products are compliant 
          with the standard */
        if (notifySusiRealSpeed) {
          if (MyBuffer[BufferR].B.arg1 & 0x80) {
            notifySusiRealSpeed(MyBuffer[BufferR].B.arg1 & 0x7F,SUSI_DIR_FWD);
          }
          else {
            notifySusiRealSpeed(MyBuffer[BufferR].B.arg1,SUSI_DIR_REV);
          }
        }
        break;
      case 0x25:
      case 0x51:
        /*Locomotive target speed step : 0010-0101 (0x25 = 37) R G6 G5 G4 - G3 G2 G1 G0
          Received speed level of the "Host" normalized to 127 speed levels. G = 0 means locomotive 
          should stop, G = 1 ... 127 is the normalized speed R = direction of travel with R = 0 for reverse and 
          R = 1 for forward.*/
        if (notifySusiRequestSpeed) {
          if (MyBuffer[BufferR].B.arg1 & 0x80) {
            notifySusiRequestSpeed(MyBuffer[BufferR].B.arg1 & 0x7F,SUSI_DIR_FWD);
          }
          else {
            notifySusiRequestSpeed(MyBuffer[BufferR].B.arg1,SUSI_DIR_REV);
          }
        }
        break;
      case 0x26:
        /*Load control : 0010-0110 (0x26 = 38) P7 P6 P5 P4 - P3 P2 P1 P0
          The load state can be detected by motor voltage, current or power. 0 = no load, 127 = 
          maximum load. Negative values are also possible, which are transmitted in 2's complement. This 
          mean less load than driving on flat surface. */
        if (notifySusiMotorLoad) {
          notifySusiMotorLoad(static_cast<int8_t>(MyBuffer[BufferR].B.arg1));
        }
        break;
      //case 0x50  see upwards with 0x24
      //case 0x51  see upwards with 0x25
      case 0x52:
        /*DCC speed step : 0101-0010 (0x52 = 82) R G6 G5 G4 - G3 G2 G1 G0
          This value is only normalized from 14 or 28 speed steps to 127 speed steps if necessary. There 
          is no adjustment by any CVs.*/
        if (notifySusiDCCSpeed) {
          if (MyBuffer[BufferR].B.arg1 & 0x80) {
            notifySusiDCCSpeed(MyBuffer[BufferR].B.arg1 & 0x7F,SUSI_DIR_FWD);
          }
          else {
            notifySusiDCCSpeed(MyBuffer[BufferR].B.arg1,SUSI_DIR_REV);
          }
        }
        break;
      case 0x28:
        /*Analog function group 1 : 0010-1xxx (0x28 = 40 to 0x2F = 47) A7 A6 A5 A4 - A3 A2 A1 A0
          The eight commands of this group allow the transmission of eight different analog values in 
          digital mode.*/
        if (notifySusiAnalogFunction) {
          notifySusiAnalogFunction(SUSI_AN_FN_1,MyBuffer[BufferR].B.arg1);
        }
        break;
      case 0x29:
        /*Analog function group 1 : 0010-1xxx (0x28 = 40 to 0x2F = 47) A7 A6 A5 A4 - A3 A2 A1 A0
          The eight commands of this group allow the transmission of eight different analog values in 
          digital mode.*/
        if (notifySusiAnalogFunction) {
          notifySusiAnalogFunction(SUSI_AN_FN_2,MyBuffer[BufferR].B.arg1);
        }
        break;
      case 0x2A:
        /*Analog function group 1 : 0010-1xxx (0x28 = 40 to 0x2F = 47) A7 A6 A5 A4 - A3 A2 A1 A0
          The eight commands of this group allow the transmission of eight different analog values in 
          digital mode.*/
        if (notifySusiAnalogFunction) {
          notifySusiAnalogFunction(SUSI_AN_FN_3,MyBuffer[BufferR].B.arg1);
        }
        break;
      case 0x2B:
        /*Analog function group 1 : 0010-1xxx (0x28 = 40 to 0x2F = 47) A7 A6 A5 A4 - A3 A2 A1 A0
          The eight commands of this group allow the transmission of eight different analog values in 
          digital mode.*/
        if (notifySusiAnalogFunction) {
          notifySusiAnalogFunction(SUSI_AN_FN_4,MyBuffer[BufferR].B.arg1);
        }
        break;
      case 0x2C:
        /*Analog function group 1 : 0010-1xxx (0x28 = 40 to 0x2F = 47) A7 A6 A5 A4 - A3 A2 A1 A0
          The eight commands of this group allow the transmission of eight different analog values in 
          digital mode.*/
        if (notifySusiAnalogFunction) {
          notifySusiAnalogFunction(SUSI_AN_FN_5,MyBuffer[BufferR].B.arg1);
        }
        break;
      case 0x2D:
        /*Analog function group 1 : 0010-1xxx (0x28 = 40 to 0x2F = 47) A7 A6 A5 A4 - A3 A2 A1 A0
          The eight commands of this group allow the transmission of eight different analog values in 
          digital mode.*/
        if (notifySusiAnalogFunction) {
          notifySusiAnalogFunction(SUSI_AN_FN_6,MyBuffer[BufferR].B.arg1);
        }
        break;
      case 0x2E:
        /*Analog function group 1 : 0010-1xxx (0x28 = 40 to 0x2F = 47) A7 A6 A5 A4 - A3 A2 A1 A0
          The eight commands of this group allow the transmission of eight different analog values in 
          digital mode.*/
        if (notifySusiAnalogFunction) {
          notifySusiAnalogFunction(SUSI_AN_FN_7,MyBuffer[BufferR].B.arg1);
        }
        break;
      case 0x2F:
        /*Analog function group 1 : 0010-1xxx (0x28 = 40 to 0x2F = 47) A7 A6 A5 A4 - A3 A2 A1 A0
          The eight commands of this group allow the transmission of eight different analog values in 
          digital mode.*/
        if (notifySusiAnalogFunction) {
          notifySusiAnalogFunction(SUSI_AN_FN_8,MyBuffer[BufferR].B.arg1);
        }
        break;
      case 0x30:
        /*Direct command 1 for analog operation : 0011-0000 (0x30 = 48) D7 D6 D5 D4 - D3 D2 D1 D0 
           Setting of basic functions in analog mode bypassing a function assignment. 
            Bit 0: Sound on/off 
            Bit 1: Up/break 
            Bit 2-6: Reserved 
            Bit 7: Reduced volume*/
        if (notifySusiAnalogDirectCommand) {
          notifySusiAnalogDirectCommand(1,MyBuffer[BufferR].B.arg1);
        }
        break;
      case 0x31:
        /*Direct command 2 for analog operation : 0011-0000 (0x31 = 49) D7 D6 D5 D4 - D3 D2 D1 D0
           Setting of basic functions in analog mode bypassing a function assignment. 
            Bit 0: Front light 
            Bit 1: Rear light 
            Bit 2: Parking light 
            Bit 3-7: Reserved*/
        if (notifySusiAnalogDirectCommand) {
          notifySusiAnalogDirectCommand(2,MyBuffer[BufferR].B.arg1);
        }
        break;
      case 0x00:
        /*No Operation : 0000-0000 (0x00 = 0) X X X X - X X X X
           The command does not cause any action in the SUSI-Module. The data can have any value. 
           The command can be used as a gap filler or for test purposes. */
        if (notifySusiNoOperation) {
          notifySusiNoOperation(MyBuffer[BufferR].B.arg1);
        }
        break;
      case 0x5E:  // && 0x5F
        /*Module address low : 0101-1110 (0x5E = 94) A7 A6 A5 A4 - A3 A2 A1 A0
            Transmits the least significant bits of the active digital address of the "Host" when it is in a 
            digital operating mode. The command is always sent in pairs before the address high byte. If the two 
            commands do not follow each other directly, they are to be ignored. */
        LowBinary = MyBuffer[BufferR].B.arg1;   // copy low byte for future
        WaitHighBinary = 2;                     // set waiting flag
        break;
      case 0x5F:  // && 0x5E
        if (WaitHighBinary == 2) {                   // only if previous one was 0x6E
          WaitHighBinary = 0;                   // remove waiting flag
          uint16_t BAddress = MyBuffer[BufferR].B.arg1;
          BAddress = BAddress << 8;
          BAddress |= LowBinary;
          if (notifySusiMasterAddress) {
            notifySusiMasterAddress(BAddress);
          }
        }
        break;
      case 0x6C:
        /*Module control byte : 0110-1100 (0x6C = 108) B7 B6 B5 B4 - B3 B2 B1 B0 
            Bit 0 = Buffer Control: 0 = Buffer off, 1 = Buffer on 
            Bit 1 = Reset function: 0 = set all functions to "Off", 1 = normal operation 
            All other bits reserved by the RailCommunity. 
            If implemented, bits 0 and 1 must be set to 1 in the SUSI-Module after a reset. */
        if (notifySusiControllModule) {
          notifySusiControllModule(MyBuffer[BufferR].B.arg1);
        }
        break;
      case 0x77:
        /*CV manipulation - check byte (3-byte): 0111-0111 (0x77 = 119)   1 V6 V5 V4 - V3 V2 V1 V0 D7 D6 D5 D4 - D3 D2 D1 D0 
            DCC command for byte check in service and operation mode
            V = CV number 897 ... 1024 (value 0 = CV 897, value 127 = CV 1024)
            D = comparison value for checking. If D corresponds to the stored CV value, the SUSI-Module 
            responds with an acknowledge.
            This and the following two commands are the 3-byte packets mentioned in section 4 according 
            to [S-9.2.1].*/
        if (IsValidCV(MyBuffer[BufferR].B.arg1)) {                      // is command valid for this slave?
		  if ((MyBuffer[BufferR].B.arg1 == 1) || (MyBuffer[BufferR].B.arg1 == 124)) {
            if (MyBuffer[BufferR].B.arg2 == CV_Index) { SendACK(); }                         // for index response is instant ...
          } else {
            if (notifySusiCVRead) {                                                                     // If there is a CV storage system
              if (notifySusiCVRead(MyBuffer[BufferR].B.arg1, CV_Index) == MyBuffer[BufferR].B.arg2) { SendACK(); }  // for others, function with CV and index must be called
            }
          }
        }
        break;
      case 0x7B:
        /*CV manipulation - bit manipulation (3-byte): 0111-1011 (0x7B = 123) 1 V6 V5 V4 - V3 V2 V1 V0 1 1 1 K - D B2 B1 B0
            DCC command bit manipulate in service and operation mode V = CV number 897 ... 1024 
            (value 0 = CV 897, value 127 = CV 1024)
            K = 0: Check bit. If D matches the bit state at bit position B of the CV, the SUSI-Module responds 
            with an acknowledge.
            K = 1: Bit Write. D is written to bit position B of the CV. The SUSI-Module confirms the writing 
            with an acknowledge*/
        if ((IsValidCV(MyBuffer[BufferR].B.arg1)) && ((MyBuffer[BufferR].B.arg2 & 0xE0) == 0xE0)) {     // is command valid for this slave?
          uint8_t BitMask = 1 << (MyBuffer[BufferR].B.arg2 & 0x07);         // prepare bit mask
          uint8_t CVValue;
          if ((MyBuffer[BufferR].B.arg1 == 1) || (MyBuffer[BufferR].B.arg1 == 124)) {
            CVValue = CV_Index;                         // for index response is instant ...
          } else {
            if (notifySusiCVRead) {                                                                     // If there is a CV storage system
              CVValue = notifySusiCVRead(MyBuffer[BufferR].B.arg1, CV_Index);  // for others, function with CV and index must be called
            }
          }
		  if (MyBuffer[BufferR].B.arg2 & 0x10) {                  // K=1 for write
            if ((MyBuffer[BufferR].B.arg1 == 1) || (MyBuffer[BufferR].B.arg1 == 124)) {
              if (MyBuffer[BufferR].B.arg2 & 0x08) {CV_Index |= BitMask;} else {CV_Index &= (!BitMask);}  // for index response is instant ...
			  SendACK();                          // confirm
            } else {
              if (notifySusiCVRead) {                                                                     // If there is a CV storage system
			    if (MyBuffer[BufferR].B.arg2 & 0x08) {CVValue |= BitMask;} else {CVValue &= (!BitMask);}
                if (notifySusiCVWrite) {
                  if (notifySusiCVWrite(MyBuffer[BufferR].B.arg1, CV_Index, CVValue) == CVValue) {   // for others, function with CV and index must be called
                    SendACK();     // confirm
                  }
                }
              }
            }
		  } else {                                                  // K=0 for read
            if ((MyBuffer[BufferR].B.arg1 == 1) || (MyBuffer[BufferR].B.arg1 == 124)) {
              CV_Index &= BitMask;                          // for index response is instant ...
              if ((CV_Index) == (MyBuffer[BufferR].B.arg2 & 0x08)) {SendACK();}                          // if they are same, confirm
            } else {
              if (notifySusiCVRead) {                                                                     // If there is a CV storage system
                CVValue &= BitMask;
                if (((CVValue) && 1) == ((MyBuffer[BufferR].B.arg2 & 0x08) && 1)) {SendACK();}                          // if they are same, confirm
              }
            }
          }
        }
        break;
      case 0x7C:
        /*CV manipulation - write byte (3-byte): decoder reset by write CV8=8 -> 0x7C, 0x07, 0x08
            some decoders use different value than 8 :)*/
        if ((notifyCVResetFactoryDefault) && (MyBuffer[BufferR].B.arg1 == 0x07)) {
          notifyCVResetFactoryDefault(MyBuffer[BufferR].B.arg2);
          SendACK();     // confirm
        }
        break;
      case 0x7F:
        /*CV manipulation - write byte (3-byte): 0111-1111 (0x7F = 127)    1 V6 V5 V4 - V3 V2 V1 V0 D7 D6 D5 D4 - D3 D2 D1 D0
            DCC command byte write in service and operation mode
            V = CV number 897 ... 1024 (value 0 = CV 897, value 127 = CV 1024)
            D = value to write into the CV. The SUSI-Module confirms the writing with an acknowledge.
            The commands 0x01 to 0x0F, 0x80 to 0x8F and 0xE0 to 0xFF are defined in [RCN-601] and 
            reserved for BiDi.*/
        if (IsValidCV(MyBuffer[BufferR].B.arg1)) {                      // is command valid for this slave?
		  if ((MyBuffer[BufferR].B.arg1 == 1) || (MyBuffer[BufferR].B.arg1 == 124)) {
            CV_Index= MyBuffer[BufferR].B.arg2;
            SendACK();                           // for index response is instant ...
          } else {
            if (notifySusiCVWrite) {                                                                     // If there is a CV storage system
              if (notifySusiCVWrite(MyBuffer[BufferR].B.arg1, CV_Index,MyBuffer[BufferR].B.arg2) == MyBuffer[BufferR].B.arg2) { SendACK(); }  // for others, function with CV and index must be called
            }
          }
        }
        break;
      default:
        ResponseStatus = -1;
        if (notifySusiUnknownMessage) {                                                                     // If there is a notify about unknowns
          notifySusiUnknownMessage(MyBuffer[BufferR].B.cmnd, MyBuffer[BufferR].B.arg1);                     // notify about unknowns...
        }
        
        break;
    }
    //Serial.print(" ");
    //Serial.println(MyBuffer[BufferR].B.arg1, HEX);
    MyBuffer[BufferR].B.used = 0;
    if (++BufferR == BUFFER_SIZE) {BufferR = 0;}             // rotate cyrcular pointer
  }
  return ResponseStatus;
}

/*CV mapping:
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

/**********************************************************************************************************************/
/* CV validation */

bool SUSI2::IsValidCV(uint8_t CV_Value) {
  CV_Value ^= 0x80;                       // for standard usage upper bit must be 1, but is not practical to use.
  if (CV_Value & 0x80) {return false;}    // only values up to 127 are allowed for new implementations
  if (CV_Value < 4) {return true;}        // CV897 - CV899 are always correct, they are common for all slaves
  if (CV_Value > 122) {return true;}      // CV1020 - CV1024 are always correct, they are common for all slaves
  if ((_slaveAddress == 1) && (CV_Value>2) && (CV_Value<43)) {return true;} // CV900 - CV939 are correct for slave 1
  if ((_slaveAddress == 2) && (CV_Value>42) && (CV_Value<83)) {return true;} // CV940 - CV979 are correct for slave 2
  if ((_slaveAddress == 3) && (CV_Value>82) && (CV_Value<123)) {return true;} // CV980 - CV1019 are correct for slave 3
  return false;
}


/**********************************************************************************************************************/
/* End */
