/*
 * Microvisor Clock Demo -- Clock class
 *
 * @author      Tony Smith
 * @copyright   2024, KORE Wireless
 * @licence     MIT
 *
 */
#ifndef _CLOCK_HEADER_
#define _CLOCK_HEADER_


typedef struct {
    bool        mode;       // `true` for 24-hour clock; `false` for 12-hout clock
    bool        bst;        // Display according to current daylight savings
    bool        colon;      // Show the colon separator between hours and minutes on the display
    bool        flash;      // Flash the colon separator if it's being shown
    bool        led;        // Flash the LED in sync with the colon
    uint32_t    brightness; // Display brightness (1-15)
} Prefs;


class Clock {

    public:
        // Constructor
        Clock(const Prefs& inPrefs, const HT16K33_Segment& inDisplay, const bool gotPrefs);
        // Methods
        bool                setTimeFromRTC(void);
        [[noreturn]] void   loop(void);

    private:
        //Methods
        uint32_t            bcd(uint32_t bin_value) const;
        bool                isBST(void) const;
        bool                bstCheck(void) const;
        uint32_t            dayOfWeek(int a_day, int a_month, int a_year) const;
        bool                isLeapYear(uint32_t a_year) const;
        // Properties
        uint32_t            hour = 0;
        uint32_t            minutes = 0;
        uint32_t            seconds = 0;
        uint32_t            year = 0;
        uint32_t            month = 0;
        uint32_t            day = 0;
        // Following set by constructor
        Prefs               prefs;
        HT16K33_Segment     display;
        bool                receivedPrefs;
};


#endif      // _CLOCK_HEADER_
