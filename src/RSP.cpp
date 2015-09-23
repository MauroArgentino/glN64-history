#include <windows.h>
#include <math.h>
#include "glNintendo64().h"
#include "Debug.h"
#include "RSP.h"
#include "RDP.h"
#include "N64.h"
#include "Fast3D.h"
#include "F3DEX.h"
#include "F3DEX2.h"
#include "3DMath.h"
#include "textures.h"
#include "Config.h"

// Here since they change from ucode to ucode
DWORD RSP_GEOMETRYMODE_ZBUFFER;
DWORD RSP_GEOMETRYMODE_TEXTURE_ENABLE;
DWORD RSP_GEOMETRYMODE_SHADE;
DWORD RSP_GEOMETRYMODE_SHADING_SMOOTH;
DWORD RSP_GEOMETRYMODE_CULL_FRONT;
DWORD RSP_GEOMETRYMODE_CULL_BACK;
DWORD RSP_GEOMETRYMODE_CULL_BOTH;
DWORD RSP_GEOMETRYMODE_FOG;
DWORD RSP_GEOMETRYMODE_LIGHTING;
DWORD RSP_GEOMETRYMODE_TEXTURE_GEN;
DWORD RSP_GEOMETRYMODE_TEXTURE_GEN_LINEAR;
DWORD RSP_GEOMETRYMODE_LOD;

RSPInfo		RSP;

void RSP_LoadMatrix( float mtx[4][4], DWORD address )
{
	float recip = 1.5258789e-05f;

	__asm {
		mov		esi, dword ptr [RDRAM];
		add		esi, dword ptr [address];
		mov		edi, dword ptr [mtx];

		mov		ecx, 4
LoadLoop:
		fild	word ptr [esi+02h]
		movzx	eax, word ptr [esi+22h]
		mov		dword ptr [edi], eax
		fild	dword ptr [edi]
		fmul	dword ptr [recip]
		fadd
		fstp	dword ptr [edi]

		fild	word ptr [esi+00h]
		movzx	eax, word ptr [esi+20h]
		mov		dword ptr [edi+04h], eax
		fild	dword ptr [edi+04h]
		fmul	dword ptr [recip]
		fadd
		fstp	dword ptr [edi+04h]

		fild	word ptr [esi+06h]
		movzx	eax, word ptr [esi+26h]
		mov		dword ptr [edi+08h], eax
		fild	dword ptr [edi+08h]
		fmul	dword ptr [recip]
		fadd
		fstp	dword ptr [edi+08h]

		fild	word ptr [esi+04h]
		movzx	eax, word ptr [esi+24h]
		mov		dword ptr [edi+0Ch], eax
		fild	dword ptr [edi+0Ch]
		fmul	dword ptr [recip]
		fadd
		fstp	dword ptr [edi+0Ch]

		add		esi, 08h
		add		edi, 10h
		loop	LoadLoop
	}
}

void RSP_UpdateLights()
{
	float mtx[4][4];
	float vec[3][3];
	int i;

	for (i = 0; i < RSP.numLights; i++)
	{
		CopyMatrix( mtx, RSP.modelViewStack[RSP.modelViewi] );
		Transpose3x3Matrix( mtx );
        RSP.lights[i].mx = RSP.lights[i].x;
		RSP.lights[i].my = RSP.lights[i].y;
		RSP.lights[i].mz = RSP.lights[i].z;

		TransformVector( &RSP.lights[i].mx, mtx );
		Normalize( &RSP.lights[i].mx );
	}

	RSP.update &= ~UPDATE_LIGHTS;
}

void RSP_LightVertex( BYTE v )
{
	int i;
	float intensity;
	float r, g, b;

	r = RSP.lights[RSP.numLights].r;
	g = RSP.lights[RSP.numLights].g;
	b = RSP.lights[RSP.numLights].b;

	for (i = 0; i < RSP.numLights; i++)
	{
		intensity = DotProduct( &RSP.vertices[v].nx, &RSP.lights[i].mx );
 
		if (intensity < 0.0f) intensity = 0.0f;

		r += RSP.lights[i].r * intensity;
		g += RSP.lights[i].g * intensity;
		b += RSP.lights[i].b * intensity;
	}

	RSP.vertices[v].r = r;
	RSP.vertices[v].g = g;
	RSP.vertices[v].b = b;
}

