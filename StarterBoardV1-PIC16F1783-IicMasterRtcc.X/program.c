#include "program.h"


// Delay x1.5us
void delay_x1o5us(uint8_t delay) {
    for(uint8_t i=0; i<delay; i++) NOP();
}

// Delay x24.25us
void delay_x24o25us(uint16_t delay) {
    for(uint16_t i=0; i<delay; i++) delay_x1o5us(15);
}

// Delay x1ms
void delay_ms(uint32_t delay) {
    for(uint32_t i=0; i<delay; i++) delay_x24o25us(41);
}


void programInitialize(void) {
    TRISBbits.TRISB6 = 0;
    TRISBbits.TRISB7 = 0;
    
    led1 = 0;
    led2 = 0;
    
    TRISBbits.TRISB5 = 0;
    TRISBbits.TRISB4 = 0;
    TRISBbits.TRISB3 = 0;
    TRISBbits.TRISB2 = 0;
    TRISBbits.TRISB1 = 0;
    TRISCbits.TRISC5 = 0;
    
    ANSELBbits.ANSB5 = 0;
    ANSELBbits.ANSB4 = 0;
    ANSELBbits.ANSB3 = 0;
    ANSELBbits.ANSB2 = 0;
    ANSELBbits.ANSB1 = 0;
    
    RS_Pin = 0;
    E_Pin = 0;
    D4_Pin = 0;
    D5_Pin = 0;
    D6_Pin = 0;
    D7_Pin = 0;
    
    TRISAbits.TRISA1 = 1;
    TRISAbits.TRISA2 = 1;
    TRISAbits.TRISA3 = 1;
    TRISAbits.TRISA4 = 1;
    
    ANSELAbits.ANSA1 = 0;
    ANSELAbits.ANSA2 = 0;
    ANSELAbits.ANSA3 = 0;
    ANSELAbits.ANSA4 = 0;
    
    TRISAbits.TRISA5 = 1;
    ANSELAbits.ANSA5 = 0;
    
    // SCl pin
    TRISCbits.TRISC3 = 1;
    APFCONbits.SCKSEL = 0;
    
    // SDa pin
    TRISCbits.TRISC4 = 1;
    APFCONbits.SDISEL = 0;
    
    lcd_Initialize();
    
    rtcc_LcdDisplay(0, 0, 0, 0, 0, 0, 0, 0);
    
    i2c_Initialize(_User_FOSC, 1200); // Set bit rate x9bit/baud = 10.8kHz
}

void programLoop(void) {
    uint8_t bcdSecond = 0; // Variable for current second
    uint8_t bcdSecondPrev = 0; // Variable for previous second
    uint8_t decSecond = 0;
    
    uint8_t bcdMinute = 0;
    uint8_t bcdHour = 0;
    uint8_t bcdDate = 0;
    uint8_t bcdMonth = 0;
    uint8_t bcdYear = 0;
    
    uint8_t decMinute = 0;
    uint8_t decHour = 0;
    uint8_t decDate = 0;
    uint8_t decMonth = 0;
    uint8_t decYear = 0;
    uint8_t decCentury = 0;
    
    uint8_t day = 0;
    
    // rtcc_WriteRegister(2, 7, 10, 0, 24, 14, 20, 0); // Use this function to set RTCC module
    
    while(1) {
        bcdSecond = i2c_MasterByteReadSlave(_Address_RTCC, 0); // Read RTCC second
        
        if(bcdSecond != bcdSecondPrev) { // Condition is true if variable bcdSec not same as bcdSecPrev
            // Read all RTCC data, refer to table timekeeping registers in RTCC datasheet page 11
            
            bcdMinute = i2c_MasterByteReadSlave(_Address_RTCC, 1);
            bcdHour = i2c_MasterByteReadSlave(_Address_RTCC, 2);
            day = i2c_MasterByteReadSlave(_Address_RTCC, 3);
            bcdDate = i2c_MasterByteReadSlave(_Address_RTCC, 4);
            bcdMonth = i2c_MasterByteReadSlave(_Address_RTCC, 5);
            bcdYear = i2c_MasterByteReadSlave(_Address_RTCC, 6);
            
            // Convert BCD to DEC number
            
            decSecond = rtcc_ConvertBcdToDec(bcdSecond);
            decMinute = rtcc_ConvertBcdToDec(bcdMinute);
            decHour = rtcc_ConvertBcdToDec(bcdHour);
            decDate = rtcc_ConvertBcdToDec(bcdDate);
            decMonth = rtcc_ConvertBcdToDec(bcdMonth & 0x1F); // Register address 05h, read 4th bit until 0th bit
            decYear = rtcc_ConvertBcdToDec(bcdYear);
            decCentury = bcdMonth >> 7; // Register address 05h, read 7th bit only
            
            rtcc_LcdDisplay(day , decDate, decMonth, 20 + decCentury, decYear,
                decHour, decMinute, decSecond);
        }
        
        bcdSecondPrev = bcdSecond;
        
        delay_ms(200); // RTCC refresh rate
    }
}


