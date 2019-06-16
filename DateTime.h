/*
 * Taken from https://github.com/adafruit/RTClib
 * and modified for LPC1768 by Neal Horman July 2012.
 *
 * Modified by André Sá de Mello in June 2019 to extend functionality.
 *
 * Released to the Public Domain.
 */

#ifndef _DATETIME_H_
#define _DATETIME_H_

#include "mbed.h"

// Simple general-purpose date/time class (no TZ / DST / leap second handling!)
class DateTime
{
public:
    DateTime(uint64_t t = 0);
    DateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour = 0, uint8_t min = 0, uint8_t sec = 0);
    DateTime(const char* date, const char* time);
    uint16_t year() const       { return 2000 + y_off; }
    uint8_t month() const       { return m; }
    uint8_t day() const         { return d; }
    uint8_t hour() const        { return hh; }
    uint8_t minute() const      { return mm; }
    uint8_t second() const      { return ss; }
    uint8_t day_of_week() const;
    const char *isoformat() const;

    // 64-bit times as seconds since 1/1/1970
    uint64_t unixtime() const;

protected:
    uint8_t y_off, m, d, hh, mm, ss;
};

#endif
