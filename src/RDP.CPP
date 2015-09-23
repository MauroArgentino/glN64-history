#include <windows.h>
#include <gl/gl.h>
#include "Debug.h"
#include "RSP.h"
#include "glNintendo64().h"
#include "RDP.h"
#include "N64.h"
#include <stdio.h>
#include "convert.h"

RDPInfo RDP;

BOOL RDP_DumpDL;
BOOL RDP_DumpNextDL;

void RDP_Unknown()
{
#ifdef DEBUG
	DebugMsg( DEBUG_UNKNOWN, "0x%08X: 0x%08X 0x%08X RDP_Unknown\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_UNKNOWN, "\tUnknown opcode %02X\r\n", RSP.cmd0 >> 24 );
#endif
	RSP.PC[RSP.PCi] += 8;
}

void RDP_NoOp()
{
#ifdef DEBUG
	DebugMsg( DEBUG_HANDLED | DEBUG_IGNORED, "0x%08X: 0x%08X 0x%08X RDP_NoOp\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_HANDLED | DEBUG_IGNORED, "\tIgnored0\r\n" );
#endif
	RSP.PC[RSP.PCi] += 8;
}

void RDP_TexRect()
{
	float x1 = ((RSP.cmd0 & 0x00FFF000) >> 12) * 0.25f;
	float y1 = (RSP.cmd0 & 0x00000FFF) * 0.25f;
	float x0 = ((RSP.cmd1 & 0x00FFF000) >> 12) * 0.25f;
	float y0 = (RSP.cmd1 & 0x00000FFF) * 0.25f;	
	BYTE tile = (RSP.cmd1 >> 24) & 0x7;

	RSP.cmd0 = *(DWORD*)&RDRAM[RSP.PC[RSP.PCi] + 12];
	RSP.cmd1 = *(DWORD*)&RDRAM[RSP.PC[RSP.PCi] + 20];

 	float dsdx = (SHORT)((RSP.cmd1 >> 16) & 0xFFFF) * 9.765625e-4f;
	float dtdy = (SHORT)(RSP.cmd1 & 0xFFFF) * 9.765625e-4f;

 	if (RDP.otherMode_H & CYCLETYPE_COPY)
	{
		dsdx /= 4.0f;
 		x1 += 1.0f;
		y1 += 1.0f;
	} 

  	float u0 = (SHORT)((RSP.cmd0 & 0xFFFF0000) >> 16) / 32.0f;
   	float v0 = (SHORT)(RSP.cmd0 & 0x0000FFFF) / 32.0f;
   	float u1 = u0 + ((x1 - x0 - 1) * dsdx);
 	float v1 = v0 + ((y1 - y0 - 1) * dtdy);

	// TexRect uses a specific tile
	RSP.textureTile[0] = &RDP.tiles[tile];
	RSP.textureTile[1] = &RDP.tiles[tile + 1];

	OGL_DrawTexturedRect( x0, u0, y0, v0, x1, u1, y1, v1 );

	// Set the tile back
	RSP.textureTile[0] = &RDP.tiles[RSP.texture.tile];
	RSP.textureTile[1] = &RDP.tiles[RSP.texture.tile + 1];

#ifdef DEBUG
	DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X RDP_TexRect\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
 	DebugMsg( DEBUG_HANDLED, "\tx0 = %f, y0 = %f, x1 = %f, y1 = %f, s = %f, t = %f, dsdx = %f, dtdy = %f\r\n",
		x0, y0, x1, y1, u0, v0, dsdx, dtdy );
#endif DEBUG
	RSP.PC[RSP.PCi] += 24;
}

void RDP_TexRectFlip()
{
#ifdef DEBUG
	DebugMsg( DEBUG_UNHANDLED, "0x%08X: 0x%08X 0x%08X RDP_TexRectFlip\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_UNHANDLED, "\tUnhandled\r\n" );
#endif
	RSP.PC[RSP.PCi] += 8;
}

void RDP_LoadSync()
{
#ifdef DEBUG
	DebugMsg( DEBUG_IGNORED, "0x%08X: 0x%08X 0x%08X RDP_LoadSync\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_IGNORED, "\tNotified render that texture update is necessary\r\n" );
#endif
	RSP.PC[RSP.PCi] += 8;
}

void RDP_PipeSync()
{
#ifdef DEBUG
	DebugMsg( DEBUG_IGNORED, "0x%08X: 0x%08X 0x%08X RDP_PipeSync\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_IGNORED, "\tIgnored\r\n" );
#endif
	RSP.PC[RSP.PCi] += 8;
}

void RDP_TileSync()
{
#ifdef DEBUG
	DebugMsg( DEBUG_IGNORED, "0x%08X: 0x%08X 0x%08X RDP_TileSync\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_IGNORED, "\tIgnored\r\n" );
#endif
	RSP.PC[RSP.PCi] += 8;
}

void RDP_FullSync()
{
	*REG.MI_INTR |= MI_INTR_DP;
	CheckInterrupts();

#ifdef DEBUG
	DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X RDP_FullSync\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_HANDLED, "\tGenerated DP Interrupt\r\n" );
#endif
	RSP.PC[RSP.PCi] += 8;
}

void RDP_SetKeyGB()
{
#ifdef DEBUG
	DebugMsg( DEBUG_UNHANDLED, "0x%08X: 0x%08X 0x%08X RDP_SetKeyGB\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_UNHANDLED, "\tUnhandled\r\n" );
#endif
	RSP.PC[RSP.PCi] += 8;
}

void RDP_SetKeyR()
{
#ifdef DEBUG
	DebugMsg( DEBUG_UNHANDLED, "0x%08X: 0x%08X 0x%08X RDP_SetKeyR\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_UNHANDLED, "\tUnhandled\r\n" );
#endif
	RSP.PC[RSP.PCi] += 8;
}

void RDP_SetConvert()
{
#ifdef DEBUG
	DebugMsg( DEBUG_UNHANDLED, "0x%08X: 0x%08X 0x%08X RDP_SetConvert\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_UNHANDLED, "\tUnhandled\r\n" );
#endif
	RSP.PC[RSP.PCi] += 8;
}

void RDP_SetScissor()
{
	BYTE mode = (RSP.cmd1 >> 24) & 0x03;
	int ulx = ((RSP.cmd0 >> 12) & 0x0FFF) >> 2;
	int uly =  (RSP.cmd0        & 0x0FFF) >> 2;
	int lrx = ((RSP.cmd1 >> 12) & 0x0FFF) >> 2;
	int	lry =  (RSP.cmd1        & 0x0FFF) >> 2;

	glScissor( ulx * OGL.scaleX, (RDP.height - lry) * OGL.scaleY + OGL.heightOffset,
		(lrx - ulx) * OGL.scaleX, (lry - uly) * OGL.scaleY );
	
#ifdef DEBUG
	DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X RDP_SetScissor\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_HANDLED, "\tMode = %i, ULX = %i, ULY = %i, LRX = %i, LRY = %i\r\n", mode, ulx, uly, lrx, lry );
#endif

	RSP.PC[RSP.PCi] += 8;
}

void RDP_SetPrimDepth()
{
	RDP.primDepth = ((RSP.cmd1 >> 16) & 0x7FFF) * 3.0518509e-5f;

#ifdef DEBUG
	DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X RDP_SetPrimDepth\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_HANDLED, "\tPrimDepth = %f\r\n", RDP.primDepth );
#endif

	RSP.PC[RSP.PCi] += 8;
}

void RDP_SetOtherMode()
{
	RDP.changed.otherMode_H |= RDP.otherMode_H ^ (RSP.cmd0 & 0x00FFFFFF);
	RDP.otherMode_H = RSP.cmd0 & 0x00FFFFFF;

	RDP.changed.otherMode_L |= RDP.otherMode_L ^ RSP.cmd1;
	RDP.otherMode_L = RSP.cmd1;

#ifdef DEBUG
	DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X RDP_SetOtherMode\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_HANDLED, "\totherMode_L = %u, otherMode_H = %u\r\n", RDP.otherMode_H, RDP.otherMode_L );
#endif

	RSP.PC[RSP.PCi] += 8;
}

void RDP_LoadTlut()
{
	BYTE tile = (RSP.cmd1 >> 24) & 0x07;
	WORD i, count = (WORD)((RSP.cmd1 >> 14) & 0x3FF) + 1;
	DWORD *destPalette = &RDP.palette[RDP.tiles[tile].tMem - 256];
	WORD *srcPalette = (WORD*)&RDRAM[RDP.textureImage.address];

	if ((RDP.otherMode_H & TEXTLUT_IA16) == TEXTLUT_IA16)
		for (i = 0; i < count; i++)
			destPalette[i] = IA88_ABGR8888( srcPalette[i^1] );
	else
		for (i = 0; i < count; i++)
			destPalette[i] = RGBA5551_ABGR8888( srcPalette[i^1] );

	//RDP.textureImage.address += count << 1;

#ifdef DEBUG
	DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X RDP_LoadTlut\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_HANDLED, "\tTile = %u, Start = %u, Count = %u\r\n", tile, RDP.tiles[tile].tMem - 256, count );
#endif

	RSP.PC[RSP.PCi] += 8;
}

void RDP_SetTileSize()
{
	BYTE tile = (RSP.cmd1 >> 24) & 0x7;

	RDP.tiles[tile].fulS = ((RSP.cmd0 >> 12) & 0xFFF) / 4.0f;
	RDP.tiles[tile].fulT = (RSP.cmd0 & 0xFFF) / 4.0f;

	RDP.tiles[tile].ulS = (RSP.cmd0 >> 14) & 0x3FF;
	RDP.tiles[tile].ulT = (RSP.cmd0 >> 2) & 0x3FF;
 	RDP.tiles[tile].lrS = (RSP.cmd1 >> 14) & 0x3FF;
 	RDP.tiles[tile].lrT = (RSP.cmd1 >> 2) & 0x3FF;

	RSP.update |= UPDATE_TEXTURES;

#ifdef DEBUG
	DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X RDP_SetTileSize\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_HANDLED, "\tTile = %u, ulS = %u, ulT = %u, lrS = %u, lrT = %u\r\n", tile, RDP.tiles[tile].ulS, RDP.tiles[tile].ulT, RDP.tiles[tile].lrS, RDP.tiles[tile].lrT );
#endif
	RSP.PC[RSP.PCi] += 8;
}

void RDP_LoadBlock()
{
	// 32-bit textures are strange on the N64. They are actually arranged much like
	// matrices.

	// On the real N64, it gets loaded with low and high words split, every four
	// pixels (qword alignment). Like this:

	// |RG0|RG1|RG2|RG3|BA0|BA1|BA2|BA3|

	// Instead of putting them in order, like this:

	// |RG0|BA0|RG1|BA1|RG2|BA2|RG3|BA3|

	// This is why 32-bit texture's line value seems to be half what it should be,
	// and why they're interleaved on qwords instead of dwords.

	// Fortunately, games depend on the RDP to do this, and since we're doing the
	// loading here, we can fudge the emulation. We do have to do the interleaving
	// correctly though. We can load them normally, we just have to do special handling
	// with 32-bit textures to make sure they're interleaved/deinterleaved correctly.

	void (*Interleave)( void *mem, WORD numDWords );

	DWORD64 *src, *dest;
	DWORD y, bytes, line, bpl, height;

	RDP.loadTile = &RDP.tiles[(RSP.cmd1 >> 24) & 0x7];

	// ulS - ignored
	// ulT - ignored
	// lrS = ((width * height) >> size_shift) - 1. So, number of texels to load = (lrS + 1) << size_shift;
	// dxT = reciprocal of number of 64-bit words in a line, 1.11 fixed point

	RDP.loadTile->ulS = (WORD)(RSP.cmd0 >> 14) & 0x3FF;
	RDP.loadTile->ulT = (WORD)(RSP.cmd0 >>  2) & 0x3FF;

	RDP.loadTile->lrS = (WORD)(RSP.cmd1 >> 12) & 0xFFF;
 	RDP.loadTile->dxT = (WORD)RSP.cmd1 & 0xFFF;

 	bytes = (RDP.loadTile->lrS + 1) << max(1, RDP.loadTile->size - 1);

	// Stop it if it tries to load outside RDRAM, not at all, or bigger than TMem
	if (((RDP.textureImage.address + bytes) > 0x007FFFFF) || (bytes == 0) || (((RDP.loadTile->tMem << 3) + bytes) > 4096))
	{
		RSP.PC[RSP.PCi] += 8;
		return;
	}

	src = (DWORD64*)(RDRAM + RDP.textureImage.address);
	dest = RDP.tMem + RDP.loadTile->tMem;

	if (RDP.loadTile->dxT)
	{
		line = (2047 + RDP.loadTile->dxT) / RDP.loadTile->dxT;
		bpl = line << 3;
		height = bytes / bpl;

		if (RDP.loadTile->size == IMAGE_SIZE_32b)
			Interleave = QWordInterleave;
		else
			Interleave = DWordInterleave;

		for (y = 0; y < height; y++)
		{
			UnswapCopy( src, dest, bpl );
			if (y & 1) Interleave( dest, line );

			src += line;
			dest += line;
		}
	}
	else
		UnswapCopy( src, dest, bytes );

	RDP.loadTile->loadType = LOADTYPE_BLOCK;

	RSP.update |= UPDATE_TEXTURES;

#ifdef DEBUG
	DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X RDP_LoadBlock\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_HANDLED, "\tTile = %u, ulS = %u, ulT = %u, lrS = %u, lrT = %u\r\n", (RSP.cmd1 >> 24) & 0x7, RDP.loadTile->ulS, RDP.loadTile->ulT, RDP.loadTile->lrS, RDP.loadTile->lrT );
#endif
	RSP.PC[RSP.PCi] += 8;
}

void RDP_LoadTile()
{
	void (*Interleave)( void *mem, WORD numDWords );

	WORD line, bpl, y, height;
	DWORD address;

	BYTE *src;
	DWORD64 *dest;

	BYTE tile = (RSP.cmd1 >> 24) & 0x7;
	RDP.loadTile = &RDP.tiles[tile];

	RDP.loadTile->ulS = (WORD)(RSP.cmd0 >> 14) & 0x3FF;
	RDP.loadTile->ulT = (WORD)(RSP.cmd0 >>  2) & 0x3FF;
	RDP.loadTile->lrS = (WORD)(RSP.cmd1 >> 14) & 0x3FF;
	RDP.loadTile->lrT = (WORD)(RSP.cmd1 >>  2) & 0x3FF;

	// If line is 0, what's the point in continuing?
	if (RDP.loadTile->line == 0)
	{
		RSP.PC[RSP.PCi] += 8;
		return;
	}

	height = RDP.loadTile->lrT - RDP.loadTile->ulT + 1;

	dest = &RDP.tMem[RDP.loadTile->tMem];
	address = RDP.textureImage.address + RDP.loadTile->ulT * RDP.textureImage.bpl + (RDP.loadTile->ulS << RDP.textureImage.size >> 1);
	bpl = (RDP.loadTile->lrS - RDP.loadTile->ulS + 1) << RDP.loadTile->size >> 1;

	// Don't let it go outside RDRAM
	if (((address + height * bpl) > 0x007FFFFF) ||
		(((RDP.loadTile->tMem << 3) + bpl * height) > 4096)) // Stay within TMEM
	{
		RSP.PC[RSP.PCi] += 8;
		return;
	}

	src = &RDRAM[address];

	// Line given for 32-bit is half what it seems it should since they split the
	// high and low words...it's strange.
	if (RDP.loadTile->size == IMAGE_SIZE_32b)
	{
		line = RDP.loadTile->line << 1;
		Interleave = QWordInterleave;
	}
	else
	{
		line = RDP.loadTile->line;
		Interleave = DWordInterleave;
	}

	for (y = 0; y < height; y++)
	{
		UnswapCopy( src, dest, bpl );
		if (y & 1) Interleave( dest, line );

		src += RDP.textureImage.bpl;
 		dest += line;
	}

	RDP.loadTile->loadType = LOADTYPE_TILE;

	RSP.update |= UPDATE_TEXTURES;

#ifdef DEBUG
	DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X RDP_LoadTile\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_HANDLED, "\tTile = %u, ulS = %u, ulT = %u, lrS = %u, lrT = %u\r\n", tile, RDP.tiles[tile].ulS, RDP.tiles[tile].ulT, RDP.tiles[tile].lrS, RDP.tiles[tile].lrT );
#endif
	RSP.PC[RSP.PCi] += 8;
}

void RDP_SetTile()
{
 	BYTE tile = (RSP.cmd1 >> 24) & 0x7;

	RDP.tiles[tile].format  = ((RSP.cmd0 >> 21) & 0x7);
	RDP.tiles[tile].size    = ((RSP.cmd0 >> 19) & 0x3);
	RDP.tiles[tile].line    = ((RSP.cmd0 >>  9) & 0x1FF);
	RDP.tiles[tile].tMem    = ( RSP.cmd0        & 0x1FF);
	RDP.tiles[tile].palette = ((RSP.cmd1 >> 20) & 0xF);
	RDP.tiles[tile].clampT  = ((RSP.cmd1 >> 19) & 0x1);
	RDP.tiles[tile].mirrorT = ((RSP.cmd1 >> 18) & 0x1);
	RDP.tiles[tile].maskT   = ((RSP.cmd1 >> 14) & 0xF);
	RDP.tiles[tile].shiftT  = ((RSP.cmd1 >> 10) & 0xF);
	RDP.tiles[tile].clampS  = ((RSP.cmd1 >>  9) & 0x1);
	RDP.tiles[tile].mirrorS = ((RSP.cmd1 >>  8) & 0x1);
	RDP.tiles[tile].maskS   = ((RSP.cmd1 >>  4) & 0xF);
	RDP.tiles[tile].shiftS  = ( RSP.cmd1        & 0xF);

	// Force clamping if not masking
	if (RDP.tiles[tile].maskS == 0)
		RDP.tiles[tile].clampS = 1;

	if (RDP.tiles[tile].maskT == 0)
		RDP.tiles[tile].clampT = 1;

	RSP.update |= UPDATE_TEXTURES;

#ifdef DEBUG
	DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X RDP_SetTile\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_HANDLED, "\tTile= %u, Format = %i, Size = %i, Line = %i, TMem = %i, Palette = %i\r\n", tile,
		RDP.tiles[tile].format, RDP.tiles[tile].size, RDP.tiles[tile].line, RDP.tiles[tile].tMem, RDP.tiles[tile].palette );
	DebugMsg( DEBUG_HANDLED, "\tClampT = %i, MirrorT = %i, MaskT = %i, ShiftT = %i, ClampS = %i, MirrorS = %i, MaskS = %i, ShiftS = %i\r\n",
		RDP.tiles[tile].clampT, RDP.tiles[tile].mirrorT, RDP.tiles[tile].maskT, RDP.tiles[tile].shiftT, RDP.tiles[tile].clampS, RDP.tiles[tile].mirrorS, RDP.tiles[tile].maskS, RDP.tiles[tile].shiftS );
#endif
	RSP.PC[RSP.PCi] += 8;
}