// Delay to debounce mechanical noise
void pb_DelayDebounce(void) {
    delay_ms(200);
}

uint8_t rtcc_ConvertBcdToDec(uint8_t bcd) {
    return ((bcd >> 4) & 0x0F) * 10 + (bcd & 0x0F);
}

uint8_t rtcc_ConvertDecToBcd(uint8_t dec) {
    uint8_t msb = (uint8_t)(dec/10);
    uint8_t lsb = dec - (msb * 10);
    
    return (uint8_t)((msb << 4) | lsb);
}

void rtcc_WriteRegister(uint8_t day, uint8_t date,
        uint8_t month, uint8_t century, uint8_t year,
        uint8_t hour, uint8_t minute, uint8_t second) { // Write data to RTCC register
    // Write all RTCC data, refer to table timekeeping registers in RTCC datasheet page 11
    i2c_MasterByteWriteSlave(_Address_RTCC, 3, day);
    i2c_MasterByteWriteSlave(_Address_RTCC, 6, rtcc_ConvertDecToBcd(year));
    i2c_MasterByteWriteSlave(_Address_RTCC, 5, (uint8_t)(rtcc_ConvertDecToBcd(month) | (century << 7)));
    i2c_MasterByteWriteSlave(_Address_RTCC, 4, rtcc_ConvertDecToBcd(date));
    i2c_MasterByteWriteSlave(_Address_RTCC, 2, rtcc_ConvertDecToBcd(hour));
    i2c_MasterByteWriteSlave(_Address_RTCC, 1, rtcc_ConvertDecToBcd(minute));
    i2c_MasterByteWriteSlave(_Address_RTCC, 0, 0);
}

void rtcc_LcdDisplay(uint8_t day, uint8_t date, 
        uint8_t month, uint8_t century, uint8_t year, 
        uint8_t hour, uint8_t minute, uint8_t second) {
    lcd_Goto(0, 0);
    lcd_PrintString("I2C RTCC");
    
    lcd_Goto(0, 11);
    lcd_PrintDigitInt32(date, 2, false, true);
    lcd_PrintChar('/');
    
    lcd_Goto(0, 14);
    lcd_PrintDigitInt32(month, 2, false, true);
    
    lcd_Goto(1, 12);
    lcd_PrintDigitInt32(century, 2, false, true);
    
    lcd_Goto(1, 14);
    lcd_PrintDigitInt32(year, 2, false, true);
    
    lcd_Goto(1, 0);
    lcd_PrintDigitInt32(hour, 2, false, true);
    lcd_PrintChar(':');
    
    lcd_Goto(1, 3);
    lcd_PrintDigitInt32(minute, 2, false, true);
    lcd_PrintChar(':');
    
    lcd_Goto(1, 6);
    lcd_PrintDigitInt32(second, 2, false, true);
    
    lcd_Goto(1, 9);
    switch(day) {
        case 1:
            lcd_PrintString("Su");
            break;
            
        case 2:
            lcd_PrintString("Mo");
            break;
            
        case 3:
            lcd_PrintString("Tu");
            break;
            
        case 4:
            lcd_PrintString("We");
            break;
            
        case 5:
            lcd_PrintString("Th");
            break;
            
        case 6:
            lcd_PrintString("Fr");
            break;
            
        case 7:
            lcd_PrintString("Sa");
            break;
            
        default:
            lcd_PrintString("Dd");
            break;
    }
}

void lcd_DelaySetupTime(void) {
    // China TGK LCD delay
    delay_x1o5us(200);
}

void lcd_DelayPulse(void) {
    // China TGK LCD delay
    delay_x1o5us(200);
}

void lcd_EPulse(void) {
    E_Pin = 1;
    lcd_DelayPulse();
    
    E_Pin = 0;
    lcd_DelayPulse();
}

