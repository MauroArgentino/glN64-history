#include <windows.h>
#include <memory.h>
#include <GL/gl.h>
#include "glext.h"
#include "Textures.h"
#include "RDP.h"
#include "RSP.h"
#include "N64.h"
#include "CRC.h"
#include "convert.h"
#include "2xSAI.h"

TextureCache	cache;

typedef DWORD (*GetFormatFunc)( DWORD64 *src, WORD x, WORD i, BYTE palette );

/*GetFormatFunc GetFormat[4][5] =
{
	{ Get4B_CI, GetNone, Get4B_CI, Get4B_IA, Get4B_I },
	{ GetNone, GetNone, Get8B_CI, Get8B_IA, Get8B_I },
	{ Get16B_RGBA, GetNone, GetNone, Get16B_IA, GetNone },
	{ Get32B_RGBA, GetNone, GetNone, GetNone, GetNone }
};*/

const struct
{
	GetFormatFunc	GetTexelFunc;
	WORD lineShift, maxTexels;
} imageFormat[4][5] =
{
	{ // 4-bit
		{ Get4b_CI, 4, 4096 }, // RGBA (Banjo-Kazooie uses this, doesn't make sense, but it works...)
		{ GetNone, 0, 0 }, // YUV
		{ Get4b_CI, 4, 4096 }, // CI
		{ Get4b_IA, 4, 8192 }, // IA
		{ Get4b_I, 4, 8192 }, // I
	},
	{ // 8-bit
		{ GetNone, 0, 0 }, // RGBA
		{ GetNone, 0, 0 }, // YUV
		{ Get8b_CI, 3, 2048 }, // CI
		{ Get8b_IA, 3, 4096 }, // IA
		{ Get8b_I, 3, 4096 }, // I
	},
	{ // 16-bit
		{ Get16b_RGBA, 2, 2048 }, // RGBA
		{ GetNone, 2, 2048 }, // YUV
		{ GetNone, 0, 0 }, // CI
		{ Get16b_IA, 2, 2048 }, // IA
		{ GetNone, 0, 0 }, // I
	},
	{ // 32-bit
		{ Get32b_RGBA, 2, 1024 }, // RGBA
		{ GetNone, 0, 0 }, // YUV
		{ GetNone, 0, 0 }, // CI
		{ GetNone, 0, 0 }, // IA
		{ GetNone, 0, 0 }, // I
	}
};

void TextureCache_Init()
{
	cache.current[0] = NULL;
	cache.current[1] = NULL;
	cache.top = NULL;
	cache.bottom = NULL;
	cache.numCached = 0;
	cache.cachedBytes = 0;
	cache.enable2xSaI = OGL.enable2xSaI;

	glGenTextures( 32, cache.glNoiseNames );

	BYTE noise[64*64*4];
	for (int i = 0; i < 32; i++)
	{
		glBindTexture( GL_TEXTURE_2D, cache.glNoiseNames[i] );

		srand( timeGetTime() );

		for (int y = 0; y < 64; y++)
		{
			for (int x = 0; x < 64; x++)
			{
				BYTE random = rand();
				noise[y*64*4+x*4] = random;
				noise[y*64*4+x*4+1] = random;
				noise[y*64*4+x*4+2] = random;
				noise[y*64*4+x*4+3] = random;
			}
		}
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, 64, 64, 0, GL_RGBA, GL_UNSIGNED_BYTE, noise );
	}

	CRC_BuildTable();
}

BOOL TextureCache_Verify()
{
	int i = 0;
	CachedTexture *current;
	
	current = cache.top;

	while (current)
	{
		i++;
		current = current->lower;
	}
	if (i != cache.numCached) return FALSE;

	current = cache.bottom;
	while (current)
	{
		i++;
		current = current->higher;
	}
	if (i != cache.numCached) return FALSE;

	return TRUE;
}

void TextureCache_RemoveBottom()
{
	CachedTexture *newBottom = cache.bottom->higher;

	glDeleteTextures( 1, &cache.bottom->glName );
	cache.cachedBytes -= cache.bottom->textureBytes;
	free( cache.bottom );

    cache.bottom = newBottom;
	
	if (cache.bottom)
		cache.bottom->lower = NULL;

	cache.numCached--;
}

CachedTexture *TextureCache_AddTop()
{
	while (cache.cachedBytes > cache.maxBytes)
		TextureCache_RemoveBottom();

	CachedTexture *newtop = (CachedTexture*)malloc( sizeof( CachedTexture ) );

	glGenTextures( 1, &newtop->glName );

	newtop->lower = cache.top;
	newtop->higher = NULL;

	if (cache.top)
		cache.top->higher = newtop;

	if (!cache.bottom)
		cache.bottom = newtop;

    cache.top = newtop;

	cache.numCached++;

	return newtop;
}

