/*****************************************************************************
 @Project	: 
 @File 		: 
 @Details  	: All Ports and peripherals configuration                    
 @Author	: 
 @Hardware	: 
 
 --------------------------------------------------------------------------
 @Revision	:
  Ver  	Author    	Date        	Changes
 --------------------------------------------------------------------------
   1.0  Name     XXXX-XX-XX  		Initial Release
   
******************************************************************************/

#ifndef __HAL_DOT_H__
#define __HAL_DOT_H__


/*****************************************************************************
 Define
******************************************************************************/
#define PA_UART0_RX				0U		// PA0
#define PA_UART0_TX				1U		// PA1
#define PA_LCD_SSI0_SCK		2U
#define PA_LCD_SSI0_CS		3U
#define PA_BUZZER					4U
#define PA_LCD_SSI0_MOSI 	5U
#define PA_LCD_DC					6U
#define PA_LCD_RESET			7U

#define PB_LED_IR					0U
#define PB_LCD_BL					1U
#define PB_T3CCP0					2U
#define PB_T3CCP1					3U
#define PB_ADC_AIN10			4U
//#define PB5_TESTPIN				5U
#define PB_T0CCP0					6U
#define PC_STEPPER0				4U
#define PC_STEPPER1				5U
#define PC_STEPPER2				6U
#define PC_STEPPER3				7U

#define PD_OUTPUT					1U
#define PD_KEYPAD_ROW0		0U
#define PD_KEYPAD_ROW1		1U
#define PD_KEYPAD_ROW2		2U
#define PD_KEYPAD_ROW3		3U

#define PE_IR_INPUT				0U
#define PE_KEYPAD_COL0		1U
#define PE_KEYPAD_COL1		2U
#define PE_KEYPAD_COL2		3U
#define PE_ADC_AIN8				5U
#define PE_ADC_AIN9				4U

#define PF_SW2						0U
#define PF_LED_RED				1U
#define PF_LED_BLUE				2U
#define PF_LED_GREEN			3U
#define PF_SW1						4U

#define SYSCTL_RCGCTIMER_R0 0x00000001 // Timer 0 Run Mode Clock Gating Ctrl
#define SYSCTL_PRTIMER_R0 0x00000001 // Timer 0 Peripheral Ready

#define PE_ADC_AIN8        5U
#define PE_ADC_AIN9        4U
#define AIN8               8U
#define AIN9               9U

/*****************************************************************************
 Type definition
******************************************************************************/


/*****************************************************************************
 Macro
******************************************************************************/
#define BIT( x )					(1U<<(x))

/* Verification Output */
#define VERIFY_ON()					( GPIOB->DATA |= BIT(PB_T3CCP0) )
#define VERIFY_OFF()				( GPIOB->DATA &= ~BIT(PB_T3CCP0))
#define VERIFY_SET( x )			((x)? VERIFY_ON() : VERIFY_OFF())

/* Buttons */
#define  SW1()  						( ((~GPIOF->DATA) >> PF_SW1) & 0x01 ) 	// read SW1: TRUE when SW1 pressed
#define  SW2()  						( ((~GPIOF->DATA) >> PF_SW2) & 0x01 ) 	// read SW2: TRUE when SW2 pressed

/* Buzzer */
#define BUZZER_ON()					( GPIOA->DATA |= BIT(PA_BUZZER) )
#define BUZZER_OFF()				( GPIOA->DATA &= ~BIT(PA_BUZZER))
#define BUZZER_SET( x )			((x)? BUZZER_ON() : BUZZER_OFF())

/* LCD */
#define LCD_CS_HIGH()				(GPIOA->DATA |= BIT(PA_LCD_SSI0_CS))
#define LCD_CS_LOW()				(GPIOA->DATA &= ~BIT(PA_LCD_SSI0_CS))
#define LCD_CS_SET( x )				((x)? LCD_CS_ON() : LCD_CS_OFF())

#define LCD_DC_HIGH()				(GPIOA->DATA |= BIT(PA_LCD_DC))
#define LCD_DC_LOW()				(GPIOA->DATA &= ~BIT(PA_LCD_DC))
#define LCD_DC_SET( x )				((x)? LCD_DC_ON() : LCD_DC_OFF())

#define LCD_RESET_HIGH()			(GPIOA->DATA |= BIT(PA_LCD_RESET))
#define LCD_RESET_LOW()				(GPIOA->DATA &= ~BIT(PA_LCD_RESET))
#define LCD_RESET_SET( x )			((x)? LCD_RESET_ON() : LCD_RESET_OFF())

