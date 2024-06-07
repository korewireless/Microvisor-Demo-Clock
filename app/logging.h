/**
 *
 * Microvisor Remote Debugging Demo
 *
 * Copyright © 2024, KORE Wireless
 * Licence: MIT
 *
 */
#ifndef LOGGING_H
#define LOGGING_H


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

// Microvisor includes
#include "stm32u5xx_hal.h"
#include "mv_syscalls.h"

// App includes
#include "uart_logging.h"


/*
 * CONSTANTS
 */
#define     USER_HANDLE_LOGGING_STARTED         0xFFFF
#define     USER_HANDLE_LOGGING_OFF             0

#define     LOG_MESSAGE_MAX_LEN_B               1024
#define     LOG_BUFFER_SIZE_B                   4096


#ifdef __cplusplus
extern "C" {
#endif


/*
 * PROTOTYPES
 */
void            server_log(const char* format_string, ...);
void            server_error(const char* format_string, ...);


#ifdef __cplusplus
}
#endif


#endif /* LOGGING_H */
