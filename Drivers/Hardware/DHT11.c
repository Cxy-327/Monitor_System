#include "DHT11.h"
#include "OLED.h"
#include "Delay.h"
#include "stm32f1xx_hal.h"


/* 实现GPIO的基本操作 */

/*****DHT11的数据引脚配置为输出*******/
static void DHT11_PinCfgAsOutput(void)
{
	//已配置为开漏上拉
}

/*****DHT11的数据引脚配置为输入*******/
static void DHT11_PinCfgAsInput(void)
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
	//主机释放总线，DHT11接管控制
}

/******设置DHT11的数据引脚的输出值******/
static void DHT11_PinSet(int val)
{
	if (val)
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
	else
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
}


/*****读取DHT11的数据引脚******/
static int DHT11_PinRead(void)
{
    if (GPIO_PIN_SET == HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1))
		return 1;
	else
		return 0;
}



/* 实现DHT11的读操作 */

/*****给DHT11发出启动信号******/
static void DHT11_Start(void)
{
	DHT11_PinSet(0);
	mdelay(20);
	DHT11_PinCfgAsInput();
}


/*****等待DHT11的回应信号******/
static int DHT11_Wait_Ack(void)
{
	udelay(60);
	return DHT11_PinRead();
}

/*****指定时间内等待数据引脚变为某个值******/
static int DHT11_WaitFor_Val(int val, int timeout_us)
{
	while (timeout_us--)
	{
		if (DHT11_PinRead() == val)
			return 0; /* ok */
		udelay(1);
	}
	return -1; /* err */
}


/******读取DH11 1byte数据******/
static int DHT11_ReadByte(void)
{
	int i;
	int data = 0;
	
	for (i = 0; i < 8; i++)
	{
		if (DHT11_WaitFor_Val(1, 1000))		//等待数据，超过1s则报错
		{
			//printf("dht11 wait for high data err!\n\r");
			return -1;
		}
		
		//0信号保持时间约30ns,1信号保持时间约70ns
		//40ns后，引脚值仍为1则，该信号为1
		udelay(40);
		data <<= 1;
		if (DHT11_PinRead() == 1)
		{
			data |= 1;
		}
		
		if (DHT11_WaitFor_Val(0, 1000))		//bit数据间有50us低电平时隙
		{
			//printf("dht11 wait for low data err!\n\r");
			return -1;
		}
	}
	
	return data;
}


/*主要的调用函数*/

/******DHT11的初始化函数******/
void DHT11_Init(void)
{
	DHT11_PinCfgAsOutput();
	DHT11_PinSet(1);
	//mdelay(2000);
}



/******读取DHT11的温度/湿度*******/
int DHT11_Read(int *hum, int *temp)
{
	unsigned char hum_m, hum_n;
	unsigned char temp_m, temp_n;
	unsigned char check;	
	
	DHT11_Start();		//主机发送开始信号
	
	if (0 != DHT11_Wait_Ack())		//等待DHT11应答
	{
		return -1;
	}

	if (0 != DHT11_WaitFor_Val(1, 1000))  //等待ACK变为高电平, 超时时间是1000us
	{
		return -1;
	}

	if (0 != DHT11_WaitFor_Val(0, 1000))  //数据阶段: 等待低电平, 超时时间是1000us
	{		//printf("dht11 wait for data low err!\n\r");
		return -1;
	}

	//按数据格式读取数据
	hum_m  = DHT11_ReadByte();
	hum_n  = DHT11_ReadByte();
	temp_m = DHT11_ReadByte();
	temp_n = DHT11_ReadByte();
	check  = DHT11_ReadByte();

	DHT11_PinCfgAsOutput();
	DHT11_PinSet(1);

	if (hum_m + hum_n + temp_m + temp_n == check)	//进行校验
	{
		*hum  = hum_m;
		*temp = temp_m;
		return 0;
	}
	else
	{
		return -1;
	}

}


///******DHT11显示（测试）******/
//void DHT11_Test(void)
//{
//	int hum, temp;
//	
//	DHT11_Init();
//	
//	while (1)
//	{
//		if (DHT11_Read(&hum, &temp) !=0 )		//数据读取失败，重新初始化
//		{
//			DHT11_Init();
//		}
//		else																//数据读取成功，进行打印
//		{
//			OLED_ShowString(1, 1, "DHT11:");
//      OLED_ShowString(2, 1, "Humidity Temp");
//      OLED_ShowNum(3, 1, hum,2);
//      OLED_ShowChar(3, 3, '%');
//      OLED_ShowSignedNum(3,10, temp,2);
//		}
//		mdelay(2000);				//读取周期为2s
//	}
//}



