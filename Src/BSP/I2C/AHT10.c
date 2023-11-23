/*****************************************************************************
 @Project	: MET2304 Lab Assignment
 @File 		: AHT10.c
 @Details : 
 @Author	: fongfh
 @Hardware: Tiva LaunchPad
 
 --------------------------------------------------------------------------
 @Revision	:
  Ver  	Author    	Date        	Changes
 --------------------------------------------------------------------------
   1.0  fongfh      27 Oct 23 		Initial Release (FA23)
******************************************************************************/

#include <Common.h>
#include "BSP.h"
#include "IRQ.h"
#include "i2c.h"
#include "AHT10.h"

static PI2C_HANDLE g_pI2cHandle;

static uint32_t Humd;
static uint32_t Temp;
/*****************************************************************************
 Local function
*****************************************************************************/
void delay_ms( int ms );

/*****************************************************************************
 Implementation
******************************************************************************/
void AHT10_Init( PI2C_HANDLE pHandle, AHT10 *pAHT10 )
{
	uint8_t data[2];
	delay_ms (AHT10_POWER_ON_DELAY);
	
  g_pI2cHandle = pHandle;
	pAHT10->I2C_adr = AHT10_I2C_ADDRESS;
	data[0] = AHT10_INIT_CAL_ENABLE;	// 0x08
	data[1] = AHT10_DATA_NOP;					// 0x00	

	I2CWrite( g_pI2cHandle, pAHT10->I2C_adr, AHT10_INIT_CMD, data, 2U);  
}

void AHT10Trigger( PI2C_HANDLE pHandle, AHT10 *pAHT10 )
{
	uint8_t data[3];
  g_pI2cHandle = pHandle;		
	/* fill in with your own codes */
  /* Prepare data for starting AHT10 measurement */
	data[0] = AHT10_START_MEASURMENT_CMD;		// 0xAC
	data[1] = AHT10_DATA_MEASURMENT_CMD;		// 0x33
	
	/* Send command to AHT10 sensor to start measurement */	
	I2CWrite( g_pI2cHandle, pAHT10->I2C_adr, AHT10_START_MEASURMENT_CMD, data, 2U);  
}

void AHT10ReadRawdata( PI2C_HANDLE pHandle, AHT10 *pAHT10 )
{
	uint8_t data[7]; // Buffer to store raw data read from sensor
	g_pI2cHandle = pHandle; // Assign global I2C handle to the passed handle
	 
	/* fill in with your own codes */
  /* Read raw data from AHT10 sensor */
	I2CRead( g_pI2cHandle, pAHT10->I2C_adr, 6U); // Read 6 bytes of data from the AHT10 sensor
    
	/* Extract humidity and temperature data from raw I2C data */
	data[0] = g_pI2cHandle->data_rx[1];
	data[1] = g_pI2cHandle->data_rx[2];
	data[2] = g_pI2cHandle->data_rx[3];
	data[3] = g_pI2cHandle->data_rx[3];
	data[4] = g_pI2cHandle->data_rx[4];
	data[5] = g_pI2cHandle->data_rx[5];	
	
	/*  Process the raw humidity I2C data */
	Humd &= ~0xFFFFFFFF;
	Humd |= (data[1]<<16) + (data[2]<<8) + data[3];
	Humd = Humd>>4;
	/*  Process the raw temperature I2C data */
	Temp &= ~0xFFFFFFFF;
	Temp = (data[3]<<16) + (data[4]<<8) + data[5];
	Temp &= ~(0xF<<20);
   
 /* Convert raw values to float */
	float fHumd = (float)(Humd);
	float fTemp	= (float)(Temp);
	
  /* Calculate real temperature and humidity values */
	pAHT10->fTemperature = (fTemp/1048576)*200-50;
	pAHT10->fHumidity = (fHumd/1048576)*100;	
}
