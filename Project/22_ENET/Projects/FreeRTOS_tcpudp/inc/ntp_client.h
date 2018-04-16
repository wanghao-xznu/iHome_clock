/*!
    \file  udp_echo.h
    \brief the header file of udp_echo 
*/

/*
    Copyright (C) 2017 GigaDevice

    2017-07-28, V1.0.0, demo for GD32F30x
*/

#ifndef NTP_CLINET_H
#define NTP_CLINET_H

#define NTP_PKT_LEN            48   //48bytes
#define NTP_PORT               123

#define NTP_LI_NO_WARNING          (0x00<<6) //no warning
#define NTP_VERSION               (4/* NTP Version 4*/<<3) 
#define NTP_MODE_CLIENT            0x03
#define NTP_MODE_SERVER            0x04
#define NTP_MODE_BROADCAST         0x05
#define NTP_MODE_MASK              0x07
#define RECEIVE_TS_OFFSET          32 // Receive_Timestamp offset
#define TRANSMIT_TS_OFFSET         40 //Transmit_Timestamp offset

/* number of seconds between 1900 and 1970 */
#define DIFF_SEC_1900_1970         (2208988800)
#define SEC_TIME_ZONE              + (8*60*60) //Beijing,GMT+8

extern xQueueHandle hNTPQueueHandle;

typedef struct
{
    uint32_t coarse;
    uint32_t fine;
}NTP_TIME;

typedef struct
{
    uint8_t leap_ver_mode;
    uint8_t startum;
    int8_t poll;
    int8_t precision;
    int32_t root_dealy;
    int32_t root_dispersion;
    int32_t reference_identifier;
    NTP_TIME reference_timestamp;
    NTP_TIME originage_timestamp;
    NTP_TIME receive_timestamp;
    NTP_TIME transmit_timestamp;
}NTP_PACK;

/* function declarations */
/* initialize the tcp_client application */
void ntp_client_init(void);

#endif /* NTP_CLINET_H */
