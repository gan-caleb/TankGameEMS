/*****************************************************************************
 @Project	: 	ECE300 - Tiva Base Board
 @File : 			i2c.c
 @Details : 	TM4C123G I2C driver               
 @Author	: 	ldenissen
 @Hardware	: TM4C123G
 
 --------------------------------------------------------------------------
 @Revision	:
  Ver  	Author    	Date        	Changes
 --------------------------------------------------------------------------
   1.0  ldenissen   2019-05-22 		Initial Release
	 1.1  ldenissen   2020-08-10 		Bug Fix on callback for writing
	 1.2  ldenissen   2020-08-31 		Bug Fix on callback for reading
	 1.3  ldenissen   2020-09-12 		Workaround for Master Write Error Errata 
******************************************************************************/

#ifndef __I2C_DOC_H__
#define __I2C_DOC_H__

#include "Common.h"

/*****************************************************************************
 Define
******************************************************************************/

#define I2C_OK					0
#define I2C_ERR					-1

#define I2C_100K				0U
#define I2C_400K				1U
#define I2C_600K				2U
#define I2C_800K				3U
#define I2C_1000K				4U

#define I2C_0						0U
#define I2C_1						1U
#define I2C_2						2U
#define I2C_3						3U


/*****************************************************************************
 Type definition
******************************************************************************/

typedef struct _tagI2C_Handle
{
	void 				*pI2C;       // points to data structure for I2C0, I2C1, I2C2, I2C3
	int					Irq;         
	uint8_t			data_rx[20]; // stores data received from I2C bus
	uint32_t 		nBytes;	     // number of bytes ot transmit/receive
	uint32_t		nByteCnt;    // byte counter
	BOOL				bBusy;       
	BOOL				bRead;
	BOOL				bError;
	BOOL				bAddressNack;
}I2C_HANDLE, 	*PI2C_HANDLE;


/******************************************************************************
 Global functions
******************************************************************************/


/******************************************************************************
 @Description 	: Intialize I2C bus

 @param			: pHandle - pointer to I2C data structure 
							nI2C - 0 to 3 for I2C0 to I2C3 
							nFreq - Timer tick speed in Hz
							bIs32Bit - Set TRUE for 32 bit mode, otherwise timer will work in 16 bit mode
 @revision		: 1.0.0
 
******************************************************************************/
int I2CInit( PI2C_HANDLE pHandle, uint8_t nI2C, uint8_t nI2C_speed);

/******************************************************************************
 @Description 	: Write bytes to I2C device

 @param			: pHandle - To an intialized handle pointer
							Address - Address of the I2C device to write to
							ctrl_byte - control byte - can be register on I2C device to write to
							*data - Pointer to buffer that contains the data to be send
							nBytes - Number of bytes that should be transmitted
 @revision		: 1.0.0
 
******************************************************************************/
char I2CWrite( PI2C_HANDLE pHandle, uint8_t slave_addr, uint8_t ctrl_byte, uint8_t *data, uint32_t nbytes );

char I2CWriteByte( PI2C_HANDLE pHandle, uint8_t slave_addr, uint8_t reg_addr );

/******************************************************************************
 @Description 	: Read bytes from I2C device

 @param			: pHandle - To an intialized handle pointer
							Address - Address of the I2C device to read from
							*data - Pointer to buffer that will store the received data
							nBytes - Number of bytes requested
 @revision		: 1.0.0
******************************************************************************/
char I2CRead( PI2C_HANDLE pHandle, uint8_t slave_addr, uint32_t nbytes );

#endif /* __I2C_DOC_H__ */
