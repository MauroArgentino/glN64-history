#ifndef GLNINTENDO64_H
#define GLNINTENDO64_H
//#define DEBUG
#include <commctrl.h>

#include <windows.h>

extern HWND			hWnd;
extern HWND			hFullscreen;
extern HWND			hStatusBar;
extern HINSTANCE	hInstance;

extern char			pluginName[];

extern void (*CheckInterrupts)( void );

#endif