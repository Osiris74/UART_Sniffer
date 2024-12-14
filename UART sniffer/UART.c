#include "UART.h"
#include "CONSTANTS.h"
#include "Common.h"
#include <string.h>


typedef struct
{
	volatile char data[UART_MAX_BUFFER];
	volatile char length;
} Message;

// Contains indexes of TX and RX buffers
typedef struct  
{
	volatile unsigned char UART_OUT_IDX;    //TX index
	volatile unsigned char UART_IN_IDX;		//RX index
	
	unsigned char UART_RX_BUFFER[UART_MAX_BUFFER];	//Buffer for tx data
	unsigned char UART_TX_BUFFER[UART_MAX_BUFFER];	//Buffer for rx data
	
	volatile int  rx_msg_len;
} Indexes;

/*
typedef struct
{
	char UART_OPERATION_MODE;				//Operation mode to check what is going on 
	char UART_FLAG;							//Flag indicating tx and rx of data
} Flags;
*/


typedef struct
{
	// Queue of messages
	volatile Message messageQueue [QUEUE_SIZE];
	volatile char queueHead;		// starting pointer
	volatile char queueTail;		// end pointer
	volatile char currentIndex;		// index of current msg byte
	volatile char sending;			// message flag
	volatile char FULL;
} UartQueue;

//unsigned char UART_RX_BUFFER[UART_MAX_BUFFER];	//Buffer for tx data
//unsigned char UART_TX_BUFFER[UART_MAX_BUFFER];	//Buffer for rx data
//volatile unsigned char UART_OUT_IDX;    //TX index
//volatile unsigned char UART_IN_IDX;		//RX index
//char UART_OPERATION_MODE;				//Operation mode to check what is going on 
//char UART_FLAG;							//Flag indicating tx and rx of data

long tempMsgValue;

int  UART_TX_MSG_LEN;


// initialization
UartQueue uart0_Queue;
Indexes   uart0_indexes;
volatile Flags	  uart0_flags;

UartQueue uart1_Queue;
Indexes   uart1_indexes;
Flags	  uart1_flags;



void uart_rx_data(Indexes *uart_indexes, Flags *uart_flags, char byte)
{
		if (byte==UART_STOP_BYTE)
		{
			// Saves last byte
			
			uart_indexes->UART_RX_BUFFER[uart_indexes->UART_IN_IDX] = byte;
			uart_indexes->UART_IN_IDX								= uart_indexes->UART_IN_IDX + 1;
			
			//uart_indexes->UART_RX_BUFFER[uart_indexes->UART_IN_IDX] = NEW_LINE;
			//uart_indexes->UART_IN_IDX								= uart_indexes->UART_IN_IDX + 1;
			
			uart_flags->UART_OPERATION_MODE							= IDLE_MODE;
			uart_flags->UART_FLAG									= DATA_RECEIVED_FLAG;
			uart_indexes->rx_msg_len				     			= uart_indexes->UART_IN_IDX;			
			uart_indexes->UART_IN_IDX								= 0;
			
			// indicates that user would like to read data
			if (uart_indexes->UART_RX_BUFFER[0] == 0x26)
			{
				uart_flags->UART_FLAG								= DATA_REQUEST_FLAG;
			}
		}
		else
		{
			uart_flags->UART_OPERATION_MODE 						= RX_MODE;
			uart_indexes->UART_RX_BUFFER[uart_indexes->UART_IN_IDX] = byte;
			uart_indexes->UART_IN_IDX								= uart_indexes->UART_IN_IDX + 1;
		}
}



void USART_INIT(void)
{
	// UART0 initialization
	UBRR0H = HI(bauddivider);
	UBRR0L = LO(bauddivider);
	UCSR0A = 1<<UDRE0|1<<U2X0;
	UCSR0B = 1<<RXEN0|1<<TXEN0|0<<UDRIE0|1<<RXCIE0;
	UCSR0C = 0<<USBS0|1<<UCSZ01|1<<UCSZ00;
	
	uart0_indexes.UART_OUT_IDX		= 0;
	uart0_indexes.UART_IN_IDX		= 0;
	uart0_flags.UART_OPERATION_MODE = IDLE_MODE;
	uart0_flags.UART_FLAG			= DATA_OPERATION_COMPLETE_FLAG;
	
	
	uart0_indexes.rx_msg_len   		= 0;
	//UART_TX_MSG_LEN					= 0;
	
	uart0_Queue.queueHead			= 0;
	uart0_Queue.queueTail			= 0;
	uart0_Queue.currentIndex		= 0;
	uart0_Queue.sending				= 0;
	
	//tempMsgValue			= 0;
	
	
	uart1_indexes.UART_OUT_IDX		= 0;
	uart1_indexes.UART_IN_IDX		= 0;
	uart1_flags.UART_OPERATION_MODE = IDLE_MODE;
	uart1_flags.UART_FLAG			= DATA_OPERATION_COMPLETE_FLAG;
	
	uart1_indexes.rx_msg_len   		= 0;
	//UART_TX_MSG_LEN					= 0;
	
	uart1_Queue.queueHead			= 0;
	uart1_Queue.queueTail			= 0;
	uart1_Queue.currentIndex		= 0;
	uart1_Queue.sending				= 0;
	
	// UART1 initialization	
	UBRR1H = HI(bauddivider);
	UBRR1L = LO(bauddivider);
	UCSR1A = 1<<UDRE1|1<<U2X1;
	UCSR1B = 1<<RXEN1|1<<TXEN1|0<<UDRIE1|1<<RXCIE1;
	UCSR1C = 0<<USBS1|1<<UCSZ11|1<<UCSZ10;
}

