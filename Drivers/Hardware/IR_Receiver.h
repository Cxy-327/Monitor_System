#ifndef __IR_RECEIVER_H
#define __IR_RECEIVER_H

#include <stdint.h>

void IRReceiver_Init(void);
int IRReceiver_Read(uint8_t *pDev, uint8_t *pData);
const char *IRReceiver_CodeToString(uint8_t code);
//void IRReceiver_Test(void);
int  IRReceiver_SetVal (int * Thresholds_Val );


#endif
