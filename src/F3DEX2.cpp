#include <windows.h>
#include "gl/gl.h"
#include "glNintendo64().h"
#include "F3DEX2.h"
#include "S2DEX.h"
#include "Debug.h"
#include "N64.h"
#include "RSP.h"	
#include "RDP.h"
#include "3DMath.h"
#include "Math.h"

void F3DEX2_EndDL();

void F3DEX2_NoOp()
{
#ifdef DEBUG
	DebugMsg( DEBUG_HANDLED | DEBUG_IGNORED, "0x%08X: 0x%08X 0x%08X F3DEX2_NoOp\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_HANDLED | DEBUG_IGNORED, "\tIgnored\r\n" );
#endif
	RSP.PC[RSP.PCi] += 8;
}

void F3DEX2_Vtx()
{
	DWORD address = RSP_SegmentAddress(RSP.cmd1);
	BYTE num = ((RSP.cmd0 >> 12) & 0xFF);
	BYTE v0 = ((RSP.cmd0 >> 1) & 0x7F) - num;

	if (v0 + num > 80)
	{
#ifdef DEBUG
		DebugMsg( DEBUG_ERROR | DEBUG_IGNORED, "0x%08X: 0x%08X 0x%08X F3DEX2_Vtx\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
		DebugMsg( DEBUG_ERROR | DEBUG_IGNORED, "\tAttempting to write vertices to invalid location, ignoring\r\n" );
#endif
		RSP.PC[RSP.PCi] += 8;
		return;
	}

#ifdef DEBUG
	DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X F3DEX2_Vtx\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
#endif

	RSP_LoadVertices( address, v0, num );

	RSP.PC[RSP.PCi] += 8;
}

void F3DEX2_ModifyVtx()
{
	WORD	vtx = (RSP.cmd0 & 0xFFFF) >> 1;
	BYTE	where = (RSP.cmd0 >> 16) & 0xFF;
	
	switch (where)
	{
		case F3DEX2_POINT_RGBA:
			RSP.vertices[vtx].r = ((RSP.cmd1 >> 24) & 0xFF) * 0.0039215689f;
			RSP.vertices[vtx].g = ((RSP.cmd1 >> 16) & 0xFF) * 0.0039215689f;
			RSP.vertices[vtx].b = ((RSP.cmd1 >>  8) & 0xFF) * 0.0039215689f;
			RSP.vertices[vtx].a = ((RSP.cmd1      ) & 0xFF) * 0.0039215689f;
			break;
		case F3DEX2_POINT_ST:
			RSP.vertices[vtx].s = (SHORT)((RSP.cmd1 >> 16) & 0xFFFF) * 0.03125f;
			RSP.vertices[vtx].t = (SHORT)(RSP.cmd1 & 0xFFFF) * 0.03125f;
			break;
		case F3DEX2_POINT_XYSCREEN:
			break;
		case F3DEX2_POINT_ZSCREEN:
			break;
	}

	RSP.vertices[vtx].changed = TRUE;

#ifdef DEBUG
	DebugMsg( DEBUG_UNHANDLED, "0x%08X: 0x%08X 0x%08X F3DEX2_ModifyVtx\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_UNHANDLED, "\tUnhandled\r\n" );
#endif
	RSP.PC[RSP.PCi] += 8;
}

void F3DEX2_CullDL()
{
	WORD vStart = (RSP.cmd0 & 0xFFFF) >> 1;
	WORD vEnd = (RSP.cmd1 & 0xFFFF) >> 1;
	float x, y, z, xClipDir, yClipDir, zClipDir;
	BOOL xPass, yPass, zPass;
	int i;

 	/*for (i = vStart; i <= vEnd; i++)
	{
 		x = RSP.vertices[i].x / RSP.vertices[i].w;
		y = RSP.vertices[i].y / RSP.vertices[i].w;
		z = RSP.vertices[i].z / RSP.vertices[i].w;

//		if (((RSP.vertices[i].x > -RSP.vertices[i].w) && (RSP.vertices[i].x <= RSP.vertices[i].w)) &&
//			((RSP.vertices[i].y > -RSP.vertices[i].w) && (RSP.vertices[i].y <= RSP.vertices[i].w)) &&
		if	((z >= 0.0f) && (z <= 1.0f))
		{
			RSP.PC[RSP.PCi] += 8;
			return;
		}
	}*/

	xClipDir = 0.0f;
	yClipDir = 0.0f;
	zClipDir = 0.0f;

	xPass = FALSE;
	yPass = FALSE;
	zPass = FALSE;
	xClipDir = 0.0f;

	for (i = vStart; i <= vEnd; i++)
	{
 		x = RSP.vertices[i].x / RSP.vertices[i].w;

		if ((x >= -1.0f) && (x <= 1.0f))
			xPass = TRUE;
		else
		{
			if (xClipDir == 0.0f)
				xClipDir = x / fabs( x );
			else if (xClipDir != (x / fabs( x )))
				xPass = TRUE;
		}

 		y = RSP.vertices[i].y / RSP.vertices[i].w;

		if ((y >= -1.0f) && (y <= 1.0f))
			yPass = TRUE;
		else
		{
			if (yClipDir == 0.0f)
				yClipDir = y / fabs( y );
			else if (yClipDir != (y / fabs( y )))
				yPass = TRUE;
		}

 		z = RSP.vertices[i].z / RSP.vertices[i].w;

		if ((RSP.vertices[i].z >= 0.0f) && (z <= 1.0f))
			zPass = TRUE;
		else
		{
			if (zClipDir == 0.0f)
				zClipDir = RSP.vertices[i].z / fabs( RSP.vertices[i].z );
			else if (zClipDir != (RSP.vertices[i].z / fabs( RSP.vertices[i].z )))
				zPass = TRUE;
		}
	}

	if (!xPass || !yPass || !zPass)
	{
#ifdef DEBUG
		DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X F3DEX2_CullDL\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
		DebugMsg( DEBUG_HANDLED, "\tDL culled, ending...\r\n" );
#endif

		F3DEX2_EndDL();
		return;
	}

	RSP.PC[RSP.PCi] += 8;

#ifdef DEBUG
		DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X F3DEX2_CullDL\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
		DebugMsg( DEBUG_HANDLED, "\tDL passed, continuing...\r\n" );
#endif
}

void F3DEX2_Branch_Z()
{
	DWORD address = RSP_SegmentAddress( *(DWORD*)&RDRAM[RSP.PC[RSP.PCi]-4] );

	WORD vertex = (RSP.cmd0 & 0xFFF) >> 1;
	float zval = RSP.cmd1 / 1000.0f;

	if ((RSP.vertices[vertex].z / RSP.vertices[vertex].w) < zval)
	{
#ifdef DEBUG
	DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X F3DEX2_Branch_Z\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_HANDLED, "\tZ less than test value, branching...\r\n" );
#endif
		RSP.PC[RSP.PCi] = address;
		return;
	}

#ifdef DEBUG
	DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X F3DEX2_Branch_Z\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_HANDLED, "\tZ greater than test value, continuing...\r\n" );
#endif
	RSP.PC[RSP.PCi] += 8;
}

void F3DEX2_Tri1()
{
	BYTE tri[3];
	do
	{
		tri[0] = (RSP.cmd0 >> 17) & 0x7F;
		tri[1] = (RSP.cmd0 >>  9) & 0x7F;
		tri[2] = (RSP.cmd0 >>  1) & 0x7F;
		OGL_AddTriangle( tri );
		
#ifdef DEBUG
		DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X F3DEX2_Tri1\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
		DebugMsg( DEBUG_HANDLED, "\tAdding triangle: %u, %u, %u with flags of 0x%04X to triangle buffer\r\n",
			tri[0], tri[1], tri[2] );

		if (OGL.numVertices == 255)
		{
			DebugMsg( DEBUG_ERROR, "0x%08X: 0x%08X 0x%08X F3DEX2_Tri1\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
			DebugMsg( DEBUG_ERROR, "\tTriangle buffer filled!\r\n" );
		}
#endif
		RSP.PC[RSP.PCi] += 8;

		RSP.cmd0 = *(DWORD*)&RDRAM[RSP.PC[RSP.PCi]];
		RSP.cmd1 = *(DWORD*)&RDRAM[RSP.PC[RSP.PCi] + 4];
	}
	while (((RSP.cmd0 >> 24) == 0x05) && (OGL.numVertices < 255));

	if (((RSP.cmd0 >> 24) != 0x06) && ((RSP.cmd0 >> 24) != 0x07))
	{
#ifdef DEBUG
		DebugMsg( DEBUG_HANDLED, "\tSending %u triangles to OpenGL for drawing\r\n", OGL.numVertices / 3 );
#endif
		OGL_DrawTriangles();
	}

	GFXOp[RSP.cmd0 >> 24]();
}

void F3DEX2_Tri2()
{
	BYTE tri[3];

	do
	{
		tri[0] = (RSP.cmd0 >> 17) & 0x7F;
		tri[1] = (RSP.cmd0 >>  9) & 0x7F;
		tri[2] = (RSP.cmd0 >>  1) & 0x7F;
		OGL_AddTriangle( tri );

#ifdef DEBUG
		DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X F3DEX2_Tri2\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
		DebugMsg( DEBUG_HANDLED, "\tAdding triangle: %u, %u, %u with flags of 0x%04X to triangle buffer\r\n",
			tri[0], tri[1], tri[2] );

		if (OGL.numVertices == 255)
		{
			DebugMsg( DEBUG_ERROR, "0x%08X: 0x%08X 0x%08X F3DEX2_Tri2\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
			DebugMsg( DEBUG_ERROR, "\tTriangle buffer filled!\r\n" );
		}
#endif

		tri[0] = (RSP.cmd1 >> 17) & 0x7F;
		tri[1] = (RSP.cmd1 >>  9) & 0x7F;
		tri[2] = (RSP.cmd1 >>  1) & 0x7F;
		OGL_AddTriangle( tri );

#ifdef DEBUG
		DebugMsg( DEBUG_HANDLED, "\tAdding triangle: %u, %u, %u with flags of 0x%04X to triangle buffer\r\n",
			tri[0], tri[1], tri[2] );

		if (OGL.numVertices == 255)
		{
			DebugMsg( DEBUG_ERROR, "0x%08X: 0x%08X 0x%08X F3DEX2_Tri2\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
			DebugMsg( DEBUG_ERROR, "\tTriangle buffer filled!\r\n" );
		}
#endif

		RSP.PC[RSP.PCi] += 8;

		RSP.cmd0 = *(DWORD*)&RDRAM[RSP.PC[RSP.PCi]];
 		RSP.cmd1 = *(DWORD*)&RDRAM[RSP.PC[RSP.PCi] + 4];
	}
	while (((RSP.cmd0 >> 24) == 0x06) && (OGL.numVertices < 255));

	if (((RSP.cmd0 >> 24) != 0x05) && ((RSP.cmd0 >> 24) != 0x07))
	{
#ifdef DEBUG
		DebugMsg( DEBUG_HANDLED, "\tSending %u triangles to OpenGL for drawing\r\n", OGL.numVertices / 3 );
#endif
		OGL_DrawTriangles();
	}

	GFXOp[RSP.cmd0 >> 24]();
}

void F3DEX2_Quad()
{
	BYTE tri[3];
	do
	{
		tri[0] = (RSP.cmd0 >> 17) & 0x7F;
		tri[1] = (RSP.cmd0 >>  9) & 0x7F;
		tri[2] = (RSP.cmd0 >>  1) & 0x7F;
		OGL_AddTriangle( tri );

		RSP.numTriangles++;
		tri[0] = (RSP.cmd1 >> 17) & 0x7F;
		tri[1] = (RSP.cmd1 >>  9) & 0x7F;
		tri[2] = (RSP.cmd1 >>  1) & 0x7F;
		OGL_AddTriangle( tri );

#ifdef DEBUG
		DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X F3DEX2_Quad\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
		DebugMsg( DEBUG_HANDLED, "\tAdding quad: %u, %u, %u, %u to triangle buffer\r\n",
			(RSP.cmd1 >> 25) & 0x7F, (RSP.cmd1 >> 17) & 0x7F, (RSP.cmd1 >>  9) & 0x7F, (RSP.cmd1 >>  1) & 0x7F );

		if (OGL.numVertices == 255)
		{
			DebugMsg( DEBUG_ERROR, "0x%08X: 0x%08X 0x%08X F3DEX2_Quad\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
			DebugMsg( DEBUG_ERROR, "\tTriangle buffer filled!\r\n" );
		}
#endif
		RSP.numTriangles = 0;

 		RSP.PC[RSP.PCi] += 8;

		RSP.cmd0 = *(DWORD*)&RDRAM[RSP.PC[RSP.PCi]];
		RSP.cmd1 = *(DWORD*)&RDRAM[RSP.PC[RSP.PCi] + 4];
	}
	while (((RSP.cmd0 >> 24) == 0x07) && (OGL.numVertices < 255));

	if (((RSP.cmd0 >> 24) != 0x05) && ((RSP.cmd0 >> 24) != 0x06))
	{
#ifdef DEBUG
		DebugMsg( DEBUG_HANDLED, "\tSending %u triangles to OpenGL for drawing\r\n", OGL.numVertices / 3 );
#endif
		OGL_DrawTriangles();
	}

	GFXOp[RSP.cmd0 >> 24]();
}

void F3DEX2_Line3D()
{
#ifdef DEBUG
	DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X F3DEX2_Line3D\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_HANDLED, "\tUnhandled\r\n" );
#endif
	RSP.PC[RSP.PCi] += 8;
}

void F3DEX2_Special_3()
{
#ifdef DEBUG
	DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X F3DEX2_Special_3\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_HANDLED, "\tUnhandled\r\n" );
#endif
	RSP.PC[RSP.PCi] += 8;
}

void F3DEX2_Special_2()
{
#ifdef DEBUG
	DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X F3DEX2_Special_2\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_HANDLED, "\tUnhandled\r\n" );
#endif
	RSP.PC[RSP.PCi] += 8;
}

void F3DEX2_Special_1()
{
#ifdef DEBUG
	DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X F3DEX2_Special_1\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_HANDLED, "\tUnhandled\r\n" );
#endif
	RSP.PC[RSP.PCi] += 8;
}

void F3DEX2_DMA_IO()
{
#ifdef DEBUG
	DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X F3DEX2_DMA_IO\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_HANDLED, "\tUnhandled\r\n" );
#endif
	RSP.PC[RSP.PCi] += 8;
}

