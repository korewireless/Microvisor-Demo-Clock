/*
 * Microvisor Clock Demo -- Clock class
 *
 * @author      Tony Smith
 * @copyright   2024, KORE Wireless
 * @licence     MIT
 *
 */
#include "main.h"


/**
 * @brief Basic driver for HT16K33-based display.
 *
 * @param inPrefs:   Reference to the app's preferences data.
 * @param inDisplay: Reference to the app's display instance.
 * @param gotPrefs:  Have we loaded clock settings yet?
 */
Clock::Clock(const Prefs& inPrefs, const HT16K33_Segment& inDisplay, const bool gotPrefs)
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
    static struct tm timeStore;
    uint64_t usec = 0;
    uint32_t now_year;
    uint32_t now_month;
    uint32_t now_day;
    uint32_t now_hour;
    uint32_t now_mins;
    uint32_t now_secs;

    if (mvGetWallTime(&usec) == MV_STATUS_OKAY) {
        // Get the time in seconds
        const time_t secs = (time_t)usec / 1000000;

        // Write time string as "2022-05-10 13:30:58"
        if (strftime(timestamp, 64, "%F %T", gmtime_r(&secs, &timeStore)) > 0) {
            // Convert the hour, minute and second values back
            int result = sscanf(timestamp, "%lu-%lu-%lu %lu:%lu:%lu", &now_year, &now_month, &now_day, &now_hour, &now_mins, &now_secs);
            if (result == 6) {
                year = now_year;
                month = now_month;
                day = now_day;
                hour = now_hour;
                minutes = now_mins;
                seconds = now_secs;
                return true;
            }
        }
    }

    return false;
}


/**
 * @brief Loop the clock display and update routine.
 */
[[noreturn]] void Clock::loop(void) {

    constexpr uint32_t CONFIG_ACQUIRE_PERIOD_MINS = 4;

    // Update brightness
    display.setBrightness(prefs.brightness);

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
        // The decimal point by the first digit is used to indicate
        // connection status (lit if the clock is disconnected)
        uint32_t netState = Config::Network::getState();
        auto decimal = (uint8_t)(bcd(displayHour) & 0xFF);
        display.setNumber(decimal & 0x0F, 1, false);
        if (!prefs.mode && displayHour < 10) {
            // Show a blank space in the first digit
            display.setGlyph(0, 0, (netState != (uint32_t)NET_STATE::ONLINE));
        } else {
            display.setNumber((decimal >> 4) & 0x0F, 0, (netState != (uint32_t)NET_STATE::ONLINE));
        }

        // Display the minute
        // The decimal point by the last digit is used to indicate AM/PM,
        // but only for the 12-hour clock mode (mode == False)
        decimal = (uint8_t)(bcd(minutes) & 0xFF);
        display.setNumber((decimal >> 4) & 0x0F, 2, false);
        display.setNumber(decimal & 0x0F, 3, (prefs.mode ? false : isPM));

        // Set the colon and present the display
        if (prefs.colon) {
            // Show the colon, either solid or flash
            if (prefs.flash) {
                // Show the colon every two seconds, for a second
                display.setColon(seconds % 2 == 0);
                // Flash the NDB LED in sync
                if (prefs.led) HAL_GPIO_WritePin(LED_GPIO_BANK, LED_GPIO_PIN, seconds % 2 == 0 ? GPIO_PIN_SET : GPIO_PIN_RESET);
            } else {
                // Illuminate the colon permanently
                display.setColon(prefs.colon);
            }
        }

        // Tell the display driver to update the LED
        display.draw();

        // Reload prefs if we haven't done so yet
        if (netState == (uint32_t)NET_STATE::ONLINE && !receivedPrefs && minutes != 0 && minutes % CONFIG_ACQUIRE_PERIOD_MINS == 0) {
            receivedPrefs = Config::getPrefs(prefs);
            if (receivedPrefs) {
                // Update brightness
                display.setBrightness(prefs.brightness);
                server_log("Clock settings retrieved");
            } else {
                server_error("Clock settings not retrieved (%u)", minutes);
            }
        }

        // Reset the prefs get flag periodically
        if (minutes != 0 && minutes % 15 == 0) {
            receivedPrefs = false;
        }
    }
}


/**
 * @brief Convert an integer to a binary coded decimal representation.
 *
 * @param rawInt: The source value.
 *
 * @returns The BCD encoding.
 */
uint32_t Clock::bcd(uint32_t rawInt) const {

    uint32_t result = 0;
    uint32_t shift = 0;

    while (rawInt) {
        result += ((rawInt % 10) << shift);
        rawInt = rawInt / 10;
        shift += 4;
    }

    return (result & 0xFF);
}


/**
 * @brief Are we in UK daylight savings time?
 *
 * @returns `true` if DST is active, otherwise `false`.
 */
bool Clock::isBST(void) const {

    return bstCheck();
}


/**
 * @brief Are we in UK daylight savings time?
 *
 * @returns `true` if DST is active, otherwise `false`.
 */
bool Clock::bstCheck(void) const {

    if (month > 3 && month < 10) return true;

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
uint32_t Clock::dayOfWeek(int aDay, int aMonth, int aYear) const {

    aMonth -= 2;
    if (aMonth < 1) aMonth += 12;
    const uint32_t century = aYear / 100;
    aYear -= (century * 100);
    aYear -= (month > 10 ? 1 : 0);

    int dow = aDay + ((13 * aMonth - 1) / 5) + aYear + (int)(year / 4) + (int)(century / 4) - (2 * century);
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
bool Clock::isLeapYear(uint32_t aYear) const {

    if ((aYear % 4 == 0) && (aYear % 100 > 0 || aYear % 400 == 0)) return true;
    return false;
}
