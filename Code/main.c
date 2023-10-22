/* 
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "define.h"
#include "keyboard.h"
#include "PC_keyboard.h"


/* This MIDI example send sequence of note (on/off) repeatedly. To test on PC, you need to install
 * synth software and midi connection management software. On
 * - Linux (Ubuntu): install qsynth, qjackctl. Then connect TinyUSB output port to FLUID Synth input port
 * - Windows: install MIDI-OX
 * - MacOS: SimpleSynth
 */

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+


/* Blink pattern
 * - 250 ms  : device not mounted
 * - 1000 ms : device mounted
 * - 2500 ms : device is suspended
 */

volatile ringBuff senddat ={};
volatile uint8_t senddatabuffer[45] = {};
volatile LED_Interval pico_led={},r_led={},g_led={},b_led={};

#define Led_Pin_PICO 25
#define PICO_LED_Stat(x) gpio_put(Led_Pin_PICO, x)
#define R_LED_Stat(x)    gpio_put(Pin_LED_R,x);
#define B_LED_Stat(x)    gpio_put(Pin_LED_B,x);
#define G_LED_Stat(x)    gpio_put(Pin_LED_G,x);

void chagneLED(volatile LED_Interval *led);
void led_blinking_task(void);

void Board_pin_Config();


/*------------- MAIN -------------*/
int main(void)
{
  Board_pin_Config();
  stdio_init_all();
  init_keyboard();
  pico_led.OFF = 200000;
  pico_led._ON = 400000;
  r_led._ON = 1;
  r_led.OFF = 400000;
  g_led._ON = 1;
  g_led.OFF = 40000000;
  b_led._ON = 1;
  b_led.OFF = 40000000;
  senddat.buffer = (uint8_t *)senddatabuffer;
  senddat.max_size = 40;

  tusb_init();

  while (1)
  {
    tud_task(); // tinyusb device task
    led_blinking_task();

    hid_task();
    keyboard_hander();
  }


  return 0;
}

void Board_pin_Config()
{
    gpio_init(Led_Pin_PICO);
    gpio_set_dir(Led_Pin_PICO, GPIO_OUT);
    for(int i=0;i<=2;i++)
    {
      gpio_init(i);
      gpio_set_dir(i, GPIO_OUT);
    }
    for(int i=6;i<=22;i++)
    {
      gpio_init(i);
      gpio_set_dir(i, GPIO_IN);
      gpio_set_pulls(i,1,0);
    }
      gpio_init(Pin_Key9);
      gpio_set_dir(Pin_Key9, GPIO_IN);
      gpio_set_pulls(Pin_Key9,1,0);

      gpio_init(Pin_AD2);
      gpio_set_dir(Pin_AD2, GPIO_IN);
      gpio_set_pulls(Pin_AD2,1,0);
      gpio_init(Pin_AD3);
      gpio_set_dir(Pin_AD3, GPIO_IN);
      gpio_set_pulls(Pin_AD3,1,0);
}




//--------------------------------------------------------------------+
// BLINKING TASK
//--------------------------------------------------------------------+
void led_blinking_task(void)
{
  chagneLED(&pico_led);
  PICO_LED_Stat(pico_led.stat);

  chagneLED(&r_led);
  R_LED_Stat(!r_led.stat);

  chagneLED(&g_led);
  G_LED_Stat(!g_led.stat);

  chagneLED(&b_led);
  B_LED_Stat(!b_led.stat);
}


void chagneLED(volatile LED_Interval *led)
{
    if (led->lastCheck > time_us_64())
    {
        return;
    }
    if (led->stat)
    {
        led->lastCheck = time_us_64() + led->OFF;
        led->stat = 0;
    }
    else
    {
        led->lastCheck = time_us_64() + led->_ON;
        led->stat = 1;
    }
}