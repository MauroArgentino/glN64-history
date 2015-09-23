#ifndef TEXTURES_H
#define TEXTURES_H

#include <windows.h>
#include <gl/gl.h>
#include "convert.h"
#include "RDP.h"

struct CachedTexture
{
	GLuint	glName;
	DWORD	address;
	DWORD	crc;
	float	fulS, fulT;
	WORD	ulS, ulT, lrS, lrT;
	BYTE	maskS, maskT;
	BOOL	clampS, clampT;
	BOOL	mirrorS, mirrorT;
	WORD	line;
	WORD	size;
	WORD	format;
	WORD	tMem;
	BYTE	palette;
	WORD	width, height;			// N64 width and height
	WORD	clampWidth, clampHeight;// Size to clamp to
	WORD	realWidth, realHeight;	// Actual texture size
	FLOAT	scaleS, scaleT;			// Scale to map to 0.0-1.0 and scale from shift
	FLOAT	offsetScaleS, offsetScaleT;// Scale to map to 0.0-1.0
	DWORD	textureBytes;

	CachedTexture	*lower, *higher;
	DWORD	lastDList;
};


typedef struct
{
	CachedTexture	*bottom, *top;

	CachedTexture	*(current[2]);
	DWORD			maxBytes;
	DWORD			cachedBytes;
	WORD			numCached;
	DWORD			hits, misses;
	GLuint			glNoiseNames[32];
	BOOL			enable2xSaI;
} TextureCache;

extern TextureCache cache;

inline DWORD GetNone( DWORD64 *src, WORD x, WORD i, BYTE palette )
{
	return 0x00000000;
}

inline DWORD Get4b_CI( DWORD64 *src, WORD x, WORD i, BYTE palette )
{
	BYTE color4B;

	color4B = ((BYTE*)src)[(x>>1)^(i<<1)];

	if (x & 1)
		return RDP.palette[(palette << 4) + (color4B & 0x0F)];
	else
		return RDP.palette[(palette << 4) + (color4B >> 4)];
}

inline DWORD Get4b_IA( DWORD64 *src, WORD x, WORD i, BYTE palette )
{
	BYTE color4B;

	color4B = ((BYTE*)src)[(x>>1)^(i<<1)];

	return IA31_ABGR8888( (x & 1) ? (color4B & 0x0F) : (color4B >> 4) );
}

inline DWORD Get4b_I( DWORD64 *src, WORD x, WORD i, BYTE palette )
{
	BYTE color4B;

	color4B = ((BYTE*)src)[(x>>1)^(i<<1)];

	return I4_ABGR8888( (x & 1) ? (color4B & 0x0F) : (color4B >> 4) );
}

inline DWORD Get8b_CI( DWORD64 *src, WORD x, WORD i, BYTE palette )
{
	return RDP.palette[((BYTE*)src)[x^(i<<1)]];
}

inline DWORD Get8b_IA( DWORD64 *src, WORD x, WORD i, BYTE palette )
{
	return IA44_ABGR8888(((BYTE*)src)[x^(i<<1)]);
}

inline DWORD Get8b_I( DWORD64 *src, WORD x, WORD i, BYTE palette )
{
	return I8_ABGR8888(((BYTE*)src)[x^(i<<1)]);
}

inline DWORD Get16b_RGBA( DWORD64 *src, WORD x, WORD i, BYTE palette )
{
	return RGBA5551_ABGR8888(swapword(((WORD*)src)[x^i]));
}

inline DWORD Get16b_IA( DWORD64 *src, WORD x, WORD i, BYTE palette )
{
	return IA88_ABGR8888(((WORD*)src)[x^i]);
}

inline DWORD Get32b_RGBA( DWORD64 *src, WORD x, WORD i, BYTE palette )
{
	return ((DWORD*)src)[x^i];
}

inline WORD pow2( WORD dim )
{
	WORD i = 1;

	while (i < dim) i <<= 1;

	return i;
}

void TextureCache_Init();
void TextureCache_Destroy();
void TextureCache_ActivateTexture( BYTE t );
void TextureCache_ActivateNoise( BYTE t );
BOOL TextureCache_Verify();

#endif