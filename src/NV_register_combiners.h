#include <gl/gl.h>

struct CombinerInput
{
	GLenum input;
	GLenum mapping;
	GLenum usage;
};

struct CombinerVariable
{
	GLenum input;
	GLenum mapping;
	GLenum usage;
	BOOL used;
};

struct GeneralCombiner
{
	CombinerVariable A, B, C, D;

	struct
	{
		GLenum ab;
		GLenum cd;
		GLenum sum;
	} output;
};

struct RegisterCombiners
{
	GeneralCombiner color[8];
	GeneralCombiner alpha[8];

	struct
	{
		CombinerVariable A, B, C, D, E, F, G;
	} final;
};

static CombinerInput CombinerInputs[] =
{
	// CMB
	{ GL_SPARE0_NV,				GL_UNSIGNED_IDENTITY_NV,	GL_RGB },
	// T0
	{ GL_TEXTURE0_ARB,			GL_UNSIGNED_IDENTITY_NV,	GL_RGB },
	// T1
	{ GL_TEXTURE1_ARB,			GL_UNSIGNED_IDENTITY_NV,	GL_RGB },
	// PRIM
	{ GL_CONSTANT_COLOR0_NV,	GL_UNSIGNED_IDENTITY_NV,	GL_RGB },
	// SHADE
	{ GL_PRIMARY_COLOR_NV,		GL_UNSIGNED_IDENTITY_NV,	GL_RGB },
	// ENV
	{ GL_CONSTANT_COLOR1_NV,	GL_UNSIGNED_IDENTITY_NV,	GL_RGB },
	// CENTER
	{ GL_ZERO,					GL_UNSIGNED_IDENTITY_NV,	GL_RGB },
	// SCALE
	{ GL_ZERO,					GL_UNSIGNED_IDENTITY_NV,	GL_RGB },
	// CMBALPHA
	{ GL_SPARE0_NV,				GL_UNSIGNED_IDENTITY_NV,	GL_ALPHA },
	// T0ALPHA
	{ GL_TEXTURE0_ARB,			GL_UNSIGNED_IDENTITY_NV,	GL_ALPHA },
	// T1ALPHA
	{ GL_TEXTURE1_ARB,			GL_UNSIGNED_IDENTITY_NV,	GL_ALPHA },
	// PRIMALPHA
	{ GL_CONSTANT_COLOR0_NV,	GL_UNSIGNED_IDENTITY_NV,	GL_RGB },
	// SHADEALPHA
	{ GL_PRIMARY_COLOR_NV,		GL_UNSIGNED_IDENTITY_NV,	GL_ALPHA },
	// ENVALPHA
	{ GL_CONSTANT_COLOR1_NV,	GL_UNSIGNED_IDENTITY_NV,	GL_RGB },
	// LODFRAC
	{ GL_ZERO,					GL_UNSIGNED_IDENTITY_NV,	GL_RGB },
	// PRIMLODFRAC
	{ GL_ZERO,					GL_UNSIGNED_IDENTITY_NV,	GL_RGB },
	// NOISE
	{ GL_TEXTURE1_ARB,			GL_UNSIGNED_IDENTITY_NV,	GL_RGB },
	// K4
	{ GL_ZERO,					GL_UNSIGNED_IDENTITY_NV,	GL_RGB },
	// K5
	{ GL_ZERO,					GL_UNSIGNED_IDENTITY_NV,	GL_RGB },
	// ONE
	{ GL_ZERO,					GL_UNSIGNED_INVERT_NV,		GL_RGB },
	// ZERO
	{ GL_ZERO,					GL_UNSIGNED_IDENTITY_NV,	GL_RGB }
};

void Init_NV_register_combiners();
void Set_NV_register_combiners( Combiner *color, Combiner *alpha );
void Update_NV_register_combiners_Colors();