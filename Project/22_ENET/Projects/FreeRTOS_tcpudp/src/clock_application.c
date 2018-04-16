/*!
    \file  gd32f307c_eval.c
    \brief firmware functions to manage leds, keys, COM ports
*/

/*
    Copyright (C) 2017 GigaDevice

    2017-07-28, V1.0.0, demo for GD32F30x
*/

#include <gd32f30x.h>
#include "clock_application.h"
#include "display_dp.h"
#include "stdint.h"
#include "gpio_i2c_driver.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "stdio.h"
#include "queue.h"
#include "local_message.h"

uint32_t SystemSecoundTime = 0;
UTCTimeStruct_t SystemUTCTime = {0};

TimerHandle_t SecondTimer = NULL;


//init time is 2018-03-09  Friday  00:00:00
#define DEC2BCD(value)       (((value / 10) << 4) | (value % 10))
#define BCD2DEC(vaule)       ((10*(value>>4)) + ((value<<4)>>4))


void SecondTimerCallback( TimerHandle_t pxTimer )
{
    #ifdef RTC_PCF8523
    uint8_t buf[10];
    uint8_t BCD_HIGH = 0;
    uint8_t BCD_LOW = 0;
    iic_read_device(0xd0, 3, 7, buf + 3);

    BCD_HIGH = ((0x7F&buf[3]) >> 4 );
    BCD_LOW = (0x7F&buf[3])&0x0f;
    SystemUTCTime.Seconds = 10 * BCD_HIGH + BCD_LOW;
    BCD_HIGH = ((0x7F&buf[4]) >> 4 );
    BCD_LOW = (0x7F&buf[4])&0x0f;
    SystemUTCTime.Minutes = 10 * BCD_HIGH + BCD_LOW;
    BCD_HIGH = ((0x3F&buf[5]) >> 4 );
    BCD_LOW = (0x3F&buf[5])&0x0f;
    SystemUTCTime.Hours = 10 * BCD_HIGH + BCD_LOW;

    BCD_HIGH = ((0x3f&buf[6]) >> 4 );
    BCD_LOW = (0x3f&buf[6])&0x0f;
    SystemUTCTime.Days = 10 * BCD_HIGH + BCD_LOW;
    BCD_HIGH = ((0x07&buf[7]) >> 4 );
    BCD_LOW = (0x07&buf[7])&0x0f;
    SystemUTCTime.Weekdays = 10 * BCD_HIGH + BCD_LOW;
    BCD_HIGH = ((0x1F&buf[8]) >> 4 );
    BCD_LOW = (0x1F&buf[8])&0x0f;
    SystemUTCTime.Months = 10 * BCD_HIGH + BCD_LOW;
    BCD_HIGH = ((0xFF&buf[9]) >> 4 );
    BCD_LOW = (0xFF&buf[9])&0x0f;
    SystemUTCTime.Years = 10 * BCD_HIGH + BCD_LOW;
    utc_time_to_second_time(SystemUTCTime, &SystemSecoundTime);
    printf(" RTC ==== time data: year:%d,month:%d,date:%d, weekday:%d,Hour:%d,minute:%d,Second:%d! \n", \
            SystemUTCTime.Years, \
            SystemUTCTime.Months, \
            SystemUTCTime.Days, \
            SystemUTCTime.Weekdays, \
            SystemUTCTime.Hours, \
            SystemUTCTime.Minutes, \
            SystemUTCTime.Seconds);
    #endif
    #ifdef RTC_INTERNAL
    SystemSecoundTime = rtc_counter_get();
    second_time_to_utc_time(&SystemUTCTime, SystemSecoundTime);
    #if 0
    printf(" Internal RTC ==== time data: year:%d,month:%d,date:%d, weekday:%d,Hour:%d,minute:%d,Second:%d! \n", \
            SystemUTCTime.Years, \
            SystemUTCTime.Months, \
            SystemUTCTime.Days, \
            SystemUTCTime.Weekdays, \
            SystemUTCTime.Hours, \
            SystemUTCTime.Minutes, \
            SystemUTCTime.Seconds);
    #endif
    #endif

}

