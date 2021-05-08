#include "main_cpp.h"
#include "main.h"
#include <cstdio>
#include <cstdarg>
#include <cstring>
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

struct RHT03Data
{
	uint16_t humidity;
	uint16_t temp;
	uint8_t checksum;
} __attribute__((packed));

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

	constexpr size_t num_bits = 40;
	std::array<uint8_t, num_bits> low_times, high_times;
	auto wait_for_pulse = [](bool state)
	{
		uint32_t start = __HAL_TIM_GET_COUNTER(&htim2);
		while (ReadTempPin() == state) ;
		return __HAL_TIM_GET_COUNTER(&htim2) - start;
	};
	for (size_t i = 0; i < num_bits; ++i)
	{
		low_times[i] = wait_for_pulse(false);
		high_times[i] = wait_for_pulse(true);
	}

	RHT03Data data{};
	uint8_t bytes[sizeof(data)]{};
	for (size_t i = 0; i < num_bits; ++i)
	{
		PrintLine("%2u, %3u, %3u", i, low_times[i], high_times[i]);
	    bytes[i / 8] <<= 1;
	    if (high_times[i] > low_times[i])
	    {
	    	bytes[i / 8] |= 1;
	    }
	}
	//memcpy(&data, bytes, sizeof(data));
	data.humidity = bytes[0] << 8 | bytes[1];
	data.temp = bytes[2] << 8 | bytes[3];
	data.checksum = bytes[4];

	PrintLine("humidity data: 0x%02X", data.humidity);
	PrintLine("temp data: 0x%02X", data.temp);
	PrintLine("checksum data: 0x%02X", data.checksum);

	uint8_t sum = bytes[0] + bytes[1] + bytes[2] + bytes[3];
	if (sum != data.checksum)
	{
		PrintLine("Checksum mismatch: %u != %u", sum, data.checksum);
		return;
	}

	PrintLine("Humidity: %f", data.humidity / 10.0);
	PrintLine("Temp: %f", data.temp / 10.0);
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