void lcd_WriteData(uint8_t data) {
    // Send upper nibble data
    D7_Pin = (data >> 7) & 0x01;
    D6_Pin = (data >> 6) & 0x01;
    D5_Pin = (data >> 5) & 0x01;
    D4_Pin = (data >> 4) & 0x01;
    
    lcd_EPulse();
    
    // Send lower nibble data
    D7_Pin = (data >> 3) & 0x01;
    D6_Pin = (data >> 2) & 0x01;
    D5_Pin = (data >> 1) & 0x01;
    D4_Pin = data & 0x01;
    
    lcd_EPulse();
    
    delay_x1o5us(30);   // Execution time for instruction >37us - Page 24
                        // delay = 37us / 1.5us = 25
}

void lcd_PrintCmd(uint8_t command) {
    RS_Pin = 0;
    lcd_DelaySetupTime();
    
    lcd_WriteData(command);
}

void lcd_Initialize(void) {
    delay_ms(20);           // Wait for LCD power supply rise time >10ms - Datasheet page 50
    
    // China TGK LCD reset process
    lcd_PrintCmd(0x33);
    lcd_PrintCmd(0x32);
    
    // LCD command - Datasheet page 24
    lcd_PrintCmd(0x28);     // Set LCD to 4-bit mode
    lcd_PrintCmd(0x02);     // Set DDRAM address counter to 0
    lcd_PrintCmd(0x0C);     // Display is set ON, cursor is set OFF, cursor blink is set OFF
    lcd_PrintCmd(0x06);     // Cursor is set to shift right
    lcd_PrintCmd(0x80);     // Set cursor back to home
    lcd_PrintCmd(0x01);     // Clear entire display
    
    delay_x24o25us(65);     // Execution time to clear display instruction, lcd_PrintCmd(0x01) >1.52ms,
                            // delay = 1.52ms / 24.25us = 63
}

void lcd_ClearAll(void) {
    lcd_PrintCmd(0x02);
    lcd_PrintCmd(0x01);
    delay_x24o25us(65);
}

void lcd_Goto(uint8_t y, uint8_t x) {
    switch(y) {
        case 0:
            lcd_PrintCmd(0x80 + x);
            break;
            
        case 1:
            lcd_PrintCmd(0xC0 + x);
            break;
            
        default:
            lcd_PrintCmd(0x80 + x);
            break;
    }
}

void lcd_PrintChar(char character) {
    RS_Pin = 1;
    lcd_DelaySetupTime();
    
    lcd_WriteData(character);
}

void lcd_PrintString(char *string) {
    while(*string!=0) {
        lcd_PrintChar(*string);
        string++;
    }
}

void lcd_PrintInt32(int32_t number) {
    uint8_t i1 = 0,
            i2 = 0,
            totalDigit = 0;
    
    char numberRevChar[11];
    char numberChar[11];
    
    memset(numberRevChar, 0, 11);
    memset(numberChar, 0, 11);
    
    if(number<0) {
        lcd_PrintChar('-');
        number = labs(number);
    }
    
    do {
        int32_t tempN = number;
        number /= 10;
        char tempC = (char)(tempN -10 * number);
        numberRevChar[i1] = tempC + 48;
        i1++;
    } while(number);
    
    totalDigit = i1;
    
    for(i1=totalDigit, i2=0; i1>0; i1--, i2++) {
        numberChar[i2] = numberRevChar[i1-1];
    }
    
    lcd_PrintString(numberChar);
}

void lcd_PrintDigitInt32(int32_t number, uint8_t noDigit, bool enSign, bool enZero) {
    uint8_t i1 = 0,
            i2 = 0,
            totalDigit = 0;
    
    char numberRevChar[11];
    char numberChar[11];
    
    memset(numberRevChar, 0, 11);
    memset(numberChar, 0, 11);
    
    if(number<0) {
        if(enSign) lcd_PrintChar('-');
        number = labs(number);
    } else {
        if(enSign) lcd_PrintChar(' ');
    }
    
    do {
        int32_t tempN = number;
        number /= 10;
        char tempC = (char)(tempN -10 * number);
        numberRevChar[i1] = tempC + 48;
        i1++;
    } while(number);
    
    totalDigit = i1;
    
    for(i1=0; i1<(noDigit-totalDigit); i1++) {
        if(enZero) lcd_PrintChar('0');
        else lcd_PrintChar(' ');
    }
    
    for(i1=totalDigit, i2=0; i1>0; i1--, i2++) {
        numberChar[i2] = numberRevChar[i1-1];
    }
    
    lcd_PrintString(numberChar);
}

