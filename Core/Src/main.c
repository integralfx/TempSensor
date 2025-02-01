#include "main.h"
#include "main_cpp.h"
#include <stdio.h>

TIM_HandleTypeDef htim2;
USART_HandleTypeDef husart2;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART2_Init(void);

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
