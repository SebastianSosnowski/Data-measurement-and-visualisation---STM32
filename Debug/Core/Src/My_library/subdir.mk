################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/My_library/BMP280.c \
../Core/Src/My_library/GFX.c \
../Core/Src/My_library/ILI9341.c \
../Core/Src/My_library/XPT2046.c 

OBJS += \
./Core/Src/My_library/BMP280.o \
./Core/Src/My_library/GFX.o \
./Core/Src/My_library/ILI9341.o \
./Core/Src/My_library/XPT2046.o 

C_DEPS += \
./Core/Src/My_library/BMP280.d \
./Core/Src/My_library/GFX.d \
./Core/Src/My_library/ILI9341.d \
./Core/Src/My_library/XPT2046.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/My_library/%.o Core/Src/My_library/%.su: ../Core/Src/My_library/%.c Core/Src/My_library/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../FATFS/Target -I../FATFS/App -I../Middlewares/Third_Party/FatFs/src -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-My_library

clean-Core-2f-Src-2f-My_library:
	-$(RM) ./Core/Src/My_library/BMP280.d ./Core/Src/My_library/BMP280.o ./Core/Src/My_library/BMP280.su ./Core/Src/My_library/GFX.d ./Core/Src/My_library/GFX.o ./Core/Src/My_library/GFX.su ./Core/Src/My_library/ILI9341.d ./Core/Src/My_library/ILI9341.o ./Core/Src/My_library/ILI9341.su ./Core/Src/My_library/XPT2046.d ./Core/Src/My_library/XPT2046.o ./Core/Src/My_library/XPT2046.su

.PHONY: clean-Core-2f-Src-2f-My_library

