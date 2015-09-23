#ifndef S2DEX_H
#define S2DEX_H

typedef struct
{
	WORD	imageW;			// The width of BG image(u10.2)
	WORD	imageX;			// The x-coordinate of the upper-left position of BG image (u10.5)
	WORD	frameW;			// The width of the transfer frame (u10.2)
	WORD	frameX;			// The upper-left position of the transfer frame(s10.2)
	WORD	imageH;			// The height of BG image (u10.2)
	WORD	imageY;			// The y-coordinate of the upper-left position of BG image (u10.5)
	WORD	frameH;			// The height of the transfer frame (u10.2)
	WORD	frameY;			// The upper-left position of the transfer frame (s10.2)
	DWORD	imagePtr;		// The texture address of the upper-left position of BG image
	BYTE	imageSiz;		// The size of BG image G_IM_SIZ_* 
	BYTE	imageFmt;		// The format of BG image G_IM_FMT_*
	WORD	imageLoad;		// Which to use, LoadBlock and LoadTile 
	WORD	imageFlip;		// Image horizontal flip. Flip using G_BG_FLAG_FLIPS.*/
	WORD	imagePal;		// The pallete number                        
} ObjBg;

void S2DEX_BG_Copy();

#endif