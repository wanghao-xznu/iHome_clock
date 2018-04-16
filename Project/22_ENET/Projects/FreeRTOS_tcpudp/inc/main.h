/*!
    \file  main.h
    \brief the header file of main 
*/

/*
    Copyright (C) 2017 GigaDevice

    2017-07-28, V1.0.0, demo for GD32F30x
*/

#ifndef MAIN_H
#define MAIN_H

#include "gd32f30x.h"
#include "stdint.h"
#include "gd32f30x_enet_eval.h"
#include "app_flash.h"

#define IHOME_CLOCK_CONFIG_OFFSET               (0x08040000 - 0x800)
#define IHOME_CLOCK_CONFIG_MAC_OFFSET           IHOME_CLOCK_CONFIG_OFFSET
#define IHOME_CLOCK_CONFIG_MAC_ASCII_OFFSET     (IHOME_CLOCK_CONFIG_MAC_OFFSET + 12*4)
#define IHOME_CLOCK_CONFIG_SN_OFFSET            (IHOME_CLOCK_CONFIG_MAC_ASCII_OFFSET + 12*4)


#define CLOCK_SW_VERSION_MAJOR          0x01
#define CLOCK_SW_VERSION_MINOR          0x00
#define CLOCK_SW_VERSION_REVERSION      0x01
#define CLOCK_SW_VERSION_BUILD          0x00

#define CLOCK_HW_VERSION_MAJOR          0x02
#define CLOCK_HW_VERSION_MINOR          0x03
#define CLOCK_HW_VERSION_REVERSION      0x2D
#define CLOCK_HW_VERSION_BUILD          0x00

#define CLOCK_SN_VERSION_REV_0          0x4A
#define CLOCK_SN_VERSION_REV_1          0x59
#define CLOCK_SN_VERSION_YEAR_0         0x14
#define CLOCK_SN_VERSION_YEAR_1         0x12
#define CLOCK_SN_VERSION_MONTH          0x03
#define CLOCK_SN_VERSION_DAY            0x18
#define CLOCK_SN_VERSION_BATCH_0        0x00
#define CLOCK_SN_VERSION_BATCH_1        0x01


#define USE_DHCP       /* enable DHCP, if disabled static address is used */

/* MAC address: MAC_ADDR0:MAC_ADDR1:MAC_ADDR2:MAC_ADDR3:MAC_ADDR4:MAC_ADDR5 */
//default MAC Value 9c:43:1e:10:00:01
#define MAC_ADDR0   ((uint8_t)((REG32(IHOME_CLOCK_CONFIG_OFFSET + 0*4))*0x10 + (0xf&REG32(IHOME_CLOCK_CONFIG_OFFSET + 1*4))))
#define MAC_ADDR1   ((uint8_t)((REG32(IHOME_CLOCK_CONFIG_OFFSET + 2*4))*0x10 + (0xf&REG32(IHOME_CLOCK_CONFIG_OFFSET + 3*4))))
#define MAC_ADDR2   ((uint8_t)((REG32(IHOME_CLOCK_CONFIG_OFFSET + 4*4))*0x10 + (0xf&REG32(IHOME_CLOCK_CONFIG_OFFSET + 5*4))))
#define MAC_ADDR3   ((uint8_t)((REG32(IHOME_CLOCK_CONFIG_OFFSET + 6*4))*0x10 + (0xf&REG32(IHOME_CLOCK_CONFIG_OFFSET + 7*4))))
#define MAC_ADDR4   ((uint8_t)((REG32(IHOME_CLOCK_CONFIG_OFFSET + 8*4))*0x10 + (0xf&REG32(IHOME_CLOCK_CONFIG_OFFSET + 9*4))))
#define MAC_ADDR5   ((uint8_t)((REG32(IHOME_CLOCK_CONFIG_OFFSET + 10*4))*0x10 + (0xf&REG32(IHOME_CLOCK_CONFIG_OFFSET + 11*4))))


