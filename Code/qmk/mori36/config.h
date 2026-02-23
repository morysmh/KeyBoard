// Copyright 2018-2020
// ENDO Katsuhiro <ka2hiro@curlybracket.co.jp>
// David Philip Barr <@davidphilipbarr>
// Pierre Chevalier <pierrechevalier83@gmail.com>
// SPDX-License-Identifier: GPL-2.0+

#pragma once

//#define EE_HANDS
#define MASTER_RIGHT

#define SERIAL_USART_FULL_DUPLEX
#define SERIAL_USART_TX_PIN GP4
#define SERIAL_USART_RX_PIN GP5

/* RP2040- and hardware-specific config */
#define RP2040_BOOTLOADER_DOUBLE_TAP_RESET // Activates the double-tap behavior
#define RP2040_BOOTLOADER_DOUBLE_TAP_RESET_TIMEOUT 500U
#define PICO_XOSC_STARTUP_DELAY_MULTIPLIER 64
