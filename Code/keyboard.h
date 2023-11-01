#ifndef KeyRead_Header_Gaurd
#define	KeyRead_Header_Gaurd

#include "define.h"
#include "PC_keyboard.h"
#include "USART_keyboard.h"

#define NORMAL_PRESS    0
#define LONG_PRESS      1
#define DOUBLE_PRESS    2

#define LONG_PRESS_TIME  550000

typedef struct
{
    uint64_t t_time;
    uint8_t stat;
    uint16_t keyvalue;
    uint16_t charValue;
} KeyObject;
void keyboard_hander();
void init_keyboard(uint8_t i_usbEN);

uint16_t correspondKey(uint8_t key);
void readkey();
void settimestamp(uint8_t key,uint8_t key_status);

uint8_t change_layer(volatile uint16_t **nextLayer,uint16_t key_char);
void key_translate();

void ringbuff_tail_plus_one(volatile ringBuff * i_ring); 

void ringbuff_plus_one_head(volatile ringBuff * i_ring); 

uint8_t get_char_from_layer(volatile uint16_t *ptr,uint16_t i_key,uint8_t i_method);
#endif