#define IHOME_CLOCK_SN_M_0    ((uint8_t)(REG32(IHOME_CLOCK_CONFIG_SN_OFFSET + 0*4)))
#define IHOME_CLOCK_SN_M_1    ((uint8_t)(REG32(IHOME_CLOCK_CONFIG_SN_OFFSET + 1*4)))
#define IHOME_CLOCK_SN_M_2    ((uint8_t)(REG32(IHOME_CLOCK_CONFIG_SN_OFFSET + 2*4)))
#define IHOME_CLOCK_SN_M_3    ((uint8_t)(REG32(IHOME_CLOCK_CONFIG_SN_OFFSET + 3*4)))
#define IHOME_CLOCK_SN_M_4    ((uint8_t)(REG32(IHOME_CLOCK_CONFIG_SN_OFFSET + 4*4)))
#define IHOME_CLOCK_SN_M_5    ((uint8_t)(REG32(IHOME_CLOCK_CONFIG_SN_OFFSET + 5*4)))
#define IHOME_CLOCK_SN_M_6    ((uint8_t)(REG32(IHOME_CLOCK_CONFIG_SN_OFFSET + 6*4)))
#define IHOME_CLOCK_SN_M_7    ((uint8_t)(REG32(IHOME_CLOCK_CONFIG_SN_OFFSET + 7*4)))
#define IHOME_CLOCK_SN_M_8    ((uint8_t)(REG32(IHOME_CLOCK_CONFIG_SN_OFFSET + 8*4)))
#define IHOME_CLOCK_SN_M_9    ((uint8_t)(REG32(IHOME_CLOCK_CONFIG_SN_OFFSET + 9*4)))

#define IHOME_CLOCK_SN_YEAR_0    ((uint8_t)(REG32(IHOME_CLOCK_CONFIG_SN_OFFSET + 10*4)))
#define IHOME_CLOCK_SN_YEAR_1    ((uint8_t)(REG32(IHOME_CLOCK_CONFIG_SN_OFFSET + 11*4)))
#define IHOME_CLOCK_SN_MONTH_0    ((uint8_t)(REG32(IHOME_CLOCK_CONFIG_SN_OFFSET + 12*4)))
#define IHOME_CLOCK_SN_MONTH_1    ((uint8_t)(REG32(IHOME_CLOCK_CONFIG_SN_OFFSET + 13*4)))
#define IHOME_CLOCK_SN_DAY_0    ((uint8_t)(REG32(IHOME_CLOCK_CONFIG_SN_OFFSET + 14*4)))
#define IHOME_CLOCK_SN_DAY_1    ((uint8_t)(REG32(IHOME_CLOCK_CONFIG_SN_OFFSET + 15*4)))

#define IHOME_CLOCK_SN_NUM_0    ((uint8_t)(REG32(IHOME_CLOCK_CONFIG_SN_OFFSET + 16*4)))
#define IHOME_CLOCK_SN_NUM_1    ((uint8_t)(REG32(IHOME_CLOCK_CONFIG_SN_OFFSET + 17*4)))
#define IHOME_CLOCK_SN_NUM_2    ((uint8_t)(REG32(IHOME_CLOCK_CONFIG_SN_OFFSET + 18*4)))
#define IHOME_CLOCK_SN_NUM_3    ((uint8_t)(REG32(IHOME_CLOCK_CONFIG_SN_OFFSET + 19*4)))
#define IHOME_CLOCK_SN_NUM_4    ((uint8_t)(REG32(IHOME_CLOCK_CONFIG_SN_OFFSET + 20*4)))
 
/* static IP address: IP_ADDR0.IP_ADDR1.IP_ADDR2.IP_ADDR3 */
#define IP_ADDR0   192
#define IP_ADDR1   168
#define IP_ADDR2   128
#define IP_ADDR3   138


/* net mask */
#define NETMASK_ADDR0   255
#define NETMASK_ADDR1   255
#define NETMASK_ADDR2   255
#define NETMASK_ADDR3   0

/* gateway address */
#define GW_ADDR0   192
#define GW_ADDR1   168
#define GW_ADDR2   128
#define GW_ADDR3   1  

/* MII and RMII mode selection */
//#define RMII_MODE  // user have to provide the 50 MHz clock by soldering a 50 MHz oscillator
#define MII_MODE

/* clock the PHY from external 25MHz crystal (only for MII mode) */
#ifdef  MII_MODE
#define PHY_CLOCK_MCO
#endif

#define CONSOLE_RX_LEN  100

#endif /* MAIN_H */
