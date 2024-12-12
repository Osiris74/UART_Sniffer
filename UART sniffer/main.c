/*
 * UART sniffer.c
 *
 * Created: 12/9/2024 3:36:23 PM
 * Author : Alexey
 */ 


#define 	F_CPU   16000000L

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <avr/eeprom.h>


#include "UART.h"
#include "CONSTANTS.h"

volatile long data_addr;



void save_eeprom_data()
{
	if (data_addr == 0)
	{
		// write data length
		unsigned char data[32];
		int len;
		
		len = uart_return_RX_buf(data);				
		data_addr = len;		
						
		// Zero length of the buff
		uart_set_RX_buf_len();
		
		// Saves length of the data		
		eeprom_write_byte((uint8_t*)0, data_addr >> 8);
		eeprom_write_byte((uint8_t*)1, data_addr	   );
		
		//DEBUG
		//UART_AddToQueue(data, len);
		
		// Add data to the eeprom
		for (int i=0; i < len; i++)
		{
			eeprom_write_byte((uint8_t*) (2+i), data[i]);
		}
	}
	else
	{
		// TO DO: maybe I do not need to read address from eeprom
		// I need only to re-write it 
		
		unsigned int offset;
		
		// Read length of previous data
		offset      = eeprom_read_byte((uint8_t*)0);
		offset      = offset << 8;
		offset	    = offset + eeprom_read_byte((uint8_t*)1);
		
		unsigned char data[32];
		int len;
		
		len         = uart_return_RX_buf(data);
		data_addr   = offset + len;
		
		// Zero length of the buff
		uart_set_RX_buf_len();
		
		// Set new length of the data
		eeprom_write_byte((uint8_t*)0, data_addr >> 8);
		eeprom_write_byte((uint8_t*)1, data_addr	   );
		
		// DEBUG
		//UART_AddToQueue(buf, temp_len);
				
		for (int i=0; i < len; i++)
		{
			eeprom_write_byte((uint8_t*) (2+i+offset), data[i]);
		}
	}
}


void send_eeprom_data()
{
	long data_length;
	
	data_length = eeprom_read_byte((uint8_t*)0);
	data_length = data_length << 8;
	data_length = data_length + eeprom_read_byte((uint8_t*)1);
	
	if (data_length > 0)
	{
		char buf[32];
		int buf_counter = 0;
		char temp;
	
		for (int i=0; i < data_length; i++)
		{
				temp			 = eeprom_read_byte((uint8_t*) (i + 2));
				if (temp == NEW_LINE)
				{
					buf[buf_counter] = temp;
					buf_counter		 = buf_counter + 1;
					UART_AddToQueue(buf, buf_counter);
					buf_counter		 = 0;
				}
				else
				{
					buf[buf_counter] = temp;
					buf_counter		 = buf_counter + 1;	
				}
		}
		
		
		if (buf_counter != 0)
		{
			UART_AddToQueue(buf, buf_counter);
			buf_counter = 0;
		}
	}
	
	// Clear memory
	eeprom_write_byte((uint8_t*)0, 0);
	eeprom_write_byte((uint8_t*)1, 0);
}



int main(void)
{
    /* Replace with your application code */
	data_addr = 0;
	USART_INIT();
	sei();
    while (1) 
    {
		if ((uart0_flags.UART_FLAG == DATA_RECEIVED_FLAG) || (uart0_flags.UART_FLAG == DATA_REQUEST_FLAG)) 
		{
			
			// Data was received -> need to save it to the EEPROM			
			if (uart0_flags.UART_FLAG == DATA_REQUEST_FLAG)
			{
				send_eeprom_data();
			}
			
			else
			{
				save_eeprom_data();	
			}			
			uart_set_RX_buf_len();
			uart0_flags.UART_FLAG = DATA_OPERATION_COMPLETE_FLAG;
		}
    }
}