void TextureCache_MoveToTop( CachedTexture *newtop )
{
	if (newtop == cache.top) return;

	if (newtop == cache.bottom)
	{
		cache.bottom = newtop->higher;
		cache.bottom->lower = NULL;
	}
	else
	{
		newtop->higher->lower = newtop->lower;
		newtop->lower->higher = newtop->higher;
	}

	newtop->higher = NULL;
	newtop->lower = cache.top;
	cache.top->higher = newtop;
	cache.top = newtop;
}

void TextureCache_Destroy()
{
	while (cache.bottom)
		TextureCache_RemoveBottom();
}

void TextureCache_Load( CachedTexture *texInfo )
{
	DWORD *dest, *scaledDest;

	DWORD64 *src;
	WORD x, y, i, j, tx, ty, s, t, line;
	DWORD color;
	WORD mirrorSBit, maskSMask, clampSClamp;
	WORD mirrorTBit, maskTMask, clampTClamp;
	GetFormatFunc	GetTexel;

 	texInfo->textureBytes = (texInfo->realWidth * texInfo->realHeight) << 2;

	dest = (DWORD*)malloc( texInfo->textureBytes );

	GetTexel = imageFormat[texInfo->size][texInfo->format].GetTexelFunc;

	line = texInfo->line;

	if (texInfo->size == IMAGE_SIZE_32b)
		line <<= 1;

	if (texInfo->maskS)
	{
		maskSMask = (1 << texInfo->maskS) - 1;
		if (texInfo->mirrorS)
			mirrorSBit = maskSMask + 1;
		else
			mirrorSBit = 0;
	}
	else
	{
		mirrorSBit = 0x0000;
		maskSMask = 0xFFFF;
	}

	if (texInfo->maskT)
	{
		maskTMask = (1 << texInfo->maskT) - 1;
		if (texInfo->mirrorT)
			mirrorTBit = maskTMask + 1;
		else
			mirrorTBit = 0;
	}
	else
	{
		mirrorTBit = 0x0000;
		maskTMask = 0xFFFF;
	}

	if (texInfo->clampS)
		clampSClamp = texInfo->lrS - texInfo->ulS;
	else
		clampSClamp = 0xFFFF;

	if (texInfo->clampT)
		clampTClamp = texInfo->lrT - texInfo->ulT;
	else
		clampTClamp = 0xFFFF;

	j = 0;
	for (y = 0; y < texInfo->realHeight; y++)
	{
		ty = min(y, clampTClamp) & maskTMask;

		if (y & mirrorTBit)
			ty ^= maskTMask;

		src = &RDP.tMem[texInfo->tMem] + line * ty;

		i = (ty & 1) << 1;
		for (x = 0; x < texInfo->realWidth; x++)
		{
			tx = min(x, clampSClamp) & maskSMask;

			if (x & mirrorSBit)
				tx ^= maskSMask;

			dest[j++] = GetTexel( src, tx, i, texInfo->palette );
		}
	}

	if (cache.enable2xSaI)
	{
		texInfo->textureBytes <<= 2;

		scaledDest = (DWORD*)malloc( texInfo->textureBytes );

		_2xSaI( dest, scaledDest, texInfo->realWidth, texInfo->realHeight, texInfo->clampS, texInfo->clampT );

		// Send texture to OpenGL
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, texInfo->realWidth << 1, texInfo->realHeight << 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, scaledDest );

		free( dest );
		free( scaledDest );
	}
	else
	{
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, texInfo->realWidth, texInfo->realHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, dest );

		free( dest );
	}
}

DWORD TextureCache_CalculateCRC( BYTE t, WORD width, WORD height )
{
	DWORD crc;
	DWORD y, i, bpl, lineBytes, line;
	DWORD64 *src;

	src = (DWORD64*)&RDP.tMem[RSP.textureTile[t]->tMem];
	bpl = width << RSP.textureTile[t]->size >> 1;
	lineBytes = RSP.textureTile[t]->line << 3;

	line = RSP.textureTile[t]->line;
 	if (RSP.textureTile[t]->size == IMAGE_SIZE_32b)
		line <<= 1;

	crc = 0xFFFFFFFF;
 	for (y = 0; y < height; y++)
	{
		crc = CRC_Calculate( crc, src, bpl );

		src += line;
	}

   	if (RSP.textureTile[t]->format == IMAGE_FORMAT_CI)
	{
		if (RSP.textureTile[t]->size == IMAGE_SIZE_4b)
			crc = CRC_Calculate( crc, &RDP.palette[RSP.textureTile[t]->palette << 4], 64 );
		else if (RSP.textureTile[t]->size == IMAGE_SIZE_8b)
			crc = CRC_Calculate( crc, &RDP.palette[0], 1024 );
	}
	return crc;
}

