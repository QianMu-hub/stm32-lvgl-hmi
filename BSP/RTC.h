/**
 * @file    RTC.h
 * @brief   STM32 RTC 驱动接口（HAL 实现）。
 *          对应实现：RTC.c
 *
 * 功能：初始化 RTC、设置/更新日期时间、读取日期时间、
 *       以及从 HTTP 日期字符串直接同步 RTC。
 */

#ifndef __RTC_H
#define __RTC_H

#include <stdint.h>
#include "stm32f4xx_hal_rtc.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  初始化 RTC（时钟源 LSE/LSI 自适应、首次上电写入默认时间）。
 */
void RTC_Init(void);

/**
 * @brief  设置 RTC 日期时间。
 * @param  year    0-99，表示 2000-2099 年
 * @param  month   1-12
 * @param  day     1-31
 * @param  weekDay 1=Mon .. 7=Sun
 * @param  hour    0-23
 * @param  minute  0-59
 * @param  second  0-59
 * @retval 0 成功；-1 失败。
 */
int RTC_SetDateTime(uint8_t year, uint8_t month, uint8_t day, uint8_t weekDay,
                    uint8_t hour, uint8_t minute, uint8_t second);

/**
 * @brief  从 HTTP 日期字符串同步 RTC。
 *         date 形如 "Wed, 26 Aug 2026 07:45:03 GMT"。
 * @param  date           HTTP Date 响应头内容。
 * @param  tz_offset_sec  时区偏移（秒）；中国 UTC+8 传 8*3600，UTC 传 0，负值表示西时区。
 * @retval 0 成功；-1 失败（解析失败或写入失败）。
 */
int RTC_UpdateTimeFromHttp(const char *date, long tz_offset_sec);

/**
 * @brief  读取 RTC 日期时间。
 * @param  sDate 输出日期结构体（WeekDay/Month/Date/Year）。
 * @param  sTime 输出时间结构体（Hours/Minutes/Seconds）。
 * @retval 0 成功；-1 参数为空。
 * @note   必须先读时间再读日期，HAL 会据此解锁影子寄存器。
 */
int RTC_GetDateTime(RTC_DateTypeDef *sDate, RTC_TimeTypeDef *sTime);

#ifdef __cplusplus
}
#endif

#endif /* __RTC_H */
