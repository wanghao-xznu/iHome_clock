/*!
    \file  gd32f307c_eval.h
    \brief definitions for GD32f307C_EVAL's leds, keys and COM ports hardware resources
*/

/*
    Copyright (C) 2017 GigaDevice

    2017-07-28, V1.0.0, demo for GD32F30x
*/

#ifndef CLOCK_APPLICATION_H
#define CLOCK_APPLICATION_H


#include "gd32f30x.h"
#include "stdint.h"
#include "gd32f30x_enet_eval.h"

//#define RTC_PCF8523
#define RTC_INTERNAL
#ifdef RTC_INTERNAL
//#define INTERNAL_RTC_INTERRUPT
#endif

#define YearLength(yr) (IsLeapYear(yr) ? 366 : 365)
#define IsLeapYear(yr) (!((yr) % 400) || (((yr) % 100) && !((yr) % 4)))

typedef struct
{
    uint8_t Seconds;
    uint8_t Minutes;
    uint8_t Hours;
    uint8_t Days;
    uint8_t Weekdays;
    uint8_t Months;
    uint16_t Years;
} UTCTimeStruct_t;

typedef enum
{
    RTC_SECOND,
    RTC_MINUTE,
    RTC_HOUR,
    RTC_WEEKDAY,
    RTC_DAY,
    RTC_MONTH,
    RTC_YEAR,
    RTC_CONTROL,

}RTC_PCF8523_T;

extern uint32_t SystemSecoundTime;
extern UTCTimeStruct_t SystemUTCTime;
void second_time_to_utc_time(UTCTimeStruct_t *utc_time, uint32_t sec_time);
void utc_time_to_second_time(UTCTimeStruct_t utc_time, uint32_t *sec_time);
void system_clock_init_by_second(uint32_t sec_time);
void system_clock_init_by_utc(UTCTimeStruct_t utc_time);
void RTC_PCF8523_Init(void);
void RTC_SET_TIME(UTCTimeStruct_t utc_setting);
void rtc_configuration(void);
void initernal_rtc_module_init(void);
void iHomeClockRTC_Init(void);


#endif /* CLOCK_APPLICATION_H */
