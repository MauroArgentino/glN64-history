#include <windows.h>
#include <math.h>
#include <gl/gl.h>
#include <string.h>
#include "glNintendo64().h"
#include "OpenGL.h"
#include "RSP.h"
#include "RDP.h"
#include "glext.h"
#include "Textures.h"
#include "Combiner.h"

GLInfo	OGL;

// NV_register_combiners functions
PFNGLCOMBINERPARAMETERFVNVPROC glCombinerParameterfvNV;
PFNGLCOMBINERPARAMETERFNVPROC glCombinerParameterfNV;
PFNGLCOMBINERPARAMETERIVNVPROC glCombinerParameterivNV;
PFNGLCOMBINERPARAMETERINVPROC glCombinerParameteriNV;
PFNGLCOMBINERINPUTNVPROC glCombinerInputNV;
PFNGLCOMBINEROUTPUTNVPROC glCombinerOutputNV;
PFNGLFINALCOMBINERINPUTNVPROC glFinalCombinerInputNV;
PFNGLGETCOMBINERINPUTPARAMETERFVNVPROC glGetCombinerInputParameterfvNV;
PFNGLGETCOMBINERINPUTPARAMETERIVNVPROC glGetCombinerInputParameterivNV;
PFNGLGETCOMBINEROUTPUTPARAMETERFVNVPROC glGetCombinerOutputParameterfvNV;
PFNGLGETCOMBINEROUTPUTPARAMETERIVNVPROC glGetCombinerOutputParameterivNV;
PFNGLGETFINALCOMBINERINPUTPARAMETERFVNVPROC glGetFinalCombinerInputParameterfvNV;
PFNGLGETFINALCOMBINERINPUTPARAMETERIVNVPROC glGetFinalCombinerInputParameterivNV;

// ARB_multitexture functions
PFNGLACTIVETEXTUREARBPROC glActiveTextureARB;
PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTextureARB;
PFNGLMULTITEXCOORD2FARBPROC glMultiTexCoord2fARB;

// EXT_fog_coord functions
PFNGLFOGCOORDFEXTPROC glFogCoordfEXT;
PFNGLFOGCOORDFVEXTPROC glFogCoordfvEXT;
PFNGLFOGCOORDDEXTPROC glFogCoorddEXT;
PFNGLFOGCOORDDVEXTPROC glFogCoorddvEXT;
PFNGLFOGCOORDPOINTEREXTPROC glFogCoordPointerEXT;

// EXT_secondary_color functions
PFNGLSECONDARYCOLOR3BEXTPROC glSecondaryColor3bEXT;
PFNGLSECONDARYCOLOR3BVEXTPROC glSecondaryColor3bvEXT;
PFNGLSECONDARYCOLOR3DEXTPROC glSecondaryColor3dEXT;
PFNGLSECONDARYCOLOR3DVEXTPROC glSecondaryColor3dvEXT;
PFNGLSECONDARYCOLOR3FEXTPROC glSecondaryColor3fEXT;
PFNGLSECONDARYCOLOR3FVEXTPROC glSecondaryColor3fvEXT;
PFNGLSECONDARYCOLOR3IEXTPROC glSecondaryColor3iEXT;
PFNGLSECONDARYCOLOR3IVEXTPROC glSecondaryColor3ivEXT;
PFNGLSECONDARYCOLOR3SEXTPROC glSecondaryColor3sEXT;
PFNGLSECONDARYCOLOR3SVEXTPROC glSecondaryColor3svEXT;
PFNGLSECONDARYCOLOR3UBEXTPROC glSecondaryColor3ubEXT;
PFNGLSECONDARYCOLOR3UBVEXTPROC glSecondaryColor3ubvEXT;
PFNGLSECONDARYCOLOR3UIEXTPROC glSecondaryColor3uiEXT;
PFNGLSECONDARYCOLOR3UIVEXTPROC glSecondaryColor3uivEXT;
PFNGLSECONDARYCOLOR3USEXTPROC glSecondaryColor3usEXT;
PFNGLSECONDARYCOLOR3USVEXTPROC glSecondaryColor3usvEXT;
PFNGLSECONDARYCOLORPOINTEREXTPROC glSecondaryColorPointerEXT;

BOOL isExtensionSupported( const char *extension )
{
	const GLubyte *extensions = NULL;
	const GLubyte *start;
	GLubyte *where, *terminator;

	where = (GLubyte *) strchr(extension, ' ');
	if (where || *extension == '\0')
		return 0;

	extensions = glGetString(GL_EXTENSIONS);

	start = extensions;
	for (;;)
	{
		where = (GLubyte *) strstr((const char *) start, extension);
		if (!where)
			break;

		terminator = where + strlen(extension);
		if (where == start || *(where - 1) == ' ')
			if (*terminator == ' ' || *terminator == '\0')
				return TRUE;

		start = terminator;
	}

	return FALSE;
}

