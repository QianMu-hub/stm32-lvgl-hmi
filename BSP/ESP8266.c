
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
uint32_t time_start_time = 0;
uint32_t time_end_time = 0;

/**
 * @brief  判断串口接收环形缓冲区是否为空
 * @param  无
 * @retval 1 - 空（读指针与写指针重合）；0 - 非空
 */
static int isRX_BufferEmpty(void)
{
	return (RX_Buffer_R == RX_Buffer_W);
}

/**
 * @brief  判断串口接收环形缓冲区是否已满
 * @param  无
 * @retval 1 - 已满；0 - 未满
 * @note   缓冲区牺牲一个字节用于区分"满"和"空"，即最多只能暂存 BUF_LEN-1 个字节
 */
static int isRX_BufferFull(void)
{
	return (RX_Buffer_R == NEXT_POS(RX_Buffer_W));
}

/**
 * @brief  把一个字节写入串口接收环形缓冲区（由 USART6 接收中断逐字节调用）
 * @param  data  待写入的字节（串口刚收到的数据）
 * @retval 无
 * @note   缓冲区满时该字节被直接丢弃（不覆盖旧数据），因此读取不及时会丢数据
 */
static void PutDataToBuf(uint8_t data)
{
	if (!isRX_BufferFull())
	{
		RX_Buffer[RX_Buffer_W] = data;
		RX_Buffer_W = NEXT_POS(RX_Buffer_W);
	}
}

/**
 * @brief  从串口接收环形缓冲区取出一个字节
 * @param  无
 * @retval 取出的字节；缓冲区为空时返回 0xFF（调用前可用 isRX_BufferEmpty 判断）
 */
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

/**
 * @brief  以中断方式向 ESP8266 发送 AT 指令：只负责发出，不等待发送完成，也不等待模块应答
 * @param  cmd   待发送的指令字符串（通常以 "\r\n" 结尾）
 * @param  size  实际发送的字节数，不含字符串结尾的 '\0'，因此调用处一般写 sizeof(cmd)-1 或 strlen(cmd)
 * @retval 无
 * @note   上一次发送未完成前不宜再次调用（HAL_UART_Transmit_IT 只有一份发送缓冲）；
 *         模块的应答由 USART6 接收中断逐字节存入环形缓冲区，之后可用 Data_Read_all / Data_Read_line 读取
 */
void AT_Send(char *cmd, uint16_t size)
{
	HAL_UART_Transmit_IT(&uart6, (uint8_t *)cmd, size);
}

/**
 * @brief  发送 AT 指令并阻塞等待模块应答中出现指定关键字，用于"发一条、等一个回复"的场合
 * @param  cmd      待发送的指令字符串（以 "\r\n" 结尾），发送长度由 strlen 计算
 * @param  reply    期望在应答中匹配到的关键字，如 "OK"、"CONNECT"、"ready"、"HTTP"
 * @param  timeout  等待应答的超时时间，单位: 毫秒(ms)
 * @retval 1 - 超时前匹配到 reply（成功）；0 - 超时仍未匹配到（失败）
 * @note   1) 每次调用会先把环形缓冲区清空（丢弃尚未读取的旧数据），匹配时使用本地读指针，
 *            全局读指针不会前移，所以匹配过的应答内容之后仍可被 Data_Read_all / Data_Read_line 读出；
 *         2) 等待循环里调用了 vTaskDelay，必须在 FreeRTOS 任务中调用，不能在中断服务函数中调用。
 */
