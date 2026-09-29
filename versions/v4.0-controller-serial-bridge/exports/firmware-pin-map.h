#pragma once

/* NewPlayBridge2 production hardware handoff. */
#define PLAYBRIDGE_PIN_CMD             19
#define PLAYBRIDGE_PIN_CLK             32
#define PLAYBRIDGE_PIN_ATT             21
#define PLAYBRIDGE_PIN_DATA_DRIVE      33
#define PLAYBRIDGE_PIN_ACK_DRIVE       27
#define PLAYBRIDGE_PIN_ACK_SENSE       34
#define PLAYBRIDGE_PIN_STATUS_LED      25
#define PLAYBRIDGE_PIN_UART2_TX        17
#define PLAYBRIDGE_PIN_UART2_RX        16

/* ESP32 UART0 remains dedicated to CH340C programming/debug. */
#define PLAYBRIDGE_PIN_UART0_TX         1
#define PLAYBRIDGE_PIN_UART0_RX         3

/* DATA and ACK drives are external 2N7002 open-drain stages. */
#define PLAYBRIDGE_DATA_ACTIVE_LOW      1
#define PLAYBRIDGE_ACK_ACTIVE_LOW       1

