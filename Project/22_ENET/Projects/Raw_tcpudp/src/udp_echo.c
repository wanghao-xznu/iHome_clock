/*!
    \file  udp_echo.c
    \brief UDP demo program 
*/

/*
    Copyright (C) 2017 GigaDevice

    2017-07-28, V1.0.0, demo for GD32F30x
*/

#include "udp_echo.h"  
#include "lwip/udp.h"
#include <string.h>
#include <stdio.h>
#include "gd32f30x.h"

static void udp_echo_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, struct ip_addr *addr, u16_t port);


/*!
    \brief      called when a data is received on the udp connection 
    \param[in]  args: the user argument
    \param[in]  pcb: the udp_pcb that has received the data
    \param[in]  p: the packet buffer
    \param[in]  addr: pointer on the receive IP address
    \param[in]  port: receive port number
    \param[out] none
    \retval     none
*/ 
static void udp_echo_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, struct ip_addr *addr, u16_t port)
{
    struct ip_addr destaddr = *addr;

    if(p != NULL){
        udp_sendto(pcb, p, &destaddr, port); 
        /* end of processing, we free the pbuf */
        pbuf_free(p);
    }
}

/*!
    \brief      initialize the udp_client application
    \param[in]  none
    \param[out] none
    \retval     none
*/
void udp_echo_init(void)
{
    struct udp_pcb *udppcb;  
  
    /* create a new UDP control block */
    udppcb = udp_new();
    /* assign to the new pcb a local IP address and a port number */
    udp_bind(udppcb, IP_ADDR_ANY, 1025);
    udp_recv(udppcb,udp_echo_recv,NULL);

}

