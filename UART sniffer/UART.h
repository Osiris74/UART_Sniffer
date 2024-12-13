#ifndef UART_H_
#define UART_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#define XTAL 16000000L
#define baudrate 250000L
#define bauddivider 51			//38400 velocity			//(XTAL/(16*baudrate)-1)	//250kBaud
#define HI(x) ((x)>>8)
#define LO(x) ((x)& 0xFF)

#define UART_MAX_RING_BUFFER_LENGTH  0x20   // Max ring buffer length

#define UART_MAX_BUFFER		0x20		// 32 byte - Length of the transmition buffer
#define UART_STOP_BYTE      0x0D		// Carridge return
#define UART_ECHO_BYTE      '?'			// Fix me for another bytes
#define NEW_LINE			0x0A

#define IDLE_MODE			0x00
#define TX_MODE				0x01
#define RX_MODE				0x02

#define DATA_OPERATION_COMPLETE_FLAG 0x00
#define DATA_TRANSMIT_FLAG			 0x01
#define DATA_RECEIVED_FLAG		     0x02
#define DATA_READY_FLAG              0x03
#define DATA_ECHO_FLAG               0x04
#define DATA_REQUEST_FLAG			 0x05

////////////////////UART///////////////////////////////////////////////
// Length of received data
extern unsigned char rx_buffer[UART_MAX_BUFFER];
extern int  rx_msg_len;
extern char UART_OPERATION_MODE;				//Operation mode to check what is going on 
extern char UART_FLAG;

void USART_INIT(void);
//void UART_TRANSMIT(int len, unsigned char *buffer);
void UART_CHECK_RX(void);


//////////////////////////////////////////////////MESSAGE STRUCTURE////////////////////////////////

#define QUEUE_SIZE								0x04
#define BUF_MASK (UART_MAX_RING_BUFFER_LENGTH-1)

char UART_AddToQueue(unsigned char *data, unsigned int length, char uart_number);

void uart_send_data(long data, char msg_id);

int uart_return_RX_buf(unsigned char* data, char uart_number);

void uart_set_RX_buf_len(char uart_number);


volatile typedef struct
{
	volatile char UART_OPERATION_MODE;				//Operation mode to check what is going on
	volatile char UART_FLAG;						//Flag indicating tx and rx of data
} Flags;



volatile extern Flags	  uart0_flags;
volatile extern Flags	  uart1_flags;


#endif

