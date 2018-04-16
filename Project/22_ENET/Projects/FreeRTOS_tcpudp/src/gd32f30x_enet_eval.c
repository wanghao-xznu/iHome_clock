/*!
    \file  gd32f30x_enet_eval.c
    \brief ethernet hardware configuration
*/

/*
    Copyright (C) 2017 GigaDevice

    2017-07-28, V1.0.0, demo for GD32F30x
*/

#include "gd32f30x_enet.h"
#include "gd32f30x_enet_eval.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "main.h"

static __IO uint32_t enet_init_status = 0;

static void enet_gpio_config(void);
static void enet_mac_dma_config(void);
static void nvic_configuration(void);

bool enet_get_phy_link_status(void)
{
    uint16_t phy_value = 0U;
    uint32_t timeout = 0U;
    /* here we need read twice refer RTL8201 */
    do {
        enet_phy_write_read(ENET_PHY_READ, PHY_ADDRESS, PHY_REG_BSR, &phy_value);
        phy_value &= PHY_LINKED_STATUS;
        vTaskDelay(1000);
        timeout++;
    } while ((RESET == phy_value) && (timeout < 5));
    
    if(phy_value == PHY_LINKED_STATUS)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
/*!
    \brief      setup ethernet system(GPIOs, clocks, MAC, DMA, systick)
    \param[in]  none
    \param[out] none
    \retval     none
*/
ErrStatus enet_system_setup(void)
{
    ErrStatus enet_state = ERROR;
    nvic_configuration();

    /* configure the GPIO ports for ethernet pins */
    enet_gpio_config();

    /* configure the ethernet MAC/DMA */
    enet_mac_dma_config();

    if (0 == enet_init_status) {
        return enet_state;
    }
    else
    {
        enet_interrupt_enable(ENET_DMA_INT_NIE);
        enet_interrupt_enable(ENET_DMA_INT_RIE);
        enet_state = SUCCESS;
        return enet_state;
    }


}

/*!
    \brief      configures the ethernet interface
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void enet_mac_dma_config(void)
{
    /* enable ethernet clock  */
    rcu_periph_clock_enable(RCU_ENET);
    rcu_periph_clock_enable(RCU_ENETTX);
    rcu_periph_clock_enable(RCU_ENETRX);

    /* reset ethernet on AHB bus */
    enet_deinit();

    /* configure the parameters which are usually less cared for enet initialization */
//  enet_initpara_config(HALFDUPLEX_OPTION, ENET_CARRIERSENSE_ENABLE|ENET_RECEIVEOWN_ENABLE|ENET_RETRYTRANSMISSION_DISABLE|ENET_BACKOFFLIMIT_10|ENET_DEFERRALCHECK_DISABLE);
//  enet_initpara_config(DMA_OPTION, ENET_FLUSH_RXFRAME_ENABLE|ENET_SECONDFRAME_OPT_ENABLE|ENET_NORMAL_DESCRIPTOR);

#ifdef CHECKSUM_BY_HARDWARE
    enet_init_status = enet_init(ENET_AUTO_NEGOTIATION, ENET_AUTOCHECKSUM_DROP_FAILFRAMES, ENET_BROADCAST_FRAMES_PASS);
#else
    enet_init_status = enet_init(ENET_AUTO_NEGOTIATION, ENET_NO_AUTOCHECKSUM, ENET_BROADCAST_FRAMES_PASS);
#endif /* CHECKSUM_BY_HARDWARE */

}

/*!
    \brief      configures the nested vectored interrupt controller
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void nvic_configuration(void)
{
    nvic_vector_table_set(NVIC_VECTTAB_FLASH, 0x0);
    nvic_irq_enable(ENET_IRQn, 2, 0);
}

/*!
    \brief      configures the different GPIO ports
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void enet_gpio_config(void)
{
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_GPIOC);
    rcu_periph_clock_enable(RCU_AF);

    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_MAX, GPIO_PIN_8);

#ifdef MII_MODE

#ifdef PHY_CLOCK_MCO
    /* output HXTAL clock (25MHz) on CKOUT0 pin(PA8) to clock the PHY */
    rcu_ckout0_config(RCU_CKOUT0SRC_HXTAL);
