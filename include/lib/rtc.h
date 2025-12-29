#ifndef LIB_RTC_H
#define LIB_RTC_H

#include <stdint.h>

typedef struct {
    uint8_t sec;
    uint8_t min;
    uint8_t hour;
    uint8_t day;
    uint8_t month;
    uint16_t year;
} rtc_time_t;

void rtc_time_sys(rtc_time_t* out);
int timezone_sys(void);

#endif
