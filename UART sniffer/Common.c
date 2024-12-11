#include "Common.h"
#include "UART.h"
#include "CONSTANTS.h"


void long_to_byte_array(long value, unsigned char *buff)
{
	buff[0] = value >> 24;
	buff[1] = value >> 16;
	buff[2] = value >> 8;
	buff[3] = value >> 0;
}


void prepare_and_send_data(int length, unsigned char *buffer, char msg_id)
{
	// increase length of TX buffer
	// We add: 2 bytes of packet length
	//		   1 byte  of msg_id
	//         1 stop  byte
	
	unsigned int newLength = length + 4;
	
	unsigned char buff [newLength];
	char i;
	
	// MSB
	buff[0] = (newLength-1) >> 8;
	// LSB
	buff[1] = (newLength-1);
	// MSG_ID
	buff[2] = msg_id;
	
	for (i=0; i<length; i++)
	{
		buff[i+3] = buffer[i];
	}
	//
	buff[newLength-1] = MSG_STOB_BYTE;
		
	UART_AddToQueue(buff, newLength);
}