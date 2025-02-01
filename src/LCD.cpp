#include "LCD.hpp"
#include "Pins.hpp"
#include "Time.hpp"
#include "stm32l4xx_hal.h"
#include <array>
#include <utility>

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