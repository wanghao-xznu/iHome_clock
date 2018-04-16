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
#include "tcpip.h"
#include "gd32f307c_eval.h"
#include "hello_gigadevice.h"
#include "tcp_client.h"
#include "udp_echo.h"
#include "ntp_client.h"
#include "local_message.h"
#include "display_dp.h"
#include "clock_application.h"
#include "app_flash.h"
#include "sockets.h"
#include "string.h"

#define INIT_TASK_PRIO          ( tskIDLE_PRIORITY + 1 )
#define DHCP_TASK_PRIO          ( tskIDLE_PRIORITY + 4 )
#define MAIN_TASK_PRIO          ( tskIDLE_PRIORITY + 2 )
#define CHECK_LINK_TASK_PRIO    ( tskIDLE_PRIORITY + 2 )


xQueueHandle hMainTaskQueueHandle;

extern struct netif g_mynetif;
static bool EthInitStatusSuccess = FALSE;
bool LocalLinkStatus = FALSE;

void main_task(void *pvParameters);
void init_task(void *pvParameters);
void linkCheck_task(void *pvParameters);

uint8_t console_rx_buf[CONSOLE_RX_LEN] = {0};


/*!
    \brief      main function
    \param[in]  none
    \param[out] none
    \retval     none
*/
int main(void)
{
    /* configure 4 bits pre-emption priority */
    nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);

    /* init task */
    xTaskCreate(init_task, "INIT", configMINIMAL_STACK_SIZE * 2, NULL, INIT_TASK_PRIO, NULL);

    /* start scheduler */
    vTaskStartScheduler();

    while (1) {
    }
}

/*!
    \brief      init task
    \param[in]  pvParameters not used
    \param[out] none
    \retval     none
*/
void init_task(void *pvParameters)
{
    /* initilaize consol uart */
    {
        gd_eval_com_init(EVAL_COM1);
    }

    /* dispaly ENET mac address and SN */
    {
        //uint32_t sn_num = REG32(IHOME_CLOCK_MAC_FLASH_OFFSET);
        printf("iHomecomm Clock SN      -->%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c \n", \
                IHOME_CLOCK_SN_M_0, \
                IHOME_CLOCK_SN_M_1, \
                IHOME_CLOCK_SN_M_2, \
                IHOME_CLOCK_SN_M_3, \
                IHOME_CLOCK_SN_M_4, \
                IHOME_CLOCK_SN_M_5, \
                IHOME_CLOCK_SN_M_6, \
                IHOME_CLOCK_SN_M_7, \
                IHOME_CLOCK_SN_M_8, \
                IHOME_CLOCK_SN_M_9, \
                IHOME_CLOCK_SN_YEAR_0, \
                IHOME_CLOCK_SN_YEAR_1, \
                IHOME_CLOCK_SN_MONTH_0, \
                IHOME_CLOCK_SN_MONTH_1, \
                IHOME_CLOCK_SN_DAY_0, \
                IHOME_CLOCK_SN_DAY_1, \
                IHOME_CLOCK_SN_NUM_0, \
                IHOME_CLOCK_SN_NUM_1, \
                IHOME_CLOCK_SN_NUM_2, \
                IHOME_CLOCK_SN_NUM_3, \
                IHOME_CLOCK_SN_NUM_4 \
                );
        if((MAC_ADDR0 == 0xff)&&(MAC_ADDR1 == 0xff)&&(MAC_ADDR2 == 0xff)&&(MAC_ADDR3 == 0xff)&&(MAC_ADDR4 == 0xff)&&(MAC_ADDR5 == 0xff))
        {
            printf("iHome Clock comming from factory! Set default MAC Value 9c:43:1e:10:00:01 \n");
            uint8_t default_mac[12] = {9,12,4,3,1,14,1,0,0,0,0,1};
            fmc_unlock();
            for(uint8_t i = 0; i < 12; i++)
            {
                fmc_word_program(IHOME_CLOCK_CONFIG_MAC_OFFSET + i*4, default_mac[i]);
                fmc_flag_clear(FMC_FLAG_BANK0_END | FMC_FLAG_BANK0_WPERR | FMC_FLAG_BANK0_PGERR );
            }
            fmc_lock();
        }
        printf("iHomecomm Clock MAC ADDR-->0x%x:0x%x:0x%x:0x%x:0x%x:0x%x \n",MAC_ADDR0,MAC_ADDR1,MAC_ADDR2,MAC_ADDR3,MAC_ADDR4,MAC_ADDR5);
    }

    /* Test logic FS */
    {
        //FsLogicInit();
    }

    /* initilaize display module */
    {
        DisplayDeviceInit();
        DisplayDriverInit();
    }
    /* initilaize RTC clock module */
    {
        iHomeClockRTC_Init();
    }

    /* configure ethernet (GPIOs, clocks, MAC, DMA) */
    if(enet_system_setup() != ERROR)
    {
        /* initilaize the LwIP stack */
        EthInitStatusSuccess = TRUE;
        lwip_stack_init();
        LocalLinkStatus = TRUE;
    }
    else
    {
        EthInitStatusSuccess = FALSE;
        printf("enet system setup fail, \n ");
    }

    #if 1
    udp_echo_init();
    #endif
    ntp_client_init();

#ifdef USE_DHCP
    /* start DHCP client */
    xTaskCreate(dhcp_task, "DHCP", configMINIMAL_STACK_SIZE * 2, NULL, DHCP_TASK_PRIO, NULL);
#endif /* USE_DHCP */

    /* start toogle LED task every 250ms */
    xTaskCreate(main_task, "MAIN_TASK", configMINIMAL_STACK_SIZE, NULL, MAIN_TASK_PRIO, NULL);
    
    xTaskCreate(linkCheck_task, "CHECK_LINK_TASK", DEFAULT_THREAD_STACKSIZE, NULL, CHECK_LINK_TASK_PRIO, NULL);

    for ( ;; ) {
        vTaskDelete(NULL);
    }
}

