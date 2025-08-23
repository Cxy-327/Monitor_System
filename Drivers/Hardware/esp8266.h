#ifndef __ESP8266_H_
#define __ESP8266_H_

#include <stdint.h>
#include "stm32f1xx_hal.h"
#include "stdio.h"
#include "string.h"
#include "Buffer.h"
//#include "uart.h"
#include "usart.h"
#include "stm32f1xx_hal_uart.h"
#include "FreeRTOS.h"
#include "task.h"

#define ACK_SUCCESS   1
#define ACK_DEFEAT    0

#define ESP_BUF_SIZE  128
extern BufferTypeDef ESP_RX_BUF;
extern uint8_t ESP_RX_BUF_BUFF[ESP_BUF_SIZE];
extern BufferClip ESP_RX_CLIP;
extern uint8_t ESP_RX_CLIP_DATA[ESP_BUF_SIZE];
extern unsigned char rx_byte;

void esp8266_init(void);	//初始化
void ESP8266_Send_Data(uint8_t *data, uint8_t length);
void ESP8266_Send_String(uint8_t* data);
uint8_t ESP8266_Send_Cmd(char *cmd, char *ack);	//发送命令，校验应答
uint8_t ESP8266_Send_Cmd_GetAck(char *cmd, char *ack);//发送命令，获取应答


void ESP8266_Reset(void );
void ESP8266_Set_Echo_Off(void );
uint8_t ESP8266_Set_Stationmode(void);
uint8_t ESP8266_Connect_AP(const char *ssid, const char *passwd);
uint8_t ESP8266_Set_Link_Mux(uint8_t mode);
uint8_t ESP8266_Connect_TCP(const char *addr, const char *port);
uint8_t ESP8266_Start_Passthrough(void);
uint8_t ESP8266_Quit_Passthrough(void);
uint8_t ESP8266_Passthrough_Request( const char *addr, char *port, void (*function)());


#endif