void OGL_InitExtensions()
{
	OGL.ATIX_texture_env_combine3 = isExtensionSupported( "GL_ATIX_texture_env_combine3" );

	OGL.NV_texture_env_combine4 = isExtensionSupported( "GL_NV_texture_env_combine4" );

	if (OGL.NV_register_combiners = isExtensionSupported( "GL_NV_register_combiners" ))
	{
		glCombinerParameterfvNV = (PFNGLCOMBINERPARAMETERFVNVPROC)wglGetProcAddress( "glCombinerParameterfvNV" );
		glCombinerParameterfNV = (PFNGLCOMBINERPARAMETERFNVPROC)wglGetProcAddress( "glCombinerParameterfNV" );
		glCombinerParameterivNV = (PFNGLCOMBINERPARAMETERIVNVPROC)wglGetProcAddress( "glCombinerParameterivNV" );
		glCombinerParameteriNV = (PFNGLCOMBINERPARAMETERINVPROC)wglGetProcAddress( "glCombinerParameteriNV" );
		glCombinerInputNV = (PFNGLCOMBINERINPUTNVPROC)wglGetProcAddress( "glCombinerInputNV" );
		glCombinerOutputNV = (PFNGLCOMBINEROUTPUTNVPROC)wglGetProcAddress( "glCombinerOutputNV" );
		glFinalCombinerInputNV = (PFNGLFINALCOMBINERINPUTNVPROC)wglGetProcAddress( "glFinalCombinerInputNV" );
		glGetCombinerInputParameterfvNV = (PFNGLGETCOMBINERINPUTPARAMETERFVNVPROC)wglGetProcAddress( "glGetCombinerInputParameterfvNV" );
		glGetCombinerInputParameterivNV = (PFNGLGETCOMBINERINPUTPARAMETERIVNVPROC)wglGetProcAddress( "glGetCombinerInputParameterivNV" );
		glGetCombinerOutputParameterfvNV = (PFNGLGETCOMBINEROUTPUTPARAMETERFVNVPROC)wglGetProcAddress( "glGetCombinerOutputParameterfvNV" );
		glGetCombinerOutputParameterivNV = (PFNGLGETCOMBINEROUTPUTPARAMETERIVNVPROC)wglGetProcAddress( "glGetCombinerOutputParameterivNV" );
		glGetFinalCombinerInputParameterfvNV = (PFNGLGETFINALCOMBINERINPUTPARAMETERFVNVPROC)wglGetProcAddress( "glGetFinalCombinerInputParameterfvNV" );
		glGetFinalCombinerInputParameterivNV = (PFNGLGETFINALCOMBINERINPUTPARAMETERIVNVPROC)wglGetProcAddress( "glGetFinalCombinerInputParameterivNV" );
		glGetIntegerv( GL_MAX_GENERAL_COMBINERS_NV, &OGL.maxGeneralCombiners );
	}

	if (OGL.ARB_multitexture = isExtensionSupported( "GL_ARB_multitexture" ))
	{
		glActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC)wglGetProcAddress( "glActiveTextureARB" );
		glClientActiveTextureARB = (PFNGLCLIENTACTIVETEXTUREARBPROC)wglGetProcAddress( "glClientActiveTextureARB" );
		glMultiTexCoord2fARB = (PFNGLMULTITEXCOORD2FARBPROC)wglGetProcAddress( "glMultiTexCoord2fARB" );
		glGetIntegerv( GL_MAX_TEXTURE_UNITS_ARB, &OGL.maxTextureUnits );
	}

	if (OGL.EXT_fog_coord = isExtensionSupported( "GL_EXT_fog_coord" ))
	{
		glFogCoordfEXT = (PFNGLFOGCOORDFEXTPROC)wglGetProcAddress( "glFogCoordfEXT" );
		glFogCoordfvEXT = (PFNGLFOGCOORDFVEXTPROC)wglGetProcAddress( "glFogCoordfvEXT" );
		glFogCoorddEXT = (PFNGLFOGCOORDDEXTPROC)wglGetProcAddress( "glFogCoorddEXT" );
		glFogCoorddvEXT = (PFNGLFOGCOORDDVEXTPROC)wglGetProcAddress( "glFogCoorddvEXT" );
		glFogCoordPointerEXT = (PFNGLFOGCOORDPOINTEREXTPROC)wglGetProcAddress( "glFogCoordPointerEXT" );
	}

	if (OGL.EXT_secondary_color = isExtensionSupported( "GL_EXT_secondary_color" ))
	{
		glSecondaryColor3bEXT = (PFNGLSECONDARYCOLOR3BEXTPROC)wglGetProcAddress( "glSecondaryColor3bEXT" );
		glSecondaryColor3bvEXT = (PFNGLSECONDARYCOLOR3BVEXTPROC)wglGetProcAddress( "glSecondaryColor3bvEXT" );
		glSecondaryColor3dEXT = (PFNGLSECONDARYCOLOR3DEXTPROC)wglGetProcAddress( "glSecondaryColor3dEXT" );
		glSecondaryColor3dvEXT = (PFNGLSECONDARYCOLOR3DVEXTPROC)wglGetProcAddress( "glSecondaryColor3dvEXT" );
		glSecondaryColor3fEXT = (PFNGLSECONDARYCOLOR3FEXTPROC)wglGetProcAddress( "glSecondaryColor3fEXT" );
		glSecondaryColor3fvEXT = (PFNGLSECONDARYCOLOR3FVEXTPROC)wglGetProcAddress( "glSecondaryColor3fvEXT" );
		glSecondaryColor3iEXT = (PFNGLSECONDARYCOLOR3IEXTPROC)wglGetProcAddress( "glSecondaryColor3iEXT" );
		glSecondaryColor3ivEXT = (PFNGLSECONDARYCOLOR3IVEXTPROC)wglGetProcAddress( "glSecondaryColor3ivEXT" );
		glSecondaryColor3sEXT = (PFNGLSECONDARYCOLOR3SEXTPROC)wglGetProcAddress( "glSecondaryColor3sEXT" );
		glSecondaryColor3svEXT = (PFNGLSECONDARYCOLOR3SVEXTPROC)wglGetProcAddress( "glSecondaryColor3svEXT" );
		glSecondaryColor3ubEXT = (PFNGLSECONDARYCOLOR3UBEXTPROC)wglGetProcAddress( "glSecondaryColor3ubEXT" );
		glSecondaryColor3ubvEXT = (PFNGLSECONDARYCOLOR3UBVEXTPROC)wglGetProcAddress( "glSecondaryColor3ubvEXT" );
		glSecondaryColor3uiEXT = (PFNGLSECONDARYCOLOR3UIEXTPROC)wglGetProcAddress( "glSecondaryColor3uiEXT" );
		glSecondaryColor3uivEXT = (PFNGLSECONDARYCOLOR3UIVEXTPROC)wglGetProcAddress( "glSecondaryColor3uivEXT" );
		glSecondaryColor3usEXT = (PFNGLSECONDARYCOLOR3USEXTPROC)wglGetProcAddress( "glSecondaryColor3usEXT" );
		glSecondaryColor3usvEXT = (PFNGLSECONDARYCOLOR3USVEXTPROC)wglGetProcAddress( "glSecondaryColor3usvEXT" );
		glSecondaryColorPointerEXT = (PFNGLSECONDARYCOLORPOINTEREXTPROC)wglGetProcAddress( "glSecondaryColorPointerEXT" );
	}

	OGL.EXT_texture_env_combine = isExtensionSupported( "GL_EXT_texture_env_combine" );
	OGL.ARB_texture_env_combine = isExtensionSupported( "GL_ARB_texture_env_combine" );
	OGL.ARB_texture_env_crossbar = isExtensionSupported( "GL_ARB_texture_env_crossbar" );
}

