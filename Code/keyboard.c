#include "keyboard.h"
#include "PC_keyboard.h"

extern volatile uint16_t Layer1[][2];
extern volatile uint16_t Layer2[][2];
extern volatile uint16_t Layer3[][2];
extern volatile uint16_t Layer4[][2];
volatile KeyObject keystamp[512];
volatile ringBuff r_stmp = {};
volatile uint8_t r__usbEN = 1;
volatile uint16_t *current_Layer;
void keyboard_hander()
{
    readkey();
    key_translate();
}
void init_keyboard(uint8_t i_usbEN)
{
    r_stmp.max_size = 500;
    r_stmp.head = 0;
    r_stmp.tail = 0;
    r__usbEN = i_usbEN;
    change_layer(&current_Layer , 0XF0,kb_PRESS_KEY);
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
    static volatile uint8_t  a_lastState[Keys_Count + 5]= {kb_RELEASE_KEY,kb_RELEASE_KEY,kb_RELEASE_KEY,kb_RELEASE_KEY,kb_RELEASE_KEY,kb_RELEASE_KEY,kb_RELEASE_KEY,kb_RELEASE_KEY,kb_RELEASE_KEY,kb_RELEASE_KEY,kb_RELEASE_KEY,kb_RELEASE_KEY,kb_RELEASE_KEY,kb_RELEASE_KEY,kb_RELEASE_KEY,kb_RELEASE_KEY,kb_RELEASE_KEY,kb_RELEASE_KEY};
    int8_t r_isKeyPressed = 0;
    uint8_t r_in = 0;
    if(t_delay > time_us_64())
        return;
    t_delay = time_us_64() + 5;
    for(uint i = 0;i < Keys_Count;i++)
    {
        if(a_lastState[i] == kb_PRESS_KEY)
        {
            r_isKeyPressed = 1;
        }
        r_in = kb_PRESS_KEY;
        if(gpio_get(c_keys[i]) == 1)
            r_in = kb_RELEASE_KEY;
        if(a_lastState[i] == r_in)
        {
            a_last_time[i] = time_us_64() + t_bounce;
            continue;
        }
        if(a_last_time[i] < time_us_64())
        {
            a_lastState[i] = kb_RELEASE_KEY;
            if(gpio_get(c_keys[i]) == 0)
            {
                a_lastState[i] = kb_PRESS_KEY;
                r_isKeyPressed = 1;
            }
            settimestamp(c_keys[i],a_lastState[i]);
        }
    }
    if(!r_isKeyPressed)
        settimestamp(0,kb_RELEASE_ALL);
}
void settimestamp(uint8_t key,uint8_t key_status)
{
    keyboard_uart keydata;
    static volatile uint8_t r_relase_all_Send = 3;
    if((key_status == kb_RELEASE_ALL) && (r_relase_all_Send == 3))
        return;
    if(key_status == kb_RELEASE_ALL)
    {
        if(key == 0)
            r_relase_all_Send |= 1;
        else
            r_relase_all_Send |= 2;
    }
    if((key_status == kb_PRESS_KEY) || (key_status == kb_RELEASE_KEY))
    {
        if(key < 128)
            r_relase_all_Send &= ~1;
        else
            r_relase_all_Send &= ~2;
    }
    if((key_status == kb_RELEASE_ALL) && (r_relase_all_Send != 3))
        return;
    keystamp[r_stmp.head].keyvalue = correspondKey(key);
    keystamp[r_stmp.head].t_time = time_us_64();
    keystamp[r_stmp.head].stat = key_status;
    if(r__usbEN)
        ringbuff_plus_one_head(&r_stmp);
    else
    {
        keydata.command = key_status;
        keydata.data = key + 128;
        uh_encode(keydata);
    }
}
uint32_t backward_search(uint32_t start)
{
    uint32_t r_back = start - 1;
    if(start == 0)
        r_back = r_stmp.max_size;
    while (start != r_back)
    {
        if(keystamp[r_back].keyvalue == keystamp[start].keyvalue)
            break;
        r_back=((r_back == 0)?r_stmp.max_size:r_back - 1);
    }
    return r_back;
    
}
uint8_t change_layer(volatile uint16_t **nextLayer,uint16_t key_char,uint8_t PressOrRelease)
{
    const uint32_t LayerPointerAddress[18] = {(uint32_t)Layer1,(uint32_t)Layer2,(uint32_t)Layer3,(uint32_t)Layer4};
    volatile uint32_t r_tmp = 0;
    static volatile uint8_t until_release = 0;
    volatile uint16_t i_tmp = key_char;
    if(key_char < 0xF0)
        return 0;
    key_char -= 0xF0;
    if((PressOrRelease == kb_RELEASE_KEY) && (until_release == 0))
        return 0;
    if((until_release == 1) && (PressOrRelease == kb_RELEASE_KEY))
    {
        key_char = 0;
        until_release = 0;
    }
    if(key_char > 15)
    {
        until_release = 1;
        key_char -= 16;
    }
    if(key_char >= 15)
        key_char = 15;
    if(key_char == 0)
        kb_change_RGB(200,0,0);
    if(key_char == 1)
        kb_change_RGB(0,200,0);
    if(key_char == 2)
        kb_change_RGB(0,0,200);
    if(key_char == 3)
        kb_change_RGB(200,100,200);
    *nextLayer = (volatile uint16_t *)LayerPointerAddress[key_char]; 
    if(*nextLayer == 0)
        *nextLayer = (volatile uint16_t *)LayerPointerAddress[0];
    r_tmp = (uint32_t)nextLayer;
    r_tmp = (uint32_t)*nextLayer;
    r_tmp = (uint32_t)**nextLayer;
    return 1;
}
void key_translate()
{
    static volatile uint64_t t_delay = 0;
    uint16_t r_next = 0;
    static volatile uint32_t r_tmp =0;
    if(t_delay == 0)
    {
    }
    if(t_delay > time_us_64())
        return;
    t_delay = time_us_64() + 5;
    if(r_stmp.head == r_stmp.tail)
        return;
    r_next = r_stmp.tail + 1;
    r_next = ((r_next > r_stmp.max_size)?0:r_next);
    if(keystamp[r_stmp.tail].stat == kb_RELEASE_KEY)
    {
        r_tmp = backward_search(r_stmp.tail);
        if(keystamp[r_tmp].charValue < 0xF0)
            write_on_keyboard(keystamp[r_tmp].charValue,kb_RELEASE_KEY);
        else if (keystamp[r_tmp].charValue >= 0xF0)
            change_layer(&current_Layer,keystamp[r_tmp].charValue,kb_RELEASE_KEY);
        ringbuff_tail_plus_one(&r_stmp);
        return;
    }
    if(keystamp[r_stmp.tail].stat == kb_RELEASE_ALL)
    {
        ringbuff_tail_plus_one(&r_stmp);
        write_on_keyboard(0,kb_RELEASE_ALL);
        return;
    }
    if((r_next == r_stmp.head) && ((keystamp[r_stmp.tail].t_time + LONG_PRESS_TIME) < time_us_64()))
    {
        keystamp[r_stmp.tail].charValue = get_char_from_layer(current_Layer,keystamp[r_stmp.tail].keyvalue,LONG_PRESS);
        if(keystamp[r_stmp.tail].charValue == 0)
            keystamp[r_stmp.tail].charValue = get_char_from_layer(current_Layer,keystamp[r_stmp.tail].keyvalue,NORMAL_PRESS);
        if(change_layer(&current_Layer,keystamp[r_stmp.tail].charValue,kb_PRESS_KEY) == 0)
            write_on_keyboard(keystamp[r_stmp.tail].charValue,kb_PRESS_KEY);
        ringbuff_tail_plus_one(&r_stmp);
        return;
    }
    if(r_next != r_stmp.head)
    {
        keystamp[r_stmp.tail].charValue = get_char_from_layer(current_Layer,keystamp[r_stmp.tail].keyvalue,NORMAL_PRESS);
        if(change_layer(&current_Layer,keystamp[r_stmp.tail].charValue,kb_PRESS_KEY) == 0)
            write_on_keyboard(keystamp[r_stmp.tail].charValue,kb_PRESS_KEY);
        ringbuff_tail_plus_one(&r_stmp);
        return;
    }
    return;
}

uint16_t get_char_from_layer(volatile uint16_t *ptr,uint16_t i_key,uint8_t i_method)
{
    uint16_t r_ret = 0;
    volatile uint16_t (*i_layer)[2];
    i_layer = (volatile uint16_t (*)[2])ptr;
    if(i_key >= 0xF0) //TODO Use proper Handeling of High Values
        return 0;
    if(i_key)
        i_key--;
    if (i_key < Keys_Total)
    r_ret = i_layer[i_key][0];
    if(i_method == LONG_PRESS)
        r_ret = i_layer[i_key][1];
    return r_ret;
}
void ringbuff_plus_one_head(volatile ringBuff * i_ring){i_ring->head = ((i_ring->head > i_ring->max_size)?0:i_ring->head + 1);}
void ringbuff_tail_plus_one(volatile ringBuff * i_ring){i_ring->tail = ((i_ring->tail > i_ring->max_size)?0:i_ring->tail + 1);}

uint16_t correspondKey(uint8_t key)
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
        return r_ret;
    return r_ret + Keys_Count;
}