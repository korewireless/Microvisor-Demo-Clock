/*
 * Microvisor Clock Demo -- HT16K33 display driver
 *
 * @author      Tony Smith
 * @copyright   2023, KORE Wireless
 * @licence     MIT
 *
 */
#ifndef _HT16K33_HEADER_
#define _HT16K33_HEADER_


/**
    A basic driver for I2C-connected HT16K33-based four-digit, seven-segment displays.
 */
class HT16K33_Segment {

    public:
        // Constants
        enum class CMD {
            GENERIC_DISPLAY_ON =        0x81,
            GENERIC_DISPLAY_OFF =       0x80,
            GENERIC_SYSTEM_ON =         0x21,
            GENERIC_SYSTEM_OFF =        0x20,
            GENERIC_DISPLAY_ADDRESS =   0x00,
            GENERIC_BRIGHTNESS =        0xE0,
            GENERIC_BLINK =             0x81
        };

        enum class DATA {
            ADDRESS =                   0x70
        };

        enum class SEGMENT {
            COLON_ROW =                 0x04,
            MINUS_CHAR =                0x10,
            DEGREE_CHAR =               0x11,
            SPACE_CHAR =                0x00
        };

        // Constructor
        explicit HT16K33_Segment(uint8_t address = (uint8_t)DATA::ADDRESS);
        // Methods
        void                init(uint32_t brightness = 15);
        void                power(bool doTurnOn = true) const;
        void                setBrightness(uint32_t brightness = 15) const;
        HT16K33_Segment&    setColon(bool isSet = false);
        HT16K33_Segment&    setGlyph(uint32_t glyph, uint32_t digit, bool hasDot = false);
        HT16K33_Segment&    setNumber(uint32_t number, uint32_t digit, bool hasDot = false);
        HT16K33_Segment&    setAlpha(char chr, uint32_t digit, bool hasDot = false);
        HT16K33_Segment&    clear(void);
        void                draw(void) const;



    private:
        // Methods
        uint8_t         buffer[16];
        uint8_t         i2cAddr;
        // Constants
        // Following are populated in the constructor
        const uint8_t   CHARSET[18] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F,
                                       0x6F, 0x5F, 0x7C, 0x58, 0x5E, 0x7B, 0x71, 0x40, 0x63};
        const uint8_t   POS[4] = {0, 2, 6, 8};
};


#endif  // _HT16K33_HEADER_
