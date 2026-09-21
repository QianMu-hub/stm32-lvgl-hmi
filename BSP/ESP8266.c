
#include "ESP8266.h"
#include "city_config.h"
/* 环形缓冲区 */
#define BUF_LEN 2048
volatile static uint8_t RX_Buffer[BUF_LEN];
static int RX_Buffer_R, RX_Buffer_W;

#define NEXT_POS(x) ((x + 1) % BUF_LEN)

TaskHandle_t Get_Weather_Data_Task_handle;
TaskHandle_t weather_task_handle;

volatile uint8_t is_esp8266_ready = 0;

uint8_t UART_Temp;
char AT[] = "AT\r\n";
char RST[] = "AT+RST\r\n";
char ATCMD[] = "AT+HTTPCLIENT=?\r\n";
char ATE0[] = "ATE0\r\n";
char AT_GMR[] = "AT+GMR\r\n";
char wifi_sc[] = "AT+CWSTATE?\r\n";
char connect_wifi[] = "AT+CWJAP=\"H-03-1-2G\",\"jiang2019\"\r\n";
char station_mode[] = "AT+CWMODE=1\r\n";
char TCP[] = "AT+CIPSTART=\"TCP\",\"15.235.222.69\",80\r\n";
char CIPMODE[] = "AT+CIPMODE=1\r\n";
char CIPSEND[] = "AT+CIPSEND\r\n";
char CIPCLOSE[] = "AT+CIPCLOSE\r\n";
char data_end[] = "+++"; //
char get_weather_data[] = "\r\n";
uint32_t time_start_time = 0;
uint32_t time_end_time = 0;
// uint8_t get_ip[]="AT+CIPDOMAIN=\"api.openweathermap.org\"\r\n";
// uint8_t SSL[]="AT+CIPSTART=\"TCP\",\"www.bing.com\",443,,,5000\r\n";

// int frame_ready;

/**********************************************************************
 * 函数名称： isRX_BufferEmpty
 * 功能描述： 环形缓冲区是否空
 ***********************************************************************/
static int isRX_BufferEmpty(void)
{
	return (RX_Buffer_R == RX_Buffer_W);
}

/**********************************************************************
 * 函数名称： isRX_BufferFull
 * 功能描述： 环形缓冲区是否满
 ***********************************************************************/
static int isRX_BufferFull(void)
{
	return (RX_Buffer_R == NEXT_POS(RX_Buffer_W));
}

/**********************************************************************
 * 函数名称： PutDataToBuf
 * 功能描述： 写入环形缓冲区
 ***********************************************************************/
static void PutDataToBuf(uint8_t data)
{
	if (!isRX_BufferFull())
	{
		RX_Buffer[RX_Buffer_W] = data;
		RX_Buffer_W = NEXT_POS(RX_Buffer_W);
	}
}

/**********************************************************************
 * 函数名称： GetDataFromBuf
 * 功能描述： 从环形缓冲区读取数据
 ***********************************************************************/
static uint8_t GetDataFromBuf(void)
{
	unsigned char data = 0xff;
	if (!isRX_BufferEmpty())
	{
		data = RX_Buffer[RX_Buffer_R];
		RX_Buffer_R = NEXT_POS(RX_Buffer_R);
	}
	return data;
}

