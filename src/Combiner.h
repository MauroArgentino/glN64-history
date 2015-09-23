#ifndef COMBINER_H
#define COMBINER_H

#include "glNintendo64().h"
#include "OpenGL.h"

#define NV_REGISTER_COMBINERS	0
#define ARB_TEXTURE_ENV_COMBINE 1

#define LOAD		0
#define SUB			1
#define MUL			2
#define ADD			3
#define INTER		4

#define COMBINED	0
#define TEXTURE0	1
#define TEXTURE1	2
#define PRIMITIVE	3
#define SHADE		4
#define ENVIRONMENT	5
#define CENTER		6
#define SCALE		7
#define CMBALPHA	8
#define T0ALPHA		9
#define T1ALPHA		10
#define PRIMALPHA	11
#define SHADEALPHA	12
#define ENVALPHA	13
#define LODFRACTION	14
#define PRIMLODFRAC	15
#define NOISE		16
#define K4			17
#define K5			18
#define ONE			19
#define ZERO		20

const bool Precombinable[21] = 
{
	true,	// COMBINED		0
	false,	// TEXTURE0		1
	false,  // TEXTURE1		2
	true,	// PRIMITIVE	3
	true,	// SHADE		4
	true,	// ENVIRONMENT	5
	true,	// CENTER		6
	true,	// SCALE		7
	true,	// CMBALPHA		8
	false,	// T0ALPHA		9
	false,	// T1ALPHA		10
	true,	// PRIMALPHA	11
	true,	// SHADEALPHA	12
	true,	// ENVALPHA		13
	true,	// LODFRACTION	14
	true,	// PRIMLODFRAC	15
	false,	// NOISE		16
	true,	// K4			17
	true,	// K5			18
	true,	// ONE			19
	true	// ZERO			20
};

struct CombinerOp
{
	int op;
	int param1;
	int param2;
	int param3;
};

struct CombinerStage
{
	int numOps;
	CombinerOp op[6];
};

struct Combiner
{
	int numStages;
	CombinerStage stage[2];
};

struct CombinerCycle
{
	int A, B, C, D;
};

static int CCExpandA[] = 
{
	COMBINED,		TEXTURE0,		TEXTURE1,		PRIMITIVE,		
	SHADE,			ENVIRONMENT,	ONE,			NOISE,
	ZERO,			ZERO,			ZERO,			ZERO,
	ZERO,			ZERO,			ZERO,			ZERO
};

static int CCExpandB[] = 
{
	COMBINED,		TEXTURE0,		TEXTURE1,		PRIMITIVE,		
	SHADE,			ENVIRONMENT,	CENTER,			K4,
	ZERO,			ZERO,			ZERO,			ZERO,
	ZERO,			ZERO,			ZERO,			ZERO
};

static int CCExpandC[] = 
{
	COMBINED,		TEXTURE0,		TEXTURE1,		PRIMITIVE,		
	SHADE,			ENVIRONMENT,	SCALE,			CMBALPHA,
	T0ALPHA,		T1ALPHA,		PRIMALPHA,		SHADEALPHA,
	ENVALPHA,		LODFRACTION,	PRIMLODFRAC,	K5,
	ZERO,			ZERO,			ZERO,			ZERO,
	ZERO,			ZERO,			ZERO,			ZERO,
	ZERO,			ZERO,			ZERO,			ZERO,
	ZERO,			ZERO,			ZERO,			ZERO
};

static int CCExpandD[] = 
{
	COMBINED,		TEXTURE0,		TEXTURE1,		PRIMITIVE,		
	SHADE,			ENVIRONMENT,		ONE,		ZERO
};

static int ACExpandA[] = 
{
	COMBINED,		T0ALPHA,		T1ALPHA,		PRIMALPHA,		
	SHADEALPHA,		ENVALPHA,		ONE,			ZERO
};

static int ACExpandB[] = 
{
	COMBINED,		T0ALPHA,		T1ALPHA,		PRIMALPHA,		
	SHADEALPHA,		ENVALPHA,		ONE,			ZERO
};

static int ACExpandC[] = 
{
	LODFRACTION,	T0ALPHA,		T1ALPHA,		PRIMALPHA,		
	SHADEALPHA,		ENVALPHA,		PRIMLODFRAC,	ZERO,
};