void F3DEX2_Texture()
{
	RSP.texture.level = ((RSP.cmd0 >> 11) & 0x3);
	RSP.texture.tile = ((RSP.cmd0 >> 8) & 0x3);
	RSP.texture.on = (RSP.cmd0 & 0xFF);
	RSP.texture.scaleS = ((RSP.cmd1 >> 16) & 0xFFFF) * 1.5259022e-005;
	RSP.texture.scaleT = (RSP.cmd1 & 0xFFFF) * 1.5259022e-005;

	RSP.textureTile[0] = &RDP.tiles[RSP.texture.tile];
	RSP.textureTile[1] = &RDP.tiles[RSP.texture.tile+1];

#ifdef DEBUG
	DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X F3DEX2_Texture\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_HANDLED, "\tLevel = %u, Tile = %u, On = %u, ScaleS = %f, ScaleT = %f\r\n", RSP.texture.level, RSP.texture.tile, RSP.texture.on, RSP.texture.scaleS, RSP.texture.scaleT );
#endif

	RSP.PC[RSP.PCi] += 8;
}

void F3DEX2_PopMtx()
{
	if (RSP.modelViewi > 0)
		RSP.modelViewi--;

	RSP.update |= UPDATE_COMBINEDMATRIX | UPDATE_LIGHTS;

#ifdef DEBUG
	DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X F3DEX2_PopMatrix\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_HANDLED, "\tPopping model view matrix\r\n" );