void TextureCache_ActivateTexture( BYTE t )
{
	if (cache.enable2xSaI != OGL.enable2xSaI)
	{
		TextureCache_Destroy();
		TextureCache_Init();
	}

	int i, j, k;
	CachedTexture *current;
	DWORD crc;
	BYTE *source = &RDRAM[RDP.textureImage.address];
	DWORD64 *src;
	WORD bpl, cacheNum, maxTexels;
	WORD tileWidth, maskWidth, loadWidth, lineWidth, clampWidth, height;
	WORD tileHeight, maskHeight, loadHeight, lineHeight, clampHeight, width;

	maxTexels = imageFormat[RSP.textureTile[t]->size][RSP.textureTile[t]->format].maxTexels;

	tileWidth = RSP.textureTile[t]->lrS - RSP.textureTile[t]->ulS + 1;
	tileHeight = RSP.textureTile[t]->lrT - RSP.textureTile[t]->ulT + 1;

	maskWidth = 1 << RSP.textureTile[t]->maskS;
	maskHeight = 1 << RSP.textureTile[t]->maskT;

	loadWidth = RDP.loadTile->lrS - RDP.loadTile->ulS + 1;
	loadHeight = RDP.loadTile->lrT - RDP.loadTile->ulT + 1;

	lineWidth = RSP.textureTile[t]->line << imageFormat[RSP.textureTile[t]->size][RSP.textureTile[t]->format].lineShift;

	if (lineWidth) // Don't allow division by zero
		lineHeight = min( min( maxTexels / lineWidth, tileHeight ), maskHeight );
	else
		lineHeight = 0;

	if (RSP.textureTile[t]->maskS && ((maskWidth * maskHeight) <= maxTexels))
		width = maskWidth; // Use mask width if set and valid
	else if ((tileWidth * tileHeight) <= maxTexels)
		width = tileWidth; // else use tile width if valid
	else if (RDP.loadTile->loadType == LOADTYPE_TILE)
		width = loadWidth; // else use load width if load done with LoadTile
	else
		width = lineWidth; // else use line-based width

	if (RSP.textureTile[t]->maskT && ((maskWidth * maskHeight) <= maxTexels))
		height = maskHeight;
	else if ((tileWidth * tileHeight) <= maxTexels)
		height = tileHeight;
	else if (RDP.loadTile->loadType == LOADTYPE_TILE)
		height = loadHeight;
	else
		height = lineHeight;

	clampWidth = RSP.textureTile[t]->clampS ? tileWidth : width;
	clampHeight = RSP.textureTile[t]->clampT ? tileHeight : height;

	crc = TextureCache_CalculateCRC( t, width, height );

	current = cache.top;
	
 	while (current)
  	{
		if ((current->crc == crc) &&
			(current->address == RDP.textureImage.address) &&
//			(current->palette == RSP.textureTile[t]->palette) &&
			(current->width == width) &&
			(current->height == height) &&
			(current->clampWidth == clampWidth) &&
			(current->clampHeight == clampHeight) &&
			(current->maskS == RSP.textureTile[t]->maskS) &&
			(current->maskT == RSP.textureTile[t]->maskT) &&
			(current->mirrorS == RSP.textureTile[t]->mirrorS) &&
			(current->mirrorT == RSP.textureTile[t]->mirrorT) &&
			(current->clampS == RSP.textureTile[t]->clampS) &&
			(current->clampT == RSP.textureTile[t]->clampT) &&
//			(current->tMem == RSP.textureTile[t]->tMem) &&
/*			(current->ulS == RSP.textureTile[t]->ulS) &&
			(current->ulT == RSP.textureTile[t]->ulT) &&
			(current->lrS == RSP.textureTile[t]->lrS) &&
			(current->lrT == RSP.textureTile[t]->lrT) &&*/
			(current->format == RSP.textureTile[t]->format) &&
			(current->size == RSP.textureTile[t]->size))
		{
			cache.current[t] = current;
			TextureCache_MoveToTop( current );

			// If multitexturing, set the appropriate texture
			if (OGL.ARB_multitexture)
				glActiveTextureARB( GL_TEXTURE0_ARB + t );

			// Bind the cached texture
			glBindTexture( GL_TEXTURE_2D, cache.current[t]->glName );

			// Set filter mode. Almost always bilinear, but check anyways
			if ((RDP.otherMode_H & TEXTFILT_BILERP) || (RDP.otherMode_H & TEXTFILT_AVERAGE) || (OGL.forceBilinear))
			{
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
			}
			else
			{
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
			}

			// Set clamping modes
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, cache.current[t]->clampS ? GL_CLAMP : GL_REPEAT );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, cache.current[t]->clampT ? GL_CLAMP : GL_REPEAT );

			cache.current[t]->lastDList = RSP.DList;
			cache.hits++;
			return;
		}

		current = current->lower;
	}

	cache.misses++;

	// If multitexturing, set the appropriate texture
	if (OGL.ARB_multitexture)
		glActiveTextureARB( GL_TEXTURE0_ARB + t );

	cache.current[t] = TextureCache_AddTop();

	glBindTexture( GL_TEXTURE_2D, cache.current[t]->glName );

	cache.current[t]->address = RDP.textureImage.address;
	cache.current[t]->crc = crc;

	cache.current[t]->format = RSP.textureTile[t]->format;
	cache.current[t]->size = RSP.textureTile[t]->size;

	cache.current[t]->width = width;
	cache.current[t]->height = height;

	cache.current[t]->clampWidth = clampWidth;
	cache.current[t]->clampHeight = clampHeight;

	cache.current[t]->palette = RSP.textureTile[t]->palette;
	cache.current[t]->fulS = RSP.textureTile[t]->fulS;
	cache.current[t]->fulT = RSP.textureTile[t]->fulT;
	cache.current[t]->ulS = RSP.textureTile[t]->ulS;
	cache.current[t]->ulT = RSP.textureTile[t]->ulT;
	cache.current[t]->lrS = RSP.textureTile[t]->lrS;
	cache.current[t]->lrT = RSP.textureTile[t]->lrT;
	cache.current[t]->maskS = RSP.textureTile[t]->maskS;
	cache.current[t]->maskT = RSP.textureTile[t]->maskT;
	cache.current[t]->mirrorS = RSP.textureTile[t]->mirrorS;
	cache.current[t]->mirrorT = RSP.textureTile[t]->mirrorT;
 	cache.current[t]->clampS = RSP.textureTile[t]->clampS;
	cache.current[t]->clampT = RSP.textureTile[t]->clampT;
	cache.current[t]->line = RSP.textureTile[t]->line;
	cache.current[t]->tMem = RSP.textureTile[t]->tMem;
	cache.current[t]->lastDList = RSP.DList;

	if (cache.current[t]->clampS)
		cache.current[t]->realWidth = pow2( clampWidth );
	else if (cache.current[t]->mirrorS)
		cache.current[t]->realWidth = maskWidth << 1;
	else
		cache.current[t]->realWidth = pow2( width );

	if (cache.current[t]->clampT)
		cache.current[t]->realHeight = pow2( clampHeight );
	else if (cache.current[t]->mirrorT)
		cache.current[t]->realHeight = maskHeight << 1;
	else
		cache.current[t]->realHeight = pow2( height );

	cache.current[t]->scaleS = 1.0f / (float)cache.current[t]->realWidth;
	cache.current[t]->scaleT = 1.0f / (float)cache.current[t]->realHeight;

	cache.current[t]->offsetScaleS = cache.current[t]->scaleS;
	cache.current[t]->offsetScaleT = cache.current[t]->scaleT;

	if (RSP.textureTile[t]->shiftS > 10)
		cache.current[t]->scaleS *= (float)(1 << (16 - RSP.textureTile[t]->shiftS));
	else if (RSP.textureTile[t]->shiftS > 0)
		cache.current[t]->scaleS /= (float)(1 << RSP.textureTile[t]->shiftS);

	if (RSP.textureTile[t]->shiftT > 10)
		cache.current[t]->scaleT *= (float)(1 << (16 - RSP.textureTile[t]->shiftT));
	else if (RSP.textureTile[t]->shiftT > 0)
		cache.current[t]->scaleT /= (float)(1 << RSP.textureTile[t]->shiftT);

	TextureCache_Load( cache.current[t] );

	// Set filter mode. Almost always bilinear, but check anyways
	if ((RDP.otherMode_H & TEXTFILT_BILERP) || (RDP.otherMode_H & TEXTFILT_AVERAGE) || (OGL.forceBilinear))
	{
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	}
	else
	{
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	}

	// Set clamping modes
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, cache.current[t]->clampS ? GL_CLAMP : GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, cache.current[t]->clampT ? GL_CLAMP : GL_REPEAT );

	cache.cachedBytes += cache.current[t]->textureBytes;
}

void TextureCache_ActivateNoise( BYTE t )
{
	if (OGL.ARB_multitexture)
		glActiveTextureARB( GL_TEXTURE0_ARB + t );

	glBindTexture( GL_TEXTURE_2D, cache.glNoiseNames[RSP.DList & 0x1F] );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
}