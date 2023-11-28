/*
 * Microvisor Clock Demo -- I2C namespace
 *
 * @author      Tony Smith
 * @copyright   2023, KORE Wireless
 * @licence     MIT
 *
 */
#include "main.h"


/*
 * GLOBALS
 */
// Defined in `main.cpp`
extern      I2C_HandleTypeDef   i2c;
extern      bool                doUseI2C;


#ifdef __cplusplus
extern "C" {
#endif
// Required on STM32 HAL callouts implemented in C++
void HAL_I2C_MspInit(I2C_HandleTypeDef *i2c);
#ifdef __cplusplus
}
#endif


namespace I2C {

/**
 * @brief Check for presence of a known device by its I2C address.
 *
 * @param address: The device's address.
 *
 * @returns `true` if the device is present, otherwise `false`.
 */
static bool check(uint32_t address) {

    uint8_t timeoutCount = 0;

    while(true) {
        HAL_StatusTypeDef status = HAL_I2C_IsDeviceReady(&i2c, (uint8_t)address << 1, 1, 100);
        if (status == HAL_OK) {
            return true;
        } else {
            uint32_t err = HAL_I2C_GetError(&i2c);
            server_error("HAL_I2C_IsDeviceReady() : %i", status);
            server_error("HAL_I2C_GetError():       %li", err);
        }

        // Flash the LED eight times on device not ready
        for (uint8_t i = 0 ; i < 8 ; ++i) {
            HAL_GPIO_TogglePin(LED_GPIO_BANK, LED_GPIO_PIN);
            HAL_Delay(100);
        }

        HAL_Delay(1000);
        timeoutCount++;
        if (timeoutCount > 10) break;
    }

    return false;
}


/**
 * @brief Set up the I2C block.
 *
 * Takes values from #defines set in `i2c.h`
 */
void setup(uint32_t targetAddress) {

    // I2C1 pins are:
    //   SDA -> PB9
    //   SCL -> PB6
    i2c.Instance              = I2C1;
    i2c.Init.Timing           = 0x00C01F67;  // FROM ST SAMPLE
    i2c.Init.AddressingMode   = I2C_ADDRESSINGMODE_7BIT;
    i2c.Init.DualAddressMode  = I2C_DUALADDRESS_DISABLE;
    i2c.Init.OwnAddress1      = 0x00;
    i2c.Init.OwnAddress2      = 0x00;
    i2c.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    i2c.Init.GeneralCallMode  = I2C_GENERALCALL_DISABLE;
    i2c.Init.NoStretchMode    = I2C_NOSTRETCH_ENABLE;

    // Initialize the I2C itself with the i2c handle
    if (HAL_I2C_Init(&i2c) != HAL_OK) {
        server_error("I2C init failed");
        return;
    }

    // I2C is up, so check peripheral availability
    doUseI2C = check(targetAddress);
}


/**
 * @brief Convenience function to write a single byte to the bus.
 *
 * @param address: The I2C address of the device to write to.
 * @param byte:    The byte to send.
 */
void writeByte(uint8_t address, uint8_t byte) {

    HAL_I2C_Master_Transmit(&i2c, address << 1, &byte, 1, 100);
}


/**
 * @brief Convenience function to write a single byte to the bus.
 *
 * @param address: The I2C address of the device to write to.
 * @param byte:    The byte to send.
 */
void writeBlock(uint8_t address, uint8_t *data, uint8_t count) {

    HAL_I2C_Master_Transmit(&i2c, address << 1, data, count, 100);
}


}   // namespace I2C


/**
 * @brief HAL-called function to configure I2C.
 *
 * @param i2c: A HAL I2C_HandleTypeDef pointer to the I2C instance.
 */
void HAL_I2C_MspInit(I2C_HandleTypeDef *i2c) {

    // This SDK-named function is called by HAL_I2C_Init()

    // Configure U5 peripheral clock
    RCC_PeriphCLKInitTypeDef PeriphClkInit = { 0 };
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C1;
    PeriphClkInit.I2c1ClockSelection   = RCC_I2C1CLKSOURCE_PCLK1;

    // Initialize U5 peripheral clock
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
        server_error("HAL_RCCEx_PeriphCLKConfig() failed");
        return;
    }

    // Enable the I2C GPIO interface clock
    __HAL_RCC_GPIOB_CLK_ENABLE();

    // Configure the GPIO pins for I2C
    // Pin PB6 - SCL
    // Pin PB9 - SDA
    GPIO_InitTypeDef gpioConfig = { 0 };
    gpioConfig.Pin       = GPIO_PIN_6 | GPIO_PIN_9;
    gpioConfig.Mode      = GPIO_MODE_AF_OD;
    gpioConfig.Pull      = GPIO_NOPULL;
    gpioConfig.Speed     = GPIO_SPEED_FREQ_LOW;
    gpioConfig.Alternate = GPIO_AF4_I2C1;

    // Initialize the pins with the setup data
    HAL_GPIO_Init(I2C_GPIO_BANK, &gpioConfig);

    // Enable the I2C1 clock
    __HAL_RCC_I2C1_CLK_ENABLE();
}
