#ifndef RDP_H
#define RDP_H

#include <windows.h>
#include "OpenGL.h"
#include <stdio.h>
#include "Combiner.h"
#include "glNintendo64().h"

#define LOADTYPE_BLOCK			0
#define LOADTYPE_TILE			1

#define IMAGE_FORMAT_RGBA		0
#define IMAGE_FORMAT_YUV		1
#define IMAGE_FORMAT_CI			2
#define IMAGE_FORMAT_IA			3
#define IMAGE_FORMAT_I			4

#define IMAGE_SIZE_4b			0
#define IMAGE_SIZE_8b			1
#define IMAGE_SIZE_16b			2
#define IMAGE_SIZE_32b			3

#define IMAGE_SIZE_4b_SHIFT		2
#define IMAGE_SIZE_8b_SHIFT		1
#define IMAGE_SIZE_16b_SHIFT	0
#define IMAGE_SIZE_32b_SHIFT	0

// SetOtherMode_L shifts
#define	OTHERMODE_L_ALPHACOMPARE			0
#define	OTHERMODE_L_ZSOURCE					2
#define	OTHERMODE_L_RENDERMODE				3
#define	OTHERMODE_L_BLENDER					16

// Alpha compare modes
#define ALPHACOMPARE						0x3

#define	ALPHACOMPARE_NONE					0x0 
#define	ALPHACOMPARE_THRESHOLD				0x1
#define	ALPHACOMPARE_DITHER					0x3

// Z Sources
#define	ZSOURCE_PIXEL						0x0000
#define	ZSOURCE_PRIMITIVE					0x0004

// Render modes
#define RENDERMODE_ZMODE					0x0C30

#define	RENDERMODE_AA_ENABLE				0x0008 // Enable AntiAliasing
#define	RENDERMODE_Z_COMPARE				0x0010 // Z Compare
#define	RENDERMODE_Z_UPDATE					0x0020 // Z Update
#define	RENDERMODE_IM_RD					0x0040 // Image Read
#define	RENDERMODE_CLR_ON_CVG				0x0080 // Clear on Coverage
#define	RENDERMODE_CVG_DST_CLAMP			0x0000 // Clamp Coverage
#define	RENDERMODE_CVG_DST_WRAP				0x0100 // Wrap Coverage
#define	RENDERMODE_CVG_DST_FULL				0x0200 // Full coverage
#define	RENDERMODE_CVG_DST_SAVE				0x0300 // Save Coverage
#define	RENDERMODE_ZMODE_OPA				0x0000 // Opaque
#define	RENDERMODE_ZMODE_INTER				0x0400 // Interpenetrating
#define	RENDERMODE_ZMODE_XLU				0x0800 // Transparent
#define	RENDERMODE_ZMODE_DECAL				0x0C00 // Decal
#define	RENDERMODE_CVG_X_ALPHA				0x1000 // Coverage * Alpha
#define	RENDERMODE_ALPHA_CVG_SEL			0x2000 // Use Coverage for Alpha
#define	RENDERMODE_FORCE_BL					0x4000 // Force Blender
#define	RENDERMODE_TEX_EDGE					0x0000

// SetOtherMode_H shifts
#define	OTHERMODE_H_BLENDMASK				0	/* unsupported */
#define	OTHERMODE_H_ALPHADITHER				4
#define	OTHERMODE_H_RGBDITHER				6

#define	OTHERMODE_H_COMBKEY					8
#define	OTHERMODE_H_TEXTCONV				9
#define	OTHERMODE_H_TEXTFILT				12
#define	OTHERMODE_H_TEXTLUT					14
#define	OTHERMODE_H_TEXTLOD					16
#define	OTHERMODE_H_TEXTDETAIL				17
#define	OTHERMODE_H_TEXTPERSP				19
#define	OTHERMODE_H_CYCLETYPE				20
#define	OTHERMODE_H_COLORDITHER				22	/* unsupported in HW 2.0 */
#define	OTHERMODE_H_PIPELINE				23

// Texture Filters
#define TEXTFILT							0x00005000
#define TEXTFILT_POINT						0x00000000
#define TEXTFILT_AVERAGE					0x00003000
#define TEXTFILT_BILERP						0x00002000

#define TEXTLUT_NONE						0x00000000
#define TEXTLUT_RGBA16						0x00008000
#define TEXTLUT_IA16						0x0000C000

// Cycle Types
#define CYCLETYPE							0x00300000
#define	CYCLETYPE_1CYCLE					0x00000000
#define	CYCLETYPE_2CYCLE					0x00100000
#define	CYCLETYPE_COPY						0x00200000
#define	CYCLETYPE_FILL						0x00300000

