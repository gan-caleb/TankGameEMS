/*****************************************************************************
 @Project		: ECE300
 @File 			: Hal.c
 @Details  	: All Ports and peripherals configuration                    
 @Author		: fongfh
 @Hardware	: Tiva LaunchPad 
 
 --------------------------------------------------------------------------
 @Revision	:
  Ver  	Author    	Date        	Changes
 --------------------------------------------------------------------------
   1.0  Name     XXXX-XX-XX  		Initial Release
   
******************************************************************************/
#include <Common.h>
#include "Hal.h"


/*****************************************************************************
 Define
******************************************************************************/


/*****************************************************************************
 Type definition
******************************************************************************/


/*****************************************************************************
 Global Variables
******************************************************************************/


/*****************************************************************************
 Local Variables
******************************************************************************/


/*****************************************************************************
 Implementation
******************************************************************************/

void Port_Init( void )
{
	/* Enable Clock for SSI0 module */
	SYSCTL->RCGCSSI |= SYSCTL_RCGCSSI_R0;
	
	/* Waiting clock ready */
	while( 0 == (SYSCTL->PRSSI & SYSCTL_PRSSI_R0) );
	
	/*Enable Clocks Gate for Timers */
	SYSCTL->RCGCTIMER	|= SYSCTL_RCGCTIMER_R0  
					| SYSCTL_RCGCTIMER_R1 
					| SYSCTL_RCGCTIMER_R2 
					| SYSCTL_RCGCTIMER_R3 
					| SYSCTL_RCGCTIMER_R4 
					| SYSCTL_RCGCTIMER_R5; 
	
	/* enable clock to UART0    */
	SYSCTL->RCGCUART |= SYSCTL_RCGCUART_R0;
	
	/* Waiting clock ready */
	while( 0 == (SYSCTL->PRTIMER & SYSCTL_PRTIMER_R0) );
	while( 0 == (SYSCTL->PRTIMER & SYSCTL_PRTIMER_R1) );
	while( 0 == (SYSCTL->PRTIMER & SYSCTL_PRTIMER_R2) );
	while( 0 == (SYSCTL->PRTIMER & SYSCTL_PRTIMER_R3) );
	
	/*Enable Clocks Gate for GPIO */
	SYSCTL->RCGCGPIO |= SYSCTL_RCGCGPIO_R0
					 | SYSCTL_RCGCGPIO_R1
					 | SYSCTL_RCGCGPIO_R2
					 | SYSCTL_RCGCGPIO_R3
					 | SYSCTL_RCGCGPIO_R4
					 | SYSCTL_RCGCGPIO_R5;
	
	/* Wait for UART to be ready */
	while( 0 == (SYSCTL->PRUART & SYSCTL_PRUART_R0) ); // UART0
	
	/* Waiting clock ready */
	while( 0 == (SYSCTL->PRGPIO & SYSCTL_PRGPIO_R0) );
	while( 0 == (SYSCTL->PRGPIO & SYSCTL_PRGPIO_R1) );
	while( 0 == (SYSCTL->PRGPIO & SYSCTL_PRGPIO_R2) );
	while( 0 == (SYSCTL->PRGPIO & SYSCTL_PRGPIO_R3) );
	while( 0 == (SYSCTL->PRGPIO & SYSCTL_PRGPIO_R4) );
	while( 0 == (SYSCTL->PRGPIO & SYSCTL_PRGPIO_R5) );
	

	/* Verification Output */
	GPIOB->DIR |= BIT(PB_T3CCP0);
	GPIOB->DEN |= BIT(PB_T3CCP0);	
	GPIOB->AFSEL &= ~BIT(PB_T3CCP0);

	

	/* User button SW1 & SW2 PLAYER 1 */
	GPIOF->LOCK = GPIO_LOCK_KEY;
	
	GPIOF->CR |= (BIT(PF_SW1) | BIT(PF_SW2));
	GPIOF->DIR &= ~(BIT(PF_SW1) | BIT(PF_SW2));
	GPIOF->DEN |= (BIT(PF_SW1) | BIT(PF_SW2));
	GPIOF->AFSEL &= ~(BIT(PF_SW1) | BIT(PF_SW2));
	GPIOF->PCTL &= ~(BIT(PF_SW1) | BIT(PF_SW2));
	GPIOF->PUR |= (BIT(PF_SW1) | BIT(PF_SW2));
	
	GPIOF->IM &= ~(BIT(PF_SW1) | BIT(PF_SW2));
	GPIOF->IBE &= ~(BIT(PF_SW1) | BIT(PF_SW2)); /* Disable both edge */
	GPIOF->IS &= ~(BIT(PF_SW1) | BIT(PF_SW2)); /* edge detection */
	GPIOF->IEV = ~(BIT(PF_SW1) | BIT(PF_SW2)); /* Falling edge */
	GPIOF->IM |= (BIT(PF_SW1) | BIT(PF_SW2));
	
/* Host Board Configuration - Using LED Pins as Inputs for PLAYER 2 */

// Allow changes to PF_LED_RED and PF_LED_BLUE
GPIOF->CR |= BIT(PF_LED_RED) | BIT(PF_LED_BLUE);

// Set PF_LED_RED and PF_LED_BLUE as inputs
GPIOF->DIR &= ~(BIT(PF_LED_RED) | BIT(PF_LED_BLUE));

// Enable digital functionality for PF_LED_RED and PF_LED_BLUE
GPIOF->DEN |= BIT(PF_LED_RED) | BIT(PF_LED_BLUE);

// Disable alternate functions for PF_LED_RED and PF_LED_BLUE
GPIOF->AFSEL &= ~(BIT(PF_LED_RED) | BIT(PF_LED_BLUE));
GPIOF->PCTL &= ~(BIT(PF_LED_RED) | BIT(PF_LED_BLUE));

// If using internal pull-up resistors
GPIOF->PUR |= BIT(PF_LED_RED) | BIT(PF_LED_BLUE);

// Configure interrupt settings (if using interrupts)
GPIOF->IM &= ~(BIT(PF_LED_RED) | BIT(PF_LED_BLUE)); // Disable interrupts
GPIOF->IS &= ~(BIT(PF_LED_RED) | BIT(PF_LED_BLUE)); // Edge trigger
GPIOF->IBE &= ~(BIT(PF_LED_RED) | BIT(PF_LED_BLUE)); // Single edge
GPIOF->IEV &= ~(BIT(PF_LED_RED) | BIT(PF_LED_BLUE)); // Falling edge
GPIOF->IM |= BIT(PF_LED_RED) | BIT(PF_LED_BLUE); // Re-enable interrupts


	GPIOB->LOCK = GPIO_LOCK_KEY;	/* Unlock Port B */
  GPIOB->CR |= BIT(PB_6);					/* Set the Commit Control register for PB6 */
  GPIOB->DIR |= BIT(PB_6);				/* Clear the Direction register bit for PB6 */
  GPIOB->PUR |= BIT(PB_6);					/* Enable the pull-up resistor on PB6 */
  GPIOB->DEN |= BIT(PB_6);					/* Enable digital I/O on PB6 */
	
	/* initialize GPIO PA0 (UART0_RX) & PA1 (UART0_TX)   */
	GPIOA->AFSEL |= BIT(PA_UART0_RX) | BIT(PA_UART0_TX);
	GPIOA->DEN |= BIT(PA_UART0_RX) | BIT(PA_UART0_TX);
	GPIOA->AMSEL &= ~( BIT(PA_UART0_RX) | BIT(PA_UART0_TX) );
	GPIOA->PCTL &= ~( GPIO_PCTL_PA0_M | GPIO_PCTL_PA1_M ); /* clear Port C config bits  */
	GPIOA->PCTL |= GPIO_PCTL_PA0_U0RX | GPIO_PCTL_PA1_U0TX;
	
	//Initialise Buzzer
	GPIOA->DIR |= BIT(PA_BUZZER);
	GPIOA->DEN |= BIT(PA_BUZZER);
	GPIOA->AFSEL &=	~( BIT(PA_BUZZER));
	
	/* SSI & LCD */
	GPIOA->LOCK = GPIO_LOCK_KEY;
	
	GPIOA->DIR |= BIT(PA_LCD_SSI0_CS) | BIT(PA_LCD_DC) | BIT(PA_LCD_RESET);
	GPIOA->DEN |= BIT(PA_LCD_SSI0_CS) | BIT(PA_LCD_DC) | BIT(PA_LCD_RESET);
	GPIOA->AFSEL &= ~(BIT(PA_LCD_SSI0_CS) | BIT(PA_LCD_DC) | BIT(PA_LCD_RESET));
	GPIOA->PCTL &= ~( GPIO_PCTL_PA3_M | GPIO_PCTL_PA6_M | GPIO_PCTL_PA7_M );
	
	GPIOA->DEN |= BIT(PA_LCD_SSI0_SCK) | BIT(PA_LCD_SSI0_MOSI);
	GPIOA->AFSEL |= (BIT(PA_LCD_SSI0_SCK) | BIT(PA_LCD_SSI0_MOSI) );
	GPIOA->PCTL &= ~(GPIO_PCTL_PA2_M | GPIO_PCTL_PA5_M);
	GPIOA->PCTL |= (GPIO_PCTL_PA2_SSI0CLK | GPIO_PCTL_PA5_SSI0TX);
	
	GPIOB->DIR |= BIT(PB_LCD_BL);
	GPIOB->DEN |= BIT(PB_LCD_BL);
	
	/* Keypad 3x4 matrix */
	GPIOD->DIR |= BIT(PD_KEYPAD_ROW0) | BIT(PD_KEYPAD_ROW1) | BIT(PD_KEYPAD_ROW2) | BIT(PD_KEYPAD_ROW3);
	GPIOD->DEN |=  BIT(PD_KEYPAD_ROW0) | BIT(PD_KEYPAD_ROW1) | BIT(PD_KEYPAD_ROW2) | BIT(PD_KEYPAD_ROW3);
	GPIOD->AFSEL &= ~(BIT(PD_KEYPAD_ROW0) | BIT(PD_KEYPAD_ROW1) | BIT(PD_KEYPAD_ROW2) | BIT(PD_KEYPAD_ROW3));
	GPIOD->PCTL &= ~( GPIO_PCTL_PD0_M | GPIO_PCTL_PD1_M | GPIO_PCTL_PD2_M | GPIO_PCTL_PD3_M );
	

	GPIOE->DIR &= ~(BIT(PE_KEYPAD_COL0) | BIT(PE_KEYPAD_COL1) | BIT(PE_KEYPAD_COL2));
	GPIOE->DEN |= (BIT(PE_KEYPAD_COL0) | BIT(PE_KEYPAD_COL1) | BIT(PE_KEYPAD_COL2));
	GPIOE->AFSEL &= ~(BIT(PE_KEYPAD_COL0) | BIT(PE_KEYPAD_COL1) | BIT(PE_KEYPAD_COL2));
	GPIOE->PCTL &= ~( GPIO_PCTL_PE2_M | GPIO_PCTL_PE3_M | GPIO_PCTL_PE4_M );
	GPIOE->PUR |= (BIT(PE_KEYPAD_COL0) | BIT(PE_KEYPAD_COL1) | BIT(PE_KEYPAD_COL2));
	
	 /* Enable Clocks Gate for ADC0 */
  SYSCTL->RCGCADC |= SYSCTL_RCGCADC_R0; 
  /* Enable Clocks Gate for ADC0 */
  SYSCTL->RCGCADC |= SYSCTL_RCGCADC_R1;   

  /* Wait of clock to be ready */
  while( 0 == (SYSCTL->PRADC & SYSCTL_PRADC_R0) );
  while( 0 == (SYSCTL->PRADC & SYSCTL_PRADC_R1) );

  /* Configure PE4 - AIN9 and PE5 - AIN8 as Analog In */
  GPIOE->CR |= (BIT(PE_ADC_AIN8) | BIT(PE_ADC_AIN9));
  GPIOE->AFSEL |= (BIT(PE_ADC_AIN8) | BIT(PE_ADC_AIN9));
  GPIOE->DEN &= ~(BIT(PE_ADC_AIN8) | BIT(PE_ADC_AIN9));
  GPIOE->AMSEL |= (BIT(PE_ADC_AIN8) | BIT(PE_ADC_AIN9));

  /* Enable POT (PE5) */
  GPIOE->AFSEL  |= BIT(PE_ADC_AIN8);     // enable alternate function
  GPIOE->DIR    &= ~BIT(PE_ADC_AIN8);   // set direction to input
  GPIOE->DEN    &= ~BIT(PE_ADC_AIN8);   // disable digital
  GPIOE->AMSEL   |= BIT(PE_ADC_AIN8);    // analog mode select
  
  /* Enable External POT (PE4)  - Player 2*/
  GPIOE->AFSEL  |= BIT(PE_ADC_AIN9);     // enable alternate function
  GPIOE->DIR    &= ~BIT(PE_ADC_AIN9);   // set direction to input
  GPIOE->DEN    &= ~BIT(PE_ADC_AIN9);   // disable digital
  GPIOE->AMSEL   |= BIT(PE_ADC_AIN9);    // analog mode select

  /* Init ADC0 */
  SYSCTL->RCGCADC |= SYSCTL_RCGCADC_R0;  // Enable clock to ADC0
  while ( 0 == (SYSCTL->PRADC & SYSCTL_PRADC_R0)){};  // Wait for ADC0 to be ready    
  /* Init ADC1 */
  SYSCTL->RCGCADC |= SYSCTL_RCGCADC_R1;  // Enable clock to ADC0
  while ( 0 == (SYSCTL->PRADC & SYSCTL_PRADC_R1)){};  // Wait for ADC1 to be ready
	
	/* Setup GPIO PB6 pin for Timer 0A */
	GPIOB->DIR |= BIT(PB_T0CCP0);
	GPIOB->DEN |= BIT(PB_T0CCP0);	
	GPIOB->AFSEL |= BIT(PB_T0CCP0);
	GPIOB->PCTL &= ~GPIO_PCTL_PB6_M;
	GPIOB->PCTL |= GPIO_PCTL_PB6_T0CCP0;
	
	/* Configure Timer 0A */
	TIMER0->CTL &= ~TIMER_CTL_TAEN; 					/* 1. disable timer 0 during setup */
	TIMER0->CFG |= TIMER_CFG_16_BIT;					/* 2. set to 16-bit mode */
	TIMER0->TAMR |= TIMER_TAMR_TAMR_PERIOD ; 	/* 3. periodic mode */
	TIMER0->TAMR &= ~TIMER_TAMR_TACDIR; 			/* 4. count down */
	TIMER0->TAILR = 0xFC03; 									/* 5. reload value; max = 0xFFFF.*/
	TIMER0->TAPR = 0x3D; 											/* 6. prescaler (8 bits) */
	TIMER0->IMR |= TIMER_IMR_TATOIM; 					/* 7. enable timeout intr */
	TIMER0->ICR |= TIMER_ICR_TATOCINT; 				/* 8. clear timeout flag */
	TIMER0->CTL |= TIMER_CTL_TAEN; 						/* 9. enable timer 0 */

	/** initialize UART0   **/	
	UART0->CTL &= ~UART_CTL_UARTEN; 					/* disable UART during initialization  	*/
	UART0->CC &= ~UART_CC_CS_M; 							//  clock source mask
	UART0->CC |= UART_CC_CS_SYSCLK; 					// set to system clock
	UART0->CTL &= ~UART_CTL_HSE;							/* use 16X CLKDIV	*/
	
	// 80 MHz
	UART0->IBRD = 5;	 					/* int (80,000,000 / (16 * 921,600)) = 5.425347    */
	UART0->FBRD = 27;  					/* int (5.425347 * 64 + 0.5 = int (27.72) = 27   */
	UART0->LCRH &= ~UART_LCRH_WLEN_M; 	
	/**		UART Settings for Putty		**/
	UART0->LCRH |= UART_LCRH_WLEN_7 | UART_LCRH_PEN; 	/* 7 data bits, parity enable  	*/
	UART0->LCRH |= UART_LCRH_EPS ;   									/* even parity  								*/ 	
	UART0->LCRH &= ~UART_LCRH_STP2;  									/* 1 stop bit 									*/ 
	UART0->LCRH |= UART_LCRH_FEN; 										/* enable FIFO*/
	UART0->CTL |= UART_CTL_TXE; 										/* transmit enable 							*/
	UART0->CTL |= UART_CTL_TXE | UART_CTL_RXE ; 			/* transmit & receive enable 							*/
	UART0->CTL |= UART_CTL_UARTEN; 										/* enable UART0   		*/

}
	/** Write an ASCII character to UART0				**/
	/** character = ASCII to write 							**/
	void write_ASCII_UART0 (char character )
	{
		while( 0 != (UART0->FR & UART_FR_TXFF) ){}; 	/* wait if TX FIFO full					*/
		UART0->DR = character; 												/* write character to UART data reg */
	}

	// receive character from UART0
	char read_ASCII_UART0 (void)
	{
		while (0 != (UART0->FR & UART_FR_RXFE)){} /* loop if Rx FIFO is empty */
		return (UART0->DR);
	}
















