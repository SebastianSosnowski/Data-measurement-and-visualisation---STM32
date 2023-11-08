# Projekt-STM32
The goal of this project was to perform 3 tasks: measure temperature and pressure, display the data on screen as a plot and save data to SD card. All of them were combined as an application, which every 1 second performs temperature and pressure reading with a use of sensor, which communicates with uC through I2C protocol. Then the data is send to TFT touch panel, which communicates with uC through SPI protocol. The screen can switch between displaying 2 dynamic plots of temperature and pressure from last 10 seconds. In addition, it is possible to save actuallly displayed data to SD card, which communicates through SDIO. Whole program was written with a use of HAL libraries.
List of used devices:

1) Nucleo board STM32F411RE

2) Screen MSP2402,

3) Sensor BMP280,

4) Card reader SD SDIO,

5) SD card.

![image](https://github.com/Mefiu1000/Projekt-STM32/assets/68904952/7f0fd44b-8888-48a6-bc05-3abc4172c4db)

Switching between plots is performed by pressing adequate button on the screen. A pressing  is signalised by color change from white to green. Analogically saving data to SD card is performed.

![image](https://github.com/Mefiu1000/Projekt-STM32/assets/68904952/2b3a2092-716d-4284-8930-164550221360)
