#include <windows.h>
#include <stdio.h>

#define		DEBUG_HANDLED	0x0000
#define		DEBUG_UNKNOWN	0x0001
#define		DEBUG_UNHANDLED 0x0002
#define		DEBUG_IGNORED	0x0004
#define		DEBUG_COMBINE	0x0008
#define		DEBUG_ERROR		0x0010

void OpenDebugDlg();
void CloseDebugDlg();
void DebugMsg( BYTE type, LPCSTR format, ... );
void StartDump( char *filename );
void EndDump();