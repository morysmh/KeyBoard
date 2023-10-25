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
#include "USART_keyboard.h"


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

void Usart_init_main();

/*------------- MAIN -------------*/
int main(void)
{
  static volatile uint8_t c_usbEN = 0;
  static volatile uint64_t t_time = 0;
  Board_pin_Config();
  t_time = time_us_64() + 1000;
  while(t_time > time_us_64());
  c_usbEN = !gpio_get(Pin_HandSel);
  stdio_init_all();
  init_keyboard(c_usbEN);
  Usart_init_main();
  init_PC_keyboard();
  volatile uint8_t r_crc = 0;
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
  uh_init(uart1);
  if(c_usbEN)
    tusb_init();

  while (1)
  {
    led_blinking_task();

    uh_hanlder();
    if(c_usbEN)
    {
      tud_task(); // tinyusb device task
      hid_task();
    }
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

    gpio_init(Pin_HandSel);
    gpio_set_dir(Pin_HandSel,GPIO_IN);
    gpio_set_pulls(Pin_HandSel,1,0);

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


#ifdef __cplusplus
extern "C"
{
#endif

    void usbUartRxHandler()
    {
        if (uart_is_readable(uart1))
        {
            (void)uart_getc(uart1);
        }
    }
#ifdef __cplusplus
}
#endif

void Usart_init_main()
{
    // Set up our UART with a basic baud rate.
    uart_init(uart1, 2400);
    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select
    gpio_set_function(Pin_TX, GPIO_FUNC_UART);
    gpio_set_function(Pin_RX, GPIO_FUNC_UART);

    // Actually, we want a different speed
    // The call will return the actual baud rate selected, which will be as close as
    // possible to that requested
    int __unused actual = uart_set_baudrate(uart1, 115200);

    // Set UART flow control CTS/RTS, we don't want these, so turn them off
    uart_set_hw_flow(uart1, false, false);

    // Set our data format
    uart_set_format(uart1, 8, 1, UART_PARITY_NONE);

    // Turn off FIFO's - we want to do this character by character
    uart_set_fifo_enabled(uart1, false);

    // Set up a RX interrupt
    // We need to set up the handler first
    // Select correct interrupt for the UART we are using
    int UART_IRQ = UART1_IRQ;

    // And set up and enable the interrupt handlers
    irq_set_exclusive_handler(UART_IRQ, usbUartRxHandler);
    irq_set_enabled(UART_IRQ, true);

    // Now enable the UART to send interrupts - RX only
    uart_set_irq_enables(uart1, false, false);

    // OK, all set up.
    // Lets send a basic string out, and then run a loop and wait for RX interrupts
    // The handler will count them, but also reflect the incoming data back with a slight change!
}