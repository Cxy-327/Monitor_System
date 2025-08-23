#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Buffer.h"

#if 0

#define  buffer_size   1024				//定义buffer的size
uint8_t  buffer[buffer_size];
#define  chipdata_size   128				//定义chip的size
uint8_t  chipdata[chipdata_size];

/*根据定义的size和buffer[]进行结构体初始化*/
void Buffer_Init(BufferTypeDef* buff)
{
	buff->buf =buffer;
	buff->size =buffer_size;
  buff->front = 0;
  buff->rear = 0;
}

void Chip_Init(BufferClip* clip)
{
	clip->size = chipdata_size;
	clip->data = chipdata;
	clip->length = 0;
}

#endif


/*重置环形缓冲区*/
//读写指针置零，缓冲区清零
void Buffer_Reset(BufferTypeDef* buff)
{
  buff->front = 0;
  buff->rear = 0;
	
	memset(buff->buf,0,buff->size);
}

void Chip_Reset(BufferClip *chip)
{
	chip->length = 0;
	
	memset (chip->data,0,chip->size );
}

/*计算有效数据长度*/
uint16_t Buffer_Length(BufferTypeDef* buff)
{
  if (buff->rear >= buff->front) {
    return buff->rear - buff->front;	//未溢出
  } else {
    return (buff->size - buff->front) + (buff->rear - 0);	//环形溢出
  }
}

/*压入一个字节的数据*/
uint8_t Buffer_Push(BufferTypeDef* buff, uint8_t data)
{
  buff->buf[(buff->rear)] = data;		//数据写入
  buff->rear++;										//写指针++

  if (buff->rear >= buff->size) {
    buff->rear = 0;								//写指针溢出，回到0
  }
  if (buff->front == buff->rear) {	//缓冲区已满
    buff->front = (buff->front + 1) % buff->size;	//放弃一个有效数据，读指针移至下一个位置
    return NULL;				//指示缓冲区已满
  } else {
    return !NULL;
  }
}

/*弹出一个字节的数据*/
uint8_t Buffer_Pop(BufferTypeDef* buff, uint8_t* data)
{
  if (buff->front == buff->rear) return NULL;		//缓冲区空，返回NULL

  *data = buff->buf[buff->front];		//读出数据
  buff->front = (buff->front + 1) % buff->size;	//读指针下移
  return !NULL;
}

/*弹出所有有效数据，并存放到chip*/		//bug，未考虑chip已满的情况
uint8_t Buffer_Pop_All(BufferTypeDef* buff, BufferClip* clip)
{
  if (buff->front == buff->rear) return NULL;		//缓冲区为空
  
	/*meset用于将一段内存空间设置为指定值*/
  memset(clip->data, 0x00, clip->size * sizeof(uint8_t));	//清空chip
  clip->length = 0;
	
  if (buff->front > buff->rear) {		//环形溢出
    while (buff->front < buff->size && clip->length <= clip->size) {	//溢出部分写入chip
      *(clip->data + clip->length++) = buff->buf[buff->front++];
    }
    if (buff->front == buff->size) {
      buff->front = 0;
    }
  }
  while (buff->front < buff->rear && clip->length < clip->size-1) {	//未溢出部分写入chip
    *(clip->data + clip->length++) = buff->buf[buff->front++];
  }
	
	// 为chip 添加 \0 结尾符
    clip->data[clip->length] = '\0';
	
  return !NULL;
}

/*打印buffer的有效数据*/
void Buffer_Print(BufferTypeDef* buff)
{
//  printf("BUFF:\n front=%03d,rear=%03d",buff->front, buff->rear);
//  if (buff->front == buff->rear) {
//    // print nothing;
//  } else if (buff->front < buff->rear) {
//    for(int i=buff->front; i < buff->rear; i++) {
//      printf("%c", buff->buf[i]);
//    }
//  } else {
//    for(int i = buff->front; i < buff->size; i++) {
//      printf("%c", buff->buf[i]);
//    }
//    for(int i = 0; i < buff->rear; i++) {
//      printf("%c", buff->buf[i]);
//    }
//  }
//  printf("\r\n");
}
/*有效数据、十六进制*/
void Buffer_Print_Hex(BufferTypeDef* buff)
{
//  printf("BUFF:\n front=%03d,rear=%03d",buff->front, buff->rear);
//  if (buff->front == buff->rear) {
//    // print nothing;
//  } else if (buff->front < buff->rear) {
//    for(int i=buff->front; i<buff->rear; i++) {
//      printf("%02X ", buff->buf[i]);
//    }
//  } else {
//    for(int i=buff->front; i < buff->size; i++) {
//      printf("%02X ", buff->buf[i]);
//    }
//    for(int i=0; i<buff->rear; i++) {
//      printf("%02X ", buff->buf[i]);
//    }
//  }
//  printf("\r\n");
}

/*打印全部内容（包括无效数据）*/
void Buffer_Print_All(BufferTypeDef* buff)
{
//  printf("BUFF:\n front=%d,rear=%d",buff->front, buff->rear);
//  for(int i=0; i < buff->size; i++) {
//    printf("%c", buff->buf[i]);
//  }
//  printf("\r\n");
}

void Buffer_Clip_Print(BufferClip* clip)
{
//  printf("CLIP:\n lenth=%03d ", clip->length);
//  for(int i = 0; i < clip->length; i++) {
//    printf("%c", clip->data[i]);
//  }
//  printf("\r\n");
}

void Buffer_Clip_Print_Hex(BufferClip* clip)
{
//  printf("CLIP: %03d ", clip->length);
//  for(int i = 0; i < clip->length; i++) {
//    printf("%02X ", clip->data[i]);
//  }
//  printf("\r\n");
}
