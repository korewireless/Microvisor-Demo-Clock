/*
 * cellular::ht16k33_driver for Raspberry Pi Pico
 *
 * @version     1.0.0
 * @author      Tony Smith
 * @copyright   2023
 * @licence     MIT
 *
 */
#include "main.h"


using std::string;


/**
 * @brief Basic driver for HT16K33-based display.
 *
 * @param address: The display's I2C address. Default: 0x70.
 */
HT16K33_Segment::HT16K33_Segment(uint32_t address) {
    
    if (address == 0x00 || address > 0x7F) address = HT16K33_ADDRESS;
    i2c_addr = address;
}


/**
 * @brief Convenience function to power on the display
 *        and set basic parameters.
 */
void HT16K33_Segment::init(uint32_t brightness) {
    
    power_on(true);
    set_brightness(brightness);
    clear();
    draw();
}


/**
 * @brief Power the display on or off.
 *
 * @param on: `true` to turn the display on, `false` to turn it off.
              Default: `true`.
 */
void HT16K33_Segment::power_on(bool on) {
    
    I2C::write_byte(i2c_addr, on ? HT16K33_GENERIC_SYSTEM_ON : HT16K33_GENERIC_DISPLAY_OFF);
    I2C::write_byte(i2c_addr, on ? HT16K33_GENERIC_DISPLAY_ON : HT16K33_GENERIC_SYSTEM_OFF);
}


/**
 * @brief Set the display brighness.
 *
 * @param brightness: A value from 0 to 15. Default: 15.
 */
void HT16K33_Segment::set_brightness(uint32_t brightness) {
    
    if (brightness > 15) brightness = 15;
    I2C::write_byte(i2c_addr, HT16K33_GENERIC_CMD_BRIGHTNESS | brightness);
}


/**
 * @brief Clear the display buffer and then write it out.
 *
 * @retval The instance.
 */
HT16K33_Segment& HT16K33_Segment::clear() {
    
    memset(buffer, 0x00, 16);
    return *this;
}

/**
 * @brief Set or unset the display colon.
 *
 * @param is_set: `true` if the colon is to be lit, otherwise `false`.
 *
 * @retval The instance.
 */
HT16K33_Segment& HT16K33_Segment::set_colon(bool is_set) {
    
    buffer[HT16K33_SEGMENT_COLON_ROW] = is_set ? 0x02 : 0x00;
    return *this;
}


/**
 * @brief Present a user-defined character glyph at the specified digit.
 *
 * @param glyph:   The glyph value.
 * @param digit:   The target digit: L-R, 0-4.
 * @param has_dot: `true` if the decimal point is to be lit, otherwise `false`.
 *                 Default: `false`.
 *
 * @retval The instance.
 */
HT16K33_Segment& HT16K33_Segment::set_glyph(uint32_t glyph, uint32_t digit, bool has_dot) {
    
    if (digit > 4) return *this;
    if (glyph > 0xFF) return *this;
    buffer[POS[digit]] = glyph;
    if (has_dot) buffer[POS[digit]] |= 0x80;
    return *this;
}


/**
 * @brief Present a decimal number at the specified digit.
 *
 * @param number:  The number (0-9).
 * @param digit:   The target digit: L-R, 0-4.
 * @param has_dot: `true` if the decimal point is to be lit, otherwise `false`.
 *                 Default: `false`.
 *
 * @retval The instance.
 */
HT16K33_Segment& HT16K33_Segment::set_number(uint32_t number, uint32_t digit, bool has_dot) {
    
    if (digit > 4) return *this;
    if (number > 9) return *this;
    return set_alpha('0' + number, digit, has_dot);
}


/**
 * @brief Present an alphanumeric character glyph at the specified digit.
 *
 * @param chr:     The character.
 * @param digit:   The target digit: L-R, 0-4.
 * @param has_dot: `true` if the decimal point is to be lit, otherwise `false`.
 *                 Default: `false`.
 *
 * @retval The instance.
 */
HT16K33_Segment& HT16K33_Segment::set_alpha(char chr, uint32_t digit, bool has_dot) {
    
    if (digit > 4) return *this;

    uint8_t char_val = 0xFF;
    if (chr == ' ') {
        char_val = HT16K33_SEGMENT_SPACE_CHAR;
    } else if (chr == '-') {
        char_val = HT16K33_SEGMENT_MINUS_CHAR;
    } else if (chr == 'o') {
        char_val = HT16K33_SEGMENT_DEGREE_CHAR;
    } else if (chr >= 'a' && chr <= 'f') {
        char_val = (uint8_t)chr - 87;
    } else if (chr >= '0' && chr <= '9') {
        char_val = (uint8_t)chr - 48;
    }

    if (char_val == 0xFF) return *this;
    buffer[POS[digit]] = CHARSET[char_val];
    if (has_dot) buffer[POS[digit]] |= 0x80;
    return *this;
}


/**
 * @brief Write the display buffer out to I2C.
 */
void HT16K33_Segment::draw() {
    
    // Set up the buffer holding the data to be
    // transmitted to the LED
    uint8_t tx_buffer[17] = {0};

    // Span the 8 bytes of the graphics buffer
    // across the 16 bytes of the LED's buffer
    for (uint32_t i = 0 ; i < 16 ; ++i) {
        tx_buffer[i + 1] = buffer[i];
    }

    // Write out the transmit buffer
    I2C::write_block(i2c_addr, tx_buffer, sizeof(tx_buffer));
}