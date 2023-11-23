/*****************************************************************************
 @Project		: MET2304 LAb Assignment
 @File 			: i2c.c
 @Details  	: TM4C123G I2C driver (polling)               
 @Author		: fongfh
 @Hardware	: TM4C123G
 
 --------------------------------------------------------------------------
 @Revision	:
  Ver  	Author    	Date        	Changes
 --------------------------------------------------------------------------
   1.0  fongfh      27 Oct 23 		Initial Release

******************************************************************************/
#include <Common.h>
#include "hal.h"
#include "i2c.h"

/*****************************************************************************
 Define
******************************************************************************/
#define TOTAL_I2C	 4U /* 4 I2C */

/*****************************************************************************
 Type definition
******************************************************************************/


/*****************************************************************************
 Global Variables
******************************************************************************/


/*****************************************************************************
 Local Variables
******************************************************************************/
static volatile PI2C_HANDLE g_aI2CIrqHandle[TOTAL_I2C] = { 0, 0, 0, 0 };

/*****************************************************************************
 Local Functions
******************************************************************************/

/*****************************************************************************
 Implementation
******************************************************************************/
int I2CInit( PI2C_HANDLE pHandle, uint8_t nI2C, uint8_t nI2C_speed)
{
	I2C0_Type 	*i2c;
	IRQn_Type 	irq;
	uint8_t 		speed;
	
	ASSERT( 0 != pHandle );
	ASSERT( 0 != nI2C );
	
	/* I2C selection base on user specified */
	switch( nI2C )
	{
		case I2C_0:
			i2c = I2C0;
			irq = I2C0_IRQn;
		break;		
		case I2C_1:
			i2c = I2C1;
			irq = I2C1_IRQn;
		break;		
		case I2C_2:
			i2c = I2C2;
			irq = I2C2_IRQn;
		break;		
		case I2C_3:
			i2c = I2C3;
			irq = I2C3_IRQn;
		break;						
		default:
			return I2C_ERR;		/* User specified an invalid timer number, return error */
	}
		
	/* Keep the I2C pointer into handle for next use */
	pHandle->pI2C = i2c;
	pHandle->Irq = irq;
		
	/* Keep the handle pointer into a local variable so that it can be accessed in ISR */
	g_aI2CIrqHandle[nI2C] = pHandle;

	i2c->MCR |= I2C_MCR_MFE;
	
	switch( nI2C_speed )
	{
		case I2C_100K:
			speed = (SystemCoreClock/(100000U*20U)-1U);
		break;		
		case I2C_400K:
			speed = (SystemCoreClock/(400000U*20U)-1U);
		break;
		case I2C_600K:
			speed = (SystemCoreClock/(600000U*20U)-1U);
		break;				
		case I2C_800K:
			speed = (SystemCoreClock/(800000U*20U)-1U);
		break;		
		case I2C_1000K:
			speed = (SystemCoreClock/(1000000U*20U)-1U);
		break;		
		default:
			return I2C_ERR;		/* User specified an invalid timer number, return error */
	}
	
	i2c->MTPR |= (I2C_MTPR_TPR_M & speed);	/* Set I2C clock speed to 100Khz @ 80Mhz */
	i2c->MTPR &= ~I2C_MTPR_HS;							/* Disable High-Speed Mode */	
	i2c->MIMR &= ~I2C_MIMR_IM;							/* turn off interrupt mask  */
	
	return I2C_OK;
}

char I2CWrite( PI2C_HANDLE pHandle, uint8_t slave_addr, uint8_t ctrl_byte, uint8_t *data, uint32_t nbytes )
{
	if (nbytes <= 0) return -1;   /* no write was performed */
	
	I2C0_Type *i2c = pHandle->pI2C;
	
	/* fill in with your own codes */
	while (i2c->MCS & I2C_MCS_BUSY) {};  				// Check if the I2C is busy
  i2c->MSA = slave_addr << 1U ;  							// Write slave address with control register (R/S)
    
  i2c->MDR = ctrl_byte;  											// Write control register (R/S)
  while (i2c->MCS & I2C_MCS_BUSBSY) {};  
  i2c->MCS = I2C_MCS_START | I2C_MCS_RUN;   	// Write START and RUN (master enable, start transmission) in master control/status register
  
  uint32_t index = 0;
  for (index = 0; index < nbytes; index++)  	// Write bytes one at a time
  {    
    while (i2c->MCS & I2C_MCS_BUSY) {}; 			// Check if the I2C is busy
    if ((i2c->MCS & I2C_MCS_ERROR) != 0)  		// Check if there is ERROR
    {
      pHandle->bError = TRUE;									// Set flag for bError
    }
    else pHandle->bError = FALSE; 						// Set flag for bEror
    i2c->MDR = data[index]; 									// Write data byte before last element to data register
    i2c->MCS = I2C_MCS_RUN; 									// Write RUN
  }
  
  while (i2c->MCS & I2C_MCS_BUSY) {}; 				// Check if the I2C is busy
  i2c->MDR = data[index]; 										// Write the last element 
  i2c->MCS = I2C_MCS_RUN | I2C_MCS_STOP; 			// Write RUN and STOP for last element

	return 0;       /* no error */
}


char I2CWriteByte( PI2C_HANDLE pHandle, uint8_t slave_addr, uint8_t reg_addr )
{
	I2C0_Type *i2c = pHandle->pI2C;
	
	/* Duneed use lah  */
	/*    \(0.0)/      */
	
	return (0);  /* no error  */
}


char I2CRead( PI2C_HANDLE pHandle, uint8_t slave_addr, uint32_t nbytes )
{
	
	if (nbytes <= 0) return -1;   /* no Read performed */
	
	I2C0_Type *i2c = pHandle->pI2C;
	pHandle->bBusy = TRUE;
	pHandle->nBytes = nbytes;
	pHandle->nByteCnt = 0U;
	
	/* fill in with your own codes */
	while(0!=(i2c->MCS & I2C_MCS_BUSY));
	
	/*	Write Slave Address to I2CMSA		*/
	i2c->MSA = (slave_addr << 1) | 1U;
	
	/*	Read I2CMCS		*/
	i2c->MCS = I2C_MCS_START | I2C_MCS_RUN | I2C_MCS_ACK;

	/*	Write to I2CMCS	*/
	uint32_t index = 0;
  for (index = 0; index < nbytes-1; index++)  // Write bytes sequentially
  {    
    while (i2c->MCS & I2C_MCS_BUSY) {}; 			// Wait if the I2C is busy
		pHandle->data_rx[index] = i2c->MDR;
		i2c->MCS = I2C_MCS_RUN | I2C_MCS_ACK;
		pHandle->nByteCnt++;
	}
	 i2c->MCS = I2C_MCS_STOP | I2C_MCS_RUN;
	while(0!=(i2c->MCS & I2C_MCS_BUSY));
	pHandle->data_rx[index] = i2c->MDR;

	return (0);  /* no error  */
}


/*****************************************************************************
 Interrupt handling
******************************************************************************/
