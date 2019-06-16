/*
 * Taken from https://github.com/adafruit/RTClib
 * and modified for LPC1768 by Neal Horman July 2012.
 *
 * Rewritten by André Sá de Mello in June 2019 to extend functionality.
 *
 * Released to the Public Domain.
 */

#ifndef _DS1307_H_
#define _DS1307_H_

#include "mbed.h"
#include "DateTime.h"

// RTC based on the DS1307 chip connected via I2C
class DS1307
{
public:
    enum rate {F_1Hz, F_4096_kHz, F_8192_kHz, F_32768_kHz};

    DS1307(PinName sdl, PinName sda) : i2c_(sdl, sda) { }
    bool set_time(const DateTime &);
    bool is_running();
    DateTime now();
    bool save_ram();
    bool load_ram();
    bool reset_ram();
    void dump_data();
    bool set_running(bool);
    bool set_wave_rate(rate);
    rate get_wave_rate();
    bool set_wave_enabled(bool);
    bool is_wave_enabled();
    bool set_wave_default_value(bool);
    bool get_wave_default_value();

    uint8_t &operator[](const uint8_t i) { return ram_[i % sizeof(ram_)]; };
protected:
    I2C i2c_;
    uint8_t ram_[56];
    bool read(const uint8_t address, uint8_t *buffer, const uint8_t len);
    bool write(const uint8_t address, const uint8_t *buffer, const uint8_t len);
};

#endif
