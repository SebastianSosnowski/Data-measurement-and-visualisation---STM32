/*
 * ILI9341.c
 *
 *  Created on: Mar 18, 2022
 *      Author: Sebastian
 */
#include "main.h"
#include "My_library/ILI9341.h"


static SPI_HandleTypeDef *TFT_SPI;



//const to put it in flash, data which is send to ILI controler
static const uint8_t initcmd[] = {
  0xEF, 3, 0x03, 0x80, 0x02,
  0xCF, 3, 0x00, 0xC1, 0x30,
  0xED, 4, 0x64, 0x03, 0x12, 0x81,
  0xE8, 3, 0x85, 0x00, 0x78,
  0xCB, 5, 0x39, 0x2C, 0x00, 0x34, 0x02,
  0xF7, 1, 0x20,
  0xEA, 2, 0x00, 0x00,
  ILI9341_PWCTR1  , 1, 0x23,             // Power control VRH[5:0]
  ILI9341_PWCTR2  , 1, 0x10,             // Power control SAP[2:0];BT[3:0]
  ILI9341_VMCTR1  , 2, 0x3e, 0x28,       // VCM control
  ILI9341_VMCTR2  , 1, 0x86,             // VCM control2
  ILI9341_MADCTL  , 1, 0x48,             // Memory Access Control
  ILI9341_VSCRSADD, 1, 0x00,             // Vertical scroll zero
  ILI9341_PIXFMT  , 1, 0x55,
  ILI9341_FRMCTR1 , 2, 0x00, 0x18,
  ILI9341_DFUNCTR , 3, 0x08, 0x82, 0x27, // Display Function Control
  0xF2, 1, 0x00,                         // 3Gamma Function Disable
  ILI9341_GAMMASET , 1, 0x01,             // Gamma curve selected
  ILI9341_GMCTRP1 , 15, 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, // Set Gamma
    0x4E, 0xF1, 0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00,
  ILI9341_GMCTRN1 , 15, 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, // Set Gamma
    0x31, 0xC1, 0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F,
  ILI9341_SLPOUT  , 0x80,                // Exit Sleep
  ILI9341_DISPON  , 0x80,                // Display on
  0x00                                   // End of list
};

static void Transmit_To_TFT(uint8_t *Data, uint32_t Size)
{
	HAL_SPI_Transmit(TFT_SPI, Data, Size, 1000);
}

static void TFT_Send_Command(uint8_t Cmd)
{
	HAL_GPIO_WritePin(TFT_DC_RS_GPIO_Port, TFT_DC_RS_Pin, GPIO_PIN_RESET); //to send command
	Transmit_To_TFT(&Cmd, 1);
}

static void Send_Command_Data(uint8_t Cmd, uint8_t *Data, uint16_t Size)
{
	HAL_GPIO_WritePin(TFT_DC_RS_GPIO_Port, TFT_DC_RS_Pin, GPIO_PIN_RESET); //to send command
	Transmit_To_TFT(&Cmd, 1);

	HAL_GPIO_WritePin(TFT_DC_RS_GPIO_Port, TFT_DC_RS_Pin, GPIO_PIN_SET); // to send data of Size amount
	Transmit_To_TFT(Data, Size);
}

static void TFT_ILI9341_Set_Addr_Window(uint16_t x1, uint16_t y1, uint16_t w, uint16_t h) //sets area of drawing
{
	uint8_t Data[4];
	uint16_t x2 = (x1 + w - 1), y2 = (y1 + h - 1);

	//8.2.20
	Data[0] = (x1 >> 8); //MSB
	Data[1] = (x1 & 0xFF); //LSB
	Data[2] = (x2 >> 8); //MSB
	Data[3] = (x2 & 0xFF); //LSB
	Send_Command_Data(ILI9341_CASET, Data, 4); //columns

	Data[0] = (y1 >> 8);
	Data[1] = (y1 & 0xFF);
	Data[2] = (y2 >> 8);
	Data[3] = (y2 & 0xFF);
	Send_Command_Data(ILI9341_PASET, Data, 4); //rows

}