void RDP_FillRect()
{
	// Only time color and depth image should be the same
	// is when clearing the depth buffer...
	if (RDP.colorImage.address == RDP.depthImageAddress)
	{
		glDepthMask( TRUE );
		glClear( GL_DEPTH_BUFFER_BIT );
		RSP.update |= UPDATE_ZUPDATE;

#ifdef DEBUG
	DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X RDP_FillRect\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_HANDLED, "\tClearing Depth Buffer..." );
#endif
		RSP.PC[RSP.PCi] += 8;
		return;
	}

 	WORD x0 = (RSP.cmd1 >> 14) & 0x3FF;
	WORD y0 = (RSP.cmd1 >>  2) & 0x3FF;
	WORD x1 = (RSP.cmd0 >> 14) & 0x3FF;
	WORD y1 = (RSP.cmd0 >>  2) & 0x3FF;

	// Optimization - detect if it's clearing the whole screen
	if ((x0 == 0) && (y0 == 0) && (x1 == RDP.width - 1) && (y1 == RDP.height- 1) && ((RDP.otherMode_H & CYCLETYPE) == CYCLETYPE_FILL))
	{
		glClearColor( RDP.fillColor.r, RDP.fillColor.g, RDP.fillColor.b, RDP.fillColor.a );
		glClear( GL_COLOR_BUFFER_BIT );

#ifdef DEBUG
	DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X RDP_FillRect\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_HANDLED, "\tClearing Color Buffer..." );