#endif /* PHY_CLOCK_MCO */

    gpio_ethernet_phy_select(GPIO_ENET_PHY_MII);

#elif defined RMII_MODE
    /* choose PLL2 to get 50MHz from 25/5*10 on CKOUT0 pin (PA8) to clock the PHY */
    rcu_pll2_config(RCU_PLL2_MUL10);
    rcu_osci_on(RCU_PLL2_CK);
    rcu_osci_stab_wait(RCU_PLL2_CK);
    rcu_ckout0_config(RCU_CKOUT0SRC_CKPLL2);
    gpio_ethernet_phy_select(GPIO_ENET_PHY_RMII);

#endif /* MII_MODE */

#ifdef MII_MODE

    /* PA0: ETH_MII_CRS */
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_MAX, GPIO_PIN_0);
    /* PA1: ETH_MII_RX_CLK */
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_MAX, GPIO_PIN_1);
    /* PA2: ETH_MDIO */
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_MAX, GPIO_PIN_2);
    /* PA3: ETH_MII_COL */
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_MAX, GPIO_PIN_3);
    /* PA7: ETH_MII_RX_DV */
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_MAX, GPIO_PIN_7);

    /* PB0: ETH_MII_RXD2 */
    gpio_init(GPIOB, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_MAX, GPIO_PIN_0);
    /* PB1: ETH_MII_RXD3 */
    gpio_init(GPIOB, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_MAX, GPIO_PIN_1);
    /* PB8: ETH_MII_TXD3 */
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_MAX, GPIO_PIN_8);
    /* PB10: ETH_MII_RX_ER */
    gpio_init(GPIOB, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_MAX, GPIO_PIN_10);
    /* PB11: ETH_MII_TX_EN */
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_MAX, GPIO_PIN_11);
    /* PB12: ETH_MII_TXD0 */
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_MAX, GPIO_PIN_12);
    /* PB13: ETH_MII_TXD1 */
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_MAX, GPIO_PIN_13);

    /* PC1: ETH_MDC */
    gpio_init(GPIOC, GPIO_MODE_AF_PP, GPIO_OSPEED_MAX, GPIO_PIN_1);
    /* PC2: ETH_MII_TXD2 */
    gpio_init(GPIOC, GPIO_MODE_AF_PP, GPIO_OSPEED_MAX, GPIO_PIN_2);
    /* PC3: ETH_MII_TX_CLK */
    gpio_init(GPIOC, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_MAX, GPIO_PIN_3);
    /* PC4: ETH_MII_RXD0 */
    gpio_init(GPIOC, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_MAX, GPIO_PIN_4);
    /* PC5: ETH_MII_RXD1 */
    gpio_init(GPIOC, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_MAX, GPIO_PIN_5);

#elif defined RMII_MODE

    /* PA1: ETH_RMII_REF_CLK */
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_MAX, GPIO_PIN_1);
    /* PA2: ETH_MDIO */
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_MAX, GPIO_PIN_2);
    /* PA7: ETH_RMII_CRS_DV */
    gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_MAX, GPIO_PIN_7);

    /* PB11: ETH_RMII_TX_EN */
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_MAX, GPIO_PIN_11);
    /* PB12: ETH_RMII_TXD0 */
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_MAX, GPIO_PIN_12);
    /* PB13: ETH_RMII_TXD1 */
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_MAX, GPIO_PIN_13);

    /* PC1: ETH_MDC */
    gpio_init(GPIOC, GPIO_MODE_AF_PP, GPIO_OSPEED_MAX, GPIO_PIN_1);
    /* PC4: ETH_RMII_RXD0 */
    gpio_init(GPIOC, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_MAX, GPIO_PIN_4);
    /* PC5: ETH_RMII_RXD1 */
    gpio_init(GPIOC, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_MAX, GPIO_PIN_5);

#endif /* MII_MODE */
}
