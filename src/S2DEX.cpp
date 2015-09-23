#include <windows.h>
#include <gl/gl.h>
#include "RDP.h"
#include "RSP.h"
#include "OpenGL.h"
#include "S2DEX.h"

// Hacked right now, always does in 4 pixel high sections
void S2DEX_BG_Copy()
{
	ObjBg	*bgObj = (ObjBg*)(RDRAM + RSP_SegmentAddress( RSP.cmd1 ));

	WORD	imageX = bgObj->imageX >> 2;
	WORD	imageW = bgObj->imageW >> 2;
	WORD	frameX = bgObj->frameX >> 2;
	WORD	frameW = bgObj->frameW >> 2;

	WORD	imageY = bgObj->imageY >> 2;
	WORD	imageH = bgObj->imageH >> 2;
	WORD	frameY = bgObj->frameY >> 2;
	WORD	frameH = bgObj->frameH >> 2;
  
	DWORD	imagePtr = RSP_SegmentAddress( bgObj->imagePtr );
	WORD	imageLoad = bgObj->imageLoad;
	BYTE	imageFmt = bgObj->imageFmt;
	BYTE	imageSiz = bgObj->imageSiz;
	WORD	imagePal = bgObj->imagePal;
	WORD	imageFlip = bgObj->imageFlip;

	RDP.textureImage.address = RSP_SegmentAddress( imagePtr );
	RDP.textureImage.format = imageFmt;
	RDP.textureImage.size = imageSiz;
	RDP.textureImage.width = imageW;
	RDP.textureImage.bpl = RDP.textureImage.width << RDP.textureImage.size >> 1;

	RDP.tiles[7].format = imageFmt;
	RDP.tiles[7].size = imageSiz;
	RDP.tiles[7].line = frameW >> (4 - imageSiz);
	if (imageSiz == IMAGE_SIZE_32b)
		RDP.tiles[7].line >>= 1;

	RDP.tiles[7].tMem = 0;

	RDP.tiles[7].palette = imagePal;
	RDP.tiles[7].clampS = 1;
	RDP.tiles[7].clampT = 1;
	RDP.tiles[7].mirrorS = 0;
	RDP.tiles[7].mirrorT = 0;
	RDP.tiles[7].maskS = 0;
	RDP.tiles[7].maskT = 0;
	RDP.tiles[7].shiftS = 0;
	RDP.tiles[7].shiftT = 0;

	RDP.tiles[7].ulS = frameX;
	RDP.tiles[7].lrS = frameW - imageX - 1;
	RDP.tiles[7].ulT = frameY;
	RDP.tiles[7].lrT = frameY + 4;

	RDP.tiles[0].format = imageFmt;
	RDP.tiles[0].size = imageSiz;
	RDP.tiles[0].line = frameW >> (4 - imageSiz);
	RDP.tiles[0].tMem = 0;

	RDP.tiles[0].palette = imagePal;
	RDP.tiles[0].clampS = 1;
	RDP.tiles[0].clampT = 1;
	RDP.tiles[0].mirrorS = 0;
	RDP.tiles[0].mirrorT = 0;
	RDP.tiles[0].maskS = 0;
	RDP.tiles[0].maskT = 0;
	RDP.tiles[0].shiftS = 0;
	RDP.tiles[0].shiftT = 0;

	RDP.tiles[0].ulS = 0;
	RDP.tiles[0].lrS = 319;
	RDP.tiles[0].ulT = 0;
	RDP.tiles[0].lrT = 3;
	RDP.tiles[0].fulS = 0.0f;
	RDP.tiles[0].fulT = 0.0f;

	for (int y = 0; y < frameH; y += 4)
	{
		RSP.cmd0 = (frameX << 14) | (y << 2);
		RSP.cmd1 = (0x07 << 24) | ((frameW - imageX - 1) << 14) | ((y + 3) << 2);

		RDP_LoadTile();
		RSP.PC[RSP.PCi] -= 8;

		OGL_DrawTexturedRect( frameX, 0, y, 0, frameX + frameW - 1, imageW - 1, y + 4, 3 );
	}
	
	RSP.PC[RSP.PCi] += 8;
}