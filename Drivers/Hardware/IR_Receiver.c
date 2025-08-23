#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "IR_Receiver.h"
#include "OLED.h"
#include "Delay.h"
#include "stm32f1xx_hal.h"
#include <math.h>
#include "Buzzer.h"
#include "usart.h"
#include "string.h"

/* 环形缓冲区: 用来保存解析出来的按键,可以防止丢失 */
#define BUF_LEN 128
static unsigned char g_KeysBuf[BUF_LEN];
static int g_KeysBuf_R, g_KeysBuf_W;

static uint64_t g_IRReceiverIRQ_Timers[68];
static int g_IRReceiverIRQ_Cnt = 0;

#define NEXT_POS(x) ((x+1) % BUF_LEN)

/* 辅助函数 */

/******环形缓冲区是否空******/
static int isKeysBufEmpty(void)
{
	return (g_KeysBuf_R == g_KeysBuf_W);
}

/******环形缓冲区是否满******/
static int isKeysBufFull(void)
{
	return (g_KeysBuf_R == NEXT_POS(g_KeysBuf_W));
}

/******按键数据写入环形缓冲区******/
static void PutKeyToBuf(unsigned char key)
{
	if (!isKeysBufFull())
	{
		g_KeysBuf[g_KeysBuf_W] = key;
		g_KeysBuf_W = NEXT_POS(g_KeysBuf_W);
	}
}

/******从环形缓冲区读取按键数据******/
static unsigned char GetKeyFromBuf(void)
{
	unsigned char key = 0xff;
	if (!isKeysBufEmpty())
	{
		key = g_KeysBuf[g_KeysBuf_R];
		g_KeysBuf_R = NEXT_POS(g_KeysBuf_R);
	}
	return key;
}

/******解析中断回调函数里记录的时间序列,得到的device和key放入环形缓冲区******/
static int IRReceiver_IRQTimes_Parse(void)
{
	uint64_t time;
	int i;
	int m, n;
	unsigned char datas[4];
	unsigned char data = 0;
	int bits = 0;
	int byte = 0;

	/* 1. 判断前导码 : 9ms的低脉冲, 4.5ms高脉冲  */
	time = g_IRReceiverIRQ_Timers[1] - g_IRReceiverIRQ_Timers[0];
	if (time < 8000000 || time > 10000000)
	{
		return -1;
	}

	time = g_IRReceiverIRQ_Timers[2] - g_IRReceiverIRQ_Timers[1];
	if (time < 3500000 || time > 55000000)
	{
		return -1;
	}

	/* 2. 解析数据 */
	for (i = 0; i < 32; i++)
	{
		m = 3 + i*2;
		n = m+1;
		time = g_IRReceiverIRQ_Timers[n] - g_IRReceiverIRQ_Timers[m];
		data <<= 1;
		bits++;
		if (time > 1000000)
		{
			/* 得到了数据1 */
			data |= 1;
		}

		if (bits == 8)
		{
			datas[byte] = data;
			byte++;
			data = 0;
			bits = 0;
		}
	}

	/* 判断数据正误 */
	datas[1] = ~datas[1];
	datas[3] = ~datas[3];
	
	if ((datas[0] != datas[1]) || (datas[2] != datas[3]))
	{
        g_IRReceiverIRQ_Cnt = 0;
        return -1;
	}

	PutKeyToBuf(datas[0]);
	PutKeyToBuf(datas[2]);
    return 0;
}


/****** 解析中断回调函数里记录的时间序列,判断是否重复码******/
static int isRepeatedKey(void)
{
	uint64_t time;

	/* 1. 判断重复码 : 9ms的低脉冲, 2.25ms高脉冲  */
	time = g_IRReceiverIRQ_Timers[1] - g_IRReceiverIRQ_Timers[0];
	if (time < 8000000 || time > 10000000)
	{
		return 0;
	}

	time = g_IRReceiverIRQ_Timers[2] - g_IRReceiverIRQ_Timers[1];
	if (time < 2000000 || time > 2500000)
	{
		return 0;
	}	

	return 1;
}


/****** 红外接收器的中断回调函数,记录中断时刻******/
void IRReceiver_IRQ_Callback(void)
{
    uint64_t time;
    static uint64_t pre_time = 0;

        
	/* 1. 记录中断发生的时刻 */	
	time = system_get_ns();
    
    /* 一次按键的最长数据 = 引导码 + 32个数据"1" = 9+4.5+2.25*32 = 85.5ms
     * 如果当前中断的时刻, 举例上次中断的时刻超过这个时间, 以前的数据就抛弃
     */
    if (time - pre_time > 100000000) 
    {
        g_IRReceiverIRQ_Cnt = 0;
    }
    pre_time = time;
    
	g_IRReceiverIRQ_Timers[g_IRReceiverIRQ_Cnt] = time;

	/* 2. 累计中断次数 */
	g_IRReceiverIRQ_Cnt++;

	/* 3. 次数达标后, 解析数据, 放入buffer */
	if (g_IRReceiverIRQ_Cnt == 4)
	{
		/* 是否重复码 */
		if (isRepeatedKey())
		{
			/* device: 0, val: 0, 表示重复码 */
//			PutKeyToBuf(0);
//			PutKeyToBuf(0);
			g_IRReceiverIRQ_Cnt = 0;
		}
	}
	if (g_IRReceiverIRQ_Cnt == 68)
	{
		IRReceiver_IRQTimes_Parse();
		g_IRReceiverIRQ_Cnt = 0;
	}
}


