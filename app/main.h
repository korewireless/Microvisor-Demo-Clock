/*
 * Microvisor Clock Demo -- main file
 *
 * @author      Tony Smith
 * @copyright   2023, KORE Wireless
 * @licence     MIT
 *
 */
#ifndef MAIN_H
#define MAIN_H


/*
 * INCLUDES
 */
#include <string>
#include <vector>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <algorithm>
// Microvisor + HAL
#include "stm32u5xx_hal.h"
#include "mv_syscalls.h"
// App
#include "i2c.h"
#include "ht16k33.h"
#include "clock.h"
#include "config.h"
#include "logging.h"
#include "uart_logging.h"
#include "ArduinoJson-v6.21.3.h"


/*
 * CONSTANTS
 */
#define     LED_GPIO_BANK               GPIOA
#define     LED_GPIO_PIN                GPIO_PIN_5

#define     I2C_GPIO_BANK               GPIOB


#endif      // MAIN_H
