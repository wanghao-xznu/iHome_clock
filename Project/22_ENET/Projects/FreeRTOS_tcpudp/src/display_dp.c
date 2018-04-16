/*!
    \file  gd32f307c_eval.c
    \brief firmware functions to manage leds, keys, COM ports
*/

/*
    Copyright (C) 2017 GigaDevice

    2017-07-28, V1.0.0, demo for GD32F30x
*/

#include <gd32f30x.h>
#include "display_dp.h"
#include "string.h"
#include "gd32f307c_eval.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "stdio.h"
#include "clock_application.h"
#include "queue.h"
#include "local_message.h"


TimerHandle_t DisPlayTimer = NULL;

const uint8_t DPtable[TABLE_LEN] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, ':', ' '}; /*  0,1,2,3,...8,9, */

uint8_t DisplayBuffer[DISPLAY_BUF_LEN] = {0};
bool SecondShowFlag = FALSE;

static void LED_DELAY_US(uint32_t count)
{
    uint32_t i = count * 50;
    while (i--);
}
void DisplayDeviceInit(void)
{
    {
        /* enable dp sdi clock */
        rcu_periph_clock_enable(LED_SDI_GPIO_CLK);
        /* configure dp clk GPIO port */
        gpio_init(LED_SDI_GPIO_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, LED_SDI_PIN);
    }
    {
        /* enable dp clk clock */
        rcu_periph_clock_enable(LED_CLK_GPIO_CLK);
        /* configure dp clk GPIO port */
        gpio_init(LED_CLK_GPIO_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, LED_CLK_PIN);
    }
    {
        /* enable dp le clock */
        rcu_periph_clock_enable(LED_LE_GPIO_CLK);
        /* configure dp le GPIO port */
        gpio_init(LED_LE_GPIO_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, LED_LE_PIN);
    }
    gpio_bit_write(LED_SDI_GPIO_PORT, LED_SDI_PIN, RESET);
    gpio_bit_write(LED_CLK_GPIO_PORT, LED_CLK_PIN, RESET);
    gpio_bit_write(LED_LE_GPIO_PORT, LED_LE_PIN, RESET);
}

void DisPlayTimerCallback( TimerHandle_t pxTimer )
{
    SystemClockDisplay();
}
void DisplayDriverInit(void)
{
    memset(DisplayBuffer, 0x00, DISPLAY_BUF_LEN);

    DisplayUpdataFramerBuf();
    DisPlayTimer = xTimerCreate( "DisPlayTimer", // Just a text name, not used by the kernel.
                                 1000 / portTICK_RATE_MS, // The timer period in ticks. 1s, don't change!
                                 pdTRUE, // The timers will auto-reload themselves when they expire.
                                 ( void *) 0, // Assign each timer a unique id equal to its array index.
                                 DisPlayTimerCallback
                               );
    if (DisPlayTimer != NULL)
    {
        xTimerStart(DisPlayTimer, 0);
    }
    printf("display driver init OK! line = %d \n", __LINE__);
}

void DisplayChar(uint8_t index, uint8_t location)
{
    if ((location < DISPLAY_BUF_LEN) && (index < TABLE_LEN))
    {
        DisplayBuffer[location] = DPtable[index];
    }
    else if (location == 6)
    {
        if (DPtable[index] == ':')
        {
            DisplayBuffer[2] |= 0x80;
        }
        else
        {
            DisplayBuffer[2] &= 0x7f;
        }
    }
    else
    {
        //do nothing
    }
}

void DisplayUpdataFramerBuf(void)
{
    RESET_LE();
    LED_DELAY_US(10);
    for (uint8_t i = 0; i < DISPLAY_BUF_LEN; i++)
    {
        DisPlayWriteByte(DisplayBuffer[i]);
    }
    RESET_CLK();
    SET_SDI();
    LED_DELAY_US(10);
    SET_LE();
    LED_DELAY_US(10);
    RESET_LE();
}
void DisPlayPowerOFF(void)
{
    memset(DisplayBuffer, 0x00, DISPLAY_BUF_LEN);
    DisplayUpdataFramerBuf();
}
void DisPlayPowerON(void)
{
    memset(DisplayBuffer, 0xff, DISPLAY_BUF_LEN);
    DisplayUpdataFramerBuf();
}
void DisPlaySpecialChar(void)
{
    memset(DisplayBuffer, 0x55, DISPLAY_BUF_LEN);
    DisplayUpdataFramerBuf();
}

void DisPlayWriteByte(uint8_t data)
{
    for (uint8_t i = 0x80; i != 0; i >>= 1)
    {
        RESET_CLK();
        LED_DELAY_US(10);
        if (data & i)
        {
            SET_SDI();
        }
        else
        {
            RESET_SDI();
        }
        SET_CLK();
        LED_DELAY_US(10);
    }
}

void SystemClockDisplay(void)
{
    static uint32_t discount = 0;
    discount++;
    
    memset(DisplayBuffer, 0x00, DISPLAY_BUF_LEN);
    
    DisplayChar((SystemUTCTime.Hours)/10, 0);
    DisplayChar((SystemUTCTime.Hours)%10, 1);
    
    DisplayChar((SystemUTCTime.Minutes)/10, 2);
    DisplayChar((SystemUTCTime.Minutes)%10, 3);
    
    if(SecondShowFlag == FALSE)
    {
        DisplayChar((SystemUTCTime.Seconds)/10, 5);
        DisplayChar((SystemUTCTime.Seconds)%10, 4);
        DisplayChar(10, 6);
    }
    else
    {
        if ((discount % 2) == 0)
        {
            /*  mean dispaly ':' */
            DisplayChar(10, 6);
        }
        else
        {
            /*  mean no dispaly ':' */
            DisplayChar(11, 6);
        }
    }
    DisplayUpdataFramerBuf();
    
    extern bool LocalLinkStatus;
    extern bool localNTPSeverRequestSuccess;
    if((SystemUTCTime.Seconds%10 == 0)&&(LocalLinkStatus == TRUE)&&(localNTPSeverRequestSuccess == FALSE))
    {
        extern xQueueHandle hNTPQueueHandle;
        EVENT_MSG ntp_client_msg;
        ntp_client_msg.event = EVNET_NTP_TIME_UPDATA_FROM_ALI_SERVER;
        if( hNTPQueueHandle != NULL)
        {
            xQueueSend(hNTPQueueHandle, &ntp_client_msg, 0x0);
        }
    }

    if((SystemUTCTime.Seconds%2 == 0)&&(LocalLinkStatus == TRUE))
    {
        printf("local server send! \n ");
        extern xQueueHandle hNTPQueueHandle;
        EVENT_MSG ntp_client_msg;
        ntp_client_msg.event = EVNET_NTP_TIME_UPDATA_FROM_LOCAL_SERVER;
        if( hNTPQueueHandle != NULL)
        {
            xQueueSend(hNTPQueueHandle, &ntp_client_msg, 0x0);
        }
    }
}

