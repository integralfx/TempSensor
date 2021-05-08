#include "main_cpp.h"
#include "main.h"
#include <cstdio>
#include <cstdarg>
#include <array>
void SetTempPinMode(bool input)
{
	GPIO_InitTypeDef GPIO_InitStruct
	{
		.Pin = TEMP_DATA_Pin,
		.Mode = input ? GPIO_MODE_INPUT : GPIO_MODE_OUTPUT_PP,
		.Pull = input ? GPIO_PULLUP : GPIO_NOPULL,
		.Speed = GPIO_SPEED_FREQ_LOW
	};
	HAL_GPIO_Init(TEMP_DATA_GPIO_Port, &GPIO_InitStruct);
}

void Delay_us(uint32_t delay)
{
	__HAL_TIM_SET_COUNTER(&htim2, 0);
	while (__HAL_TIM_GET_COUNTER(&htim2) < delay) ;
}

bool WaitForTempPin(bool state, uint32_t timeout_us)
{
	uint32_t start = __HAL_TIM_GET_COUNTER(&htim2);
	while (true)
	{
		if (ReadTempPin() == state)
		{
			return true;
		}

		if (__HAL_TIM_GET_COUNTER(&htim2) - start > timeout_us)
		{
			return false;
		}
	}
}

union RHT03Data
{
	struct
	{
		uint16_t humidity;
		uint16_t temp;
		uint8_t checksum;
	};
	uint8_t bytes[5];
};

void ReadTempData()
{
	SetTempPinMode(true);
	if (!WaitForTempPin(true, 1000))
	{
		PrintLine("Fail 0");
		return;
	}

	SetTempPinMode(false);
	WriteTempPin(false);
	HAL_Delay(10);
	SetTempPinMode(true);
	Delay_us(40);
	if (!WaitForTempPin(false, 1000) || !WaitForTempPin(true, 1000))
	{
		PrintLine("Fail 1");
		return;
	}

	// Start data transmission
	std::array<uint8_t, 40> start;
	std::array<uint8_t, start.size() + 1> stop;
	for (size_t i = 0; i < start.size(); ++i)
	{
		if (!WaitForTempPin(false, 1000))
		{
			PrintLine("Fail 2: %u", i);
			return;
		}
		start[i] = __HAL_TIM_GET_COUNTER(&htim2);
		if (!WaitForTempPin(true, 1000))
		{
			PrintLine("Fail 3: %u", i);
			return;
		}
		stop[i] = __HAL_TIM_GET_COUNTER(&htim2);
	}
	if (!WaitForTempPin(false, 1000))
	{
		PrintLine("Fail 4");
		return;
	}
	stop[40] = __HAL_TIM_GET_COUNTER(&htim2);

	RHT03Data data{};
	for (size_t i = 0; i < start.size(); ++i)
	{
		uint8_t low_time = stop[i] = start[i];
		uint8_t high_time = start[i + 1] - stop[i];
		PrintLine("%2u, low time: %3u, high time: %3u, %u", i, low_time, high_time, high_time > low_time);
		if (high_time > low_time)
		{
			data.bytes[i/8] |= 1 << (7 - i%8);
		}
	}

	PrintLine("humidity data: %u", data.humidity);
	PrintLine("temp data: %u", data.temp);
	PrintLine("checksum data: %u", data.checksum);

	uint8_t sum = data.bytes[0] + data.bytes[1] + data.bytes[2] + data.bytes[3];
	if (sum != data.checksum)
	{
		PrintLine("Checksum mismatch: %u != %u", sum, data.checksum);
		return;
	}

	PrintLine("Humidity: %.2f", data.humidity / 10.0);
	PrintLine("Temp: %.2f", data.temp / 10.0);
}

void WriteTempPin(bool state)
{
	HAL_GPIO_WritePin(TEMP_DATA_GPIO_Port, TEMP_DATA_Pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
bool ReadTempPin()
{
	return HAL_GPIO_ReadPin(TEMP_DATA_GPIO_Port, TEMP_DATA_Pin);
}

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
