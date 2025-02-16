#include "stm32l4xx_hal.h"
#include "Pins.hpp"
#include "LCD_TC1602A.hpp"
#include "Serial.hpp"
#include "Time.hpp"
#include <array>
#include <cstdio>

TIM_HandleTypeDef htim2;
USART_HandleTypeDef husart2;

static void SystemClock_Config();
static void MX_GPIO_Init();
static void MX_TIM2_Init();
static void MX_USART2_Init();

static void Error_Handler(const char* file, int line);

static void SetTempPinMode(bool input);
static bool ReadTempPin();
static void WriteTempPin(bool state);
static bool WaitForTempPin(bool state, uint32_t timeout_us);
static uint32_t WaitForTempPinPulse(bool state);
static bool ReadTempData(float* humidity, float* temp);

int main()
{
	HAL_Init();

	SystemClock_Config();

	MX_GPIO_Init();
	MX_TIM2_Init();
	MX_USART2_Init();

	HAL_TIM_Base_Start(&htim2);

	LCD_TC1602A lcd_tc1602a;
	LCD lcd{ lcd_tc1602a };

	LCDInit lcd_init
	{
		.data_size = LCDInit::DataSize::EightBits,
		.row_count = LCDInit::Rows::Two,
		.font_type = LCDInit::FontType::FiveByTenDots,
		.column_count = 16
	};
	lcd.Init(lcd_init);
	LCDSettings lcd_settings
	{
		.display_on = true,
		.cursor_on = false,
		.cursor_blink = false
	};
	lcd.SetSettings(lcd_settings);

	static constexpr uint32_t update_interval_ms = 2000;
	uint32_t last_temp_update = HAL_GetTick();
	std::array<uint8_t, 32> buffer;
	auto print_lcd_data = [&](uint8_t row, size_t max_bytes_to_read)
	{
		if (!lcd.SetCursor(row, 0))
		{
			Error_Handler(__FILE__, __LINE__);
		}

		auto bytes_read = lcd.Read(std::span<uint8_t>{ buffer }.subspan(0, max_bytes_to_read));
		buffer[bytes_read] = 0;

		PrintLine("Row %d: %s", row, buffer.data());
	};
	while (1)
	{
		uint32_t now = HAL_GetTick();
		if (now - last_temp_update > update_interval_ms)
		{
			float humidity = 0;
			float temp = 0;
			if (ReadTempData(&humidity, &temp))
			{
				if (!lcd.SetCursor(0, 0))
				{
					Error_Handler(__FILE__, __LINE__);
				}

				{
					auto length = sprintf(reinterpret_cast<char*>(buffer.data()), "Humidity : %.1f%%", humidity);
					auto bytes_written = lcd.Write({ buffer.begin(), buffer.begin() + length });
					if (bytes_written != static_cast<size_t>(length))
					{
						PrintLine("Wrote %d bytes. Expected %d bytes.", bytes_written, length);
					}

					print_lcd_data(0, bytes_written);
				}

				if (!lcd.SetCursor(1, 0))
				{
					Error_Handler(__FILE__, __LINE__);
				}

				{
					auto length = sprintf(reinterpret_cast<char*>(buffer.data()), "Temp     : %.1fC", temp);
					auto bytes_written = lcd.Write({ buffer.begin(), buffer.begin() + length });
					if (bytes_written != static_cast<size_t>(length))
					{
						PrintLine("Wrote %d bytes. Expected %d bytes.", bytes_written, length);
					}

					print_lcd_data(1, bytes_written);
				}

				// PrintLine(
				// 	"Humidity    : %.1f%%\r\n"
				// 	"Temperature : %.1f°C\r\n",
				// 	humidity, temp
				// );
			}

			last_temp_update = now;
		}

		HAL_Delay(1);
	}
}

static void SystemClock_Config(void)
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
		Error_Handler(__FILE__, __LINE__);
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
		Error_Handler(__FILE__, __LINE__);
	}
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2;
	PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);
	}
	/** Configure the main internal regulator output voltage
	 */
	if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);
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
		Error_Handler(__FILE__, __LINE__);
	}
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
	{
		Error_Handler(__FILE__, __LINE__);
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
		Error_Handler(__FILE__, __LINE__);
	}
}

static void MX_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	HAL_GPIO_WritePin(GPIOC, LCD_D4_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB, LCD_D1_Pin|LCD_E_Pin|LCD_D0_Pin|LCD_D5_Pin|LCD_RW_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOA, LCD_RS_Pin|LCD_D3_Pin|LED_Pin|LCD_D7_Pin|LCD_D2_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin : BTN_Pin */
	GPIO_InitStruct.Pin = BTN_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(BTN_GPIO_Port, &GPIO_InitStruct);
	//
	GPIO_InitStruct.Pin = LCD_D4_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = LCD_D1_Pin|LCD_E_Pin|LCD_D0_Pin|LCD_D5_Pin|LCD_RW_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pin : TEMP_DATA_Pin */
	GPIO_InitStruct.Pin = TEMP_DATA_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(TEMP_DATA_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = LCD_RS_Pin|LCD_D3_Pin|LED_Pin|LCD_D7_Pin|LCD_D2_Pin|LCD_D6_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

static void Error_Handler(const char* file, int line)
{
	PrintLine("%s:%d", file, line);

	__disable_irq();
	while (1)
	{
	}
}

static void SetTempPinMode(bool input)
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

static bool ReadTempPin()
{
	return HAL_GPIO_ReadPin(TEMP_DATA_GPIO_Port, TEMP_DATA_Pin);
}

static void WriteTempPin(bool state)
{
	HAL_GPIO_WritePin(TEMP_DATA_GPIO_Port, TEMP_DATA_Pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static bool WaitForTempPin(bool state, uint32_t timeout_us)
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

static uint32_t WaitForTempPinPulse(bool state)
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

static bool ReadTempData(float* humidity, float* temp)
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