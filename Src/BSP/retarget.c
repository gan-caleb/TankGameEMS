#include <Common.h>
#include "Hal.h"
#include "UART.h"

/*****************************************************************************
 Define
******************************************************************************/

#if defined (__ARMCC_VERSION)
	#if __ARMCC_VERSION < 6000000  
		struct __FILE
		{
			int handle; 
		};
	#endif
#endif

FILE __stdout; //STDOUT

int fputc(int ch, FILE * stream)
{
	while( 0 != (UART0->FR & UART_FR_TXFF) ){}; 	/* wait if TX FIFO full					*/
	UART0->DR = ch; 												/* write character to UART data reg */
	return ch; //return the character written to denote a successful write
}
