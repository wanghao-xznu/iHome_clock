/*!
    \file  gd32f307c_eval.c
    \brief firmware functions to manage leds, keys, COM ports
*/

/*
    Copyright (C) 2017 GigaDevice

    2017-07-28, V1.0.0, demo for GD32F30x
*/

#include <gd32f30x.h>
#include "string.h"
#include "gpio_i2c_driver.h"

void GpioI2CDriverInit(void)
{
    {
        /* enable rtc sda clock */
        rcu_periph_clock_enable(RTC_I2C_SDA_GPIO_CLK);
        /* configure rtc sda GPIO port */
        gpio_init(RTC_I2C_SDA_GPIO_PORT, GPIO_MODE_OUT_OD, GPIO_OSPEED_50MHZ, RTC_I2C_SDA_PIN);
    }
    {
        /* enable rtc scl clock */
        rcu_periph_clock_enable(RTC_I2C_SCL_GPIO_CLK);
        /* configure rtc scl GPIO port */
        gpio_init(RTC_I2C_SCL_GPIO_PORT, GPIO_MODE_OUT_OD, GPIO_OSPEED_50MHZ, RTC_I2C_SCL_PIN);
    }
    gpio_bit_write(RTC_I2C_SDA_GPIO_PORT, RTC_I2C_SDA_PIN, SET);
    gpio_bit_write(RTC_I2C_SCL_GPIO_PORT, RTC_I2C_SCL_PIN, SET);
}


void IIC_SET_SDA(void)
{
    gpio_bit_set(RTC_I2C_SDA_GPIO_PORT, RTC_I2C_SDA_PIN);
}

void IIC_RESET_SDA(void)
{
    gpio_bit_reset(RTC_I2C_SDA_GPIO_PORT, RTC_I2C_SDA_PIN);
}

void IIC_SET_SCL(void)
{
    gpio_bit_set(RTC_I2C_SCL_GPIO_PORT, RTC_I2C_SCL_PIN);
}

void IIC_RESET_SCL(void)
{
    gpio_bit_reset(RTC_I2C_SCL_GPIO_PORT, RTC_I2C_SCL_PIN);
}

unsigned char IIC_IS_SDA_H(void)
{
    if (gpio_input_bit_get(RTC_I2C_SDA_GPIO_PORT, RTC_I2C_SDA_PIN) == SET)
        return 0x01;
    else
        return 0x00;
}

unsigned char IIC_IS_SDA_L(void)
{
    if (gpio_input_bit_get(RTC_I2C_SDA_GPIO_PORT, RTC_I2C_SDA_PIN) == RESET)
        return 0x01;
    else
        return 0x00;
}

/* delay us */
static void _delay_us(int us)
{
    int i, k;
    for (i = 0; i < 68; i++)
    {
        for (k = 0; k < 2; k++)
        {
            ;
        }
    }
}
/**********************************************************
**Name:     IIC_START
**Func:     IIC Start Singal
**Note:
**********************************************************/
void IIC_START(void)
{
    IIC_SET_SDA();
    _delay_us(IIC_DELAY_US);
    IIC_SET_SCL();
    _delay_us(IIC_DELAY_US);
    IIC_RESET_SDA();
    _delay_us(IIC_DELAY_US);
    IIC_RESET_SCL();
    _delay_us(IIC_DELAY_US);
}
/**********************************************************
**Name:     IIC_STOP
**Func:     IIC Stop Singal
**Note:
**********************************************************/
void IIC_STOP(void)
{
    IIC_RESET_SCL();
    _delay_us(IIC_DELAY_US);
    IIC_RESET_SDA();
    _delay_us(IIC_DELAY_US);
    IIC_SET_SCL();
    _delay_us(IIC_DELAY_US);
    IIC_SET_SDA();
    _delay_us(IIC_DELAY_US);
}

/**********************************************************
**Name:     IIC_ACK
**Func:     IIC ACK Singal
**Note:
**********************************************************/
void IIC_ACK(void)
{
    IIC_RESET_SDA();
    _delay_us(IIC_DELAY_US);
    IIC_SET_SCL();
    _delay_us(IIC_DELAY_US);
    IIC_RESET_SCL();
    _delay_us(IIC_DELAY_US);

}

