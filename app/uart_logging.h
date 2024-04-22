/**
 *
 * Microvisor Remote Debugging Demo
 *
 * Copyright © 2024, KORE Wireless
 * Licence: Apache 2.0
 *
 */
#ifndef UART_LOGGING_H
#define UART_LOGGING_H


/*
 * INCLUDES
 */
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <assert.h>
// Microvisor + HAL
#include "stm32u5xx_hal.h"
#include "mv_syscalls.h"
// App
#include "logging.h"

/*
 * CONSTANTS
 */
#define UART_LOG_TIMESTAMP_MAX_LEN_B        64
#define UART_LOG_MESSAGE_MAX_LEN_B          1024


#ifdef __cplusplus
extern "C" {
#endif


/*
 * PROTOTYPES
 */
bool    log_uart_init(void);
void    log_uart_output(const char* buffer);


#ifdef __cplusplus
}
#endif


#endif
