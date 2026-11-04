#ifndef __FS_CRC_H__
#define __FS_CRC_H__
#include <stdint.h>

typedef unsigned char   UCHAR;
typedef unsigned int        USHORT;

USHORT  usMBCRC16( UCHAR * pucFrame, USHORT usLen );
uint8_t crc8(uint8_t *data, uint8_t length);
#endif
