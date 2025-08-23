#ifndef __BH1750_H
#define __BH1750_H

#include <stdint.h>

void BH1750_WriteReg(uint8_t RegAddress);
uint16_t BH1750_ReadReg(void);
void BH1750_Init(void);
float BH1750_ReadLight(void);

#endif 

