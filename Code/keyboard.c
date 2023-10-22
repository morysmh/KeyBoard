#include "keyboard.h"
#include "PC_keyboard.h"

extern volatile uint8_t Layer1[][3];
volatile KeyObject keystamp[200];
volatile ringBuff r_stmp = {};
void keyboard_hander()
{
    readkey();
    key_translate();
}
void init_keyboard()
{
    r_stmp.max_size = 190;
    r_stmp.head = 0;
    r_stmp.tail = 0;
}

void readkey()
{
    const int8_t c_keys[] = {
    Pin_Key5  ,
    Pin_Key6  ,
    Pin_Key13 ,
    Pin_Key12 ,  
    Pin_Key18 ,
    Pin_Key1  ,
    Pin_Key2  ,
    Pin_Key7  ,
    Pin_Key14 ,
    Pin_Key16 ,
    Pin_Key11 ,
    Pin_Key3  ,
    Pin_Key8  ,
    Pin_Key15 ,
    Pin_Key17 ,
    Pin_Key10 ,
    Pin_Key4  ,
    Pin_Key9  ,
    0,
    0
    };
    const uint64_t t_bounce = 3000;
    static volatile uint64_t t_delay = 0;
    static volatile uint64_t a_last_time[Keys_Count + 5]= {};
    static volatile uint8_t  a_lastState[Keys_Count + 5]= {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
    if(t_delay > time_us_64())
        return;
    t_delay = time_us_64() + 5;
    for(uint i = 0;i < Keys_Count;i++)
    {
        if(a_lastState[i] == gpio_get(c_keys[i]))
        {
            a_last_time[i] = time_us_64() + t_bounce;
            continue;
        }
        if(a_last_time[i] < time_us_64())
        {
            a_lastState[i] = gpio_get(c_keys[i]);
            settimestamp(correspondKey(c_keys[i]),a_lastState[i]);
        }
    }
}
void settimestamp(uint8_t key,uint8_t key_status)
{
    keystamp[r_stmp.head].keyvalue = key;
    keystamp[r_stmp.head].t_time = time_us_64();
    keystamp[r_stmp.head].stat = !key_status;
    ringbuff_plus_one_head(&r_stmp);
}
void key_translate()
{
    static volatile uint64_t t_delay = 0;
    uint8_t r_next = 0;
    uint8_t r_char = 0;
    if(t_delay > time_us_64())
        return;
    t_delay = time_us_64() + 5;
    if(r_stmp.head == r_stmp.tail)
        return;
    r_next = r_stmp.tail + 1;
    r_next = ((r_next > r_stmp.max_size)?0:r_next);
    if((keystamp[r_stmp.tail].stat == 0) && (r_next == r_stmp.head))
    {
        write_on_keyboard(HID_KEY_NONE);
        ringbuff_tail_plus_one(&r_stmp);
        return;
    }
    if(keystamp[r_stmp.tail].stat == 0)
    {
        ringbuff_tail_plus_one(&r_stmp);
        return;
    }
    if((r_next == r_stmp.head) && ((keystamp[r_stmp.tail].t_time + LONG_PRESS_TIME) < time_us_64()))
    {
        r_char = get_char_from_layer(Layer1,keystamp[r_stmp.tail].keyvalue,LONG_PRESS);
        if(r_char == 0)
            r_char = get_char_from_layer(Layer1,keystamp[r_stmp.tail].keyvalue,NORMAL_PRESS);
        write_on_keyboard(r_char);
        ringbuff_tail_plus_one(&r_stmp);
        return;
    }
    if(r_next != r_stmp.head)
    {
        r_char = get_char_from_layer(Layer1,keystamp[r_stmp.tail].keyvalue,NORMAL_PRESS);
        write_on_keyboard(r_char);
        ringbuff_tail_plus_one(&r_stmp);
        return;
    }
    return;
}

uint8_t get_char_from_layer(volatile uint8_t i_layer[][3],uint8_t i_key,uint8_t i_method)
{
    uint8_t r_ret = 0;
    for(uint8_t i=0;i<(Keys_Count * 2);i++)
    {
        if(i_layer[i][0] != i_key)
            continue;
        r_ret = i_layer[i][1];
        if(i_method == LONG_PRESS)
            r_ret = i_layer[i][2];
    }
    return r_ret;
}
void ringbuff_plus_one_head(volatile ringBuff * i_ring){i_ring->head = ((i_ring->head > i_ring->max_size)?0:i_ring->head + 1);}
void ringbuff_tail_plus_one(volatile ringBuff * i_ring){i_ring->tail = ((i_ring->tail > i_ring->max_size)?0:i_ring->tail + 1);}

uint8_t correspondKey(uint8_t key)
{
    uint8_t r_otherHand = 0, r_ret = 0;
    const int8_t c_keys[Keys_Count + 2][2]= 
    {
    Pin_Key5  ,16,
    Pin_Key6  ,17,
    Pin_Key13 ,18,
    Pin_Key12 ,15,  
    Pin_Key18 ,14,
    Pin_Key1  ,13,
    Pin_Key2  ,12,
    Pin_Key7  ,11,
    Pin_Key14 ,10,
    Pin_Key16 ,9,
    Pin_Key11 ,8,
    Pin_Key3  ,7,
    Pin_Key8  ,6,
    Pin_Key15 ,5,
    Pin_Key17 ,4,
    Pin_Key10 ,3,
    Pin_Key4  ,2,
    Pin_Key9  ,1,
    0,0
    };
    if(key > 127)
    {
        key -= 128;
        r_otherHand = 1;
    }
    for(uint i=0;i<Keys_Count;i++)
    {
        if(c_keys[i][0] == key)
        {
            r_ret = c_keys[i][1];
            break;
        }
    }
    if(r_otherHand)
        r_ret += Keys_Count;
    return r_ret;
}