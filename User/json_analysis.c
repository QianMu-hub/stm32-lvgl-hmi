
#include "json_analysis.h"
#include "RTC.h"
/**
 * @brief  读取 ESP8266 缓冲区中的完整 HTTP 响应，先按其中的 Date 头同步 RTC，再用 cJSON 解析出城市名、温度和天气描述。
 * @param  City  输出参数，存放城市名的指针的地址；取到 JSON 的 "name" 时用 pvPortMalloc 分配并拷贝，失败时不改写。
 * @param  Temp  输出参数，温度（摄氏度）的地址；取到 main.temp 时写入，失败时不改写。
 * @param  Desc  输出参数，存放天气描述的指针的地址；取到 weather[0].description 时用 pvPortMalloc 分配并拷贝。
 * @retval 无（无数据、找不到响应体或 JSON 解析失败时直接返回，输出参数保持调用前的值）
 */
void parse_weather(char **City, double *Temp, char **Desc)
{
	static char http_response[1024] = {0};
	if (Data_Read_all(http_response) == 0)
	{
		return;
	}
	time_end_time=HAL_GetTick();
	uint32_t dif= (time_end_time-time_start_time)/1000;
	//uint32_t dif=0;
	RTC_UpdateTimeFromHttp(http_response, 8 * 3600+dif); // 同步 RTC 时间（服务器时间为 UTC，中国时区 +8h）

	// 在整个响应中查找响应开头行
	char *http_start = strstr(http_response, "HTTP/1.1");
	if (http_start == NULL)
	{
		// 没有找到响应，可能是数据不完整或格式错误
		return;
	}
	// 从响应开头行开始，查找头部结束标志（空行）
	char *header_end = strstr(http_start, "\r\n\r\n");
	if (header_end == NULL)
	{
		return;
	}
	if (header_end != NULL)
	{
		char *json_body = header_end + 4; // 跳过空行
		// 找到第一个 '{'
		char *start = strchr(json_body, '{');
		if (!start)
			return;

		// 找到最后一个 '}'（从末尾往前找）
		char *end = strrchr(start, '}');
		if (!end)
			return;

		// 在 '}' 后面置 '\0'，截断字符串
		*(end + 1) = '\0';
		// HAL_UART_Transmit_IT(&uart6, json_body, strlen(json_body));
		cJSON *root = cJSON_Parse(json_body);
		if (root == NULL)
		{
			return;
		}
		// 获取城市名称
		cJSON *name = cJSON_GetObjectItem(root, "name");
		if (cJSON_IsString(name))
		{
			const char *src = name->valuestring;
			if (src != NULL)
			{
				size_t len = strlen(src) + 1; // +1 包含 '\0'
				*City = (char *)pvPortMalloc(len);
				if (*City != NULL)
				{
					strcpy(*City, src);
				}
			}
		}
		// 获取温度
		cJSON *main = cJSON_GetObjectItem(root, "main");
		if (cJSON_IsObject(main))
		{
			cJSON *temp = cJSON_GetObjectItem(main, "temp");
			if (cJSON_IsNumber(temp))
			{
				double t = temp->valuedouble;
				*Temp = t;
			}
		}
		// 获取天气描述
		cJSON *weather = cJSON_GetObjectItem(root, "weather");
		if (cJSON_IsArray(weather) && cJSON_GetArraySize(weather) > 0)
		{
			cJSON *first = cJSON_GetArrayItem(weather, 0);
			cJSON *desc = cJSON_GetObjectItem(first, "description");
			if (cJSON_IsString(desc))
			{
				const char *src = desc->valuestring;
				if (src != NULL)
				{
					size_t len = strlen(src) + 1;
					*Desc = (char *)pvPortMalloc(len);
					if (*Desc != NULL)
					{
						strcpy(*Desc, src);
					}
				}
			}
		}
		cJSON_Delete(root);
	}
	else
		return;
}

/*
天气更新任务，周期性获取天气数据并发送到队列
*/
void weather_task(void *pvParameters)
{
	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY); // 等待通知，阻塞直到有新数据到达
		// 获取天气数据（注意内存分配）
		char *city = NULL;
		char *desc = NULL;
		double temp;
		parse_weather(&city, &temp, &desc);

		// 准备消息
		weather_msg_t msg;
		msg.city = city; // 传递指针，所有权转移给接收方
		msg.desc = desc;


		// 格式化温度到字符串（附带摄氏度单位 °C；° 用 UTF-8 转义，且与后面的 C 字符串拼接，
		// 避免 \xB0C 被误解析为十六进制转义 0xB0C）
		
		int temp_int = (int)(temp * 10 + (temp >= 0 ? 0.5 : -0.5));
		snprintf(msg.temp_str, sizeof(msg.temp_str), "%d.%d \xC2\xB0" "C", temp_int / 10, abs(temp_int % 10));

		// 发送到队列（若队列满则等待最多 100ms）
		if (xQueueSend(weather_queue, &msg, pdMS_TO_TICKS(100)) != pdPASS)
		{
			// 发送失败，释放内存
			vPortFree(city);
			vPortFree(desc);
		}
	}
}