void RTC_SET_TIME(UTCTimeStruct_t utc_setting)
{
    #ifdef RTC_PCF8523
    uint8_t rtc_buf[10];
    rtc_buf[0] = 0x58;
    rtc_buf[2] = 0xa0;
    rtc_buf[3] = ((utc_setting.Seconds / 10) << 4) | (utc_setting.Seconds % 10);
    rtc_buf[4] = ((utc_setting.Minutes / 10) << 4) | (utc_setting.Minutes % 10);
    rtc_buf[5] = ((utc_setting.Hours / 10) << 4) | (utc_setting.Hours % 10);
    rtc_buf[7] = (((utc_setting.Weekdays) / 10) << 4) | ((utc_setting.Weekdays) % 10);  /* only can set here 1~7 */
    rtc_buf[6] = ((utc_setting.Days / 10) << 4) | (utc_setting.Days % 10);
    rtc_buf[8] = ((utc_setting.Months / 10) << 4) | (utc_setting.Months % 10);
    rtc_buf[9] = ((utc_setting.Years / 10) << 4) | (utc_setting.Years % 10);
    iic_write_device(0xd0, 2, 8, rtc_buf + 2);
    #endif
    #ifdef RTC_INTERNAL
    uint32_t tmp =0;
    {
        /* TODO by Howie */
        rtc_configuration();
        bkp_write_data(BKP_DATA_0, 0xA5A5);
        /* clear reset flags */
        rcu_all_reset_flag_clear();
    }
    utc_time_to_second_time(utc_setting, &tmp);
    /* wait until last write operation on RTC registers has finished */
    rtc_lwoff_wait();
    /* change the current time */
    rtc_counter_set(tmp);
    rtc_lwoff_wait();
    /* wait until last write operation on RTC registers has finished */
    rtc_lwoff_wait();
    #endif
    SystemClockDisplay();
    if (DisPlayTimer != NULL)
    {
        xTimerReset(DisPlayTimer, 0);
    }
    if (SecondTimer != NULL)
    {
        xTimerReset(SecondTimer, 0);
    }
}

void iHomeClockRTC_Init(void)
{
    #ifdef RTC_PCF8523
    RTC_PCF8523_Init();
    #endif
    #ifdef RTC_INTERNAL
    initernal_rtc_module_init();
    #endif
    SecondTimer = xTimerCreate( "SecondTimer", // Just a text name, not used by the kernel.
                                1000 / portTICK_RATE_MS, // The timer period in ticks. 1s, don't change!
                                pdTRUE, // The timers will auto-reload themselves when they expire.
                                ( void *) 0, // Assign each timer a unique id equal to its array index.
                                SecondTimerCallback
                              );

    if (SecondTimer != NULL)
    {
        xTimerStart(SecondTimer, 0);
    }
}
void RTC_PCF8523_Init(void)
{
    system_clock_init_by_utc(SystemUTCTime);
    GpioI2CDriverInit();
}

/**
  * @brief   system clock init
  * @param   init offset secod, sometimes is zero
  * @return  void
  */
void system_clock_init_by_second(uint32_t sec_time)
{
    SystemSecoundTime = sec_time;

    second_time_to_utc_time(&SystemUTCTime, SystemSecoundTime);
}
void system_clock_init_by_utc(UTCTimeStruct_t utc_time)
{
    SystemUTCTime = utc_time;

    utc_time_to_second_time(SystemUTCTime, &SystemSecoundTime);
}
static uint8_t month_length_calc(uint8_t lpyr, uint8_t mon)
{
    uint8_t days = 31;

    if (mon == 1)   // feb
    {
        days = (28 + lpyr);
    }
    else
    {
        if (mon > 6)   // aug-dec
        {
            mon--;
        }

        if (mon & 1)
        {
            days = 30;
        }
    }
    return (days);
}

void second_time_to_utc_time(UTCTimeStruct_t *utc_time, uint32_t sec_time)
{
    /* calculate the time less than a day - hours, minutes, seconds */
    {
        uint32_t day = sec_time % (86400UL);
        utc_time->Seconds = day % 60UL;
        utc_time->Minutes = (day % 3600UL) / 60UL;
        utc_time->Hours = day / 3600UL;
    }

    /* Fill in the calendar - day, month, year */
    {
        uint16_t numDays = sec_time / (86400UL);
        utc_time->Years = 2000 - 2000;
        while (numDays >= YearLength(utc_time->Years))
        {
            numDays -= YearLength(utc_time->Years);
            utc_time->Years++;
        }

        utc_time->Months = 0;
        while (numDays >= month_length_calc(IsLeapYear(utc_time->Years), utc_time->Months))
        {
            numDays -= month_length_calc(IsLeapYear(utc_time->Years), utc_time->Months);
            utc_time->Months++;
        }

        utc_time->Days = numDays;
        utc_time->Months = utc_time->Months + 1;
        utc_time->Days = utc_time->Days + 1;
    }
    /* Fill in the weekday */
    {
        uint32_t day = sec_time / (86400UL);
        utc_time->Weekdays = (((day % 7) + 5) % 7);
    }
}
void utc_time_to_second_time(UTCTimeStruct_t utc_time, uint32_t *sec_time)
{
    uint32_t i = 0;
    uint32_t offset = 0;
    uint8_t leapYear = IsLeapYear(utc_time.Years + 2000);

    /* day time */
    offset += utc_time.Seconds;
    offset += utc_time.Minutes * 60;
    offset += utc_time.Hours * 60 * 60;


    offset += (86400UL) * (utc_time.Days - 1);

    for (i = 0; i < utc_time.Months - 1; ++i)
    {
        /* month start from 1 */
        offset += month_length_calc(leapYear, i) * (86400UL);
    }

    for (i = 0; i < utc_time.Years ; ++i)
    {
        if (IsLeapYear(i + 2000))
        {
            offset += (86400UL) * 366;
        }
        else
        {
            offset += (86400UL) * 365;
        }
    }
    *sec_time = offset;
}

