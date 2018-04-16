/*!
    \file  main.h
    \brief the header file of main 
*/

/*
    Copyright (C) 2017 GigaDevice

    2017-07-28, V1.0.0, demo for GD32F30x
*/

#ifndef LOCAL_MESSAGE_H
#define LOCAL_MESSAGE_H

#include "gd32f30x.h"
#include "stdint.h"
#include "gd32f30x_enet_eval.h"


typedef struct
{
    uint16_t event;
    uint16_t msg_length;
    void *pBuf;
    uint32_t parm;
} EVENT_MSG;

typedef enum
{
    EVNET_NTP_TIME_UPDATA_FROM_ALI_SERVER,
    EVNET_NTP_TIME_START,
    EVNET_NTP_TIME_STOP,
    EVNET_NTP_TIME_UPDATA_FROM_LOCAL_SERVER,

} EVENT_NTP_TYPE;

typedef enum
{
    EVENT_SECOND,
    EVENT_MINUTE,
    EVENT_HOUR,
    EVENT_DAY,
    EVENT_PUSH_CLOCK_INFO,
    EVENT_ASSIGN_MAC_AND_SN,
    EVENT_FACTORY_TEST,
}EVENT_MAIN_TYPE;

#endif /* LOCAL_MESSAGE_H */
