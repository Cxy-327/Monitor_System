#ifndef __BUFFER_H_
#define __BUFFER_H_

#include <stdint.h>

typedef struct
{
  uint8_t* buf;
  uint16_t size;
  uint16_t front;
  uint16_t rear;
} BufferTypeDef;

typedef struct
{
  uint8_t size;
  uint8_t length;
  uint8_t* data;
} BufferClip;


//void Buffer_Init(BufferTypeDef* buff);
//void Chip_Init(BufferClip* clip);
void Buffer_Reset(BufferTypeDef* buff);
void Chip_Reset(BufferClip *chip);
uint16_t  Buffer_Length(BufferTypeDef* buff);
uint8_t   Buffer_Push(BufferTypeDef* buff, uint8_t data);
uint8_t   Buffer_Pop(BufferTypeDef* buff, uint8_t* data);
uint8_t   Buffer_Pop_All(BufferTypeDef* buff, BufferClip* clip);
void Buffer_Print(BufferTypeDef* buff);
void Buffer_Print_Hex(BufferTypeDef* buff);
void Buffer_Print_All(BufferTypeDef* buff);

void Buffer_Clip_Print(BufferClip* clip);
void Buffer_Clip_Print_Hex(BufferClip* clip);

#endif