void initernal_rtc_module_init(void)
{
    #ifdef INTERNAL_RTC_INTERRUPT
    /* do NVIC setting for RTC MODULE */
    nvic_irq_enable(RTC_IRQn,3,0);
    #endif
    /* check if RTC have been init */
    if (bkp_read_data(BKP_DATA_0) != 0xA5A5)
    {
        printf("\r\nThis is a RTC demo!\r\n");
        printf("\r\nRTC not yet configured.... \n");
        /* RTC configuration */
        rtc_configuration();
        {
            /* setting init time here 0 means 2000,1,1, 00:00:00 */
            uint32_t temp = 0;
            /* wait until last write operation on RTC registers has finished */
            rtc_lwoff_wait();
            /* change the current time */
            rtc_counter_set(temp);
            rtc_lwoff_wait();
            /* wait until last write operation on RTC registers has finished */
            rtc_lwoff_wait();
        }
        
        bkp_write_data(BKP_DATA_0, 0xA5A5);

    }
    else
    {
        /* check if the power on reset flag is set */
        if (rcu_flag_get(RCU_FLAG_PORRST) != RESET){
            printf("\r\n\n Power On Reset occurred....");
        }else if (rcu_flag_get(RCU_FLAG_SWRST) != RESET){
            /* check if the pin reset flag is set */
            printf("\r\n\n External Reset occurred....");
        }
        printf("\r\n No need to configure RTC.... \n");
        /* wait for RTC registers synchronization */
        rtc_register_sync_wait();
        rtc_lwoff_wait();
        #ifdef INTERNAL_RTC_INTERRUPT
        /* enable the RTC second */
        rtc_interrupt_enable(RTC_INT_SECOND);
        #endif
        /* wait until last write operation on RTC registers has finished */
        rtc_lwoff_wait();
    }
    /* clear reset flags */
    rcu_all_reset_flag_clear();
    SystemSecoundTime = rtc_counter_get();
    second_time_to_utc_time(&SystemUTCTime, SystemSecoundTime);
}
void rtc_configuration(void)
{
    /* RTC configuration */
    /* enable PMU and BKPI clocks */
    rcu_periph_clock_enable(RCU_BKPI);
    rcu_periph_clock_enable(RCU_PMU);
    /* allow access to BKP domain */
    pmu_backup_write_enable();

    /* reset backup domain */
    bkp_deinit();

    /* enable LXTAL !!!!!Please Be care of CLOCK SOURCE for RTC!!!!!!*/
    rcu_osci_on(RCU_LXTAL);
    /* wait till LXTAL is ready */
    rcu_osci_stab_wait(RCU_LXTAL);
    
    /* select RCU_LXTAL as RTC clock source */
    rcu_rtc_clock_config(RCU_RTCSRC_LXTAL);

    /* enable RTC Clock */
    rcu_periph_clock_enable(RCU_RTC);

    /* wait for RTC registers synchronization */
    rtc_register_sync_wait();

    /* wait until last write operation on RTC registers has finished */
    rtc_lwoff_wait();

    #ifdef INTERNAL_RTC_INTERRUPT
    /* enable the RTC second interrupt*/
    rtc_interrupt_enable(RTC_INT_SECOND);
    #endif
    /* wait until last write operation on RTC registers has finished */
    rtc_lwoff_wait();

    /* set RTC prescaler: set RTC period to 1s */
    rtc_prescaler_set(32768);

    /* wait until last write operation on RTC registers has finished */
    rtc_lwoff_wait();
}

/*!
    \brief      this function handles RTC global interrupt request
    \param[in]  none
    \param[out] none
    \retval     none
*/
void RTC_IRQHandler(void)
{
    if (rtc_flag_get(RTC_FLAG_SECOND) != RESET){
        /* clear the RTC second interrupt flag*/
        rtc_flag_clear(RTC_FLAG_SECOND);    
        /* enable time update */
    }
    SystemSecoundTime = rtc_counter_get();
    second_time_to_utc_time(&SystemUTCTime, SystemSecoundTime);
    printf(" RTC ==== time data: year:%d,month:%d,date:%d, weekday:%d,Hour:%d,minute:%d,Second:%d! \n", \
            SystemUTCTime.Years, \
            SystemUTCTime.Months, \
            SystemUTCTime.Days, \
            SystemUTCTime.Weekdays, \
            SystemUTCTime.Hours, \
            SystemUTCTime.Minutes, \
            SystemUTCTime.Seconds);
}
