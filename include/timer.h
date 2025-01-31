#ifndef Z_TIMER_H
#define Z_TIMER_H

#include <stdint.h>
#include <iostream>

using ullong = unsigned long long;

extern const char *g_week[7];
struct split_time_t
{
    unsigned int sec;
    unsigned int min;
    unsigned int hour;
    unsigned int mday;
    unsigned int mon;
    unsigned int year;
    unsigned int wday;
    unsigned int yday;
    std::string  to_string() const
    {
        char pstr[40];
        snprintf(pstr, 40, "%u-%u-%u %02u:%02u:%02u %s", year, mon, mday, hour, min, sec, g_week[wday]);
        return std::string(pstr);
    }
};

uint64_t     gettime_ms();
uint64_t     gettime_us();
split_time_t get_split_time(uint64_t time_tl, unsigned int start_year = 1970, int utc_region = 8);
uint64_t     get_total_time(split_time_t s_time, unsigned int start_year = 1970);

std::ostream &operator<<(std::ostream &out, const split_time_t &s_time);

#endif