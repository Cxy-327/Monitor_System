#ifndef __DELAY_H
#define __DELAY_H

#include <stdint.h>

void udelay(int us);
void mdelay(int ms);
uint64_t system_get_ns(void);

#endif 

