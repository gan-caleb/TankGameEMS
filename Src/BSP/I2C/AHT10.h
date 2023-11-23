/*****************************************************************************
 @Project	: MET2304 Lab Assignment
 @File 		: AHT10.h
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

#ifndef __AHT10_DOT_H__
#define __AHT10_DOT_H__

#include <Common.h>
#include "i2c.h"

/*****************************************************************************
 Type definition
******************************************************************************/

/*** Definitions for AHT10    ***/

#define AHT10_I2C_ADDRESS						0x38

#define AHT10_START_MEASURMENT_CMD	0xAC	//start measurment command
#define AHT10_NORMAL_CMD            0xA8  //normal cycle mode command
#define AHT10_SOFT_RESET_CMD        0xBA  //soft reset command

#define AHT10_INIT_CMD							0xE1	//initialization command for AHT10

// init parameters, send after initialization command, followed by a 0z00.
#define AHT10_INIT_NORMAL_MODE			0x00  //enable normal mode
#define AHT10_INIT_CYCLE_MODE				0x20  //enable cycle mode
#define AHT10_INIT_CMD_MODE					0x40  //enable command mode
#define AHT10_INIT_CAL_ENABLE				0x08  //load factory calibration coeff

#define AHT10_DATA_MEASURMENT_CMD   0x33  
#define AHT10_DATA_NOP              0x00  

// delay values are in ms
#define AHT10_MEASURMENT_DELAY     80 	//at least 75 milliseconds
#define AHT10_POWER_ON_DELAY       40 	//at least 20..40 milliseconds
#define AHT10_CMD_DELAY            350	//at least 300 milliseconds
#define AHT10_SOFT_RESET_DELAY     20		//less than 20 milliseconds

/******************************************************************************
 Data Structures & Definitions
******************************************************************************/

typedef struct tag_AHT10 {
		BOOL 		bPresent;			// flag to indicate if AHT10 is present
		uint8_t I2C_adr;		// AHT10 address
		uint8_t Status;			// stores Status bit from AHT10
		float 	fTemperature; // stores computed value for temperature
		float 	fHumidity;		// stores computed value for humidity
		uint8_t data[10];   // stores raw data read from AHT10
} AHT10; 

/******************************************************************************
 Functions
******************************************************************************/

/* Initialize AHT10 module     */
void AHT10_Init(PI2C_HANDLE pHandle, AHT10 *pAHT10);

/* Sends Trigger command to AHT10 to initiate a measurement   */
void AHT10Trigger(PI2C_HANDLE pHandle, AHT10 *pAHT10);

/* Reads raw data (6 bytes) from AHT10                                     */
/* Raw data is read from I2C bus and can be stored in array AHT10.data[]   */
void AHT10ReadRawdata(PI2C_HANDLE pHandle, AHT10 *pAHT10);

#endif /* __AHT10_DOT_H__ */
