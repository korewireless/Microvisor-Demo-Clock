/*
 * Microvisor Clock Demo -- main file
 *
 * @version     0.1.0
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
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <algorithm>

// Microvisor includes
#include "stm32u5xx_hal.h"
#include "mv_syscalls.h"

// App includes
#include "i2c.h"
#include "ht16k33.h"
#include "clock.h"
#include "config.h"
#include "logging.h"
#include "uart_logging.h"
#include "ArduinoJson-v6.21.3.h"


using std::vector;
using std::string;
using std::vector;
using std::stringstream;


/*
 * CONSTANTS
 */
#define     LED_GPIO_BANK               GPIOA
#define     LED_GPIO_PIN                GPIO_PIN_5
#define     LED_FLASH_PERIOD_MS         1000


#ifdef __cplusplus
extern "C" {
#endif



#ifdef __cplusplus
}           // extern "C"
#endif


#endif      // MAIN_H
