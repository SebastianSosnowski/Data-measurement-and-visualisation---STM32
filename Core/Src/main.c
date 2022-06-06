/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2022 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "fatfs.h"
#include "i2c.h"
#include "sdio.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
//#include "BMP280.h"
#include "My_library/BMP280.h"
#include "My_library/ILI9341.h"
#include "My_library/GFX.h"
#include "My_library/XPT2046.h"
#include "My_library/font_8x5.h"
#include "string.h"
#include "stdio.h"
#include "math.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

float T, P;

int pomiary_T[11] = { [ 0 ... 10 ] = -1 }, pomiary_P[11] = { [ 0 ... 10 ] = -1 }; //Data converted to pixels
float RAW_T[11] = { [ 0 ... 10 ] = -1 }, RAW_P[11] = { [ 0 ... 10 ] = -1 }; //Data from sensor

uint32_t Timer_BMP;

uint8_t licznik = 0;
uint8_t choice = 1; // 0 - temp is measured, 1 - pressure is measured
uint8_t save = 1; // to prevent bug after saving
uint16_t X, Y; //tp touch coordinates

	/*SD card */
FRESULT Result;
FATFS SD_FATFS;
FIL SD_Card_File;
uint8_t Size;
char data[60]; //buffor UART
	/*SD card */

	/* Buttons coordinates*/
int T_x = 300;
int T_y = 55;

int P_x = 300;
int P_y = 115;

int S_x = 300;
int S_y = 175;
	/* Buttons coordinates*/
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_NVIC_Init(void);
/* USER CODE BEGIN PFP */

