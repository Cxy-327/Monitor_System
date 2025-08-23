#include "esp8266.h"

#define ESP_TX_PIN    GPIO_Pin_9   //PA9（USART1_TX）
#define ESP_TX_GPIO   GPIOA
#define ESP_RX_PIN    GPIO_Pin_10   //PA10(USART1_RX）
#define ESP_RX_GPIO   GPIOA
#define ESP_USART     USART1

/*用于ESP8266接收数据的buffer、chip、rx_byte*/

BufferTypeDef ESP_RX_BUF;				//用于中断回调函数中，将接收到的数据存入
uint8_t ESP_RX_BUF_BUFF[ESP_BUF_SIZE] = {0x00};

BufferClip ESP_RX_CLIP;					//用于从ESP_RX_BUF读出数据（主要用于读出应答）
uint8_t ESP_RX_CLIP_DATA[ESP_BUF_SIZE] = {0x00};

unsigned char rx_byte;	//用于中断触发时，自动接收数据（重复覆盖写入）

/*初始化*/
void esp8266_init(void)
{
	/*初始化接收用的buffer和chip*/
	ESP_RX_BUF.buf = ESP_RX_BUF_BUFF;
  ESP_RX_BUF.size = ESP_BUF_SIZE;
  ESP_RX_CLIP.data = ESP_RX_CLIP_DATA;
  ESP_RX_CLIP.size = ESP_BUF_SIZE;
	/*UART1 PA9 PA10*/
	
//	ESP8266_Reset();		  //重启esp8266
  ESP8266_Set_Echo_Off();		//关闭回显
	HAL_UART_Receive_IT(&huart1 ,&rx_byte,1);	//开启中断，指定数据缓冲区、接收数据个数
	
//  printf("## ESP8266 Initialized ##\r\n");
	HAL_Delay(100);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart == &huart1)		//回调函数，将接收到的数据压入ESP_RX_BUF_BUFF
	{
		Buffer_Push( &ESP_RX_BUF, rx_byte);
		HAL_UART_Receive_IT(&huart1 ,&rx_byte,1);
	}
}

//发送字符串，传入字符数组
void ESP8266_Send_String(uint8_t* data)
{
//  uint8_t* start = data;
  while (*data != '\0') {

		HAL_UART_Transmit( &huart1, data, 1 , HAL_MAX_DELAY);
		
    while(__HAL_UART_GET_FLAG ( &huart1, UART_FLAG_TC) == RESET) {}
    
    data++;
  }
//  printf("USART_Sent: %s\r\n", start);
}


//发送数据，传入数组和长度
void ESP8266_Send_Data(uint8_t *data, uint8_t length)
{
		HAL_UART_Transmit( &huart1, data, length, HAL_MAX_DELAY);
		
    while(__HAL_UART_GET_FLAG ( &huart1, UART_FLAG_TC) == RESET) {}  //等待发送完成
			
}

//传入命令和预期应答
uint8_t ESP8266_Send_Cmd(char *cmd, char *ack)
{
	
	char full_cmd[64];
	sprintf (full_cmd ,"%s\r\n",cmd);					//命令格式化，添加\r\n结尾
	
	Buffer_Reset(&ESP_RX_BUF);								//清空缓冲区，为接收应答做准备
	Chip_Reset(&ESP_RX_CLIP);
	
  ESP8266_Send_String((uint8_t *)full_cmd);		//命令发送
//  printf("Send Cmd: %s\n",full_cmd);
	
  vTaskDelay( pdMS_TO_TICKS(1000));					//等待应答
	
	
	if(Buffer_Length(&ESP_RX_BUF) != NULL)	//判断是否接收到有效数据
	{
		
		Buffer_Pop_All(&ESP_RX_BUF , &ESP_RX_CLIP);  //将应答存入chip
//		ack = (char*)ESP_RX_CLIP.data;
//		printf("ack:%s\n",ack);
		Buffer_Clip_Print(&ESP_RX_CLIP);						//打印应答

		// 查找预期响应（跳过回显部分）
     char* response_start = strstr((char*)ESP_RX_CLIP.data, full_cmd);
     if(response_start == NULL) 
			{
				response_start = (char*)ESP_RX_CLIP.data;  // 如果没有找到命令，从开头开始
      } 
			else 
			{
         response_start += strlen (full_cmd);  // 跳过命令
			}
		
		if(strstr(response_start, ack) != NULL)	//判断是否为预期应答
		{
//			printf("ack success\r\n\n");
			return  ACK_SUCCESS;
		}
		else 
		{
//			printf("ack defeat\r\n\n");
			return ACK_DEFEAT;
		}
	}
	else
	{
//		printf("return defeat\r\n\n");
		return ACK_DEFEAT;
	}

}

