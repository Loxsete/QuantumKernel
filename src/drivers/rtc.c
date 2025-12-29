#include "drivers/rtc.h"
#include "drivers/io.h"
#include "fs/fat32.h"
#include "lib/katoi.h"

#define RTC_PORT 0x70
#define RTC_DATA 0x71

int timezone_offset = 0;

static uint8_t rtc_read_reg(uint8_t reg) {
    outb(RTC_PORT, reg);
    return inb(RTC_DATA);
}

static uint8_t bcd_to_dec(uint8_t val) {
    return ((val >> 4) * 10) + (val & 0x0F);
}

int get_timezone_offset(int month, int day) {
    if (month > 3 && month < 10) return 3;
    if (month == 3 && day >= 25) return 3;
    if (month == 10 && day <= 31) return 3;
    return 2;
}



rtc_time_t rtc_read(void) {
    rtc_time_t t;
    t.sec   = bcd_to_dec(rtc_read_reg(0x00));
    t.min   = bcd_to_dec(rtc_read_reg(0x02));
    t.hour  = bcd_to_dec(rtc_read_reg(0x04));
    t.day   = bcd_to_dec(rtc_read_reg(0x07));
    t.month = bcd_to_dec(rtc_read_reg(0x08));
    t.year  = bcd_to_dec(rtc_read_reg(0x09)) + 2000;
    return t;
}


void load_timezone(void) {
    fat32_file_t file;
    char buf[16];
    
    if (fat32_open(&file, "tz.txt", FAT32_O_RDONLY) != 0)
        return; 
    
    int read_bytes = fat32_read(&file, buf, sizeof(buf)-1);
    buf[read_bytes] = 0;
    
    if (buf[0] == 'U' && buf[1] == 'T' && buf[2] == 'C') {
        timezone_offset = katoi(buf + 3);
    }
    
    fat32_close(&file);
}

rtc_time_t rtc_get_local_time(void) {
    rtc_time_t t = rtc_read();
    int offset = get_timezone_offset(t.month, t.day);
    t.hour = (t.hour + offset) % 24;
    return t;
}

void rtc_time_sys(rtc_time_t* out) {
    if (!out) return;
    *out = rtc_get_local_time();
}

int timezone_sys(void) {
    return timezone_offset;
}
