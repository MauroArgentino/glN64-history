#include <windows.h>
#include <gl/gl.h>
#include "Combiner.h"
#include "ARB_texture_env_combine.h"
#include "RDP.h"

int colorConstants[8], alphaConstants[8];

#define SetColorCombinerArg( n, a, i ) \
	envCombiner.color[n].a.source = TexEnvArgs[i].source; \
	envCombiner.color[n].a.operand = TexEnvArgs[i].operand

#define SetColorCombinerValues( n, a, s, o ) \
	envCombiner.color[n].a.source = s; \
	envCombiner.color[n].a.operand = o

#define SetAlphaCombinerArg( n, a, i ) \
	envCombiner.alpha[n].a.source = TexEnvArgs[i].source; \
	envCombiner.alpha[n].a.operand = GL_SRC_ALPHA

#define SetAlphaCombinerValues( n, a, s ) \
	envCombiner.alpha[n].a.source = s; \
	envCombiner.alpha[n].a.operand = GL_SRC_ALPHA

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

void Update_ARB_texture_env_combine_Colors()
{
	GLcolor color;

	for (int i = 0; i < OGL.maxTextureUnits; i++)
	{
		SetConstant( color, colorConstants[i], alphaConstants[i] );

		glActiveTextureARB( GL_TEXTURE0_ARB + i );
		glTexEnvfv( GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, (GLfloat*)&color );
	}
}

void Set_ARB_texture_env_combine( Combiner *color, Combiner *alpha )
{
	TexEnvCombiner envCombiner;
	int curUnit;

	RDP.useT0 = TRUE;
	RDP.useT1 = TRUE;

	for (int i = 0; i < OGL.maxTextureUnits; i++)
	{
		envCombiner.color[i].combine = GL_REPLACE;
		envCombiner.alpha[i].combine = GL_REPLACE;

		SetColorCombinerValues( i, arg0, GL_PREVIOUS_ARB, GL_SRC_COLOR );
		SetColorCombinerValues( i, arg1, GL_PREVIOUS_ARB, GL_SRC_COLOR );
		SetColorCombinerValues( i, arg2, GL_PREVIOUS_ARB, GL_SRC_COLOR );
		colorConstants[i] = COMBINED;

		SetAlphaCombinerValues( i, arg0, GL_PREVIOUS_ARB );
		SetAlphaCombinerValues( i, arg1, GL_PREVIOUS_ARB );
		SetAlphaCombinerValues( i, arg2, GL_PREVIOUS_ARB );
		alphaConstants[i] = COMBINED;
	}

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
					if (TexEnvArgs[alpha->stage[i].op[j].param1].source == GL_CONSTANT_ARB)
						alphaConstants[curUnit] = alpha->stage[i].op[j].param1;

					SetAlphaCombinerArg( curUnit, arg0, alpha->stage[i].op[j].param1 );

					if ((TexEnvArgs[alpha->stage[i].op[j].param1].source == GL_CONSTANT_ARB) &&
						(alpha->stage[i].numOps > (j + 1)) &&
						(TexEnvArgs[alpha->stage[i].op[j + 1].param1].source == GL_CONSTANT_ARB))
						curUnit++;
//					j = max( alpha->stage[i].numOps - 1 - OGL.maxTextureUnits, 0);
					break;
				case SUB:
					if (TexEnvArgs[alpha->stage[i].op[j].param1].source == GL_CONSTANT_ARB)
						alphaConstants[curUnit] = alpha->stage[i].op[j].param1;

					envCombiner.alpha[curUnit].combine = GL_SUBTRACT_ARB;
					SetAlphaCombinerArg( curUnit, arg1, alpha->stage[i].op[j].param1 );
					curUnit++;
					break;
				case MUL:
					if (TexEnvArgs[alpha->stage[i].op[j].param1].source == GL_CONSTANT_ARB)
						alphaConstants[curUnit] = alpha->stage[i].op[j].param1;

					envCombiner.alpha[curUnit].combine = GL_MODULATE;
					SetAlphaCombinerArg( curUnit, arg1, alpha->stage[i].op[j].param1 );
					curUnit++;
					break;
				case ADD:
					if (TexEnvArgs[alpha->stage[i].op[j].param1].source == GL_CONSTANT_ARB)
						alphaConstants[curUnit] = alpha->stage[i].op[j].param1;

					envCombiner.alpha[curUnit].combine = GL_ADD;
					SetAlphaCombinerArg( curUnit, arg1, alpha->stage[i].op[j].param1 );
					curUnit++;
					break;
				case INTER:
					if (TexEnvArgs[alpha->stage[i].op[j].param1].source == GL_CONSTANT_ARB)
						alphaConstants[curUnit] = alpha->stage[i].op[j].param1;

					if (TexEnvArgs[alpha->stage[i].op[j].param2].source == GL_CONSTANT_ARB)
					{
						if (alphaConstants[curUnit])
						{
							combiner.vertex.alpha = alpha->stage[i].op[j].param2;
							alpha->stage[i].op[j].param2 = SHADE;
						}
						else
							alphaConstants[curUnit] = alpha->stage[i].op[j].param2;
					}

					if (TexEnvArgs[alpha->stage[i].op[j].param3].source == GL_CONSTANT_ARB)
						alphaConstants[curUnit] = alpha->stage[i].op[j].param3;

					envCombiner.alpha[curUnit].combine = GL_INTERPOLATE_ARB;
					SetAlphaCombinerArg( curUnit, arg0, alpha->stage[i].op[j].param1 );
					SetAlphaCombinerArg( curUnit, arg1, alpha->stage[i].op[j].param2 );
					SetAlphaCombinerArg( curUnit, arg2, alpha->stage[i].op[j].param3 );
					curUnit++;
					break;
			}
			if (curUnit == OGL.maxTextureUnits)
				curUnit--;
		}
	}

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

						colorConstants[curUnit] = color->stage[i].op[j].param1;
					}

					SetColorCombinerArg( curUnit, arg0, color->stage[i].op[j].param1 );

					if ((TexEnvArgs[color->stage[i].op[j].param1].source == GL_CONSTANT_ARB) &&
						(color->stage[i].numOps > (j + 1)) &&
						(TexEnvArgs[color->stage[i].op[j + 1].param1].source == GL_CONSTANT_ARB))
						curUnit++;

