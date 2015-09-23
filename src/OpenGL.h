#ifndef OPENGL_H

#define OPENGL_H

#include <windows.h>
#include <gl/gl.h>
#include "glext.h"

struct GLVertex
{
	float x, y, z, w;
	float r, g, b, a;
	float s0, t0, s1, t1;
	float fog;
};

struct GLInfo
{
	HGLRC	hRC, hFullscreenRC;
	HDC		hDC, hFullscreenDC;

	DWORD	fullscreenWidth, fullscreenHeight, fullscreenBits, fullscreenRefresh;
	DWORD	width, height, windowedWidth, windowedHeight, heightOffset;

	BOOL	fullscreen, forceBilinear, fog;

	float	scaleX, scaleY;

	BOOL	ATIX_texture_env_combine3;	// Radeon

	BOOL	ARB_multitexture;			// TNT, GeForce, Rage 128, Radeon
	BOOL	ARB_texture_env_combine;	// GeForce, Rage 128, Radeon
	BOOL	ARB_texture_env_crossbar;	// Radeon (GeForce supports it, but doesn't report it)

	BOOL	EXT_fog_coord;				// TNT, GeForce, Rage 128, Radeon
	BOOL	EXT_texture_env_combine;	// TNT, GeForce, Rage 128, Radeon
	BOOL	EXT_secondary_color;

	BOOL	NV_texture_env_combine4;	// TNT, GeForce
	BOOL	NV_register_combiners;		// GeForce

	int		maxTextureUnits;			// TNT = 2, GeForce = 2-4, Rage 128 = 2, Radeon = 3-6
	int		maxGeneralCombiners;

	BOOL	enable2xSaI;
	float	originAdjust;

	GLVertex vertices[80];
	BYTE	triangles[80][3];
	BYTE	numTriangles;
	BYTE	numVertices;
	HWND	hFullscreenWnd;

	GLubyte	stipplePattern[32][8][128];
	BYTE	lastStipple;

	BYTE	combiner;
};

extern GLInfo OGL;

struct GLcolor
{
	float r, g, b, a;
};

extern PFNGLCOMBINERPARAMETERFVNVPROC glCombinerParameterfvNV;
extern PFNGLCOMBINERPARAMETERFNVPROC glCombinerParameterfNV;
extern PFNGLCOMBINERPARAMETERIVNVPROC glCombinerParameterivNV;
extern PFNGLCOMBINERPARAMETERINVPROC glCombinerParameteriNV;
extern PFNGLCOMBINERINPUTNVPROC glCombinerInputNV;
extern PFNGLCOMBINEROUTPUTNVPROC glCombinerOutputNV;
extern PFNGLFINALCOMBINERINPUTNVPROC glFinalCombinerInputNV;
extern PFNGLGETCOMBINERINPUTPARAMETERFVNVPROC glGetCombinerInputParameterfvNV;
extern PFNGLGETCOMBINERINPUTPARAMETERIVNVPROC glGetCombinerInputParameterivNV;
extern PFNGLGETCOMBINEROUTPUTPARAMETERFVNVPROC glGetCombinerOutputParameterfvNV;
extern PFNGLGETCOMBINEROUTPUTPARAMETERIVNVPROC glGetCombinerOutputParameterivNV;
extern PFNGLGETFINALCOMBINERINPUTPARAMETERFVNVPROC glGetFinalCombinerInputParameterfvNV;
extern PFNGLGETFINALCOMBINERINPUTPARAMETERIVNVPROC glGetFinalCombinerInputParameterivNV;
extern PFNGLACTIVETEXTUREARBPROC glActiveTextureARB;
extern PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTextureARB;
extern PFNGLMULTITEXCOORD2FARBPROC glMultiTexCoord2fARB;
extern PFNGLSECONDARYCOLOR3FVEXTPROC glSecondaryColor3fvEXT;

BOOL OGL_Start();
BOOL OGL_StartFullscreen();
void OGL_Stop();
void OGL_DrawRect( GLfloat x0, GLfloat y0, GLfloat x1, GLfloat y1, GLcolor color );
void OGL_AddTriangle( BYTE tri[3] );
void OGL_DrawTriangles();
void OGL_DrawTexturedRect( float x0, float u0, float y0, float v0,
						  float x1, float u1, float y1, float v1 );
void OGL_SetWireframe( BOOL wireframe );
void OGL_UpdateScale();
void OGL_ResizeWindow( WORD width, WORD height );
void OGL_InitExtensions();
void OGL_ToggleFullscreen();

#endif