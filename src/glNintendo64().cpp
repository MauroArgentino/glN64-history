#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <process.h>
#include "glNintendo64().h"
#include "Debug.h"
#include "Zilmar GFX 1.3.h"
#include "OpenGL.h"
#include "N64.h"
#include "RSP.h"
#include "RDP.h"
#include "Config.h"
#include "Textures.h"

HWND		hWnd;
HWND		hStatusBar;
HWND		hFullscreen;
HINSTANCE	hInstance;

char		pluginName[] = "glNintendo64() v0.3";

void (*CheckInterrupts)( void );

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD dwReason, LPVOID lpvReserved)
{
	hInstance = hinstDLL;
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		Config_LoadConfig();
		RSP.thread = NULL;
		OGL.hRC = NULL;
		OGL.hDC = NULL;
		OGL.hFullscreenRC = NULL;
		OGL.hFullscreenDC = NULL;
		hFullscreen = NULL;
	}
	return TRUE;
}

EXPORT void CALL CaptureScreen ( char * Directory )
{
	return;
}

LRESULT CALLBACK FullscreenWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	switch (uMsg)
	{
		case WM_SYSCOMMAND:
			switch (wParam)
			{
				case SC_SCREENSAVE:
				case SC_MONITORPOWER:
					return 0;
			}
			break;

		case WM_CLOSE:
			PostQuitMessage(0);
			return 0;

		case WM_MOUSEACTIVATE:
			return MA_NOACTIVATEANDEAT;

/*		case WM_ACTIVATE:
			switch (wParam)
			{
				case WA_ACTIVE:
					DEVMODE fullscreenMode;
					memset( &fullscreenMode, 0, sizeof(DEVMODE) );
					fullscreenMode.dmSize = sizeof(DEVMODE);
					fullscreenMode.dmPelsWidth			= OGL.fullscreenWidth;
					fullscreenMode.dmPelsHeight			= OGL.fullscreenHeight;
					fullscreenMode.dmBitsPerPel			= OGL.fullscreenBits;
					fullscreenMode.dmDisplayFrequency	= OGL.fullscreenRefresh;
					fullscreenMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;

					if (ChangeDisplaySettings( &fullscreenMode, CDS_FULLSCREEN )!= DISP_CHANGE_SUCCESSFUL)
					{
						MessageBox( NULL, "Failed to change display mode", pluginName, MB_ICONERROR | MB_OK );
					}

					ShowWindow( hWnd, SW_RESTORE );
					ShowCursor( FALSE );
					break;

				case WA_INACTIVE:
					ShowWindow( hWnd, SW_MINIMIZE );
					ShowCursor( TRUE );
					ChangeDisplaySettings( NULL, 0 );
					break;
			}
			return 0;*/
	}
		
	return DefWindowProc( hWnd, uMsg, wParam, lParam );
}

BOOL CreateFullscreen()
{
	DEVMODE fullscreenMode;
	memset( &fullscreenMode, 0, sizeof(DEVMODE) );
	fullscreenMode.dmSize = sizeof(DEVMODE);
	fullscreenMode.dmPelsWidth			= OGL.fullscreenWidth;
	fullscreenMode.dmPelsHeight			= OGL.fullscreenHeight;
	fullscreenMode.dmBitsPerPel			= OGL.fullscreenBits;
	fullscreenMode.dmDisplayFrequency	= OGL.fullscreenRefresh;
	fullscreenMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;

	if (ChangeDisplaySettings( &fullscreenMode, CDS_FULLSCREEN )!= DISP_CHANGE_SUCCESSFUL)
	{
		MessageBox( NULL, "Failed to change display mode", pluginName, MB_ICONERROR | MB_OK );
		return FALSE;
	}

	WNDCLASS wc;

	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc		= (WNDPROC)FullscreenWndProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hInstance		= hInstance;
	wc.hIcon			= NULL;
	wc.hCursor			= NULL;
	wc.hbrBackground	= NULL;
	wc.lpszMenuName		= NULL;
	wc.lpszClassName	= "glNintendo64()";

	if (!RegisterClass(&wc))
	{
		MessageBox( NULL, "Error creating fullscreen window class", pluginName, MB_ICONERROR | MB_OK );
		ChangeDisplaySettings( NULL, 0 );
		return FALSE;
	}

	if (!(hFullscreen = CreateWindowEx(
		WS_EX_TOPMOST,
		"glNintendo64()",
		pluginName,
		WS_CLIPSIBLINGS |
		WS_CLIPCHILDREN |
		WS_POPUP,
		0, 0,
		OGL.fullscreenWidth,
		OGL.fullscreenHeight,
		hWnd,
		NULL,
		hInstance,
		NULL)))	
	{
		MessageBox( NULL, "Failed to create fullscreen window", pluginName, MB_ICONERROR | MB_OK );

		ChangeDisplaySettings( NULL, 0 );
		UnregisterClass( "glNintendo64()", hInstance );
		return FALSE;
	}

	ShowWindow( hFullscreen, SW_SHOW );
	ShowCursor( FALSE );
}