/*****外部中断的中断回调函数*******/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin==GPIO_PIN_12)
	{
		IRReceiver_IRQ_Callback();
	}
}


/******  红外接收器的初始化函数******/
void IRReceiver_Init(void)
{
    /* 在MX_GPIO_Init()中已将PA10配置为双边沿触发, 并使能了中断 */
}


/******  红外接收器的读取函数******/
int IRReceiver_Read(uint8_t *pDev, uint8_t *pData)
{
    if (isKeysBufEmpty())
        return -1;
    
    *pDev  = GetKeyFromBuf();
    *pData = GetKeyFromBuf();
    return 0;
}


/******  把接收到的按键码转换为按键名字******/
const char *IRReceiver_CodeToString(uint8_t code)
{
    const uint8_t codes[]= {0xa2, 0xe2, 0x22, 0x02, 0xc2, 0xe0, 0xa8, 0x90, \
                            0x68, 0x98, 0xb0, 0x30, 0x18, 0x7a, 0x10, 0x38, \
                            0x5a, 0x42, 0x4a, 0x52, 0x00};
    const char *names[]= {"Power", "Menu", "Test", "+", "Return", "Left", "Play", "Right", \
                            "0", "-", "C", "1", "2", "3", "4", "5", \
                            "6", "7", "8", "9", "Repeat"};
    int i;
    
    for (i = 0; i < sizeof(codes)/sizeof(codes[0]); i++)
    {
        if (code == codes[i])
        {
            return names[i];
        }
    }
    return "Error";
}


///*****红外接收器测试程序******/
//void IRReceiver_Test(void)
//{
//    uint8_t dev, data;
//	
//    IRReceiver_Init();

//    while (1)
//    {
//        OLED_ShowString(1, 1, "IR Receiver: ");        
//        OLED_ShowString(2, 1, "Device  Data");

//        if (!IRReceiver_Read(&dev, &data))
//        {
//            OLED_ShowString(3, 1, "                ");
//            OLED_ShowHexNum(3, 1, dev, 2);
//            OLED_ShowHexNum(3, 8, data, 2);
//            OLED_ShowString(4, 1, "                ");
//            OLED_ShowString(4, 1, "Key name: ");
//            OLED_ShowString(4, 10, (char*)IRReceiver_CodeToString(data));
////						mdelay (1000);
////						OLED_Clear ();
//        }
//    }
//}

/*****红外接收器设值******/
int  IRReceiver_SetVal (int * Thresholds_Val )
{
	uint8_t dev=0, data=0;
	int i; 
	int value=0;
	const char* Key;
		
	IRReceiver_Init();
	
	while(1)
	{
				/****** "Power" , 接入设值 ******/
		osDelay(20);
		if(!IRReceiver_Read(&dev, &data) && data == 0xa2)
		{
				osDelay(10);
				Buzzer_200ms();
//				Key = IRReceiver_CodeToString(data);
//				OLED_Clear ();
//				OLED_ShowString(1,1,"Key:");
//				OLED_ShowString(1,6,(char*)Key);
//				OLED_ShowHexNum(4, 1, data, 2);
			
				/*****  需完成三次读值  *****/
				for(i=0 ;i<3 ;i++ )
					{			
						osDelay(20);
						while(IRReceiver_Read(&dev, &data)!= 0)    //直到再次有数据可以读取，才跳出循环

						osDelay(10);
						
						Key = IRReceiver_CodeToString(data);													//得到key
						value  = value  + (*Key - '0')* (int) pow (10,( 2 -i ));			//得到数值						taskENTER_CRITICAL();
						Buzzer_200ms();
//						OLED_Clear ();																				//进行显示
//						OLED_ShowString(1,1,"Key:");
//						OLED_ShowString(1,6,(char*)Key);
//						OLED_ShowString(2,1,"key-'0'");
//						OLED_ShowNum(2,9,(*Key - '0') ,3);
//						OLED_ShowNum(3,1,(int)pow(10,(2-i)) ,3);
//						OLED_ShowString(4,1,"value:");
//						OLED_ShowNum(4,7,value  ,3);
						
						if(i == 2 )
						{
							*Thresholds_Val = value ;				//阈值输入完成，返回数值
						}
						
					}

		/****** 判断 “温度” “湿度” “光照” *******/
			osDelay(20);
			while(IRReceiver_Read(&dev, &data)!= 0)
			
			osDelay(10);
			Buzzer_200ms();
			
			switch (data  )
			{
				case  0x22	:				//"Test" -> 温度
					return 1;
				
				case 	0x02	:				//"+" -> 湿度
					return 2;
				
				case	0xc2	:				//"Return" -> 光照
					return 3;
				
				default :
					return 0;
			}
				
       /*****  "Play”，完成，返回设值  ******/					
//				while(IRReceiver_Read(&dev, &data)!= 0 && data != 0xa8)		//“Play”，完成，返回设值
//					
//				mdelay(10);
//				Buzzer_100ms();
//				OLED_Clear ();
//				OLED_ShowString(1,1,"Play");
//				OLED_ShowString(2,1,"Val:");
//				OLED_ShowNum(2,6,Val ,3);
//				OLED_ShowHexNum(4, 1, data, 2);
//				return Val;
		}
	}
}
