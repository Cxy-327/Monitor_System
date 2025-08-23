#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "Delay.h"
#include "stm32f1xx_hal.h"

void Buzzer_200ms (void )
{
		HAL_GPIO_WritePin (GPIOA ,GPIO_PIN_8 ,GPIO_PIN_RESET);
		osDelay(200);
		HAL_GPIO_WritePin (GPIOA ,GPIO_PIN_8 ,GPIO_PIN_SET);
}	

