/**
 * date_to_timestamp.c
 *
 * 从 HTTP 响应头 "Date: Wed, 26 Aug 2026 07:45:03 GMT" 中提取时间。
 *
 * 提供两类功能：
 *   1) parse_http_date_to_timestamp()  -> Unix 时间戳（自 1970-01-01 00:00:00 UTC 的秒数）
 *   2) parse_http_date_to_rtc()        -> 直接填充 STM32 HAL 的 RTC_DateTypeDef / RTC_TimeTypeDef，
 *                                        可传给 HAL_RTC_SetDate() / HAL_RTC_SetTime()
 *
 * 不依赖 timegm()/mktime()，可在 STM32/裸机环境下直接使用。
 * HAL 相关部分在 USE_HAL_DRIVER 定义时才会编译（CubeMX 工程默认已定义）。
 */

#include <stdio.h>
#include <string.h>

#if defined(USE_HAL_DRIVER)
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_rtc.h"
#endif

/* ---------------------------------------------------------------
 * 解析结果结构：把字段从日期字符串中一次性取出，供上层复用
 * --------------------------------------------------------------- */
typedef struct
{
    int weekday;   /* 1=Mon ... 7=Sun */
    int day;       /* 1-31 */
    int month;     /* 1-12 */
    int year;      /* 完整四位年份，如 2026 */
    int hour;
    int minute;
    int second;
} http_date_fields_t;

static const char *s_mon_name[12] = {
    "Jan","Feb","Mar","Apr","May","Jun",
    "Jul","Aug","Sep","Oct","Nov","Dec"
};

static const char *s_wday_name[7] = {
    "Mon","Tue","Wed","Thu","Fri","Sat","Sun"
};

/* ---------------------------------------------------------------
 * 在字符串中定位 HTTP 日期 "Wdy, DD Mon YYYY HH:MM:SS" 的起始位置。
 * 返回指向星期名的指针；找不到返回 NULL。
 * 兼容传入完整 HTTP 响应头（日期位于 "Date: " 之后）或单独日期字符串。
 * --------------------------------------------------------------- */
static const char *find_http_date(const char *s)
{
    if (s == NULL)
        return NULL;

    for (const char *p = s; *p != '\0'; p++)
    {
        /* 判断当前位置是否为星期名 */
        int wk = -1;
        for (int i = 0; i < 7; i++)
        {
            if (strncmp(p, s_wday_name[i], 3) == 0)
            {
                wk = i;
                break;
            }
        }
        if (wk < 0)
            continue;

        /* 星期名后必须是 ", "，接着才是日期 */
        if (p[3] == ',' && p[4] == ' ')
            return p;
    }
    return NULL;
}

/* ---------------------------------------------------------------
 * 解析 "Wdy, DD Mon YYYY HH:MM:SS GMT" 格式的 HTTP 日期
 * 支持直接传入日期串，或包含该日期的完整 HTTP 响应/头部。
 * 成功返回 0，失败返回 -1
 * --------------------------------------------------------------- */
static int parse_http_date(const char *date, http_date_fields_t *f)
{
    if (date == NULL || f == NULL)
        return -1;

    /* 定位真正的日期起始位置（跳过 "HTTP/1.1 200 OK\r\n..." 等前置内容） */
    const char *start = find_http_date(date);
    if (start == NULL)
        start = date;  /* 退回到从头解析 */

    char wday[4], mon[4];
    if (sscanf(start, "%3s, %d %3s %d %d:%d:%d",
               wday, &f->day, mon, &f->year,
               &f->hour, &f->minute, &f->second) != 7)
        return -1;

    /* 匹配星期 */
    f->weekday = -1;
    for (int i = 0; i < 7; i++)
    {
        if (strncmp(wday, s_wday_name[i], 3) == 0)
        {
            f->weekday = i + 1;
            break;
        }
    }
    if (f->weekday < 0)
        return -1;

    /* 匹配月份 */
    f->month = -1;
    for (int i = 0; i < 12; i++)
    {
        if (strncmp(mon, s_mon_name[i], 3) == 0)
        {
            f->month = i + 1;
            break;
        }
    }
    if (f->month < 0)
        return -1;

    /* 简单范围校验 */
    if (f->day < 1 || f->day > 31 ||
        f->hour > 23 || f->minute > 59 || f->second > 60)  /* 60 允许闰秒 */
        return -1;

    return 0;
}

/* ---------------------------------------------------------------
 * 日期 -> 自 epoch 起的天数（Howard Hinnant civil_from_days 逆算法）
 * --------------------------------------------------------------- */
static long days_from_civil(int y, unsigned m, unsigned d)
{
    y -= m <= 2;
    const int era = (y >= 0 ? y : y - 399) / 400;
    const unsigned yoe = (unsigned)(y - era * 400);        /* [0, 399] */
    const unsigned doy = (153u * (m + (m > 2 ? -3 : 9)) + 2u) / 5u + d - 1u;
    const unsigned doe = yoe * 365u + yoe / 4u - yoe / 100u + doy;  /* [0, 146096] */
    return era * 146097L + (long)doe - 719468L;
}

/* ---------------------------------------------------------------
 * 天数 -> 公历日期（Howard Hinnant civil_from_days 正算法）
 * --------------------------------------------------------------- */
