/*
 * XPT2046.c
 *
 */

#include "main.h"
#include "My_library/ILI9341.h"
#include "My_library/XPT2046.h"

static SPI_HandleTypeDef *TP_SPI;
static IRQn_Type TP_IRQn;

static uint8_t ChannelSettingsX, ChannelSettingsY; // XPT2046 control bytes for channels
static uint8_t SendBuffer[5]; // Send buffer with control bytes packed
static uint8_t ReceiveBuffer[5]; // Receive buffer with XPT2046 conversions
static uint8_t CalibrationMode = 0;

/*
uint16_t calA[] = { 20, 25 };             //calibration points must be independent, i.e. not line up
uint16_t calB[] = { 160, 220 };
uint16_t calC[] = { 300, 110 };

uint16_t calA_raw[] = { 0, 0 };           //store raw touch data for calibration points
uint16_t calB_raw[] = { 0, 0 };
uint16_t calC_raw[] = { 0, 0 };
*/

uint16_t TouchSamples[2];

typedef struct
{
	long double A_x;
	long double B_x;
	long double D_x;
	long double A_y;
	long double B_y;
	long double D_y;
} CalibData_t;

typedef enum
{
	TP_IDLE, 		//0
	TP_TOUCHED,		//1
	TP_RELEASED		//2
} TP_State;

volatile TP_State TouchState;

#if (TOUCH_ROTATION == 0)
CalibData_t CalibrationData = {-.0009337, -.0636839, 250.342, -.0889775, -.00118110, 356.538}; // default calibration data
#endif
#if (TOUCH_ROTATION == 1)
CalibData_t CalibrationData = { -.0885542, .0016532, 349.800, .0007309, .06543699, -15.290 }; // default calibration data
#endif
#if (TOUCH_ROTATION == 2)
CalibData_t CalibrationData = {.0006100, .0647828, -13.634, .0890609, .0001381, -35.73}; // default calibration data
#endif
#if (TOUCH_ROTATION == 3)
CalibData_t CalibrationData = {.0902216, .0006510, -38.657, -.0010005, -.0667030, 258.08}; // default calibration data
#endif

void TP_Task(void)
{
	switch(TouchState)
	{
	case TP_IDLE:
		break;
	case TP_TOUCHED:
		if(HAL_GPIO_ReadPin(TP_IRQ_GPIO_Port, TP_IRQ_Pin) == GPIO_PIN_RESET)
		{
			HAL_SPI_TransmitReceive(TP_SPI, SendBuffer, ReceiveBuffer, 5, 1000); //Sendbuffer - CHN config, ReciveBuff - Raw data from ADC
			TP_ADC_to_Pixels(&TouchSamples[0], &TouchSamples[1]);
			TouchState = TP_RELEASED;
		}
		else
		{
			TouchState = TP_RELEASED;
		}
		break;
	case TP_RELEASED:
		TouchState = TP_IDLE;
		while(HAL_NVIC_GetPendingIRQ(TP_IRQn)) //czyszczenie wszystkich flag zwiazanych z tym przerwaniem (2 konkretnie)
		{
			__HAL_GPIO_EXTI_CLEAR_IT(TP_IRQ_Pin); //flaga od pinu
			HAL_NVIC_ClearPendingIRQ(TP_IRQn);
		}
		HAL_NVIC_EnableIRQ(EXTI0_IRQn);
		break;
	}
}

void TP_IRQ(void)
{
	HAL_NVIC_DisableIRQ(EXTI0_IRQn);
	TouchState = TP_TOUCHED;
}

		/* Calibration */
