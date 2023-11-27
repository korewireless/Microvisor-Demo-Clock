/*
 * Microvisor Clock Demo -- main file
 *
 * @version     0.1.0
 * @author      Tony Smith
 * @copyright   2023, KORE Wireless
 * @licence     MIT
 *
 */
#include "main.h"
#include "app_version.h"


/*
 * STATIC PROTOTYPES
 */
static        void setupI2C(void);
static        void setupGPIO(void);
static        void logDeviceInfo(void);
static inline void setDefaults(Prefs& settings);

/*
 * GLOBALS
 */
// I2C-related values
I2C_HandleTypeDef i2c;
bool doUseI2C = false;


/**
 * @brief Get the MV clock value.
 *
 * @returns The clock value.
 */
uint32_t SECURE_SystemCoreClockUpdate() {

    uint32_t clock = 0;
    mvGetHClk(&clock);
    return clock;
}


/**
 * @brief System clock configuration.
 */
void system_clock_config(void) {

    SystemCoreClockUpdate();
    HAL_InitTick(TICK_INT_PRIORITY);
}


/**
 * @brief Initialize the MCU GPIO
 *
 * Used to flash the Nucleo's USER LED, which is on GPIO Pin PA5.
 * and as an interrupt source (GPIO Pin PF3) connected to the
 * LIS3DH motion sensor.
 */
static void setupGPIO(void) {

    // Enable GPIO port clock
    __HAL_RCC_GPIOA_CLK_ENABLE();

    // Configure GPIO pin for the on-board LED
    GPIO_InitTypeDef gpio_init = { 0 };
    gpio_init.Pin   = LED_GPIO_PIN;
    gpio_init.Mode  = GPIO_MODE_OUTPUT_PP;
    gpio_init.Pull  = GPIO_PULLUP;
    gpio_init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(LED_GPIO_BANK, &gpio_init);

    // Clear the LED
    HAL_GPIO_WritePin(LED_GPIO_BANK, LED_GPIO_PIN, GPIO_PIN_RESET);
}


/**
 * @brief Initialise the modem power pin.
 */
static void setupI2C(void) {

    // Initialize the I2C bus for the display and sensor
    I2C::setup(HT16K33_Segment::ADDRESS);
}


/**
 * @brief Punch in default settings.
 *
 * @param settings: Reference to the app's Prefs.
 */
static inline void setDefaults(Prefs& settings) {

    settings.mode = false;
    settings.bst = true;
    settings.colon = true;
    settings.flash = true;
    settings.brightness = 4;
}


/**
 * @brief Show basic device info.
 */
static void logDeviceInfo(void) {

    uint8_t buffer[35] = { 0 };
    mvGetDeviceId(buffer, 34);
    server_log("Device: %s", buffer);
    server_log("   App: %s %s", APP_NAME, APP_VERSION);
    server_log(" Build: %i", BUILD_NUM);
}


/*
 * RUNTIME START
 */

int main() {

    // Reset of all peripherals, initializes the Flash interface and the Systick.
    HAL_Init();

    // Configure the system clock
    system_clock_config();

    // Set up the hardware
    setupGPIO();
    setupI2C();

    // Instantiate the display driver
    HT16K33_Segment display = HT16K33_Segment(HT16K33_Segment::ADDRESS);
    display.init();

    // Create a preferencs store and
    // set the defaults
    Prefs prefs;
    setDefaults(prefs);

    // Display SYNC while we wait for the RTC to be set
    uint8_t sync[4] = {0x6D, 0x6E, 0x37, 0x39};
    display.init(prefs.brightness);
    for (uint32_t i = 0 ; i < 4 ; ++i) display.setGlyph(sync[i], i, false);
    display.draw();

    // Open the network
    // NOTE Do this before calling `log_device_info()`
    Config::Network::open();

    // Get the Device ID and build number
    logDeviceInfo();

    // Load in the clock settings
    bool gotPrefs = Config::getPrefs(prefs);
    if (gotPrefs) {
        server_log("Clock settings retrieved");
    } else {
        server_error("Clock settings not yet retrieved");
    }

    // Instantiate a Clock object and run it
    Clock mvclock = Clock(prefs, display, gotPrefs);
    mvclock.loop();

    // Just in case...
    return 0;
}
