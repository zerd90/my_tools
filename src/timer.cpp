#include "timer.h"
#include <time.h>

#if defined(WIN32) || defined(_WIN32)
    #include <windows.h>
int gettimeofday(struct timeval *tp, void *tzp)
{
    (void)tzp;
    time_t     clock;
    struct tm  tm;
    SYSTEMTIME wtm;
    GetLocalTime(&wtm);
    tm.tm_year  = wtm.wYear - 1900;
    tm.tm_mon   = wtm.wMonth - 1;
    tm.tm_mday  = wtm.wDay;
    tm.tm_hour  = wtm.wHour;
    tm.tm_min   = wtm.wMinute;
    tm.tm_sec   = wtm.wSecond;
    tm.tm_isdst = -1;
    clock       = mktime(&tm);
    tp->tv_sec  = (long)clock;
    tp->tv_usec = wtm.wMilliseconds * 1000;
    return (0);
}
#else
    #include <sys/time.h>
#endif
static uint64_t gAppStartTimeUs = gettime_us(false);
uint64_t gettime_ms(bool fromAppStart)
{
    struct timeval tim;
    gettimeofday(&tim, NULL);
    uint64_t currTime = (uint64_t)tim.tv_sec * 1000 + tim.tv_usec / 1000;
    if (fromAppStart)
        return currTime - gAppStartTimeUs / 1000;
    else
        return currTime;
}

uint64_t gettime_us(bool fromAppStart)
{
    struct timeval tim;
    gettimeofday(&tim, NULL);
    uint64_t currTime = (uint64_t)tim.tv_sec * 1000000 + tim.tv_usec;
    if (fromAppStart)
        return currTime - gAppStartTimeUs;
    else
        return currTime;
}


int is_leap_year(int year)
{
    if (year % 100 == 0)
    {
        if (year % 400 == 0)
            return 1;
        else
            return 0;
    }

    if (year % 4 == 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int get_day_of_year(int year)
{
    if (is_leap_year(year))
        return 366;
    else
        return 365;
}

int get_days_of_month(int year, int month)
{
    static const int days_of_month[2][12] = {
        {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
        {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
    };

    return days_of_month[is_leap_year(year)][month - 1];
}

int get_week(int y, int m, int d)
{
    int week = 0;
    if (m == 1 || m == 2)
    {
        m += 12;
        y--;
    }
    week = (d + 2 * m + 3 * (m + 1) / 5 + y + y / 4 - y / 100 + y / 400) % 7;
    return week;
}

split_time_t get_split_time(uint64_t time_tl, unsigned int start_year, int utc_region)
{
    split_time_t res;
    time_tl += utc_region * 3600;
    uint64_t day_tl   = time_tl / (3600 * 24);
    int      time_day = time_tl % (3600 * 24);

    unsigned int year  = start_year;
    unsigned int month = 1;
    unsigned int day   = 1;
    unsigned int hour, minute, second;

    while (1)
    {
        unsigned int cur_days = get_day_of_year(year);
        if (day_tl >= cur_days)
        {
            year++;
            day_tl -= cur_days;
        }
        else
        {
            break;
        }
    }

    res.yday = (unsigned int)day_tl;

    while (1)
    {
        unsigned int cur_days = get_days_of_month(year, month);
        if (day_tl >= cur_days)
        {
            month++;
            day_tl -= cur_days;
        }
        else
        {
            break;
        }
    }

    day += (unsigned int)day_tl;

    hour     = time_day / 3600;
    time_day = time_day % 3600;
    minute   = time_day / 60;
    second   = time_day % 60;

    res.year = year;
    res.mon  = month;
    res.mday = day;
    res.hour = hour;
    res.min  = minute;
    res.sec  = second;

    res.wday = get_week(year, month, day);

    return res;
}

uint64_t get_total_time(split_time_t s_time, unsigned int start_year)
{
    if (s_time.year < start_year)
        return 0;

    uint64_t time_tl = 0;
    for (unsigned int i = start_year; i < s_time.year; i++)
    {
        time_tl += get_day_of_year(i);
    }

    for (unsigned int i = 0; i < s_time.mon; i++)
    {
        time_tl += get_days_of_month(s_time.year, i);
    }
    time_tl += (s_time.mday - 1);

    time_tl *= 24 * 3600;

    time_tl += s_time.hour * 3600 + s_time.min * 60 + s_time.sec;

    return time_tl;
}

std::ostream &operator<<(std::ostream &out, const split_time_t &s_time)
{
    return out << s_time.to_string();
}

const char *g_week[7] = {"MON", "TUE", "WED", "THU", "FRI", "SAT", "SUN"};
