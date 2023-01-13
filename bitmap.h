
#ifndef BITMAP_H
#define BITMAP_H

#include <stdint.h>

typedef struct SPOS {
	int16_t x;
	int16_t y;
} sPOS;

typedef struct SBITMAP {
	uint8_t w;
	uint8_t h;
	const uint8_t* map;
} sBITMAP;

typedef struct SRECT {
	int16_t x;
	int16_t y;
	int16_t w;
	int16_t h;
} sRECT;

typedef enum BLENDMODE {
	REPLACE,
	OR,
	ERASE,
	AND,
	NOT,
	XOR,   // 异或
	XNOR,  // 同或
	FILL,  // 填充
	CLEAR, // 清除
} eBlendMode;

#endif
