#ifndef CONVERT_H
#define CONVERT_H

const unsigned char Five2Eight[32] = {
	  0, // 00000 = 00000000
	  8, // 00001 = 00001000
	 16, // 00010 = 00010000
	 25, // 00011 = 00011001
	 33, // 00100 = 00100001
	 41, // 00101 = 00101001
	 49, // 00110 = 00110001
	 58, // 00111 = 00111010
	 66, // 01000 = 01000010
	 74, // 01001 = 01001010
	 82, // 01010 = 01010010
	 90, // 01011 = 01011010
	 99, // 01100 = 01100011
	107, // 01101 = 01101011
	115, // 01110 = 01110011
	123, // 01111 = 01111011
	132, // 10000 = 10000100
	140, // 10001 = 10001100
	148, // 10010 = 10010100
	156, // 10011 = 10011100
	165, // 10100 = 10100101
	173, // 10101 = 10101101
	181, // 10110 = 10110101
	189, // 10111 = 10111101
	197, // 11000 = 11000101
	206, // 11001 = 11001110
	214, // 11010 = 11010110
	222, // 11011 = 11011110
	230, // 11100 = 11100110
	239, // 11101 = 11101111
	247, // 11110 = 11110111
	255  // 11111 = 11111111
};

const unsigned char Four2Eight[16] = {
	  0, // 0000 = 00000000
	 17, // 0001 = 00010001
	 34, // 0010 = 00100010
	 51, // 0011 = 00110011
	 68, // 0100 = 01000100
	 85, // 0101 = 01010101
	102, // 0110 = 01100110
	119, // 0111 = 01110111
	136, // 1000 = 10001000
	153, // 1001 = 10011001
	170, // 1010 = 10101010
	187, // 1011 = 10111011
	204, // 1100 = 11001100
	221, // 1101 = 11011101
	238, // 1110 = 11101110
	255  // 1111 = 11111111
};

const unsigned char Three2Eight[8] = {
	  0, // 000 = 00000000
     36, // 001 = 00100100
	 73, // 010 = 01001001
	109, // 011 = 01101101
	146, // 100 = 10010010
	182, // 101 = 10110110
    219, // 110 = 11011011
	255, // 111 = 11111111
};
const unsigned char Two2Eight[4] = {
	  0, // 00 = 00000000
	 85, // 01 = 01010101
	170, // 10 = 10101010
	255  // 11 = 11111111
};

const unsigned char One2Eight[2] = {
	  0, // 0 = 00000000
	255, // 1 = 11111111
};

// Un-swaps on the dword, works with non-dword aligned addresses
inline void UnswapCopy( void *src, void *dest, WORD numBytes )
{
	__asm
	{
		mov		ecx, 0
		mov		esi, dword ptr [src]
		mov		edi, dword ptr [dest]

		mov		ebx, esi
		and		ebx, 0x3			// ebx = number of leading bytes

		cmp		ebx, 0
 		jz		StartDWordLoop
		neg		ebx
		add		ebx, 4
		mov		ecx, ebx

		xor		esi, 3
LeadingLoop:				// Copies leading bytes, in reverse order (un-swaps)
		mov		al, byte ptr [esi]
		mov		byte ptr [edi], al
		sub		esi, 1
		add		edi, 1
		loop	LeadingLoop
		add		esi, 5

StartDWordLoop:
		mov		cx, [numBytes]
		sub		cx,	bx			// Don't copy what's already been copied

		add		cx, 3			// Round up to nearest dword
		shr		cx, 2

		cmp		cx, 0			// If there's nothing to do, don't do it
		jle		Done

		// Copies from source to destination, bswap-ing first
DWordLoop:
		mov		eax, dword ptr [esi]
		bswap	eax
		mov		dword ptr [edi], eax
		add		esi, 4
		add		edi, 4
		loop	DWordLoop
Done:
	}
}

inline void DWordInterleave( void *mem, WORD numDWords )
{
	__asm {
		mov		esi, dword ptr [mem]
		mov		edi, dword ptr [mem]
		add		edi, 4
		mov		ecx, 0
		mov		cx, word ptr [numDWords]
DWordInterleaveLoop:
		mov		eax, dword ptr [esi]
		mov		ebx, dword ptr [edi]
		mov		dword ptr [esi], ebx
		mov		dword ptr [edi], eax
		add		esi, 8
		add		edi, 8
		loop	DWordInterleaveLoop
	}
}