void RSP_CombineMatrices()
{
	CopyMatrix( RSP.combinedMtx, RSP.projectionMtx );
	MultMatrix( RSP.combinedMtx, RSP.modelViewStack[RSP.modelViewi] );

	RSP.update &= ~UPDATE_COMBINEDMATRIX;
}

void RSP_Unknown()
{
	if (RSP_DetectUCode())
		return;

#ifdef DEBUG
	DebugMsg( DEBUG_UNKNOWN, "0x%08X: 0x%08X 0x%08X RSP_Unknown\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
	DebugMsg( DEBUG_UNKNOWN, "\tUnknown opcode 0x%02X\r\n", RSP.cmd0 >> 24 );
#endif
	RSP.PC[RSP.PCi] += 8;
}

void RSP_LoadVertices( DWORD address, BYTE v0, BYTE num )
{
	Vertex *destVertex;
	RSPVertex *srcVertex;
	int i;

	if (RSP.update & UPDATE_COMBINEDMATRIX)
		RSP_CombineMatrices();

	if (RSP.update & UPDATE_LIGHTS)
		RSP_UpdateLights();

	srcVertex = (RSPVertex*)&RDRAM[address];
	destVertex = &RSP.vertices[v0];

	for (i = 0; i < num; i++)
	{
		destVertex->x = (float)srcVertex->x;
		destVertex->y = (float)srcVertex->y;
		destVertex->z = (float)srcVertex->z;
		destVertex->flag = srcVertex->flag;
		destVertex->s = (float)srcVertex->s * 0.03125f;
		destVertex->t = (float)srcVertex->t * 0.03125f;

		if (RSP.geometryMode & RSP_GEOMETRYMODE_LIGHTING)
		{
   			destVertex->nx = (float)srcVertex->normal.x;
			destVertex->ny = (float)srcVertex->normal.y;
			destVertex->nz = (float)srcVertex->normal.z;

			Normalize( &destVertex->nx );

			RSP_LightVertex( v0+i );

			if (RSP.geometryMode & RSP_GEOMETRYMODE_TEXTURE_GEN)
			{
				TransformVector( &destVertex->nx, RSP.combinedMtx );
				Normalize( &destVertex->nx );
				if (RSP.geometryMode & RSP_GEOMETRYMODE_TEXTURE_GEN_LINEAR)
				{   
					destVertex->s = acosf(destVertex->nx) * 325.94931f;
					destVertex->t = acosf(destVertex->ny) * 325.94931f;
				}
				else
				{
					destVertex->s = (destVertex->nx + 1.0f) * 512.0f;
					destVertex->t = (destVertex->ny + 1.0f) * 512.0f;
				}
			}
		}
		else
		{			       
			destVertex->r = srcVertex->color.r * 0.0039215689f;
			destVertex->g = srcVertex->color.g * 0.0039215689f;
 			destVertex->b = srcVertex->color.b * 0.0039215689f;
			destVertex->a = srcVertex->color.a * 0.0039215689f;
		}

		destVertex->s *= RSP.texture.scaleS;
		destVertex->t *= RSP.texture.scaleT;

  		TransformVertex( &destVertex->x, RSP.combinedMtx, RSP.perspNorm );

		destVertex->changed = TRUE;

#ifdef DEBUG	
		DebugMsg( DEBUG_HANDLED, "\tx = %f, y = %f, z = %f, s = %f, t = %f, nx = %f, ny = %f, nz = %f, r = %u, g = %u, b = %u\r\n",
			destVertex->x, RSP.vertices[i].y, destVertex->z, destVertex->s, destVertex->t,
			destVertex->nx, destVertex->ny, destVertex->nz,
			destVertex->r, destVertex->g, destVertex->b );
#endif
		srcVertex++;
		destVertex++;
	}
}

void Fast3DEXT_Vtx()
{
	DWORD address = RSP_SegmentAddress(RSP.cmd1);
	BYTE num = (RSP.cmd0 >> 9) & 0x7f;
	BYTE v0 = ((RSP.cmd0 >> 16) & 0xff) / 5;

	if ((((RSP.cmd0 & 0x1FF) + 1) >> 4) != num)
		if (RSP_DetectUCode())
			return;

	if (v0 + num > 32)
	{
#ifdef DEBUG
		DebugMsg( DEBUG_ERROR | DEBUG_IGNORED, "0x%08X: 0x%08X 0x%08X Fast3DEXT_Vtx\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
		DebugMsg( DEBUG_ERROR | DEBUG_IGNORED, "\tAttempting to write vertices to invalid location, ignoring\r\n" );
#endif
		RSP.PC[RSP.PCi] += 8;
		return;
	}

#ifdef DEBUG
	DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X Fast3DEXT_Vertex\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
#endif

	RSP_LoadVertices( address, v0, num );

	RSP.PC[RSP.PCi] += 8;
}

void Fast3DEXT_Tri1()
{
	RSPTriangle		*srcTriangle;

	do
	{
		srcTriangle = (RSPTriangle*)&RSP.cmd1;

		RSP.triangleFlags[RSP.numTriangles] = srcTriangle->flag;
		RSP.triangles[RSP.numTriangles].v0 = srcTriangle->v0 / 5;
		RSP.triangles[RSP.numTriangles].v1 = srcTriangle->v1 / 5;
		RSP.triangles[RSP.numTriangles].v2 = srcTriangle->v2 / 5;
		
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

	if (((RSP.cmd0 >> 24) != 0xB5) && ((RSP.cmd0 >> 24) != 0xB1))
	{
#ifdef DEBUG
		DebugMsg( DEBUG_HANDLED, "\tSending %u triangles to OpenGL for drawing\r\n", RSP.numTriangles );
#endif
		OGL_DrawTriangles();
		RSP.numTriangles = 0;
	}

	GFXOp[RSP.cmd0 >> 24]();
}

void Fast3DEXT_Tri2()
{
	RSPTriangle		*tmpTriangle;

	do
	{
		tmpTriangle = (RSPTriangle*)&RSP.cmd0;

		RSP.triangleFlags[RSP.numTriangles] = 0;
		RSP.triangles[RSP.numTriangles].v0 = tmpTriangle->v0 / 5;
		RSP.triangles[RSP.numTriangles].v1 = tmpTriangle->v1 / 5;
		RSP.triangles[RSP.numTriangles].v2 = tmpTriangle->v2 / 5;
		
#ifdef DEBUG
		DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X Fast3DEXT_Tri2\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
		DebugMsg( DEBUG_HANDLED, "\tAdding triangle: %u, %u, %u with flags of 0x%04X to triangle buffer\r\n",
			RSP.triangles[RSP.numTriangles].v0, RSP.triangles[RSP.numTriangles].v1, RSP.triangles[RSP.numTriangles].v2, RSP.triangleFlags[RSP.numTriangles] );

		if (RSP.numTriangles == 80)
		{
			DebugMsg( DEBUG_ERROR, "0x%08X: 0x%08X 0x%08X Fast3DEXT_Tri2\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
			DebugMsg( DEBUG_ERROR, "\tTriangle buffer filled!\r\n" );
		}
#endif

		RSP.numTriangles++;

		tmpTriangle = (RSPTriangle*)&RSP.cmd1;

		RSP.triangleFlags[RSP.numTriangles] = 0;
		RSP.triangles[RSP.numTriangles].v0 = tmpTriangle->v0 / 5;
		RSP.triangles[RSP.numTriangles].v1 = tmpTriangle->v1 / 5;
		RSP.triangles[RSP.numTriangles].v2 = tmpTriangle->v2 / 5;
		
#ifdef DEBUG
		DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X Fast3DEXT_Tri2\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
		DebugMsg( DEBUG_HANDLED, "\tAdding triangle: %u, %u, %u with flags of 0x%04X to triangle buffer\r\n",
			RSP.triangles[RSP.numTriangles].v0, RSP.triangles[RSP.numTriangles].v1, RSP.triangles[RSP.numTriangles].v2, RSP.triangleFlags[RSP.numTriangles] );

		if (RSP.numTriangles == 80)
		{
			DebugMsg( DEBUG_ERROR, "0x%08X: 0x%08X 0x%08X Fast3DEXT_Tri2\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
			DebugMsg( DEBUG_ERROR, "\tTriangle buffer filled!\r\n" );
		}
#endif

		RSP.numTriangles++;

		RSP.PC[RSP.PCi] += 8;

		RSP.cmd0 = *(DWORD*)&RDRAM[RSP.PC[RSP.PCi]];
		RSP.cmd1 = *(DWORD*)&RDRAM[RSP.PC[RSP.PCi] + 4];
	}
	while (((RSP.cmd0 >> 24) == 0xB1) && (RSP.numTriangles < 80));

	if (((RSP.cmd0 >> 24) != 0xBF) && ((RSP.cmd0 >> 24) != 0xB5))
	{
#ifdef DEBUG
		DebugMsg( DEBUG_HANDLED, "\tSending %u triangles to OpenGL for drawing\r\n", RSP.numTriangles );
#endif
		OGL_DrawTriangles();
		RSP.numTriangles = 0;
	}

	GFXOp[RSP.cmd0 >> 24]();
}

void Fast3DEXT_Quad3D()
{
	RSPTriangle		*tmpTriangle;

	do
	{
		tmpTriangle = (RSPTriangle*)&RSP.cmd0;

		RSP.triangleFlags[RSP.numTriangles] = 0;
		RSP.triangles[RSP.numTriangles].v0 = ((RSP.cmd1 >> 24) & 0xFF) / 5;
		RSP.triangles[RSP.numTriangles].v1 = ((RSP.cmd1 >> 16) & 0xFF) / 5;
		RSP.triangles[RSP.numTriangles].v2 = ((RSP.cmd1 >> 8) & 0xFF) / 5;
		
#ifdef DEBUG
		DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X Fast3DEXT_Tri2\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
		DebugMsg( DEBUG_HANDLED, "\tAdding triangle: %u, %u, %u with flags of 0x%04X to triangle buffer\r\n",
			RSP.triangles[RSP.numTriangles].v0, RSP.triangles[RSP.numTriangles].v1, RSP.triangles[RSP.numTriangles].v2, RSP.triangleFlags[RSP.numTriangles] );

		if (RSP.numTriangles == 80)
		{
			DebugMsg( DEBUG_ERROR, "0x%08X: 0x%08X 0x%08X Fast3DEXT_Tri2\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
			DebugMsg( DEBUG_ERROR, "\tTriangle buffer filled!\r\n" );
		}
#endif

		RSP.numTriangles++;

		tmpTriangle = (RSPTriangle*)&RSP.cmd1;

		RSP.triangleFlags[RSP.numTriangles] = 0;
		RSP.triangles[RSP.numTriangles].v0 = (RSP.cmd1 & 0xFF) / 5;
		RSP.triangles[RSP.numTriangles].v1 = ((RSP.cmd1 >> 24) & 0xFF) / 5;
		RSP.triangles[RSP.numTriangles].v2 = ((RSP.cmd1 >> 8) & 0xFF) / 5;
		
#ifdef DEBUG
		DebugMsg( DEBUG_HANDLED, "0x%08X: 0x%08X 0x%08X Fast3DEXT_Tri2\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
		DebugMsg( DEBUG_HANDLED, "\tAdding triangle: %u, %u, %u with flags of 0x%04X to triangle buffer\r\n",
			RSP.triangles[RSP.numTriangles].v0, RSP.triangles[RSP.numTriangles].v1, RSP.triangles[RSP.numTriangles].v2, RSP.triangleFlags[RSP.numTriangles] );

		if (RSP.numTriangles == 80)
		{
			DebugMsg( DEBUG_ERROR, "0x%08X: 0x%08X 0x%08X Fast3DEXT_Tri2\r\n", RSP.PC[RSP.PCi], RSP.cmd0, RSP.cmd1 );
			DebugMsg( DEBUG_ERROR, "\tTriangle buffer filled!\r\n" );
		}
#endif

		RSP.numTriangles++;

		RSP.PC[RSP.PCi] += 8;

		RSP.cmd0 = *(DWORD*)&RDRAM[RSP.PC[RSP.PCi]];
		RSP.cmd1 = *(DWORD*)&RDRAM[RSP.PC[RSP.PCi] + 4];
	}
	while (((RSP.cmd0 >> 24) == 0xB5) && (RSP.numTriangles < 80));

	if (((RSP.cmd0 >> 24) != 0xBF) && ((RSP.cmd0 >> 24) != 0xB1))
	{
#ifdef DEBUG
		DebugMsg( DEBUG_HANDLED, "\tSending %u triangles to OpenGL for drawing\r\n", RSP.numTriangles );
#endif
		OGL_DrawTriangles();
		RSP.numTriangles = 0;
	}

	GFXOp[RSP.cmd0 >> 24]();
}

void RSP_UpdateColorImage()
{
	WORD *colorBuffer = (WORD*)&RDRAM[RDP.colorImage.address];
	BYTE *frameBuffer = (BYTE*)malloc( OGL.width * OGL.height * 3 );
	BYTE *framePixel;

	int x, y, frameX, frameY, i;

	glReadBuffer( GL_BACK );
	glReadPixels( 0, 0, OGL.width - 1, OGL.height - 1, GL_RGB, GL_UNSIGNED_BYTE, frameBuffer );
	
	i = 0;
	for (y = 0; y < RDP.height; y++)
	{
		frameY = OGL.height - (y * OGL.scaleY);
		for (x = 0; x < RDP.width; x++)
		{
			frameX = x * OGL.scaleX;
			framePixel = &frameBuffer[(OGL.width * frameY + frameX) * 3];
			colorBuffer[i^1] =	((framePixel[0] >> 3) << 11) |
								((framePixel[1] >> 3) <<  6) |
								((framePixel[2] >> 3) <<  1);
			i++;
		}
	}
	free( frameBuffer );
}

DWORD WINAPI RSP_ThreadProc( LPVOID lpParameter )
{
	BOOL depthMask;
	int i;

	OGL.hDC = NULL;
	OGL.hRC = NULL;

	OGL.fullscreen = FALSE;

	Config_LoadConfig();

	RSP.DList = 0;
	RSP.textureTile[0] = &RDP.tiles[0];
	RSP.textureTile[1] = &RDP.tiles[1];

	// Default to Fast3D
	RSP_SetUCode( UCODE_FAST3D );

	OGL_Start();
	Combiner_Init();

	SetEvent( RSP.threadFinished );

	while (TRUE)
	{
		switch (WaitForMultipleObjects( 4, RSP.threadMsg, FALSE, INFINITE ))
		{
			case (WAIT_OBJECT_0 + RSPMSG_PROCESSDLIST):
				// Hard to detect when it changes, so just do it every time
				OGL_UpdateScale();

				RSP_ProcessDList();
				break;
			case (WAIT_OBJECT_0 + RSPMSG_SWAPBUFFERS):
				if (RSP.update & UPDATE_FRONTBUFFER)
				{
//					RSP_UpdateColorImage();

					if (OGL.fullscreen)
						SwapBuffers( OGL.hFullscreenDC );
					else
						SwapBuffers( OGL.hDC );

					RSP.update &= ~UPDATE_FRONTBUFFER;
				}
				break;
			case (WAIT_OBJECT_0 + RSPMSG_CLOSE):
				OGL_Stop();
				SetEvent( RSP.threadFinished );
				return 1;
			case (WAIT_OBJECT_0 + RSPMSG_CHANGEWINDOW):
				OGL_ToggleFullscreen();
				break;
		}
		SetEvent( RSP.threadFinished );
	}
	return 0;
}

void RSP_ProcessDList()
{
#ifdef DEBUG
	if (RSP.dumpNextDL)
	{
		StartDump( "c:\\dl_dump.txt" );
		RSP.dumpDL = TRUE;
		RSP.dumpNextDL = FALSE;
	}
#endif
	RSP.modelViewi = 0;

	RSP.PC[0] = *(DWORD*)&DMEM[0x0ff0];
	RSP.PCi = 0;
	RSP.modelViewi = 0;

	RSP.halt = FALSE;

	while (!RSP.halt)
	{
		RSP.cmd0 = *(DWORD*)&RDRAM[RSP.PC[RSP.PCi]];
		RSP.cmd1 = *(DWORD*)&RDRAM[RSP.PC[RSP.PCi] + 4];

		GFXOp[RSP.cmd0 >> 24]();
	}

	RSP.DList++;
	RSP.update |= UPDATE_FRONTBUFFER;

#ifdef DEBUG
	if (RSP.dumpDL)
	{
		RSP.dumpDL = FALSE;
		EndDump();
	}
#endif

}

void RSP_Init()
{
	RSP.DList = 0;
	RSP.textureTile[0] = &RDP.tiles[0];
	RSP.textureTile[1] = &RDP.tiles[1];
	RSP_SetUCode( UCODE_FAST3D );
}

void RSP_SetUCode( BYTE uCode )
{
	RSP.uCode = uCode;

	// Initialize all commands to RSP_Unknown
	for (int i = 0x00; i < 0x100; i++)
		GFXOp[i] = RSP_Unknown;

	// Initialize RDP commands
	RDP_Init();

	switch (RSP.uCode)
	{
		case UCODE_FAST3D:
			Fast3D_Init();
			break;
		case UCODE_FAST3DEXT:
			Fast3D_Init();
			GFXOp[0x04] = Fast3DEXT_Vtx;
			GFXOp[0xB1] = Fast3DEXT_Tri2;
			GFXOp[0xB5] = Fast3DEXT_Quad3D;
			GFXOp[0xBF] = Fast3DEXT_Tri1;
			break;
		case UCODE_F3DEX:
			F3DEX_Init();
			break;
		case UCODE_F3DEX2:
			F3DEX2_Init();
			break;
	}

	RSP.update = 0xFFFFFFFF;
	RSP.perspNorm = 1.0f;
}

// I'm currently using two methods to detect the ucode. The first method is using G_VTX
// to detect the ucode. I can do this because for some reason (probably some sort of
// optimization) the ucodes send the number if vertices twice - in two different forms.
// So what I do is just compute it boths ways and see if I come up with the same thing.
// The second way is to simply take unknown opcodes and see if they are known to another
// ucode. It's really pretty crude, but works well. The biggest problem is that it will 
// sometimes take a while to get to a point where it can recognize the ucode and miss 
// some opcodes, but it's usually no big deal.

// Still a work in progress...
BOOL RSP_DetectUCode()
{
	// Change the ucode if the user forced it
	if (RSP.forceUCode) return TRUE;

	BYTE command = (BYTE)(RSP.cmd0 >> 24);

	// G_VTX is a good indicator, see if we can find it there
	if (command == 0x04)
	{
		if (RSP.uCode != UCODE_FAST3D)
		{
			if ((((RSP.cmd0 >> 20) & 0x0F) + 1) == ((RSP.cmd0 & 0xFFFF) >> 4))
			{
				RSP_SetUCode( UCODE_FAST3D );
				return TRUE;
			}
		}
		if (RSP.uCode != UCODE_FAST3DEXT)
		{
			if (((RSP.cmd0 >> 9) & 0x7F) == (((RSP.cmd0 & 0x1FF) + 1) >> 4))
			{
				RSP_SetUCode( UCODE_FAST3DEXT );
				RSP.forceUCode = TRUE; // Temporary
				return TRUE;
			}
		}
		if (RSP.uCode != UCODE_F3DEX)
		{
			if (((RSP.cmd0 >> 10) & 0x3F) == (((RSP.cmd0 & 0x3FF) + 1) >> 4))
			{
				RSP_SetUCode( UCODE_F3DEX );
				return TRUE;
			}
		}
	}
	else if ((command >= 0xB0) && (command <= 0xB2))
	{
		RSP_SetUCode( UCODE_F3DEX );
		return TRUE;
	}
	else if ((command >= 0xB3) && (command <= 0xBF))
	{
		RSP_SetUCode( UCODE_FAST3D );
		return TRUE;
	}
	else if ((command >= 0xD0) && (command <= 0xE3))
	{
		RSP_SetUCode( UCODE_F3DEX2 );
		return TRUE;
	}

	// Couldn't find a match
	return FALSE;
}