// Color Combine Params
#define COLORCOMBINE_COMBINED			0
#define COLORCOMBINE_TEXEL0				1
#define COLORCOMBINE_TEXEL1				2
#define COLORCOMBINE_PRIMITIVE			3
#define COLORCOMBINE_SHADE				4
#define COLORCOMBINE_ENVIRONMENT		5
#define COLORCOMBINE_CENTER				6
#define COLORCOMBINE_SCALE				6
#define COLORCOMBINE_COMBINED_ALPHA		7
#define COLORCOMBINE_TEXEL0_ALPHA		8
#define COLORCOMBINE_TEXEL1_ALPHA		9
#define COLORCOMBINE_PRIMITIVE_ALPHA	10
#define COLORCOMBINE_SHADE_ALPHA		11
#define COLORCOMBINE_ENV_ALPHA			12
#define COLORCOMBINE_LOD_FRACTION		13
#define COLORCOMBINE_PRIM_LOD_FRAC		14
#define COLORCOMBINE_NOISE				7
#define COLORCOMBINE_K4					7
#define COLORCOMBINE_K5					15
#define COLORCOMBINE_1					6
#define COLORCOMBINE_0					31

// Alpha Combine Params
#define ALPHACOMBINE_COMBINED			0
#define ALPHACOMBINE_TEXEL0				1
#define ALPHACOMBINE_TEXEL1				2
#define ALPHACOMBINE_PRIMITIVE			3
#define ALPHACOMBINE_SHADE				4
#define ALPHACOMBINE_ENVIRONMENT		5
#define ALPHACOMBINE_LOD_FRACTION		0
#define ALPHACOMBINE_PRIM_LOD_FRAC		6
#define ALPHACOMBINE_1					6
#define ALPHACOMBINE_0					7

#ifdef DEBUG
static const char *CCStrings32[32] = {
	"Combined", "Texel0", "Texel1", "Primitive", "Shade", "Environment", "Center/Scale/1",
	"Combined Alpha/Noise/K4", "Texel0 Alpha", "Texel1 Alpha", "Primitive Alpha",
	"Shade Alpha", "Environment Alpha", "LOD Fraction", "Primitive LOD Fraction", "K5",
	"?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "?", "0"
};

static const char *CCStrings16[16] = {
	"Combined", "Texel0", "Texel1", "Primitive", "Shade", "Environment", "Center/Scale/1",
	"Combined Alpha/Noise/K4", "Texel0 Alpha", "Texel1 Alpha", "Primitive Alpha",
	"Shade Alpha", "Environment Alpha", "LOD Fraction", "Primitive LOD Fraction", "0"
};

static const char *CCStrings8[8] = {
	"Combined", "Texel0", "Texel1", "Primitive", "Shade", "Environment", "Center/Scale/1",
	"0"
};

static const char *ACStrings[8] = {
	"Combined/LOD Fraction", "Texel0", "Texel1", "Primitive", "Shade", "Environment",
	"Primitive LOD Fraction/1", "0"
};
#endif

typedef struct
{
	BYTE format;		// e.g. RGBA, YUV etc
	BYTE size;		// e.g 4/8/16/32bpp
	WORD line;		// Line size
	WORD tMem;		// Location in tmem

	BYTE palette;	// 0..15
	BOOL clampS, clampT;
	BOOL mirrorS, mirrorT;
	BYTE maskS, maskT;
	BYTE shiftS, shiftT;

	float fulS, fulT; // For texture coordinates

	WORD ulS;		// Upper left S
	WORD ulT;		// Upper Left T
	WORD lrS;		// Lower Right S
	WORD lrT;		// Lower Right T
	WORD dxT;		// Reciprocol of number of QWords in a line

	BYTE loadType;
} tile;

struct RDPInfo
{
	struct
	{
		BYTE format, size;
		WORD width;
		DWORD address;
		WORD bpl;
	} textureImage;

	struct
	{
		BYTE format, size;
		WORD width;
		DWORD address;
		WORD bpl;
	} colorImage;

	DWORD depthImageAddress;

	tile loadTiles[8];
	BYTE numLoads;

	tile tiles[8];
	tile *loadTile;

	DWORD otherMode_L;
	DWORD otherMode_H;

	DWORD palette[256];

	WORD width, height;

	DWORD64 tMem[512];

	DWORD combine0, combine1;

	GLcolor blendColor, primColor, fogColor, envColor, fillColor;
	float primLODFrac, primDepth;

	BOOL useT0, useT1, useNoise;

	struct
	{
		DWORD otherMode_L;
		DWORD otherMode_H;
	} changed;
};

extern RDPInfo RDP;

void RDP_Init();
void RDP_LoadTile();

#endif