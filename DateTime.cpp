/*
 * Taken from https://github.com/adafruit/RTClib
 * and modified for LPC1768 by Neal Horman July 2012.
 *
 * Modified by André Sá de Mello in June 2019 to extend functionality.
 *
 * Released to the Public Domain.
 */

#include "DateTime.h"

#define SECONDS_PER_DAY 86400L
#define SECONDS_FROM_1970_TO_2000 946684800

////////////////////////////////////////////////////////////////////////////////
// utility code, some of this could be exposed in the DateTime API if needed

static const uint8_t daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

// number of days since 2000/01/01, valid for 2001..2099
static inline uint16_t date2days(uint16_t y, uint8_t m, uint8_t d)
{
    if (y >= 2000) y -= 2000;

    uint16_t days = d;
    for (uint8_t i = 1; i < m; ++i) days += daysInMonth[i - 1];

    if (m > 2 && y % 4 == 0) ++days;

    return days + 365 * y + (y + 3) / 4 - 1;
}

static inline uint64_t time2long(uint16_t d, uint8_t h, uint8_t m, uint8_t s)
{
    return ((d * 24L + h) * 60 + m) * 60 + s;
}

////////////////////////////////////////////////////////////////////////////////
// DateTime implementation - ignores time zones and DST changes
// NOTE: also ignores leap seconds, see http://en.wikipedia.org/wiki/Leap_second

DateTime::DateTime(uint64_t t)
{
    t -= SECONDS_FROM_1970_TO_2000;    // bring to 2000 timestamp from 1970

    ss = t % 60;
    t /= 60;
    mm = t % 60;
    t /= 60;
    hh = t % 24;

    uint16_t days = t / 24;
    uint8_t leap;

    for (y_off = 0; ; ++y_off) {
        leap = y_off % 4 == 0;
        if (days < 365 + leap) break;
        days -= 365 + leap;
    }

    for (m = 1; ; ++m) {
        uint8_t daysPerMonth = daysInMonth[m - 1];
        if (leap && m == 2) ++daysPerMonth;
        if (days < daysPerMonth) break;
        days -= daysPerMonth;
    }
    d = days + 1;
}

DateTime::DateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec)
{
    if (year >= 2000) year -= 2000;
    y_off = year;
    m = month;
    d = day;
    hh = hour;
    mm = min;
    ss = sec;
}

static uint8_t conv2d(const char* p)
{
    uint8_t v = 0;
    if ('0' <= *p && *p <= '9') v = *p - '0';
    return 10 * v + *++p - '0';
}

// A convenient constructor for using "the compiler's time":
//   DateTime now (__DATE__, __TIME__);
// NOTE: using PSTR would further reduce the RAM footprint
DateTime::DateTime(const char* date, const char* time)
{
    // sample input: date = "Dec 26 2009", time = "12:34:56"
    y_off = conv2d(date + 9);
    // Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec
    switch (date[0])
    {
        case 'J': m = date[1] == 'a' ? 1 : m = date[2] == 'n' ? 6 : 7; break;
        case 'F': m = 2; break;
        case 'A': m = date[2] == 'r' ? 4 : 8; break;
        case 'M': m = date[2] == 'r' ? 3 : 5; break;
        case 'S': m = 9; break;
        case 'O': m = 10; break;
        case 'N': m = 11; break;
        case 'D': m = 12; break;
    }
    d = conv2d(date + 4);
    hh = conv2d(time);
    mm = conv2d(time + 3);
    ss = conv2d(time + 6);
}

uint8_t DateTime::day_of_week() const
{
    uint16_t day = date2days(y_off, m, d);
    return (day + 6) % 7; // Jan 1, 2000 is a Saturday, i.e. returns 6
}

uint64_t DateTime::unixtime() const
{
    uint16_t days = date2days(y_off, m, d);
    return time2long(days, hh, mm, ss) + SECONDS_FROM_1970_TO_2000;
}

const char *DateTime::isoformat() const
{
    static char buffer[20];
    sprintf(buffer, "%04d-%02d-%02dT%02d:%02d:%02d", year(), month(), day(), hour(), minute(), second());
    return buffer;
}
