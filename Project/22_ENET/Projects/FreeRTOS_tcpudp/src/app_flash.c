/*!
    \file  gd32f307c_eval.c
    \brief firmware functions to manage leds, keys, COM ports
*/

/*
    Copyright (C) 2017 GigaDevice

    2017-07-28, V1.0.0, demo for GD32F30x
*/

#include <gd32f30x.h>
#include "string.h"
#include "gd32f307c_eval.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "stdio.h"
#include "app_flash.h"
#include "gd32f30x_fmc.h"

uint32_t FsBackUpBuf[FMC_PAGE_SIZE/sizeof(uint32_t)] = {0};

void FsLogicInit(void)
{
    #if 0
    uint32_t testValue = 0;
    FsLogicWrite(FMC_WRITE_START_ADDR, 0x55555555);
    FsLogicRead(FMC_WRITE_START_ADDR, &testValue);
    printf("testValue = 0x%x \n", testValue);
    FsLogicWrite(FMC_WRITE_START_ADDR + 4, 0xaaaaaaaa);
    FsLogicRead(FMC_WRITE_START_ADDR + 4, &testValue);
    printf("testValue = 0x%x \n", testValue);
    FsLogicWrite(FMC_WRITE_START_ADDR + 8, 0xbbbbbbbb);
    FsLogicRead(FMC_WRITE_START_ADDR + 8, &testValue);
    printf("testValue = 0x%x \n", testValue);
    
    FsLogicRead(FMC_WRITE_START_ADDR, &testValue);
    printf("testValue = 0x%x \n", testValue);
    FsLogicRead(FMC_WRITE_START_ADDR + 4, &testValue);
    printf("testValue = 0x%x \n", testValue);
    FsLogicRead(FMC_WRITE_START_ADDR + 8, &testValue);
    printf("testValue = 0x%x \n", testValue);
    #endif
}

bool FsLogicWrite(uint32_t address, uint32_t value)
{
    if((address < FMC_WRITE_START_ADDR) || (address >= FMC_WRITE_END_ADDR ))
    {
        return FALSE;
    }
    memcpy(FsBackUpBuf, (uint32_t *)(address - address%FMC_PAGE_SIZE), FMC_PAGE_SIZE);
    
    
    FsBackUpBuf[(address%FMC_PAGE_SIZE)/sizeof(uint32_t)] = value;
    
    /* unlock the flash program/erase controller */
    fmc_unlock();
    /* erase the flash pages */
    fmc_page_erase(address - address%FMC_PAGE_SIZE);
    /* clear all pending flags */
    fmc_flag_clear(FMC_FLAG_BANK0_END | FMC_FLAG_BANK0_WPERR | FMC_FLAG_BANK0_PGERR );
    /* program flash */
    uint32_t i = 0;
    for(uint32_t addr = address - address%FMC_PAGE_SIZE; addr < address - address%FMC_PAGE_SIZE + FMC_PAGE_SIZE; addr = addr + 4)
    {
        fmc_word_program(addr, FsBackUpBuf[i++]);
        fmc_flag_clear(FMC_FLAG_BANK0_END | FMC_FLAG_BANK0_WPERR | FMC_FLAG_BANK0_PGERR );
    }


    /* lock the main FMC after the erase operation */
    fmc_lock();
    
    return TRUE;
}

bool FsLogicRead(uint32_t address, uint32_t *value)
{
    if((address < FMC_WRITE_START_ADDR) || (address >= FMC_WRITE_END_ADDR ))
    {
        return FALSE;
    }
    *value = REG32(address);
    
    return TRUE;
}