void TFT_Clear_Screen(uint16_t color)
{
	uint32_t Size = ILI9341_TFTWIDTH * ILI9341_TFTHEIGHT; //how long is data to send, how many pixels
	TFT_ILI9341_Set_Addr_Window(0, 0, ILI9341_TFTWIDTH, ILI9341_TFTHEIGHT); //set window to whole screen
	TFT_Send_Command(ILI9341_RAMWR); //send command to RAM

	HAL_GPIO_WritePin(TFT_DC_RS_GPIO_Port, TFT_DC_RS_Pin, GPIO_PIN_SET); //data mode
	while(Size > 0U)
	{
		/* Wait until TXE flag is set to send data */
		if(__HAL_SPI_GET_FLAG(TFT_SPI, SPI_FLAG_TXE))
		{
			*((__IO uint8_t*) &TFT_SPI->Instance->DR) = (color >> 8); //up 8 bits of pixel

			while(__HAL_SPI_GET_FLAG(TFT_SPI, SPI_FLAG_TXE) != SET); //wait until transfer is done i.e. 8 bits was sent

			*((__IO uint8_t*) &TFT_SPI->Instance->DR) = (color & 0xFF); // 2nd 8 bits of pixel
			Size--;
		}
	}

	while(__HAL_SPI_GET_FLAG(TFT_SPI,SPI_FLAG_BSY) != RESET); //stay in loop until spi used by tft is used
}

void TFT_Draw_Pixel(int16_t x, int16_t y, uint16_t color)
{
	uint8_t Data[2];
	if((x >= 0) && (x <= ILI9341_TFTWIDTH) && (y >= 0) && (y <= ILI9341_TFTHEIGHT))
	{
		TFT_ILI9341_Set_Addr_Window(x, y, 1, 1); //ustaw rozmiar okna pixel
		Data[0] = (color >> 8);
		Data[1] = (color & 0xFF);
		Send_Command_Data(ILI9341_RAMWR, Data, 2); //write to Ram and set color
	}
}

void TFT_Set_Rotation(uint8_t Rotation)
{
	if(Rotation > 3)
		return; // can't be higher than 3

	switch(Rotation)
	{
	case 0:
		Rotation = (MADCTL_MX | MADCTL_BGR);
		break;
	case 1:
		Rotation = (MADCTL_MV | MADCTL_BGR);
		break;
	case 2:
		Rotation = (MADCTL_MY | MADCTL_BGR);
		break;
	case 3:
		Rotation = (MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_BGR);
		break;
	}

	Send_Command_Data(ILI9341_MADCTL, &Rotation, 1);
}

void TFT_ILI9341_Init(SPI_HandleTypeDef *SPI)
{
	uint8_t cmd, x, numArgs;
	const uint8_t *addr = initcmd;

	TFT_SPI = SPI;

#if(ILI9341_HW_RESET == 1)
	HAL_GPIO_WritePin(TFT_Reset_GPIO_Port, TFT_Reset_Pin, GPIO_PIN_RESET); //reset low
	HAL_Delay(20);
	HAL_GPIO_WritePin(TFT_Reset_GPIO_Port, TFT_Reset_Pin, GPIO_PIN_SET); // reset high
	HAL_Delay(20);
#else
	TFT_Send_Command(ILI9341_SWRESET); // Engage software reset
	HAL_Delay(150);
#endif

	while((cmd = *(addr++)) > 0)
	{
		x = *(addr++);
		numArgs = x & 0x7F;
		Send_Command_Data(cmd, (uint8_t*) addr, numArgs);
		addr += numArgs;
		if(x & 0x80)
		{
			HAL_Delay(150);
		}
	}
	TFT_Set_Rotation(ILI9341_ROTATION);
}
