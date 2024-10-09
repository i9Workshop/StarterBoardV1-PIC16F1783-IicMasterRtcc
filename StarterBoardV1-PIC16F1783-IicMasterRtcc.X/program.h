#ifndef program_H
#define	program_H

#include <xc.h>

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#ifdef	__cplusplus
extern "C" {
#endif
    
    
#define _User_FOSC      32000000 // CPU Clock Frequency
    
#define _Address_RTCC   0x68
    
#define led1        LATBbits.LATB7
#define led2        LATBbits.LATB6
    
#define RS_Pin      LATBbits.LATB5
#define E_Pin       LATBbits.LATB4
#define D4_Pin      LATBbits.LATB3
#define D5_Pin      LATBbits.LATB2
#define D6_Pin      LATBbits.LATB1
#define D7_Pin      LATCbits.LATC5
    
#define pb_Up       PORTAbits.RA1
#define pb_Down     PORTAbits.RA2
#define pb_Left     PORTAbits.RA3
#define pb_Right    PORTAbits.RA4
    
#define res_Sw      PORTAbits.RA5


    void delay_x1o5us(uint8_t delay);
    void delay_x24o25us(uint16_t delay);
    void delay_ms(uint32_t delay);
    
    void programInitialize(void);
    void programLoop(void);
    
    void pb_DelayDebounce(void);
    
    uint8_t rtcc_ConvertBcdToDec(uint8_t bcd);
    uint8_t rtcc_ConvertDecToBcd(uint8_t dec);
    void rtcc_WriteRegister(uint8_t day, uint8_t date, 
        uint8_t month, uint8_t century, uint8_t year, 
        uint8_t hour, uint8_t minute, uint8_t second);
    void rtcc_LcdDisplay(uint8_t day, uint8_t date,
        uint8_t month, uint8_t century, uint8_t year,
        uint8_t hour, uint8_t minute, uint8_t second);
    
    void lcd_DelaySetupTime(void);
    void lcd_DelayPulse(void);
    void lcd_EPulse(void);
    void lcd_WriteData(uint8_t data);
    
    void lcd_PrintCmd(uint8_t command);
    void lcd_Initialize(void);
    void lcd_ClearAll(void);
    void lcd_Goto(uint8_t y, uint8_t x);
    void lcd_PrintChar(char character);
    void lcd_PrintString(char *string);
    void lcd_PrintInt32(int32_t number);
    void lcd_PrintDigitInt32(int32_t number, uint8_t noDigit, bool enSign, bool enZero);
    
    void i2c_Initialize(uint32_t fosc, uint16_t baudrate);
    void i2c_BusIdle(void);
    void i2c_MasterStart(void);
    void i2c_MasterRstart(void);
    void i2c_MasterStop(void);
    void i2c_MasterWrite(uint8_t byte);
    uint8_t i2c_MasterRead(uint8_t ack);
    
    void i2c_MasterByteWriteSlave(uint8_t addrDev, uint8_t addrReg, uint8_t data);
    uint8_t i2c_MasterByteReadSlave(uint8_t addrDev, uint8_t addrReg);
    

#ifdef	__cplusplus
}
#endif

#endif	/* program_H */