void OGL_UpdateScale()
{
	float scaleX = (float)(*REG.VI_X_SCALE & 0xFFFF) / 1024.0f;
	float scaleY = (float)(*REG.VI_Y_SCALE & 0xFFFF) / 2048.0f;

	RDP.width = ((*REG.VI_H_START & 0xFFFF) - (*REG.VI_H_START >> 16)) * scaleX;
	RDP.height = ((*REG.VI_V_START & 0xFFFF) - (*REG.VI_V_START >> 16)) * scaleY * 1.0126582f;

	if (RDP.width == 0.0f)
		RDP.width = 320.0f;

	if (RDP.height == 0.0f)
		RDP.height = 240.0f;

	OGL.scaleX = OGL.width / (float)RDP.width;
	OGL.scaleY = OGL.height / (float)RDP.height;
}

void OGL_ResizeWindow( WORD width, WORD height)
{
	RECT	windowRect, statusRect;

	OGL.width = OGL.windowedWidth = width;
	OGL.height = OGL.windowedHeight = height;

	GetClientRect( hWnd, &windowRect );
	GetWindowRect( hStatusBar, &statusRect );

	OGL.heightOffset = (statusRect.bottom - statusRect.top);
	windowRect.right = windowRect.left + OGL.width - 1;
	windowRect.bottom = windowRect.top + OGL.height - 1 + OGL.heightOffset;

	AdjustWindowRect( &windowRect, GetWindowLong( hWnd, GWL_STYLE ), (BOOL)GetMenu( hWnd ) );

	SetWindowPos( hWnd, NULL, 0, 0,	windowRect.right - windowRect.left + 1,
					windowRect.bottom - windowRect.top + 1, SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE );

	OGL_UpdateScale();

	RSP.update |= UPDATE_VIEWPORT;
}

void OGL_InitStates()
{
	OGL.originAdjust = (OGL.enable2xSaI ? 0.25 : 0.50);

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();

	glEnable( GL_SCISSOR_TEST );

	glVertexPointer( 4, GL_FLOAT, sizeof( GLVertex ), &OGL.vertices[0].x );
	glEnableClientState( GL_VERTEX_ARRAY );

	glColorPointer( 4, GL_FLOAT, sizeof( GLVertex ), &OGL.vertices[0].r );
	glEnableClientState( GL_COLOR_ARRAY );

	if (OGL.ARB_multitexture)
	{
		glClientActiveTextureARB( GL_TEXTURE0_ARB );
		glTexCoordPointer( 2, GL_FLOAT, sizeof( GLVertex ), &OGL.vertices[0].s0 );
		glEnableClientState( GL_TEXTURE_COORD_ARRAY );

		glClientActiveTextureARB( GL_TEXTURE1_ARB );
		glTexCoordPointer( 2, GL_FLOAT, sizeof( GLVertex ), &OGL.vertices[0].s1 );
		glEnableClientState( GL_TEXTURE_COORD_ARRAY );
	}
	else
	{
		glTexCoordPointer( 2, GL_FLOAT, sizeof( GLVertex ), &OGL.vertices[0].s0 );
		glEnableClientState( GL_TEXTURE_COORD_ARRAY );
	}

	if (OGL.EXT_fog_coord)
	{
		glFogCoordPointerEXT( GL_FLOAT, sizeof( GLVertex ), &OGL.vertices[0].fog );
		glHint( GL_FOG_HINT, GL_NICEST );
		glEnableClientState( GL_FOG_COORDINATE_ARRAY_EXT );
		glFogi( GL_FOG_COORDINATE_SOURCE_EXT, GL_FOG_COORDINATE_EXT );

		glFogf( GL_FOG_MODE, GL_LINEAR );
		glFogf( GL_FOG_START, 0.0f );
		glFogf( GL_FOG_END, 255.0f );
	}

	glPolygonOffset( -1.0f, -1.0f );

	glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
	glClear( GL_COLOR_BUFFER_BIT );

	srand( timeGetTime() );

	for (int i = 0; i < 32; i++)
	{
		for (int j = 0; j < 8; j++)
			for (int k = 0; k < 128; k++)
				OGL.stipplePattern[i][j][k] =((i > (rand() >> 10)) << 7) |
											((i > (rand() >> 10)) << 6) |
											((i > (rand() >> 10)) << 5) |
											((i > (rand() >> 10)) << 4) |
											((i > (rand() >> 10)) << 3) |
											((i > (rand() >> 10)) << 2) |
											((i > (rand() >> 10)) << 1) |
											((i > (rand() >> 10)) << 0);
	}
}