inline void QWordInterleave( void *mem, WORD numDWords )
{
	__asm
	{
	// Interleave the line on the qword
		mov		esi, dword ptr [mem]
		mov		edi, dword ptr [mem]
		add		edi, 8
		mov		ecx, 0
		mov		cx, word ptr [numDWords]
		shr		cx, 1
QWordInterleaveLoop:
		mov		eax, dword ptr [esi]
		mov		ebx, dword ptr [edi]
		mov		dword ptr [esi], ebx
		mov		dword ptr [edi], eax
		add		esi, 4
		add		edi, 4
		mov		eax, dword ptr [esi]
		mov		ebx, dword ptr [edi]
		mov		dword ptr [esi], ebx
		mov		dword ptr [edi], eax
		add		esi, 12
		add		edi, 12
		loop	QWordInterleaveLoop
	}
}

inline DWORD swapdword( DWORD value )
{
	__asm
	{
		mov		eax, dword ptr [value]
		bswap	eax
	}
}

inline WORD swapword( WORD value )
{
	__asm
	{
		mov		ax, word ptr [value]
		xchg	ah, al
	}
}

inline DWORD RGBA8888_ABGR8888( DWORD color )
{
	__asm
	{
		mov		eax, dword ptr[color]
		bswap	eax
	}
}

inline DWORD RGBA5551_ABGR8888( WORD color )
{
	__asm
	{
		mov		ebx, 00000000h
		mov		cx, word ptr [color]

		mov		bx, cx
		and		bx, 01h
        mov		al, byte ptr [One2Eight+ebx]

		mov		bx, cx
		shr		bx, 01h
		and		bx, 1Fh
		mov		ah, byte ptr [Five2Eight+ebx]

		bswap	eax

		mov		bx, cx
		shr		bx, 06h
		and		bx, 1Fh
		mov		ah, byte ptr [Five2Eight+ebx]

		mov		bx, cx
		shr		bx, 0Bh
		and		bx, 1Fh
		mov		al, byte ptr [Five2Eight+ebx]
	}
}

inline DWORD IA88_ABGR8888( WORD color )
{
	__asm
	{
		mov		cx, word ptr [color]

        mov		al, ch
		mov		ah, cl

		bswap	eax

		mov		ah, cl
		mov		al, cl
	}
}

// For low-endian values
inline DWORD AI88_ABGR8888( WORD color )
{
	__asm
	{
		mov		cx, word ptr [color]

        mov		al, cl
		mov		ah, ch

		bswap	eax

		mov		ah, ch
		mov		al, ch
	}
}

inline DWORD IA44_ABGR8888( BYTE color )
{
	__asm
	{
		mov		ebx, 00000000h
		mov		cl, byte ptr [color]

		mov		bl, cl
		shr		bl, 04h
		mov		ch, byte ptr [Four2Eight+ebx]

		mov		bl, cl
		and		bl, 0Fh
		mov		cl, byte ptr [Four2Eight+ebx]

        mov		al, cl
		mov		ah, ch

		bswap	eax

		mov		ah, ch
		mov		al, ch
	}
}

inline DWORD IA31_ABGR8888( BYTE color )
{
	__asm
	{
		mov		ebx, 00000000h
		mov		cl, byte ptr [color]

		mov		bl, cl
		shr		bl, 01h
		mov		ch, byte ptr [Three2Eight+ebx]

		mov		bl, cl
		and		bl, 01h
		mov		cl, byte ptr [One2Eight+ebx]

        mov		al, cl
		mov		ah, ch

		bswap	eax

		mov		ah, ch
		mov		al, ch
	}
}

inline DWORD I8_ABGR8888( BYTE color )
{
	__asm
	{
		mov		cl, byte ptr [color]

		mov		al, cl
		mov		ah, cl
		bswap	eax
		mov		ah, cl
		mov		al, cl
	}
}

inline DWORD I4_ABGR8888( BYTE color )
{
	__asm
	{
		mov		ebx, 00000000h

		mov		bl, byte ptr [color]
		mov		cl, byte ptr [Four2Eight+ebx]

        mov		al, cl
		mov		ah, cl
		bswap	eax
		mov		ah, cl
		mov		al, cl
	}
}
#endif