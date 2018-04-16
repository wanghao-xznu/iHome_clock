/*!
    \file  gd32f30x_it.c
    \brief interrupt service routines
*/

/*
    Copyright (C) 2017 GigaDevice

    2017-07-28, V1.0.0, demo for GD32F30x
*/

#include "gd32f30x.h"
#include "gd32f30x_it.h"
#include "main.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "lwip/sys.h"
#include "local_message.h"

extern xSemaphoreHandle g_rx_semaphore;

/*!
    \brief      this function handles NMI exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void NMI_Handler(void)
{
}

/*!
    \brief      this function handles HardFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void HardFault_Handler(void)
{
    /* if Hard Fault exception occurs, go to infinite loop */
    while (1) {
        NVIC_SystemReset();
    }
}

/*!
    \brief      this function handles MemManage exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void MemManage_Handler(void)
{
    /* if Memory Manage exception occurs, go to infinite loop */
    while (1) {
        NVIC_SystemReset();
    }
}

/*!
    \brief      this function handles BusFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void BusFault_Handler(void)
{
    /* if Bus Fault exception occurs, go to infinite loop */
    while (1) {
        NVIC_SystemReset();
    }
}

/*!
    \brief      this function handles UsageFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void UsageFault_Handler(void)
{
    /* if Usage Fault exception occurs, go to infinite loop */
    while (1) {
        NVIC_SystemReset();
    }
}

/*!
    \brief      this function handles ethernet interrupt request
    \param[in]  none
    \param[out] none
    \retval     none
*/
void ENET_IRQHandler(void)
{
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    /* frame received */
    if (SET == enet_interrupt_flag_get(ENET_DMA_INT_FLAG_RS)) {
        /* give the semaphore to wakeup LwIP task */
        xSemaphoreGiveFromISR(g_rx_semaphore, &xHigherPriorityTaskWoken);
    }

    /* clear the enet DMA Rx interrupt pending bits */
    enet_interrupt_flag_clear(ENET_DMA_INT_FLAG_RS_CLR);
    enet_interrupt_flag_clear(ENET_DMA_INT_FLAG_NI_CLR);

    /* switch tasks if necessary */
    if (pdFALSE != xHigherPriorityTaskWoken) {
        portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
    }
}

/*!
    \brief      this function handles USART0 exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
extern uint8_t console_rx_buf[CONSOLE_RX_LEN];
void USART0_IRQHandler(void)
{
    static int counter = 0;
    if(RESET != usart_interrupt_flag_get(USART0, USART_INT_FLAG_RBNE)){
        /* read one byte from the receive data register */
        if(counter < (CONSOLE_RX_LEN - 1))
        {
            console_rx_buf[counter++] = (uint8_t)usart_data_receive(USART0);
        }
    }
    if(RESET != usart_interrupt_flag_get(USART0, USART_INT_FLAG_RT)){
        console_rx_buf[counter++] = 0;
        printf("RX interrupt flag time out! \n ");
        usart_interrupt_flag_clear(USART0,USART_INT_FLAG_RT);
        //send message to main task
        extern xQueueHandle hMainTaskQueueHandle;
        EVENT_MSG assign_mac_and_sn_msg;
        assign_mac_and_sn_msg.event = EVENT_ASSIGN_MAC_AND_SN;
        assign_mac_and_sn_msg.pBuf = console_rx_buf;
        assign_mac_and_sn_msg.msg_length = counter;
        if( hMainTaskQueueHandle != NULL)
        {
            xQueueSendFromISR(hMainTaskQueueHandle, &assign_mac_and_sn_msg, 0x0);
        }
        counter = 0;
    }
}

