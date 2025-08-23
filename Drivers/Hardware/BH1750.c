#include "MyI2C.h"
#include "Delay.h"

#define BH1750_ADDRESS 0x46

//读写函数
void BH1750_WriteReg(uint8_t Command)
{
	MyI2C_Start();
	MyI2C_SendByte(BH1750_ADDRESS);
	MyI2C_ReceiveAck();
	MyI2C_SendByte(Command);
	MyI2C_ReceiveAck();
	MyI2C_Stop();
}

uint16_t BH1750_ReadReg(void)       //获取16位的数据
{
	uint16_t Data;
    uint8_t temp1;
    uint8_t temp2;
    
	
	MyI2C_Start();
	MyI2C_SendByte(BH1750_ADDRESS | 0x01); //读位
	MyI2C_ReceiveAck();
	temp1= MyI2C_ReceiveByte();   //接收高八位
    MyI2C_SendAck(0);
    temp2= MyI2C_ReceiveByte();
	MyI2C_SendAck(1);                       //非应答，告诉从机停止读
	MyI2C_Stop();
	
    Data=(temp1<<8)+temp2; //2个字节合成数据 
    
	return Data;
}

void BH1750_Init(void)
{    
    BH1750_WriteReg(0x01);  //上电
    BH1750_WriteReg(0x10);  //高分辨率连续测量
    mdelay(200);
}


//获取光照度
float BH1750_ReadLight(void)
{
    float E;
    
    E=BH1750_ReadReg()/1.2;     //计算光照度
    
    return E;
}

