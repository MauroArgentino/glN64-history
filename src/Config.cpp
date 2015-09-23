#include <windows.h>
#include "Config.h"
#include "glNintendo64().h"
#include "Resource.h"
#include "RSP.h"
#include "Textures.h"
#include "OpenGL.h"
#include <commctrl.h>

HWND hConfigDlg;

struct
{
	struct
	{
		DWORD width, height, bitDepth, refreshRate;
	} selected;

	DWORD bitDepth[4];

	struct
	{
		DWORD	width, height;
	} resolution[32];

	DWORD refreshRate[32];
	
	DWORD	numBitDepths;
	DWORD	numResolutions;
	DWORD	numRefreshRates;
} fullscreen;

#define numWindowedModes 12

struct
{
	WORD width, height;
	char *description;
} windowedModes[12] = {
	{ 320, 240, "320 x 240" },
	{ 400, 300, "400 x 300" },
	{ 480, 360, "480 x 360" },
	{ 640, 480, "640 x 480" },
	{ 800, 600, "800 x 600" },
	{ 960, 720, "960 x 720" },
	{ 1024, 768, "1024 x 768" },
	{ 1152, 864, "1152 x 864" },
	{ 1280, 960, "1280 x 960" },
	{ 1280, 1024, "1280 x 1024" },
	{ 1440, 1080, "1440 x 1080" },
	{ 1600, 1200, "1600 x 1200" }
};

void Config_LoadConfig()
{
	int i;
	char text[256];
	OGL.fullscreenWidth = GetPrivateProfileInt( "Display", "Fullscreen Width", 640, "plugin\\glNintendo64().ini" );
	OGL.fullscreenHeight = GetPrivateProfileInt( "Display", "Fullscreen Height", 480, "plugin\\glNintendo64().ini" );
	OGL.fullscreenBits = GetPrivateProfileInt( "Display", "Fullscreen Bit Depth", 16, "plugin\\glNintendo64().ini" );
	OGL.fullscreenRefresh = GetPrivateProfileInt( "Display", "Fullscreen Refresh Rate", 60, "plugin\\glNintendo64().ini" );
	OGL.windowedWidth = GetPrivateProfileInt( "Display", "Windowed Width", 640, "plugin\\glNintendo64().ini" );
	OGL.windowedHeight = GetPrivateProfileInt( "Display", "Windowed Height", 480, "plugin\\glNintendo64().ini" );
	OGL.forceBilinear = GetPrivateProfileInt( "Display", "Force Bilinear", 0, "plugin\\glNintendo64().ini" );
	OGL.enable2xSaI = GetPrivateProfileInt( "Display", "Enable 2xSaI", 0, "plugin\\glNintendo64().ini" );
	OGL.fog = GetPrivateProfileInt( "Display", "Fog", 1, "plugin\\glNintendo64().ini" );
	
	cache.maxBytes = GetPrivateProfileInt( "Emulation", "Cache Size", 32, "plugin\\glNintendo64().ini" ) * 1048576;
	OGL.combiner = GetPrivateProfileInt( "Emulation", "Combiner", 0, "plugin\\glNintendo64().ini" );
}

void Config_SaveConfig()
{
	int i;
	char text[256];

	itoa( OGL.fullscreenBits, text, 10 );
	WritePrivateProfileString( "Display", "Fullscreen Bit Depth", text, "plugin\\glNintendo64().ini" );
	itoa( OGL.fullscreenWidth, text, 10 );
	WritePrivateProfileString( "Display", "Fullscreen Width", text, "plugin\\glNintendo64().ini" );
	itoa( OGL.fullscreenHeight, text, 10 );
	WritePrivateProfileString( "Display", "Fullscreen Height", text, "plugin\\glNintendo64().ini" );
	itoa( OGL.fullscreenRefresh, text, 10 );
	WritePrivateProfileString( "Display", "Fullscreen Refresh Rate", text, "plugin\\glNintendo64().ini" );

	itoa( OGL.windowedWidth, text, 10 );
	WritePrivateProfileString( "Display", "Windowed Width", text, "plugin\\glNintendo64().ini" );
	itoa( OGL.windowedHeight, text, 10 );
	WritePrivateProfileString( "Display", "Windowed Height", text, "plugin\\glNintendo64().ini" );

	WritePrivateProfileString( "Display", "Force Bilinear", OGL.forceBilinear ? "1" : "0", "plugin\\glNintendo64().ini" );
	WritePrivateProfileString( "Display", "Enable 2xSaI", OGL.enable2xSaI ? "1" : "0", "plugin\\glNintendo64().ini" );
	WritePrivateProfileString( "Display", "Fog", OGL.fog ? "1" : "0", "plugin\\glNintendo64().ini" );

	itoa( cache.maxBytes / 1048576, text, 10 );
	WritePrivateProfileString( "Emulation", "Cache Size", text, "plugin\\glNintendo64().ini" );

	itoa( OGL.combiner, text, 10 );
	WritePrivateProfileString( "Emulation", "Combiner", text, "plugin\\glNintendo64().ini" );
}