BOOL OGL_Start()
{
	OGL_ResizeWindow( OGL.windowedWidth, OGL.windowedHeight );

	int		pixelFormat;
	PIXELFORMATDESCRIPTOR pfd = { 
		sizeof(PIXELFORMATDESCRIPTOR),    // size of this pfd 
		1,                                // version number 
		PFD_DRAW_TO_WINDOW |              // support window 
		PFD_SUPPORT_OPENGL |              // support OpenGL 
		PFD_DOUBLEBUFFER,                 // double buffered 
		PFD_TYPE_RGBA,                    // RGBA type 
		32,                               // 24-bit color depth 
		0, 0, 0, 0, 0, 0,                 // color bits ignored 
		0,                                // no alpha buffer 
		0,                                // shift bit ignored 
		0,                                // no accumulation buffer 
		0, 0, 0, 0,                       // accum bits ignored 
		32,                               // 32-bit z-buffer     
		0,                                // no stencil buffer 
		0,                                // no auxiliary buffer 
		PFD_MAIN_PLANE,                   // main layer 
		0,                                // reserved 
		0, 0, 0                           // layer masks ignored 
	};

	if ((OGL.hDC = GetDC( hWnd )) == NULL)
	{
		MessageBox( hWnd, "Error while getting a device context!", pluginName, MB_ICONERROR | MB_OK );
		return FALSE;
	}

	if ((pixelFormat = ChoosePixelFormat( OGL.hDC, &pfd )) == 0)
	{
		MessageBox( hWnd, "Unable to find a suitable pixel format!", pluginName, MB_ICONERROR | MB_OK );
		OGL_Stop();
		return FALSE;
	}

	if ((SetPixelFormat( OGL.hDC, pixelFormat, &pfd )) == FALSE)
	{
		MessageBox( hWnd, "Error while setting pixel format!", pluginName, MB_ICONERROR | MB_OK );
		OGL_Stop();
		return FALSE;
	}

	if ((OGL.hRC = wglCreateContext( OGL.hDC )) == NULL)
	{
		MessageBox( hWnd, "Error while creating OpenGL context!", pluginName, MB_ICONERROR | MB_OK );
		OGL_Stop();
		return FALSE;
	}

	if ((wglMakeCurrent( OGL.hDC, OGL.hRC )) == FALSE)
	{
		MessageBox( hWnd, "Error while making OpenGL context current!", pluginName, MB_ICONERROR | MB_OK );
		OGL_Stop();
		return FALSE;
	}

	OGL_InitExtensions();

	OGL_InitStates();

	TextureCache_Init();
	return TRUE;
}

void OGL_StopFullscreen()
{
	if (OGL.hFullscreenRC)
	{
		if (!wglMakeCurrent( NULL, NULL ))
			MessageBox( hFullscreen, "Error while uncurrentizing the OpenGL context!", pluginName, MB_ICONERROR | MB_OK );

		if (!wglDeleteContext( OGL.hFullscreenRC ))
			MessageBox( hFullscreen, "Error while deleting the OpenGL context!", pluginName, MB_ICONERROR | MB_OK );

		OGL.hFullscreenRC = NULL;
	}

	if (OGL.hFullscreenDC)
	{
		if (!ReleaseDC( hFullscreen, OGL.hFullscreenDC ))
			MessageBox( hFullscreen, "Error while releasing the device context!", pluginName, MB_ICONERROR | MB_OK );

		OGL.hFullscreenDC = NULL;
	}

	OGL.fullscreen = FALSE;
}

BOOL OGL_StartFullscreen()
{
	int		pixelFormat;
	PIXELFORMATDESCRIPTOR pfd = { 
		sizeof(PIXELFORMATDESCRIPTOR),    // size of this pfd 
		1,                                // version number 
		PFD_DRAW_TO_WINDOW |              // support window 
		PFD_SUPPORT_OPENGL |              // support OpenGL 
		PFD_DOUBLEBUFFER,                 // double buffered 
		PFD_TYPE_RGBA,                    // RGBA type 
		OGL.fullscreenBits,               // 24-bit color depth 
		0, 0, 0, 0, 0, 0,                 // color bits ignored 
		0,                                // no alpha buffer 
		0,                                // shift bit ignored 
		0,                                // no accumulation buffer 
		0, 0, 0, 0,                       // accum bits ignored 
		OGL.fullscreenBits,               // 32-bit z-buffer     
		0,                                // no stencil buffer 
		0,                                // no auxiliary buffer 
		PFD_MAIN_PLANE,                   // main layer 
		0,                                // reserved 
		0, 0, 0                           // layer masks ignored 
	};

	if ((OGL.hFullscreenDC = GetDC( hFullscreen )) == NULL)
	{
		MessageBox( hFullscreen, "Error while getting a device context!", pluginName, MB_ICONERROR | MB_OK );
		return FALSE;
	}

	if ((pixelFormat = ChoosePixelFormat( OGL.hFullscreenDC, &pfd )) == 0)
	{
		MessageBox( hFullscreen, "Unable to find a suitable pixel format!", pluginName, MB_ICONERROR | MB_OK );
		return FALSE;
	}

	if ((SetPixelFormat( OGL.hFullscreenDC, pixelFormat, &pfd )) == FALSE)
	{
		MessageBox( hFullscreen, "Error while setting pixel format!", pluginName, MB_ICONERROR | MB_OK );
		return FALSE;
	}

	if ((OGL.hFullscreenRC = wglCreateContext( OGL.hFullscreenDC )) == NULL)
	{
		MessageBox( hFullscreen, "Error while creating OpenGL context!", pluginName, MB_ICONERROR | MB_OK );
		OGL_StopFullscreen();
		return FALSE;
	}

	wglShareLists( OGL.hRC, OGL.hFullscreenRC );

	if ((wglMakeCurrent( OGL.hFullscreenDC, OGL.hFullscreenRC )) == FALSE)
	{
		MessageBox( hFullscreen, "Error while making OpenGL context current!", pluginName, MB_ICONERROR | MB_OK );
		OGL_StopFullscreen();
		return FALSE;
	}

	OGL_InitStates();
	Combiner_Init();

	return TRUE;
}

