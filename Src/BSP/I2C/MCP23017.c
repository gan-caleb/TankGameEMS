/*****************************************************************************
 @Project	: MET2304 Lab Assignment
 @File 		: MCP23017.c
 @Details : 
 @Author	: fongfh
 @Hardware: Tiva LaunchPad
 
 --------------------------------------------------------------------------
 @Revision	:
  Ver  	Author    	Date        	Changes
 --------------------------------------------------------------------------
   1.0  fongfh      27 Oct 23  		Initial Release (FA23)
******************************************************************************/

#include <Common.h>
#include "BSP.h"
#include "i2c.h"
#include "MCP23017.h"

static PI2C_HANDLE g_pI2cHandle;

/*****************************************************************************
 Local function
*****************************************************************************/
void delay_ms( int ms );


/*****************************************************************************
 Implementation
******************************************************************************/

/* Initialise MCP23017  */
void MCP23017_Init (PI2C_HANDLE pHandle, MCP23017 *pMCP23017)
{
	uint8_t data[2] = {0,0};
	
	g_pI2cHandle = pHandle;
	
	pMCP23017->I2C_addr = MCP23017_ADDR;
	data[0] = 0x00;	// 16-bit mode (BANK = 0), Sequential Operation (SEQOP = 0)
	I2CWrite( g_pI2cHandle, pMCP23017->I2C_addr, MCP23017_IOCONA, data, 1U);  
	
	data[0] = 0x00;	// data = 0x00 (port A as output)
	data[1] = 0x00;	// data = 0x00 (port B as output)
	
	pMCP23017->Digit1 = DEFAULT_7_SEGMENT;		// init seven segment display two 2 dots
	pMCP23017->Digit2 = DEFAULT_7_SEGMENT;		// init seven segment display two 2 dots

	MCP23017_Write (pHandle, pMCP23017);	// Write value to 7-SegDisplay

	
	I2CWrite( g_pI2cHandle, MCP23017_ADDR, MCP23017_IODIRA, data, 2U);  
}

/* function to write to the MCP23017 Output Latch registers    */ 
void MCP23017_Write (PI2C_HANDLE pHandle, MCP23017 *pMCP23017 )
{
	/* Static array used for converting int to 7-Seg values */
	static uint8_t SevenSegConv[10] = {SEVEN_SEG_0, SEVEN_SEG_1, SEVEN_SEG_2, SEVEN_SEG_3, SEVEN_SEG_4, SEVEN_SEG_5, SEVEN_SEG_6, SEVEN_SEG_7, SEVEN_SEG_8, SEVEN_SEG_9};
	
	uint8_t digit1, digit2;	// Declare variable to contain the value to write for each digit
	if (pMCP23017->Digit1 == DEFAULT_7_SEGMENT || pMCP23017->Digit2 == DEFAULT_7_SEGMENT ) { digit2 = digit1 = SEVEN_SEG_DOT; } // Set both values to be 'dot' if value is set to default
	else
	{
		digit1 = SevenSegConv[pMCP23017->Digit1];		// set for digit in tens place
		digit2 = SevenSegConv[pMCP23017->Digit2];		// set for digit in onces place
	}
	uint8_t data[2] = {digit2, digit1}; // init an array to contain the 2 values for writing
	
	I2CWrite( g_pI2cHandle, pMCP23017->I2C_addr, MCP23017_OLATA, data, 2U);  // Write data  
}
