/*!
    \file  readme.txt
    \brief description of EXMC NandFlash demo
*/

/*
    Copyright (C) 2017 GigaDevice

    2017-07-28, V1.0.0, demo for GD32F30x
*/

  This demo is based on the GD32307C-EVAL-V1.0 board, it shows how to use EXMC peripheral
to control NandFlash. This board uses EXMC_BANK1_NAND to support NAND memory.
  This demo shows the write and read data operation process of NandFlash by EXMC module. 
Firstly, read NAND_ID. If the operation fails, this demo will print out the failure information 
and light on LED4, otherwise print out the ID information. Secondly, write and read the NAND 
memory. If the test result is correct, LED2 will be turned on and the data written to the NAND 
will be printed out.
  The timing parameters are calculated according to the 120MHz system clock.
  P2 and P3 must be fitted to the EXMC port.
  JP24 must be fitted to the Nand port.
