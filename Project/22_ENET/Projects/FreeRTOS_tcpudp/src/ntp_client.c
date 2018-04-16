/*!
    \file  udp_echo.c
    \brief UDP demo program
*/

/*
    Copyright (C) 2017 GigaDevice

    2017-07-28, V1.0.0, demo for GD32F30x
*/

#include "lwip/opt.h"
#include "lwip/tcp.h"
#include "lwip/sys.h"
#include "ntp_client.h"
#include <string.h>
#include <stdio.h>
#include "gd32f30x.h"
#include "lwip/memp.h"
#include "main.h"
#include "lwip/api.h"
#include "lwip/sockets.h"
#include "time.h"
#include "local_message.h"
#include "clock_application.h"

#define NTP_TASK_PRIO       ( tskIDLE_PRIORITY + 5)

xQueueHandle hNTPQueueHandle = NULL ;
bool localNTPSeverRequestSuccess = FALSE;

static void ntp_task(void *arg)
{
    int  ret, recvnum, sockfd = -1;
    int udp_port = NTP_PORT;
    
    struct sockaddr_in bod_addr;
    u32_t len;
    
    uint8_t ntp_buf[NTP_PKT_LEN];
    EVENT_MSG ntp_client_msg;

    bod_addr.sin_family = AF_INET;
    bod_addr.sin_port = htons(udp_port);
    bod_addr.sin_addr.s_addr = htons(INADDR_ANY);
    
    hNTPQueueHandle = xQueueCreate(0x08, sizeof(EVENT_MSG));

    while (1)
    {
        vTaskDelay(3000 / portTICK_PERIOD_MS);
        /* create the socket */
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd < 0) {
            vTaskDelay(10 * 1000 / portTICK_PERIOD_MS);
            continue;
        }

        ret = bind(sockfd, (struct sockaddr *)&bod_addr, sizeof(bod_addr));

        if (ret < 0) {
            lwip_close(sockfd);
            sockfd = -1;
            vTaskDelay(10 * 1000 / portTICK_PERIOD_MS);
            continue;
        }

        int nNetTimeout=1000;
        int ret = setsockopt(sockfd,SOL_SOCKET, SO_RCVTIMEO,&nNetTimeout,sizeof(int));
        while (-1 != sockfd) 
        {
            if (xQueueReceive(hNTPQueueHandle, &ntp_client_msg, portMAX_DELAY) == pdPASS)
            {
                switch (ntp_client_msg.event)
                {
                    case EVNET_NTP_TIME_UPDATA_FROM_ALI_SERVER:
                        {
                            /* the below parameter for ali NTP server */
                            struct sockaddr_in ali_ntp_addr;
                            struct ip_addr ali_ipaddr;
                            IP4_ADDR(&ali_ipaddr, 182, 92, 12, 11);
                            /* set up address to connect to */
                            ali_ntp_addr.sin_family = AF_INET;
                            ali_ntp_addr.sin_port = htons(udp_port);
                            ali_ntp_addr.sin_addr.s_addr = ali_ipaddr.addr;
                            /* the below do NTP time update */
                            memset(ntp_buf, 0, NTP_PKT_LEN);
                            ntp_buf[0] = NTP_LI_NO_WARNING|NTP_VERSION|NTP_MODE_CLIENT;
                            sendto(sockfd, ntp_buf, NTP_PKT_LEN, MSG_DONTWAIT, (struct sockaddr *)&ali_ntp_addr, sizeof(ali_ntp_addr));
                            
                            recvnum = recvfrom(sockfd, ntp_buf, NTP_PKT_LEN, 0, (struct sockaddr *)&ali_ntp_addr, &len);
                            
                            if(recvnum == NTP_PKT_LEN)
                            {
                                if(((ntp_buf[0]& NTP_MODE_MASK) == NTP_MODE_SERVER)|| ((ntp_buf[0] & NTP_MODE_MASK) == NTP_MODE_BROADCAST))
                                {
                                    // extract time from packet
                                    uint32_t Transmit_Timestamp = 0;
                                    UTCTimeStruct_t local_utc_time = {0};
                                    Transmit_Timestamp =ntp_buf[TRANSMIT_TS_OFFSET]<<24 | ntp_buf[TRANSMIT_TS_OFFSET+1]<<16|
                                                        ntp_buf[TRANSMIT_TS_OFFSET+2]<<8 |ntp_buf[TRANSMIT_TS_OFFSET+3];
                                    // start conver time format
                                    Transmit_Timestamp += SEC_TIME_ZONE;
                                    
                                    second_time_to_utc_time(&local_utc_time, Transmit_Timestamp);
                                    local_utc_time.Years = local_utc_time.Years - 100;
                                    utc_time_to_second_time(local_utc_time, &Transmit_Timestamp);
                                    #if 0
                                    printf("time data: year:%d,month:%d,date:%d, weekday:%d,Hour:%d,minute:%d,Second:%d! \n", \
                                            local_utc_time.Years, \
                                            local_utc_time.Months, \
                                            local_utc_time.Days, \
                                            local_utc_time.Weekdays, \
                                            local_utc_time.Hours, \
                                            local_utc_time.Minutes, \
                                            local_utc_time.Seconds);
                                    printf("SystemSecoundTime = %d;Transmit_Timestamp = %d;SystemSecoundTime - Transmit_Timestamp = %d \n", \
                                            SystemSecoundTime, Transmit_Timestamp, SystemSecoundTime - Transmit_Timestamp);
                                    #endif
                                    uint32_t detal = (SystemSecoundTime > Transmit_Timestamp)?(SystemSecoundTime - Transmit_Timestamp):(Transmit_Timestamp - SystemSecoundTime);
                                    if(detal >= 2)
                                    {
                                        printf("We need do calibration by UTP time ===>detal = %d \n", detal );
                                        RTC_SET_TIME(local_utc_time);
                                    }
                                    else
                                    {
                                        #if 0
                                        printf("NO need do calibration by UTP time ===>detal = %d \n", detal );
                                        #endif
                                    }
                                }
                            }
                            
                            break;
                        }
                    case EVNET_NTP_TIME_START:
                        {
                            break;
                        }
                    case EVNET_NTP_TIME_STOP:
                        {
                            break;
                        }
                    case EVNET_NTP_TIME_UPDATA_FROM_LOCAL_SERVER:
                    {
                        /* the below parameter for local NTP server */
                        struct sockaddr_in local_server_ntp_addr;
                        struct ip_addr local_server_ipaddr;
                        extern uint32_t clock_server_ip[4];
                        IP4_ADDR(&local_server_ipaddr, clock_server_ip[0], clock_server_ip[1], clock_server_ip[2], clock_server_ip[3]);
                        if(local_server_ipaddr.addr == 0)
                        {
                            printf("Server Addr equal zeor or other reasons direct return \n");
                            break;
                        }
                        else
                        {
                            printf("Got Server Addr IP \n");
                        }
                        /* set up address to connect to */
                        local_server_ntp_addr.sin_family = AF_INET;
                        local_server_ntp_addr.sin_port = htons(udp_port);
                        local_server_ntp_addr.sin_addr.s_addr = local_server_ipaddr.addr;
                        
                        memset(ntp_buf, 0, NTP_PKT_LEN);
                        ntp_buf[0] = NTP_LI_NO_WARNING|NTP_VERSION|NTP_MODE_CLIENT;
                        sendto(sockfd, ntp_buf, NTP_PKT_LEN, MSG_DONTWAIT, (struct sockaddr *)&local_server_ntp_addr, sizeof(local_server_ntp_addr));
                        
                        recvnum = recvfrom(sockfd, ntp_buf, NTP_PKT_LEN, 0, (struct sockaddr *)&local_server_ntp_addr, &len);
                        
                        if(recvnum == NTP_PKT_LEN)
                        {
                            if(((ntp_buf[0]& NTP_MODE_MASK) == NTP_MODE_SERVER)|| ((ntp_buf[0] & NTP_MODE_MASK) == NTP_MODE_BROADCAST))
                            {
                                // extract time from packet
                                uint32_t Transmit_Timestamp = 0;
                                UTCTimeStruct_t local_utc_time = {0};
                                Transmit_Timestamp =ntp_buf[TRANSMIT_TS_OFFSET]<<24 | ntp_buf[TRANSMIT_TS_OFFSET+1]<<16|
                                                    ntp_buf[TRANSMIT_TS_OFFSET+2]<<8 |ntp_buf[TRANSMIT_TS_OFFSET+3];
                                // start conver time format
                                Transmit_Timestamp += SEC_TIME_ZONE;
                                
                                second_time_to_utc_time(&local_utc_time, Transmit_Timestamp);
                                local_utc_time.Years = local_utc_time.Years - 100;
                                utc_time_to_second_time(local_utc_time, &Transmit_Timestamp);
                                uint32_t detal = (SystemSecoundTime > Transmit_Timestamp)?(SystemSecoundTime - Transmit_Timestamp):(Transmit_Timestamp - SystemSecoundTime);
                                if(detal >= 2)
                                {
                                    printf("We need do calibration by UTP time ===>detal = %d \n", detal );
                                    RTC_SET_TIME(local_utc_time);
                                }
                                localNTPSeverRequestSuccess = TRUE;
                            }
                        }
                        break;
                    }
                    default:
                        break;
                }
            }
        }

        lwip_close(sockfd);
        vTaskDelay(10 * 1000 / portTICK_PERIOD_MS);
    }

}

/*!
    \brief      initialize the ntp client application
    \param[in]  none
    \param[out] none
    \retval     none
*/
void ntp_client_init(void)
{
    xTaskCreate(ntp_task, "ntp_task", DEFAULT_THREAD_STACKSIZE, NULL, NTP_TASK_PRIO, NULL);
}
