#ifndef CONSTANTS_H
#define CONSTANTS_H

#define MSG_STOB_BYTE             '&'


/////////////////////MSG FROM HOST TO MCU////////////////////////////////////
#define MSG_RX_ID_SET_T			  0x01
#define MSG_RX_ID_SET_PID_P	      0x02
#define MSG_RX_ID_SET_PID_I	      0x03
#define MSG_RX_ID_SET_PID_D	      0x04
#define MSG_RX_ID_PING	          0x05
/*
#define MSG_RX_ID_REQUEST_PID_P	  0x06
#define MSG_RX_ID_REQUEST_PID_I	  0x07
#define MSG_RX_ID_REQUEST_PID_D	  0x08
*/


/////////////////////MSG FROM MCU TO HOST////////////////////////////////////
#define MSG_TX_ID_TEMPERATURE 0x01
#define MSG_TX_ID_PID_P	      0x02
#define MSG_TX_ID_PID_I	      0x03
#define MSG_TX_ID_PID_D	      0x04
#define MSG_TX_ID_PING		  0x05
#define MSG_TX_ID_PID_OUT     0x06


volatile extern long target_temp;
volatile extern char need_to_recalculate_Temp;

#endif