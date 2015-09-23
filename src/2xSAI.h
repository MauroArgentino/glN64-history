#ifndef _2XSAI_H
#define _2XSAI_H

void Super2xSaI( DWORD *srcPtr, DWORD *destPtr, WORD width, WORD height );
void _2xSaI( DWORD *srcPtr, DWORD *destPtr, WORD width, WORD height, BOOL clampS, BOOL clampT );
#endif