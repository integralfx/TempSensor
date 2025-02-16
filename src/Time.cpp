#include "Time.hpp"
#include "stm32l4xx_hal.h"

extern TIM_HandleTypeDef htim2;

uint32_t Timer_100ns()
{
	 return __HAL_TIM_GET_COUNTER(&htim2);
}

uint32_t Timer_us()
{
	return Timer_100ns() / 10;
}

void Delay_100ns(uint32_t multiplier)
{
	// Timer has a frequency of 10MHz (100ns).
	__HAL_TIM_SET_COUNTER(&htim2, 0);
	while (Timer_100ns() < multiplier) ;
}

void Delay_us(uint32_t delay)
{
	// 1us = 1000ns, but we can only delay in increments of 100ns,
	// hence why we multiply by 10.
	Delay_100ns(delay * 10);
}