#endif
		RSP.PC[RSP.PCi] += 8;
		return;
	}

	OGL_DrawRect( x0, y0, x1, y1, RDP.fillColor );

#ifdef DEBUG
	DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X RDP_FillRect\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_HANDLED, "\tx0 = %f, y0 = %f, x1 = %f, y1 = %f\r\n", x0, y0, x1, y1 );
#endif

	RSP.PC[RSP.PCi] += 8;
}

void RDP_SetFillColor()
{
	RDP.fillColor.r = (GLfloat)((RSP.cmd1 >> 11) & 0x1F) * 3.22581e-02f;
	RDP.fillColor.g = (GLfloat)((RSP.cmd1 >> 6) & 0x1F) * 3.22581e-02f;
	RDP.fillColor.b = (GLfloat)((RSP.cmd1 >> 1) & 0x1F) * 3.22581e-02f;
	RDP.fillColor.a = (RSP.cmd1 & 0x1) ? 255.0f : 0.0f;

#ifdef DEBUG
	DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X RDP_SetFillColor\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_HANDLED, "\tR = %f, G = %f, B = %f\r\n", RDP.fillColor.r, RDP.fillColor.g, RDP.fillColor.b );
#endif

	RSP.PC[RSP.PCi] += 8;
}

void RDP_SetBlendColor()
{
	RDP.blendColor.r = (GLfloat)((RSP.cmd1 >> 24) & 0xFF) * 3.9215689e-03f;
	RDP.blendColor.g = (GLfloat)((RSP.cmd1 >> 16) & 0xFF) * 3.9215689e-03f;
	RDP.blendColor.b = (GLfloat)((RSP.cmd1 >>  8) & 0xFF) * 3.9215689e-03f;
	RDP.blendColor.a = (GLfloat)((RSP.cmd1      ) & 0xFF) * 3.9215689e-03f;

#ifdef DEBUG
	DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X RDP_SetBlendColor\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_HANDLED, "\tR = %f, G = %f, B = %f, A = %f\r\n", RDP.blendColor.r, RDP.blendColor.g, RDP.blendColor.b, RDP.blendColor.a );
