/*
 * Taken from https://github.com/adafruit/RTClib
 * and modified for LPC1768 by Neal Horman July 2012.
 *
 * Rewritten by André Sá de Mello in June 2019 to extend functionality.
 *
 * Released to the Public Domain.
 */

#include "mbed.h"
#include "DS1307.h"

#define DS1307_ADDRESS 0xD0

////////////////////////////////////////////////////////////////////////////////
// DS1307 implementation

static inline uint8_t bcd2bin (uint8_t x) { return x - 6 * (x >> 4); }
static inline uint8_t bin2bcd (uint8_t x) { return x + 6 * (x / 10); }

static inline const char *rate2str(uint8_t rate_code)
{
    switch (rate_code) {
        case 0x00:
            return "1Hz";
        case 0x01:
            return "4.096kHz";
        case 0x02:
            return "8.192kHz";
        case 0x03:
            return "32.768kHz";
        default:
            return nullptr;
    }
}

bool DS1307::load_ram()
{
    return read(0x08, ram_, sizeof(ram_));
}

bool DS1307::save_ram()
{
    return write(0x08, ram_, sizeof(ram_));
}

bool DS1307::reset_ram()
{
    memset(ram_, 0, sizeof(ram_));
    return write(0x08, ram_, sizeof(ram_));
}

void DS1307::dump_data()
{
    uint8_t buffer[64], i;
    read(0x00, buffer, sizeof(buffer));
    printf("===== CLOCK DUMP ======\n");
    for (i = 0; i < 6; ++i) {
        printf("%02x ", buffer[i]);
    }
    printf(
        "%02x\n"
        "CH: %d, 24/12: %d\n"
        "==== CONTROL DUMP =====\n"
        "%02x\nOUT: %d, SQWE: %d, RS: %d%d\n"
        "RATE: %s\n"
        "====== RAM DUMP =======\n",
        buffer[6],
        (bool) (buffer[0] & 0x80),
        (bool) (buffer[2] & 0x40),
        buffer[7],
        (bool) (buffer[7] & 0x80),
        (bool) (buffer[7] & 0x10),
        (bool) (buffer[7] & 0x02),
        (bool) (buffer[7] & 0x01),
        rate2str(buffer[7] & 0x03)
    );
    for (i = 0; i < sizeof(ram_); ++i) {
        if ((i + 1) % 8 == 0) {
            printf("%02x\n", ram_[i]);
        } else {
            printf("%02x ", ram_[i]);
        }
    }
}

bool DS1307::read(const uint8_t address, uint8_t *buffer, const uint8_t len)
{
    if (i2c_.write(DS1307_ADDRESS, (const char*) &address, 1, true)) {
        return false;
    }
    if (i2c_.read(DS1307_ADDRESS, (char *) buffer, len)) {
        return false;
    }
    return true;
}

bool DS1307::write(
    const uint8_t address, const uint8_t *buffer, const uint8_t len)
{
    if (len > 64) return false;
    uint8_t local_buffer[65];
    local_buffer[0] = address;
    memcpy(&local_buffer[1], buffer, len);
    return !i2c_.write(DS1307_ADDRESS, (const char*) local_buffer, len + 1);
}

bool DS1307::set_running(bool enable)
{
    uint8_t byte;
    read(0x00, &byte, 1);
    if (enable) {
        byte &= 0x7F; // set bit 7 [CH] to 0
    } else {
        byte |= 0x80; // set bit 7 [CH] to 1
    }
    return write(0x00, &byte, 1);
}

bool DS1307::is_running()
{
    uint8_t byte;
    read(0x00, &byte, 1);
    return !(byte & 0x80);
}

bool DS1307::set_time(const DateTime& dt)
{
    uint8_t buffer[7];
    read(0x00, buffer, sizeof(buffer));
    // preserve original CH bit
    buffer[0] = bin2bcd(dt.second()) | (buffer[0] & 0x80);
    buffer[1] = bin2bcd(dt.minute());
    if (buffer[2] & 0x40) {
        // 12h mode
        if (dt.hour() > 12) {
            buffer[2] = 0x60 | bin2bcd(dt.hour() - 12);
        } else {
            buffer[2] = 0x40 | bin2bcd(dt.hour());
        }
    } else {
        // 24h mode
        buffer[2] = bin2bcd(dt.hour());
    }
    buffer[3] = bin2bcd(dt.day_of_week());
    buffer[4] = bin2bcd(dt.day());
    buffer[5] = bin2bcd(dt.month());
    buffer[6] = bin2bcd(dt.year() - 2000);
    return write(0x00, buffer, sizeof(buffer));
}

DateTime DS1307::now()
{
    uint8_t buffer[7];
    read(0x00, buffer, sizeof(buffer));
    uint8_t hour;
    if (buffer[2] & 0x40) {
        // bit 6 set, 12h mode
        hour = bcd2bin(buffer[2] & 0x1F); // bits [0..4]
        if (buffer[2] & 0x20) {
            hour += 12; // bit 5 set, add 12h
        }
    } else {
        // bit 6 unset, 24h mode
        hour = bcd2bin(buffer[2] & 0x3F); // bits [0..5]
    }
    return DateTime(
        bcd2bin(buffer[6]) + 2000, // RTC year stored as offset from 2000
        bcd2bin(buffer[5]),
        bcd2bin(buffer[4]),
        hour,
        bcd2bin(buffer[1]),
        bcd2bin(buffer[0] & 0x7F) // bits [0..6]
    );
}

bool DS1307::set_wave_rate(DS1307::rate rate)
{
    uint8_t byte;
    if (!read(0x07, &byte, 1)) return false;
    byte = (byte & 0xFC) | rate;
    return write(0x07, &byte, 1);
}

DS1307::rate DS1307::get_wave_rate()
{
    uint8_t byte;
    read(0x07, &byte, 1);
    return DS1307::rate(byte & 0x03);
}

bool DS1307::set_wave_enabled(bool enabled)
{
    uint8_t byte;
    if (!read(0x07, &byte, 1)) return false;
    byte = (byte & 0xEF) | (enabled << 4);
    return write(0x07, &byte, 1);
}

bool DS1307::is_wave_enabled()
{
    uint8_t byte;
    read(0x07, &byte, 1);
    return byte & 0x10;
}

bool DS1307::set_wave_default_value(bool high)
{
    uint8_t byte;
    if (!read(0x07, &byte, 1)) return false;
    byte = (byte & 0x7F) | (high << 7);
    return write(0x07, &byte, 1);
}

bool DS1307::get_wave_default_value()
{
    uint8_t byte;
    read(0x07, &byte, 1);
    return byte & 0x80;
}
