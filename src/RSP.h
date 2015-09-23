#ifndef RSP_H
#define RSP_H

#include <windows.h>
#include "N64.h"
#include "RDP.h"

#define RSPMSG_CLOSE			0
#define RSPMSG_SWAPBUFFERS		1
#define RSPMSG_PROCESSDLIST		2
#define RSPMSG_CHANGEWINDOW		3

#define UCODE_FAST3D	0 // Mario
#define UCODE_FAST3DEXT	1 // Waverace
#define UCODE_F3DEX		2 // Mario Kart, Star Fox
#define UCODE_F3DEX2	3 // Zelda, Smash Brothers

// Update flags for rendering
#define UPDATE_COMBINEDMATRIX				0x0002
#define UPDATE_TEXTURES						0x0004
#define UPDATE_TEXTURE_GEN					0x0008
#define UPDATE_FOG							0x0010
#define UPDATE_ZMODE						0x0020
#define UPDATE_CULLMODE						0x0040
#define UPDATE_COMBINE						0x0080
#define UPDATE_COMBINE_COLORS				0x0100
#define UPDATE_VIEWPORT						0x0200
#define UPDATE_LIGHTS						0x0400
#define UPDATE_FRONTBUFFER					0x0800
#define UPDATE_ZCOMPARE						0x1000
#define UPDATE_ZINTER						0x2000
#define UPDATE_ZUPDATE						0x4000
#define UPDATE_ALL							0xFFFF

// Flags for SetGeometryMode, since changes with uCode, need to use variables
extern DWORD RSP_GEOMETRYMODE_ZBUFFER;
extern DWORD RSP_GEOMETRYMODE_TEXTURE_ENABLE;
extern DWORD RSP_GEOMETRYMODE_SHADE;
extern DWORD RSP_GEOMETRYMODE_SHADING_SMOOTH;
extern DWORD RSP_GEOMETRYMODE_CULL_FRONT;
extern DWORD RSP_GEOMETRYMODE_CULL_BACK;
extern DWORD RSP_GEOMETRYMODE_CULL_BOTH;
extern DWORD RSP_GEOMETRYMODE_FOG;
extern DWORD RSP_GEOMETRYMODE_LIGHTING;
extern DWORD RSP_GEOMETRYMODE_TEXTURE_GEN;
extern DWORD RSP_GEOMETRYMODE_TEXTURE_GEN_LINEAR;
extern DWORD RSP_GEOMETRYMODE_LOD;

// Segment to use is in first byte, offset is in the last 3
// Should I mask it down to 0x003FFFFF?
#define RSP_SegmentAddress(seg) ( RSP.segment[(seg >> 24) & 0x0F] + (seg & 0x007FFFFF) )

typedef struct
{
	short y;
	short x;

	unsigned short flag;
	short z;

	short t;
	short s;

	union {
		struct _rgba
		{
			BYTE a;
			BYTE b;
			BYTE g;
			BYTE r;
		} color;
		struct _normal
		{
			char a;
			char z;	// b
			char y;	//g
			char x;	//r
		} normal;
	};
} RSPVertex;

typedef struct
{
	float x, y, z, w;
	float nx, ny, nz;
	float s, t;
	float r, g, b, a;
	WORD flag;
	BOOL changed;
} Vertex;

typedef struct
{
	BYTE v2, v1, v0, flag;
} RSPTriangle;

typedef struct
{
	BYTE v0, v1, v2;
} Triangle;

typedef struct {
	BYTE pad1, b, g, r;
	BYTE pad2, b2, g2, r2;
	CHAR pad3, z, y, x;
} RSPLight;

typedef struct {
	float	x, y, z;
	float	mx, my, mz; // Transformed
	float	r, g, b;
} Light;

typedef struct
{
	HANDLE thread;

	DWORD PC[10];
	BYTE PCi;
	BOOL busy, halt, close;

	DWORD segment[16];

	DWORD cmd0, cmd1;

	BYTE numLights;
	BYTE numTriangles;

	DWORD geometryMode;

	Light			lights[8];

	// Keeping these seperated lets me use glDrawElements
	Triangle		triangles[80];
	BYTE			triangleFlags[80];

	struct
	{
		BYTE	on, tile, level;
		float	scaleS, scaleT;
	} texture;

	tile *(textureTile[2]);

	Vertex			vertices[80];
	
	BOOL dumpNextDL, dumpDL;

	DWORD update;
	DWORD64 blender;

	float			fogMultiplier, fogOffset, fogMin, fogMax;
	float			perspNorm;
	float			combinedMtx[4][4];
	float			projectionMtx[4][4];
	float			modelViewStack[18][4][4];
	BYTE			modelViewi;

	BYTE			uCode;
	BOOL			forceUCode, purpleUnknownCombiners;

	DWORD			DList;

	struct
	{
		DWORD		geometryMode;
	} changed;

	struct
	{
		float x, y, width, height, nearZ, farZ;
	} viewport;

	// Events for thread messages, see defines at the top, or RSP_Thread
	HANDLE			threadMsg[4];
	// Event to notify main process that the RSP is finished with what it was doing
	HANDLE			threadFinished;

} RSPInfo;

extern RSPInfo	RSP;

// Functions common to all uCodes
void RSP_Init();
void RSP_ProcessDList();
DWORD WINAPI RSP_ThreadProc( LPVOID lpParameter );
void RSP_LightVertex( BYTE v );
void RSP_UpdateLights();
void RSP_UpdateColorImage();
void RSP_CombineMatrices();
void RSP_LoadMatrix( float mtx[4][4], DWORD address );

void RSP_LoadVertices( DWORD address, BYTE v0, BYTE num );
void RSP_SetUCode( BYTE uCode );
BOOL RSP_DetectUCode();

#endif