#ifndef MainMenu_Header
#define	MainMenu_Header

#include <stdio.h>
#include "pico/stdlib.h"

#include "hardware/uart.h"
#include "hardware/irq.h"
#include "hardware/irq.h"

#define Pin_LED_B   0
#define Pin_LED_G   1
#define Pin_LED_R   2
#define Pin_HandSel 3
#define Pin_TX      4
#define Pin_RX      5
#define Pin_Key5    6
#define Pin_Key6    7
#define Pin_Key13   8
#define Pin_Key12   9
#define Pin_Key18   10
#define Pin_Key1    11
#define Pin_Key2    12
#define Pin_Key7    13
#define Pin_Key14   14
#define Pin_Key16   15
#define Pin_Key11   16
#define Pin_Key3    17
#define Pin_Key8    18
#define Pin_Key15   19
#define Pin_Key17   20
#define Pin_Key10   21
#define Pin_Key4    22
#define Pin_Key9    26
#define Pin_AD2     27
#define Pin_AD3     28

#define Keys_Count  18
#define Keys_Total  36


typedef struct
{
    int64_t OFF;
    int64_t _ON;
    int64_t lastCheck;
    int8_t stat;
} LED_Interval;


typedef struct
{
    uint32_t head;
    uint32_t tail;
    uint32_t max_size;
    uint8_t *buffer;
} ringBuff;



#define Software_Layer1    0xF0
#define Software_Layer2    0xF1
#define Software_Layer3    0xF2
#define Software_Layer4    0xF3
#define Software_Layer5    0xF4
#define Software_Layer6    0xF5
#define Software_Layer7    0xF6
#define Software_Layer8    0xF7
#define Software_Layer9    0xF8
#define Software_Layer10   0xF9
#define Software_Layer11   0xFA
#define Software_Layer12   0xFB
#define Software_Layer13   0xFC
#define Software_Layer14   0xFD
#define Software_Layer15   0xFE
#define Software_Layer16   0xFF

#endif