void Measurements(void);
void Draw_Axes(void);
void Draw_Buttons(void);
void Save_to_SD(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  MX_DMA_Init();
  MX_SPI2_Init();
  MX_SDIO_SD_Init();
  MX_FATFS_Init();
  MX_SPI1_Init();

  /* Initialize interrupts */
  MX_NVIC_Init();
  /* USER CODE BEGIN 2 */

	BMP280_Init(&hi2c1);

	TFT_ILI9341_Init(&hspi1);

	XPT2046_Init(&hspi2, EXTI0_IRQn);

	GFX_SetFont(font_8x5);

	TFT_Clear_Screen(ILI9341_BLACK);

	Draw_Buttons();

	Draw_Axes(); //draw Coordinate system 0 - T, 1 - P

	Timer_BMP = HAL_GetTick();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while(1)
	{
		TP_Task();
		if((HAL_GetTick() - Timer_BMP) > 1000)
		{
			Timer_BMP = HAL_GetTick();
			Measurements();
		}

		if(TP_Is_Touched())
		{
			TP_Get_Point(&X, &Y);
			if((choice == 1) && ( ( (X > T_x-5) && (X < T_x + 20) ) && ( (Y > T_y-10) && (Y < T_y + 30) ) ) ) //Change axes from P to T
			{
				choice = 0;
				licznik = 0;
				save = 1;
				for(uint8_t i = 0; i < 11; i++)
				{
					pomiary_P[i] = -1;
					RAW_P[i] = -1;
				}
				TFT_Clear_Screen(ILI9341_BLACK);
				Draw_Buttons();
				Draw_Axes(); //draw Coordinate system 0 - T, 1 - P
			}
			else if((choice == 0) && (((X > P_x-5) && (X < P_x+ 20 )) && ((Y > P_y -10) && (Y < P_y + 30 )))) //Change axes from T to P
			{
				choice = 1;
				licznik = 0;
				save = 1;
				for(uint8_t i = 0; i < 11; i++)
				{
					pomiary_T[i] = -1;
					RAW_T[0] = -1;
				}
				TFT_Clear_Screen(ILI9341_BLACK);
				Draw_Buttons();
				Draw_Axes(); //draw Coordinate system 0 - T, 1 - P
			}
			else if((save == 1) && ( ((X > S_x-5) && (X < S_x+ 20 )) && ((Y > S_y -10) && (Y < S_y + 30 )) ) ) //Save to file
			{
				save = 0;
				Save_to_SD();
			}
		}
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 72;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 3;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief NVIC Configuration.
  * @retval None
  */
static void MX_NVIC_Init(void)
{
  /* EXTI0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);
}

/* USER CODE BEGIN 4 */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) //obsluga przerwania
{
	if(GPIO_Pin == TP_IRQ_Pin)
	{
		TP_IRQ();
	}
}

		/* Interface functions START */

static void Move_to_Left_RAW(float RAW[]) // e.g. value from RAW[2] goes to RAW[1] etc.
{
	for(uint8_t i = 1; i < 11; i++)
	{
		RAW[i - 1] = RAW[i];
	}
}

static void Move_to_Left(int pomiary[]) // e.g. value from pomiary[2] goes to RAW[1] etc.
{
	for(uint8_t i = 1; i < 11; i++) //clear whole chart from display
	{
		int x_wid = 22; // 23
		int img_x = 50;  // beginning of the coordinate system 46
		GFX_DrawLine(img_x + (i - 1) * x_wid, pomiary[i - 1], img_x + i * x_wid, pomiary[i], ILI9341_BLACK);
		GFX_DrawCircle(img_x + (i - 1) * x_wid, pomiary[i - 1], 3, ILI9341_BLACK);
		GFX_DrawCircle(img_x + i * x_wid, pomiary[i], 3, ILI9341_BLACK);
	}
	GFX_DrawFastVLine(50, 10, 220, ILI9341_BLUE);
	GFX_DrawFastHLine(50, 230, 220, ILI9341_BLUE);
	for(uint8_t i = 1; i < 11; i++)
	{
		pomiary[i - 1] = pomiary[i];
	}
}

static int Measurement_to_Pixel(float Measurement) //Converts measurement to corresponding number of pixels
{
	float Temp;

	switch(choice)
	{
	case 0:
		Temp = -5.5 * Measurement + 230; // Convert Temp value to pixels
		break;
	case 1:
		Temp = -4.4 * Measurement + 4498; // Convert Pressure value to pixels
	}

	if(Temp > 230) //value went below 0 deg
	{
		Temp = 230;
	}
	else if(Temp < 10) //value exceeded 40 deg
	{
		Temp = 10;
	}
	return (int) roundf(Temp); //return rounded value as int
}

static void Draw_Chart(int pomiary[]) //Draws whole plot
{
	for(uint8_t i = 1; i < 11; i++)
	{
		if(pomiary[i] != -1) //used at the beginning, -1 means that its not measurement
		{
			int x_wid = 22; // 23
			int img_x = 50;  // beginning of the coordinate system 46
			GFX_DrawLine(img_x + (i - 1) * x_wid, pomiary[i - 1], img_x + i * x_wid, pomiary[i], ILI9341_RED);
			GFX_DrawCircle(img_x + (i - 1) * x_wid, pomiary[i - 1], 3, ILI9341_WHITE);
			GFX_DrawCircle(img_x + i * x_wid, pomiary[i], 3, ILI9341_WHITE);
		}
		else
		{
			break;
		}
	}
}

void Save_to_SD(void) //Saves measurements to text files
{
	Result = f_mount(&SD_FATFS, "", 1);

	if(Result != FR_OK)
	{
		Size = sprintf(data, "Mount error \n\r");
		HAL_UART_Transmit(&huart2, (uint8_t*) data, Size, 1000);
	}
	else
	{
		Size = sprintf(data, "Mount correct  \n\r");
		HAL_UART_Transmit(&huart2, (uint8_t*) data, Size, 1000);

		if(choice == 0)
		{
			Result = f_open(&SD_Card_File, "pomiar_T.txt", FA_WRITE | FA_CREATE_ALWAYS);
		}
		else
		{
			Result = f_open(&SD_Card_File, "pomiar_P.txt", FA_WRITE | FA_CREATE_ALWAYS);
		}

		if(Result != FR_OK)
		{
			Size = sprintf(data, "File open error \n\r");
			HAL_UART_Transmit(&huart2, (uint8_t*) data, Size, 1000);
		}
		else
		{
			Size = sprintf(data, "File opened \n\r");
			HAL_UART_Transmit(&huart2, (uint8_t*) data, Size, 1000);

			if(choice == 0)
			{
				char str[10];
				for(uint8_t i = 0; i < 11; i++)
				{
					sprintf(str, "%.2f", RAW_T[i]);
					f_printf(&SD_Card_File, " %s \n", str);

				}
			}
			else
			{
				char str[10];
				for(uint8_t i = 0; i < 11; i++)
				{
					sprintf(str, "%.2f", RAW_P[i]);
					f_printf(&SD_Card_File, " %s \n", str);
				}
			}

			Result = f_close(&SD_Card_File);
			if(Result != FR_OK)
			{
				Size = sprintf(data, "File close error \n\r");
				HAL_UART_Transmit(&huart2, (uint8_t*) data, Size, 1000);
				GFX_DrawString(S_x, S_y, "S", ILI9341_GREEN);
				GFX_DrawRectangle(S_x - 5, S_y-10, 20, 30, ILI9341_RED);
				HAL_Delay(1000);
				GFX_DrawString(S_x, S_y, "S", ILI9341_WHITE);
				GFX_DrawRectangle(S_x - 5, S_y-10, 20, 30, ILI9341_WHITE);
			}
			else
			{
				Size = sprintf(data, "File closed \n\r");
				HAL_UART_Transmit(&huart2, (uint8_t*) data, Size, 1000);
				GFX_DrawString(S_x, S_y, "S", ILI9341_GREEN);
				GFX_DrawRectangle(S_x - 5, S_y-10, 20, 30, ILI9341_GREEN);
				HAL_Delay(1000);
				GFX_DrawString(S_x, S_y, "S", ILI9341_WHITE);
				GFX_DrawRectangle(S_x - 5, S_y-10, 20, 30, ILI9341_WHITE);
			}
		}

	}
}

void Draw_Buttons(void) //Draw used buttons
{
	switch(choice)
	{
	case 0:
		GFX_DrawString(T_x, T_y, "T", ILI9341_GREEN);
		GFX_DrawRectangle(T_x - 5, T_y-10, 20, 30, ILI9341_GREEN);
		GFX_DrawString(P_x, P_y, "P", ILI9341_WHITE);
		GFX_DrawRectangle(P_x - 5, P_y-10, 20, 30, ILI9341_WHITE);
		break;
	case 1:
		GFX_DrawString(T_x, T_y, "T", ILI9341_WHITE);
		GFX_DrawRectangle(T_x - 5, T_y-10, 20, 30, ILI9341_WHITE);
		GFX_DrawString(P_x, P_y, "P", ILI9341_GREEN);
		GFX_DrawRectangle(P_x - 5, P_y-10, 20, 30, ILI9341_GREEN);
		break;
	}
	GFX_DrawString(S_x, S_y, "S", ILI9341_WHITE);
	GFX_DrawRectangle(S_x - 5, S_y-10, 20, 30, ILI9341_WHITE);
}

void Draw_Axes(void) //Draw actually used coordinate system
{
	GFX_DrawFastVLine(50, 10, 220, ILI9341_BLUE);
	GFX_DrawFastHLine(50, 230, 220, ILI9341_BLUE);
	GFX_SetFontSize(1);

	if(choice == 0) // Axes for T
	{
		uint8_t Temp_axe[6] = { 0, 8, 16, 24, 32, 40 };
		char str[2];

		for(uint8_t i = 0; i < 11; i++) // x axis
		{
			sprintf(str, "%d", i);
			GFX_DrawString(48 + 22 * i, 232, str, ILI9341_BLUE);
		}
		for(uint8_t i = 0; i < 6; i++) // y axis
		{
			sprintf(str, "%d", Temp_axe[i]);
			GFX_DrawString(35, 226 - i * 44, str, ILI9341_BLUE);
		}
	}
	else if(choice == 1) // Axes for P
	{
		uint16_t Press_axe[6] = { 970, 980, 990, 1000, 1010, 1020 };
		char str[4];

		for(uint8_t i = 0; i < 11; i++) // x axis
		{
			sprintf(str, "%d", i);
			GFX_DrawString(48 + 22 * i, 232, str, ILI9341_BLUE);
		}
		for(uint8_t i = 0; i < 6; i++) // y axis
		{
			sprintf(str, "%d", Press_axe[i]);
			GFX_DrawString(25, 226 - i * 44, str, ILI9341_BLUE);
		}
	}
	GFX_SetFontSize(2);
}

void Measurements(void) // Manage whole process of measuring, processing and drawing
{
	BMP280_ReadPressureTemp(&P, &T);
	if(licznik >= 11)
	{
		switch(choice)
		{
		case 0:
			Move_to_Left(pomiary_T);
			Move_to_Left_RAW(RAW_T);
			break;
		case 1:
			Move_to_Left(pomiary_P);
			Move_to_Left_RAW(RAW_P);
			break;
		}
		licznik = 10;
	}
	switch(choice)
	{
	case 0:
		pomiary_T[licznik] = Measurement_to_Pixel(T);
		RAW_T[licznik] = T;
		Draw_Chart(pomiary_T);
		break;
	case 1:
		pomiary_P[licznik] = Measurement_to_Pixel(P);
		RAW_P[licznik] = P;
		Draw_Chart(pomiary_P);
		break;
	}
	licznik++;
}

		/* Interface functions END */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while(1)
	{
	}
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