void Config_ApplyDlgConfig( HWND hWndDlg )
{
	char text[256];
	int i;

	if ((SendDlgItemMessage( hWndDlg, IDC_FORCEUCODE, BM_GETCHECK, NULL, NULL ) == BST_CHECKED) &&
		(MessageBox( hWndDlg, "Warning: Forcing the plug-in to the incorrect microcode could cause the emulator to crash. Make sure you set the correct microcode before continuing\n\nAre you sure you want to force the microcode?", "glNintendo64() Warning", MB_YESNO ) == IDYES))
	{
		RSP.forceUCode = TRUE;
 		RSP_SetUCode( SendDlgItemMessage( hWndDlg, IDC_UCODE, CB_GETCURSEL, NULL, NULL ) );
	}
	else
	{
		RSP.forceUCode = FALSE;
		SendDlgItemMessage( hWndDlg, IDC_FORCEUCODE, BM_SETCHECK, BST_UNCHECKED, NULL );
		SendDlgItemMessage( hWndDlg, IDC_UCODE, CB_SETCURSEL, (LPARAM)RSP.uCode, NULL );
		EnableWindow( GetDlgItem( hWndDlg, IDC_UCODE ), FALSE );
	}

	SendDlgItemMessage( hWndDlg, IDC_CACHEMEGS, WM_GETTEXT, 4, (LPARAM)text );
	cache.maxBytes = atol( text ) * 1048576;

	OGL.forceBilinear = (SendDlgItemMessage( hWndDlg, IDC_FORCEBILINEAR, BM_GETCHECK, NULL, NULL ) == BST_CHECKED);
	OGL.enable2xSaI = (SendDlgItemMessage( hWndDlg, IDC_ENABLE2XSAI, BM_GETCHECK, NULL, NULL ) == BST_CHECKED);
	OGL.fog = (SendDlgItemMessage( hWndDlg, IDC_FOG, BM_GETCHECK, NULL, NULL ) == BST_CHECKED);
	OGL.originAdjust = (OGL.enable2xSaI ? 0.25 : 0.50);

	OGL.fullscreenBits = fullscreen.bitDepth[SendDlgItemMessage( hWndDlg, IDC_FULLSCREENBITDEPTH, CB_GETCURSEL, 0, 0 )];
	i = SendDlgItemMessage( hWndDlg, IDC_FULLSCREENRES, CB_GETCURSEL, 0, 0 );
	OGL.fullscreenWidth = fullscreen.resolution[i].width;
	OGL.fullscreenHeight = fullscreen.resolution[i].height;
	OGL.fullscreenRefresh = fullscreen.refreshRate[SendDlgItemMessage( hWndDlg, IDC_FULLSCREENREFRESH, CB_GETCURSEL, 0, 0 )];

	i = SendDlgItemMessage( hWndDlg, IDC_WINDOWEDRES, CB_GETCURSEL, 0, 0 );
	OGL.windowedWidth = windowedModes[i].width;
	OGL.windowedHeight = windowedModes[i].height;

	OGL.combiner = SendDlgItemMessage( hWndDlg, IDC_COMBINER, CB_GETCURSEL, NULL, NULL );

	if ((!OGL.fullscreen) && (OGL.hRC))
		OGL_ResizeWindow( OGL.windowedWidth, OGL.windowedHeight );

	Config_SaveConfig();
}