/*发送AT指令*/
void AT_Send(char *cmd, uint16_t size)
{
	HAL_UART_Transmit_IT(&uart6, (uint8_t *)cmd, size);
}
/*发送AT指令pro*/
uint8_t AT_Send_pro(char *cmd, char *reply, uint16_t timeout)
{
	if (reply == NULL)
	{
		return 1;
	}
	RX_Buffer_W = RX_Buffer_R;	// 清空环形缓冲区
	int Buffer_R = RX_Buffer_R; // 本地读指针
	uint8_t cahe;
	uint8_t reply_len = strlen((const char *)reply);
	uint8_t size = strlen((const char *)cmd);
	HAL_UART_Transmit_IT(&uart6, (uint8_t *)cmd, size);
	if (reply != NULL)
	{
		uint32_t start_tick = HAL_GetTick();		   // 记录开始时间
		uint8_t match_index = 0;					   // 匹配索引
		while ((HAL_GetTick() - start_tick) < timeout) // 等待超时
		{
			if (!(Buffer_R == RX_Buffer_W)) // 如果环形缓冲区不为空
			{
				cahe = RX_Buffer[Buffer_R];	   // 读取当前字节
				Buffer_R = NEXT_POS(Buffer_R); // 更新本地读指针

				if (cahe == reply[match_index]) // 如果当前字节匹配 reply 中的字符
				{
					match_index++; // 匹配成功，往后推进
					if (match_index == reply_len)
					{
						return 1; // 完全匹配
					}
				}
				else
				{
					// 失败时，检查当前字节是否能作为新匹配的起点
					if (cahe == reply[0])
					{
						match_index = 1;
					}
					else
					{
						match_index = 0;
					}
				}
			}
			vTaskDelay(pdMS_TO_TICKS(10)); // 避免忙等待，降低CPU占用
		}
		return 0; // 超时未匹配到 "OK" 或 "CONNECT"
	}
	return 0;
}
void AT_AP_Connect(void)
{
	AT_Send(connect_wifi, (sizeof connect_wifi) - 1);
}

void AT_CMD(void)
{
	AT_Send(AT_GMR, (sizeof AT_GMR) - 1);
}
/*



*/
void Get_Weather_Data(void)
{
	while (is_esp8266_ready == 0)
	{
	}

	uint8_t ready = 0;
	char http_req[256];
	city_build_http_request(http_req, sizeof(http_req)); // 生成带当前城市的请求

	for (int i = 0; i < 2; i++)
	{
		time_start_time = HAL_GetTick();
		if (AT_Send_pro(TCP, "CONNECT", 5000) == 0)
		{
			continue;
		}

		if (AT_Send_pro(CIPMODE, "OK", 2000) == 0)
		{
			vTaskDelay(pdMS_TO_TICKS(1000));
			AT_Send(data_end, (sizeof data_end) - 1);
			vTaskDelay(pdMS_TO_TICKS(100));
			AT_Send(CIPCLOSE, (sizeof CIPCLOSE) - 1);
			continue;
		}
		if (AT_Send_pro(CIPSEND, "OK", 2000) == 0)
		{
			vTaskDelay(pdMS_TO_TICKS(1000));
			AT_Send(data_end, (sizeof data_end) - 1);
			vTaskDelay(pdMS_TO_TICKS(100));
			AT_Send(CIPCLOSE, (sizeof CIPCLOSE) - 1);
			continue;
		}
		if (AT_Send_pro(http_req, "HTTP", 8000) == 0)
		{
			vTaskDelay(pdMS_TO_TICKS(1000));
			AT_Send(data_end, (sizeof data_end) - 1);
			vTaskDelay(pdMS_TO_TICKS(100));
			AT_Send(CIPCLOSE, (sizeof CIPCLOSE) - 1);
			continue;
		}
		uint32_t start = HAL_GetTick();
		while ((HAL_GetTick() - start) < 3000) // 等待 5 秒，检查数据是否到达
		{
			// 简单判断缓冲区里有没有数据（且最后收到的是 '}'）
			if (!isRX_BufferEmpty())
			{
				int temp_r = RX_Buffer_R;
				int temp_w = RX_Buffer_W;
				int available = (temp_w - temp_r + BUF_LEN) % BUF_LEN;
				if (available > 100)
				{ // 至少 100 字节，基本确定 JSON 来了
					int last = (temp_w - 1 + BUF_LEN) % BUF_LEN;
					if (RX_Buffer[last] == '}' || RX_Buffer[last] == '\n')
					{
						ready = 1;
						break; // 数据已到达，立即跳出延时
					}
				}
			}
			vTaskDelay(pdMS_TO_TICKS(10)); // 没数据就等 10ms 再查
		}
		if (ready)
		{
			vTaskDelay(pdMS_TO_TICKS(1000));
			AT_Send(data_end, (sizeof data_end) - 1);
			vTaskDelay(pdMS_TO_TICKS(100));
			AT_Send(CIPCLOSE, (sizeof CIPCLOSE) - 1);
			xTaskNotifyGive(weather_task_handle);
			break; // 数据已到达，跳出循环
		}
	}
}
/* 获取天气数据任务 */
void Get_Weather_Data_Task(void *pvParameters)
{
	vTaskDelay(pdMS_TO_TICKS(3000));
	while (ulTaskNotifyTake(pdTRUE, 0) == pdPASS)
		;
	Get_Weather_Data();
	// 周期性获取（每 10 分钟）
	while (1)
	{
		ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(10 * 60 * 1000));
		Get_Weather_Data();
		while (ulTaskNotifyTake(pdTRUE, 0) == pdPASS)
			;
	}
}

