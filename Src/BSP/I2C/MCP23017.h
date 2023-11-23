/*****************************************************************************
 @Project	: MET2304 Lab Assignment
 @File 		: MCP23017.h
 @Details : 
 @Author	: fongfh
 @Hardware: Tiva LaunchPad
 
 --------------------------------------------------------------------------
 @Revision	:
  Ver  	Author    	Date        	Changes
 --------------------------------------------------------------------------
   1.0  fongfh      27 Oct 23  		Initial Release
   1.1	MarcusTZY 	12 Nov 23			Assignment 2
******************************************************************************/

#ifndef __MCP23017_DOT_H__
#define __MCP23017_DOT_H__

#include <Common.h>
#include "i2c.h"

/*****************************************************************************
 Type definition
******************************************************************************/

/*** Definitions for MCP23017      ***/

#define MCP23017_ADDR 			0x20 // MCP23017 Address

/* MCP23017 registers  */
#define MCP23017_IODIRA 0x00   // I/O direction register A
#define MCP23017_IPOLA 0x02    // Input polarity port register A
#define MCP23017_GPINTENA 0x04 // Interrupt-on-change pins A
#define MCP23017_DEFVALA 0x06  // Default value register A
#define MCP23017_INTCONA 0x08  // Interrupt-on-change control register A
#define MCP23017_IOCONA 0x0A   // I/O expander configuration register A
#define MCP23017_GPPUA 0x0C    // GPIO pull-up resistor register A
#define MCP23017_INTFA 0x0E    // Interrupt flag register A
#define MCP23017_INTCAPA 0x10  // Interrupt captured value for port register A
#define MCP23017_GPIOA 0x12    // General purpose I/O port register A
#define MCP23017_OLATA 0x14    // Output latch register 0 A

#define MCP23017_IODIRB 0x01   // I/O direction register B
#define MCP23017_IPOLB 0x03    // Input polarity port register B
#define MCP23017_GPINTENB 0x05 // Interrupt-on-change pins B
#define MCP23017_DEFVALB 0x07  // Default value register B
#define MCP23017_INTCONB 0x09  // Interrupt-on-change control register B
#define MCP23017_IOCONB 0x0B   // I/O expander configuration register B
#define MCP23017_GPPUB 0x0D    // GPIO pull-up resistor register B
#define MCP23017_INTFB 0x0F    // Interrupt flag register B
#define MCP23017_INTCAPB 0x11  // Interrupt captured value for port register B
#define MCP23017_GPIOB 0x13    // General purpose I/O port register B
#define MCP23017_OLATB 0x15    // Output latch register 0 B

#define MCP23017_INT_ERR 255   // Interrupt error

/*** Definitions for 7-Segment displays    ***/
#define SEVEN_SEG_0					0xC0
#define SEVEN_SEG_1					0xF9
#define SEVEN_SEG_2					0xA4
#define SEVEN_SEG_3					0xB0
#define SEVEN_SEG_4					0x99
#define SEVEN_SEG_5					0x92
#define SEVEN_SEG_6					0x82
#define SEVEN_SEG_7					0xF8
#define SEVEN_SEG_8					0x80
#define SEVEN_SEG_9					0x98
#define SEVEN_SEG_DOT 			0x7F

#define DEFAULT_7_SEGMENT		100U	// Default value to write to init 7 Segment Display

/******************************************************************************
 Data Structures & Definitions
******************************************************************************/

typedef struct tag_MCP23017 {
	uint8_t 	I2C_addr; 	// MCP23017 I2c addr
	uint8_t 	Mode;    			// Count-down timer Mode: SET, COUNT_DOWN, STOP
	uint8_t 	Digit1; 		// left 7-segment digit value (0-9)
	uint8_t 	Digit2; 		// right 7-segment digit value (0-9)
} MCP23017; 

typedef enum {SET, COUNT_DOWN, STOP} Mode;

/******************************************************************************
 Functions
******************************************************************************/
/*  Initialise MCP23017     */
void MCP23017_Init (PI2C_HANDLE pHandle, MCP23017 *pMCP23017);

/* Writes commands and data to MCP23017   */
void MCP23017_Write (PI2C_HANDLE pHandle, MCP23017 *pMCP23017);

#endif /* __MCP23017_DOT_H__ */