void DestroyFullscreen()
{
	if (hFullscreen)
	{
		ShowCursor( TRUE );
		DestroyWindow( hFullscreen );
		UnregisterClass( "glNintendo64()", hInstance );
		ChangeDisplaySettings( NULL, 0 );

		hFullscreen = NULL;
	}
}

EXPORT void CALL ChangeWindow (void)
{
	if (!OGL.fullscreen)
		if (!CreateFullscreen())
			return;

	// Anything affecting OpenGL has to be done in the RSP thread,
	SetEvent( RSP.threadMsg[RSPMSG_CHANGEWINDOW] );
	WaitForSingleObject( RSP.threadFinished, INFINITE );

	// and anything affecting the main window has to be done is this thread.
	if (!OGL.fullscreen)
	{
		DestroyFullscreen();

		OGL_ResizeWindow( OGL.width, OGL.height );
	}
}

EXPORT void CALL CloseDLL (void)
{
}

EXPORT void CALL DllAbout ( HWND hParent )
{
	MessageBox( hParent, "glNintendo64() v0.3 by Orkin", pluginName, MB_OK );
}

EXPORT void CALL DllConfig ( HWND hParent )
{
	Config_DoConfig();
}

EXPORT void CALL DllTest ( HWND hParent )
{
}

EXPORT void CALL DrawScreen (void)
{
}

EXPORT void CALL GetDllInfo ( PLUGIN_INFO * PluginInfo )
{
	PluginInfo->Version = 0x103;
	PluginInfo->Type = PLUGIN_TYPE_GFX;
	strcpy( PluginInfo->Name, pluginName );
	PluginInfo->NormalMemory = FALSE;
	PluginInfo->MemoryBswaped = TRUE;
}

