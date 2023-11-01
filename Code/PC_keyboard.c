#include "PC_keyboard.h"

extern volatile LED_Interval pico_led;
extern volatile LED_Interval r_led;
extern volatile LED_Interval g_led;
extern volatile LED_Interval b_led;
//static volatile uint8_t r_allKey[10] = {};
const uint8_t c_max_key = 6;
static volatile uint8_t r_newData = 0;

volatile ringBuff rb_send = {};
volatile uint8_t sendBuffer[21][7] ={};
// Invoked when device is mounted
void change_led_duty(LED_Interval *led,uint8_t duty)
{
  led->_ON = ((uint32_t)duty * 20000UL)/ 255UL;
  led->OFF = ((255UL - (uint32_t)duty) * 20000UL)/ 255UL;
}
void kb_change_RGB(uint8_t r,uint8_t g,uint8_t b)
{
  change_led_duty((LED_Interval *)&r_led,r);
  change_led_duty((LED_Interval *)&g_led,g);
  change_led_duty((LED_Interval *)&b_led,b);
}

void tud_mount_cb(void)
{
  pico_led.OFF = BLINK_MOUNTED;
  pico_led._ON = BLINK_MOUNTED;
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
  pico_led.OFF = BLINK_NOT_MOUNTED;
  pico_led._ON = BLINK_NOT_MOUNTED;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
  (void) remote_wakeup_en;
  pico_led.OFF = BLINK_SUSPENDED;
  pico_led._ON = BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
  pico_led.OFF = BLINK_MOUNTED;
  pico_led._ON = BLINK_MOUNTED;
}
uint8_t deletefromString(uint8_t *i_data,uint8_t len,uint8_t key)
{
  uint8_t last_data_index = 0;
  uint8_t r_ret = 0;
  if((key == 0) || (i_data == 0))
    return 0;
  for(uint i=0;i<len;i++)
  {
    if(i_data[i] == key)
    {
      i_data[i] = HID_KEY_NONE;
      r_ret++;
      break;
    }
  }
  for(uint i=0;i<(len - 1);i++)
  {
    if(i_data[i] == HID_KEY_NONE)
    {
      i_data[i] = i_data[i+1];
      i_data[i+1] = HID_KEY_NONE;
    }
  }
  return r_ret;
}
uint8_t check_if_system_key(uint8_t i_key)
{
  uint8_t controll_keys[] = {
  HID_KEY_CONTROL_LEFT,
  HID_KEY_SHIFT_LEFT,
  HID_KEY_ALT_LEFT,
  HID_KEY_GUI_LEFT,
  HID_KEY_CONTROL_RIGHT,
  HID_KEY_SHIFT_RIGHT,
  HID_KEY_ALT_RIGHT,
  HID_KEY_GUI_RIGHT,
  0};
  uint8_t i = 0;
  for(uint i = 0;i<(sizeof(controll_keys)/sizeof(uint8_t));i++)
  {
    if((i_key == controll_keys[i]) && (i_key != 0))
      return 1;
  }
  return 0;
}
void init_PC_keyboard()
{
  rb_send.max_size = 20;
  rb_send.tail = 0;
  rb_send.head = 0;
}
void write_on_keyboard(uint8_t i_key,uint8_t i_press_or_release)
{
  //r_newData = 1; 
  static volatile uint8_t r_allKey[10] = {};
  static volatile uint32_t r_tmp = 0;
  static volatile uint8_t r_indx = 0;

  if((i_press_or_release != kb_RELEASE_ALL) && (i_key == 0))
    return;
  if(i_press_or_release == kb_RELEASE_KEY)
  {
    deletefromString((uint8_t *)r_allKey,c_max_key,i_key);
    if((check_if_system_key(i_key)) && (r_indx)) 
      r_indx--;
  }
  if(i_press_or_release == kb_PRESS_KEY)
  {
    r_allKey[r_indx] = i_key;
    if((check_if_system_key(i_key)) && (r_indx < 6)) 
      r_indx++;
  }
  if(i_press_or_release == kb_RELEASE_ALL)
  {
   r_indx = 0;
    for(uint i = 0;i<c_max_key;i++)
      r_allKey[0] = HID_KEY_NONE;
  }

  for(uint i = 0;i<6;i++)
    sendBuffer[rb_send.head][i] = r_allKey[i];
  ringbuff_plus_one_head(&rb_send);
  r_tmp = i_key;
  return;
}
//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

static void send_hid_report(uint8_t report_id, uint8_t  *keycode)
{
  // skip if hid is not ready yet
  if (!tud_hid_ready())
    return;

  tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keycode);
}

// Every 10ms, we will sent 1 report for each HID profile (keyboard, mouse etc ..)
// tud_hid_report_complete_cb() is used to send the next report after previous one is complete
void hid_task(void)
{
  // Poll every 10ms
  const uint64_t interval_us = 10000;
  static uint64_t next_interval = interval_us * 100;

  if (next_interval > time_us_64())
    return; // not enough time
  next_interval = time_us_64() + interval_us;

  // Remote wakeup

  if (tud_suspended())
  {
    // Wake up host if we are in suspend mode
    // and REMOTE_WAKEUP feature is enabled by host
    tud_remote_wakeup();
  }
  send_if_data_available();

}

// Invoked when sent REPORT successfully to host
// Application can use this to send the next report
// Note: For composite reports, report[0] is report ID
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const *report, uint8_t len)
{
  (void)instance;
  (void)len;
  (void)report;
 send_if_data_available();
}
void send_if_data_available()
{
  static volatile uint8_t r_none_send = 0;
  uint8_t r_sendData[8] = {};
  if(rb_send.head == rb_send.tail)
    return;
  if (!tud_hid_ready())
    return;

  for(uint i = 0;i<c_max_key;i++)
    r_sendData[i] = sendBuffer[rb_send.tail][i];
  ringbuff_tail_plus_one(&rb_send);
  //return;
  send_hid_report(REPORT_ID_KEYBOARD, r_sendData);
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen)
{
  // TODO not Implemented
  (void)instance;
  (void)report_id;
  (void)report_type;
  (void)buffer;
  (void)reqlen;

  return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize)
{
  (void)instance;

  if (report_type == HID_REPORT_TYPE_OUTPUT)
  {
    // Set keyboard LED e.g Capslock, Numlock etc...
    if (report_id == REPORT_ID_KEYBOARD)
    {
      // bufsize should be (at least) 1
      if (bufsize < 1)
        return;

      uint8_t const kbd_leds = buffer[0];

      if (kbd_leds & KEYBOARD_LED_CAPSLOCK)
      {
        // Capslock On: disable blink, turn led on
        pico_led._ON = 0;
        pico_led.stat = 1;
      }
      else
      {
        // Caplocks Off: back to normal blink
        pico_led.OFF = BLINK_MOUNTED;
        pico_led._ON = BLINK_MOUNTED;
      }
    }
  }
}