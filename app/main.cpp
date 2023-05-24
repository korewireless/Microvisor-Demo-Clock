/*
 * Microvisor Clock Demo -- main file
 *
 * @version     1.0.0
 * @author      Tony Smith
 * @copyright   2023
 * @licence     MIT
 *
 */
#include "main.h"
#include "app_version.h"


using std::vector;
using std::string;
using std::vector;
using std::stringstream;


/*
 * STATIC PROTOTYPES
 */
static void setup_i2c(void);
static void setup_gpio(void);
static void log_device_info(void);
static inline void set_defaults(Prefs& settings);

/*
 * GLOBALS
 */
// I2C-related values
I2C_HandleTypeDef i2c;
bool do_use_i2c = false;


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
static void setup_gpio(void) {
    
    // Enable GPIO port clock
    __HAL_RCC_GPIOA_CLK_ENABLE();

    // Configure GPIO pin output Level
    HAL_GPIO_WritePin(LED_GPIO_BANK, LED_GPIO_PIN, GPIO_PIN_RESET);

    // Configure GPIO pin for the on-board LED
    GPIO_InitTypeDef gpio_init = { 0 };
    gpio_init.Pin   = LED_GPIO_PIN;
    gpio_init.Mode  = GPIO_MODE_OUTPUT_PP;
    gpio_init.Pull  = GPIO_PULLUP;
    gpio_init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(LED_GPIO_BANK, &gpio_init);
}


/**
 * @brief Initialise the modem power pin.
 */
static void setup_i2c(void) {
    
    // Initialize the I2C bus for the display and sensor
    I2C::setup();
}


/**
 * @brief Show basic device info.
 */
static void log_device_info(void) {
    
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
    
    // Open the network
    Config::Network::open();
    
    // Get the Device ID and build number
    log_device_info();
    
    // Set up the hardware
    setup_gpio();
    setup_i2c();
    
    // Instantiate the display driver
    HT16K33_Segment display = HT16K33_Segment(HT16K33_ADDRESS);
    
    // Set the default prefs
    Prefs prefs;
    set_defaults(prefs);
    
    // Display SYNC while we wait for the RTC to be set
    uint8_t sync[4] = {0x6D, 0x6E, 0x37, 0x39};
    display.init(prefs.brightness);
    for (uint32_t i = 0 ; i < 4 ; ++i) display.set_glyph(sync[i], i, false);
    display.draw();
    
    Config::Channel::open();
    if (Config::get_prefs(prefs)) server_log("GOT PREFS");
    
    // Instantiate a Clock object and run it
    Clock mvclock = Clock(prefs, display);
    mvclock.loop();

    return 0;
}


/**
 * @brief Punch in default settings.
 *
 * @param settings: Reference to the app's Prefs.
 */
static inline void set_defaults(Prefs& settings) {
    
    settings.mode = false;
    settings.bst = true;
    settings.colon = true;
    settings.flash = true;
    settings.brightness = 4;
}