#endif

	RSP.PC[RSP.PCi] += 8;
}

void RDP_SetFogColor()
{
	RDP.fogColor.r = (GLfloat)((RSP.cmd1 >> 24) & 0xFF) * 3.9215689e-03f;
	RDP.fogColor.g = (GLfloat)((RSP.cmd1 >> 16) & 0xFF) * 3.9215689e-03f;
	RDP.fogColor.b = (GLfloat)((RSP.cmd1 >>  8) & 0xFF) * 3.9215689e-03f;
	RDP.fogColor.a = (GLfloat)((RSP.cmd1      ) & 0xFF) * 3.9215689e-03f;

	glFogfv( GL_FOG_COLOR, &RDP.fogColor.r );

#ifdef DEBUG
	DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X RDP_SetFogColor\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_HANDLED, "\tR = %f, G = %f, B = %f, A = %f\r\n", RDP.blendColor.r, RDP.fogColor.g, RDP.fogColor.b, RDP.fogColor.a );
#endif

	RSP.PC[RSP.PCi] += 8;
}

void RDP_SetPrimColor()
{
  	RDP.primColor.r = ((RSP.cmd1 >> 24) & 0xFF) * 3.9215689e-03f;
	RDP.primColor.g = ((RSP.cmd1 >> 16) & 0xFF) * 3.9215689e-03f;
	RDP.primColor.b = ((RSP.cmd1 >>  8) & 0xFF) * 3.9215689e-03f;
	RDP.primColor.a = ((RSP.cmd1      ) & 0xFF) * 3.9215689e-03f;
	RDP.primLODFrac = (RSP.cmd0 & 0xFF) * 3.9215689e-03f;

	RSP.update |= UPDATE_COMBINE_COLORS;

#ifdef DEBUG
	DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X RDP_SetPrimColor\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_HANDLED, "\tR = %f, G = %f, B = %f, A = %f\r\n", RDP.primColor.r, RDP.primColor.g, RDP.primColor.b, RDP.primColor.a );