EXPORT BOOL CALL InitiateGFX (GFX_INFO Gfx_Info)
{
	hWnd = Gfx_Info.hWnd;
	hStatusBar = Gfx_Info.hStatusBar;

	DMEM = Gfx_Info.DMEM;
	IMEM = Gfx_Info.IMEM;
	RDRAM = Gfx_Info.RDRAM;

	REG.MI_INTR = Gfx_Info.MI_INTR_REG;
	REG.DPC_START = Gfx_Info.DPC_START_REG;
	REG.DPC_END = Gfx_Info.DPC_END_REG;
	REG.DPC_CURRENT = Gfx_Info.DPC_CURRENT_REG;
	REG.DPC_STATUS = Gfx_Info.DPC_STATUS_REG;
	REG.DPC_CLOCK = Gfx_Info.DPC_CLOCK_REG;
	REG.DPC_BUFBUSY = Gfx_Info.DPC_BUFBUSY_REG;
	REG.DPC_PIPEBUSY = Gfx_Info.DPC_PIPEBUSY_REG;
	REG.DPC_TMEM = Gfx_Info.DPC_TMEM_REG;

	REG.VI_STATUS = Gfx_Info.VI_STATUS_REG;
	REG.VI_ORIGIN = Gfx_Info.VI_ORIGIN_REG;
	REG.VI_WIDTH = Gfx_Info.VI_WIDTH_REG;
	REG.VI_INTR = Gfx_Info.VI_INTR_REG;
	REG.VI_V_CURRENT_LINE = Gfx_Info.VI_V_CURRENT_LINE_REG;
	REG.VI_TIMING = Gfx_Info.VI_TIMING_REG;
	REG.VI_V_SYNC = Gfx_Info.VI_V_SYNC_REG;
	REG.VI_H_SYNC = Gfx_Info.VI_H_SYNC_REG;
	REG.VI_LEAP = Gfx_Info.VI_LEAP_REG;
	REG.VI_H_START = Gfx_Info.VI_H_START_REG;
	REG.VI_V_START = Gfx_Info.VI_V_START_REG;
	REG.VI_V_BURST = Gfx_Info.VI_V_BURST_REG;
	REG.VI_X_SCALE = Gfx_Info.VI_X_SCALE_REG;
	REG.VI_Y_SCALE = Gfx_Info.VI_Y_SCALE_REG;

	CheckInterrupts = Gfx_Info.CheckInterrupts;

	return TRUE;
}

EXPORT void CALL MoveScreen (int xpos, int ypos)
{
}

EXPORT void CALL ProcessDList(void)
{
	if (RSP.thread)
	{
		SetEvent( RSP.threadMsg[RSPMSG_PROCESSDLIST] );
		WaitForSingleObject( RSP.threadFinished, INFINITE );
	}
}

EXPORT void CALL ProcessRDPList(void)
{
}

EXPORT void CALL RomClosed (void)
{
	int i;

	if (RSP.thread)
	{
		SetEvent( RSP.threadMsg[RSPMSG_CLOSE] );
		WaitForSingleObject( RSP.threadFinished, INFINITE );
		for (i = 0; i < 4; i++)
			if (RSP.threadMsg[i])
				CloseHandle( RSP.threadMsg[i] );
		CloseHandle( RSP.threadFinished );
		CloseHandle( RSP.thread );
	}

	RSP.thread = NULL;

#ifdef DEBUG
	CloseDebugDlg();
#endif
}

EXPORT void CALL RomOpen (void)
{
	DWORD threadID;
	int i;

	// Create RSP message events
	for (i = 0; i < 4; i++) 
	{ 
		RSP.threadMsg[i] = CreateEvent( NULL, FALSE, FALSE, NULL );
		if (RSP.threadMsg[i] == NULL)
		{ 
			MessageBox( hWnd, "Error creating video thread message events, closing video thread...", "glNintendo64() Error", MB_OK | MB_ICONERROR );
			return;
		} 
	} 

	// Create RSP finished event
	RSP.threadFinished = CreateEvent( NULL, FALSE, FALSE, NULL );
	if (RSP.threadFinished == NULL)
	{ 
		MessageBox( hWnd, "Error creating video thread finished event, closing video thread...", "glNintendo64() Error", MB_OK | MB_ICONERROR );
		return;
	} 

	RSP.thread = CreateThread( NULL, 4096, RSP_ThreadProc, NULL, NULL, &threadID );
	//WaitForSingleObject( RSP.threadFinished, INFINITE );

#ifdef DEBUG
	OpenDebugDlg();
	RSP.dumpNextDL = TRUE;
#endif
}

EXPORT void CALL ShowCFB (void)
{
	
}

EXPORT void CALL UpdateScreen (void)
{
	if (RSP.thread)
	{
		SetEvent( RSP.threadMsg[RSPMSG_SWAPBUFFERS] );
		WaitForSingleObject( RSP.threadFinished, INFINITE );
	}
}

EXPORT void CALL ViStatusChanged (void)
{
}

EXPORT void CALL ViWidthChanged (void)
{
}