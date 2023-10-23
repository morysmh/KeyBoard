#include "USART_keyboard.h"
uart_inst_t *r_uartkb_inst = 0;
uint8_t uh_send(uint8_t *data, uint8_t len)
{
    static volatile uint8_t r_buff[10] = {};
    static volatile uint8_t r_index = 0;
    static volatile int8_t  r_remain = 0;
    if(len > 5)
        len = 5;
    if (len != 0)
    {
        for (r_remain = 0; r_remain < len; r_remain++)
            r_buff[r_remain] = data[r_remain];
        r_remain = len;
        r_index = 0;
    }
    if (!uart_is_writable(r_uartkb_inst))
        return r_remain;
    if (r_remain <= 0)
        return 0;
    uart_putc_raw(r_uartkb_inst, r_buff[r_index]);
    r_index++;
    r_remain--;
    return r_remain;
}
void uh_hanlder()
{
    static volatile uint64_t t_time = 0;
    static keyboard_uart t_key = {};
    if(t_time > time_us_64())
        return;
    t_time = time_us_64() + 98;
    uh_buffer_send(t_key,kb_Buffer_Send_No_Action);
    uh_receive();
}
void uh_init(uart_inst_t *r_us)
{
    if ((r_us != uart0) && (r_us != uart1))
        return;
    r_uartkb_inst = r_us;
}
void uh_receive()
{
    static volatile uint64_t t_lastrx = 0;
    static volatile uint8_t r_buff[10] = {};
    static volatile uint8_t r_index = 0;
    const uint64_t c_rxTimeout = 3000;
    if (r_index >= 3)
    {
        uh_prepear((uint8_t *)r_buff, 3);
        r_index = 0;
    }
    if (!uart_is_readable(r_uartkb_inst))
        return;
    if (t_lastrx < time_us_64())
        r_index = 0;
    t_lastrx = time_us_64() + c_rxTimeout;
    r_buff[r_index] = uart_getc(r_uartkb_inst);
    r_index++;
}
void uh_addtostring(uint8_t *i_base,uint8_t *i_add,uint8_t len)
{
    for(;len;len--)
    {
        *i_base = *i_add;
        i_base++;
        i_add++;
    }
}
uint8_t uh_genCRC(uint8_t *i_data, uint8_t len)
{
	int32_t _polynome   = 0xAA;
	int32_t _endMask    = 0;
	int32_t _crc        = 0;
	int32_t l = 0;
	uint8_t r_total[120] = {};
    uint8_t *ptr_data;
    if (len == 0)
    {
        return 0;
    }
    uh_addtostring(&r_total[0],i_data,len);
    uh_addtostring(&r_total[len],"afb4",4);
    len += 4;
    ptr_data = r_total;
	while(len--)
	{
		_crc = _crc ^ (*ptr_data);
		for(l=0;l<8;l++)
		{
			_crc = _crc & 0xFF;
			if(_crc &(1<<7))
			{
				_crc = (_crc<<1);
				_crc = (_crc ^ _polynome) ;
			}
			else
			{
				_crc = (_crc << 1);
			}
			_crc = _crc & 0xFF;
		}
        ptr_data++;
	}
	return ((_crc ^ _endMask) & 0xFF);
}
void uh_prepear(uint8_t *data, uint8_t len)
{
    static volatile keyboard_uart r_kbua = {};
    static volatile uint8_t r_crc = 0;
    static volatile uint8_t r_send[5] = {};
    if (len == 0)
    {
        return;
    }
    r_kbua.command = data[0];
    r_kbua.data = data[1];
    r_crc = uh_genCRC(data, 2);
    if (r_crc == data[2])
    {
        uh_decode(r_kbua);
        r_kbua.command = kb_CORRECT_CRC;
        uh_buffer_send(r_kbua,kb_Buffer_Send_Write_CRC_Response);
    }
    if (r_crc != data[2])
    {
        r_kbua.command = kb_CRC_CHECK_FAILED;
        uh_buffer_send(r_kbua,kb_Buffer_Send_Write_CRC_Response);
    }
    return;
}

void uh_buffer_send(keyboard_uart i_key,enum_buffer_send r_add_to_buffer)
{
    static volatile keyboard_uart b_kb[15] = {};
    static volatile ringBuff r_rb = {};
    static volatile uint8_t r_send[5] = {};
    static volatile uint8_t r_data_in_progress = 0;
    static volatile uint64_t t_retransmit = 0;
    const uint64_t c_time_retransmit = 25000;
    if(r_rb.max_size != 10)
    {
        r_rb.max_size = 10;
        r_rb.head = 0;
        r_rb.tail = 0;
    }
    if(r_add_to_buffer == kb_Buffer_Send_Write_CRC_Response)
    {
        r_send[0] = i_key.command;
        r_send[1] = i_key.data;
        r_send[2] = uh_genCRC((uint8_t *)r_send, 2);
        r_data_in_progress = 1;
        t_retransmit = time_us_64() + 1000;
        if((i_key.command == kb_CRC_CHECK_FAILED) || (i_key.command == kb_CORRECT_CRC))
        {
            if(r_rb.head != r_rb.tail)
                r_data_in_progress = 0;
        }
        if((r_data_in_progress == 0) && (r_rb.head != r_rb.tail))
        {
            t_retransmit = time_us_64() + 1000;
            ringbuff_tail_plus_one(&r_rb);
        }
    }
    if(r_add_to_buffer == kb_Buffer_Send_Write_To_Buffer)
    {
        b_kb[r_rb.head].command = i_key.command;
        b_kb[r_rb.head].data = i_key.data;
        ringbuff_plus_one_head(&r_rb);
        t_retransmit = time_us_64() + 1000;
    }
    if(uh_send(0,0) != 0)
        return;
    if(t_retransmit > time_us_64())
        return;
    if(r_data_in_progress == 1)
    {
        t_retransmit = time_us_64() + c_time_retransmit;
        uh_send((uint8_t *)r_send, 3);
        r_data_in_progress = 0;
        return;
    }
    if(r_rb.head == r_rb.tail)
        return;
    t_retransmit = time_us_64() + c_time_retransmit;
    r_send[0] = b_kb[r_rb.tail].command;
    r_send[1] = b_kb[r_rb.tail].data;
    r_send[2] = uh_genCRC((uint8_t *)r_send, 2);
    uh_send((uint8_t *)r_send, 3);
    return;
}
void uh_decode(keyboard_uart data)
{
    switch (data.command)
    {
    case kb_PRESS_KEY:
        settimestamp(data.data, 0);
        break;
    case kb_RELEASE_KEY:
        settimestamp(data.data, 1);
        break;
    case kb_RELEASE_ALL:
        //settimestamp(0, 3);
        break;
    default:
        break;
    }
}
void uh_encode(keyboard_uart data)
{
    uh_buffer_send(data,kb_Buffer_Send_Write_To_Buffer);
}