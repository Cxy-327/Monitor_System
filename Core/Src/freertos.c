/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "queue.h"
#include "semphr.h"
#include "OLED.h"
#include "DHT11.h"
#include "Delay.h"
#include "BH1750.h"
#include "IR_Receiver.h"
#include "Buzzer.h"
#include "Motor.h"
#include "tim.h"
//#include "Uart.h"
#include "Buffer.h"
#include "esp8266.h"
#include "super_PC.h"
#include "pid.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* 获取温度、湿度、光照数据任务的配置 */
#define TASK_GET_SENSOR_DATA_STACK 48
#define TASK_GET_SENSOR_DATA_PRIORITY osPriorityNormal
TaskHandle_t task_get_sensor_data_handle;
void task_get_sensor_data(void *pvParameters);

/* 温度、湿度、光照显示任务的配置 */
#define TASK_OLED_SHOW_STACK 60
#define TASK_OLED_SHOW_PRIORITY osPriorityNormal
TaskHandle_t task_oled_show_handle;
void task_oled_show(void *pvParameters);


/* 阈值设置接收任务的配置 */
#define TASK_THRESHOLDS_STACK 60
#define TASK_THRESHOLDS_PRIORITY osPriorityNormal
TaskHandle_t task_thresholds_handle;
void task_thresholds(void *pvParameters);

/* 自动控制任务的配罿 */
#define TASK_AUTO_CTRL_STACK 80
#define TASK_AUTO_CTRL_PRIORITY osPriorityNormal
TaskHandle_t task_auto_ctrl_handle;
void task_auto_ctrl(void *pvParameters);

/* 数据上传任务的配置 */
#define TASK_ESP8266_STACK 128
#define TASK_ESP8266_PRIORITY osPriorityAboveNormal
TaskHandle_t task_esp8266_handle;
void task_esp8266(void *pvParameters);


#define  SSID     "OPPO_K10_5G"
#define  PASSWD   "19970327"

#define  SERVER_IP    "192.168.113.77"
#define  SERVER_PORT  "8080"  

/*队列、互斥量句柄*/
QueueHandle_t data_queuehandle;
QueueHandle_t oled_mutexhandle;
QueueHandle_t esp8266_mutex_handle;

/*温、湿、光的默认阈值*/
#define  Default_Temp_Thresholds   1000
#define  Default_Hum_Thresholds    1000
#define  Default_Light_Thresholds  1000

/********pid 扩展**********/
extern tPid  pid_MotorSpeed;
extern tPid  pid_LED;


/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
		typedef struct {
					int Temp;
					int Hum;
					int Light;
		}	Sensor_Data;
	
		int		temp ,hum ,light;
		
		Sensor_Data sensor_data_get, sensor_data_show,sensor_data_send;
		
		int		Thresholds_Val =0 ,Thresholds_Type =0 ;
		
		int		temp_T = Default_Temp_Thresholds,
					hum_T = Default_Hum_Thresholds,
					light_T = Default_Light_Thresholds ;				//温、湿、光初始阈值
/* USER CODE END Variables */
osThreadId StartDefaultTasHandle;
osMessageQId myQueueModeHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
////任务相关配置
//	osThreadId get_sensor_dataHandle;
//	void get_sensor_data(void const * argument);

//	osThreadId task_oled_showHandle;
//	void oled_show(void const * argument);
//			
//	osThreadId thresholdsHandle;
//	void thresholds(void const * argument);

//	osThreadId temp_thresholdsHandle;
//	void temp_thresholds(void const * argument);

//	osThreadId hum_thresholdsHandle;
//	void hum_thresholds(void const * argument);

//	osThreadId light_thresholdsHandle;
//	void light_thresholds(void const * argument);

//	osThreadId esp8266Handle;
//	void esp8266(void const * argument);

/* USER CODE END FunctionPrototypes */