void UpdateFullscreenConfig( HWND hWndDlg )
{
	DEVMODE deviceMode;
	int i;
	char text[256];

	memset( &fullscreen.bitDepth, 0, sizeof( fullscreen.bitDepth ) );
	memset( &fullscreen.resolution, 0, sizeof( fullscreen.resolution ) );
	memset( &fullscreen.refreshRate, 0, sizeof( fullscreen.refreshRate ) );
	fullscreen.numBitDepths = 0;
	fullscreen.numResolutions = 0;
	fullscreen.numRefreshRates = 0;

	i = 0;
	SendDlgItemMessage( hWndDlg, IDC_FULLSCREENBITDEPTH, CB_RESETCONTENT, 0, 0 );
	while (EnumDisplaySettings( NULL, i, &deviceMode ) != 0)
	{
		for (int j = 0; j < fullscreen.numBitDepths; j++)
		{
			if (deviceMode.dmBitsPerPel == fullscreen.bitDepth[j])
				break;
		}
		
		if ((deviceMode.dmBitsPerPel != fullscreen.bitDepth[j]) && (deviceMode.dmBitsPerPel > 8))
		{
			fullscreen.bitDepth[fullscreen.numBitDepths] = deviceMode.dmBitsPerPel;
			sprintf( text, "%i bit", deviceMode.dmBitsPerPel );
			SendDlgItemMessage( hWndDlg, IDC_FULLSCREENBITDEPTH, CB_ADDSTRING, 0, (LPARAM)text );

			if (fullscreen.selected.bitDepth == deviceMode.dmBitsPerPel)
				SendDlgItemMessage( hWndDlg, IDC_FULLSCREENBITDEPTH, CB_SETCURSEL, fullscreen.numBitDepths, 0 );
			fullscreen.numBitDepths++;
		}

		i++;
	}

	if (SendDlgItemMessage( hWndDlg, IDC_FULLSCREENBITDEPTH, CB_GETCURSEL, 0, 0 ) == CB_ERR)
		SendDlgItemMessage( hWndDlg, IDC_FULLSCREENBITDEPTH, CB_SETCURSEL, SendDlgItemMessage( hWndDlg, IDC_FULLSCREENBITDEPTH, CB_GETCOUNT, 0, 0 ) - 1, 0 );


	i = 0;
	SendDlgItemMessage( hWndDlg, IDC_FULLSCREENRES, CB_RESETCONTENT, 0, 0 );
	while (EnumDisplaySettings( NULL, i, &deviceMode ) != 0)
	{
		for (int j = 0; j < fullscreen.numResolutions; j++)
		{
			if ((deviceMode.dmPelsWidth == fullscreen.resolution[j].width) &&
				(deviceMode.dmPelsHeight == fullscreen.resolution[j].height))
			{
				break;
			}
		}
		if (((deviceMode.dmPelsWidth != fullscreen.resolution[j].width) ||
			(deviceMode.dmPelsHeight != fullscreen.resolution[j].height)) &&
			(deviceMode.dmBitsPerPel != fullscreen.selected.bitDepth))
		{
			fullscreen.resolution[fullscreen.numResolutions].width = deviceMode.dmPelsWidth;
			fullscreen.resolution[fullscreen.numResolutions].height = deviceMode.dmPelsHeight;
			sprintf( text, "%i x %i", deviceMode.dmPelsWidth, deviceMode.dmPelsHeight );
			SendDlgItemMessage( hWndDlg, IDC_FULLSCREENRES, CB_ADDSTRING, 0, (LPARAM)text );

			if ((fullscreen.selected.width == deviceMode.dmPelsWidth) &&
				(fullscreen.selected.height == deviceMode.dmPelsHeight))
				SendDlgItemMessage( hWndDlg, IDC_FULLSCREENRES, CB_SETCURSEL, fullscreen.numResolutions, 0 );

			fullscreen.numResolutions++;
		}
		i++;
	}

	if (SendDlgItemMessage( hWndDlg, IDC_FULLSCREENRES, CB_GETCURSEL, 0, 0 ) == CB_ERR)
		SendDlgItemMessage( hWndDlg, IDC_FULLSCREENRES, CB_SETCURSEL, SendDlgItemMessage( hWndDlg, IDC_FULLSCREENRES, CB_GETCOUNT, 0, 0 ) - 1, 0 );

	i = 0;
	SendDlgItemMessage( hWndDlg, IDC_FULLSCREENREFRESH, CB_RESETCONTENT, 0, 0 );
	while (EnumDisplaySettings( NULL, i, &deviceMode ) != 0)
	{
		for (int j = 0; j < fullscreen.numRefreshRates; j++)
		{
			if ((deviceMode.dmDisplayFrequency == fullscreen.refreshRate[j]))
				break;
		}
		if ((deviceMode.dmDisplayFrequency != fullscreen.refreshRate[j]) && 
				(deviceMode.dmPelsWidth == fullscreen.selected.width) &&
				(deviceMode.dmPelsHeight == fullscreen.selected.height) &&
				(deviceMode.dmBitsPerPel == fullscreen.selected.bitDepth))
		{
			fullscreen.refreshRate[j] = deviceMode.dmDisplayFrequency;
			sprintf( text, "%i Hz", deviceMode.dmDisplayFrequency );
			SendDlgItemMessage( hWndDlg, IDC_FULLSCREENREFRESH, CB_ADDSTRING, 0, (LPARAM)text );

			if (fullscreen.selected.refreshRate == deviceMode.dmDisplayFrequency)
				SendDlgItemMessage( hWndDlg, IDC_FULLSCREENREFRESH, CB_SETCURSEL, fullscreen.numRefreshRates, 0 );

			fullscreen.numRefreshRates++;
		}
		i++;
	}
	if (SendDlgItemMessage( hWndDlg, IDC_FULLSCREENREFRESH, CB_GETCURSEL, 0, 0 ) == CB_ERR)
		SendDlgItemMessage( hWndDlg, IDC_FULLSCREENREFRESH, CB_SETCURSEL, SendDlgItemMessage( hWndDlg, IDC_FULLSCREENREFRESH, CB_GETCOUNT, 0, 0 ) - 1, 0 );
}