#define LCD_BL_ON()					(GPIOB->DATA |= BIT(PB_LCD_BL))
#define LCD_BL_OFF()				(GPIOB->DATA &= ~BIT(PB_LCD_BL))
#define LCD_BL_SET( x )				((x)? LCD_BL_ON() : LCD_BL_OFF())

/* Keypad */
#define KEYPAD_ROW0_ON()				(GPIOD->DATA |= BIT(PD_KEYPAD_ROW0))
#define KEYPAD_ROW0_OFF()				(GPIOD->DATA &= ~BIT(PD_KEYPAD_ROW0))
#define KEYPAD_ROW0_SET( x )		((x)? KEYPAD_ROW0_ON() : KEYPAD_ROW0_OFF())

#define KEYPAD_ROW1_ON()				(GPIOD->DATA |= BIT(PD_KEYPAD_ROW1))
#define KEYPAD_ROW1_OFF()				(GPIOD->DATA &= ~BIT(PD_KEYPAD_ROW1))
#define KEYPAD_ROW1_SET( x )		((x)? KEYPAD_ROW1_ON() : KEYPAD_ROW1_OFF())

#define KEYPAD_ROW2_ON()				(GPIOD->DATA |= BIT(PD_KEYPAD_ROW2))
#define KEYPAD_ROW2_OFF()				(GPIOD->DATA &= ~BIT(PD_KEYPAD_ROW2))
#define KEYPAD_ROW2_SET( x )		((x)? KEYPAD_ROW2_ON() : KEYPAD_ROW2_OFF())

#define KEYPAD_ROW3_ON()				(GPIOD->DATA |= BIT(PD_KEYPAD_ROW3))
#define KEYPAD_ROW3_OFF()				(GPIOD->DATA &= ~BIT(PD_KEYPAD_ROW3))
#define KEYPAD_ROW3_SET( x )		((x)? KEYPAD_ROW3_ON() : KEYPAD_ROW3_OFF())

#define KEYPAD_ALL_ROWS_ON()		(GPIOD->DATA |= BIT(PD_KEYPAD_ROW0)   \
																	| BIT(PD_KEYPAD_ROW1) \
																	| BIT(PD_KEYPAD_ROW2) \
																	| BIT(PD_KEYPAD_ROW3))

#define KEYPAD_ALL_ROWS_OFF()		(GPIOD->DATA &= ~(BIT(PD_KEYPAD_ROW0)   \
																	| BIT(PD_KEYPAD_ROW1) \
																	| BIT(PD_KEYPAD_ROW2) \
																	| BIT(PD_KEYPAD_ROW3)))							
													
#define KEYPAD_COL_IN()					((GPIOE->DATA&(BIT(PE_KEYPAD_COL0) | BIT(PE_KEYPAD_COL1) | BIT(PE_KEYPAD_COL2)))>>PE_KEYPAD_COL0)

#define KEPAD_ROW_MASKED      	(*((volatile unsigned long *)(GPIOD_BASE + (0x0F << 2U))))

#define STEPPER_OUT_MASKED      (*((volatile unsigned long *)(GPIOC_BASE + (0xF0 << 2U))))
	

/* ADC */
#define ADC0_SS0_Start()    (  (ADC0->PSSI |= ADC_PSSI_SS0) )
#define ADC0_BUSY()          ( (ADC0->ACTSS & ADC_ACTSS_BUSY) == 0x01? TRUE : FALSE)
#define ADC0_GET_MUX0_FIFO()      (ADC0->SSFIFO0)

#define ADC1_SS0_Start()    (  (ADC1->PSSI |= ADC_PSSI_SS0) )
#define ADC1_BUSY()          ( (ADC1->ACTSS & ADC_ACTSS_BUSY) == 0x01? TRUE : FALSE)
#define ADC1_GET_MUX0_FIFO()       (ADC1->SSFIFO0)

/******************************************************************************
 Global functions
******************************************************************************/


/******************************************************************************
 @Description 	: 

 @param			: 
 
 @revision		: 1.0.0
 
******************************************************************************/
void Port_Init( void );

unsigned char MatrixKeypad_Scan(long*);

unsigned char MatrixKeypad_In(void);

#endif /* __HAL_DOT_H__ */









