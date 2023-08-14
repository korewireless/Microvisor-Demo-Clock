/*
 * Microvisor Clock Demo -- Clock class
 *
 * @version     1.0.0
 * @author      Tony Smith
 * @copyright   2023, KORE Wireless
 * @licence     MIT
 *
 */
#include "main.h"


using std::vector;
using std::string;
using std::vector;
using std::stringstream;


/**
 * @brief Basic driver for HT16K33-based display.
 *
 * @param in_prefs:   Reference to the app's preferences data.
 * @param in_display: Reference to the app's display instance.
 * @param got_prefs:  Have we loaded clock settings yet?
 */
Clock::Clock(Prefs& in_prefs, HT16K33_Segment& in_display, bool got_prefs)

    :prefs(in_prefs),
     display(in_display),
     received_prefs(got_prefs)
{

}


/**
 * @brief Set the current time from the STM32U5 RTC
 *        via Microvisor.
 *
 * @returns `true` if the time was set, otherwise `false`.
 */
bool Clock::set_time_from_rtc(void) {
    
    static char timestamp[65] = {0};
    uint64_t usec = 0;
    
    if (mvGetWallTime(&usec) == MV_STATUS_OKAY) {
        // Get the time in seconds
        time_t secs = (time_t)usec / 1000000;
        
        // Write time string as "2022-05-10 13:30:58"
        strftime(timestamp, 48, "%F %T", gmtime(&secs));

        // Convert the hour, minute and second values back
        sscanf(timestamp, "%lu-%lu-%lu %lu:%lu:%lu", &year, &month, &day, &hour, &minutes, &seconds);
        return true;
    }
    
    return false;
}


/**
 * @brief Loop the clock display and update routine.
 */
void Clock::loop(void) {

    while (true) {
        // Check the time
        set_time_from_rtc();

        // Update display hour for DST, if allowed
        uint32_t display_hour = hour;
        if (prefs.bst && is_bst()) display_hour = (display_hour + 1) % 24;
        bool is_pm = (display_hour > 11);

        // Calculate and set the hours digits
        if (!prefs.mode) {
            if (is_pm) display_hour -= 12;
            if (display_hour == 0) display_hour = 12;
        }

        // Display the hour
        // The decimal point by the first digit is used to indicate connection status
        // (lit if the clock is disconnected)
        uint32_t decimal = bcd(display_hour);
        display.set_number(decimal & 0x0F, 1, false);
        if (!prefs.mode && display_hour < 10) {
            display.set_glyph(0, 0, false);
        } else {
            display.set_number(decimal >> 4, 0, false);
        }

        // Display the minute
        // The decimal point by the last digit is used to indicate AM/PM,
        // but only for the 12-hour clock mode (mode == False)
        decimal = bcd(minutes);
        display.set_number(decimal >> 4, 2, false);
        display.set_number(decimal & 0x0F, 3, (prefs.mode ? false : is_pm));

        // Set the colon and present the display
        if (prefs.colon) {
            if (prefs.flash) {
                display.set_colon(seconds % 2 == 0);
                HAL_GPIO_WritePin(LED_GPIO_BANK, LED_GPIO_PIN, seconds % 2 == 0 ? GPIO_PIN_SET : GPIO_PIN_RESET);
            } else {
                display.set_colon(prefs.colon);
            }
        }

        display.draw();

        // Reload prefs if we haven't done so yet
        if (!received_prefs && (minutes % 2 == 0)) {
            received_prefs = Config::get_prefs(prefs);
            if (received_prefs) server_log("Clock settings retrieved");
        }
    }
}


/**
 * @brief Convert an integer to a bimary coded decimal representation.
 *
 * @param raw_int: The source value.
 *
 * @returns The BCD encoding.
 */
uint32_t Clock::bcd(uint32_t raw_int) {

    for (uint32_t i = 0 ; i < 8 ; ++i) {
        raw_int = raw_int << 1;
        if (i == 7) break;
        if ((raw_int & 0x0F00) > 0x04FF) raw_int += 0x0300;
        if ((raw_int & 0xF000) > 0x4FFF) raw_int += 0x3000;
    }
    
    return (raw_int >> 8) & 0xFF;
}


/**
 * @brief Are we in daylight savings time?
 *
 * @returns `true` if DST is active, otherwise `false`.
 */
bool Clock::is_bst(void) {
    
    return bst_check();
}


/**
 * @brief Are we in daylight savings time?
 *        This assumes the clock is in the UK, but could
 *        easily be adapted for other locations.
 *
 * @returns `true` if DST is active, otherwise `false`.
 */
bool Clock::bst_check(void) {
    
    if (month > 3 and month < 10) return true;

    if (month == 3) {
        // BST starts on the last Sunday of March
        for (int i = 31 ; i > 24 ; --i) {
            if ((day_of_week(i, 3, (int)year) == 0) && (day >= (uint32_t)i)) return true;
        }
    }
    
    if (month == 10) {
        // BST ends on the last Sunday of October
        for (int i = 31 ; i > 24 ; --i) {
            if ((day_of_week(i, 10, (int)year) == 0) && (day < (uint32_t)i)) return true;
        }
    }
    
    return false;
}


/**
 * @brief Determine the day of the week for a given day, month and year, using
 *        Zeller's Rule (see http://mathforum.org/dr.math/faq/faq.calendar.html).
 *
 * @param a_day:   The day of the month (1-31).
 * @param a_month: A month (1-12).
 * @param a_year:  A year (including the century, ie. '2019' not '19').
 *
 * @returns The day of the week: 0 (Monday) to 6 (Sunday).
 */
uint32_t Clock::day_of_week(int a_day, int a_month, int a_year) {
    
    a_month -= 2;
    if (a_month < 1) a_month += 12;
    uint32_t century = (int)(a_year / 100);
    a_year -= (century * 100);
    a_year -= (month > 10 ? 1 : 0);
    
    int dow = a_day + (int)((13 * a_month - 1) / 5) + a_year + (int)(year / 4) + (int)(century / 4) - (2 * century);
    dow %= 7;
    if (dow < 0) dow += 7;
    return (uint32_t)dow;
}


/**
 * @brief Is the specified year a leap year?
 *
 * @param a_year:  A year (including the century, ie. '2019' not '19').
 *
 * @returns `true` if the year is a leap year, otherwise `false`.
 */
bool Clock::is_leap_year(uint32_t a_year) {
    
    if ((a_year % 4 == 0) && ((a_year % 100 > 0 || a_year % 400 == 0))) return true;
    return false;
}