#endif

	RSP.PC[RSP.PCi] += 8;
}

void RDP_SetEnvColor()
{
	RDP.envColor.r = (GLfloat)((RSP.cmd1 >> 24) & 0xFF) * 3.9215689e-03f;
	RDP.envColor.g = (GLfloat)((RSP.cmd1 >> 16) & 0xFF) * 3.9215689e-03f;
	RDP.envColor.b = (GLfloat)((RSP.cmd1 >>  8) & 0xFF) * 3.9215689e-03f;
	RDP.envColor.a = (GLfloat)((RSP.cmd1      ) & 0xFF) * 3.9215689e-03f;

	RSP.update |= UPDATE_COMBINE_COLORS;

#ifdef DEBUG
	DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X RDP_SetEnvColor\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_HANDLED, "\tR = %f, G = %f, B = %f, A = %f\r\n", RDP.envColor.r, RDP.envColor.g, RDP.envColor.b, RDP.envColor.a );
#endif

	RSP.PC[RSP.PCi] += 8;
}

void RDP_SetCombine()
{
	DWORD oldCombine0 = RDP.combine0;
	DWORD oldCombine1 = RDP.combine1;

	RDP.combine0 = RSP.cmd0;
	RDP.combine1 = RSP.cmd1;

	if ((oldCombine0 != RDP.combine0) ||
		(oldCombine1 != RDP.combine1))
		RSP.update |= UPDATE_COMBINE;

#ifdef DEBUG
	DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X RDP_SetCombine\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_HANDLED, "\tCombine Mode = 0x%08X%08X\r\n", RDP.combine0, RDP.combine1 );