//发送命令，通过指针ack获取应答
uint8_t ESP8266_Send_Cmd_GetAck(char *cmd, char *ack)
{
	
	char full_cmd[64];
	sprintf (full_cmd ,"%s\r\n",cmd);					//命令格式化，添加\r\n结尾
	
	Buffer_Reset(&ESP_RX_BUF);								//清空缓冲区，为接收应答做准备
	Chip_Reset(&ESP_RX_CLIP);
	
  ESP8266_Send_String((uint8_t *)full_cmd);		//命令发送
//  printf("Send Cmd: %s\n",full_cmd);
	
  vTaskDelay( pdMS_TO_TICKS(1000));					//等待应答
	
	
	if(Buffer_Length(&ESP_RX_BUF) != NULL)	//判断是否接收到有效数据
	{
		
		Buffer_Pop_All(&ESP_RX_BUF , &ESP_RX_CLIP);  //将应答存入chip
//		ack = (char*)ESP_RX_CLIP.data;
//		printf("ack:%s\n",ack);
		Buffer_Clip_Print(&ESP_RX_CLIP);						//打印应答
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!		
			// 查找预期响应（跳过回显部分）
     char* response_start = strstr((char*)ESP_RX_CLIP.data, full_cmd);
     if(response_start == NULL) 
			{
				response_start = (char*)ESP_RX_CLIP.data;  // 如果没有找到命令，从开头开始
//				printf("no_echo");
      } 
			else 
			{
         response_start += strlen (full_cmd);  // 跳过命令
//				printf("have+echo");
			}
			
			strcpy(ack,response_start);
			return ACK_SUCCESS ;
	}
	else
	{
//		printf("return defeat\r\n\n");
		return ACK_DEFEAT;
	}

}


/*esp8266重启*/
void ESP8266_Reset(void)
{
  ESP8266_Send_String((uint8_t *)"AT+RST\r\n");
	vTaskDelay( pdMS_TO_TICKS(5000));
}

/*关闭回显*/
void ESP8266_Set_Echo_Off(void)
{
  ESP8266_Send_String((uint8_t *)"ATE0\r\n");
	vTaskDelay( pdMS_TO_TICKS(500));
}

/*设置STA模式*/
//uint8_t ESP8266_Set_Stationmode(void)
//{
//  if(ESP8266_Send_Cmd("AT\r\n", "OK") != ACK_SUCCESS) 
//	{
//    ESP8266_Quit_Passthrough();		//退出透传模式
//  }

//  if(ESP8266_Send_Cmd("AT+CWMODE?\r\n", "+CWMODE:1") != ACK_SUCCESS)
//		{ 
//			printf("Not in station mode\r\n");
//				
//			if (ESP8266_Send_Cmd("AT+CWMODE=1\r\n", "OK") != ACK_SUCCESS) 
//				{
//					printf("Station mode switch failed\r\n");
//				} 
//			else 
//				{
//					printf("Station mode switch succeeded\r\n");
//				}
//			ESP8266_Reset();
//		} 
//	else 
//		{
//			printf("In station mode.\r\n");
//		}
//  
//  if(ESP8266_Send_Cmd("AT+CWAUTOCONN?\r\n", "+CWAUTOCONN:1") != ACK_SUCCESS) // 0:no 1:auto
//		{ 
//			printf("Auto connection is OFF\r\n");
//			if (ESP8266_Send_Cmd("AT+CWAUTOCONN=1\r\n", "OK") != ACK_SUCCESS) 
//				{
//				printf("Auto connection turn ON failed\r\n");
//				} 
//			else 
//				{
//				printf("Auto connection turn ON succeeded\r\n");
//				}
//		} 
//		else {
//    printf("Auto connection is ON\r\n");
//  }

//  if(ESP8266_Send_Cmd("AT+CWMODE?\r\n", "+CWMODE:1") == ACK_SUCCESS)
//		{
//			return ACK_SUCCESS;
//		} 
//	else 
//		{
//			return ACK_DEFEAT;
//		}
//}

#if 0

uint_t  ESP8266_Set_APmode(char *ap_ssid, char *ap_pwd, char chl, char ecn)
{
		//设置AP模式
}

#endif

