#ifndef USART_keyboard_Header_Gaurd
#define	USART_keyboard_Header_Gaurd

#include "define.h"
#include "keyboard.h"

typedef struct
{
    uint8_t command;
    uint8_t data;
} keyboard_uart;

enum  {
    kb_PRESS_KEY = 2,
    kb_RELEASE_KEY = 3,
    kb_RELEASE_ALL = 4,
    kb_CORRECT_CRC = 0,
    kb_CRC_CHECK_FAILED = 255,
};
typedef enum  {
    kb_Buffer_Send_No_Action = -1,
    kb_Buffer_Send_Write_To_Buffer = 1,
    kb_Buffer_Send_Write_CRC_Response = 0,
}enum_buffer_send;

uint8_t uh_send(uint8_t *data,uint8_t len);
void uh_receive();
uint8_t uh_genCRC(uint8_t *data,uint8_t len);
void uh_prepear(uint8_t *data,uint8_t len);
void uh_decode(keyboard_uart data);
void uh_encode(keyboard_uart data);
void uh_addtostring(uint8_t *i_base,uint8_t *i_add,uint8_t len);
void uh_hanlder();
void uh_init(uart_inst_t *r_us);
void uh_buffer_send(keyboard_uart i_key,enum_buffer_send r_add_to_buffer);
#endif