void i2c_Initialize(uint32_t fosc, uint16_t baudrate) {
    SSPSTATbits.SMP = 1; // Set disabled slew rate control for standard speed mode - Page 304
    
    SSPCON1bits.SSPM = 8; // Set MSSP module for I2C master mode with clock = FOSC/(4*(SSPADD+1)) - Page 306
    
    SSPCON3bits.SDAHT = 0; // Set 100ns for minimum SDA hold time - Page 308
    
    if(baudrate>2223) baudrate = 2223; // Set maximum baud rate to 2223 - baud rate tested and worked below 2224
    else if(baudrate<1113) baudrate = 1113; // Set minimum baud rate to 1113 - baud rate tested and worked above 1112
    SSPADD = (uint8_t)((fosc/baudrate) / 4 - 1); // Set baud rate generator register - Page 309
    
    SSPCON1bits.SSPEN = 1; // Set enable MSSP module - Page 306
    
    delay_x1o5us(23); // Wait for MSSP module to be ready - Page 373 from I/O pin timing
}

void i2c_BusIdle(void) { // Wait for I2C bus to be idle
    while(SSPCON2 & 0b00011111); // Polling to hold program when one of SSPCON2 bit from 4th to 0th is 1 - Page 307
                                 // The registers are ACKEN, RCEN, PEN, RSEN or SEN.
}

void i2c_MasterStart(void) { // Initiate start condition to bus
    i2c_BusIdle();
    
    SSPCON2bits.SEN = 1; // Set to initiate start condition - Page 307
    NOP(); // Wait for start condition setup time - Page 385
    NOP(); // Tdelay = THD:STA, minimum high speed
    NOP(); // Tdelay = 600ns / 0.125us = 4.8
    NOP();
    NOP();
}

void i2c_MasterRstart(void) { // Initiate repeat start condition to bus
    i2c_BusIdle();
    
    SSPCON2bits.RSEN = 1; // Set to initiate repeat start condition - Page 307
    NOP(); // Wait for start condition setup time - Page 385
    NOP(); // Tdelay = THD:STA, minimum high speed
    NOP(); // Tdelay = 600ns / 0.125us = 4.8
    NOP();
    NOP();
}

void i2c_MasterStop(void) { // Initiate stop condition to bus
    i2c_BusIdle();
    
    SSPCON2bits.PEN = 1; // Set to initiate stop condition - Page 307
    
    delay_x1o5us(7); // Wait for stop condition setup time - Page 385 and 386
                     // Tdelay = TR + TSU:STO + TBUF
                     // delay = Tdelay / 1.5us = (1000ns + 4700ns + 4.7uf) / 1.5uf = 6.9
                     // Value in datasheet not tested note as * in page 386
    
    delay_x24o25us(60); // Additional time the bus must be free before a new transmission can start - tested
}

void i2c_MasterWrite(uint8_t byte) { // Write a byte to bus
    i2c_BusIdle();
    
    SSPBUF = byte;          // Write to MSSP register to transmit data - Page 291
    
    while(SSPSTATbits.BF);  // Polling to hold program while MSSP module transmit not complete  - Page 293
}

uint8_t i2c_MasterRead(uint8_t ack) { // Read a byte from bus
    uint8_t data = 0;        // Declare a variable to read MSSP receive register
    
    i2c_BusIdle();
    
    SSPCON2bits.RCEN = 1;    // Set enable MSSP receive mode - Page 307
    
    while(!SSPSTATbits.BF);  // Polling to hold program while MSSP module receive not complete - Page 293
    
    data = SSPBUF;           // Read MSSP receive register - Page 305
    
    SSPCON2bits.ACKEN = ack; // Set variable ack value to acknowledge sequence - Page 307
    
    return data;
}

void i2c_MasterByteWriteSlave(uint8_t addrDev, uint8_t addrReg, uint8_t data) {
    i2c_MasterStart();
    
    i2c_MasterWrite((uint8_t)(addrDev << 1)); // Device write address
    
    i2c_MasterWrite(addrReg); // RTCC register address
    
    i2c_MasterWrite(data); // Write data to RTCC register
    
    i2c_MasterStop();
}

uint8_t i2c_MasterByteReadSlave(uint8_t addrDev, uint8_t addrReg) {
    uint8_t data = 0;
    
    i2c_MasterStart();
    
    i2c_MasterWrite((uint8_t)(addrDev << 1)); // Device write address
    
    i2c_MasterWrite(addrReg); // RTCC register address
    
    i2c_MasterRstart();
    
    i2c_MasterWrite((uint8_t)((addrDev << 1) | 1)); // Device read address
    
    data = i2c_MasterRead(0); // Read data from RTCC register
    
    i2c_MasterStop();
    
    return data;
}
