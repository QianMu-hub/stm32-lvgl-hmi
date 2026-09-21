/**
 * @file    city_config.h
 * @brief   Major China cities config: replace city in weather HTTP request.
 *
 * Cities use OpenWeatherMap English names only (q=Beijing etc.),
 * to avoid source-encoding problems and to be shown by default font.
 */

#ifndef CITY_CONFIG_H
#define CITY_CONFIG_H

#include <stdint.h>

/* City item */
typedef struct {
    const char *query_name; /* OpenWeatherMap q param English name (Ganzhou/Beijing/...) */
} city_item_t;

/* Number of cities */
uint16_t city_get_count(void);

/* Get city item by index; NULL if out of range */
const city_item_t *city_get_item(uint16_t index);

/* Set current city by index; return 0 on success, -1 if out of range */
int city_set_by_index(uint16_t index);

/* Get current city index */
uint16_t city_get_current_index(void);

/* Switch to next/prev city (wraps around) */
int city_next(void);
/**
 * @brief  切换到城市列表中的上一个城市（到列表开头后循环到最后一个）
 * @param  无
 * @retval 恒为 0，表示切换成功
 */
int city_prev(void);

/* Get current city English query name */
const char *city_get_query_name(void);

/* Build full HTTP GET request (with current city) into buf, null-terminated */
void city_build_http_request(char *buf, uint16_t buf_size);

#endif /* CITY_CONFIG_H */
