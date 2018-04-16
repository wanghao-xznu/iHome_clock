/*!
    \file  gd32f30x_enet_eval.h
    \brief the header file of gd32f30x_enet_eval 
*/

/*
    Copyright (C) 2017 GigaDevice

    2017-07-28, V1.0.0, demo for GD32F30x
*/

#ifndef GD32F30x_ENET_EVAL_H
#define GD32F30x_ENET_EVAL_H

/* function declarations */
/* setup ethernet system(GPIOs, clocks, MAC, DMA, systick) */
ErrStatus enet_system_setup(void);
bool enet_get_phy_link_status(void);

#endif /* GD32F30x_ENET_EVAL_H */
