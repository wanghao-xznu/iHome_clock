/*!
    \file  gd32f307c_eval.h
    \brief definitions for GD32f307C_EVAL's leds, keys and COM ports hardware resources
*/

/*
    Copyright (C) 2017 GigaDevice

    2017-07-28, V1.0.0, demo for GD32F30x
*/

#ifndef GPIO_I2C_DRIVER_H
#define GPIO_I2C_DRIVER_H


#include "gd32f30x.h"

#define IIC_STATUS_TRUE                         1
#define IIC_STATUS_FALSE                        0
#define IIC_ACK_ENABLE                          1
#define IIC_ACK_DISABLE                         0
#define RTC_I2C_SDA_PIN                         GPIO_PIN_15
#define RTC_I2C_SDA_GPIO_PORT                   GPIOB
#define RTC_I2C_SDA_GPIO_CLK                    RCU_GPIOB

#define RTC_I2C_SCL_PIN                         GPIO_PIN_14
#define RTC_I2C_SCL_GPIO_PORT                   GPIOB
#define RTC_I2C_SCL_GPIO_CLK                    RCU_GPIOB


#define IIC_DELAY_US    2

void GpioI2CDriverInit(void);
void IIC_SET_SDA(void);
void IIC_RESET_SDA(void);
void IIC_SET_SCL(void);
void IIC_RESET_SCL(void);
unsigned char IIC_IS_SDA_H(void);
unsigned char IIC_IS_SDA_L(void);
void IIC_START(void);
void IIC_STOP(void);
void IIC_ACK(void);
void IIC_NACK(void);
unsigned char IIC_WAIT_ACK(void);
unsigned char IIC_WRITE_BYTE(unsigned char para);
unsigned char IIC_READ_BYTE(uint8_t ack);
void iic_write_device(uint8_t slave_addr, uint8_t reg_addr, uint8_t len, uint8_t *buf);
void iic_read_device(uint8_t slave_addr, uint8_t reg_addr, uint8_t len, uint8_t *buf);



#endif /* GPIO_I2C_DRIVER_H */