BOOL CALLBACK ConfigDlgProc( HWND hWndDlg, UINT message, WPARAM wParam, LPARAM lParam ) 
{ 
	char text[256];
	int	 i;
	DEVMODE deviceMode;
	switch (message) 
    { 
		case WM_INITDIALOG:
			hConfigDlg = hWndDlg;

			fullscreen.selected.width = OGL.fullscreenWidth;
			fullscreen.selected.height = OGL.fullscreenHeight;
			fullscreen.selected.bitDepth = OGL.fullscreenBits;
			fullscreen.selected.refreshRate = OGL.fullscreenRefresh;
			UpdateFullscreenConfig( hWndDlg );

			EnumDisplaySettings( NULL, ENUM_CURRENT_SETTINGS, &deviceMode );

			// Fill windowed mode resolution
			for (i = 0; i < numWindowedModes; i++)
			{
				if ((deviceMode.dmPelsWidth > windowedModes[i].width) &&
					(deviceMode.dmPelsHeight > windowedModes[i].height))
				{
					SendDlgItemMessage( hWndDlg, IDC_WINDOWEDRES, CB_ADDSTRING, 0, (LPARAM)windowedModes[i].description );
					if ((OGL.windowedWidth == windowedModes[i].width) &&
					    (OGL.windowedHeight == windowedModes[i].height))
						SendDlgItemMessage( hWndDlg, IDC_WINDOWEDRES, CB_SETCURSEL, i, 0 );
				}
			}
			SendDlgItemMessage( hWndDlg, IDC_ENABLE2XSAI, BM_SETCHECK, OGL.enable2xSaI ? (LPARAM)BST_CHECKED : (LPARAM)BST_UNCHECKED, NULL );
			// Set forced microcode check box
			SendDlgItemMessage( hWndDlg, IDC_FORCEUCODE, BM_SETCHECK, RSP.forceUCode ? (LPARAM)BST_CHECKED : (LPARAM)BST_UNCHECKED, NULL );
			// Set forced bilinear check box
			SendDlgItemMessage( hWndDlg, IDC_FORCEBILINEAR, BM_SETCHECK, OGL.forceBilinear ? (LPARAM)BST_CHECKED : (LPARAM)BST_UNCHECKED, NULL );
			// Enable/disable ucode list box
			EnableWindow( GetDlgItem( hWndDlg, IDC_UCODE ), RSP.forceUCode );
			// Enable/disable fog
			SendDlgItemMessage( hWndDlg, IDC_FOG, BM_SETCHECK, OGL.fog ? (LPARAM)BST_CHECKED : (LPARAM)BST_UNCHECKED, NULL );
			// Add Combiners
			SendDlgItemMessage( hWndDlg, IDC_COMBINER, CB_ADDSTRING, NULL, (LPARAM)"GeForce" );
			SendDlgItemMessage( hWndDlg, IDC_COMBINER, CB_ADDSTRING, NULL, (LPARAM)"OpenGL 1.4" );
			// Select active combiner
			SendDlgItemMessage( hWndDlg, IDC_COMBINER, CB_SETCURSEL, (LPARAM)OGL.combiner, NULL );
			// Add ucodes to list
			SendDlgItemMessage( hWndDlg, IDC_UCODE, CB_ADDSTRING, NULL, (LPARAM)"Fast3D Series" );
			SendDlgItemMessage( hWndDlg, IDC_UCODE, CB_ADDSTRING, NULL, (LPARAM)"Fast3D EXT" );
			SendDlgItemMessage( hWndDlg, IDC_UCODE, CB_ADDSTRING, NULL, (LPARAM)"F3DEX Series" );
			SendDlgItemMessage( hWndDlg, IDC_UCODE, CB_ADDSTRING, NULL, (LPARAM)"F3DEX2 Series" );
			// Select active ucode
			SendDlgItemMessage( hWndDlg, IDC_UCODE, CB_SETCURSEL, (LPARAM)RSP.uCode, NULL );

			_ltoa( cache.maxBytes / 1048576, text, 10 );
			SendDlgItemMessage( hWndDlg, IDC_CACHEMEGS, WM_SETTEXT, NULL, (LPARAM)text );

			// Deactivate apply button
			EnableWindow( GetDlgItem( hWndDlg, IDC_APPLY ), FALSE );

			return TRUE;

        case WM_COMMAND: 
            switch (LOWORD(wParam)) 
            { 
                case IDOK: 
					Config_ApplyDlgConfig( hWndDlg );
					EndDialog( hWndDlg, wParam );
					hConfigDlg = NULL;
					return TRUE;
 
                case IDCANCEL: 
                    EndDialog( hWndDlg, wParam );
					hConfigDlg = NULL;
                    return TRUE;

				case IDC_APPLY:
					Config_ApplyDlgConfig( hWndDlg );
					EnableWindow( GetDlgItem( hWndDlg, IDC_APPLY ), FALSE );
					return TRUE;
				case IDC_FORCEUCODE:
					EnableWindow( GetDlgItem( hWndDlg, IDC_UCODE ), SendDlgItemMessage( hWndDlg, IDC_FORCEUCODE, BM_GETCHECK, NULL, NULL ) == BST_CHECKED );
				case IDC_UCODE:
				case IDC_COMBINER:
				case IDC_FOG:
				case IDC_ENABLE2XSAI:
				case IDC_FORCEBILINEAR:
				case IDC_WIREFRAME:
					EnableWindow( GetDlgItem( hWndDlg, IDC_APPLY ), TRUE );
					return TRUE;
				case IDC_WINDOWEDRES:
					if (HIWORD(wParam) == CBN_SELCHANGE)
						EnableWindow( GetDlgItem( hWndDlg, IDC_APPLY ), TRUE );
					return TRUE;
				case IDC_FULLSCREENBITDEPTH:
					if (HIWORD(wParam) == CBN_SELCHANGE)
					{
						fullscreen.selected.bitDepth = fullscreen.bitDepth[SendDlgItemMessage( hWndDlg, IDC_FULLSCREENBITDEPTH, CB_GETCURSEL, 0, 0 )];

						UpdateFullscreenConfig( hWndDlg );
						EnableWindow( GetDlgItem( hWndDlg, IDC_APPLY ), TRUE );
					}
					break;
				case IDC_FULLSCREENRES:
					if (HIWORD(wParam) == CBN_SELCHANGE)
					{
						i = SendDlgItemMessage( hWndDlg, IDC_FULLSCREENRES, CB_GETCURSEL, 0, 0 );
						fullscreen.selected.width = fullscreen.resolution[i].width;
						fullscreen.selected.height = fullscreen.resolution[i].height;

						UpdateFullscreenConfig( hWndDlg );
						EnableWindow( GetDlgItem( hWndDlg, IDC_APPLY ), TRUE );
					}
					break;
				case IDC_FULLSCREENREFRESH:
					if (HIWORD(wParam) == CBN_SELCHANGE)
					{
						fullscreen.selected.refreshRate = fullscreen.refreshRate[SendDlgItemMessage( hWndDlg, IDC_FULLSCREENREFRESH, CB_GETCURSEL, 0, 0 )];

						UpdateFullscreenConfig( hWndDlg );
						EnableWindow( GetDlgItem( hWndDlg, IDC_APPLY ), TRUE );
					}
					break;
			} 
    } 
    return FALSE; 
} 

void Config_DoConfig()
{
	if (!hConfigDlg)
		DialogBox( hInstance, MAKEINTRESOURCE(IDD_CONFIGDLG), hWnd, (DLGPROC)ConfigDlgProc );
}
