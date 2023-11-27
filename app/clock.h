/*
 * Microvisor Clock Demo -- Clock class
 *
 * @version     0.1.0
 * @author      Tony Smith
 * @copyright   2023, KORE Wireless
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
    uint32_t    brightness; // Display brightness (1-15)
} Prefs;


class Clock {

    public:
        // Constructor
        Clock(Prefs& inPrefs, HT16K33_Segment& inDisplay, bool gotPrefs);
        // Methods
        bool                setTimeFromRTC(void);
        void                loop(void);

    private:
        //Methods
        uint32_t            bcd(uint32_t bin_value);
        bool                isBST(void);
        bool                bstCheck(void);
        uint32_t            dayOfWeek(int a_day, int a_month, int a_year);
        bool                isLeapYear(uint32_t a_year);
        // Properties
        uint32_t            hour = 0;
        uint32_t            minutes = 0;
        uint32_t            seconds = 0;
        uint32_t            year = 0;
        uint32_t            month = 0;
        uint32_t            day = 0;
        bool                isTimeSet = false;
        // Following set by constructor
        Prefs               prefs;
        HT16K33_Segment     display;
        bool                receivedPrefs;
};


#endif      // _CLOCK_HEADER_