//					j = max( color->stage[i].numOps + color->stage[i + 1].numOps * (color->numStages - 1) - 1 - OGL.maxTextureUnits, 0);
					break;
				case SUB:
					if (TexEnvArgs[color->stage[i].op[j].param1].source == GL_CONSTANT_ARB)
						colorConstants[curUnit] = color->stage[i].op[j].param1;

					envCombiner.color[curUnit].combine = GL_SUBTRACT_ARB;
					SetColorCombinerArg( curUnit, arg1, color->stage[i].op[j].param1 );
					curUnit++;
					break;
				case MUL:
					if (TexEnvArgs[color->stage[i].op[j].param1].source == GL_CONSTANT_ARB)
						colorConstants[curUnit] = color->stage[i].op[j].param1;

					envCombiner.color[curUnit].combine = GL_MODULATE;
					SetColorCombinerArg( curUnit, arg1, color->stage[i].op[j].param1 );
					curUnit++;
					break;
				case ADD:
					if (TexEnvArgs[color->stage[i].op[j].param1].source == GL_CONSTANT_ARB)
						colorConstants[curUnit] = color->stage[i].op[j].param1;

					envCombiner.color[curUnit].combine = GL_ADD;
					SetColorCombinerArg( curUnit, arg1, color->stage[i].op[j].param1 );
					curUnit++;
					break;
				case INTER:
					if (TexEnvArgs[color->stage[i].op[j].param1].source == GL_CONSTANT_ARB)
						colorConstants[curUnit] = color->stage[i].op[j].param1;

					if (TexEnvArgs[color->stage[i].op[j].param2].source == GL_CONSTANT_ARB)
					{
						if (colorConstants[curUnit])
						{
							combiner.vertex.color = color->stage[i].op[j].param2;
							color->stage[i].op[j].param2 = SHADE;
						}
						else
							colorConstants[curUnit] = color->stage[i].op[j].param2;
					}

					if (TexEnvArgs[color->stage[i].op[j].param3].source == GL_CONSTANT_ARB)
					{
						if (colorConstants[curUnit])
						{
							combiner.vertex.color = color->stage[i].op[j].param3;
							color->stage[i].op[j].param3 = SHADE;
						}
						else
							colorConstants[curUnit] = color->stage[i].op[j].param3;
					}

					envCombiner.color[curUnit].combine = GL_INTERPOLATE_ARB;
					SetColorCombinerArg( curUnit, arg0, color->stage[i].op[j].param1 );
					SetColorCombinerArg( curUnit, arg1, color->stage[i].op[j].param2 );
					SetColorCombinerArg( curUnit, arg2, color->stage[i].op[j].param3 );
					curUnit++;
					break;
			}
		}
	}

	for (int i = 0; i < OGL.maxTextureUnits; i++)
	{
		glActiveTextureARB( GL_TEXTURE0_ARB + i );

		glTexEnvi( GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, envCombiner.color[i].combine );

		glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB,  envCombiner.color[i].arg0.source );
		glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, envCombiner.color[i].arg0.operand );
		glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB,  envCombiner.color[i].arg1.source );
		glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND1_RGB_ARB, envCombiner.color[i].arg1.operand );
		glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE2_RGB_ARB,  envCombiner.color[i].arg2.source );
		glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND2_RGB_ARB, envCombiner.color[i].arg2.operand );

		glTexEnvi( GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB, envCombiner.alpha[i].combine );

		glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_ARB,  envCombiner.alpha[i].arg0.source );
		glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_ARB, envCombiner.alpha[i].arg0.operand );
		glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_ARB,  envCombiner.alpha[i].arg1.source );
		glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND1_ALPHA_ARB, envCombiner.alpha[i].arg1.operand );
		glTexEnvi( GL_TEXTURE_ENV, GL_SOURCE2_ALPHA_ARB,  envCombiner.alpha[i].arg2.source );
		glTexEnvi( GL_TEXTURE_ENV, GL_OPERAND2_ALPHA_ARB, envCombiner.alpha[i].arg2.operand );
	}
}
