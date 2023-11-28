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
        // Constructor
        HT16K33_Segment(uint8_t address = ADDRESS);
        // Methods
        void                init(uint32_t brightness = 15);
        void                power(bool doTurnOn = true);
        void                setBrightness(uint32_t brightness = 15);
        HT16K33_Segment&    setColon(bool isSet = false);
        HT16K33_Segment&    setGlyph(uint32_t glyph, uint32_t digit, bool hasDot = false);
        HT16K33_Segment&    setNumber(uint32_t number, uint32_t digit, bool hasDot = false);
        HT16K33_Segment&    setAlpha(char chr, uint32_t digit, bool hasDot = false);
        HT16K33_Segment&    clear(void);
        void                draw(void);

        // Constants
        /*
        static const uint8_t GENERIC_DISPLAY_ON =       0x81;
        static const uint8_t GENERIC_DISPLAY_OFF =      0x80;
        static const uint8_t GENERIC_SYSTEM_ON =        0x21;
        static const uint8_t GENERIC_SYSTEM_OFF =       0x20;
        static const uint8_t GENERIC_DISPLAY_ADDRESS =  0x00;
        static const uint8_t GENERIC_CMD_BRIGHTNESS =   0xE0;
        static const uint8_t GENERIC_CMD_BLINK =        0x81;
        static const uint8_t ADDRESS =                  0x70;
        static const uint8_t SEGMENT_COLON_ROW =        0x04;
        static const uint8_t SEGMENT_MINUS_CHAR =       0x10;
        static const uint8_t SEGMENT_DEGREE_CHAR =      0x11;
        static const uint8_t SEGMENT_SPACE_CHAR =       0x00;
        */
        enum {
            GENERIC_DISPLAY_ON =       0x81,
            GENERIC_DISPLAY_OFF =      0x80,
            GENERIC_SYSTEM_ON =        0x21,
            GENERIC_SYSTEM_OFF =       0x20,
            GENERIC_DISPLAY_ADDRESS =  0x00,
            GENERIC_CMD_BRIGHTNESS =   0xE0,
            GENERIC_CMD_BLINK =        0x81,
            ADDRESS =                  0x70,
            SEGMENT_COLON_ROW =        0x04,
            SEGMENT_MINUS_CHAR =       0x10,
            SEGMENT_DEGREE_CHAR =      0x11,
            SEGMENT_SPACE_CHAR =       0x00
        };

    private:
        // Methods
        uint8_t             buffer[16];
        uint32_t            i2cAddr;
        // Constants
        // Following are populated in the constructor
        const uint8_t       CHARSET[18];
        const uint8_t       POS[4];

        /*
        // The key alphanumeric characters we can show:
        // 0-9, A-F, minus, degree
        static constexpr uint8_t  CHARSET[18] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F,
                                                 0x6F, 0x5F, 0x7C, 0x58, 0x5E, 0x7B, 0x71, 0x40, 0x63};

        // The positions of the segments within the buffer
        static constexpr uint32_t POS[4] = {0, 2, 6, 8};
        */
};


#endif  // _HT16K33_HEADER_
