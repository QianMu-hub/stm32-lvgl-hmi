/**
 * @file    date_to_timestamp.h
 * @brief   解析 HTTP 日期字符串 "Wdy, DD Mon YYYY HH:MM:SS GMT"，
 *          提供 Unix 时间戳转换 与 STM32 HAL RTC 结构体填充接口。
 *
 * 对应实现：date_to_timestamp.c
 */

#ifndef DATE_TO_TIMESTAMP_H
#define DATE_TO_TIMESTAMP_H

#include <stdint.h>

#if defined(USE_HAL_DRIVER)
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_rtc.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  将 HTTP 日期字符串解析为 Unix 时间戳（UTC，自 1970-01-01 00:00:00 的秒数）。
 * @param  date 形如 "Wed, 26 Aug 2026 07:45:03 GMT" 的日期字符串。
 * @retval 成功返回时间戳(>=0)；失败返回 -1。
 */
long parse_http_date_to_timestamp(const char *date);

#if defined(USE_HAL_DRIVER)
/**
 * @brief  将 HTTP 日期字符串(UTC)解析为 HAL 的 RTC_DateTypeDef / RTC_TimeTypeDef，
 *         并应用时区偏移（秒），可直接传给 HAL_RTC_SetDate() / HAL_RTC_SetTime()。
 * @param  date          形如 "Wed, 26 Aug 2026 07:45:03 GMT" 的日期字符串。
 * @param  tz_offset_sec 时区偏移，单位秒；中国 UTC+8 传 8*3600，UTC 传 0，负值表示西时区。
 * @param  rtc_date      输出：WeekDay=1..7, Month=1..12, Date=1..31, Year=0..99(2000-2099)。
 * @param  rtc_time      输出：Hours/Minutes/Seconds。
 * @retval 成功返回 0；失败返回 -1。
 */
int parse_http_date_to_rtc_ex(const char *date, long tz_offset_sec,
                              RTC_DateTypeDef *rtc_date,
                              RTC_TimeTypeDef *rtc_time);

/**
 * @brief  将 HTTP 日期字符串解析为 HAL 的 RTC_DateTypeDef / RTC_TimeTypeDef，
 *         不进行时区偏移（等价于 tz_offset_sec = 0）。
 */
int parse_http_date_to_rtc(const char *date,
                           RTC_DateTypeDef *rtc_date,
                           RTC_TimeTypeDef *rtc_time);
#endif /* USE_HAL_DRIVER */

#ifdef __cplusplus
}
#endif

#endif /* DATE_TO_TIMESTAMP_H */
