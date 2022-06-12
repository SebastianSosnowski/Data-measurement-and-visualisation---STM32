# Projekt-STM32
Projekt zaliczeniowy (BMP280+TFT_MSP+SD)
W ramach projektu napisano aplikację, która dokonuje co 1 sekundę odczytu temperatury 
i ciśnienia z użyciem czujnika, który komunikuje się z mikrokontrolerem poprzez protokół I2C. 
Następnie dane te przesyłane są na wyświetlacz TFT z dotykiem, który komunikuje się 
z mikrokontrolerem poprzez protokół SPI. Wyświetlacz ma możliwość wyświetlania
2 dynamicznych wykresów pokazujących zmianę wartości temperatury i ciśnienia w ciągu ostatnich 
10 sekund. Dodatkowo w każdej chwili dane z pomiarów można zapisać na kartę SD
z wykorzystaniem interfejsu SDIO.
Lista wykorzystanych urządzeń:

1) Płytka rozwojowa STM32F411RE,

2) wyświetlacz MSP2402,

3) czujnik BMP280,

4) czytnik kart SD SDIO,

5) karta SD.
