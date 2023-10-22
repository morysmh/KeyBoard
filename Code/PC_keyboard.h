#ifndef PC_Keybard_Handler
#define	PC_Keybard_Handler

#include "define.h"
#include "keyboard.h"
#include "tusb.h"
#include "usb_descriptors.h"

enum  {
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED = 1000,
  BLINK_SUSPENDED = 2500,
};

void hid_task(void);

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize);
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen);
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint8_t len);
static void send_hid_report(uint8_t report_id, uint32_t btn);
void tud_mount_cb(void);
void tud_umount_cb(void);
void tud_suspend_cb(bool remote_wakeup_en);
void tud_resume_cb(void);

void write_on_keyboard(uint8_t i_key);

void send_if_data_available();
#endif