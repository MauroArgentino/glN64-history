#include <windows.h>

void CRC_BuildTable();

DWORD CRC_Calculate( DWORD crc, void *buffer, DWORD count );