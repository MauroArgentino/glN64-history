#include <windows.h>
#include "N64.h"

BYTE *DMEM;
BYTE *IMEM;
BYTE *RDRAM;

N64Regs REG;

GFXFunc GFXOp[256];