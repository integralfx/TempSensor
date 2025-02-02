#include "Serial.hpp"
#include "stm32l4xx_hal.h"
#include <cstdarg>
#include <cstdio>
#include <cstring>

extern USART_HandleTypeDef husart2;

bool Print(const char* format, ...)
{
	char str[100]{};
	va_list args;
	va_start(args, format);
	int length = vsprintf(str, format, args);
	va_end(args);
	if (length < 0)
	{
		return false;
	}

	return HAL_USART_Transmit(&husart2, reinterpret_cast<uint8_t*>(str), length, 1000) == HAL_OK;
}

bool PrintLine(const char* format, ...)
{
	char str[100]{};
	va_list args;
	va_start(args, format);
	int length = vsprintf(str, format, args);
	va_end(args);
	if (length < 0)
	{
		return false;
	}
	str[length++] = '\r';
	str[length++] = '\n';

	return HAL_USART_Transmit(&husart2, reinterpret_cast<uint8_t*>(str), length, 1000) == HAL_OK;
}
