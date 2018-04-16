/*!
    \file  http_adc_led.c
    \brief webserver demo 
*/

/*
    Copyright (C) 2017 GigaDevice

    2017-07-28, V1.0.0, demo for GD32F30x
*/

#include "lwip/debug.h"
#include "httpd.h"
#include "lwip/tcp.h"
#include "fs.h"
#include "main.h"
#include "gd32f307c_eval.h"
#include <string.h>
#include <stdlib.h>


/* ssi handler for ADC monitor check for tag "gd" */
char const* http_adc_tag = "gd";
char const** ssi_tag = &http_adc_tag;

/* cgi call table */
tCGI cgi_table[1];
/* cgi handler for LED control */ 
const char * http_led_control(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
/* html request for "/leds.cgi" will start http_led_control */
const tCGI leds_cgi={"/leds.cgi", http_led_control};


/*!
    \brief      ssi handler for ADC monitor
*/
u16_t http_adc_monitor(int iIndex, char *pcInsert, int iInsertLen)
{
    char val_1, val_2, val_3, val_4; 
    uint32_t adc_val = 0;
  
    if(0 == iIndex){ 
        /* get ADC conversion value */
        adc_val = adc_inserted_data_read(ADC0, ADC_INSERTED_CHANNEL_0);
        adc_val = (uint32_t)(adc_val * 3300 / 4096);  
         
        /* get digits to display */
        val_1= adc_val/1000;
        val_2= (adc_val-(val_1*1000))/100 ;
        val_3= (adc_val-((val_1*1000)+(val_2*100)))/10;
        val_4= adc_val -((val_1*1000)+(val_2*100)+ (val_3*10));
            
        /* prepare data to be inserted in html */
        *pcInsert       = (char)(val_1 + 0x30);
        *(pcInsert + 1) = (char)(val_2 + 0x30);
        *(pcInsert + 2) = (char)(val_3 + 0x30);
        *(pcInsert + 3) = (char)(val_4 + 0x30);
        
        /* 4 characters need to be inserted in html*/
        return 4;
    }
    return 0;
}

/*!
    \brief      cgi handler for LED control
*/
const char * http_led_control(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    uint32_t i;
  
    if(0 == iIndex){
        gd_eval_led_off(LED2);
        gd_eval_led_off(LED3);
        gd_eval_led_off(LED4);
        gd_eval_led_off(LED5);
        
        /* check cgi parameter */
        for(i = 0; i < iNumParams; i++){
            /* check parameter "cb" */
            if(0 == strcmp(pcParam[i] , "cb")){   
                
                if(0 == strcmp(pcValue[i], "2")){ 
                    gd_eval_led_on(LED2);
                  
                }else if(0 == strcmp(pcValue[i], "3")){
                    gd_eval_led_on(LED3);               
                
                }else if(0 == strcmp(pcValue[i], "4")){
                    gd_eval_led_on(LED4);
                
                }else if(0 == strcmp(pcValue[i], "5")){
                    gd_eval_led_on(LED5);
                
                }
            }
        }
    }
    /* uri to send after cgi call*/
    return "/LED.html";  
}

/*!
    \brief      init ssi handler
*/
void httpd_ssi_init(void)
{  
    http_set_ssi_handler(http_adc_monitor, (char const **)ssi_tag, 1);
}

/*!
    \brief      init cgi handler
*/
void httpd_cgi_init(void)
{ 
    cgi_table[0] = leds_cgi;
    http_set_cgi_handlers(cgi_table, 1);
}
