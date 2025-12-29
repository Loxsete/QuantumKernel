#ifndef RTC_H
#define RTC_H

#include <stdint.h>

typedef struct {
    uint8_t sec;
    uint8_t min;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint16_t year;
} rtc_time_t;

void rtc_init(void);
rtc_time_t rtc_read(void);
rtc_time_t rtc_get_local_time(void);
void load_timezone(void);
void rtc_time_sys(rtc_time_t* out);
int timezone_sys(void);

#endif
