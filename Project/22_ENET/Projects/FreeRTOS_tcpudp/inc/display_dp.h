/*!
    \file  gd32f307c_eval.h
    \brief definitions for GD32f307C_EVAL's leds, keys and COM ports hardware resources
*/

/*
    Copyright (C) 2017 GigaDevice

    2017-07-28, V1.0.0, demo for GD32F30x
*/

#ifndef DISPLAY_DP_H
#define DISPLAY_DP_H

#ifdef cplusplus
 extern "C" {
#endif
#include "FreeRTOS.h"
#include "timers.h"

#define LED_SDI_PIN                         GPIO_PIN_10
#define LED_SDI_GPIO_PORT                   GPIOC
#define LED_SDI_GPIO_CLK                    RCU_GPIOC
  
#define LED_CLK_PIN                         GPIO_PIN_11
#define LED_CLK_GPIO_PORT                   GPIOC
#define LED_CLK_GPIO_CLK                    RCU_GPIOC
  
#define LED_LE_PIN                         GPIO_PIN_12
#define LED_LE_GPIO_PORT                   GPIOC
#define LED_LE_GPIO_CLK                    RCU_GPIOC

#define DISPLAY_BUF_LEN     6  //其中中间的分隔符需要用 对 DisplayBuffer + 3 的BIT7 清0
#define TABLE_LEN           12

#define SET_SDI()                gpio_bit_write(LED_SDI_GPIO_PORT, LED_SDI_PIN, SET)
#define RESET_SDI()              gpio_bit_write(LED_SDI_GPIO_PORT, LED_SDI_PIN, RESET)
#define SET_CLK()                gpio_bit_write(LED_CLK_GPIO_PORT, LED_CLK_PIN, SET)
#define RESET_CLK()              gpio_bit_write(LED_CLK_GPIO_PORT, LED_CLK_PIN, RESET)
#define SET_LE()                 gpio_bit_write(LED_LE_GPIO_PORT, LED_LE_PIN, SET)
#define RESET_LE()               gpio_bit_write(LED_LE_GPIO_PORT, LED_LE_PIN, RESET)

extern TimerHandle_t DisPlayTimer;


void DisPlayWriteByte(uint8_t data);
void DisplayDeviceInit(void);
void DisplayDriverInit(void);
void DisplayUpdataFramerBuf(void);
void DisplayChar(uint8_t index, uint8_t location);
void SystemClockDisplay(void);
void DisPlayPowerOFF(void);
void DisPlayPowerON(void);
void DisPlaySpecialChar(void);

#ifdef cplusplus
}
#endif

#endif /* DISPLAY_DP_H */
