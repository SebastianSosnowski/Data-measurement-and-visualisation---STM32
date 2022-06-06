/*
 * XPT2046.h
 *
 */

#ifndef INC_XPT2046_H_
#define INC_XPT2046_H_

#define TOUCH_ROTATION	ILI9341_ROTATION
#define DISPLAY_HEIGHT	ILI9341_TFTHEIGHT
#define DISPLAY_WIDTH	ILI9341_TFTWIDTH


void TP_ADC_to_Pixels(uint16_t *Xs, uint16_t *Ys);

void XPT2046_Init(SPI_HandleTypeDef *SPI, IRQn_Type TouchPanel_IRQn);
void TP_IRQ(void);
void TP_Get_Point(uint16_t *X, uint16_t *Y);
void TP_Task(void);
//void DoCalibration(void);
uint8_t TP_Is_Touched(void);
#endif /* INC_XPT2046_H_ */