uint8_t AT_Send_pro(char *cmd, char *reply, uint16_t timeout)
{
	uint8_t reply_len = strlen((const char *)reply);
	uint8_t size = strlen((const char *)cmd);

	if (reply == NULL)
	{
		HAL_UART_Transmit_IT(&uart6, (uint8_t *)cmd, size);
		return 1;
	}

	RX_Buffer_W = RX_Buffer_R;	// 清空环形缓冲区
	int Buffer_R = RX_Buffer_R; // 本地读指针
	uint8_t cahe;

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



/**
 * @brief  把串口环形缓冲区里已收到的数据一次性全部读出为字符串（最多 1023 字节）
 * @param  data  输出缓冲区，用于保存读出的字符串，调用者需保证容量不小于 1024 字节
 * @retval 1 - 读到了数据；0 - 缓冲区为空（此时 data[0] 被置为 '\0'）
 */
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

/**
 * @brief  从串口环形缓冲区中读取一整行模块应答（例如 "OK"、"ready"）
 * @param  data  输出缓冲区，保存读出的行内容；没有完整的一行可读时置为 '\0'
 * @retval 无
 * @note   1) 开头用 HAL_Delay(20) 阻塞等待数据到达；
 *         2) 以第一个 '\r' 为起点、跳过其后 2 字节再找到的 '\n' 作为一行结束，
 *            读取时丢掉行首 2 字节与行尾 2 字节，因此只能解析形如 "\r\n...\r\n" 的完整应答；
 *         3) 读取会真正前移全局读指针，被读走的数据不再保留；
 *         4) 结束符写在 data[len-2]，而内容只写到 data[len-5]，中间 2 字节未被赋值，
 *            若调用方未预先清零，可能残留上一次的内容。
 */
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
/**
 * @brief  以中断方式发出连接指定 WiFi 热点的 AT 指令（AT+CWJAP），不等待模块应答
 * @param  无
 * @retval 无
 */
void AT_AP_Connect(void)
{
	AT_Send(connect_wifi, (sizeof connect_wifi) - 1);
}

/**
 * @brief  以中断方式发出查询模块固件版本的 AT 指令（AT+GMR），应答由串口中断接收
 * @param  无
 * @retval 无
 */
void AT_CMD(void)
{
	AT_Send(AT_GMR, (sizeof AT_GMR) - 1);
}
/**
 * @brief  发送 AT+GMR 查询模块固件版本，并启动 USART6 的单字节中断接收（该中断是 AT_Send_pro 能收到应答的前提）
 * @param  无
 * @retval 无
 */
void ESP8266_Test(void)
{
	AT_Send(AT_GMR, 8);
	HAL_UART_Receive_IT(&uart6, &UART_Temp, 1);
}

/**
 * @brief  USART6 中断服务函数：把串口中断交给 HAL 统一分发处理
 * @param  无
 * @retval 无
 * @note   接收完成时会回调 HAL_UART_RxCpltCallback，接收错误等则由 HAL 的错误回调处理
 */
void USART6_IRQHandler(void)
{
	HAL_UART_IRQHandler(&uart6);
}

/**
 * @brief  串口接收完成回调：把刚收到的单字节存入环形缓冲区，并立即重新开启下一次单字节中断接收
 * @param  huart  触发回调的串口句柄，只有 USART6 的数据会被处理
 * @retval 无
 * @note   每收一个字节就重新调用一次 HAL_UART_Receive_IT 才能保持连续接收；
 *         环形缓冲区满时该字节会被 PutDataToBuf 丢弃
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART6)
	{
		PutDataToBuf(UART_Temp);

		HAL_UART_Receive_IT(&uart6, &UART_Temp, 1);
	}
}

/**
 * @brief  ESP8266 初始化：启动 USART6 接收、等待模块上电稳定，然后反复发送复位/工作模式/联网指令，直到成功连上 WiFi 热点
 * @param  无
 * @retval 无
 * @note   1) 内部是 while 死循环：AT+CWMODE=1 与 AT+CWJAP 都返回 "OK" 才置位 is_esp8266_ready 并返回，
 *            连不上就会一直重试，因此放在独立的 ESP8266_Init_Task 任务里执行；
 *         2) 循环中的 AT_Send_pro(data_end, NULL, 0) 因 reply 为 NULL 并不会真正发送，实际未向模块发出 "+++"；
 *         3) 依赖 esp8266_uart_init() 完成的 USART6 与 PC6/PC7 引脚配置
 */
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



/**
 * @brief  阻塞等待 ESP8266 就绪后，建立 TCP 连接并按当前城市发送 HTTP 请求获取天气 JSON 数据（最多尝试 2 次），成功后唤醒 weather_task_handle 任务解析
 * @param  无
 * @retval 无
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

/**
 * @brief  天气数据任务：按"收到通知"或"10 分钟超时"两种时机调用 Get_Weather_Data 获取并刷新天气
 * @param  pvParameters  FreeRTOS 任务参数（未使用，创建任务时传入 NULL）
 * @retval 无（任务主体是死循环，不会返回）
 * @note   启动后先延时 3 秒等待 ESP8266 初始化完成，并清空可能残留的历史通知；
 *         主循环用 ulTaskNotifyTake(pdTRUE, 10 分钟) 实现定时刷新，界面按钮点击时通过
 *         xTaskNotifyGive(Get_Weather_Data_Task_handle) 可立即打断等待、马上刷新一次
 */
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

