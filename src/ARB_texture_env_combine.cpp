#include <windows.h>
#include "OpenGL.h"
#include "Combiner.h"
#include "ARB_texture_env_combine.h"
#include "RDP.h"

#define SetColorCombinerArg( n, a, i ) \
	envCombiner->color[n].a.source = TexEnvArgs[i].source; \
	envCombiner->color[n].a.operand = TexEnvArgs[i].operand

#define SetColorCombinerValues( n, a, s, o ) \
	envCombiner->color[n].a.source = s; \
	envCombiner->color[n].a.operand = o

#define SetAlphaCombinerArg( n, a, i ) \
	envCombiner->alpha[n].a.source = TexEnvArgs[i].source; \
	envCombiner->alpha[n].a.operand = GL_SRC_ALPHA

#define SetAlphaCombinerValues( n, a, s, o ) \
	envCombiner->alpha[n].a.source = s; \
	envCombiner->alpha[n].a.operand = o

void Init_ARB_texture_env_combine()
{
	DWORD tex[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	for (int i = 0; i < OGL.maxTextureUnits; i++)
	{
		glActiveTextureARB( GL_TEXTURE0_ARB + i );
		glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB );
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		glEnable( GL_TEXTURE_2D );
	}
}

void Uninit_ARB_texture_env_combine()
{
	for (int i = 0; i < OGL.maxTextureUnits; i++)
	{
		glActiveTextureARB( GL_TEXTURE0_ARB + i );
		glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
	}
}

void Update_ARB_texture_env_combine_Colors( TexEnvCombiner *envCombiner )
{
	GLcolor color;

	for (int i = 0; i < OGL.maxTextureUnits; i++)
	{
		SetConstant( color, envCombiner->color[i].constant, envCombiner->alpha[i].constant );

		glActiveTextureARB( GL_TEXTURE0_ARB + i );
		glTexEnvfv( GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, (GLfloat*)&color );
	}
}

