/*
 * Microvisor Clock Demo -- Clock class
 *
 * @version     1.0.0
 * @author      Tony Smith
 * @copyright   2023
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
        Clock(Prefs& in_prefs, HT16K33_Segment& in_display);
        
        bool                set_time_from_rtc(void);
        void                loop(void);
        
    private:
        uint32_t            bcd(uint32_t bin_value);
        bool                is_bst(void);
        bool                bst_check(void);
        uint32_t            day_of_week(int a_day, int a_month, int a_year);
        bool                is_leap_year(uint32_t a_year);
    
        uint32_t            hour = 0;
        uint32_t            minutes = 0;
        uint32_t            seconds = 0;
        uint32_t            year = 0;
        uint32_t            month = 0;
        uint32_t            day = 0;
        
        bool                is_time_set = false;
    
        Prefs               prefs;
        HT16K33_Segment     display;
};


#endif      // _CLOCK_HEADER_
