#include <Common.h>
#include "UART.h"

//static PUART_HANDLE	g_pUartHandle;


void UART_Init(PUART_HANDLE pHandle, uint8_t UART_number)
{
	UART0_Type *uart;
	float f_tmp;
	
	switch (UART_number)
	{
		case 0:
			uart = UART0;
			break;
		case 1:
			uart = UART1;
			break;
		case 2:
			uart = UART2;
			break;
		case 3:
			uart = UART3;
			break;
		default:
			break;
	}
	
	/* keep UART pointer into handle for future use */
	pHandle->pUart = uart;
	
	/** initialize UART0   **/	
	uart->CTL &= ~UART_CTL_UARTEN; 					/* disable UART during initialization  	*/
	uart->CC &= ~UART_CC_CS_M; 							/*  clock source mask										*/
	uart->CC |= UART_CC_CS_SYSCLK; 					/* set to system clock									*/
	
	uart->CTL &= ~UART_CTL_HSE;							/* use 16X CLKDIV  											*/
	
	/* 80MHz: IBRD = 5 = int (80,000,000 / (16 * 921,600)) = (int) 5.425347   			*/
	/*        FBRD = 27 = int (5.425347 * 64 + 0.5                      						*/    
	/* 50MHz: IBRD = 3 = int (50,000,000 / (16 * 921,600)) = (int) 3.3908420 				*/
	/*	      FRBD = 217 = int (5.425347 * 64 + 0.5 = int (217.5139)         			 */
	f_tmp = SystemCoreClock/(16.0 * pHandle->baud);
	uart->IBRD = (int)(f_tmp);
	uart->FBRD = (int) (f_tmp *64 + 0.5); 
	
	uart->LCRH &= ~UART_LCRH_WLEN_M; 
	switch (pHandle->databits) // data bits can be 5,6,7,8 length
	{
		case 5: // 5 data bits
			uart->LCRH |= UART_LCRH_WLEN_5;
			break;
		case 6: // 6 data bits
			uart->LCRH |= UART_LCRH_WLEN_6;
			break; 
		case 7: // 7 data bits
			uart->LCRH |= UART_LCRH_WLEN_7;
			break;
		case 8: // 8 data bits
			uart->LCRH |= UART_LCRH_WLEN_8;
			break;
		default:
				break;
	}
	
	switch (pHandle->parity) // 0 - no parity; 1 - Odd; 2 - Even
	{
		case 0: // no parity
			uart->LCRH &= ~UART_LCRH_PEN;
			break;
		case 1: // Odd parity
			uart->LCRH |= UART_LCRH_PEN;
			uart->LCRH &= ~UART_LCRH_EPS ;   
			break;
		case 2: // Even parity
			uart->LCRH |= UART_LCRH_PEN;
			uart->LCRH |= UART_LCRH_EPS ;   	
			break;
		default:
				break;
	}
	
	switch (pHandle->stop) // 1 - one stop bit; 2 - 2 stop bits
	{
		case 1: // 1 stop bit
			uart->LCRH &= ~UART_LCRH_STP2;  					
			break;
		case 2: // 2 stop bits
			uart->LCRH |= UART_LCRH_STP2;  					 	
			break;
		default:
				break;
	}

	uart->LCRH |= UART_LCRH_FEN; 									/* enable FIFO*/
	//uart->CTL |= UART_CTL_TXE; 									/* transmit enable 							*/
	uart->CTL |= UART_CTL_TXE | UART_CTL_RXE ; 		/* transmit & receive enable 							*/
	uart->CTL |= UART_CTL_UARTEN; 								/* enable UART0   		*/
}

/** Write an ASCII character to UART0				**/
/** character = ASCII to write 							**/
void write_ASCII_UART (PUART_HANDLE pHandle, char character )
{
	UART0_Type *uart = pHandle->pUart;

	while( 0 != (uart->FR & UART_FR_TXFF) ){}; 	/* wait if TX FIFO full					*/
	uart->DR = character; 												/* write character to UART data reg */
}

void write_string_UART (PUART_HANDLE pHandle, uint8_t *msg)
{
	UART0_Type *uart = pHandle->pUart;
	
	pHandle->nByteCnt=0;
	pHandle->data = msg;
	
	while (pHandle->data[pHandle->nByteCnt] != 0)
	{
		write_ASCII_UART (pHandle, pHandle->data[pHandle->nByteCnt]);
		pHandle->nByteCnt++;	
	}
		write_ASCII_UART (pHandle, 0x0D); // Carriage Return
		write_ASCII_UART (pHandle, 0x0A); // Line Feed
}

// receive character from UART0
char read_ASCII_UART (PUART_HANDLE pHandle)
{
	UART0_Type *uart = pHandle->pUart;

	while (0 != (uart->FR & UART_FR_RXFE)){} /* loop if Rx FIFO is empty */
	return (uart->DR);
}