/*!
    \brief      main task
    \param[in]  pvParameters not used
    \param[out] none
    \retval     none
*/
void main_task(void *pvParameters)
{
    EVENT_MSG msg;
    hMainTaskQueueHandle = xQueueCreate(0x10, sizeof(EVENT_MSG));
    for ( ;; )
    {
        if (xQueueReceive(hMainTaskQueueHandle, &msg, portMAX_DELAY) == pdPASS)
        {
            switch (msg.event)
            {
                case EVENT_PUSH_CLOCK_INFO:
                {
                    //printf("Howie Test IP setting LOG \n");
                    char buf[50];
                    {
                        /* the below is for ack frame head length = 10*/
                        buf[0] = 0x4A;
                        buf[1] = 0x59;
                        buf[2] = 0x1A;
                        buf[3] = 0x00;
                        buf[4] = 0x00;
                        buf[5] = 0x00;
                        buf[6] = 0x01;  /*here means clock send to server */
                        buf[7] = 0x03;  /* here is CMD ID*/
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
                        /* the below is for current TIME */
                        buf[20] = SystemUTCTime.Years;
                        buf[21] = SystemUTCTime.Months;
                        buf[22] = SystemUTCTime.Days;
                        buf[23] = SystemUTCTime.Hours;
                        buf[24] = SystemUTCTime.Minutes;
                        buf[25] = SystemUTCTime.Seconds;
                    }
                    extern int clock_server_udp_sockfd;
                    extern struct sockaddr_in clock_server_addr;
                    sendto(clock_server_udp_sockfd, buf, buf[2], MSG_DONTWAIT, (struct sockaddr *)&clock_server_addr, sizeof(clock_server_addr));
                    break;
                }
                case EVENT_ASSIGN_MAC_AND_SN:
                {
                    printf("got assign mac and sn CMD! length = %d; buf addr = 0x%p \n ", msg.msg_length,msg.pBuf);
                    /* unlock the flash program/erase controller */
                    fmc_unlock();
                    /* erase the flash config sector */
                    fmc_page_erase(IHOME_CLOCK_CONFIG_OFFSET);
                    /* clear all pending flags */
                    fmc_flag_clear(FMC_FLAG_BANK0_END | FMC_FLAG_BANK0_WPERR | FMC_FLAG_BANK0_PGERR );

                    uint8_t *pData = msg.pBuf;

                    char *mac_address = strstr((char *)pData,"SETMAC=");
                    char *sn_address = strstr((char *)pData,"SETSN=");
                    if( mac_address== NULL)
                    {
                        printf("No SETMAC CMD FOUND \n ");
                        break;
                    }
                    else
                    {
                        uint8_t mac_cmd_len = strlen("SETMAC=");
                        /* save mac address*/
                        for(uint8_t i = 0; i < 12; i++)//SETMAC=AA11BB3344ff#SETSN=JY********18040700001#
                        {
                            uint32_t tmp = *(mac_address + mac_cmd_len + i);
                            //printf("value tmp = 0x%x \n ",tmp);
                            //printf("char tmp = %c \n ",tmp);
                            fmc_word_program(IHOME_CLOCK_CONFIG_MAC_ASCII_OFFSET + i*4, tmp);
                            fmc_flag_clear(FMC_FLAG_BANK0_END | FMC_FLAG_BANK0_WPERR | FMC_FLAG_BANK0_PGERR );
                            if((tmp >= '0')&&(tmp <= '9'))
                            {
                                tmp = tmp - '0';
                            }
                            else if((tmp >= 'a') && (tmp <= 'f'))
                            {
                                tmp = tmp - 'a' + 10;
                            }
                            else if((tmp >= 'A') && (tmp <= 'F'))
                            {
                                tmp = tmp - 'A' + 10;
                            }
                            else
                            {
                                printf("Error MAC Address!! \n ");
                                break;
                            }
                            printf("After value tmp = 0x%x \n ",tmp);
                            //printf("After char tmp = %c \n ",tmp);
                            fmc_word_program(IHOME_CLOCK_CONFIG_MAC_OFFSET + i*4, tmp);
                            fmc_flag_clear(FMC_FLAG_BANK0_END | FMC_FLAG_BANK0_WPERR | FMC_FLAG_BANK0_PGERR );
                        }
                    }
                    if(sn_address == NULL)
                    {
                        printf("No SETSN CMD FOUND \n ");
                        break;
                    }
                    else
                    {
                        uint8_t sn_cmd_len = strlen("SETSN=");
                        /*save SN for product*/
                        for(uint8_t i = 0; i < 21; i++)
                        {
                            fmc_word_program(IHOME_CLOCK_CONFIG_SN_OFFSET + i*4, *(sn_address + sn_cmd_len + i));
                            fmc_flag_clear(FMC_FLAG_BANK0_END | FMC_FLAG_BANK0_WPERR | FMC_FLAG_BANK0_PGERR );
                        }
                    }
                    /* lock the main FMC after the erase operation */
                    fmc_lock();
                    //NVIC_SystemReset();
                    break;
                }
                case EVENT_FACTORY_TEST:
                {
                    extern bool FactoryTestLEDFlasg;
                    printf("Do factory Test! FactoryTestLEDFlasg = %d\n ",FactoryTestLEDFlasg);
                    while(FactoryTestLEDFlasg == TRUE)
                    {
                        if (DisPlayTimer != NULL)
                        {
                            xTimerStop(DisPlayTimer, 0);
                        }
                        /*power off all LED*/
                        DisPlayPowerOFF();
                        vTaskDelay(2000 / portTICK_PERIOD_MS);
                        /*power on all LED*/
                        DisPlayPowerON();
                        vTaskDelay(2000 / portTICK_PERIOD_MS);
                        /*show special char*/
                        DisPlaySpecialChar();
                        vTaskDelay(2000 / portTICK_PERIOD_MS);
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }
}

/*!
    \brief      main task
    \param[in]  pvParameters not used
    \param[out] none
    \retval     none
*/
void linkCheck_task(void *pvParameters)
{
    while(1)
    {
        if(enet_get_phy_link_status() == TRUE)
        {
            //printf("link check Thread, we are link now \n ");
            if(EthInitStatusSuccess == FALSE)
            {
                if(enet_system_setup() != ERROR)
                {
                    /* initilaize the LwIP stack */
                    printf("Here is enet system setup success! \n ");
                    EthInitStatusSuccess = TRUE;
                    lwip_stack_init();
                    LocalLinkStatus = TRUE;
                }
            }
            else
            {
                LocalLinkStatus = TRUE;
            }
        }
        else
        {
            //printf("link check Thread, we are dislink now \n ");
            LocalLinkStatus = FALSE;
        }
        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
}

/* retarget the C library printf function to the USART */
int fputc(int ch, FILE *f)
{
    usart_data_transmit(EVAL_COM1, (uint8_t) ch);
    while (RESET == usart_flag_get(EVAL_COM1, USART_FLAG_TBE));
    return ch;
}

