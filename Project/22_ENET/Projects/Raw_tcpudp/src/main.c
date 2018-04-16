/*!
    \file  main.c
    \brief enet demo 
*/

/*
    Copyright (C) 2017 GigaDevice

    2017-07-28, V1.0.0, demo for GD32F30x
*/

#include "gd32f30x.h"
#include "netconf.h"
#include "main.h"
#include "lwip/tcp.h"
#include "lwip/lwip_timers.h"
#include "gd32f307c_eval.h"
#include "hello_gigadevice.h"
#include "udp_echo.h"
#include "tcp_client.h"

#define SYSTEMTICK_PERIOD_MS  10

__IO uint32_t g_localtime = 0; /* for creating a time reference incremented by 10ms */
uint32_t g_timedelay;

/*!
    \brief      main function
    \param[in]  none
    \param[out] none
    \retval     none
*/
int main(void)
{
    gd_eval_com_init(EVAL_COM1);
    gd_eval_key_init(KEY_TAMPER, KEY_MODE_EXTI);
    /* setup ethernet system(GPIOs, clocks, MAC, DMA, systick) */ 
    enet_system_setup();

    /* initilaize the LwIP stack */
    lwip_stack_init();

    /* initilaize the tcp server: telnet 8000 */
    hello_gigadevice_init();
    /* initilaize the tcp client: echo 1026 */
    tcp_client_init();
    /* initilaize the udp: echo 1025 */
    udp_echo_init();

    while(1){
      
#ifndef USE_ENET_INTERRUPT      
        /* check if any packet received */
        if(enet_rxframe_size_get()){ 
            /* process received ethernet packet */
            lwip_pkt_handle();
        }        
#endif /* USE_ENET_INTERRUPT */
        
        /* handle periodic timers for LwIP */
#ifdef TIMEOUT_CHECK_USE_LWIP
        sys_check_timeouts();
        
#ifdef USE_DHCP
        lwip_dhcp_process_handle();
#endif /* USE_DHCP */ 
        
#else
        lwip_periodic_handle(g_localtime);
#endif /* TIMEOUT_CHECK_USE_LWIP */
    }   
}

/*!
    \brief      insert a delay time
    \param[in]  ncount: number of 10ms periods to wait for
    \param[out] none
    \retval     none
*/
void delay_10ms(uint32_t ncount)
{
    /* capture the current local time */
    g_timedelay = g_localtime + ncount;  

    /* wait until the desired delay finish */  
    while(g_timedelay > g_localtime){
    }
}

/*!
    \brief      updates the system local time
    \param[in]  none
    \param[out] none
    \retval     none
*/
void time_update(void)
{
    g_localtime += SYSTEMTICK_PERIOD_MS;
}

/* retarget the C library printf function to the USART */
int fputc(int ch, FILE *f)
{
    usart_data_transmit(EVAL_COM1, (uint8_t) ch);
    while (RESET == usart_flag_get(EVAL_COM1, USART_FLAG_TBE));
    return ch;
}
