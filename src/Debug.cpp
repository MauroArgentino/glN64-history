#include <windows.h>
#include <stdio.h>
#include <process.h>
#include "glNintendo64().h"
#include "Debug.h"
#include "resource.h"
#include "RDP.h"
#include "RSP.h"
#include "Textures.h"

HWND hDebugDlg;
BOOL DumpMessages;
FILE *dumpFile;
char dumpFilename[256];

BOOL CALLBACK DebugDlgProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    switch (uMsg) 
    { 
        case WM_COMMAND: 
            switch (LOWORD(wParam)) 
            { 
                case IDC_DUMP: 
					RSP.dumpNextDL = TRUE;
                    return TRUE; 
				case IDC_VERIFYCACHE:
					if (!TextureCache_Verify())
						MessageBox( NULL, "Texture cache chain is currupted!", "glNintendo64()", MB_OK );
					else
						MessageBox( NULL, "Texture cache chain verified", "glNintendo64()", MB_OK );
					return TRUE;
            }
			break;
    } 

	return FALSE;
}

void DebugDlgThreadFunc( void* )
{
	hDebugDlg = CreateDialog( hInstance, MAKEINTRESOURCE(IDD_DEBUGDLG), hWnd, DebugDlgProc );

	MSG msg;

	while (GetMessage( &msg, hDebugDlg, 0, 0 ) != 0)
	{ 
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void OpenDebugDlg()
{
	DumpMessages = FALSE;
/*	hDebugDlg = CreateDialog( hInstance, MAKEINTRESOURCE(IDD_DEBUGDLG), NULL, DebugDlgProc );

	MSG msg;

	while (GetMessage( &msg, NULL, 0, 0)
	{
		if (IsWindow(hwndGoto)
			IsDialogMessage( hwndGoto, &msg );
	}
	_endthread();*/
	_beginthread( DebugDlgThreadFunc, 255, NULL );
}

void CloseDebugDlg()
{
	DestroyWindow( hDebugDlg );
}

void StartDump( char *filename )
{
	DumpMessages = TRUE;
	strcpy( dumpFilename, filename );
	dumpFile = fopen( filename, "w" );
	fclose( dumpFile );
}

void EndDump()
{
	DumpMessages = FALSE;
	fclose( dumpFile );
}

void DebugMsg( BYTE type, LPCSTR format, ... )
{
	char text[256];

	va_list va;
 	va_start( va, format );

	if (DumpMessages)
	{
		dumpFile = fopen( dumpFilename, "a+" ); 
		vfprintf( dumpFile, format, va );
		fclose( dumpFile );
	}

	if ((type & DEBUG_COMBINE) && (type & DEBUG_UNHANDLED)) //(DEBUG_UNKNOWN | DEBUG_ERROR))
	{
		vsprintf( text, format, va );
		SendDlgItemMessage( hDebugDlg, IDC_DEBUGEDIT, EM_REPLACESEL, FALSE, (LPARAM) text );
	}
	va_end( va );
}