TexEnvCombiner *Generate_ARB_texture_env_combine( Combiner *color, Combiner *alpha )
{
	TexEnvCombiner *envCombiner = (TexEnvCombiner*)malloc( sizeof( TexEnvCombiner ) );

	int curUnit, usedUnits;

	for (int i = 0; i < OGL.maxTextureUnits; i++)
	{
		envCombiner->color[i].combine = GL_REPLACE;
		envCombiner->alpha[i].combine = GL_REPLACE;

		SetColorCombinerValues( i, arg0, GL_PREVIOUS_ARB, GL_SRC_COLOR );
		SetColorCombinerValues( i, arg1, GL_PREVIOUS_ARB, GL_SRC_COLOR );
		SetColorCombinerValues( i, arg2, GL_PREVIOUS_ARB, GL_SRC_COLOR );
		envCombiner->color[i].constant = COMBINED;

		SetAlphaCombinerValues( i, arg0, GL_PREVIOUS_ARB, GL_SRC_ALPHA );
		SetAlphaCombinerValues( i, arg1, GL_PREVIOUS_ARB, GL_SRC_ALPHA );
		SetAlphaCombinerValues( i, arg2, GL_PREVIOUS_ARB, GL_SRC_ALPHA );
		envCombiner->alpha[i].constant = CMBALPHA;
	}

	envCombiner->usesT0 = FALSE;
	envCombiner->usesT1 = FALSE;

	envCombiner->vertex.color = COMBINED;
	envCombiner->vertex.alpha = CMBALPHA;

	curUnit = 0;
	for (int i = 0; i < alpha->numStages; i++)
	{
		// ARB_texture_env_combiner can't use combined unless it's from the previous unit
		if ((i > 0) && (alpha->stage[i].op[0].param1 != COMBINED) && (alpha->stage[i].op[0].op != INTER))
			break;

		for (int j = 0; j < alpha->stage[i].numOps; j++)
		{
			switch (alpha->stage[i].op[j].op)
			{
				case LOAD:
					envCombiner->usesT0 |= (alpha->stage[i].op[j].param1 == T0ALPHA);
					envCombiner->usesT1 |= (alpha->stage[i].op[j].param1 == T1ALPHA);

					if (TexEnvArgs[alpha->stage[i].op[j].param1].source == GL_CONSTANT_ARB)
						envCombiner->alpha[curUnit].constant = alpha->stage[i].op[j].param1;

					SetAlphaCombinerArg( curUnit, arg0, alpha->stage[i].op[j].param1 );

					if ((TexEnvArgs[alpha->stage[i].op[j].param1].source == GL_CONSTANT_ARB) &&
						(alpha->stage[i].numOps > (j + 1)) &&
						(TexEnvArgs[alpha->stage[i].op[j + 1].param1].source == GL_CONSTANT_ARB))
						curUnit++;
//					j = max( alpha->stage[i].numOps - 1 - OGL.maxTextureUnits, 0);
					break;
				case SUB:
					envCombiner->usesT0 |= (alpha->stage[i].op[j].param1 == T0ALPHA);
					envCombiner->usesT1 |= (alpha->stage[i].op[j].param1 == T1ALPHA);

					// (1 - x) can use GL_ONE_MINUS_SRC_ALPHA
					if ((curUnit == 0) && (envCombiner->alpha[curUnit].arg0.source == GL_ONE))
					{
						if (TexEnvArgs[alpha->stage[i].op[j].param1].source == GL_CONSTANT_ARB)
							envCombiner->alpha[curUnit].constant = alpha->stage[i].op[j].param1;

						SetAlphaCombinerValues( curUnit, arg0, TexEnvArgs[alpha->stage[i].op[j].param1].source, GL_ONE_MINUS_SRC_ALPHA );
					}
					else
					{
						if (TexEnvArgs[alpha->stage[i].op[j].param1].source == GL_CONSTANT_ARB)
							envCombiner->alpha[curUnit].constant = alpha->stage[i].op[j].param1;

						envCombiner->alpha[curUnit].combine = GL_SUBTRACT_ARB;
						SetAlphaCombinerArg( curUnit, arg1, alpha->stage[i].op[j].param1 );
						curUnit++;
					}
					break;
				case MUL:
					envCombiner->usesT0 |= (alpha->stage[i].op[j].param1 == T0ALPHA);
					envCombiner->usesT1 |= (alpha->stage[i].op[j].param1 == T1ALPHA);

					if (TexEnvArgs[alpha->stage[i].op[j].param1].source == GL_CONSTANT_ARB)
						envCombiner->alpha[curUnit].constant = alpha->stage[i].op[j].param1;

					envCombiner->alpha[curUnit].combine = GL_MODULATE;
					SetAlphaCombinerArg( curUnit, arg1, alpha->stage[i].op[j].param1 );
					curUnit++;
					break;
				case ADD:
					envCombiner->usesT0 |= (alpha->stage[i].op[j].param1 == T0ALPHA);
					envCombiner->usesT1 |= (alpha->stage[i].op[j].param1 == T1ALPHA);

					if (TexEnvArgs[alpha->stage[i].op[j].param1].source == GL_CONSTANT_ARB)
						envCombiner->alpha[curUnit].constant = alpha->stage[i].op[j].param1;

					envCombiner->alpha[curUnit].combine = GL_ADD;
					SetAlphaCombinerArg( curUnit, arg1, alpha->stage[i].op[j].param1 );
					curUnit++;
					break;
				case INTER:
					envCombiner->usesT0 |= ((alpha->stage[i].op[j].param1 == T0ALPHA) || (alpha->stage[i].op[j].param2 == T0ALPHA) || (alpha->stage[i].op[j].param3 == T0ALPHA));
					envCombiner->usesT1 |= ((alpha->stage[i].op[j].param1 == T1ALPHA) || (alpha->stage[i].op[j].param2 == T1ALPHA) || (alpha->stage[i].op[j].param3 == T1ALPHA));

					if (TexEnvArgs[alpha->stage[i].op[j].param1].source == GL_CONSTANT_ARB)
						envCombiner->alpha[curUnit].constant = alpha->stage[i].op[j].param1;

					if (TexEnvArgs[alpha->stage[i].op[j].param2].source == GL_CONSTANT_ARB)
					{
						if (envCombiner->alpha[curUnit].constant)
						{
							envCombiner->vertex.alpha = alpha->stage[i].op[j].param2;
							alpha->stage[i].op[j].param2 = SHADE;
						}
						else
							envCombiner->alpha[curUnit].constant = alpha->stage[i].op[j].param2;
					}

					if (TexEnvArgs[alpha->stage[i].op[j].param3].source == GL_CONSTANT_ARB)
						envCombiner->alpha[curUnit].constant = alpha->stage[i].op[j].param3;

					envCombiner->alpha[curUnit].combine = GL_INTERPOLATE_ARB;
					SetAlphaCombinerArg( curUnit, arg0, alpha->stage[i].op[j].param1 );
					SetAlphaCombinerArg( curUnit, arg1, alpha->stage[i].op[j].param2 );
					SetAlphaCombinerArg( curUnit, arg2, alpha->stage[i].op[j].param3 );
					curUnit++;
					break;
			}
		}
	}

	usedUnits = curUnit;

	curUnit = 0;
	for (int i = 0; i < color->numStages; i++)
	{
		// ARB_texture_env_combiner can't use combined unless it's from the previous unit
		if ((i > 0) && (color->stage[i].op[0].param1 != COMBINED) && (color->stage[i].op[0].op != INTER))
			break;

		for (int j = 0; j < color->stage[i].numOps; j++)
		{
			switch (color->stage[i].op[j].op)
			{
				case LOAD:
					envCombiner->usesT0 |= ((color->stage[i].op[j].param1 == TEXTURE0) || (color->stage[i].op[j].param1 == T0ALPHA));
					envCombiner->usesT1 |= ((color->stage[i].op[j].param1 == TEXTURE1) || (color->stage[i].op[j].param1 == T1ALPHA));

					if (TexEnvArgs[color->stage[i].op[j].param1].source == GL_CONSTANT_ARB)
					{
						if ((color->stage[i].op[j].param1 == ONE) &&
							(color->stage[i].numOps > (j + 1)) &&
							(color->stage[i].op[j + 1].op == SUB))
						{
							j++;
							SetColorCombinerValues( curUnit, arg0, TexEnvArgs[color->stage[i].op[j].param1].source, GL_ONE_MINUS_SRC_COLOR );
							break;
						}

						envCombiner->color[curUnit].constant = color->stage[i].op[j].param1;
					}

					SetColorCombinerArg( curUnit, arg0, color->stage[i].op[j].param1 );

					if (((TexEnvArgs[color->stage[i].op[j].param1].source == GL_CONSTANT_ARB) &&
						(color->stage[i].numOps > (j + 1)) &&
						(TexEnvArgs[color->stage[i].op[j + 1].param1].source == GL_CONSTANT_ARB)) ||
						(color->stage[i].numOps == 1))
						curUnit++;

//					j = max( color->stage[i].numOps + color->stage[i + 1].numOps * (color->numStages - 1) - 1 - OGL.maxTextureUnits, 0);
					break;
				case SUB:
					envCombiner->usesT0 |= ((color->stage[i].op[j].param1 == TEXTURE0) || (color->stage[i].op[j].param1 == T0ALPHA));
					envCombiner->usesT1 |= ((color->stage[i].op[j].param1 == TEXTURE1) || (color->stage[i].op[j].param1 == T1ALPHA));

					// (1 - x) can use GL_ONE_MINUS_SRC_ALPHA
					if ((curUnit == 0) && (envCombiner->color[curUnit].arg0.source == GL_ONE))
					{
						if (TexEnvArgs[color->stage[i].op[j].param1].source == GL_CONSTANT_ARB)
							envCombiner->color[curUnit].constant = color->stage[i].op[j].param1;

						SetColorCombinerValues( curUnit, arg0, TexEnvArgs[color->stage[i].op[j].param1].source, GL_ONE_MINUS_SRC_COLOR );
					}
					else
					{
						if (TexEnvArgs[color->stage[i].op[j].param1].source == GL_CONSTANT_ARB)
							envCombiner->color[curUnit].constant = color->stage[i].op[j].param1;

						envCombiner->color[curUnit].combine = GL_SUBTRACT_ARB;
						SetColorCombinerArg( curUnit, arg1, color->stage[i].op[j].param1 );
						curUnit++;
					}
					break;
				case MUL:
					envCombiner->usesT0 |= ((color->stage[i].op[j].param1 == TEXTURE0) || (color->stage[i].op[j].param1 == T0ALPHA));
					envCombiner->usesT1 |= ((color->stage[i].op[j].param1 == TEXTURE1) || (color->stage[i].op[j].param1 == T1ALPHA));

					if (TexEnvArgs[color->stage[i].op[j].param1].source == GL_CONSTANT_ARB)
						envCombiner->color[curUnit].constant = color->stage[i].op[j].param1;

					envCombiner->color[curUnit].combine = GL_MODULATE;
					SetColorCombinerArg( curUnit, arg1, color->stage[i].op[j].param1 );
					curUnit++;
					break;
				case ADD:
					envCombiner->usesT0 |= ((color->stage[i].op[j].param1 == TEXTURE0) || (color->stage[i].op[j].param1 == T0ALPHA));
					envCombiner->usesT1 |= ((color->stage[i].op[j].param1 == TEXTURE1) || (color->stage[i].op[j].param1 == T1ALPHA));

					if (TexEnvArgs[color->stage[i].op[j].param1].source == GL_CONSTANT_ARB)
						envCombiner->color[curUnit].constant = color->stage[i].op[j].param1;

					envCombiner->color[curUnit].combine = GL_ADD;
					SetColorCombinerArg( curUnit, arg1, color->stage[i].op[j].param1 );
					curUnit++;
					break;

				case INTER:
					envCombiner->usesT0 |= ((color->stage[i].op[j].param1 == TEXTURE0) || (color->stage[i].op[j].param2 == TEXTURE0) || (color->stage[i].op[j].param3 == TEXTURE0) || (color->stage[i].op[j].param1 == T0ALPHA) || (color->stage[i].op[j].param2 == T0ALPHA) || (color->stage[i].op[j].param3 == T0ALPHA));
					envCombiner->usesT1 |= ((color->stage[i].op[j].param1 == TEXTURE1) || (color->stage[i].op[j].param2 == TEXTURE1) || (color->stage[i].op[j].param3 == TEXTURE1) || (color->stage[i].op[j].param1 == T1ALPHA) || (color->stage[i].op[j].param2 == T1ALPHA) || (color->stage[i].op[j].param3 == T1ALPHA));

					if (TexEnvArgs[color->stage[i].op[j].param1].source == GL_CONSTANT_ARB)
						envCombiner->color[curUnit].constant = color->stage[i].op[j].param1;

					if (TexEnvArgs[color->stage[i].op[j].param2].source == GL_CONSTANT_ARB)
					{
						if (envCombiner->color[curUnit].constant)
						{
							envCombiner->vertex.color = color->stage[i].op[j].param2;
							color->stage[i].op[j].param2 = SHADE;
						}
						else
							envCombiner->color[curUnit].constant = color->stage[i].op[j].param2;
					}

					if (TexEnvArgs[color->stage[i].op[j].param3].source == GL_CONSTANT_ARB)
					{
						if (envCombiner->color[curUnit].constant)
						{
							envCombiner->vertex.color = color->stage[i].op[j].param3;
							color->stage[i].op[j].param3 = SHADE;
						}
						else
							envCombiner->color[curUnit].constant = color->stage[i].op[j].param3;
					}

					envCombiner->color[curUnit].combine = GL_INTERPOLATE_ARB;
					SetColorCombinerArg( curUnit, arg0, color->stage[i].op[j].param1 );
					SetColorCombinerArg( curUnit, arg1, color->stage[i].op[j].param2 );
					SetColorCombinerArg( curUnit, arg2, color->stage[i].op[j].param3 );
					curUnit++;
					break;
			}
		}
	}

	envCombiner->usedUnits = max( curUnit, usedUnits );

	return envCombiner;
}


