#include "main_cpp.h"
#include "main.h"
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <array>

void Delay_us(uint32_t delay)
{
	__HAL_TIM_SET_COUNTER(&htim2, 0);
	while (__HAL_TIM_GET_COUNTER(&htim2) < delay) ;
}

void SetTempPinMode(bool input)
{
	GPIO_InitTypeDef GPIO_InitStruct
	{
		.Pin = TEMP_DATA_Pin,
		.Mode = input ? GPIO_MODE_INPUT : GPIO_MODE_OUTPUT_OD,
		.Pull = input ? GPIO_PULLUP : GPIO_NOPULL,
		.Speed = GPIO_SPEED_FREQ_LOW
	};
	HAL_GPIO_Init(TEMP_DATA_GPIO_Port, &GPIO_InitStruct);
}

bool ReadTempPin()
{
	return HAL_GPIO_ReadPin(TEMP_DATA_GPIO_Port, TEMP_DATA_Pin);
}

void WriteTempPin(bool state)
{
	HAL_GPIO_WritePin(TEMP_DATA_GPIO_Port, TEMP_DATA_Pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
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

uint32_t WaitForTempPinPulse(bool state)
{
	uint32_t start = __HAL_TIM_GET_COUNTER(&htim2);
	while (ReadTempPin() == state) ;
	return __HAL_TIM_GET_COUNTER(&htim2) - start;
}

struct RHT03Data
{
	uint16_t humidity;
	uint16_t temp;
	uint8_t checksum;
};

bool ReadTempData(float* humidity, float* temp)
{
	SetTempPinMode(true);
	if (!WaitForTempPin(true, 1000))
	{
		PrintLine("RHT03 is busy");
		return false;
	}

	// Send request for data
	SetTempPinMode(false);
	WriteTempPin(false);
	HAL_Delay(10);
	SetTempPinMode(true);
	Delay_us(40);
	// Wait for acknowledgement
	if (WaitForTempPinPulse(false) > 1000 || WaitForTempPinPulse(true) > 1000)
	//if (!WaitForTempPin(false, 1000) || !WaitForTempPin(true, 1000))	// This doesn't work. Data is off by 1 bit.
	{
		PrintLine("Failed to receive acknowledgement");
		return false;
	}

	// Read the data
	constexpr size_t num_bits = 40;
	std::array<uint16_t, num_bits> low_times, high_times;
	for (size_t i = 0; i < num_bits; ++i)
	{
		low_times[i] = WaitForTempPinPulse(false);
		high_times[i] = WaitForTempPinPulse(true);
	}

	RHT03Data data{};
	uint8_t bytes[sizeof(data)]{};
	for (size_t i = 0; i < num_bits; ++i)
	{
		//PrintLine("%2u, %3u, %3u", i, low_times[i], high_times[i]);
	    bytes[i / 8] <<= 1;
	    if (high_times[i] > 50)
	    {
	    	bytes[i / 8] |= 1;
	    }
	}
	data.humidity = bytes[0] << 8 | bytes[1];
	data.temp = bytes[2] << 8 | bytes[3];
	data.checksum = bytes[4];

	//PrintLine("humidity data: %u", data.humidity);
	//PrintLine("temp data: %u", data.temp);
	//PrintLine("checksum data: %u", data.checksum);

	uint8_t sum = bytes[0] + bytes[1] + bytes[2] + bytes[3];
	if (sum != data.checksum)
	{
		PrintLine("Checksum mismatch: %u != %u", sum, data.checksum);
		return false;
	}

	*humidity = data.humidity / 10.0;
	*temp = data.temp / 10.0;

	return true;
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