/**********************************************************
**Name:     IIC_NACK
**Func:     IIC NACK Singal
**Note:
**********************************************************/
void IIC_NACK(void)
{
    IIC_SET_SDA();
    _delay_us(IIC_DELAY_US);
    IIC_SET_SCL();
    _delay_us(IIC_DELAY_US);
    IIC_RESET_SCL();
    _delay_us(IIC_DELAY_US);
}
/**********************************************************
**Name:     IIC_WAIT_ACK
**Func:     wait for ack Singal
**Note:
**********************************************************/
unsigned char IIC_WAIT_ACK(void)
{
    unsigned char i;
    IIC_SET_SDA();

    IIC_RESET_SCL();
    _delay_us(IIC_DELAY_US);

    IIC_SET_SCL();
    _delay_us(IIC_DELAY_US);


    for (i = 0; i < 200; i++)
    {
        if (IIC_IS_SDA_L())
        {
            IIC_RESET_SCL();
            _delay_us(IIC_DELAY_US);
            IIC_SET_SDA();
            _delay_us(IIC_DELAY_US);
            return 0x01;

        }
        _delay_us(IIC_DELAY_US / 2);
    }

    IIC_RESET_SCL();
    _delay_us(IIC_DELAY_US);
    IIC_SET_SDA();
    _delay_us(IIC_DELAY_US);
    IIC_STOP();
    return 0x00;
}
/**********************************************************
**Name:     IIC_WRITE_BYTE
**Func:     IIC Write one byte
**Output:   true-------ACK
            false------NACK
**********************************************************/
unsigned char IIC_WRITE_BYTE(unsigned char para)
{
    unsigned char i;
    IIC_RESET_SCL();
    _delay_us(IIC_DELAY_US);
    for (i = 0x80; i != 0; i >>= 1)
    {
        if (para & i)
        {
            IIC_SET_SDA();
        }
        else
        {
            IIC_RESET_SDA();
        }
        _delay_us(IIC_DELAY_US);
        IIC_SET_SCL();
        _delay_us(IIC_DELAY_US);
        IIC_RESET_SCL();
        _delay_us(IIC_DELAY_US);
    }
    IIC_SET_SDA();
    _delay_us(IIC_DELAY_US);
    if (IIC_WAIT_ACK() == 0x01)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/**********************************************************
**Name:     IIC_READ_BYTE
**Func:     IIC Read one byte
**Input:    true-------Send ACK
            false------Send NACK
**********************************************************/
unsigned char IIC_READ_BYTE(uint8_t ack)
{
    unsigned char i;
    unsigned char para = 0;

    IIC_SET_SDA();
    for (i = 0x80; i != 0; i >>= 1)
    {
        IIC_RESET_SCL();
        _delay_us(IIC_DELAY_US);
        IIC_SET_SCL();
        _delay_us(IIC_DELAY_US >> 1);
        if (IIC_IS_SDA_H())
        {
            para |= i;
        }
        _delay_us(IIC_DELAY_US >> 1);
    }
    IIC_RESET_SCL();
    _delay_us(IIC_DELAY_US >> 1);
    IIC_SET_SDA();
    _delay_us(IIC_DELAY_US >> 1);

    if (ack)
    {
        IIC_ACK();
    }
    else
    {
        IIC_NACK();
    }
    return (para);
}

void iic_write_device(uint8_t slave_addr, uint8_t reg_addr, uint8_t len, uint8_t *buf)
{
    IIC_START();
    if (IIC_WRITE_BYTE(slave_addr) == IIC_STATUS_FALSE)
    {
        return;
    }
    if (IIC_WRITE_BYTE(reg_addr) == IIC_STATUS_FALSE)
    {
        return;
    }
    for (uint8_t i = 0; i < len; i++)
    {
        if (IIC_WRITE_BYTE(buf[i]) == IIC_STATUS_FALSE)
        {
            return;
        }
    }
    IIC_STOP();
}

void iic_read_device(uint8_t slave_addr, uint8_t reg_addr, uint8_t len, uint8_t *buf)
{
    IIC_START();
    if (IIC_WRITE_BYTE(slave_addr) == IIC_STATUS_FALSE)
    {
        return;
    }
    if (IIC_WRITE_BYTE(reg_addr) == IIC_STATUS_FALSE)
    {
        return;
    }
    IIC_START();
    if (IIC_WRITE_BYTE(slave_addr | 0x01) == IIC_STATUS_FALSE)
    {
        return;
    }
    for (uint8_t i = 0; i < len; i++)
    {
        if (i == (len - 1))
        {
            buf[i] = IIC_READ_BYTE(IIC_ACK_DISABLE);
        }
        else
        {
            buf[i] = IIC_READ_BYTE(IIC_ACK_ENABLE);
        }
    }
    IIC_STOP();
}
