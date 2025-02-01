#include "stm32l4xx_hal.h"
#include "Pins.hpp"
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <array>
#include <utility>
#include <algorithm>

TIM_HandleTypeDef htim2;
USART_HandleTypeDef husart2;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART2_Init(void);

void Error_Handler(void);

uint32_t Timer_100ns();
uint32_t Timer_us();
void Delay_100ns(uint32_t multiplier);
void Delay_us(uint32_t delay);

void SetTempPinMode(bool input);
bool ReadTempPin();
void WriteTempPin(bool state);
bool WaitForTempPin(bool state, uint32_t timeout_us);
uint32_t WaitForTempPinPulse(bool state);
bool ReadTempData(float* humidity, float* temp);

void LCD_Clear();
void LCD_ReturnHome();
void LCD_SetControl(bool display_on, bool cursor_on, bool cursor_blink);
void LCD_SetFunction(bool data_8bit, bool two_lines, bool font_10dots);
void LCD_MoveScreen(bool right);
void LCD_SetCursor(int row, int col, bool two_lines);
void LCD_WriteChar(char c);
void LCD_WriteStr(const char* str);

bool Print(const char* format, ...);
bool PrintLine(const char* format, ...);

int main(void)
{
	HAL_Init();

	SystemClock_Config();

	MX_GPIO_Init();
	MX_TIM2_Init();
	MX_USART2_Init();

	HAL_TIM_Base_Start(&htim2);

	LCD_SetFunction(true, true, true);
	Delay_us(53);
	LCD_SetControl(true, false, false);
	Delay_us(53);
	LCD_Clear();
	HAL_Delay(2);
	LCD_ReturnHome();
	HAL_Delay(2);

	/* Infinite loop */
	uint32_t last_temp_update = HAL_GetTick();
	char buffer[16+1] = { 0 };
	while (1)
	{
		uint32_t now = HAL_GetTick();
		if (now - last_temp_update > 2000)
		{
			float humidity, temp;
			if (ReadTempData(&humidity, &temp))
			{
				sprintf(buffer, "Humidity : %.1f%%", humidity);
				LCD_WriteStr(buffer);
				sprintf(buffer, "Temp     : %.1fC", temp);
				LCD_SetCursor(1, 0, true);
				Delay_us(53);
				LCD_WriteStr(buffer);
				LCD_SetCursor(0, 0, true);
//				PrintLine(
//					"Humidity    : %.1f%%\r\n"
//					"Temperature : %.1f°C\r\n",
//					humidity, temp
//				);

			}

			last_temp_update = now;
		}

		HAL_Delay(1);
	}
}

void SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
	RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
	RCC_OscInitStruct.MSIState = RCC_MSI_ON;
	RCC_OscInitStruct.MSICalibrationValue = 0;
	RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
	RCC_OscInitStruct.PLL.PLLM = 1;
	RCC_OscInitStruct.PLL.PLLN = 40;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
	RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
	RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}
	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
			|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
	{
		Error_Handler();
	}
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2;
	PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
	{
		Error_Handler();
	}
	/** Configure the main internal regulator output voltage
	 */
	if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
	{
		Error_Handler();
	}
}

static void MX_TIM2_Init(void)
{
	TIM_ClockConfigTypeDef sClockSourceConfig = {0};
	TIM_MasterConfigTypeDef sMasterConfig = {0};

	htim2.Instance = TIM2;
	htim2.Init.Prescaler = 8-1;
	htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim2.Init.Period = 4294967295;
	htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
	{
		Error_Handler();
	}
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
	{
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
	{
		Error_Handler();
	}
}

static void MX_USART2_Init(void)
{
	husart2.Instance = USART2;
	husart2.Init.BaudRate = 115200;
	husart2.Init.WordLength = USART_WORDLENGTH_8B;
	husart2.Init.StopBits = USART_STOPBITS_1;
	husart2.Init.Parity = USART_PARITY_NONE;
	husart2.Init.Mode = USART_MODE_TX_RX;
	husart2.Init.CLKPolarity = USART_POLARITY_LOW;
	husart2.Init.CLKPhase = USART_PHASE_1EDGE;
	husart2.Init.CLKLastBit = USART_LASTBIT_DISABLE;
	if (HAL_USART_Init(&husart2) != HAL_OK)
	{
		Error_Handler();
	}
}

static void MX_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB, LCD_D1_Pin|LCD_E_Pin|LCD_D0_Pin|LCD_D6_Pin
			|LCD_D2_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOA, LCD_RS_Pin|LCD_D4_Pin|LCD_D3_Pin|LED_Pin
			|LCD_D7_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(LCD_D5_GPIO_Port, LCD_D5_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin : BTN_Pin */
	GPIO_InitStruct.Pin = BTN_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(BTN_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : LCD_D1_Pin LCD_E_Pin LCD_D0_Pin LCD_D6_Pin
                           LCD_D2_Pin */
	GPIO_InitStruct.Pin = LCD_D1_Pin|LCD_E_Pin|LCD_D0_Pin|LCD_D6_Pin
			|LCD_D2_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pin : TEMP_DATA_Pin */
	GPIO_InitStruct.Pin = TEMP_DATA_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(TEMP_DATA_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : LCD_RS_Pin LCD_D4_Pin LCD_D3_Pin LED_Pin
                           LCD_D7_Pin */
	GPIO_InitStruct.Pin = LCD_RS_Pin|LCD_D4_Pin|LCD_D3_Pin|LED_Pin
			|LCD_D7_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/*Configure GPIO pin : LCD_D5_Pin */
	GPIO_InitStruct.Pin = LCD_D5_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(LCD_D5_GPIO_Port, &GPIO_InitStruct);

}

void Error_Handler(void)
{
	__disable_irq();
	while (1)
	{
	}
}

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
