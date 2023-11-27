/*
 * cellular::ht16k33_driver for Raspberry Pi Pico
 *
 * @version     0.1.0
 * @author      Tony Smith
 * @copyright   2023, KORE Wireless
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
HT16K33_Segment::HT16K33_Segment(uint8_t address)
    :i2cAddr(address),
    CHARSET{0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F,
            0x6F, 0x5F, 0x7C, 0x58, 0x5E, 0x7B, 0x71, 0x40, 0x63},
    POS{0, 2, 6, 8}
{
    if (i2cAddr == 0x00 || i2cAddr > 0x7F) i2cAddr = ADDRESS;
}


/**
 * @brief Convenience function to power on the display
 *        and set basic parameters.
 */
void HT16K33_Segment::init(uint32_t brightness) {

    power(true);
    setBrightness(brightness);
    clear();
    draw();
}


/**
 * @brief Power the display on or off.
 *
 * @param on: `true` to turn the display on, `false` to turn it off.
              Default: `true`.
 */
void HT16K33_Segment::power(bool on) {

    I2C::writeByte(i2cAddr, on ? HT16K33_Segment::GENERIC_SYSTEM_ON : HT16K33_Segment::GENERIC_DISPLAY_OFF);
    I2C::writeByte(i2cAddr, on ? HT16K33_Segment::GENERIC_DISPLAY_ON : HT16K33_Segment::GENERIC_SYSTEM_OFF);
}


/**
 * @brief Set the display brighness.
 *
 * @param brightness: A value from 0 to 15. Default: 15.
 */
void HT16K33_Segment::setBrightness(uint32_t brightness) {

    if (brightness > 15) brightness = 15;
    I2C::writeByte(i2cAddr, HT16K33_Segment::GENERIC_CMD_BRIGHTNESS | brightness);
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
 * @param isSet: `true` if the colon is to be lit, otherwise `false`.
 *
 * @retval The instance.
 */
HT16K33_Segment& HT16K33_Segment::setColon(bool isSet) {

    buffer[HT16K33_Segment::SEGMENT_COLON_ROW] = isSet ? 0x02 : 0x00;
    return *this;
}


/**
 * @brief Present a user-defined character glyph at the specified digit.
 *
 * @param glyph:  The glyph value.
 * @param digit:  The target digit: L-R, 0-4.
 * @param hasDot: `true` if the decimal point is to be lit, otherwise `false`.
 *                Default: `false`.
 *
 * @retval The instance.
 */
HT16K33_Segment& HT16K33_Segment::setGlyph(uint32_t glyph, uint32_t digit, bool hasDot) {

    if (digit > 4) return *this;
    if (glyph > 0xFF) return *this;
    buffer[HT16K33_Segment::POS[digit]] = glyph;
    if (hasDot) buffer[HT16K33_Segment::POS[digit]] |= 0x80;
    return *this;
}


/**
 * @brief Present a decimal number at the specified digit.
 *
 * @param number: The number (0-9).
 * @param digit:  The target digit: L-R, 0-4.
 * @param hasDot: `true` if the decimal point is to be lit, otherwise `false`.
 *                efault: `false`.
 *
 * @retval The instance.
 */
HT16K33_Segment& HT16K33_Segment::setNumber(uint32_t number, uint32_t digit, bool hasDot) {

    if (digit > 4) return *this;
    if (number > 9) return *this;
    return setAlpha('0' + number, digit, hasDot);
}


/**
 * @brief Present an alphanumeric character glyph at the specified digit.
 *
 * @param chr:    The character.
 * @param digit:  The target digit: L-R, 0-4.
 * @param hasDot: `true` if the decimal point is to be lit, otherwise `false`.
 *                Default: `false`.
 *
 * @retval The instance.
 */
HT16K33_Segment& HT16K33_Segment::setAlpha(char chr, uint32_t digit, bool hasDot) {

    if (digit > 4) return *this;

    uint8_t charVal = 0xFF;
    if (chr == ' ') {
        charVal = HT16K33_Segment::SEGMENT_SPACE_CHAR;
    } else if (chr == '-') {
        charVal = HT16K33_Segment::SEGMENT_MINUS_CHAR;
    } else if (chr == 'o') {
        charVal = HT16K33_Segment::SEGMENT_DEGREE_CHAR;
    } else if (chr >= 'a' && chr <= 'f') {
        charVal = (uint8_t)chr - 87;
    } else if (chr >= '0' && chr <= '9') {
        charVal = (uint8_t)chr - 48;
    }

    if (charVal == 0xFF) return *this;
    buffer[HT16K33_Segment::POS[digit]] = HT16K33_Segment::CHARSET[charVal];
    if (hasDot) buffer[HT16K33_Segment::POS[digit]] |= 0x80;
    return *this;
}


/**
 * @brief Write the display buffer out to I2C.
 */
void HT16K33_Segment::draw() {

    // Set up the buffer holding the data to be
    // transmitted to the LED
    uint8_t txBuffer[17] = {0};

    // Span the 8 bytes of the graphics buffer
    // across the 16 bytes of the LED's buffer
    for (uint32_t i = 0 ; i < 16 ; ++i) {
        txBuffer[i + 1] = buffer[i];
    }

    // Write out the transmit buffer
    I2C::writeBlock(i2cAddr, txBuffer, sizeof(txBuffer));
}