static void civil_from_days(long z, int *y, unsigned *m, unsigned *d)
{
    z += 719468L;
    const int era = (z >= 0 ? z : z - 146096) / 146097;
    const unsigned doe = (unsigned)(z - (long)era * 146097);          /* [0, 146096] */
    const unsigned yoe = (doe - doe/1460 + doe/36524 - doe/146096) / 365;  /* [0, 399] */
    const int Y = (int)yoe + era * 400;
    const unsigned doy = doe - (365u*yoe + yoe/4u - yoe/100u);        /* [0, 365] */
    const unsigned mp = (5u*doy + 2u) / 153u;                         /* [0, 11] */
    const unsigned d2 = doy - (153u*mp + 2u) / 5u + 1u;               /* [1, 31] */
    const unsigned m2 = (mp < 10u) ? mp + 3u : mp - 9u;               /* [1, 12] */

    *y = (int)Y + ((m2 <= 2u) ? 1 : 0);
    *m = m2;
    *d = d2;
}

/* ---------------------------------------------------------------
 * 把 Unix 时间戳转换为公历日期 + 时分秒 + 星期（HAL 星期：1=Mon..7=Sun）
 * --------------------------------------------------------------- */
static void timestamp_to_rtc(long ts, RTC_DateTypeDef *rtc_date, RTC_TimeTypeDef *rtc_time)
{
    long z = ts / 86400L;
    long tod = ts % 86400L;
    if (tod < 0)      /* 负时间戳时修正整除向下的余数 */
    {
        tod += 86400L;
        z   -= 1;
    }

    int y; unsigned m, d;
    civil_from_days(z, &y, &m, &d);

    rtc_date->Year  = (uint8_t)(y - 2000);
    rtc_date->Month = (uint8_t)m;
    rtc_date->Date  = (uint8_t)d;

    /* 星期：epoch(1970-01-01) 为周四；sun0 记 0=Sun..6=Sat，转 HAL 1=Mon..7=Sun */
    int wd = (int)(((z % 7L) + 4L) % 7L);
    if (wd < 0) wd += 7;
    rtc_date->WeekDay = (uint8_t)((wd == 0) ? 7 : wd);

    rtc_time->Hours   = (uint8_t)(tod / 3600L);
    rtc_time->Minutes = (uint8_t)((tod % 3600L) / 60L);
    rtc_time->Seconds = (uint8_t)(tod % 60L);
}

/* ---------------------------------------------------------------
 * 把 "Wed, 26 Aug 2026 07:45:03 GMT" 解析成 Unix 时间戳。
 * 成功返回时间戳(>=0)；失败返回 -1。
 * --------------------------------------------------------------- */
long parse_http_date_to_timestamp(const char *date)
{
    http_date_fields_t f;
    if (parse_http_date(date, &f) != 0)
        return -1;
    if (f.year < 1970)
        return -1;

    long days = days_from_civil(f.year, (unsigned)f.month, (unsigned)f.day);
    long secs = days * 86400L + f.hour * 3600L + f.minute * 60L + f.second;
    return secs;
}

/* ---------------------------------------------------------------
 * [HAL] 把 "Wed, 26 Aug 2026 07:45:03 GMT" 解析成 HAL 的
 * RTC_DateTypeDef / RTC_TimeTypeDef，可直接传给：
 *   HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
 *   HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
 * 成功返回 0；失败返回 -1（此时不应调用 HAL 写入）。
 *
 * 注意：HAL 的 Year 为 0-99（表示 2000-2099），WeekDay 为 1=Monday..7=Sunday。
 * --------------------------------------------------------------- */
#if defined(USE_HAL_DRIVER)
/* ---------------------------------------------------------------
 * [HAL] 把 "Wed, 26 Aug 2026 07:45:03 GMT" 解析成 HAL 的
 * RTC_DateTypeDef / RTC_TimeTypeDef，可加时区偏移（秒）。
 *
 * 处理流程：HTTP 日期(UTC) -> Unix 时间戳 -> 加 tz_offset_sec
 *           -> 转回公历日期 + 时分秒 + 星期。可正确处理跨天/跨月/跨年。
 * tz_offset_sec：正数表示东时区（如中国 UTC+8 传 8*3600），
 *                负数表示西时区，0 表示 UTC 不变。
 *
 * 成功返回 0；失败返回 -1。
 * --------------------------------------------------------------- */
int parse_http_date_to_rtc_ex(const char *date, long tz_offset_sec,
                              RTC_DateTypeDef *rtc_date,
                              RTC_TimeTypeDef *rtc_time)
{
    http_date_fields_t f;
    if (parse_http_date(date, &f) != 0)
        return -1;
    if (f.year < 2000 || f.year > 2099)
        return -1;

    /* UTC 时间戳 + 时区偏移 */
    long days = days_from_civil(f.year, (unsigned)f.month, (unsigned)f.day);
    long ts = days * 86400L + f.hour * 3600L + f.minute * 60L + f.second
              + tz_offset_sec;

    timestamp_to_rtc(ts, rtc_date, rtc_time);
    return 0;
}

/* [HAL] 无时区偏移版本（等价于 tz_offset_sec = 0） */
int parse_http_date_to_rtc(const char *date,
                           RTC_DateTypeDef *rtc_date,
                           RTC_TimeTypeDef *rtc_time)
{
    return parse_http_date_to_rtc_ex(date, 0L, rtc_date, rtc_time);
}
#endif /* USE_HAL_DRIVER */