#endif

	RSP.PC[RSP.PCi] += 8;
}

void RDP_SetTImg()
{
	RDP.textureImage.format = (RSP.cmd0 >> 21) & 0x7;
	RDP.textureImage.size = (RSP.cmd0 >> 19) & 0x3;
 	RDP.textureImage.width = (RSP.cmd0 & 0xFFF) + 1;
	RDP.textureImage.address = RSP_SegmentAddress(RSP.cmd1);
	// bpl doesn't actually exist on the N64, but it helps here
	RDP.textureImage.bpl = RDP.textureImage.width << RDP.textureImage.size >> 1;

#ifdef DEBUG
	DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X RDP_SetTImg\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_HANDLED, "\tFormat = %u, Size = %u, Width = %u, Address = 0x0x%08X\r\n",
		(int)RDP.textureImage.format, (int)RDP.textureImage.size, (int)RDP.textureImage.width, RDP.textureImage.address );
#endif

	RSP.PC[RSP.PCi] += 8;
}

void RDP_SetZImg()
{
	RDP.depthImageAddress = RSP_SegmentAddress( RSP.cmd1 );

#ifdef DEBUG
	DebugMsg( DEBUG_UNHANDLED, "0x%08X: 0x%08X 0x%08X RDP_SetZImg\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_UNHANDLED, "\tDepth Image Address = 0x0x%08X\r\n", RDP.depthImageAddress );
#endif

	RSP.PC[RSP.PCi] += 8;
}

void RDP_SetCImg()
{
	RDP.colorImage.format = (RSP.cmd0 >> 21) & 0x7;
	RDP.colorImage.size = (RSP.cmd0 >> 19) & 0x3;
	RDP.colorImage.width = (RSP.cmd0 & 0xFFF) + 1;
	RDP.colorImage.address = RSP_SegmentAddress( RSP.cmd1 );
	RDP.colorImage.bpl = RDP.colorImage.width << RDP.colorImage.size >> 1;

#ifdef DEBUG
	DebugMsg( DEBUG_UNHANDLED, "0x%08X: 0x%08X 0x%08X RDP_SetCImg\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_UNHANDLED, "\tColor Image Address = 0x0x%08X\r\n", RDP.colorImageAddress );
#endif

	RSP.PC[RSP.PCi] += 8;
}

void RDP_Init()
{
	int i;
	// C0-CE are RDP Commands
	for (i = 0xC0; i < 0xCF; i++)
		GFXOp[i] = RDP_Unknown;

	// E4-FF are RDP commands
	for (i = 0xE4; i < 0x100; i++)
		GFXOp[i] = RDP_Unknown;

	// Set known RDP commands
	GFXOp[0xC0] = RDP_NoOp;

	GFXOp[0xE4] = RDP_TexRect;
	GFXOp[0xE5] = RDP_TexRectFlip;
	GFXOp[0xE6] = RDP_LoadSync;
	GFXOp[0xE7] = RDP_PipeSync;
	GFXOp[0xE8] = RDP_TileSync;
	GFXOp[0xE9] = RDP_FullSync;
	GFXOp[0xEA] = RDP_SetKeyGB;
	GFXOp[0xEB] = RDP_SetKeyR;
	GFXOp[0xEC] = RDP_SetConvert;
	GFXOp[0xED] = RDP_SetScissor;
	GFXOp[0xEE] = RDP_SetPrimDepth;
	GFXOp[0xEF] = RDP_SetOtherMode;

	GFXOp[0xF0] = RDP_LoadTlut;
	GFXOp[0xF2] = RDP_SetTileSize;
	GFXOp[0xF3] = RDP_LoadBlock;
	GFXOp[0xF4] = RDP_LoadTile;
	GFXOp[0xF5] = RDP_SetTile;
	GFXOp[0xF6] = RDP_FillRect;
	GFXOp[0xF7] = RDP_SetFillColor;
	GFXOp[0xF8] = RDP_SetFogColor;
	GFXOp[0xF9] = RDP_SetBlendColor;
	GFXOp[0xFA] = RDP_SetPrimColor;
	GFXOp[0xFB] = RDP_SetEnvColor;
	GFXOp[0xFC] = RDP_SetCombine;
	GFXOp[0xFD] = RDP_SetTImg;
	GFXOp[0xFE] = RDP_SetZImg;
	GFXOp[0xFF] = RDP_SetCImg;

	RDP.loadTile = &RDP.tiles[7];
}