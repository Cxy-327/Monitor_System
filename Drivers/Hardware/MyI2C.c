#include "gpio.h"
#include "Delay.h"


/*封装SCL、SDA的读写函数*/

void MyI2C_W_SCL(uint8_t BitValue)
{
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_3,(GPIO_PinState)BitValue);
	udelay(10);
}

void MyI2C_W_SDA(uint8_t BitValue)
{
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_4,(GPIO_PinState)BitValue);
	udelay(10);
}

uint8_t MyI2C_R_SDA(void)
{
	uint8_t BitValue;
	BitValue = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_4);
	udelay(10);
	return BitValue;
}

/*I2C的六个基本时序*/

void MyI2C_Start(void)  //起始条件，SCL=1时，SDA置0
{
	MyI2C_W_SDA(1);
	MyI2C_W_SCL(1);
	MyI2C_W_SDA(0);
	MyI2C_W_SCL(0);
}

void MyI2C_Stop(void)   //终止条件，SCL=1时，SDA置1
{
	MyI2C_W_SDA(0);
	MyI2C_W_SCL(1);
	MyI2C_W_SDA(1);
}

void MyI2C_SendByte(uint8_t Byte)   //发送一个字节
{
	uint8_t i;
	for (i = 0; i < 8; i ++)
	{                                       //&操作，取对应位的数据
		MyI2C_W_SDA(Byte & (0x80 >> i));    //先填入数据
		MyI2C_W_SCL(1);                     //SCL=1时，完成写操作
		MyI2C_W_SCL(0);
	}
}

uint8_t MyI2C_ReceiveByte(void)     //接收一个字节
{
	uint8_t i, Byte = 0x00;
	MyI2C_W_SDA(1);                         //主机释放SDA，从机控制SDA
	for (i = 0; i < 8; i ++)
	{                                       //从机控制SDA，填入数据
		MyI2C_W_SCL(1);                     //SCL=1时，主机完成读操作
		if (MyI2C_R_SDA() == 1){Byte |= (0x80 >> i);} //所得数据填入Byte
		MyI2C_W_SCL(0);
	}
	return Byte;
}

    /*0为应答，1为非应答*/
void MyI2C_SendAck(uint8_t AckBit)  //发送应答
{
	MyI2C_W_SDA(AckBit);
	MyI2C_W_SCL(1);
	MyI2C_W_SCL(0);
}

uint8_t MyI2C_ReceiveAck(void)      //接收应答
{
	uint8_t AckBit;
	MyI2C_W_SDA(1);         //主机释放SDA，从机发送应答
	MyI2C_W_SCL(1);
	AckBit = MyI2C_R_SDA();
	MyI2C_W_SCL(0);
	return AckBit;
}