// Data from Master received
ISR(USART1_RX_vect)
{
	char temp;
	temp = UDR1;		
	uart_rx_data(&uart1_indexes, &uart1_flags, temp);
}

//RX USART Complete
ISR(USART0_RX_vect)
{
	char temp;
	temp = UDR0;
	uart_rx_data(&uart0_indexes, &uart0_flags, temp);
}

// Transmit message from Slave to Master
ISR(USART1_UDRE_vect)
{
		// Check have we send all bytes of current message?
	if (uart1_Queue.currentIndex < uart1_Queue.messageQueue[uart1_Queue.queueTail].length) 
	{
		// Send next Byte
		UDR1 = uart1_Queue.messageQueue[uart1_Queue.queueTail].data[uart1_Queue.currentIndex++]; 
	} 
	else 
	{
		// Current message was successfully sent
		// We need to move pointer to the next message
		uart1_Queue.queueTail = (uart1_Queue.queueTail + 1);
		uart1_Queue.queueTail = uart1_Queue.queueTail & BUF_MASK;		
		uart1_Queue.currentIndex = 0;

		// If queue is empty -> disable TX function
		if (uart1_Queue.queueTail == uart1_Queue.queueHead) 
		{
			uart1_Queue.sending      = 0;
			uart1_Queue.FULL    	 = 0;
			UCSR1B &= ~(1 << UDRIE1);  // Disable interrupt
		}
		else 
		{
			// Send next byte
			UDR1 = uart1_Queue.messageQueue[uart1_Queue.queueTail].data[uart1_Queue.currentIndex++];
		}
	}
}

// Transmit message from Master to Slave
ISR(USART0_UDRE_vect) 
{
	// Check have we send all bytes of current message?
	if (uart0_Queue.currentIndex < uart0_Queue.messageQueue[uart0_Queue.queueTail].length) 
	{
		// Send next Byte
		UDR0 = uart0_Queue.messageQueue[uart0_Queue.queueTail].data[uart0_Queue.currentIndex++]; 
	} 
	else 
	{
		// Current message was successfully sent
		// We need to move pointer to the next message
		uart0_Queue.queueTail = (uart0_Queue.queueTail + 1);
		uart0_Queue.queueTail = uart0_Queue.queueTail & BUF_MASK;		
		uart0_Queue.currentIndex = 0;

		// If queue is empty -> disable TX function
		if (uart0_Queue.queueTail == uart0_Queue.queueHead) 
		{
			uart0_Queue.sending      = 0;
			uart0_Queue.FULL		   = 0;
			UCSR0B &= ~(1 << UDRIE0);  // Disable interrupt
		}
		else 
		{
			// Send next byte
			UDR0 = uart0_Queue.messageQueue[uart0_Queue.queueTail].data[uart0_Queue.currentIndex++];
		}
	}
}



// To correctly choose which UART should be used for TX
char choose_uart(unsigned char *data, unsigned int length, Flags *uart_flags, UartQueue *uart_queue, char uart_number)
{
		unsigned int nextHead = (uart_queue->queueHead + 1);
		nextHead = nextHead & BUF_MASK;

		if ((uart_queue->queueHead < uart_queue->queueTail)  && (nextHead == uart_queue->queueTail))
		{
			uart_queue->FULL = 1;
	
			while(uart_queue->FULL);
		}

		int i;

		for (i=0; i<length; i++)
		{
			uart_queue->messageQueue[uart_queue->queueHead].data[i] = data[i];
		}
		uart_queue->messageQueue[uart_queue->queueHead].length = length;

		// Update pointer to the end of the message
		uart_queue->queueHead = nextHead;

		// There is some data inside the buffer
		if (!uart_queue->sending && uart_flags->UART_OPERATION_MODE == IDLE_MODE)
		{
			uart_queue->sending	     = 1;
			uart_queue->currentIndex = 0;
			char byte				 = uart_queue->messageQueue[uart_queue->queueTail].data[uart_queue->currentIndex++];  // start TX
			
			if (uart_number == 0)
			{
				UDR0    = byte;
				UCSR0B |= (1<<UDRIE0);
			} 
			else
			{
				UDR1    = byte;
				UCSR1B |= (1<<UDRIE1);
			}
		}	
}

// Add message to the queue
char UART_AddToQueue(unsigned char *data, unsigned int length, char uart_number) 
{
	if (uart_number == 0)
	{
		choose_uart(data, length, &uart0_flags, &uart0_Queue, 0);	
	}
	else
	{
		choose_uart(data, length, &uart1_flags, &uart1_Queue, 1);
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


/*int uart_return_RX_buf_len()
{
	int value = uart0_indexes.UART_IN_IDX; 
	return value;
}
*/

void uart_set_RX_buf_len(char uart_index)
{
	Indexes* uart_indexes;	
	if (uart_index == 0)
	{
		uart_indexes = &uart0_indexes; 
	}
	else
	{
		uart_indexes = &uart1_indexes;
	}
	uart_indexes->rx_msg_len = 0;
}

int uart_return_RX_buf(unsigned char* data, char uart_index)
{
	Indexes uart_indexes;	
	if (uart_index == 0)
	{
		uart_indexes = uart0_indexes; 
	}
	else
	{
		uart_indexes = uart1_indexes;
	}
	
	for (int i = 0; i < uart_indexes.rx_msg_len; i++) 
	{
		data[i] = uart_indexes.UART_RX_BUFFER[i]; 
	}
	
	return uart_indexes.rx_msg_len;
}