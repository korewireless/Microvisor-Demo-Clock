/*
 * Microvisor Clock Demo -- main file
 *
 * @author      Tony Smith
 * @copyright   2024, KORE Wireless
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
#include <ArduinoJson.h>


/*
 * CONSTANTS
 */
#define     LED_GPIO_BANK               GPIOA
#define     LED_GPIO_PIN                GPIO_PIN_5

#define     I2C_GPIO_BANK               GPIOB


/*
 * ENUMERATIONS
 */
enum class NET_STATE: uint32_t {
    OFFLINE = 0,
    ONLINE = 1,
    CONNECTING = 2,
    UNKNOWN = 99
};


#endif      // MAIN_H
