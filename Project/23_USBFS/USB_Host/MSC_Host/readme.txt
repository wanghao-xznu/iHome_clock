/*!
    \file  readme.txt
    \brief Description of the USB host mode to control MSC device
*/

/*
    Copyright (C) 2017 GigaDevice

    2017-07-28, V1.0.0, demo for GD32F30x
*/

  This demo is based on the GD32307C-EVAL-V1.0 board, and it provides a description of how 
to use the USBFS host.

  Insert the OTG cable to the USB_FS port. Then download it to the EVAL board and run.

  If an Udisk has been attached, the user will see the information of Udisk enumeration. 
First pressing the user key will see the Udisk information, next pressing the tamper key
will see the root content of the Udisk, then press the wakeup key will write file to the
Udisk, finally the user will see information that the msc host demo is end.