///*STA模式下连接网络*/
//uint8_t ESP8266_Connect_AP(const char *ssid, const char *passwd)
//{
//  if(ESP8266_Send_Cmd("AT+CWJAP?\r\n", "+CWJAP:") == ACK_SUCCESS)
//		{
//			return ACK_SUCCESS;
//		}
//  printf("Connecting to AP [%s]\r\n", ssid);

//  char wifi_information[150] = {0};
//  sprintf(wifi_information, "AT+CWJAP=\"%s\",\"%s\"\r\n", ssid, passwd);
//  for(int retries = 0; retries < 5; retries ++ ) 
//		{
//			if(ESP8266_Send_Cmd(wifi_information, "WIFI GOT IP") == ACK_SUCCESS)
//				{
//				printf("AP conntected.\r\n");
//				return ACK_SUCCESS;
//				}
//			else {
//				printf("AP connection failed, retry count: %d\r\n", retries);
//				}
//		}
//  return ACK_DEFEAT;
//}

//设置多路复用模式，0：单连接，1：多连接
uint8_t ESP8266_Set_Link_Mux(uint8_t mode)
{
  char cmd[14] = {0};
  char ack[12] = {0};
  sprintf(cmd, "AT+CIPMUX=%d\r\n", mode);
  sprintf(ack, "+CIPMUX:%d", mode);
  if((ESP8266_Send_Cmd("AT+CIPMUX?\r\n", ack)) == ACK_SUCCESS) 
		{
			return ACK_SUCCESS;
		} 
	else 
		{
			printf("Set link mux.\r\n");
			return ESP8266_Send_Cmd(cmd, "OK");
		}
}

uint8_t ESP8266_Connect_TCP(const char *addr, const char *port)
{
  char tcp_information[64]={0};

  sprintf(tcp_information, "AT+CIPSTART=\"TCP\",\"%s\",%s\r\n", addr, port);
  //  printf("%s\n",tcp_information);
  for(uint8_t retries = 0; retries < 5; retries ++ ) 
	{
    if(ESP8266_Send_Cmd(tcp_information, "CONNECT") == ACK_SUCCESS)
			{
				return ACK_SUCCESS;
			}
  }
  return ACK_DEFEAT;
}

#if 0
	uint8_t ESP8266_Connect_UDP(const char *addr, const char *port)
#endif

/*开启传透模式*/
uint8_t ESP8266_Start_Passthrough(void)
{
  // Ensure IPMODE = 1
  if((ESP8266_Send_Cmd("AT+CIPMODE?\r\n", "+CIPMODE:1")) != ACK_SUCCESS) {
    if (ESP8266_Send_Cmd("AT+CIPMODE=1\r\n", "OK") != ACK_SUCCESS) {
      printf("Set CIPMODE=1 failed\r\n");
      return ACK_DEFEAT;
    }
  } else {
    printf("CIPMODE=1 already set\r\n");
  }
  return ESP8266_Send_Cmd("AT+CIPSEND\r\n", "OK");
}

/*退出传透模式*/
uint8_t ESP8266_Quit_Passthrough(void)
{
  // Return true on any result
  ESP8266_Send_Cmd("+++", "");
	
	vTaskDelay( pdMS_TO_TICKS(40));
	
  return ESP8266_Send_Cmd("AT\r\n","OK");
}

/*以TCP方式建立连接*/
uint8_t ESP8266_Passthrough_Request( const char *addr, char *port, void (*function)())
{
  if (ESP8266_Set_Link_Mux(0) != ACK_SUCCESS) {
    printf("Set MUX:0 failed\r\n");
    return ACK_DEFEAT;
  }

#if 0
  if (type == UDP) {
    if(ESP8266_Connect_UDP(addr,port) != ACK_SUCCESS) {
      printf("UDP connection failed\r\n");
      return ACK_DEFEAT;
    }
  } else if (type == TCP) {
    if(ESP8266_Connect_TCP(addr,port) != ACK_SUCCESS) {
      printf("TCP connection failed\r\n");
      return ACK_DEFEAT;
    }
  } else {
    printf("Unknown connection type\r\n");
    return ACK_DEFEAT;
  }
#endif 

   if(ESP8266_Connect_TCP(addr,port) != ACK_SUCCESS) 
		 {
			 printf("TCP connection failed\r\n");
			 return ACK_DEFEAT;
		 }
	
  ESP8266_Start_Passthrough();
  function();		//连接建立后，执行特定操作
  ESP8266_Quit_Passthrough();

  ESP8266_Send_Cmd("AT+CIPCLOSE\r\v", "CLOSED");  //断开连接
  return ACK_SUCCESS;
}