void OGL_ToggleFullscreen()
{
	if (!OGL.fullscreen)
	{
		if (OGL.fullscreen = OGL_StartFullscreen())
		{
			OGL.width = OGL.fullscreenWidth;
			OGL.height = OGL.fullscreenHeight;
			OGL.heightOffset = 0;

			OGL_UpdateScale();

			RSP.update = UPDATE_ALL;
		}
	}
	else
	{
		OGL_StopFullscreen();

		wglMakeCurrent( OGL.hDC, OGL.hRC );

		OGL.fullscreen = FALSE;

		OGL.width = OGL.windowedWidth;
		OGL.height = OGL.windowedHeight;

		RSP.update = UPDATE_ALL;
	}
}

void OGL_Stop()
{
	TextureCache_Destroy();

	if (OGL.hFullscreenRC)
		OGL_StopFullscreen();

	if (OGL.hRC)
	{
		if (!wglMakeCurrent( NULL, NULL ))
			MessageBox( hWnd, "Error while uncurrentizing the OpenGL context!", pluginName, MB_ICONERROR | MB_OK );

		if (!wglDeleteContext( OGL.hRC ))
			MessageBox( hWnd, "Error while deleting the OpenGL context!", pluginName, MB_ICONERROR | MB_OK );

		OGL.hRC = NULL;
	}

	if (OGL.hDC)
	{
		if (!ReleaseDC( hWnd, OGL.hDC ))
			MessageBox( hWnd, "Error while releasing the device context!", pluginName, MB_ICONERROR | MB_OK );

		OGL.hDC = NULL;
	}
}