uint8_t Data_Read_all(char *data)
{
	while (!isRX_BufferEmpty())
	{
		for (int i = 0; i < 1023; i++)
		{
			data[i] = GetDataFromBuf();
			if (isRX_BufferEmpty())
			{
				data[i + 1] = '\0';
				return 1;
			}
		}
		data[1023] = '\0';
		return 1;
	}
	data[0] = '\0';
	return 0;
}
/*按行读取环形缓冲区*/
void Data_Read_line(char *data)
{
	HAL_Delay(20);
	int i = 0;
	int found_r = 0;
	int found_n = 0;

	int temp_r, temp_w, temp_pos;
	int len;
	temp_r = RX_Buffer_R;
	temp_w = RX_Buffer_W;

	if (temp_r == temp_w)
	{
		data[0] = '\0';
		return;
	}
	// 查找第一个 '\r'
	while (temp_r != temp_w)
	{
		if (RX_Buffer[temp_r] == '\r')
		{
			found_r = 1;
			temp_pos = temp_r;
			temp_r = NEXT_POS(temp_r);
			temp_r = NEXT_POS(temp_r);
			break;
		}
		temp_r = NEXT_POS(temp_r);
	}
	// 查找第二个 '\n'
	while (temp_r != temp_w)
	{
		if (RX_Buffer[temp_r] == '\n')
		{
			found_n = 1;
			break;
		}
		temp_r = NEXT_POS(temp_r);
	}
	if (!(found_r && found_n))
	{
		data[0] = '\0';
		return;
	}
	// 计算指令字节数
	if (temp_r >= temp_pos)
	{
		len = temp_r - temp_pos + 1;
	}
	else
	{
		len = (BUF_LEN - temp_pos) + temp_r + 1;
	}
	// 逐字节读取
	while (i < len)
	{
		uint8_t temp = GetDataFromBuf();
		if ((i > 1) && (i < (len - 2)))
		{
			data[i - 2] = temp;
		}
		i++;
	}
	data[len - 2] = '\0';
}

void ESP8266_Test(void)
{
	AT_Send(AT_GMR, 8);
	HAL_UART_Receive_IT(&uart6, &UART_Temp, 1);
}

/***********************************************
 esp8266初始化
************************************************/
void ESP8266_Init(void)
{
	uint8_t p1 = 0, p2 = 0;
	esp8266_uart_init();
	HAL_UART_Receive_IT(&uart6, &UART_Temp, 1); // 启动接收，否则 AT_Send_pro 收不到回复
	vTaskDelay(1000);							// 延时 1 秒，确保 ESP8266 模块上电稳定

	while (is_esp8266_ready == 0)
	{
		while (AT_Send_pro(RST, "ready", 3000))
		{	
			vTaskDelay(1100);
			AT_Send_pro(data_end, NULL, 0);
			vTaskDelay(1100);
		}

		if (AT_Send_pro(station_mode, "OK", 3000) == 1)
		{
			p1 = 1;
		}

		if (AT_Send_pro(connect_wifi, "OK", 10000) == 1)
		{
			p2 = 1;
		}

		if (p1 && p2)
		{
			is_esp8266_ready = 1;
		}
	}
}

/*中断*/
void USART6_IRQHandler(void)
{
	HAL_UART_IRQHandler(&uart6);
}

/*中断回调函数*/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART6)
	{
		PutDataToBuf(UART_Temp);

		HAL_UART_Receive_IT(&uart6, &UART_Temp, 1);
	}
}
