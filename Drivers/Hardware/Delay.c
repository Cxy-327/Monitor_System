#include "Delay.h"
#include "stm32f1xx_hal.h"

/******us级别的延时函数******/
void udelay(int us)
{
    extern TIM_HandleTypeDef        htim4;
    TIM_HandleTypeDef *hHalTim = &htim4;

    uint32_t ticks;
    uint32_t told, tnow, tcnt = 0;
    uint32_t reload = __HAL_TIM_GET_AUTORELOAD(hHalTim);

    ticks = us * reload / (1000);  /* 假设reload对应1ms */
    told = __HAL_TIM_GET_COUNTER(hHalTim);
    while (1)
    {
        tnow = __HAL_TIM_GET_COUNTER(hHalTim);
        if (tnow != told)
        {
            if (tnow > told)
            {
                tcnt += tnow - told;
            }
            else
            {
                tcnt += reload - told + tnow;
            }
            told = tnow;
            if (tcnt >= ticks)
            {
                break;
            }
        }
    }
}



/******ms级别的延时函数******/
void mdelay(int ms)
{
    for (int i = 0; i < ms; i++)
        udelay(1000);
}


/******获得系统时间******/
uint64_t system_get_ns(void)
{
    //extern uint32_t HAL_GetTick(void);
    extern TIM_HandleTypeDef        htim4;
    TIM_HandleTypeDef *hHalTim = &htim4;
    
    uint64_t ns = HAL_GetTick();
    uint64_t cnt;
    uint64_t reload;

    cnt = __HAL_TIM_GET_COUNTER(hHalTim);
    reload = __HAL_TIM_GET_AUTORELOAD(hHalTim);

    ns *= 1000000;
    ns += cnt * 1000000 / reload;
    return ns;
}