#endif

	RSP.PC[RSP.PCi] += 8;
}

void F3DEX2_GeometryMode()
{
	DWORD old = RSP.geometryMode;
	RSP.geometryMode = (RSP.geometryMode & (RSP.cmd0 & 0x00FFFFFF)) | RSP.cmd1;
	RSP.changed.geometryMode |= old ^ RSP.geometryMode;

#ifdef DEBUG
	DebugMsg( DEBUG_HANDLED | DEBUG_UNHANDLED, "0x%08X: 0x%08X 0x%08X F3DEX2_GeometryMode\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_HANDLED | DEBUG_UNHANDLED, "\tGeometry Mode = 0x0x%08X\r\n", RSP.geometryMode );
#endif
	RSP.PC[RSP.PCi] += 8;
}

void F3DEX2_Mtx()
{
	// The N64 stores the matrix values as s15.16 fixed-point numbers
	// It stores them in a strange order as well...

	BYTE command = (RSP.cmd0 & 0xFF)^F3DEX2_MTX_PUSH;

	DWORD address = RSP_SegmentAddress(RSP.cmd1);

	float mtx[4][4];

	RSP_LoadMatrix( mtx, address );

	if (command & F3DEX2_MTX_PROJECTION)
	{
        if (command & F3DEX2_MTX_LOAD)
			CopyMatrix( RSP.projectionMtx, mtx );
		else
			MultMatrix( RSP.projectionMtx, mtx );
	}
	else
	{
		if ((command & F3DEX2_MTX_PUSH) && (RSP.modelViewi < 9))
		{
			CopyMatrix( RSP.modelViewStack[RSP.modelViewi+1], RSP.modelViewStack[RSP.modelViewi] );
			RSP.modelViewi++;
		}

		if (command & F3DEX2_MTX_LOAD)
			CopyMatrix( RSP.modelViewStack[RSP.modelViewi], mtx );
		else
			MultMatrix( RSP.modelViewStack[RSP.modelViewi], mtx );

		RSP.update |= UPDATE_LIGHTS;
	}

	RSP.update |= UPDATE_COMBINEDMATRIX;

#ifdef DEBUG
	DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X F3DEX2_Mtx\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_HANDLED, "\t%s%s%s\r\n", (command & F3DEX2_MTX_PUSH) ? "Push " : "Don't push ",
											 (command & F3DEX2_MTX_PROJECTION) ? "projection matrix " : "world matrix ",
											 (command & F3DEX2_MTX_LOAD) ? "and load:" : "and multiply by:" );
	DebugMsg( DEBUG_HANDLED, "\t%#15.6f %#15.6f %#15.6f %#15.6f\r\n", mtx[0][0], mtx[1][0], mtx[2][0], mtx[3][0] );
	DebugMsg( DEBUG_HANDLED, "\t%#15.6f %#15.6f %#15.6f %#15.6f\r\n", mtx[0][1], mtx[1][1], mtx[2][1], mtx[3][1] );
	DebugMsg( DEBUG_HANDLED, "\t%#15.6f %#15.6f %#15.6f %#15.6f\r\n", mtx[0][2], mtx[1][2], mtx[2][2], mtx[3][2] );
	DebugMsg( DEBUG_HANDLED, "\t%#15.6f %#15.6f %#15.6f %#15.6f\r\n", mtx[0][3], mtx[1][3], mtx[2][3], mtx[3][3] );
#endif

	RSP.PC[RSP.PCi] += 8;
}