/*
void Calculate_Calibration_Parameters(void)
{
	//calculate calibration parameters
	//Kalibracja 3 punktowa
	//Na podstawie znanych punktow, ktore mialy byc wcisniete, porownujemy to co odczytal panel po wcisnieciu w tych punktach

	int32_t delta = (calA_raw[0] - calC_raw[0]) * (calB_raw[1] - calC_raw[1]) - (calB_raw[0] - calC_raw[0]) * (calA_raw[1] - calC_raw[1]);

	CalibrationData.A_x = (float) ((calA[0] - calC[0]) * (calB_raw[1] - calC_raw[1]) - (calB[0] - calC[0]) * (calA_raw[1] - calC_raw[1])) / delta;

	CalibrationData.B_x = (float) ((calA_raw[0] - calC_raw[0]) * (calB[0] - calC[0]) - (calB_raw[0] - calC_raw[0]) * (calA[0] - calC[0])) / delta;

	CalibrationData.D_x = ((uint64_t) calA[0] * (calB_raw[0] * calC_raw[1] - calC_raw[0] * calB_raw[1]) - (uint64_t) calB[0] * (calA_raw[0] * calC_raw[1] - calC_raw[0] * calA_raw[1])
			+ (uint64_t) calC[0] * (calA_raw[0] * calB_raw[1] - calB_raw[0] * calA_raw[1])) / delta;

	CalibrationData.A_y = (float) ((calA[1] - calC[1]) * (calB_raw[1] - calC_raw[1]) - (calB[1] - calC[1]) * (calA_raw[1] - calC_raw[1])) / delta;

	CalibrationData.B_y = (float) ((calA_raw[0] - calC_raw[0]) * (calB[1] - calC[1]) - (calB_raw[0] - calC_raw[0]) * (calA[1] - calC[1])) / delta;

	CalibrationData.D_y = ((uint64_t) calA[1] * (calB_raw[0] * calC_raw[1] - calC_raw[0] * calB_raw[1]) - (uint64_t) calB[1] * (calA_raw[0] * calC_raw[1] - calC_raw[0] * calA_raw[1])
			+ (uint64_t) calC[1] * (calA_raw[0] * calB_raw[1] - calB_raw[0] * calA_raw[1])) / delta;
}

void CalibrationPoint(uint16_t calX, uint16_t calY)
{
  GFX_DrawCircle(calX, calY, 6, ILI9341_WHITE);
  GFX_DrawLine(calX-4, calY, calX+4, calY, ILI9341_WHITE);
  GFX_DrawLine(calX, calY-4, calX, calY+4, ILI9341_WHITE);
}


void DoCalibration(void)
{
	uint8_t calCount = 0;
	TFT_Clear_Screen(ILI9341_BLACK); // Clear screen for black
	CalibrationMode = 1; // Set Calibration Mode

	while(calCount < 4) // GEt all points and calculate
	{
		TP_Task(); // We have to read touch points

		switch(calCount)
		{
		case 0: // 1st point
			CalibrationPoint(calA[0], calA[1]);
			if(TouchState == TP_TOUCHED)
			{
				TP_Get_Point(&calA_raw[0], &calA_raw[1]);
			}
			if(TouchState == TP_RELEASED)
			{
				HAL_Delay(200);
				calCount++;
			}
			break;
		case 1: // 2nd point
			GFX_DrawFillRectangle(calA[0]-6, calA[1]-6, 13, 13, ILI9341_BLACK);

			CalibrationPoint(calB[0], calB[1]);
			if(TouchState == TP_TOUCHED)
			{
				TP_Get_Point(&calB_raw[0], &calB_raw[1]);
			}
			if(TouchState == TP_RELEASED)
			{
				HAL_Delay(200);
				calCount++;
			}
			break;
		case 2: // 3rd point
			GFX_DrawFillRectangle(calB[0]-6, calB[1]-6, 13, 13, ILI9341_BLACK);

			CalibrationPoint(calC[0], calC[1]);
			if(TouchState == TP_TOUCHED)
			{
				TP_Get_Point(&calC_raw[0], &calC_raw[1]);
			}
			if(TouchState == TP_RELEASED)
			{
				HAL_Delay(200);
				calCount++;
			}
			break;
		case 3: // calculate and save calibration data,
			GFX_DrawFillRectangle(calC[0]-6, calC[1]-6, 13, 13, ILI9341_BLACK);

			Calculate_Calibration_Parameters();
			calCount++;
			break;
		}
	}
	CalibrationMode = 0; // Disable Calibration mdoe
}

*/
		/* Calibration */

void TP_ADC_to_Pixels(uint16_t *Xs, uint16_t *Ys)
{
	uint16_t Xp, Yp;
	//RAW ADC data
	Xp = (uint16_t) ((ReceiveBuffer[1] << 8) | (ReceiveBuffer[2]));
	Yp = (uint16_t) ((ReceiveBuffer[3] << 8) | (ReceiveBuffer[4]));

	if(CalibrationMode == 0)
	{
		*Xs = CalibrationData.A_x * Xp + CalibrationData.B_x * Yp + CalibrationData.D_x;
		*Ys = CalibrationData.A_y * Xp + CalibrationData.B_y * Yp + CalibrationData.D_y;
	}
	else;
}

uint8_t TP_Is_Touched(void)
{
	if(TouchState == TP_TOUCHED)
		return 1;
	return 0;
}

void TP_Get_Point(uint16_t *X, uint16_t *Y)
{
	*X = TouchSamples[0];
	*Y = TouchSamples[1];
}

void XPT2046_Init(SPI_HandleTypeDef *SPI, IRQn_Type TouchPanel_IRQn) // IRQn type - hardware interrupt number
{
	TP_SPI = SPI;
	TP_IRQn = TouchPanel_IRQn;
	TouchState = TP_IDLE;

	//     (     X    )           (     Y 	 )
	// (000 10010)(000 00000) (000 11010)(000 00000) (00000000)
	//	SendBuffer
	// (    0    )(    1    ) (    2    )(    3    ) (    4   )

	ChannelSettingsX = 0b10010000;
	ChannelSettingsY = 0b11010000;

	SendBuffer[0] = 0x80; //clear settings in controller
	HAL_SPI_TransmitReceive(TP_SPI, SendBuffer, ReceiveBuffer, 5, 1000); //Sendbuffer - CHN config, ReciveBuff - Raw data from ADC
	HAL_Delay(1);

	SendBuffer[0] = (ChannelSettingsX >> 3);
	SendBuffer[1] = (ChannelSettingsX << 5);
	SendBuffer[2] = (ChannelSettingsY >> 3);
	SendBuffer[3] = (ChannelSettingsY << 5);
	SendBuffer[4] = 0;
}
