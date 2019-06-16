## License
Taken from https://github.com/adafruit/RTClib
and modified for LPC1768 by Neal Horman July 2012.

Modified by André Sá de Mello in June 2019 to extend functionality.

*Released to the Public Domain.*

# Example usage

```C
#include "mbed.h"
#include "DS1307.h"

int main()
{
    DateTime now(__DATE__, __TIME__);
    printf("Compiled at: %s\n", now.isoformat());
    DS1307 rtc(p28, p27);
    rtc.set_wave_enabled(false);
    rtc.set_running(false);
    rtc.set_time(now);
    rtc.dump_data();
    rtc.set_running(true);
    while (true) {
        now = rtc.now();
        printf("TIME: %s\n", now.isoformat());
        wait(1);
    }
}
```
