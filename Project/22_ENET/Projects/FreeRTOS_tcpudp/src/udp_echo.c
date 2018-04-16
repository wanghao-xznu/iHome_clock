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
#include "udp_echo.h"
#include "tcp_client.h"
#include "FreeRTOS.h"
#include "timers.h"
#include <string.h>
#include <stdio.h>
#include "gd32f30x.h"
#include "lwip/memp.h"
#include "main.h"
#include "lwip/api.h"
#include "lwip/sockets.h"
#include "local_message.h"
#include "display_dp.h"

#define UDP_TASK_PRIO       ( tskIDLE_PRIORITY + 5)
#define MAX_BUF_SIZE        64
extern bool SecondShowFlag;

bool ClockLoginFlag = FALSE;
bool FactoryTestLEDFlasg = FALSE;
uint32_t clock_server_ip[4] = {0};
uint32_t factory_test_ip[4] = {0};
TimerHandle_t PushInfoTimer = NULL;






/*!
    \brief      udp echo task
    \param[in]  arg: user supplied argument
    \param[out] none
    \retval     none
*/
int clock_server_udp_sockfd;
struct sockaddr_in clock_server_addr;
struct sockaddr_in factory_test_addr;

void PushInfoTimerCallback( TimerHandle_t pxTimer )
{
    //printf("Push Clock Infomation callback \n ");
    extern xQueueHandle hMainTaskQueueHandle;
    EVENT_MSG setting_ip_msg;
    setting_ip_msg.event = EVENT_PUSH_CLOCK_INFO;
    if( hMainTaskQueueHandle != NULL)
    {
        xQueueSend(hMainTaskQueueHandle, &setting_ip_msg, 0x0);
    }
}
static void udp_task(void *arg)
{
    int  ret, recvnum, sockfd = -1;
    int udp_port = 1025;
    struct sockaddr_in rmt_addr, bod_addr;
    char buf[MAX_BUF_SIZE];
    u32_t len;

    /* set up address to connect to */
    rmt_addr.sin_family = AF_INET;
    rmt_addr.sin_port = htons(udp_port);
    rmt_addr.sin_addr.s_addr = IPADDR_BROADCAST;

    bod_addr.sin_family = AF_INET;
    bod_addr.sin_port = htons(udp_port);
    bod_addr.sin_addr.s_addr = htons(INADDR_ANY);
    if(PushInfoTimer == NULL)
    {
        PushInfoTimer = xTimerCreate( "PushInfoTimer", // Just a text name, not used by the kernel.
                                    1000 / portTICK_RATE_MS, // The timer period in ticks. 1s, don't change!
                                    pdTRUE, // The timers will auto-reload themselves when they expire.
                                    ( void *) 0, // Assign each timer a unique id equal to its array index.
                                    PushInfoTimerCallback
                                  );

    }

    while (1) {
        vTaskDelay(3000 / portTICK_PERIOD_MS);
        /* create the socket */
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd < 0) {
            continue;
        }

        ret = bind(sockfd, (struct sockaddr *)&bod_addr, sizeof(bod_addr));

        if (ret < 0) {
            lwip_close(sockfd);
            sockfd = -1;
            continue;
        }

        len = sizeof(rmt_addr);
        /* reveive packets from rmt_addr, and limit a reception to MAX_BUF_SIZE bytes */
        int nNetTimeout=5000;
        int ret = setsockopt(sockfd,SOL_SOCKET, SO_RCVTIMEO,&nNetTimeout,sizeof(int));

        while (-1 != sockfd) {
            clock_server_udp_sockfd = sockfd;
            memset(buf, 0x00, MAX_BUF_SIZE);
            recvnum = recvfrom(sockfd, buf, MAX_BUF_SIZE, 0, (struct sockaddr *)&rmt_addr, &len);
            if((recvnum == -1) || (recvnum == 0))
            {
                //printf("Outing of blocking Because recv timer out 5 second recvnum = %d\n ", recvnum);
            }
            else
            {
                printf("length of receive recvnum = %d \n ", recvnum);
                if(((buf[0] == 0x4a)&&(buf[1] == 0x59)) && ((buf[2] + (buf[3]<<8) + (buf[4]<<16) + (buf[5]<<24)) == recvnum))
                {
                    switch (buf[7])
                    {
                        case 0x00:
                        {
                            printf("Login CMD \n ");
                            printf("Get Server IP Address Here  \n ");
                            printf("Server IP address: %d.%d.%d.%d \r\n", buf[13], buf[12], buf[11], buf[10]);
                            clock_server_ip[3] = buf[10];
                            clock_server_ip[2] = buf[11];
                            clock_server_ip[1] = buf[12];
                            clock_server_ip[0] = buf[13];

                            memset(buf, 0x00, MAX_BUF_SIZE);
                            {
                                /* the below is for ack frame head length = 10*/
                                buf[0] = 0x4A;
                                buf[1] = 0x59;
                                buf[2] = 0x32;
                                buf[3] = 0x00;
                                buf[4] = 0x00;
                                buf[5] = 0x00;
                                buf[6] = 0x01;  /*here means clock send to server */
                                buf[7] = 0x01;  /* here is CMD ID*/
                                buf[8] = 0xff;
                                buf[9] = 0xff;
                            }
                            {
                                /* the below is for current CLOCK IP length = 4*/
                                extern struct netif g_mynetif;
                                uint32_t ip_address = g_mynetif.ip_addr.addr;
                                buf[10] = ip4_addr4_16(&ip_address);
                                buf[11] = ip4_addr3_16(&ip_address);
                                buf[12] = ip4_addr2_16(&ip_address);
                                buf[13] = ip4_addr1_16(&ip_address);
                            }
                            {
                                /* the below is for current CLOCK MAC length = 6*/
                                buf[14] = MAC_ADDR0;
                                buf[15] = MAC_ADDR1;
                                buf[16] = MAC_ADDR2;
                                buf[17] = MAC_ADDR3;
                                buf[18] = MAC_ADDR4;
                                buf[19] = MAC_ADDR5;
                            }
                            {
                                /* the below is for current CLOCK status lentgh = 1*/
                                buf[20] = SecondShowFlag;
                            }
                            {
                                /* the below is for CLOCK sw and hw version length = 8*/
                                buf[21] = CLOCK_SW_VERSION_MAJOR;
                                buf[22] = CLOCK_SW_VERSION_MINOR;
                                buf[23] = CLOCK_SW_VERSION_REVERSION;
                                buf[24] = CLOCK_SW_VERSION_BUILD;
                                
                                buf[25] = CLOCK_HW_VERSION_MAJOR;
                                buf[26] = CLOCK_HW_VERSION_MINOR;
                                buf[27] = CLOCK_HW_VERSION_REVERSION;
                                buf[28] = CLOCK_HW_VERSION_BUILD;

                                /* the below is for CLOCK information length = 21*/
                                buf[29] = IHOME_CLOCK_SN_M_0;
                                buf[30] = IHOME_CLOCK_SN_M_1;
                                buf[31] = IHOME_CLOCK_SN_M_2;
                                buf[32] = IHOME_CLOCK_SN_M_3;
                                buf[33] = IHOME_CLOCK_SN_M_4;
                                buf[34] = IHOME_CLOCK_SN_M_5;
                                buf[35] = IHOME_CLOCK_SN_M_6;
                                buf[36] = IHOME_CLOCK_SN_M_7;
                                buf[37] = IHOME_CLOCK_SN_M_8;
                                buf[38] = IHOME_CLOCK_SN_M_9;

                                buf[39] = IHOME_CLOCK_SN_YEAR_0;
                                buf[40] = IHOME_CLOCK_SN_YEAR_1;
                                buf[41] = IHOME_CLOCK_SN_MONTH_0;
                                buf[42] = IHOME_CLOCK_SN_MONTH_1;
                                buf[43] = IHOME_CLOCK_SN_DAY_0;
                                buf[44] = IHOME_CLOCK_SN_DAY_1;
                                
                                buf[45] = IHOME_CLOCK_SN_NUM_0;
                                buf[46] = IHOME_CLOCK_SN_NUM_1;
                                buf[47] = IHOME_CLOCK_SN_NUM_2;
                                buf[48] = IHOME_CLOCK_SN_NUM_3;
                                buf[49] = IHOME_CLOCK_SN_NUM_4;
                            }
                            struct ip_addr ipaddr;
                            IP4_ADDR(&ipaddr, clock_server_ip[0], clock_server_ip[1], clock_server_ip[2], clock_server_ip[3]);
                            /* set up address to connect to */
                            clock_server_addr.sin_family = AF_INET;
                            clock_server_addr.sin_port = htons(udp_port);
                            clock_server_addr.sin_addr.s_addr = ipaddr.addr;
                            sendto(sockfd, buf, buf[2], MSG_DONTWAIT, (struct sockaddr *)&clock_server_addr, sizeof(clock_server_addr));
                            if(ClockLoginFlag == FALSE)
                            {
                                ClockLoginFlag = TRUE;
                                tcp_client_init();
                                if (PushInfoTimer != NULL)
                                {
                                    xTimerStart(PushInfoTimer, 0);
                                }
                            }
                            break;
                        }
                        case 0x04:
                        {
                            printf("Setting Second Display CMD \n ");
                            if((buf[10] != MAC_ADDR0) || (buf[11] != MAC_ADDR1) || (buf[12] != MAC_ADDR2) || \
                               (buf[13] != MAC_ADDR3) || (buf[14] != MAC_ADDR4) || (buf[15] != MAC_ADDR5))
                            {
                                printf("The Mac of this package not match! \n ");
                                break;
                            }
                            if(ClockLoginFlag == TRUE)
                            {
                                if(buf[16])
                                {
                                    SecondShowFlag = TRUE;
                                }
                                else
                                {
                                    SecondShowFlag = FALSE;
                                }
                            }
                            memset(buf, 0x00, MAX_BUF_SIZE);
                            {
                                /* the below is for ack frame head length = 10*/
                                buf[0] = 0x4A;
                                buf[1] = 0x59;
                                buf[2] = 0x25;
                                buf[3] = 0x00;
                                buf[4] = 0x00;
                                buf[5] = 0x00;
                                buf[6] = 0x01;  /*here means clock send to server */
                                buf[7] = 0x05;  /* here is CMD ID*/
                                buf[8] = 0xff;
                                buf[9] = 0xff;
                            }
                            {
                                /* the below is for current CLOCK IP length = 4*/
                                extern struct netif g_mynetif;
                                uint32_t ip_address = g_mynetif.ip_addr.addr;
                                buf[10] = ip4_addr4_16(&ip_address);
                                buf[11] = ip4_addr3_16(&ip_address);
                                buf[12] = ip4_addr2_16(&ip_address);
                                buf[13] = ip4_addr1_16(&ip_address);
                            }
                            {
                                /* the below is for current CLOCK MAC length = 6*/
                                buf[14] = MAC_ADDR0;
                                buf[15] = MAC_ADDR1;
                                buf[16] = MAC_ADDR2;
                                buf[17] = MAC_ADDR3;
                                buf[18] = MAC_ADDR4;
                                buf[19] = MAC_ADDR5;
                            }
                            {
                                /* the below is for current CLOCK status lentgh = 1*/
                                buf[20] = SecondShowFlag;
                            }
                            sendto(sockfd, buf, buf[2], MSG_DONTWAIT, (struct sockaddr *)&clock_server_addr, sizeof(clock_server_addr));
                            break;
                        }
                        default:
                            break;
                    }
                }
                else if(((buf[0] == 0x49)&&(buf[1] == 0x48)) && ((buf[2] + (buf[3]<<8) + (buf[4]<<16) + (buf[5]<<24)) == recvnum))
                {
                    extern uint8_t console_rx_buf[CONSOLE_RX_LEN];
                    memset(console_rx_buf, 0, CONSOLE_RX_LEN);
                    memcpy(console_rx_buf,buf + 14,recvnum);
                    switch (buf[7])
                    {
                        case 0x30:
                        {
                            printf("Receive UDP SET MAC and SN CMD \n ");
                            //send message to main task
                            extern xQueueHandle hMainTaskQueueHandle;
                            EVENT_MSG assign_mac_and_sn_msg;
                            assign_mac_and_sn_msg.event = EVENT_ASSIGN_MAC_AND_SN;
                            assign_mac_and_sn_msg.pBuf = console_rx_buf;
                            assign_mac_and_sn_msg.msg_length = recvnum + 1;
                            if( hMainTaskQueueHandle != NULL)
                            {
                                xQueueSend(hMainTaskQueueHandle, &assign_mac_and_sn_msg, 0x0);
                            }
                            break;
                        }
                        case 0x31:
                        {
                            printf("factory test IP address: %d.%d.%d.%d \r\n", buf[13], buf[12], buf[11], buf[10]);
                            factory_test_ip[3] = buf[10];
                            factory_test_ip[2] = buf[11];
                            factory_test_ip[1] = buf[12];
                            factory_test_ip[0] = buf[13];
                            {
                                /* the below is for ack frame head length = 10*/
                                buf[0] = 0x49;
                                buf[1] = 0x48;
                                buf[2] = 47;
                                buf[3] = 0x00;
                                buf[4] = 0x00;
                                buf[5] = 0x00;
                                buf[6] = 0x01;  /*here means clock send to server */
                                buf[7] = 0x32;  /* here is CMD ID*/
                                buf[8] = 0xff;
                                buf[9] = 0xff;
                            }
                            {
                                /* the below is for current CLOCK IP length = 4*/
                                extern struct netif g_mynetif;
                                uint32_t ip_address = g_mynetif.ip_addr.addr;
                                buf[10] = ip4_addr4_16(&ip_address);
                                buf[11] = ip4_addr3_16(&ip_address);
                                buf[12] = ip4_addr2_16(&ip_address);
                                buf[13] = ip4_addr1_16(&ip_address);
                            }
                            {
                                /* the below is for current CLOCK MAC length = 6*/
                                for(uint8_t i = 0; i < 12; i++)
                                {
                                    buf[14 + i] = (uint8_t)REG32(IHOME_CLOCK_CONFIG_MAC_ASCII_OFFSET + i*4);
                                }
                            }
                            {
                                /* the below is for CLOCK information length = 21*/
                                buf[26] = IHOME_CLOCK_SN_M_0;
                                buf[27] = IHOME_CLOCK_SN_M_1;
                                buf[28] = IHOME_CLOCK_SN_M_2;
                                buf[29] = IHOME_CLOCK_SN_M_3;
                                buf[30] = IHOME_CLOCK_SN_M_4;
                                buf[31] = IHOME_CLOCK_SN_M_5;
                                buf[32] = IHOME_CLOCK_SN_M_6;
                                buf[33] = IHOME_CLOCK_SN_M_7;
                                buf[34] = IHOME_CLOCK_SN_M_8;
                                buf[35] = IHOME_CLOCK_SN_M_9;

                                buf[36] = IHOME_CLOCK_SN_YEAR_0;
                                buf[37] = IHOME_CLOCK_SN_YEAR_1;
                                buf[38] = IHOME_CLOCK_SN_MONTH_0;
                                buf[39] = IHOME_CLOCK_SN_MONTH_1;
                                buf[40] = IHOME_CLOCK_SN_DAY_0;
                                buf[41] = IHOME_CLOCK_SN_DAY_1;
                                
                                buf[42] = IHOME_CLOCK_SN_NUM_0;
                                buf[43] = IHOME_CLOCK_SN_NUM_1;
                                buf[44] = IHOME_CLOCK_SN_NUM_2;
                                buf[45] = IHOME_CLOCK_SN_NUM_3;
                                buf[46] = IHOME_CLOCK_SN_NUM_4;
                            }
                            struct ip_addr ipaddr;
                            IP4_ADDR(&ipaddr, factory_test_ip[0], factory_test_ip[1], factory_test_ip[2], factory_test_ip[3]);
                            /* set up address to connect to */
                            factory_test_addr.sin_family = AF_INET;
                            factory_test_addr.sin_port = htons(udp_port);
                            factory_test_addr.sin_addr.s_addr = ipaddr.addr;
                            sendto(sockfd, buf, buf[2], MSG_DONTWAIT, (struct sockaddr *)&factory_test_addr, sizeof(factory_test_addr));
                            break;
                        }
                        case 0x33:
                        {
                            if((buf[14] != MAC_ADDR0) || (buf[15] != MAC_ADDR1) || (buf[16] != MAC_ADDR2) || \
                               (buf[17] != MAC_ADDR3) || (buf[18] != MAC_ADDR4) || (buf[19] != MAC_ADDR5))
                            {
                                printf("The Mac of this package not match! \n ");
                                break;
                            }
                            else
                            {
                                if(buf[20] == 0x01)
                                {
                                    printf("Receive factory Test! \n ");
                                    FactoryTestLEDFlasg = TRUE;
                                    extern xQueueHandle hMainTaskQueueHandle;
                                    EVENT_MSG assign_mac_and_sn_msg;
                                    assign_mac_and_sn_msg.event = EVENT_FACTORY_TEST;
                                    if( hMainTaskQueueHandle != NULL)
                                    {
                                        xQueueSend(hMainTaskQueueHandle, &assign_mac_and_sn_msg, 0x0);
                                    }
                                }
                                else if(buf[20] == 0x00)
                                {
                                    FactoryTestLEDFlasg = FALSE;
                                    printf("Do reboot for finish factory Test! \n ");
                                    NVIC_SystemReset();
                                }
                            }
                            break;
                        }
                        default:
                            printf("UDP SET CMD ID ERROR! \n ");
                            break;
                    }
                }
                else
                {
                    printf("Error Package!!!!! \n ");
                }
            }
        }

        lwip_close(sockfd);
    }
}


/*!
    \brief      initialize the udp_echo application
    \param[in]  none
    \param[out] none
    \retval     none
*/
void udp_echo_init(void)
{
    xTaskCreate(udp_task, "UDP", DEFAULT_THREAD_STACKSIZE, NULL, UDP_TASK_PRIO, NULL);
}
