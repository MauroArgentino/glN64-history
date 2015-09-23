typedef struct
{
	GLenum source, operand;
} TexEnvCombinerArg;

typedef struct
{
	BOOL used;
	GLenum combine;
	TexEnvCombinerArg arg0, arg1, arg2;
} TexEnvCombinerStage;

typedef struct
{
	TexEnvCombinerStage color[8];
	TexEnvCombinerStage alpha[8];
} TexEnvCombiner;

static TexEnvCombinerArg TexEnvArgs[] =
{
	// CMB
	{ GL_PREVIOUS_ARB,		GL_SRC_COLOR },
	// T0
	{ GL_TEXTURE0_ARB,		GL_SRC_COLOR },
	// T1
	{ GL_TEXTURE1_ARB,		GL_SRC_COLOR },
	// PRIM
	{ GL_CONSTANT_ARB,		GL_SRC_COLOR },
	// SHADE
	{ GL_PRIMARY_COLOR_ARB,	GL_SRC_COLOR },
	// ENV
	{ GL_CONSTANT_ARB,		GL_SRC_COLOR },
	// CENTER
	{ GL_CONSTANT_ARB,		GL_SRC_COLOR },
	// SCALE
	{ GL_CONSTANT_ARB,		GL_SRC_COLOR },
	// CMBALPHA
	{ GL_PREVIOUS_ARB,		GL_SRC_ALPHA },
	// T0ALPHA
	{ GL_TEXTURE0_ARB,		GL_SRC_ALPHA },
	// T1ALPHA
	{ GL_TEXTURE1_ARB,		GL_SRC_ALPHA },
	// PRIMALPHA
	{ GL_CONSTANT_ARB,		GL_SRC_ALPHA },
	// SHADEALPHA
	{ GL_PRIMARY_COLOR_ARB,	GL_SRC_ALPHA },
	// ENVALPHA
	{ GL_CONSTANT_ARB,		GL_SRC_COLOR },
	// LODFRAC
	{ GL_CONSTANT_ARB,		GL_SRC_COLOR },
	// PRIMLODFRAC
	{ GL_CONSTANT_ARB,		GL_SRC_COLOR },
	// NOISE
	{ GL_TEXTURE1_ARB,		GL_SRC_COLOR },
	// K4
	{ GL_CONSTANT_ARB,		GL_SRC_COLOR },
	// K5
	{ GL_CONSTANT_ARB,		GL_SRC_COLOR },
	// ONE
	{ GL_CONSTANT_ARB,		GL_SRC_COLOR },
	// ZERO
	{ GL_CONSTANT_ARB,		GL_SRC_COLOR }
};

void Init_ARB_texture_env_combine();
void Set_ARB_texture_env_combine( Combiner *color, Combiner *alpha );
void Update_ARB_texture_env_combine_Colors();