void OGL_UpdateStates()
{
	static DWORD oldOtherMode_H, oldGeometryMode;
	RDP.changed.otherMode_H = oldOtherMode_H ^ RDP.otherMode_H;
	RSP.changed.geometryMode = oldGeometryMode ^ RSP.geometryMode;

	// Cull mode
	if ((RSP.changed.geometryMode & RSP_GEOMETRYMODE_CULL_BOTH) ||
		(RSP.update & UPDATE_CULLMODE))
	{
 		if (RSP.geometryMode & RSP_GEOMETRYMODE_CULL_BOTH)
		{
			glEnable( GL_CULL_FACE );

			if (RSP.geometryMode & RSP_GEOMETRYMODE_CULL_BACK)
				glCullFace( GL_BACK );
			else
				glCullFace( GL_FRONT );
		}
		else
			glDisable( GL_CULL_FACE );

		RSP.update &= ~UPDATE_CULLMODE;
	}

	// Z buffer
   	if ((RSP.changed.geometryMode & RSP_GEOMETRYMODE_ZBUFFER) ||
		(RSP.update & UPDATE_ZMODE))
	{
		if (RSP.geometryMode & RSP_GEOMETRYMODE_ZBUFFER)
			glEnable( GL_DEPTH_TEST );
		else
			glDisable( GL_DEPTH_TEST );
		RSP.update &= ~UPDATE_ZMODE;
	}

	// Z compare
	if ((RDP.changed.otherMode_L & RENDERMODE_Z_COMPARE) ||
		(RSP.update & UPDATE_ZCOMPARE))
	{
		if (RDP.otherMode_L & RENDERMODE_Z_COMPARE)
			glDepthFunc( GL_LESS );
		else
			glDepthFunc( GL_ALWAYS );
		RSP.update &= ~UPDATE_ZCOMPARE;
	}

	// Z update
	if ((RDP.changed.otherMode_L & RENDERMODE_Z_UPDATE) || // Update Z buffer
		(RDP.changed.otherMode_L & RENDERMODE_ZMODE_XLU) || // Z transparent
		(RSP.update & UPDATE_ZUPDATE))
	{
		if (RDP.otherMode_L & RENDERMODE_Z_UPDATE)
			glDepthMask( GL_TRUE );
		else
			glDepthMask( GL_FALSE );

		RSP.update &= ~UPDATE_ZUPDATE;
	}

	// Z bias
	if ((RDP.changed.otherMode_L & RENDERMODE_ZMODE_INTER) ||
		(RSP.update & UPDATE_ZINTER))
	{
		if (RDP.otherMode_L & RENDERMODE_ZMODE_INTER)
			glEnable( GL_POLYGON_OFFSET_FILL );
		else
			glDisable( GL_POLYGON_OFFSET_FILL );
		RSP.update &= ~UPDATE_ZINTER;
	}

	// Enable alpha test for threshold mode
	if (((RDP.otherMode_L & ALPHACOMPARE) == ALPHACOMPARE_THRESHOLD) && !(RDP.otherMode_L & RENDERMODE_ALPHA_CVG_SEL))
	{
		glEnable( GL_ALPHA_TEST );

		glAlphaFunc( (RDP.blendColor.a > 0.0f) ? GL_GEQUAL : GL_GREATER, RDP.blendColor.a );
	}
	// Used in TEX_EDGE and similar render modes
 	else if (RDP.otherMode_L & RENDERMODE_CVG_X_ALPHA)
	{
		glEnable( GL_ALPHA_TEST );

		// Arbitrary number -- gives nice results though
		glAlphaFunc( GL_GEQUAL, 0.5f );
	}
	else
		glDisable( GL_ALPHA_TEST );
	
	if (((RDP.otherMode_L & ALPHACOMPARE) == ALPHACOMPARE_DITHER) && !(RDP.otherMode_L & RENDERMODE_ALPHA_CVG_SEL))
		glEnable( GL_POLYGON_STIPPLE );
	else
		glDisable( GL_POLYGON_STIPPLE );

	// Once the blender code matures this should be handled there
	if ((RSP.changed.geometryMode & RSP_GEOMETRYMODE_FOG) || (RSP.update & UPDATE_FOG))
	{
		if ((RSP.geometryMode & RSP_GEOMETRYMODE_FOG) && (OGL.fog))
			glEnable( GL_FOG );
 		else
			glDisable( GL_FOG );
 	}

	if ((RDP.changed.otherMode_H & CYCLETYPE) || (RSP.update & UPDATE_COMBINE))
		Combiner_UpdateCombineMode();

	if (RSP.update & UPDATE_COMBINE_COLORS)
		Combiner_UpdateCombineColors();

	if (RDP.useT0)
	{
		glActiveTextureARB( GL_TEXTURE0_ARB );
		glEnable( GL_TEXTURE_2D );
	}
	else
	{
		glActiveTextureARB( GL_TEXTURE0_ARB );
		glDisable( GL_TEXTURE_2D );
	}

	if ((RDP.useT1) || (RDP.useNoise)) // hacked in noise support - will clean up later
	{
		glActiveTextureARB( GL_TEXTURE1_ARB );
		glEnable( GL_TEXTURE_2D );
	}
	else
	{
		glActiveTextureARB( GL_TEXTURE1_ARB );
		glDisable( GL_TEXTURE_2D );
	}

	if (RSP.update & UPDATE_TEXTURES)
 	{ 
		if (RDP.useT0)
 			TextureCache_ActivateTexture( 0 );

		if (RDP.useT1)
 			TextureCache_ActivateTexture( 1 );

		RSP.update &= ~UPDATE_TEXTURES;
	}

	if ((RDP.useNoise) && (!RDP.useT1))
		TextureCache_ActivateNoise( 1 );

	if (RSP.update & UPDATE_VIEWPORT)
	{
		glViewport( RSP.viewport.x * OGL.scaleX, (RDP.height - RSP.viewport.y) * OGL.scaleY + OGL.heightOffset, RSP.viewport.width * OGL.scaleX, RSP.viewport.height * OGL.scaleY ); 
		glDepthRange( RSP.viewport.nearZ, RSP.viewport.farZ );
		RSP.update &= ~UPDATE_VIEWPORT;
	}

	// commented out because blender code seemes to work just as well
// 	if ((RDP.otherMode_L & RENDERMODE_ALPHA_CVG_SEL) || // No coverage emulation
//		((RDP.otherMode_H & CYCLETYPE) == CYCLETYPE_COPY)) // No blending in copy mode
//		glDisable( GL_BLEND );

	// Very basic, and experimental (and probably incorrect) blender support
	// Results are good so far though...
	if ((RDP.otherMode_L & RENDERMODE_FORCE_BL) &&
		((RDP.otherMode_H & CYCLETYPE) != CYCLETYPE_COPY) &&
		((RDP.otherMode_H & CYCLETYPE) != CYCLETYPE_FILL))
	{
 		glEnable( GL_BLEND );

		switch (RDP.otherMode_L >> 16)
		{
			case 0x0448: // Add
			case 0x055A:
				glBlendFunc( GL_ONE, GL_ONE );
				break;
			case 0x0C08: // 1080 Sky
			case 0x0F0A: // Used LOTS of places
				glBlendFunc( GL_ONE, GL_ZERO );
				break;
			case 0xC810: // Blends fog
			case 0xC811: // Blends fog
			case 0x0C18: // Standard interpolated blend
			case 0x0C19: // Used for antialiasing
			case 0x0050: // Standard interpolated blend
			case 0x0055: // Used for antialiasing
				glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
				break;
			case 0x0FA5: // Seems to be doing just blend color - maybe combiner can be used for this?
			case 0x5055: // Used in Paper Mario intro, I'm not sure if this is right...
				glBlendFunc( GL_ZERO, GL_ONE );
				break;
			default:
				glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
				break;
		}
	}
	else
		glDisable( GL_BLEND );

  	RSP.update = 0;
	oldOtherMode_H = RDP.otherMode_H;
	oldGeometryMode = RSP.geometryMode;
	RDP.changed.otherMode_L = 0x00000000;
	RDP.changed.otherMode_H = 0x00000000;
	RSP.changed.geometryMode = 0x00000000;
}

void OGL_AdjustTexCoords( BYTE i )
{
	OGL.vertices[i].s0 *= cache.current[0]->scaleS;
	OGL.vertices[i].t0 *= cache.current[0]->scaleT;
}

