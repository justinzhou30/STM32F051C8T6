#ifndef __RC522_INCLUDE__
#define __RC522_INCLUDE__

#include "stm32f0xx_hal.h"
#include "cmsis_os.h"
#include "stm32_hal_legacy.h"

#define RC522_GPIO GPIOB

#define RC522_RST GPIO_PIN_11
#define RC522_SPI_CS GPIO_PIN_12
#define RC522_SPI_SCK GPIO_PIN_13
#define RC522_SPI_MOSI GPIO_PIN_14
#define RC522_SPI_MISO GPIO_PIN_15

void rc522_run(void);
void set_RST(GPIO_PinState state);
void set_CS(GPIO_PinState state);
void set_SCK(GPIO_PinState state);
void set_MOSI(GPIO_PinState state);
GPIO_PinState get_MISO(void);
void rc522_resetPin(void);



#endif

