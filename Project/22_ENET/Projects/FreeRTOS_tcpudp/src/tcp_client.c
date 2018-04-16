/*!
    \file  tcp_client.c
    \brief TCP client demo program
*/

/*
    Copyright (C) 2017 GigaDevice

    2017-07-28, V1.0.0, demo for GD32F30x
*/

#include "lwip/opt.h"
#include "lwip/sys.h"
#include "tcp_client.h"
#include <string.h>
#include <stdio.h>
#include "gd32f30x.h"
#include "main.h"
#include "lwip/tcp.h"
#include "lwip/memp.h"
#include "lwip/api.h"
#include "lwip/sockets.h"
#include "clock_application.h"
#include "local_message.h"

#define TCP_CLIENT_TASK_PRIO            ( tskIDLE_PRIORITY + 5)
#define MAX_BUF_SIZE                    64
#define TIME_WAITING_FOR_CONNECT        ( ( portTickType ) 500 )

extern uint32_t clock_server_ip[4];



/*!
    \brief      tcp_client task
    \param[in]  arg: user supplied argument
    \param[out] none
    \retval     none
*/
static void tcp_client_task(void *arg)
{
    int  ret, recvnum, sockfd = -1;
    int tcp_port = 1026;
    struct sockaddr_in PeerPC_addr, iHomeClk_addr;
    char buf[MAX_BUF_SIZE];
    struct ip_addr ipaddr;

    IP4_ADDR(&ipaddr, clock_server_ip[0], clock_server_ip[1], clock_server_ip[2], clock_server_ip[3]);

    /* set up address to connect to */
    PeerPC_addr.sin_family = AF_INET;
    PeerPC_addr.sin_port = htons(tcp_port);
    PeerPC_addr.sin_addr.s_addr = ipaddr.addr;

    iHomeClk_addr.sin_family = AF_INET;
    iHomeClk_addr.sin_port = htons(tcp_port);
    iHomeClk_addr.sin_addr.s_addr = htons(INADDR_ANY);
    printf("TCP Task Init Success! \n ");

    while (1) {
        vTaskDelay(500 / portTICK_PERIOD_MS);
        /* create the socket */
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            continue;
        }

        ret = bind(sockfd, (struct sockaddr *)&iHomeClk_addr, sizeof(iHomeClk_addr));
        if (ret < 0) {
            lwip_close(sockfd);
            sockfd = -1;
            continue;
        }

        ret = listen(sockfd, 5);
        if (ret != 0) {
            lwip_close(sockfd);
            sockfd = -1;
            continue;
        }
        printf("Listen OK \n");


        while (-1 != sockfd) {
            /* accept */
            int len = sizeof(PeerPC_addr);
            int new_fd = accept(sockfd, (struct sockaddr *)&PeerPC_addr, (socklen_t *)&len);
            printf("accept OK new_fd = %d\n",new_fd);
            if (new_fd < 0) {
                lwip_close(sockfd);
                sockfd = -1;
                continue;
            }
            recvnum = recv(new_fd, buf, MAX_BUF_SIZE, 0);
            while(recvnum > 0)
            {
                printf("recv length = %d \n", recvnum);
                if(
                    (buf[0] == 0x4a) && (buf[1] == 0x59) && ((buf[2] + (buf[3]<<8) + (buf[4]<<16) + (buf[5]<<24)) == recvnum) && (buf[7] == 0x02) && \
                    (buf[14] == MAC_ADDR0) && (buf[15] == MAC_ADDR1) && (buf[16] == MAC_ADDR2) && \
                    (buf[17] == MAC_ADDR3) && (buf[18] == MAC_ADDR4) && (buf[19] == MAC_ADDR5)
                  )
                {
                    printf("client New IP address: %d.%d.%d.%d \r\n", buf[13], buf[12], buf[11], buf[10]);
                    printf("client Old IP address: %d.%d.%d.%d \r\n", buf[23], buf[22], buf[21], buf[20]);
                    {
                        /* Here call netif for setting IP */
                        extern struct netif g_mynetif;
                        struct ip_addr ipaddr = g_mynetif.ip_addr;
                        struct ip_addr netmask = g_mynetif.netmask;
                        struct ip_addr gw = g_mynetif.gw;
                        /* static address used */
                        netif_set_down(&g_mynetif);
                        IP4_ADDR(&ipaddr, buf[13], buf[12], buf[11], buf[10] );
                        {
                            /* Seems no need set netmask and gw here TODO*/
                            #if 0
                            IP4_ADDR(&netmask, NETMASK_ADDR0, NETMASK_ADDR1, NETMASK_ADDR2, NETMASK_ADDR3);
                            IP4_ADDR(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
                            #endif
                            printf("netmask = 0x%x; gw = 0x%x \n",(unsigned int)g_mynetif.netmask.addr, (unsigned int)g_mynetif.gw.addr);
                        }
                        netif_set_addr(&g_mynetif, &ipaddr, &netmask, &gw);
                        netif_set_up(&g_mynetif);
                    }
                }
                else
                {
                    printf("Error IP setting Package \n ");
                }
                recvnum = recv(new_fd, buf, MAX_BUF_SIZE, 0);
            }
            lwip_close(new_fd);
        }

        lwip_close(sockfd);
        sockfd = -1;
    }

}


/*!
    \brief      initialize the tcp_client application
    \param[in]  none
    \param[out] none
    \retval     none
*/
xTaskHandle  hTCPClientTaskHandle = NULL;
void tcp_client_init(void)
{
    if(hTCPClientTaskHandle == NULL)
    {
        xTaskCreate(tcp_client_task, "TCP_CLIENT", DEFAULT_THREAD_STACKSIZE, NULL, TCP_CLIENT_TASK_PRIO, &hTCPClientTaskHandle);
    }
    else
    {
        printf("TCP_CLIENT Task have been create");
    }
}

