/*
 * Microvisor Clock Demo -- main file
 *
 * @version     0.1.0
 * @author      Tony Smith
 * @copyright   2023, KORE Wireless
 * @licence     MIT
 *
 */
#ifndef _HT16K33_HEADER_
#define _HT16K33_HEADER_


/*
 * CONSTANTS
 */
#define HT16K33_GENERIC_DISPLAY_ON          0x81
#define HT16K33_GENERIC_DISPLAY_OFF         0x80
#define HT16K33_GENERIC_SYSTEM_ON           0x21
#define HT16K33_GENERIC_SYSTEM_OFF          0x20
#define HT16K33_GENERIC_DISPLAY_ADDRESS     0x00
#define HT16K33_GENERIC_CMD_BRIGHTNESS      0xE0
#define HT16K33_GENERIC_CMD_BLINK           0x81
#define HT16K33_ADDRESS                     0x70

#define HT16K33_SEGMENT_COLON_ROW           0x04
#define HT16K33_SEGMENT_MINUS_CHAR          0x10
#define HT16K33_SEGMENT_DEGREE_CHAR         0x11
#define HT16K33_SEGMENT_SPACE_CHAR          0x00


/**
    A basic driver for I2C-connected HT16K33-based four-digit, seven-segment displays.
 */
class HT16K33_Segment {

    public:
        // Constructor
        HT16K33_Segment(uint32_t address = HT16K33_ADDRESS);

        void                init(uint32_t brightness = 15);
        void                power_on(bool turn_on = true);
        void                set_brightness(uint32_t brightness = 15);
        HT16K33_Segment&    set_colon(bool is_set = false);
        HT16K33_Segment&    set_glyph(uint32_t glyph, uint32_t digit, bool has_dot = false);
        HT16K33_Segment&    set_number(uint32_t number, uint32_t digit, bool has_dot = false);
        HT16K33_Segment&    set_alpha(char chr, uint32_t digit, bool has_dot = false);
        HT16K33_Segment&    clear(void);
        void                draw(void);

    private:
        uint8_t             buffer[16];
        uint32_t            i2c_addr;
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
