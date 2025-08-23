#include "stm32f1xx_hal.h"
#include "tim.h"
#include "stm32f1xx_hal_tim.h"
#include "Motor.h"


void foreward ( void )
{
		HAL_GPIO_WritePin( GPIOA ,GPIO_PIN_4 , GPIO_PIN_SET );
		HAL_GPIO_WritePin( GPIOA ,GPIO_PIN_5 , GPIO_PIN_RESET );
}

void reversal ( void )
{
		HAL_GPIO_WritePin( GPIOA ,GPIO_PIN_4 , GPIO_PIN_RESET );
		HAL_GPIO_WritePin( GPIOA ,GPIO_PIN_5 , GPIO_PIN_SET );
}

void set_speed ( int speed )
{
		HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_3);
		__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, speed);
}

void turn_on_motor ( int direct , int speed)
{

		if( !direct )		//正转
		{
			foreward ();
		}	
		else						//反转
		{
			reversal (); 
		}
		
		set_speed ( speed );
}

void turn_off_motor ( void )
{
		HAL_TIM_PWM_Stop(&htim2,TIM_CHANNEL_3);	
}

