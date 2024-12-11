#ifndef COMMON_H_
#define COMMON_H_

#include <avr/io.h>
#include <avr/interrupt.h>


// transforms long value to the buffer
void long_to_byte_array(long value, unsigned char *buff);

// add length and id bytes to the buffer
void prepare_and_send_data(int length, unsigned char *buffer, char msg_id);

#endif