void OGL_AddTriangle( BYTE tri[3] )
{
	OGL_UpdateStates();

	for (int i = 0; i < 3; i++)
	{
		OGL.triangles[OGL.numTriangles][i] = tri[i];

		if (TRUE) //RSP.vertices[tri[i]].changed) -- need to fix some state handling first
		{
			Vertex *src = &RSP.vertices[tri[i]];
			GLVertex *dest = &OGL.vertices[tri[i]];

			dest->x = src->x;
			dest->y = src->y;
			dest->z = (RDP.otherMode_L & ZSOURCE_PRIMITIVE) ? (RDP.primDepth * src->w) : ((RSP.geometryMode & RSP_GEOMETRYMODE_ZBUFFER) ? src->z : 0.0f);
			dest->w = src->w;

			if (RDP.useT0)
			{
				dest->s0 = src->s * cache.current[0]->scaleS + (OGL.originAdjust - RSP.textureTile[0]->fulS) * cache.current[0]->offsetScaleS;
				dest->t0 = src->t * cache.current[0]->scaleT + (OGL.originAdjust - RSP.textureTile[0]->fulT) * cache.current[0]->offsetScaleT;
			}

			if (RDP.useT1)
			{
				dest->s1 = src->s * cache.current[1]->scaleS + (OGL.originAdjust - RSP.textureTile[1]->fulS) * cache.current[1]->offsetScaleS;
				dest->t1 = src->t * cache.current[1]->scaleT + (OGL.originAdjust - RSP.textureTile[1]->fulT) * cache.current[1]->offsetScaleT;
			}

			dest->r = src->r;
			dest->g = src->g;
			dest->b = src->b;
			dest->a = src->a;

			SetConstant( OGL.vertices[tri[i]], combiner.vertex.color, combiner.vertex.alpha );

			if ((RSP.geometryMode & RSP_GEOMETRYMODE_FOG) && (OGL.fog))
			{
				if (dest->z < 0.0f)
					dest->fog = 0.0f;
				else
					dest->fog = max( 0.0f, dest->z / dest->w * RSP.fogMultiplier + RSP.fogOffset );
			}

			src->changed = FALSE;
		}
	}
	OGL.numTriangles++;
}

void OGL_DrawTriangles()
{
	// Okay this is a flat out hack for Mario 64. It only works if alpha comes from environment color...
	if (((RDP.otherMode_L & ALPHACOMPARE) == ALPHACOMPARE_DITHER) &&
		!(RDP.otherMode_L & RENDERMODE_ALPHA_CVG_SEL))
	{
		OGL.lastStipple = (OGL.lastStipple + 1) & 0x7;
		glPolygonStipple( OGL.stipplePattern[(BYTE)(RDP.envColor.a * 255.0f) >> 3][OGL.lastStipple] );
	}

	OGL_UpdateStates();
	glDrawElements( GL_TRIANGLES, OGL.numTriangles * 3, GL_UNSIGNED_BYTE, OGL.triangles );
	OGL.numTriangles = 0;
}
 
void OGL_DrawRect( GLfloat x0, GLfloat y0, GLfloat x1, GLfloat y1, GLcolor color )
{
	OGL_UpdateStates();

	glDisable( GL_ALPHA_TEST );
	glDisable( GL_CULL_FACE );
	glDisable( GL_POLYGON_OFFSET_FILL );
 
	glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
	glOrtho( 0, RDP.width, RDP.height, 0, -1.0f, 1.0f );

	glViewport( 0, OGL.heightOffset, OGL.width, OGL.height );

 	glColor4f( color.r, color.g, color.b, color.a );

	glBegin(GL_QUADS);
		glVertex4f( x0, y0, (RDP.otherMode_L & ZSOURCE_PRIMITIVE) ? RDP.primDepth : 0.0f, 1.0f );
		glVertex4f( x0, y1 + 1,(RDP.otherMode_L & ZSOURCE_PRIMITIVE) ? RDP.primDepth : 0.0f, 1.0f ); 
		glVertex4f( x1 + 1, y1 + 1, (RDP.otherMode_L & ZSOURCE_PRIMITIVE) ? RDP.primDepth : 0.0f, 1.0f );
		glVertex4f( x1 + 1, y0, (RDP.otherMode_L & ZSOURCE_PRIMITIVE) ? RDP.primDepth : 0.0f, 1.0f );
	glEnd();

	glLoadIdentity();

	RSP.update |= UPDATE_CULLMODE | UPDATE_ZINTER | UPDATE_VIEWPORT;
}