void StartDefault(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
	OLED_Init();
	OLED_Clear();
	DHT11_Init();
	IRReceiver_Init();
	BH1750_Init();
	//PID_init();
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
	oled_mutexhandle = xSemaphoreCreateMutex();	//创建互斥量，用于保护OLED资源
	xSemaphoreGive(oled_mutexhandle);						//释放互斥量
	
	esp8266_mutex_handle = xSemaphoreCreateMutex();
	xSemaphoreGive (esp8266_mutex_handle);	//释放
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* definition and creation of myQueueMode */
  osMessageQDef(myQueueMode, 1, uint8_t);
  myQueueModeHandle = osMessageCreate(osMessageQ(myQueueMode), NULL);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
	data_queuehandle = xQueueCreate( 1 , sizeof ( Sensor_Data ));	//创建队列用于温、湿、光数据存储
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of StartDefaultTas */
//  osThreadDef(StartDefaultTas, StartDefault, osPriorityHigh, 0, 128);
//  StartDefaultTasHandle = osThreadCreate(osThread(StartDefaultTas), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
		xTaskCreate((TaskFunction_t)task_get_sensor_data,
                (char *)"task_get_sensor_data",
                (configSTACK_DEPTH_TYPE)TASK_GET_SENSOR_DATA_STACK,
                (void *)NULL,
                (UBaseType_t)TASK_GET_SENSOR_DATA_PRIORITY,
                (TaskHandle_t *)&task_get_sensor_data_handle);	
	
    xTaskCreate((TaskFunction_t)task_oled_show,
                (char *)"task_oled_show",
                (configSTACK_DEPTH_TYPE)TASK_OLED_SHOW_STACK,
                (void *)NULL,
                (UBaseType_t)TASK_OLED_SHOW_PRIORITY,
                (TaskHandle_t *)&task_oled_show_handle);	
	
    xTaskCreate((TaskFunction_t)task_thresholds,
                (char *)"task_thresholds",
                (configSTACK_DEPTH_TYPE)TASK_THRESHOLDS_STACK,
                (void *)NULL,
                (UBaseType_t)TASK_THRESHOLDS_PRIORITY,
                (TaskHandle_t *)&task_thresholds_handle);

    xTaskCreate((TaskFunction_t)task_auto_ctrl,
                (char *)"task_auto_ctrl",
                (configSTACK_DEPTH_TYPE)TASK_AUTO_CTRL_STACK,
                (void *)NULL,
                (UBaseType_t)TASK_AUTO_CTRL_PRIORITY,
                (TaskHandle_t *)&task_auto_ctrl_handle);
																
		xTaskCreate((TaskFunction_t)task_esp8266,
                (char *)"task_esp8266",
                (configSTACK_DEPTH_TYPE)TASK_ESP8266_STACK,
                (void *)NULL,
                (UBaseType_t)TASK_ESP8266_PRIORITY,
                (TaskHandle_t *)&task_esp8266_handle);
				
			
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefault */
/**
  * @brief  Function implementing the StartDefaultTas thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefault */
void StartDefault(void const * argument)
{
  /* USER CODE BEGIN StartDefault */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefault */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void task_get_sensor_data(void *argument)
{
	while(1)
	{
		if (DHT11_Read(&hum, &temp) !=0 )		//数据读取失败，重新初始化
		{
			DHT11_Init();
		}
		else																//数据读取成功,发鿁数捿
		{
			sensor_data_get .Hum = hum ;
			sensor_data_get .Temp = temp ;
			
			xQueueSend ( data_queuehandle, &sensor_data_get ,pdMS_TO_TICKS (100));

		}
		
		light = BH1750_ReadLight();
		
		sensor_data_get .Light = light ;
		
		xQueueSend ( data_queuehandle, &sensor_data_get ,pdMS_TO_TICKS (100) );
		
		osDelay(1000);	//读取周期1s
	}
}

void task_oled_show(void *pvParameters)
{
	int  T ,H, L;
	
	while(1)
	{
		if( xQueueReceive( data_queuehandle, &sensor_data_show ,pdMS_TO_TICKS (100) )== pdPASS)
		{
			T = sensor_data_show.Temp ;
			H = sensor_data_show.Hum ;
			L = sensor_data_show.Light;
			
			
			/*使用OLED资源前，提高互斥量，判断该资源是否可获取*/
			if(xSemaphoreTake( oled_mutexhandle , pdMS_TO_TICKS (1000)) == pdPASS )
			{		
				
				OLED_ShowString(1, 1, "Temp Hum  Light");
				OLED_ShowChar(2, 8, '%');
				OLED_ShowSignedNum(2,1, T,2);
				OLED_ShowNum(2, 6, H,2);
				OLED_ShowNum(2,12,L,3);

				xSemaphoreGive(oled_mutexhandle);
				
			}
						
		}
		osDelay(1000);
	}
}

void task_thresholds(void *pvParameters)
{
		while(1)
		{
				Thresholds_Type = IRReceiver_SetVal (& Thresholds_Val );
			
				if ( Thresholds_Type != 0 )					//阈值设置完成
				{																		//判断阈值类型
					
					if ( Thresholds_Type == 1)				//温度阈值
					{
							temp_T = Thresholds_Val ;
						
							if(xSemaphoreTake( oled_mutexhandle , pdMS_TO_TICKS (1000)) == pdPASS )
							{
								
								OLED_ShowSignedNum(3,1,temp_T,2);
	//							OLED_ShowString(4,1,"                ");
	//							OLED_ShowString(4,1,"TEMP_T:");
	//							OLED_ShowSignedNum(4,8,temp_T,3);
								
									xSemaphoreGive(oled_mutexhandle);
							}

					}
					
					else if ( Thresholds_Type == 2)		//湿度阈值
					{
							hum_T = Thresholds_Val ;
						
							if(xSemaphoreTake( oled_mutexhandle , pdMS_TO_TICKS (1000)) == pdPASS )
							{
							
								OLED_ShowNum(3,6,hum_T,2);
								OLED_ShowString(3,8,"%");
	//							OLED_ShowString(4,1,"                ");
	//							OLED_ShowString(4,1,"HUM_T:");
	//							OLED_ShowNum(4,7,hum_T,3);
	//							OLED_ShowString(4,10,"%");

								xSemaphoreGive(oled_mutexhandle);
							}
						
					}

					else 															//光照阈值 ( Thresholds_Type == 3)
					{
							light_T = Thresholds_Val ;
						
							if(xSemaphoreTake( oled_mutexhandle , pdMS_TO_TICKS (1000)) == pdPASS )
							{
								
									OLED_ShowNum(3,12,light_T,3);
		//							OLED_ShowString(4,1,"                ");
		//							OLED_ShowString(4,1,"LIGHT_T:");
		//							OLED_ShowNum(4,9,light_T,3);
														
									xSemaphoreGive(oled_mutexhandle);
							}						
					}

				}
				
		}
}	

void task_auto_ctrl(void *pvParameters)
{
	while(1)
	{
		vTaskDelay(500);
		
		if(temp_T != Default_Temp_Thresholds)		//温度，非默认阈值
		{
			if( sensor_data_show .Temp  > temp_T )											//温度超过阈值，开启风机
					{
						/********可扩展pid闭环控制*********/
						/*
							int motor_speed = (int)PID_realize(&pid_MotorSpeed,(float)(sensor_data_show .Temp - temp_T));	//通过pid算法调整当前电机速度
							if(motor_speed > 100) motor_speed = 100;
							turn_on_motor(FOREWARD ,motor_speed );
						*/

						turn_on_motor(FOREWARD ,50 );
						
						if(xSemaphoreTake( oled_mutexhandle , pdMS_TO_TICKS (1000)) == pdPASS )
						{
							OLED_ShowString(4,1,"T_O");
							xSemaphoreGive(oled_mutexhandle);
						}
						
					}
					else																	//温度低于阈值，关闭风机
					{
						turn_off_motor();
						
						if(xSemaphoreTake( oled_mutexhandle , pdMS_TO_TICKS (1000)) == pdPASS )
						{
							OLED_ShowString(4,1,"   ");
							xSemaphoreGive(oled_mutexhandle);
						}
						
					}
		}
		
			if ( hum_T  != Default_Hum_Thresholds )	//湿度
			{
				if( sensor_data_show.Hum < hum_T )											//湿度低于阈值，开启水泵
					{
						
						if(xSemaphoreTake( oled_mutexhandle , pdMS_TO_TICKS (1000)) == pdPASS )
						{
							OLED_ShowString(4,6,"H_O");
							xSemaphoreGive(oled_mutexhandle);
						}
						
					}
				else
					{
						
						if(xSemaphoreTake( oled_mutexhandle , pdMS_TO_TICKS (1000)) == pdPASS )
						{
							OLED_ShowString(4,6,"   ");
							xSemaphoreGive(oled_mutexhandle);
						}
						
					}
			}
			
			if ( light_T != Default_Light_Thresholds )		//光照
			{
				if( sensor_data_show .Light  < light_T )											//光照低于阈值，开启补光灯
					{
						/********可扩展pid闭环控制*********/
						/*
							int LED_light = (int)PID_realize(&pid_LED,(float)(light_T - sensor_data_show .Light)); //通过pid算法调整当前补光灯亮度
						if(LED_light > 100) LED_light = 100;																									//补光灯则需改为pwm驱动
							turn_on_LED(LED_light);
						*/
						
						
						
							HAL_GPIO_WritePin (GPIOB ,GPIO_PIN_9,GPIO_PIN_SET );
						
						if(xSemaphoreTake( oled_mutexhandle , pdMS_TO_TICKS (1000)) == pdPASS )
						{
							OLED_ShowString(4,12,"L_O");
							xSemaphoreGive(oled_mutexhandle);
						}
						
					}
				else
					{
							HAL_GPIO_WritePin (GPIOB ,GPIO_PIN_9,GPIO_PIN_RESET );
						
						if(xSemaphoreTake( oled_mutexhandle , pdMS_TO_TICKS (1000)) == pdPASS )
						{
							OLED_ShowString(4,12,"   ");
							xSemaphoreGive(oled_mutexhandle);
						}
						
					}
			}			
			
	}
}


void task_esp8266(void *pvParameters)
{		
//	OLED_ShowString (4,10,"esp");
	char ack[32];		//用于接收应答
	char Sprintf_cmd[64];	//用于处理格式化命令
	char sensor_data[64];	//存放esp8266发送据



////		if(xSemaphoreTake(esp8266_mutex_handle ,portMAX_DELAY) == pdPASS)
////		{
			esp8266_init();	//初始化
//			
//			
			memset(Sprintf_cmd,0,sizeof(Sprintf_cmd));		
			sprintf(Sprintf_cmd,"AT+CIPSTART=\"TCP\",\"%s\",%s",SERVER_IP ,SERVER_PORT);
			ESP8266_Send_Cmd_GetAck(Sprintf_cmd,ack);			//发送TCP连接指令
			if(NULL != strstr(ack,"ALREADY CONNECTED"))
			{
//				printf("already connected");
			}
			else if(NULL != strstr(ack,"CONNECT"))
			{
//				printf("connect");
			}
			else	//未成功连接则进入五次循环连接
			{
				ESP8266_Connect_TCP(SERVER_IP ,SERVER_PORT);
			}
			
			ESP8266_Send_Cmd("AT+CIPMODE=0","OK");//非传透模式
			vTaskDelay(pdMS_TO_TICKS(1000));
			
			while(1)
			{
				if(xQueueReceive( data_queuehandle, &sensor_data_send  ,pdMS_TO_TICKS (100) )== pdPASS)
				{
					memset(sensor_data,0,sizeof(sensor_data));		
					sprintf(sensor_data ,"T:%03d,H:%03d,L:%03d",sensor_data_send.Temp ,sensor_data_send.Hum,sensor_data_send.Light );

					uint8_t data_length = strlen(sensor_data);
					sprintf (Sprintf_cmd,"AT+CIPSEND=%d",data_length);
					ESP8266_Send_Cmd (Sprintf_cmd,"OK");				//开启数据传输
					
					ESP8266_Send_String ((uint8_t *)sensor_data);		//发送数据
					
//					HAL_UART_Transmit(&huart3,( uint8_t *)"send sensordata\n",strlen(( const  char  *)"send sensordata\n"),1000);//1000
					//按数据帧格式，发送传感器数据，用于匿名上位机获取，生成曲线
					ANO_DT_Send_F2((int16_t)sensor_data_send.Temp ,(int16_t)sensor_data_send.Hum ,(int16_t)sensor_data_send.Light ,(int16_t)0);

				}
				
				osDelay(pdMS_TO_TICKS(200));
				
			}

//			
//			
////			xSemaphoreGive(esp8266_mutex_handle);
////		
////		}	

}

/* USER CODE END Application */

