#include "UART.h"
#include "CONSTANTS.h"
#include "Common.h"
#include <string.h>


typedef struct
{
	volatile char data[UART_MAX_BUFFER];
	volatile char length;
} Message;


typedef struct
{
	// Queue of messages
	volatile Message messageQueue [QUEUE_SIZE];
	volatile char queueHead;		// starting pointer
	volatile char queueTail;		// end pointer
	volatile char currentIndex;		// index of current msg byte
	volatile char sending;			// message flag
	volatile char FULL;
} UARTQueue;




unsigned char UART_RX_BUFFER[UART_MAX_BUFFER];	//Buffer for tx data
unsigned char UART_TX_BUFFER[UART_MAX_BUFFER];	//Buffer for rx data
unsigned char rx_buffer     [UART_MAX_BUFFER];	//RX Buffer to transmit data to user 

volatile unsigned char UART_OUT_IDX;    //TX index
unsigned char UART_IN_IDX;				//RX index
char UART_BYTE_CNT;						//Number of transmitted bytes
char UART_OPERATION_MODE;				//Operation mode to check what is going on 
char UART_FLAG;							//Flag indicating tx and rx of data

long tempMsgValue;

int  rx_msg_len;
int  UART_TX_MSG_LEN;


// initialization
UARTQueue uartQueue;


void USART_INIT(void)
{
	UBRR0H = HI(bauddivider);
	UBRR0L = LO(bauddivider);
	UCSR0A = 1<<UDRE0|1<<U2X0;
	UCSR0B = 1<<RXEN0|1<<TXEN0|0<<UDRIE0|1<<RXCIE0;
	UCSR0C = 0<<USBS0|1<<UCSZ01|1<<UCSZ00;
	
	UART_OUT_IDX		= 0;
	UART_IN_IDX			= 0;
	UART_BYTE_CNT		= 0;
	rx_msg_len   		= 0;
	UART_TX_MSG_LEN     = 0;
	
	uartQueue.queueHead		= 0;
	uartQueue.queueTail		= 0;
	uartQueue.currentIndex  = 0;
	uartQueue.sending		= 0;
	
	tempMsgValue			= 0;
	
	
	UART_OPERATION_MODE = IDLE_MODE;
	UART_FLAG			= DATA_OPERATION_COMPLETE_FLAG;	
}

// UART TX
ISR(USART0_UDRE_vect) 
{
	// Check have we send all bytes of current message?
	if (uartQueue.currentIndex < uartQueue.messageQueue[uartQueue.queueTail].length) 
	{
		// Send next Byte
		UDR0 = uartQueue.messageQueue[uartQueue.queueTail].data[uartQueue.currentIndex++]; 
	} 
	else 
	{
		// Current message was successfully sent
		// We need to move pointer to the next message
		uartQueue.queueTail = (uartQueue.queueTail + 1);
		uartQueue.queueTail = uartQueue.queueTail & BUF_MASK;		
		uartQueue.currentIndex = 0;

		// If queue is empty -> disable TX function
		if (uartQueue.queueTail == uartQueue.queueHead) 
		{
			uartQueue.sending      = 0;
			uartQueue.FULL		   = 0;
			UCSR0B &= ~(1 << UDRIE0);  // Disable interrupt
		}
		else 
		{
			// Send next byte
			UDR0 = uartQueue.messageQueue[uartQueue.queueTail].data[uartQueue.currentIndex++];
		}
	}
}


// Add message to the queue
char UART_AddToQueue(unsigned char *data, unsigned int length) 
{
	unsigned int nextHead = (uartQueue.queueHead + 1);
	nextHead = nextHead & BUF_MASK;
	
	if ((uartQueue.queueHead < uartQueue.queueTail)  && (nextHead == uartQueue.queueTail))
	{
		uartQueue.FULL = 1;
		
		while(uartQueue.FULL);
	}
	
	int i;
	
	for (i=0; i<length; i++)
	{
		uartQueue.messageQueue[uartQueue.queueHead].data[i] = data[i]; 
	}	
	uartQueue.messageQueue[uartQueue.queueHead].length = length;

	// Update pointer to the end of the message
	uartQueue.queueHead = nextHead;

	// There is some data inside the buffer
		if (!uartQueue.sending && UART_OPERATION_MODE == IDLE_MODE) 
		{
			uartQueue.sending	   = 1;
			uartQueue.currentIndex = 0;
			UDR0 = uartQueue.messageQueue[uartQueue.queueTail].data[uartQueue.currentIndex++];  // start TX
			UCSR0B |= (1<<UDRIE0);
		}

	return 1;
}


void uart_send_data(long data, char msg_id)
{
	unsigned char buff [5];
	// prepare buff from long data
	long_to_byte_array(data, buff);
	
	//prepare data: add length + id byte
	prepare_and_send_data(4, buff, msg_id);
}


long byteArrayToInt(const char *byteArray, int length)
{
	long result = 0;
	result |= byteArray[0] << 8;
	result |= byteArray[1];
	return result;
}

long charArrayToLong(const char *charArray, int length) 
{
	long result = 0;  
	int i;	
	result = (charArray[0] - '0');
	for (i = 1; i < length; i++) {
		// Convert ASCII to the number by lowing the value by '0' (0x30)
		// Add the result by multiplying to 10 for index
		result = result*10 + (charArray[i] - '0');
	}	
	return result;
}


//RX USART Complete
ISR(USART0_RX_vect)
{
	char temp;
	temp = UDR0;
	
	if (temp==UART_STOP_BYTE)
	{
		// Saves last byte
		UART_RX_BUFFER[UART_IN_IDX] = temp;
		UART_IN_IDX					= UART_IN_IDX + 1;
		
		UART_RX_BUFFER[UART_IN_IDX] = NEW_LINE;
		UART_IN_IDX					= UART_IN_IDX + 1;
		
	   	UART_OPERATION_MODE			= IDLE_MODE;
		UART_FLAG					= DATA_RECEIVED_FLAG;
		rx_msg_len					= UART_IN_IDX; 
		
		UART_IN_IDX					= 0;
		// indicates that user would like to read data
		if (UART_RX_BUFFER[0] == 0x26) 
		{
			UART_FLAG				= DATA_REQUEST_FLAG;
			UART_IN_IDX				= 0;
		}
	}
	else
	{
	  	UART_OPERATION_MODE			= RX_MODE;
	  	UART_RX_BUFFER[UART_IN_IDX] = temp;		  
	  	UART_IN_IDX					= UART_IN_IDX + 1;	
	}
}


int uart_return_RX_buf_len()
{
	int value = UART_IN_IDX; 
	return value;
}

void uart_set_RX_buf_len()
{
	rx_msg_len = 0;
}

int uart_return_RX_buf(unsigned char* data)
{
	for (int i = 0; i < rx_msg_len; i++) 
	{
		data[i] = UART_RX_BUFFER[i]; 
	}
	return rx_msg_len;
}