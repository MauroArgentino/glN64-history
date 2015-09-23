#include <windows.h>
#include "gl/gl.h"
#include "glNintendo64().h"
#include "Fast3D.h"
#include "Debug.h"
#include "N64.h"
#include "RSP.h"	
#include "RDP.h"
#include "3DMath.h"

void Fast3D_SPNoOp()
{
#ifdef DEBUG
	DebugMsg( DEBUG_HANDLED | DEBUG_IGNORED, "0x%08X: 0x%08X 0x%08X Fast3D_NoOp\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_HANDLED | DEBUG_IGNORED, "\tIgnored\r\n" );
#endif
	RSP.PC[RSP.PCi] += 8;
}

void Fast3D_Mtx()
{
	// The N64 stores the matrix values as s15.16 fixed-point numbers
	// with the all the integer parts first, and then the fractional parts

	BYTE command = (BYTE)(RSP.cmd0 >> 16);

	DWORD address = RSP_SegmentAddress(RSP.cmd1);

	float mtx[4][4];

	RSP_LoadMatrix( mtx, address );

	if (command & FAST3D_MTX_PROJECTION)
	{
        if (command & FAST3D_MTX_LOAD)
			CopyMatrix( RSP.projectionMtx, mtx );
		else
			MultMatrix( RSP.projectionMtx, mtx );
	}
	else
	{
		if ((command & FAST3D_MTX_PUSH) && (RSP.modelViewi < 9))
		{
			CopyMatrix( RSP.modelViewStack[RSP.modelViewi+1], RSP.modelViewStack[RSP.modelViewi] );
			RSP.modelViewi++;
		}

		if (command & FAST3D_MTX_LOAD)
			CopyMatrix( RSP.modelViewStack[RSP.modelViewi], mtx );
		else
			MultMatrix( RSP.modelViewStack[RSP.modelViewi], mtx );

		RSP.update |= UPDATE_LIGHTS;
	}

	RSP.update |= UPDATE_COMBINEDMATRIX;

#ifdef DEBUG
	DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X Fast3D_Mtx\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_HANDLED, "\t%s%s%s\r\n", (command & FAST3D_MTX_PUSH) ? "Push " : "Don't push ",
											 (command & FAST3D_MTX_PROJECTION) ? "projection matrix " : "world matrix ",
											 (command & FAST3D_MTX_LOAD) ? "and load:" : "and multiply by:" );
	DebugMsg( DEBUG_HANDLED, "\t%#15.6f %#15.6f %#15.6f %#15.6f\r\n", mtx[0][0], mtx[1][0], mtx[2][0], mtx[3][0] );
	DebugMsg( DEBUG_HANDLED, "\t%#15.6f %#15.6f %#15.6f %#15.6f\r\n", mtx[0][1], mtx[1][1], mtx[2][1], mtx[3][1] );
	DebugMsg( DEBUG_HANDLED, "\t%#15.6f %#15.6f %#15.6f %#15.6f\r\n", mtx[0][2], mtx[1][2], mtx[2][2], mtx[3][2] );
	DebugMsg( DEBUG_HANDLED, "\t%#15.6f %#15.6f %#15.6f %#15.6f\r\n", mtx[0][3], mtx[1][3], mtx[2][3], mtx[3][3] );
#endif

	RSP.PC[RSP.PCi] += 8;
}

