/*
 * Microvisor Clock Demo -- Clock class
 *
 * @version     0.1.0
 * @author      Tony Smith
 * @copyright   2023, KORE Wireless
 * @licence     MIT
 *
 */
#include "main.h"


using std::vector;
using std::string;
using std::stringstream;


/**
 * @brief Basic driver for HT16K33-based display.
 *
 * @param inPrefs:   Reference to the app's preferences data.
 * @param inDisplay: Reference to the app's display instance.
 * @param gotPrefs:  Have we loaded clock settings yet?
 */
Clock::Clock(Prefs& inPrefs, HT16K33_Segment& inDisplay, bool gotPrefs)
    :prefs(inPrefs),
    display(inDisplay),
    receivedPrefs(gotPrefs)
{}


/**
 * @brief Set the current time from the STM32U5 RTC
 *        via Microvisor.
 *
 * @returns `true` if the time was set, otherwise `false`.
 */
bool Clock::setTimeFromRTC(void) {

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

    const uint32_t configAcquirePeriodMinutes = 4;

    while (true) {
        // Check the time
        setTimeFromRTC();

        // Update display hour for DST, if allowed
        uint32_t displayHour = hour;
        if (prefs.bst && isBST()) displayHour = (displayHour + 1) % 24;
        bool isPM = (displayHour > 11);

        // Calculate and set the hours digits
        if (!prefs.mode) {
            if (isPM) displayHour -= 12;
            if (displayHour == 0) displayHour = 12;
        }

        // Display the hour
        // The decimal point by the first digit is used to indicate connection status
        // (lit if the clock is disconnected)
        uint32_t decimal = bcd(displayHour);
        display.setNumber(decimal & 0x0F, 1, false);
        if (!prefs.mode && displayHour < 10) {
            display.setGlyph(0, 0, false);
        } else {
            display.setNumber(decimal >> 4, 0, false);
        }

        // Display the minute
        // The decimal point by the last digit is used to indicate AM/PM,
        // but only for the 12-hour clock mode (mode == False)
        decimal = bcd(minutes);
        display.setNumber(decimal >> 4, 2, false);
        display.setNumber(decimal & 0x0F, 3, (prefs.mode ? false : isPM));

        // Set the colon and present the display
        if (prefs.colon) {
            if (prefs.flash) {
                display.setColon(seconds % 2 == 0);
                HAL_GPIO_WritePin(LED_GPIO_BANK, LED_GPIO_PIN, seconds % 2 == 0 ? GPIO_PIN_SET : GPIO_PIN_RESET);
            } else {
                display.setColon(prefs.colon);
            }
        }

        display.draw();

        // Reload prefs if we haven't done so yet
        if (!receivedPrefs && minutes != 0 && minutes % configAcquirePeriodMinutes == 0) {
            server_log("***");
            receivedPrefs = Config::getPrefs(prefs);
            if (receivedPrefs) server_log("Clock settings retrieved");
        }
    }
}


/**
 * @brief Convert an integer to a bimary coded decimal representation.
 *
 * @param rawInt: The source value.
 *
 * @returns The BCD encoding.
 */
uint32_t Clock::bcd(uint32_t rawInt) {

    for (uint32_t i = 0 ; i < 8 ; ++i) {
        rawInt <<= 1;
        if (i == 7) break;
        if ((rawInt & 0x0F00) > 0x04FF) rawInt += 0x0300;
        if ((rawInt & 0xF000) > 0x4FFF) rawInt += 0x3000;
    }

    return (rawInt >> 8) & 0xFF;
}


/**
 * @brief Are we in UK daylight savings time?
 *
 * @returns `true` if DST is active, otherwise `false`.
 */
bool Clock::isBST(void) {

    return bstCheck();
}


/**
 * @brief Are we in UK daylight savings time?
 *
 * @returns `true` if DST is active, otherwise `false`.
 */
bool Clock::bstCheck(void) {

    if (month > 3 and month < 10) return true;

    if (month == 3) {
        // BST starts on the last Sunday of March
        for (uint32_t i = 31 ; i > 24 ; --i) {
            if ((dayOfWeek(i, 3, (int)year) == 0) && (day >= i)) return true;
        }
    }

    if (month == 10) {
        // BST ends on the last Sunday of October
        for (uint32_t i = 31 ; i > 24 ; --i) {
            if ((dayOfWeek(i, 10, (int)year) == 0) && (day < i)) return true;
        }
    }

    return false;
}


/**
 * @brief Determine the day of the week for a given day, month and year, using
 *        Zeller's Rule (see http://mathforum.org/dr.math/faq/faq.calendar.html).
 *
 * @param aDay:   The day of the month (1-31).
 * @param aMonth: A month (1-12).
 * @param aYear:  A year (including the century, ie. '2019' not '19').
 *
 * @returns The day of the week: 0 (Monday) to 6 (Sunday).
 */
uint32_t Clock::dayOfWeek(int aDay, int aMonth, int aYear) {

    aMonth -= 2;
    if (aMonth < 1) aMonth += 12;
    const uint32_t century = (int)(aYear / 100);
    aYear -= (century * 100);
    aYear -= (month > 10 ? 1 : 0);

    int dow = aDay + (int)((13 * aMonth - 1) / 5) + aYear + (int)(year / 4) + (int)(century / 4) - (2 * century);
    dow %= 7;
    if (dow < 0) dow += 7;
    return (uint32_t)dow;
}


/**
 * @brief Is the specified year a leap year?
 *
 * @param aYear: A year (including the century, ie. '2019' not '19').
 *
 * @returns `true` if the year is a leap year, otherwise `false`.
 */
bool Clock::isLeapYear(uint32_t aYear) {

    if ((aYear % 4 == 0) && ((aYear % 100 > 0 || aYear % 400 == 0))) return true;
    return false;
}