void OGL_DrawTexturedRect( float x0, float u0, float y0, float v0,
						  float x1, float u1, float y1, float v1 )
{
	GLVertex rect[2] =
	{
		{ x0, y0, RSP.viewport.nearZ, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, u0, v0, u0, v0 },
		{ x1, y1, RSP.viewport.nearZ, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, u1, v1, u1, v1 },
	};

	if (RDP.otherMode_L & ZSOURCE_PRIMITIVE)
	{
		rect[0].z = RDP.primDepth;
		rect[1].z = RDP.primDepth;
	}

	OGL_UpdateStates();

	// Disable all the stuff not used in TexRect
	glDisable( GL_CULL_FACE );
	glDisable( GL_POLYGON_OFFSET_FILL );
	//glDisable( GL_FOG );

	if (OGL.EXT_fog_coord)
		glFogCoordfEXT( 0.0f );

	// Adjust for OpenGL's corner-based texture coords
	if (u0 < u1) u1 += 1.0f; else u0 += 1.0f;
	if (v0 < v1) v1 += 1.0f; else v0 += 1.0f;

	// No blending in copy mode
	if (((RDP.otherMode_H & TEXTFILT_BILERP) || (RDP.otherMode_H & TEXTFILT_AVERAGE) || (OGL.forceBilinear)) &&
   		(((RDP.otherMode_H & CYCLETYPE) != CYCLETYPE_COPY) || (OGL.forceBilinear)))
	{
		// Adjust for bilinear filtering
		if (!cache.current[0]->clampS)
		{
			u0 += (u0 < u1) ? OGL.originAdjust : -OGL.originAdjust;
			u1 += (u0 < u1) ? -OGL.originAdjust : OGL.originAdjust;
		}

		if (!cache.current[0]->clampT)
		{
			v0 += (v0 < v1) ? OGL.originAdjust : -OGL.originAdjust;
			v1 += (u0 < u1) ? -OGL.originAdjust : OGL.originAdjust;
		}

		if (OGL.ARB_multitexture)
		{
			glActiveTextureARB( GL_TEXTURE0_ARB );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

			glActiveTextureARB( GL_TEXTURE1_ARB );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		}
		else
		{
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		}
	}
	else
	{
		if (OGL.ARB_multitexture)
		{
			glActiveTextureARB( GL_TEXTURE0_ARB );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

			glActiveTextureARB( GL_TEXTURE1_ARB );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		}
		else
		{
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		}
	}

	// Shift and scale
	if (RDP.useT0)
	{
		rect[0].s0 = u0 * cache.current[0]->scaleS - RSP.textureTile[0]->fulS * cache.current[0]->offsetScaleS;
		rect[0].t0 = v0 * cache.current[0]->scaleT - RSP.textureTile[0]->fulT * cache.current[0]->offsetScaleT;
		rect[1].s0 = u1 * cache.current[0]->scaleS - RSP.textureTile[0]->fulS * cache.current[0]->offsetScaleS;
		rect[1].t0 = v1 * cache.current[0]->scaleT - RSP.textureTile[0]->fulT * cache.current[0]->offsetScaleT;
	}
	
	if (RDP.useT1)
	{
		rect[0].s1 = u0 * cache.current[1]->scaleS - RSP.textureTile[1]->fulS * cache.current[1]->offsetScaleS;
		rect[0].t1 = v0 * cache.current[1]->scaleT - RSP.textureTile[1]->fulT * cache.current[1]->offsetScaleT;
		rect[1].s1 = u1 * cache.current[1]->scaleS - RSP.textureTile[1]->fulS * cache.current[1]->offsetScaleS;
		rect[1].t1 = v1 * cache.current[1]->scaleT - RSP.textureTile[1]->fulT * cache.current[1]->offsetScaleT;
	}

	if (RDP.useNoise)
	{
		rect[0].s1 = 0.0f;
		rect[0].t1 = 0.0f;
		rect[1].s1 = (x1 - x0) / 64.0f;
		rect[1].t1 = (y1 - y0) / 64.0f;

		glActiveTextureARB( GL_TEXTURE1_ARB );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	}

	SetConstant( rect[0], combiner.vertex.color, combiner.vertex.alpha );

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	glOrtho( 0, RDP.width, RDP.height, 0, 1.0f, -1.0f );
	glViewport( 0, OGL.heightOffset, OGL.width, OGL.height );

   	glBegin(GL_QUADS); 
		glColor4f( rect[0].r, rect[0].g, rect[0].b, rect[0].a );

		if (OGL.ARB_multitexture)
		{
			glMultiTexCoord2fARB( GL_TEXTURE0_ARB, rect[0].s0, rect[0].t0 );
			glMultiTexCoord2fARB( GL_TEXTURE1_ARB, rect[0].s1, rect[0].t1 );
			glVertex4f( rect[0].x, rect[0].y, rect[0].z, 1.0f );

			glMultiTexCoord2fARB( GL_TEXTURE0_ARB, rect[0].s0, rect[1].t0 );
			glMultiTexCoord2fARB( GL_TEXTURE1_ARB, rect[0].s1, rect[1].t1 );
			glVertex4f( rect[0].x, rect[1].y, rect[0].z, 1.0f );

			glMultiTexCoord2fARB( GL_TEXTURE0_ARB, rect[1].s0, rect[1].t0 );
			glMultiTexCoord2fARB( GL_TEXTURE1_ARB, rect[1].s1, rect[1].t1 );
			glVertex4f( rect[1].x, rect[1].y, rect[0].z, 1.0f );

			glMultiTexCoord2fARB( GL_TEXTURE0_ARB, rect[1].s0, rect[0].t0 );
			glMultiTexCoord2fARB( GL_TEXTURE1_ARB, rect[1].s1, rect[0].t1 );
			glVertex4f( rect[1].x, rect[0].y, rect[0].z, 1.0f );
		}
		else
		{
 			glTexCoord2f( rect[0].s0, rect[0].t0 );
			glVertex4f( rect[0].x, rect[0].y, rect[0].z, 1.0f );

			glTexCoord2f( rect[0].s0, rect[1].t0 );
			glVertex4f( rect[0].x, rect[1].y, rect[0].z, 1.0f );

 			glTexCoord2f( rect[1].s0, rect[1].t0 );
			glVertex4f( rect[1].x, rect[1].y, rect[0].z, 1.0f );

 			glTexCoord2f( rect[1].s0, rect[0].t0 );
			glVertex4f( rect[1].x, rect[0].y, rect[0].z, 1.0f );
		}
	glEnd();

	glLoadIdentity();
 
	// Need to reset the states I messed up
 	RSP.update |= UPDATE_CULLMODE | UPDATE_ZINTER | UPDATE_VIEWPORT | UPDATE_FOG;
}