void F3DEX2_MoveWord()
{
	DWORD data = RSP.cmd1;
	WORD offset = (WORD)(RSP.cmd0 & 0xFFFF);
	BYTE index = ((RSP.cmd0 >> 16) & 0xFF);

	switch(index)
	{
		case F3DEX2_MOVEWORD_SEGMENT:
			
			RSP.segment[(offset >> 2) & 0xF] = data & 0x00FFFFFF;

#ifdef DEBUG
			DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X F3DEX2_MoveWord\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
			DebugMsg( DEBUG_HANDLED, "\tMOVEWORD_SEGMENT: Setting segment %i to 0x%08X\r\n", ((offset >> 2) & 0xF), data );
#endif
			break;

		case F3DEX2_MOVEWORD_FOG:
			RSP.fogMultiplier = (float)(SHORT)(data >> 16);
			RSP.fogOffset = (float)(SHORT)(data & 0xFFFF);
			break;

		case F3DEX2_MOVEWORD_LIGHTCOL:
			BYTE light;
			light = offset / 24;
			RSP.lights[light].r = ((data >> 24) & 0xFF) * 0.0039215689f;
			RSP.lights[light].g = ((data >> 16) & 0xFF) * 0.0039215689f;
			RSP.lights[light].b = ((data >> 8) & 0xFF) * 0.0039215689f;

			break;

		case F3DEX2_MOVEWORD_NUMLIGHT:
			RSP.numLights = data / 24;

#ifdef DEBUG
			DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X F3DEX2_MoveWord\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
			DebugMsg( DEBUG_HANDLED, "\tMOVEWORD_NUMLIGHT: Setting number of lights to 0x%08X\r\n", RSP.numLights );
#endif
			break;

		case F3DEX2_MOVEWORD_PERSPNORM:
			RSP.perspNorm = (float)(data & 0xFFFF) / 65535.0f;
			break;

			// handled in movemem
		case F3DEX2_MOVEWORD_FORCEMTX:
			break;

		default:
#ifdef DEBUG
			DebugMsg( DEBUG_UNKNOWN, "0x%08X: 0x%08X 0x%08X F3DEX2_MoveWord\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
			DebugMsg( DEBUG_UNKNOWN, "\tUnknown move index %02X\r\n", index );
#endif
			break;
	}
	RSP.PC[RSP.PCi] += 8;
}

void F3DEX2_MoveMem()
{
	BYTE type = (RSP.cmd0 & 0xFF);
	WORD size = ((RSP.cmd0 >> 16) & 0xFF) + 8;
	WORD offset = ((RSP.cmd0 >> 5) & 0x7F8);
	DWORD address = RSP_SegmentAddress(RSP.cmd1);

	switch (type)
	{
		case F3DEX2_MOVEMEM_VIEWPORT:
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
			DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X F3DEX2_MoveMem\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
			DebugMsg( DEBUG_HANDLED, "\tMOVEMEM_VIEWPORT: scale[0] = %f, scale[1] = %f, scale[2] = %f, scale[3] = %f\r\n", scale[0], scale[1], scale[2], scale[3] );
			DebugMsg( DEBUG_HANDLED, "\t                  trans[0] = %f, trans[1] = %f, trans[2] = %f, trans[3] = %f\r\n", trans[0], trans[1], trans[2], trans[3] );
#endif
			break;

		case F3DEX2_MOVEMEM_LIGHT:
			BYTE light;
			RSPLight *tempLight;

			light = (offset / 24) - 2;
			if (light < 8)
			{
				tempLight = (RSPLight*)&RDRAM[address];

				RSP.lights[light].x = tempLight->x;
				RSP.lights[light].y = tempLight->y;
				RSP.lights[light].z = tempLight->z;

				RSP.lights[light].r = tempLight->r * 0.0039215689f;
				RSP.lights[light].g = tempLight->g * 0.0039215689f;
				RSP.lights[light].b = tempLight->b * 0.0039215689f;

				RSP.update |= UPDATE_LIGHTS;
			}
			
#ifdef DEBUG
			DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X F3DEX2_MoveMem\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
			DebugMsg( DEBUG_HANDLED, "\tMOVEMEM_LIGHT%i: R = %u, G = %u, B = %u, X = %i, Y = %i, Z = %i\r\n", light,
				RSP.lights[light].r, RSP.lights[light].g, RSP.lights[light].b, RSP.lights[light].x, RSP.lights[light].y, RSP.lights[light].z );
#endif
			break;		
		case F3DEX2_MOVEMEM_MATRIX:
			RSP_LoadMatrix( RSP.combinedMtx, address );
			RSP.update &= ~UPDATE_COMBINEDMATRIX;
			break;

		default:
#ifdef DEBUG
			DebugMsg( DEBUG_UNKNOWN, "0x%08X: 0x%08X 0x%08X F3DEX2_MoveMem\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
			DebugMsg( DEBUG_UNKNOWN, "\tUnknown MoveMem command %i\r\n", type );
#endif
			break;
	}
	RSP.PC[RSP.PCi] += 8;
}

void F3DEX2_Load_uCode()
{
#ifdef DEBUG
	DebugMsg( DEBUG_UNHANDLED, "0x%08X: 0x%08X 0x%08X F3DEX2_Load_uCode\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_UNHANDLED, "\tUnhandled\r\n" );
#endif
	RSP.PC[RSP.PCi] += 8;
}

void F3DEX2_DL()
{
	BYTE push = ((RSP.cmd0 >> 16) & 0xFF);
	DWORD address = RSP_SegmentAddress( RSP.cmd1 );

	if (push == F3DEX2_DL_PUSH)
	{
		if (RSP.PCi < 7)
		{
#ifdef DEBUG
		DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X F3DEX2_DL\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
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
		DebugMsg( DEBUG_ERROR, "0x%08X: 0x%08X 0x%08X F3DEX2_DL\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
		DebugMsg( DEBUG_ERROR, "\tPC Stack overflow, ending display list execution\r\n" );
#endif
		}
	}
	else
	{
		RSP.PC[RSP.PCi] = address;
#ifdef DEBUG
		DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X F3DEX2_DL\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
		DebugMsg( DEBUG_HANDLED, "\tExecuting DL at 0x%08X\r\n", RSP.PC[RSP.PCi] );
#endif
	}
}

void F3DEX2_EndDL()
{
	if (RSP.PCi > 0)
	{
#ifdef DEBUG
		DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X F3DEX2_EndDL\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
		DebugMsg( DEBUG_HANDLED, "\tPopping PC Stack, PCi=%i\r\n", RSP.PCi-1 );
#endif
		RSP.PCi--;
	}
	else
	{
		RSP.halt = TRUE;
#ifdef DEBUG
		DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X F3DEX2_EndDL\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
		DebugMsg( DEBUG_HANDLED, "\tEnding Display List Execution\r\n" );
#endif
	}
}

void F3DEX2_SPNoOp()
{
#ifdef DEBUG
	DebugMsg( DEBUG_HANDLED | DEBUG_IGNORED, "0x%08X: 0x%08X 0x%08X F3DEX2_SPNoOp\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_HANDLED | DEBUG_IGNORED, "\tIgnored\r\n" );
#endif
	RSP.PC[RSP.PCi] += 8;
}

void F3DEX2_RDPHalf_1()
{
#ifdef DEBUG
	DebugMsg( DEBUG_UNHANDLED | DEBUG_IGNORED, "0x%08X: 0x%08X 0x%08X F3DEX2_RDPHalf_2\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_UNHANDLED | DEBUG_IGNORED, "\tIgnored\r\n" );
#endif
	RSP.PC[RSP.PCi] += 8;
}

void F3DEX2_SetOtherMode_L()
{
	BYTE length = (RSP.cmd0 & 0xFF) + 1;
	BYTE shift = 32 - ((RSP.cmd0 >> 8) & 0xFF) - length;
	DWORD mask = (((1 << length) - 1) << shift);

	DWORD old = RDP.otherMode_L;

	// Update othermode flags
	RDP.otherMode_L &= ~mask;
	RDP.otherMode_L |= RSP.cmd1 & mask;

	RDP.changed.otherMode_L |= old ^ RDP.otherMode_L;

#ifdef DEBUG
	switch (shift)
	{
		case OTHERMODE_L_ALPHACOMPARE:
			DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X F3DEX2_SetOtherMode_L\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
			DebugMsg( DEBUG_HANDLED, "\tAlphaCompare: %s%s%s\r\n",
				(RSP.cmd1 & ALPHACOMPARE_NONE)			? "None, "			: "",
				(RSP.cmd1 & ALPHACOMPARE_THRESHOLD)		? "Threshold, "		: "",
				(RSP.cmd1 & ALPHACOMPARE_DITHER)		? "Dither, "		: "" );
			break;

		case OTHERMODE_L_RENDERMODE:
//			if (RDP.otherMode_L & (RENDERMODE_Z_COMPARE | RENDERMODE_Z_UPDATE | RENDERMODE_ZMODE_DECAL))
//				RSP.update |= UPDATE_ZMODE;	

//			if (RDP.otherMode_L & RENDERMODE_FORCE_BL)
//				RSP.update = RSP.update;
			DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X F3DEX2_SetOtherMode_L\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
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
			DebugMsg( DEBUG_UNKNOWN, "0x%08X: 0x%08X 0x%08X F3DEX2_SetOtherMode_L\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
			DebugMsg( DEBUG_UNKNOWN, "\tUnknown SetOtherMode_L type 0x%02X. OtherMode_L = 0x%08X\r\n", shift, RDP.otherMode_L );
			break;
	}
#endif

	RSP.PC[RSP.PCi] += 8;
}

void F3DEX2_SetOtherMode_H()
{
	// Update othermode flags
	BYTE length = (RSP.cmd0 & 0xFF) + 1;
 	BYTE shift = 32 - ((RSP.cmd0 >> 8) & 0xFF) - length;

	DWORD mask = ((1 << length) - 1) << shift;
	DWORD old = RDP.otherMode_H;

	RDP.otherMode_H &= ~mask;
	RDP.otherMode_H |= RSP.cmd1 & mask;
	RDP.changed.otherMode_H |= old ^ RDP.otherMode_H;

#ifdef DEBUG
	switch (shift)
	{
		case OTHERMODE_H_CYCLETYPE:
			DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X F3DEX2_SetOtherMode_H\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
			DebugMsg( DEBUG_HANDLED, "\tCycle type changed. OtherMode_H = 0x%08X\r\n", RDP.otherMode_H );
			break;

		case OTHERMODE_H_TEXTFILT:
			//RSP.update |= UPDATE_FILTERING;
			break;

		default:
			DebugMsg( DEBUG_UNKNOWN, "0x%08X: 0x%08X 0x%08X F3DEX2_SetOtherMode_H\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
			DebugMsg( DEBUG_UNKNOWN, "\tUnknown SetOtherMode_H type 0x%02X. OtherMode_H = 0x%08X\r\n", shift, RDP.otherMode_H );
			break;
	}
#endif
	RSP.PC[RSP.PCi] += 8;
}

void F3DEX2_RDPHalf_2()
{
#ifdef DEBUG
	DebugMsg( DEBUG_UNHANDLED | DEBUG_IGNORED, "0x%08X: 0x%08X 0x%08X F3DEX2_RDPHalf_1\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_UNHANDLED | DEBUG_IGNORED, "\tIgnored\r\n" );
#endif
	RSP.PC[RSP.PCi] += 8;
}

void F3DEX2_Init()
{
	// Set GeometryMode flags
	RSP_GEOMETRYMODE_ZBUFFER			= F3DEX2_GEOMETRYMODE_ZBUFFER;
	RSP_GEOMETRYMODE_TEXTURE_ENABLE		= F3DEX2_GEOMETRYMODE_TEXTURE_ENABLE;
	RSP_GEOMETRYMODE_SHADE				= F3DEX2_GEOMETRYMODE_SHADE;
	RSP_GEOMETRYMODE_SHADING_SMOOTH		= F3DEX2_GEOMETRYMODE_SHADING_SMOOTH;
	RSP_GEOMETRYMODE_CULL_FRONT			= F3DEX2_GEOMETRYMODE_CULL_FRONT;
	RSP_GEOMETRYMODE_CULL_BACK			= F3DEX2_GEOMETRYMODE_CULL_BACK;
	RSP_GEOMETRYMODE_CULL_BOTH			= F3DEX2_GEOMETRYMODE_CULL_BOTH;
	RSP_GEOMETRYMODE_FOG				= F3DEX2_GEOMETRYMODE_FOG;
	RSP_GEOMETRYMODE_LIGHTING			= F3DEX2_GEOMETRYMODE_LIGHTING;
	RSP_GEOMETRYMODE_TEXTURE_GEN		= F3DEX2_GEOMETRYMODE_TEXTURE_GEN;
	RSP_GEOMETRYMODE_TEXTURE_GEN_LINEAR = F3DEX2_GEOMETRYMODE_TEXTURE_GEN_LINEAR;
	RSP_GEOMETRYMODE_LOD				= F3DEX2_GEOMETRYMODE_TEXTURE_GEN_LINEAR;

	GFXOp[0x00] = F3DEX2_NoOp;
	GFXOp[0x01] = F3DEX2_Vtx;
	GFXOp[0x02] = F3DEX2_ModifyVtx;
	GFXOp[0x03] = F3DEX2_CullDL;
	GFXOp[0x04] = F3DEX2_Branch_Z;
	GFXOp[0x05] = F3DEX2_Tri1;
	GFXOp[0x06] = F3DEX2_Tri2;
	GFXOp[0x07] = F3DEX2_Quad;
	GFXOp[0x08] = F3DEX2_Line3D;

	GFXOp[0x0A] = S2DEX_BG_Copy;

	GFXOp[0xD3] = F3DEX2_Special_3;
	GFXOp[0xD4] = F3DEX2_Special_2;
	GFXOp[0xD5] = F3DEX2_Special_1;
	GFXOp[0xD6] = F3DEX2_DMA_IO;
	GFXOp[0xD7] = F3DEX2_Texture;
	GFXOp[0xD8] = F3DEX2_PopMtx;
	GFXOp[0xD9] = F3DEX2_GeometryMode;
	GFXOp[0xDA] = F3DEX2_Mtx;
	GFXOp[0xDB] = F3DEX2_MoveWord;
	GFXOp[0xDC] = F3DEX2_MoveMem;
	GFXOp[0xDD] = F3DEX2_Load_uCode;
	GFXOp[0xDE] = F3DEX2_DL;
	GFXOp[0xDF] = F3DEX2_EndDL;
	GFXOp[0xE0] = F3DEX2_SPNoOp;
	GFXOp[0xE1] = F3DEX2_RDPHalf_1;
	GFXOp[0xE2] = F3DEX2_SetOtherMode_L;
	GFXOp[0xE3] = F3DEX2_SetOtherMode_H;
	GFXOp[0xF1] = F3DEX2_RDPHalf_2;
}