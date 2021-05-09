#include "main_cpp.h"
#include "main.h"
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <array>
#include <utility>
#include <algorithm>

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
	uint32_t start = Timer_us();
	while (true)
	{
		if (ReadTempPin() == state)
		{
			return true;
		}

		if (Timer_us() - start > timeout_us)
		{
			return false;
		}
	}
}

uint32_t WaitForTempPinPulse(bool state)
{
	uint32_t start = Timer_us();
	while (ReadTempPin() == state) ;
	return Timer_us() - start;
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

void LCD_SetRS(bool rs)
{
	HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, rs ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void LCD_SetEnable(bool enable)
{
	HAL_GPIO_WritePin(LCD_E_GPIO_Port, LCD_E_Pin, enable ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void LCD_SetData(uint8_t data)
{
	static std::array<std::pair<GPIO_TypeDef*, uint16_t>, 8> LCD_data_pins
	{ {
		{ LCD_D0_GPIO_Port, LCD_D0_Pin },
		{ LCD_D1_GPIO_Port, LCD_D1_Pin },
		{ LCD_D2_GPIO_Port, LCD_D2_Pin },
		{ LCD_D3_GPIO_Port, LCD_D3_Pin },
		{ LCD_D4_GPIO_Port, LCD_D4_Pin },
		{ LCD_D5_GPIO_Port, LCD_D5_Pin },
		{ LCD_D6_GPIO_Port, LCD_D6_Pin },
		{ LCD_D7_GPIO_Port, LCD_D7_Pin }
	} };

	for (size_t i = 0; i < LCD_data_pins.size(); ++i)
	{
		auto& pin = LCD_data_pins[i];
		HAL_GPIO_WritePin(pin.first, pin.second, static_cast<GPIO_PinState>((data >> i) & 0x1));
	}
}

void LCD_SendData(bool rs, uint8_t data)
{
	LCD_SetRS(rs);
	Delay_100ns(1);
	LCD_SetEnable(true);
	LCD_SetData(data);
	Delay_100ns(2);	// Min tPW is 150ns.
	LCD_SetEnable(false);
}

void LCD_Clear()
{
	LCD_SendData(false, 1);
}

void LCD_ReturnHome()
{
	LCD_SendData(false, 0b10);
}

void LCD_SetControl(bool display_on, bool cursor_on, bool cursor_blink)
{
	LCD_SendData(false, 0b1000 | (display_on << 2) | (cursor_on << 1) | cursor_blink);
}

void LCD_SetFunction(bool data_8bit, bool two_lines, bool font_10dots)
{
	LCD_SendData(false, 0b100000 | (data_8bit << 4) | (two_lines << 3) | (font_10dots << 2));
}

void LCD_MoveScreen(bool right)
{
	//LCD_SendData(false, 0b11000 | (right << 2));
	LCD_SendData(false, right ? 0b11100 : 0b11000);
}

void LCD_SetCursor(int row, int col, bool two_lines)
{
	row = std::min(row, two_lines ? 1 : 0);
	col = std::min(col, two_lines ? 39 : 79);

	uint8_t address = two_lines ? row * 40 + col : row + col;
	LCD_SendData(false, 0b10000000 | address);
}

void LCD_WriteChar(char c)
{
	LCD_SendData(true, static_cast<uint8_t>(c));
}

void LCD_WriteStr(const char* str)
{
	while (*str)
	{
		LCD_WriteChar(*str++);
		Delay_us(53);
	}
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