void Fast3D_Reserved0()
{
#ifdef DEBUG
	DebugMsg( DEBUG_HANDLED | DEBUG_IGNORED, "0x%08X: 0x%08X 0x%08X Fast3D_Reserved0\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_HANDLED | DEBUG_IGNORED, "\tIgnored\r\n" );
#endif
	RSP.PC[RSP.PCi] += 8;
}

void Fast3D_MoveMem()
{
	BYTE type = (BYTE)(RSP.cmd0 >> 16) & 0xFF;
	WORD size = (WORD)RSP.cmd0 & 0xFFFF;
	DWORD address = RSP_SegmentAddress(RSP.cmd1);

	switch (type)
	{
		case FAST3D_MOVEMEM_VIEWPORT:
/*			float scale[4];
			float trans[4];

			scale[0] = (float)*(SHORT*)&RDRAM[address+2] * 0.25f;
			scale[1] = (float)*(SHORT*)&RDRAM[address+0] * 0.25f;
			//scale[2] = (float)*(SHORT*)&RDRAM[address+6] * 0.25f;
			//scale[3] = (float)*(SHORT*)&RDRAM[address+4] * 0.25f;

			trans[0] = (float)*(SHORT*)&RDRAM[address+10] * 0.25f;
			trans[1] = (float)*(SHORT*)&RDRAM[address+8] * 0.25f;
			//trans[2] = (float)*(SHORT*)&RDRAM[address+14] * 0.25f;
			//trans[3] = (float)*(SHORT*)&RDRAM[address+12] * 0.25f;*/

			float scale[3];
			float trans[3];

			scale[0] = *(SHORT*)&RDRAM[address+2] * 0.25f;
			scale[1] = *(SHORT*)&RDRAM[address+0] * 0.25f;
			scale[2] = *(SHORT*)&RDRAM[address+6] * 9.7847357e-04f;

			trans[0] = *(SHORT*)&RDRAM[address+10] * 0.25f;
			trans[1] = *(SHORT*)&RDRAM[address+8] * 0.25f;
			trans[2] = *(SHORT*)&RDRAM[address+14] * 9.7847357e-04f;

			RSP.viewport.x = trans[0] - scale[0];
			RSP.viewport.y = trans[1] + scale[1];
			RSP.viewport.width = scale[0] * 2;
			RSP.viewport.height = scale[1] * 2;
			RSP.viewport.nearZ = trans[2] - scale[2];
			RSP.viewport.farZ = trans[2] + scale[2];

			RSP.update |= UPDATE_VIEWPORT;

#ifdef DEBUG
			DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X Fast3D_MoveMem\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
			DebugMsg( DEBUG_HANDLED, "\tMOVEMEM_VIEWPORT: scale[0] = %f, scale[1] = %f, scale[2] = %f, scale[3] = %f\r\n", scale[0], scale[1], scale[2], scale[3] );
			DebugMsg( DEBUG_HANDLED, "\t                  trans[0] = %f, trans[1] = %f, trans[2] = %f, trans[3] = %f\r\n", trans[0], trans[1], trans[2], trans[3] );
#endif
			break;

		case FAST3D_MOVEMEM_LIGHT0:
		case FAST3D_MOVEMEM_L1GHT1:
		case FAST3D_MOVEMEM_LIGHT2:
		case FAST3D_MOVEMEM_LIGHT3:
		case FAST3D_MOVEMEM_LIGHT4:
		case FAST3D_MOVEMEM_LIGHT5:
		case FAST3D_MOVEMEM_LIGHT6:
		case FAST3D_MOVEMEM_LIGHT7:
			BYTE light;
			light = ((type - FAST3D_MOVEMEM_LIGHT0) >> 1);

			RSPLight *tempLight;
			tempLight = (RSPLight*)&RDRAM[address];

			RSP.lights[light].x = tempLight->x;
			RSP.lights[light].y = tempLight->y;
			RSP.lights[light].z = tempLight->z;

			RSP.lights[light].r = tempLight->r * 0.0039215689f;
			RSP.lights[light].g = tempLight->g * 0.0039215689f;
			RSP.lights[light].b = tempLight->b * 0.0039215689f;

			RSP.update |= UPDATE_LIGHTS;

#ifdef DEBUG
			DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X Fast3D_MoveMem\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
			DebugMsg( DEBUG_HANDLED, "\tMOVEMEM_LIGHT%i: R = %u, G = %u, B = %u, X = %i, Y = %i, Z = %i\r\n", light,
				RSP.lights[light].r, RSP.lights[light].g, RSP.lights[light].b, RSP.lights[light].x, RSP.lights[light].y, RSP.lights[light].z );
#endif
			break;			
		default:
#ifdef DEBUG
			DebugMsg( DEBUG_UNKNOWN, "0x%08X: 0x%08X 0x%08X Fast3D_MoveMem\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
			DebugMsg( DEBUG_UNKNOWN, "\tUnknown MoveMem command %i\r\n", type );
#endif
			break;
	}
	RSP.PC[RSP.PCi] += 8;
}

void Fast3D_Vtx()
{
	DWORD address = RSP_SegmentAddress(RSP.cmd1);
	BYTE num = (BYTE)((RSP.cmd0 >> 20) & 0x0F) + 1;
	BYTE v0 = (BYTE)((RSP.cmd0 >> 16) & 0x0F);

	// Make sure it's valid
	if (((RSP.cmd0 & 0xFFFF) >> 4) != num)
		if (RSP_DetectUCode())
			return; // Found a matching uCode

	if (v0 + num > 16)
	{
#ifdef DEBUG
		DebugMsg( DEBUG_ERROR | DEBUG_IGNORED, "0x%08X: 0x%08X 0x%08X Fast3D_Vtx\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
		DebugMsg( DEBUG_ERROR | DEBUG_IGNORED, "\tAttempting to write vertices to invalid location, ignoring\r\n" );
#endif
		RSP.PC[RSP.PCi] += 8;
		return;
	}

#ifdef DEBUG
	DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X Fast3D_Vertex\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
#endif

	RSP_LoadVertices( address, v0, num );

	RSP.PC[RSP.PCi] += 8;
}

void Fast3D_Reserved1()
{
#ifdef DEBUG
	DebugMsg( DEBUG_HANDLED | DEBUG_IGNORED, "0x%08X: 0x%08X 0x%08X Fast3D_Reserved1\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_HANDLED | DEBUG_IGNORED, "\tIgnored\r\n" );
#endif
	RSP.PC[RSP.PCi] += 8;
}

void Fast3D_DL()
{
	BYTE push = (BYTE)((RSP.cmd0 >> 16) & 0xFF);
	DWORD address = RSP_SegmentAddress( RSP.cmd1 );

	if (push == FAST3D_DL_PUSH)
	{
		if (RSP.PCi < 7)
		{
#ifdef DEBUG
		DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X Fast3D_DL\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
		DebugMsg( DEBUG_HANDLED, "\tPushing PC stack to %i and executing DL at 0x%08X\r\n", RSP.PCi + 1, address );
#endif
			RSP.PC[RSP.PCi] += 8;
			RSP.PCi++;
			RSP.PC[RSP.PCi] = address;
		}
		else
		{
			RSP.halt = TRUE;
#ifdef DEBUG
		DebugMsg( DEBUG_ERROR, "0x%08X: 0x%08X 0x%08X Fast3D_DL\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
		DebugMsg( DEBUG_ERROR, "\tPC Stack overflow, ending display list execution\r\n" );
#endif
		}
	}
	else
	{
		RSP.PC[RSP.PCi] = address;
#ifdef DEBUG
		DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X Fast3D_DL\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
		DebugMsg( DEBUG_HANDLED, "\tExecuting DL at 0x%08X\r\n", RSP.PC[RSP.PCi] );
#endif
	}
}

void Fast3D_Reserved2()
{
#ifdef DEBUG
	DebugMsg( DEBUG_HANDLED | DEBUG_IGNORED, "0x%08X: 0x%08X 0x%08X Fast3D_Reserved2\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_HANDLED | DEBUG_IGNORED, "\tIgnored\r\n" );
#endif
	RSP.PC[RSP.PCi] += 8;
}

void Fast3D_Reserved3()
{
#ifdef DEBUG
	DebugMsg( DEBUG_HANDLED | DEBUG_IGNORED, "0x%08X: 0x%08X 0x%08X Fast3D_Reserved3\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_HANDLED | DEBUG_IGNORED, "\tIgnored\r\n" );
#endif
	RSP.PC[RSP.PCi] += 8;
}

void Fast3D_Sprite2D_Base()
{
#ifdef DEBUG
	DebugMsg( DEBUG_UNHANDLED, "0x%08X: 0x%08X 0x%08X Fast3D_Sprite2D_Base\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_UNHANDLED, "\tUnhandled\r\n" );
#endif
	RSP.PC[RSP.PCi] += 8;
}

void Fast3D_RDPHalf_2()
{
#ifdef DEBUG
	DebugMsg( DEBUG_UNHANDLED | DEBUG_IGNORED, "0x%08X: 0x%08X 0x%08X Fast3D_RDPHalf_1\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_UNHANDLED | DEBUG_IGNORED, "\tIgnored\r\n" );
#endif
	RSP.PC[RSP.PCi] += 8;
}

void Fast3D_RDPHalf_1()
{
#ifdef DEBUG
	DebugMsg( DEBUG_UNHANDLED | DEBUG_IGNORED, "0x%08X: 0x%08X 0x%08X Fast3D_RDPHalf_2\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_UNHANDLED | DEBUG_IGNORED, "\tIgnored\r\n" );
#endif
	RSP.PC[RSP.PCi] += 8;
}

void Fast3D_Line3D()
{
#ifdef DEBUG
	DebugMsg( DEBUG_UNHANDLED, "0x%08X: 0x%08X 0x%08X Fast3D_Line3D\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_UNHANDLED, "\tUnhandled\r\n" );
#endif
	RSP.PC[RSP.PCi] += 8;
}

void Fast3D_ClearGeometryMode()
{
	DWORD old = RSP.geometryMode;
	RSP.geometryMode &= ~RSP.cmd1;
	RSP.changed.geometryMode |= old ^ RSP.geometryMode;

#ifdef DEBUG
	DebugMsg( DEBUG_HANDLED | DEBUG_UNHANDLED, "0x%08X: 0x%08X 0x%08X Fast3D_ClearGeometryMode\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_HANDLED | DEBUG_UNHANDLED, "\tGeometry Mode = 0x0x%08X\r\n", RSP.geometryMode );
#endif
	RSP.PC[RSP.PCi] += 8;
}

void Fast3D_SetGeometryMode()
{
	DWORD old = RSP.geometryMode;
	RSP.geometryMode |= RSP.cmd1;
	RSP.changed.geometryMode |= old ^ RSP.cmd1;

#ifdef DEBUG
	DebugMsg( DEBUG_HANDLED | DEBUG_UNHANDLED, "0x%08X: 0x%08X 0x%08X Fast3D_SetGeometryMode\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_HANDLED | DEBUG_UNHANDLED, "\tGeometry Mode = 0x%08X\r\n", RSP.geometryMode );
#endif
	RSP.PC[RSP.PCi] += 8;
}

void Fast3D_EndDL()
{
	if (RSP.PCi > 0)
	{
#ifdef DEBUG
		DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X Fast3D_EndDL\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
		DebugMsg( DEBUG_HANDLED, "\tPopping PC Stack, PCi=%i\r\n", RSP.PCi-1 );
#endif
		RSP.PCi--;
	}
	else
	{
		RSP.halt = TRUE;
#ifdef DEBUG
		DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X Fast3D_EndDL\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
		DebugMsg( DEBUG_HANDLED, "\tEnding Display List Execution\r\n" );
#endif
	}
}

void Fast3D_SetOtherMode_L()
{
	DWORD shift = (RSP.cmd0 >> 8) & 0xFF;
	DWORD length = RSP.cmd0 & 0xFF;
	DWORD mask = ((1 << length) - 1) << shift;
	DWORD old = RDP.otherMode_L;

	RDP.otherMode_L &= ~mask;
	RDP.otherMode_L |= RSP.cmd1 & mask;
	RDP.changed.otherMode_L |= old ^ RDP.otherMode_L;

#ifdef DEBUG
	switch (shift)
	{
		case OTHERMODE_L_ALPHACOMPARE:
			DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X Fast3D_SetOtherMode_L\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
			DebugMsg( DEBUG_HANDLED, "\tAlphaCompare: %s%s%s\r\n",
				(RSP.cmd1 & ALPHACOMPARE_NONE)			? "None, "			: "",
				(RSP.cmd1 & ALPHACOMPARE_THRESHOLD)		? "Threshold, "		: "",
				(RSP.cmd1 & ALPHACOMPARE_DITHER)		? "Dither, "		: "" );
			break;

		case OTHERMODE_L_RENDERMODE:
			if (RDP.otherMode_L & (RENDERMODE_Z_COMPARE | RENDERMODE_Z_UPDATE | RENDERMODE_ZMODE_DECAL))
				RSP.update |= UPDATE_ZMODE;	

			if (RDP.otherMode_L & RENDERMODE_FORCE_BL)
				RSP.update = RSP.update;
			DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X Fast3D_SetOtherMode_L\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
			DebugMsg( DEBUG_HANDLED, "\tRenderMode: %s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s\r\n",
				(RSP.cmd1 & RENDERMODE_AA_ENABLE)		? "AA Enable"		: "",
				(RSP.cmd1 & RENDERMODE_Z_COMPARE)		? ", Z Compare"		: "",
				(RSP.cmd1 & RENDERMODE_Z_UPDATE)		? ", Z Update"		: "",
				(RSP.cmd1 & RENDERMODE_IM_RD)			? ", Im Rd"			: "",
				(RSP.cmd1 & RENDERMODE_CLR_ON_CVG)		? ", Clr on CVG"	: "",
				(RSP.cmd1 & RENDERMODE_CVG_DST_CLAMP)	? ", CVG Dst Clamp" : "",
				(RSP.cmd1 & RENDERMODE_CVG_DST_WRAP)	? ", CVG Dst Wrap"	: "",
				(RSP.cmd1 & RENDERMODE_CVG_DST_FULL)	? ", CVG Dst Full"	: "",
				(RSP.cmd1 & RENDERMODE_CVG_DST_SAVE)	? ", CVG Dst Save"	: "",
				(RSP.cmd1 & RENDERMODE_ZMODE_OPA)		? ", ZMode Opa"		: "",
				(RSP.cmd1 & RENDERMODE_ZMODE_INTER)		? ", ZMode Inter"	: "",
				(RSP.cmd1 & RENDERMODE_ZMODE_XLU)		? ", ZMode XLU"		: "",
				(RSP.cmd1 & RENDERMODE_ZMODE_DECAL)		? ", ZMode Decal"	: "",
				(RSP.cmd1 & RENDERMODE_CVG_X_ALPHA)		? ", CVG X Alpha"	: "",
				(RSP.cmd1 & RENDERMODE_ALPHA_CVG_SEL)	? ", Alpha CVG Sel"	: "",
				(RSP.cmd1 & RENDERMODE_FORCE_BL)		? ", Force Bl"		: "",
				(RSP.cmd1 & RENDERMODE_TEX_EDGE)		? ", Tex Edge"		: "" );

			break;
		case OTHERMODE_L_BLENDER:
				RSP.blender = RDP.otherMode_L & 0xFFFF0000;
			break;
		default:
			DebugMsg( DEBUG_UNKNOWN, "0x%08X: 0x%08X 0x%08X Fast3D_SetOtherMode_L\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
			DebugMsg( DEBUG_UNKNOWN, "\tUnknown SetOtherMode_L type 0x%02X. OtherMode_L = 0x%08X\r\n", shift, RDP.otherMode_L );
			break;
	}
#endif

	RSP.PC[RSP.PCi] += 8;
}

void Fast3D_SetOtherMode_H()
{
	// Update othermode flags
	DWORD shift = (RSP.cmd0 >> 8) & 0xFF;
	DWORD length = RSP.cmd0 & 0xFF;
	DWORD mask = ((1 << length) - 1) << shift;
	DWORD old = RDP.otherMode_H;

	RDP.otherMode_H &= ~mask;
	RDP.otherMode_H |= RSP.cmd1 & mask;
	RDP.changed.otherMode_H |= old ^ RDP.otherMode_H;

#ifdef DEBUG
	switch (shift)
	{
		case OTHERMODE_H_CYCLETYPE:
			DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X Fast3D_SetOtherMode_H\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
			DebugMsg( DEBUG_HANDLED, "\tCycle type changed. OtherMode_H = 0x%08X\r\n", RDP.otherMode_H );
			break;

		case OTHERMODE_H_TEXTFILT:
			//RSP.update |= UPDATE_FILTERING;
			break;

		default:
			DebugMsg( DEBUG_UNKNOWN, "0x%08X: 0x%08X 0x%08X Fast3D_SetOtherMode_H\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
			DebugMsg( DEBUG_UNKNOWN, "\tUnknown SetOtherMode_H type 0x%02X. OtherMode_H = 0x%08X\r\n", shift, RDP.otherMode_H );
			break;
	}
#endif
	RSP.PC[RSP.PCi] += 8;
}

void Fast3D_Texture()
{
	RSP.texture.level = (BYTE)((RSP.cmd0 >> 11) & 0x3);
	RSP.texture.tile = (BYTE)((RSP.cmd0 >> 8) & 0x3);
	RSP.texture.on = (BYTE)(RSP.cmd0 & 0xFF);
	RSP.texture.scaleS = ((RSP.cmd1 >> 16) & 0xFFFF) * 1.5259022e-005;
	RSP.texture.scaleT = (RSP.cmd1 & 0xFFFF) * 1.5259022e-005;

	RSP.textureTile[0] = &RDP.tiles[RSP.texture.tile];
	RSP.textureTile[1] = &RDP.tiles[RSP.texture.tile+1];
	
#ifdef DEBUG
	DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X Fast3D_Texture\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_HANDLED, "\tLevel = %u, Tile = %u, On = %u, ScaleS = %f, ScaleT = %f\r\n", RSP.texture.level, RSP.texture.tile, RSP.texture.on, RSP.texture.scaleS, RSP.texture.scaleT );
#endif

	RSP.PC[RSP.PCi] += 8;
}

void Fast3D_MoveWord()
{
	WORD offset = (WORD)((RSP.cmd0 >> 8) & 0xFFFF);
	BYTE index = (BYTE)(RSP.cmd0 & 0xFF);
	DWORD data = RSP.cmd1;

	switch (index)
	{
		case FAST3D_MOVEWORD_SEGMENT:
			RSP.segment[offset >> 2] = data & 0x00FFFFFF;

#ifdef DEBUG
			DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X Fast3D_MoveWord\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
			DebugMsg( DEBUG_HANDLED, "\tMOVEWORD_SEGMENT: Setting segment %i to 0x%08X\r\n", offset >> 2, data & 0x00FFFFFF );
#endif
			break;

		case FAST3D_MOVEWORD_FOG:
			RSP.fogMultiplier = (float)(SHORT)(data >> 16);
			RSP.fogOffset = (float)(SHORT)(data & 0xFFFF);
			break;

		case FAST3D_MOVEWORD_NUMLIGHT:
			RSP.numLights = (BYTE)(((RSP.cmd1 & 0xFFFF) >> 5) - 1) & 0x7;

#ifdef DEBUG
			DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X Fast3D_MoveWord\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
			DebugMsg( DEBUG_HANDLED, "\tMOVEWORD_NUMLIGHT: Setting number of lights to 0x%08X\r\n", RSP.numLights );
#endif
			break;

		case FAST3D_MOVEWORD_PERSPNORM:
			RSP.perspNorm = (float)(data & 0xFFFF) / 65535.0f;
#ifdef DEBUG
			DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X Fast3D_MoveWord\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
			DebugMsg( DEBUG_HANDLED, "\tMOVEWORD_PERSPNORM: PerpNorm = %f\r\n", RSP.perspNorm );
#endif
			break;

		default:
#ifdef DEBUG
			DebugMsg( DEBUG_UNKNOWN, "0x%08X: 0x%08X 0x%08X Fast3D_MoveWord\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
			DebugMsg( DEBUG_UNKNOWN, "\tUnknown move type %02X\r\n", index );
#endif
			break;
	}
	RSP.PC[RSP.PCi] += 8;
}

void Fast3D_PopMatrix()
{
	if (RSP.modelViewi > 0)
		RSP.modelViewi--;

#ifdef DEBUG
	DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X Fast3D_PopMatrix\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_HANDLED, "\tPopping world matrix\r\n" );
#endif

	RSP.PC[RSP.PCi] += 8;
}

void Fast3D_CullDL()
{
#ifdef DEBUG
		DebugMsg( DEBUG_UNHANDLED, "0x%08X: 0x%08X 0x%08X Fast3D_CullDL\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
		DebugMsg( DEBUG_UNHANDLED, "\tUnhandled\r\n" );
#endif
	RSP.PC[RSP.PCi] += 8;
}

void Fast3D_Tri1()
{
	RSPTriangle		*srcTriangle;
	BYTE	tri[3];

	RSP.numTriangles = 0;
	do
	{
		srcTriangle = (RSPTriangle*)&RSP.cmd1;

		if ((srcTriangle->v0 % 10) && !RSP.forceUCode)
		{
			RSP_SetUCode( UCODE_F3DEX );
			return;
		}
		if ((srcTriangle->v1 % 10) && !RSP.forceUCode)
		{
			RSP_SetUCode( UCODE_F3DEX );
			return;
		}
		if ((srcTriangle->v2 % 10) && !RSP.forceUCode)
		{
			RSP_SetUCode( UCODE_F3DEX );
			return;
		}

/*		RSP.triangleFlags[RSP.numTriangles] = srcTriangle->flag;
		RSP.triangles[RSP.numTriangles].v0 = srcTriangle->v0 / 10;
		RSP.triangles[RSP.numTriangles].v1 = srcTriangle->v1 / 10;
		RSP.triangles[RSP.numTriangles].v2 = srcTriangle->v2 / 10;*/
		tri[0] = srcTriangle->v0 / 10;
		tri[1] = srcTriangle->v1 / 10;
		tri[2] = srcTriangle->v2 / 10;
		OGL_AddTriangle( tri );
		
#ifdef DEBUG
		DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X Fast3D_Tri1\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
		DebugMsg( DEBUG_HANDLED, "\tAdding triangle: %u, %u, %u with flags of 0x%04X to triangle buffer\r\n",
			RSP.triangles[RSP.numTriangles].v0, RSP.triangles[RSP.numTriangles].v1, RSP.triangles[RSP.numTriangles].v2, RSP.triangleFlags[RSP.numTriangles] );

		if (RSP.numTriangles == 32)
		{
			DebugMsg( DEBUG_ERROR, "0x%08X: 0x%08X 0x%08X Fast3D_Tri1\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
			DebugMsg( DEBUG_ERROR, "\tTriangle buffer filled!\r\n" );
		}
#endif

		RSP.numTriangles++;

		RSP.PC[RSP.PCi] += 8;

		RSP.cmd0 = *(DWORD*)&RDRAM[RSP.PC[RSP.PCi]];
		RSP.cmd1 = *(DWORD*)&RDRAM[RSP.PC[RSP.PCi] + 4];
	}
	while (((RSP.cmd0 >> 24) == 0xBF) && (RSP.numTriangles < 32));

#ifdef DEBUG
	DebugMsg( DEBUG_HANDLED, "\tSending %u triangles to OpenGL for drawing\r\n", RSP.numTriangles );
#endif
	OGL_DrawTriangles();

	GFXOp[RSP.cmd0 >> 24]();
}

void Fast3D_Init()
{
	// Set GeometryMode flags
	RSP_GEOMETRYMODE_ZBUFFER			= FAST3D_GEOMETRYMODE_ZBUFFER;
	RSP_GEOMETRYMODE_TEXTURE_ENABLE		= FAST3D_GEOMETRYMODE_TEXTURE_ENABLE;
	RSP_GEOMETRYMODE_SHADE				= FAST3D_GEOMETRYMODE_SHADE;
	RSP_GEOMETRYMODE_SHADING_SMOOTH		= FAST3D_GEOMETRYMODE_SHADING_SMOOTH;
	RSP_GEOMETRYMODE_CULL_FRONT			= FAST3D_GEOMETRYMODE_CULL_FRONT;
	RSP_GEOMETRYMODE_CULL_BACK			= FAST3D_GEOMETRYMODE_CULL_BACK;
	RSP_GEOMETRYMODE_CULL_BOTH			= FAST3D_GEOMETRYMODE_CULL_BOTH;
	RSP_GEOMETRYMODE_FOG				= FAST3D_GEOMETRYMODE_FOG;
	RSP_GEOMETRYMODE_LIGHTING			= FAST3D_GEOMETRYMODE_LIGHTING;
	RSP_GEOMETRYMODE_TEXTURE_GEN		= FAST3D_GEOMETRYMODE_TEXTURE_GEN;
	RSP_GEOMETRYMODE_TEXTURE_GEN_LINEAR = FAST3D_GEOMETRYMODE_TEXTURE_GEN_LINEAR;
	RSP_GEOMETRYMODE_LOD				= FAST3D_GEOMETRYMODE_TEXTURE_GEN_LINEAR;

	GFXOp[0x00] = Fast3D_SPNoOp;
	GFXOp[0x01] = Fast3D_Mtx;
	GFXOp[0x02] = Fast3D_Reserved0;
	GFXOp[0x03] = Fast3D_MoveMem;
	GFXOp[0x04] = Fast3D_Vtx;
	GFXOp[0x05] = Fast3D_Reserved1;
	GFXOp[0x06] = Fast3D_DL;
	GFXOp[0x07] = Fast3D_Reserved2;
	GFXOp[0x08] = Fast3D_Reserved3;
	GFXOp[0x09] = Fast3D_Sprite2D_Base;

	GFXOp[0xB3] = Fast3D_RDPHalf_2;
	GFXOp[0xB4] = Fast3D_RDPHalf_1;
	GFXOp[0xB5] = Fast3D_Line3D;
	GFXOp[0xB6] = Fast3D_ClearGeometryMode;
	GFXOp[0xB7] = Fast3D_SetGeometryMode;
	GFXOp[0xB8] = Fast3D_EndDL;
	GFXOp[0xB9] = Fast3D_SetOtherMode_L;
	GFXOp[0xBA] = Fast3D_SetOtherMode_H;
	GFXOp[0xBB] = Fast3D_Texture;
	GFXOp[0xBC] = Fast3D_MoveWord;
	GFXOp[0xBD] = Fast3D_PopMatrix;
	GFXOp[0xBE] = Fast3D_CullDL;
	GFXOp[0xBF] = Fast3D_Tri1;
}