
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_rcc.h"
#include "stm32f4xx_hal_rcc_ex.h"
#include "stm32f4xx_hal_rtc.h"
#include "stm32f4xx_hal_rtc_ex.h"
#include "main.h"
#include "RTC.h"
#include "date_to_timestamp.h"

RTC_HandleTypeDef hrtc = {0};

/**
 * @brief  初始化 RTC：使能电源与备份域时钟，时钟源优先 LSE（起振失败自动降级为 LSI），并仅在首次上电（备份寄存器 RTC_BKP_DR1 未标记）写入 2026-01-01 12:00:00 默认时间
 * @param  无
 * @retval 无
 */
void RTC_Init(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};

    __HAL_RCC_PWR_CLK_ENABLE(); // 使能电源接口时钟
    HAL_PWR_EnableBkUpAccess(); // 使能备份域访问（取消写保护）

    // 优先使用 LSE，若未接晶振/起振失败则降级到 LSI
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE;
    RCC_OscInitStruct.LSEState = RCC_LSE_ON; // 开启LSE
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        // LSE 起振失败，改用 LSI
        RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI;
        RCC_OscInitStruct.LSIState = RCC_LSI_ON;
        if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
        {
            Error_Handler();
        }
        PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
        PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
    }
    else
    {
        PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
        PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE; // 选择LSE作为RTC时钟源
    }
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
        Error_Handler();
    }
    __HAL_RCC_RTC_ENABLE(); // 使能RTC时钟

    // 先配置实例与参数，再初始化（HAL_RTC_Init 内部会自动等待寄存器同步）
    hrtc.Instance = RTC;
    hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
    hrtc.Init.AsynchPrediv = 127; // 异步预分频器值
    hrtc.Init.SynchPrediv = 255;  // 同步预分频器值
    hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
    if (HAL_RTC_Init(&hrtc) != HAL_OK)
    {
        Error_Handler();
    }

    // 仅首次上电写入默认时间，之后保持（用备份寄存器做已初始化标志）
    if (HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1) != 0x32F2U)
    {
        sTime.Hours = 12;
        sTime.Minutes = 0;
        sTime.Seconds = 0;
        sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
        sTime.StoreOperation = RTC_STOREOPERATION_RESET;
        if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
        {
            Error_Handler();
        }

        sDate.WeekDay = RTC_WEEKDAY_MONDAY;
        sDate.Month = RTC_MONTH_JANUARY;
        sDate.Date = 1;
        sDate.Year = 26; // 表示2026年
        if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
        {
            Error_Handler();
        }

        HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, 0x32F2U);
    }
}

/* ---------------------------------------------------------------
 * 设置 RTC 日期时间
 * --------------------------------------------------------------- */
int RTC_SetDateTime(uint8_t year, uint8_t month, uint8_t day, uint8_t weekDay,
                    uint8_t hour, uint8_t minute, uint8_t second)
{
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};

    sTime.Hours = hour;
    sTime.Minutes = minute;
    sTime.Seconds = second;
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;
    if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
    {
        return -1;
    }

    sDate.Year = year;      // 0-99
    sDate.Month = month;    // 1-12
    sDate.Date = day;       // 1-31
    sDate.WeekDay = weekDay;// 1=Mon..7=Sun
    if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
    {
        return -1;
    }

    return 0;
}

/* ---------------------------------------------------------------
 * 从 HTTP 日期字符串同步 RTC 时间
 * tz_offset_sec：时区偏移（秒），中国 UTC+8 传 8*3600，UTC 传 0。
 * --------------------------------------------------------------- */
int RTC_UpdateTimeFromHttp(const char *date, long tz_offset_sec)
{
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};

    if (parse_http_date_to_rtc_ex(date, tz_offset_sec, &sDate, &sTime) != 0)
    {
        return -1;
    }

    return RTC_SetDateTime(sDate.Year, sDate.Month, sDate.Date, sDate.WeekDay,
                           sTime.Hours, sTime.Minutes, sTime.Seconds);
}

/* ---------------------------------------------------------------
 * 读取 RTC 日期时间
 * 注意：必须先读时间再读日期（HAL 需读时间寄存器以解锁日期影子寄存器）
 * --------------------------------------------------------------- */
int RTC_GetDateTime(RTC_DateTypeDef *sDate, RTC_TimeTypeDef *sTime)
{
    if (sDate == NULL || sTime == NULL)
    {
        return -1;
    }

    HAL_RTC_GetTime(&hrtc, sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, sDate, RTC_FORMAT_BIN);
    return 0;
}
