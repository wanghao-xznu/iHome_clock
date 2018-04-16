/*!
    \file  gd32f307c_eval.h
    \brief definitions for GD32f307C_EVAL's leds, keys and COM ports hardware resources
*/

/*
    Copyright (C) 2017 GigaDevice

    2017-07-28, V1.0.0, demo for GD32F30x
*/

#ifndef APP_FLASH_H
#define APP_FLASH_H

#ifdef cplusplus
 extern "C" {
#endif

/* There is 256K for code size from 0x08000000 ~ 0x08040000  */
/* We prepare 0x20000 for User Data */
#define FMC_PAGE_SIZE           ((uint16_t)0x800U)
#define FMC_WRITE_START_ADDR    ((uint32_t)0x08020100U)
#define FMC_WRITE_END_ADDR      ((uint32_t)0x08020800U)


void FsLogicInit(void);
bool FsLogicWrite(uint32_t address, uint32_t value);
bool FsLogicRead(uint32_t address, uint32_t *value);

#ifdef cplusplus
}
#endif

#endif /* APP_FLASH_H */