static int ACExpandD[] = 
{
	COMBINED,		T0ALPHA,		T1ALPHA,		PRIMALPHA,		
	SHADEALPHA,		ENVALPHA,		ONE,			ZERO
};

#ifdef DEBUG
static char *CCSubtractA[] =
{
	"Combined",			"Texel0",			"Texel1",			"Primitive",
	"Shade",			"Environment",		"1",				"Noise",
	"0",				"0",				"0",				"0",
	"0",				"0",				"0",				"0"
};

static char *CCSubtractB[] =
{
	"Combined",			"Texel0",			"Texel1",			"Primitive",
	"Shade",			"Environment",		"Center",			"K4",
	"0",				"0",				"0",				"0",
	"0",				"0",				"0",				"0"
};

static char *CCMultiply[] =
{
	"Combined",			"Texel0",			"Texel1",			"Primitive",
	"Shade",			"Environment",		"Scale",			"Combined Alpha",
	"T0 Alpha",			"T1_Alpha",			"Primitive Alpha",	"Shade Alpha",
	"Environment Alpha","LOD Fraction",		"Prim LOD Fraction","K5",
	"0",				"0",				"0",				"0",
	"0",				"0",				"0",				"0",
	"0",				"0",				"0",				"0",
	"0",				"0",				"0",				"0"
};

static char *CCAdd[] =
{
	"Combined",			"Texel0",			"Texel1",			"Primitive",
	"Shade",			"Environment",		"1",				"0"
};

static char *ACSubtractA[] =
{
	"Combined",			"Texel0",			"Texel1",			"Primitive",
	"Shade",			"Environment",		"1",				"0"
};

static char *ACSubtractB[] =
{
	"Combined",			"Texel0",			"Texel1",			"Primitive",
	"Shade",			"Environment",		"1",				"0"
};

static char *ACMultiply[] =
{ 
	"LOD Fraction",		"Texel0",			"Texel1",			"Primitive",
	"Shade",			"Environment",		"Prim LOD Fraction","0"
};

static char *ACAdd[] =
{
	"Combined",			"Texel0",			"Texel1",			"Primitive",
	"Shade",			"Environment",		"1",				"0"
};
#endif

extern struct CombinerInfo
{
	struct
	{
		WORD color, alpha;
	} vertex;

	int combiner;
} combiner;

#define SetConstant( constant, color, alpha ) \
	switch (color) \
	{ \
		case PRIMITIVE: \
			constant.r = RDP.primColor.r; \
			constant.g = RDP.primColor.g; \
			constant.b = RDP.primColor.b; \
			break; \
		case ENVIRONMENT: \
			constant.r = RDP.envColor.r; \
			constant.g = RDP.envColor.g; \
			constant.b = RDP.envColor.b; \
			break; \
		case PRIMALPHA: \
			constant.r = RDP.primColor.a; \
			constant.g = RDP.primColor.a; \
			constant.b = RDP.primColor.a; \
			break; \
		case ENVALPHA: \
			constant.r = RDP.envColor.a; \
			constant.g = RDP.envColor.a; \
			constant.b = RDP.envColor.a; \
			break; \
		case PRIMLODFRAC: \
			constant.r = RDP.primLODFrac; \
			constant.g = RDP.primLODFrac; \
			constant.b = RDP.primLODFrac; \
			break; \
		case ONE: \
			constant.r = 1.0f; \
			constant.g = 1.0f; \
			constant.b = 1.0f; \
			break; \
		case ZERO: \
			constant.r = 0.0f; \
			constant.g = 0.0f; \
			constant.b = 0.0f; \
			break; \
	} \
\
	switch (alpha) \
	{ \
		case PRIMALPHA: \
			constant.a = RDP.primColor.a; \
			break; \
		case ENVALPHA: \
			constant.a = RDP.envColor.a; \
			break; \
		case PRIMLODFRAC: \
			constant.a = RDP.primLODFrac; \
			break; \
		case ONE: \
			constant.a = 1.0f; \
			break; \
		case ZERO: \
			constant.a = 0.0f; \
			break; \
	}

void Combiner_Init();
void Combiner_UpdateCombineColors();
void Combiner_UpdateCombineMode();
#endif