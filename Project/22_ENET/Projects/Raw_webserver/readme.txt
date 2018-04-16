/*!
    \file  readme.txt
    \brief description of the enet demo
*/

/*
    Copyright (C) 2017 GigaDevice

    2017-07-28, V1.0.0, demo for GD32F30x
*/

  This demo is based on the GD32307C-EVAL-V1.0 board, it shows how to configure the 
enet peripherals to send and receive frames in normal mode and use lwip tcp/ip 
stack to realize webserver application. 

  JP4, JP13, JP18, JP19 must be fitted. JP5 jump to Usart.

  It is configured in RMII mode, and 25MHz oscillator is used, the system clock 
is configured to 120MHz. Users should configure ip address, mask and gw of GD32307C-EVAL-V1.0 
board according to the actual net situation from the private defines in main.h.

  This demo realize webserver application:
  Users can visit the eval board through Internet Explorer, the eval board acts as a webserver,
and the url is the local ip address of the eval board. There are two experiments realized, one is 
the LEDs control, the other one is the ADC monitoring Vref voltage in real-time.
  If users need dhcp function, it can be configured from the private defines in main.h. This 
function is closed by default. Users can use a router to connect the eval board, and use the HyperTerminal
to print the automatic allocated ip address, then connect your mobile phone to the wifi which
is the router send. Users can visit the eval board and control it on your mobile phone.
 
  By default, the packet reception is polled in while(1). If users want to receive packet in 
interrupt service, uncomment the macro define USE_ENET_INTERRUPT in main.h.

  