void Set_ARB_texture_env_combine( TexEnvCombiner *envCombiner )
{
	RDP.useT0 = combiner.usesT0 = envCombiner->usesT0;
	RDP.useT1 = combiner.usesT1 = envCombiner->usesT1;
	RDP.useNoise = FALSE;
	combiner.usesNoise = FALSE;

	combiner.vertex.color = envCombiner->vertex.color;
	combiner.vertex.alpha = envCombiner->vertex.alpha;

	for (int i = 0; i < OGL.maxTextureUnits; i++)
	{
		glActiveTextureARB( GL_TEXTURE0_ARB + i );

		if ((i < envCombiner->usedUnits ) || ((i < 2) && envCombiner->usesT1))
		{
			glEnable( GL_TEXTURE_2D );

			glTexEnvi( GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, envCombiner->color[i].combine );

			glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB,  envCombiner->color[i].arg0.source );
			glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, envCombiner->color[i].arg0.operand );
			glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB,  envCombiner->color[i].arg1.source );
			glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND1_RGB_ARB, envCombiner->color[i].arg1.operand );
			glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE2_RGB_ARB,  envCombiner->color[i].arg2.source );
			glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND2_RGB_ARB, envCombiner->color[i].arg2.operand );

			glTexEnvi( GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB, envCombiner->alpha[i].combine );

			glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_ARB,  envCombiner->alpha[i].arg0.source );
			glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_ARB, envCombiner->alpha[i].arg0.operand );
			glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_ARB,  envCombiner->alpha[i].arg1.source );
			glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND1_ALPHA_ARB, envCombiner->alpha[i].arg1.operand );
			glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE2_ALPHA_ARB,  envCombiner->alpha[i].arg2.source );
			glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND2_ALPHA_ARB, envCombiner->alpha[i].arg2.operand );
		}
		else
		{
			glDisable( GL_TEXTURE_2D );
		}			
	}
}
