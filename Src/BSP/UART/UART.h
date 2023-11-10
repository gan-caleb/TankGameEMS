#include <Common.h>

#define PARITY_NONE 	0UL
#define PARITY_ODD 		1UL
#define PARITY_EVEN 	2UL

#define DATABITS7		 	7UL
#define DATABITS8		 	8UL
#define DATABITS9		 	9UL

#define STOP1		 			1UL
#define STOP2		 			2UL

#define UART_0				0U 	// Rx - PA0			; Tx - PA1
#define UART_1				1U 	// Rx - PC4,PB0	; Tx - PC5,PB1
#define UART_2				2U 	// Rx - PD6			; Tx - PD7
#define UART_3				3U	// Rx - PC6			; Tx - PC7
#define UART_4				4U	// Rx - PC4			; Tx - PC5
#define UART_5				5U	// Rx - PE4			; Tx - PE5
#define UART_6				6U	// Rx - PD4			; Tx - PD5
#define UART_7				7U	// Rx - PE0			; Tx - PE1

typedef struct _tagUART_Handle
{
	void 	*pUart;
	int		irq;
	int		baud;
	int		databits;
	int		parity;
	int		stop;
	uint8_t	*data;
	uint32_t nByteCnt;
}UART_HANDLE, *PUART_HANDLE;

#define UART0_RXDATA() (!(UART0->FR & UART_FR_RXFE))  // returns 1 if rx_data is present
#define UART2_RXDATA() (!(UART2->FR & UART_FR_RXFE))


void UART_Init(PUART_HANDLE pHandle, uint8_t UART_number);
void write_ASCII_UART (PUART_HANDLE pHandle, char character);
void write_string_UART (PUART_HANDLE pHandle, uint8_t *msg);
char read_ASCII_UART (PUART_HANDLE pHandle);
