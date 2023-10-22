#include "PC_keyboard.h"

extern volatile LED_Interval pico_led;

extern volatile ringBuff senddat;
extern volatile uint8_t senddatabuffer;
// Invoked when device is mounted
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

void write_on_keyboard(uint8_t i_key)
{
  senddat.buffer[senddat.head] = i_key;
  ringbuff_plus_one_head(&senddat);
}
//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

static void send_hid_report(uint8_t report_id, uint32_t btn)
{
  // skip if hid is not ready yet
  if (!tud_hid_ready())
    return;
  uint8_t keycode[6] = {0};
  keycode[0] = btn;

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
  if((senddat.head == senddat.tail) && (r_none_send == 0))
    return;
  if (!tud_hid_ready())
    return;
  if(senddat.head == senddat.tail)
    return;
  //if(senddat.head == senddat.tail)
  //{
  //  send_hid_report(REPORT_ID_KEYBOARD, 0);
  //  r_none_send = 0;
  //  return;
  //}
  //r_none_send = 1;
  send_hid_report(REPORT_ID_KEYBOARD, senddat.buffer[senddat.tail]);
  ringbuff_tail_plus_one(&senddat);
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