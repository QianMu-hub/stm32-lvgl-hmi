/**
 * @file    city_config.c
 * @brief   Major China cities config: build weather HTTP request with selected city.
 *
 * All city names are ASCII English (recognized by OpenWeatherMap).
 * Default city is Ganzhou.
 */

#include "city_config.h"
#include <stdio.h>
#include <string.h>

/* OpenWeatherMap API Key (same as the original request) */
#include "api_keys.h"
#ifndef CITY_APPID
#define CITY_APPID  "OpenWeatherMapAPIKey"
#endif

static const city_item_t g_cities[] = {
    {"Ganzhou"},
    {"Beijing"},
    {"Shanghai"},
    {"Guangzhou"},
    {"Shenzhen"},
    {"Chengdu"},
    {"Chongqing"},
    {"Hangzhou"},
    {"Wuhan"},
    {"Nanjing"},
    {"Tianjin"},
    {"Xi'an"},
    {"Suzhou"},
    {"Qingdao"},
    {"Dalian"},
    {"Xiamen"},
    {"Changsha"},
    {"Zhengzhou"},
    {"Shenyang"},
    {"Harbin"},
    {"Kunming"},
    {"Nanning"},
    {"Fuzhou"},
    {"Hefei"},
    {"Jinan"},
    {"Shijiazhuang"},
    {"Taiyuan"},
    {"Nanchang"},
    {"Changchun"},
    {"Guiyang"},
    {"Lanzhou"},
    {"Urumqi"},
    {"Hohhot"},
    {"Yinchuan"},
    {"Xining"},
    {"Lhasa"},
};

#define CITY_COUNT  (sizeof(g_cities) / sizeof(g_cities[0]))

static uint16_t s_current_city = 0;

/**
 * @brief   获取城市总数。
 * @return  城市表 g_cities 的条目数。
 */
uint16_t city_get_count(void)
{
    return CITY_COUNT;
}

/**
 * @brief   按索引获取城市条目。
 * @param   index  城市索引（0 ~ city_get_count()-1）
 * @return  指向对应城市条目的指针；越界返回 NULL。
 */
const city_item_t *city_get_item(uint16_t index)
{
    if (index >= CITY_COUNT)
        return NULL;
    return &g_cities[index];
}

/**
 * @brief   按索引设置当前城市。
 * @param   index  城市索引（0 ~ city_get_count()-1）
 * @return  0 成功；-1 索引越界。
 */
int city_set_by_index(uint16_t index)
{
    if (index >= CITY_COUNT)
        return -1;
    s_current_city = index;
    return 0;
}

/**
 * @brief   获取当前城市索引。
 * @return  当前城市的索引。
 */
uint16_t city_get_current_index(void)
{
    return s_current_city;
}

/**
 * @brief   切换到下一个城市（循环到第一个）。
 * @return  恒为 0。
 */
int city_next(void)
{
    s_current_city = (s_current_city + 1) % CITY_COUNT;
    return 0;
}

/**
 * @brief   切换到上一个城市（循环到最后一个）。
 * @return  恒为 0。
 */
int city_prev(void)
{
    s_current_city = (s_current_city + CITY_COUNT - 1) % CITY_COUNT;
    return 0;
}

/**
 * @brief   获取当前城市的英文查询名（用于 HTTP 请求 q=xxx）。
 * @return  当前城市的 query_name 字符串。
 */
const char *city_get_query_name(void)
{
    return g_cities[s_current_city].query_name;
}

/**
 * @brief   生成带当前城市的完整 HTTP GET 请求到 buf。
 * @param   buf       输出缓冲区。
 * @param   buf_size  缓冲区大小（字节），请求末尾带 '\0'。
 * @note    请求格式：GET /data/2.5/weather?q=<city>&appid=<key>&units=metric ...
 *          含 HTTP 头部结尾的 \r\n\r\n。
 */
void city_build_http_request(char *buf, uint16_t buf_size)
{
    snprintf(buf, buf_size,
             "GET /data/2.5/weather?q=%s&appid=%s&units=metric HTTP/1.1\r\n"
             "Host: api.openweathermap.org\r\n"
             "Connection: close\r\n\r\n",
             city_get_query_name(), CITY_APPID);
}
