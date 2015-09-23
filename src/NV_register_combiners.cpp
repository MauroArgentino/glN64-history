#include <windows.h>
#include "OpenGL.h"
#include "Combiner.h"
#include "NV_register_combiners.h"
#include "Debug.h"
#include "RDP.h"
#include "RSP.h"

struct 
{
	WORD color, alpha;
} constantColor[2];

WORD secondaryColor;

#define SetColorCombinerInput( n, v, p, s ) \
	regCombiners.color[n].v.input = CombinerInputs[p].input; \
	regCombiners.color[n].v.mapping = CombinerInputs[p].mapping; \
	regCombiners.color[n].v.usage = CombinerInputs[p].usage; \
	regCombiners.color[n].v.used = s

#define SetColorCombinerVariable( n, v, i, m, u, s ) \
	regCombiners.color[n].v.input = i; \
	regCombiners.color[n].v.mapping = m; \
	regCombiners.color[n].v.usage = u; \
	regCombiners.color[n].v.used = s

#define SetAlphaCombinerInput( n, v, p, use ) \
	regCombiners.alpha[n].v.input = CombinerInputs[p].input; \
	regCombiners.alpha[n].v.mapping = CombinerInputs[p].mapping; \
	regCombiners.alpha[n].v.usage = GL_ALPHA; \
	regCombiners.alpha[n].v.used = use

#define SetAlphaCombinerVariable( n, v, i, m, u, use ) \
	regCombiners.alpha[n].v.input = i; \
	regCombiners.alpha[n].v.mapping = m; \
	regCombiners.alpha[n].v.usage = u; \
	regCombiners.alpha[n].v.used = use

#define SetFinalCombinerInput( v, p, use ) \
	regCombiners.final.v.input = CombinerInputs[p].input; \
	regCombiners.final.v.mapping = CombinerInputs[p].mapping; \
	regCombiners.final.v.usage = CombinerInputs[p].usage; \
	regCombiners.final.v.used = use

#define SetFinalCombinerVariable( v, i, m, u, use ) \
	regCombiners.final.v.input = i; \
	regCombiners.final.v.mapping = m; \
	regCombiners.final.v.usage = u; \
	regCombiners.final.v.used = use

void Init_NV_register_combiners()
{
	if (!OGL.NV_register_combiners)
	{
		OGL.combiner = combiner.combiner = ARB_TEXTURE_ENV_COMBINE;
		Combiner_Init();
	}

	glCombinerParameteriNV( GL_COLOR_SUM_CLAMP_NV, GL_TRUE );
	glEnable( GL_REGISTER_COMBINERS_NV );

	glActiveTextureARB( GL_TEXTURE0_ARB );
	glEnable( GL_TEXTURE_2D );

	glActiveTextureARB( GL_TEXTURE1_ARB );
	glEnable( GL_TEXTURE_2D );
}

void Update_NV_register_combiners_Colors()
{
	GLcolor color;

	for (int i = 0; i < 2; i++)
	{
		SetConstant( color, constantColor[i].color, constantColor[i].alpha );

	  	glCombinerParameterfvNV( GL_CONSTANT_COLOR0_NV + i, (GLfloat*)&color );
	}

	SetConstant( color, secondaryColor, ZERO );
	glSecondaryColor3fvEXT( (GLfloat*)&color );
}

void Set_NV_register_combiners( Combiner *color, Combiner *alpha )
{
	int curCombiner, numCombiners;

	RegisterCombiners regCombiners;

	for (int i = 0; i < OGL.maxGeneralCombiners; i++)
	{
		SetColorCombinerInput( i, A, ZERO, FALSE );
		SetColorCombinerInput( i, B, ZERO, FALSE );
		SetColorCombinerInput( i, C, ZERO, FALSE );
		SetColorCombinerInput( i, D, ZERO, FALSE );
		regCombiners.color[i].output.ab = GL_DISCARD_NV;
		regCombiners.color[i].output.cd = GL_DISCARD_NV;
		regCombiners.color[i].output.sum = GL_DISCARD_NV;

		SetAlphaCombinerInput( i, A, ZERO, FALSE );
		SetAlphaCombinerInput( i, B, ZERO, FALSE );
		SetAlphaCombinerInput( i, C, ZERO, FALSE );
		SetAlphaCombinerInput( i, D, ZERO, FALSE );
		regCombiners.alpha[i].output.ab = GL_DISCARD_NV;
		regCombiners.alpha[i].output.cd = GL_DISCARD_NV;
		regCombiners.alpha[i].output.sum = GL_DISCARD_NV;
	}

	SetFinalCombinerInput( A, ONE, FALSE );
	SetFinalCombinerInput( B, COMBINED, FALSE );
	SetFinalCombinerInput( C, ZERO, FALSE );
	SetFinalCombinerInput( D, ZERO, FALSE );
	SetFinalCombinerInput( E, ZERO, FALSE );
	SetFinalCombinerInput( F, ZERO, FALSE );
	SetFinalCombinerInput( G, COMBINED, FALSE );

	if ((RSP.geometryMode & RSP_GEOMETRYMODE_FOG) &&
		(((RDP.otherMode_H & CYCLETYPE) == CYCLETYPE_1CYCLE) ||
		((RDP.otherMode_H & CYCLETYPE) == CYCLETYPE_2CYCLE)))
	{
		SetFinalCombinerVariable( A, GL_FOG, GL_UNSIGNED_IDENTITY_NV, GL_ALPHA, FALSE );
		SetFinalCombinerVariable( C, GL_FOG, GL_UNSIGNED_IDENTITY_NV, GL_RGB, FALSE );
	}

	CombinerInputs[PRIMITIVE].input = GL_ZERO;
	CombinerInputs[PRIMALPHA].input = GL_ZERO;
	CombinerInputs[ENVIRONMENT].input = GL_ZERO;
	CombinerInputs[ENVALPHA].input = GL_ZERO;
	CombinerInputs[PRIMLODFRAC].input = GL_ZERO;
	constantColor[0].color = COMBINED;
	constantColor[0].alpha = CMBALPHA;
	constantColor[1].color = COMBINED;
	constantColor[1].alpha = CMBALPHA;
	secondaryColor = COMBINED;
	
	curCombiner = 0;
	for (int i = 0; i < alpha->numStages; i++)
	{
		for (int j = 0; j < alpha->stage[i].numOps; j++)
		{
			if ((alpha->stage[i].op[j].param1 == SHADEALPHA) ||
				(alpha->stage[i].op[j].param2 == SHADEALPHA) ||
				(alpha->stage[i].op[j].param3 == SHADEALPHA))
			{
				combiner.vertex.alpha = SHADEALPHA;
			}

			if (((alpha->stage[i].op[j].param1 == PRIMALPHA) ||
				(alpha->stage[i].op[j].param2 == PRIMALPHA) ||
				(alpha->stage[i].op[j].param3 == PRIMALPHA)) &&
				(CombinerInputs[PRIMALPHA].input == GL_ZERO))
			{
				if (constantColor[0].alpha == CMBALPHA)
				{
					constantColor[0].alpha = PRIMALPHA;
					CombinerInputs[PRIMALPHA].input = GL_CONSTANT_COLOR0_NV;
					CombinerInputs[PRIMALPHA].usage = GL_ALPHA;
				}
				else if (constantColor[1].alpha == CMBALPHA)
				{
					constantColor[1].alpha = PRIMALPHA;
					CombinerInputs[PRIMALPHA].input = GL_CONSTANT_COLOR1_NV;
					CombinerInputs[PRIMALPHA].usage = GL_ALPHA;
				}
				else if (combiner.vertex.alpha == CMBALPHA)
				{
					combiner.vertex.alpha = PRIMALPHA;
					CombinerInputs[PRIMALPHA].input = GL_PRIMARY_COLOR_NV;
					CombinerInputs[PRIMALPHA].usage = GL_ALPHA;
				}
			}

			if (((alpha->stage[i].op[j].param1 == ENVALPHA) ||
				(alpha->stage[i].op[j].param2 == ENVALPHA) ||
				(alpha->stage[i].op[j].param3 == ENVALPHA)) &&
				(CombinerInputs[ENVALPHA].input == GL_ZERO))
			{
				if (constantColor[0].alpha == CMBALPHA)
				{
					constantColor[0].alpha = ENVALPHA;
					CombinerInputs[ENVALPHA].input = GL_CONSTANT_COLOR0_NV;
					CombinerInputs[ENVALPHA].usage = GL_ALPHA;
				}
				else if (constantColor[1].alpha == CMBALPHA)
				{
					constantColor[1].alpha = ENVALPHA;
					CombinerInputs[ENVALPHA].input = GL_CONSTANT_COLOR1_NV;
					CombinerInputs[ENVALPHA].usage = GL_ALPHA;
				}
				else if (combiner.vertex.alpha == CMBALPHA)
				{
					combiner.vertex.alpha = ENVALPHA;
					CombinerInputs[ENVALPHA].input = GL_PRIMARY_COLOR_NV;
					CombinerInputs[ENVALPHA].usage = GL_ALPHA;
				}
			}

			if (((alpha->stage[i].op[j].param1 == PRIMLODFRAC) ||
				(alpha->stage[i].op[j].param2 == PRIMLODFRAC) ||
				(alpha->stage[i].op[j].param3 == PRIMLODFRAC)) &&
				(CombinerInputs[PRIMLODFRAC].input == GL_ZERO))
			{
				if (constantColor[0].alpha == CMBALPHA)
				{
					constantColor[0].alpha = PRIMLODFRAC;
					CombinerInputs[PRIMLODFRAC].input = GL_CONSTANT_COLOR0_NV;
					CombinerInputs[PRIMLODFRAC].usage = GL_ALPHA;
				}
				else if (constantColor[1].alpha == CMBALPHA)
				{
					constantColor[1].alpha = PRIMLODFRAC;
					CombinerInputs[PRIMLODFRAC].input = GL_CONSTANT_COLOR1_NV;
					CombinerInputs[PRIMLODFRAC].usage = GL_ALPHA;
				}
				else if (combiner.vertex.alpha == CMBALPHA)
				{
					combiner.vertex.alpha = PRIMLODFRAC;
					CombinerInputs[PRIMLODFRAC].input = GL_PRIMARY_COLOR_NV;
					CombinerInputs[PRIMLODFRAC].usage = GL_ALPHA;
				}
			}
		
			RDP.useT0 |= (alpha->stage[i].op[j].param1 == T0ALPHA) || (alpha->stage[i].op[j].param2 == T0ALPHA) || (alpha->stage[i].op[j].param3 == T0ALPHA);
			RDP.useT1 |= (alpha->stage[i].op[j].param1 == T1ALPHA) || (alpha->stage[i].op[j].param2 == T1ALPHA) || (alpha->stage[i].op[j].param3 == T1ALPHA);
			RDP.useNoise |= (alpha->stage[i].op[j].param1 == NOISE) || (alpha->stage[i].op[j].param2 == NOISE) || (alpha->stage[i].op[j].param3 == NOISE);

			switch (alpha->stage[i].op[j].op)
			{
				case LOAD:
					if (regCombiners.alpha[curCombiner].A.used)
					{
						curCombiner++;

						if (curCombiner == OGL.maxGeneralCombiners)
							break;
					}

					SetAlphaCombinerInput( curCombiner, A, alpha->stage[i].op[j].param1, TRUE );
					SetAlphaCombinerInput( curCombiner, B, ONE, FALSE );
					regCombiners.alpha[curCombiner].output.sum = GL_SPARE0_NV;
					break;

				case SUB:
					if (regCombiners.alpha[curCombiner].C.used || regCombiners.alpha[curCombiner].D.used)
					{
						curCombiner++;

						if (curCombiner == OGL.maxGeneralCombiners)
							break;

						SetAlphaCombinerVariable( curCombiner, A, GL_SPARE0_NV, GL_SIGNED_IDENTITY_NV, GL_RGB, TRUE );
						SetAlphaCombinerInput( curCombiner, B, ONE, FALSE );
						regCombiners.alpha[curCombiner].output.sum = GL_SPARE0_NV;
					}

					if (alpha->stage[i].op[j].param1 == ONE)
					{
						SetAlphaCombinerVariable( curCombiner, C, GL_ZERO, GL_EXPAND_NORMAL_NV, GL_ALPHA, TRUE );
					}
					else
					{
						SetAlphaCombinerVariable( curCombiner, C, CombinerInputs[alpha->stage[i].op[j].param1].input, GL_SIGNED_NEGATE_NV, GL_ALPHA, TRUE );
					}

					SetAlphaCombinerInput( curCombiner, D, ONE, FALSE );
					break;

				case MUL:
					if (regCombiners.alpha[curCombiner].B.used || regCombiners.alpha[curCombiner].D.used)
					{
						curCombiner++;

						if (curCombiner == OGL.maxGeneralCombiners)
							break;

						SetAlphaCombinerVariable( curCombiner, A, GL_SPARE0_NV, GL_SIGNED_IDENTITY_NV, GL_ALPHA, TRUE );
						regCombiners.alpha[curCombiner].output.sum = GL_SPARE0_NV;
					}

					SetAlphaCombinerInput( curCombiner, B, alpha->stage[i].op[j].param1, TRUE );

					if (regCombiners.alpha[curCombiner].C.used)
					{
						SetAlphaCombinerInput( curCombiner, D, alpha->stage[i].op[j].param1, TRUE );
					}
					break;
				case ADD:
					if (regCombiners.alpha[curCombiner].C.used || regCombiners.alpha[curCombiner].D.used)
					{
						curCombiner++;

						if (curCombiner == OGL.maxGeneralCombiners)
							break;

						SetAlphaCombinerVariable( curCombiner, A, GL_SPARE0_NV, GL_SIGNED_IDENTITY_NV, GL_ALPHA, TRUE );
						SetAlphaCombinerInput( curCombiner, B, ONE, FALSE );
						regCombiners.alpha[curCombiner].output.sum = GL_SPARE0_NV;
					}

					SetAlphaCombinerInput( curCombiner, C, alpha->stage[i].op[j].param1, TRUE );
					SetAlphaCombinerInput( curCombiner, D, ONE, FALSE );
					break;

				case INTER:
					if (regCombiners.alpha[curCombiner].A.used ||
						regCombiners.alpha[curCombiner].B.used ||
						regCombiners.alpha[curCombiner].C.used ||
						regCombiners.alpha[curCombiner].D.used)
					{
						curCombiner++;

						if (curCombiner == OGL.maxGeneralCombiners)
							break;
					}

					SetAlphaCombinerInput( curCombiner, A, alpha->stage[i].op[j].param1, TRUE );
					SetAlphaCombinerInput( curCombiner, B, alpha->stage[i].op[j].param3, TRUE );
					SetAlphaCombinerInput( curCombiner, C, alpha->stage[i].op[j].param2, TRUE );
					SetAlphaCombinerVariable( curCombiner, D, CombinerInputs[alpha->stage[i].op[j].param3].input, GL_UNSIGNED_INVERT_NV, GL_ALPHA, TRUE );

					regCombiners.alpha[curCombiner].output.sum = GL_SPARE0_NV;
					break;
			}

			if (curCombiner == OGL.maxGeneralCombiners)
				break; // Get out if the combiners are full
		}
		if (curCombiner == OGL.maxGeneralCombiners)
			break; // Get out if the combiners are full
	}

	numCombiners = min( curCombiner + 1, OGL.maxGeneralCombiners );

	curCombiner = 0;
	for (int i = 0; i < (color->numStages) && (curCombiner < OGL.maxGeneralCombiners); i++)
	{
		for (int j = 0; (j < color->stage[i].numOps) && (curCombiner < OGL.maxGeneralCombiners); j++)
		{
			if ((color->stage[i].op[j].param1 == SHADEALPHA) ||
				(color->stage[i].op[j].param2 == SHADEALPHA) ||
				(color->stage[i].op[j].param3 == SHADEALPHA))
			{
				combiner.vertex.alpha = SHADEALPHA;
			}

			if ((color->stage[i].op[j].param1 == SHADE) ||
				(color->stage[i].op[j].param2 == SHADE) ||
				(color->stage[i].op[j].param3 == SHADE))
			{
				combiner.vertex.color = SHADE;
			}

			if (((color->stage[i].op[j].param1 == PRIMITIVE) ||
				(color->stage[i].op[j].param2 == PRIMITIVE) ||
				(color->stage[i].op[j].param3 == PRIMITIVE)) &&
				(CombinerInputs[PRIMITIVE].input == GL_ZERO))
			{
				if (constantColor[0].color == COMBINED)
				{
					constantColor[0].color = PRIMITIVE;
					CombinerInputs[PRIMITIVE].input = GL_CONSTANT_COLOR0_NV;
					CombinerInputs[PRIMITIVE].usage = GL_RGB;
				}
				else if (constantColor[1].color == COMBINED)
				{
					constantColor[1].color = PRIMITIVE;
					CombinerInputs[PRIMITIVE].input = GL_CONSTANT_COLOR1_NV;
					CombinerInputs[PRIMITIVE].usage = GL_RGB;
				}
				else if (secondaryColor == COMBINED)
				{
					secondaryColor = PRIMITIVE;
					CombinerInputs[PRIMITIVE].input = GL_SECONDARY_COLOR_NV;
					CombinerInputs[PRIMITIVE].usage = GL_RGB;
				}
				else if (combiner.vertex.color == COMBINED)
				{
					combiner.vertex.color = PRIMITIVE;
					CombinerInputs[PRIMITIVE].input = GL_PRIMARY_COLOR_NV;
					CombinerInputs[PRIMITIVE].usage = GL_RGB;
				}
			}

			if (((color->stage[i].op[j].param1 == PRIMALPHA) ||
				(color->stage[i].op[j].param2 == PRIMALPHA) ||
				(color->stage[i].op[j].param3 == PRIMALPHA)) &&
				(CombinerInputs[PRIMALPHA].input == GL_ZERO))
			{
				if (constantColor[0].alpha == CMBALPHA)
				{
					constantColor[0].alpha = PRIMALPHA;
					CombinerInputs[PRIMALPHA].input = GL_CONSTANT_COLOR0_NV;
					CombinerInputs[PRIMALPHA].usage = GL_ALPHA;
				}
				else if (constantColor[1].alpha == CMBALPHA)
				{
					constantColor[1].alpha = PRIMALPHA;
					CombinerInputs[PRIMALPHA].input = GL_CONSTANT_COLOR1_NV;
					CombinerInputs[PRIMALPHA].usage = GL_ALPHA;
				}
				else if (combiner.vertex.alpha == SHADEALPHA)
				{
					combiner.vertex.alpha = PRIMALPHA;
					CombinerInputs[PRIMALPHA].input = GL_PRIMARY_COLOR_NV;
					CombinerInputs[PRIMALPHA].usage = GL_ALPHA;
				}
				else if (constantColor[0].color == COMBINED)
				{
					constantColor[0].color = PRIMALPHA;
					CombinerInputs[PRIMALPHA].input = GL_CONSTANT_COLOR0_NV;
					CombinerInputs[PRIMALPHA].usage = GL_RGB;
				}
				else if (constantColor[1].color == COMBINED)
				{
					constantColor[1].color = PRIMALPHA;
					CombinerInputs[PRIMALPHA].input = GL_CONSTANT_COLOR1_NV;
					CombinerInputs[PRIMALPHA].usage = GL_RGB;
				}
				else if (secondaryColor == COMBINED)
				{
					secondaryColor = PRIMALPHA;
					CombinerInputs[PRIMALPHA].input = GL_SECONDARY_COLOR_NV;
					CombinerInputs[PRIMALPHA].usage = GL_RGB;
				}
				else if (combiner.vertex.color == COMBINED)
				{
					combiner.vertex.color = PRIMALPHA;
					CombinerInputs[PRIMALPHA].input = GL_PRIMARY_COLOR_NV;
					CombinerInputs[PRIMALPHA].usage = GL_RGB;
				}
			}

			if (((color->stage[i].op[j].param1 == ENVIRONMENT) ||
				(color->stage[i].op[j].param2 == ENVIRONMENT) ||
				(color->stage[i].op[j].param3 == ENVIRONMENT)) &&
				(CombinerInputs[ENVIRONMENT].input == GL_ZERO))
			{
				if (constantColor[0].color == COMBINED)
				{
					constantColor[0].color = ENVIRONMENT;
					CombinerInputs[ENVIRONMENT].input = GL_CONSTANT_COLOR0_NV;
					CombinerInputs[ENVIRONMENT].usage = GL_RGB;
				}
				else if (constantColor[1].color == COMBINED)
				{
					constantColor[1].color = ENVIRONMENT;
					CombinerInputs[ENVIRONMENT].input = GL_CONSTANT_COLOR1_NV;
					CombinerInputs[ENVIRONMENT].usage = GL_RGB;
				}
				else if (secondaryColor == COMBINED)
				{
					secondaryColor = ENVIRONMENT;
					CombinerInputs[ENVIRONMENT].input = GL_SECONDARY_COLOR_NV;
					CombinerInputs[ENVIRONMENT].usage = GL_RGB;
				}
				else if (combiner.vertex.color == COMBINED)
				{
					combiner.vertex.color = ENVIRONMENT;
					CombinerInputs[ENVIRONMENT].input = GL_PRIMARY_COLOR_NV;
					CombinerInputs[ENVIRONMENT].usage = GL_RGB;
				}
			}

			if (((color->stage[i].op[j].param1 == ENVALPHA) ||
				(color->stage[i].op[j].param2 == ENVALPHA) ||
				(color->stage[i].op[j].param3 == ENVALPHA)) &&
				(CombinerInputs[ENVALPHA].input == GL_ZERO))
			{
				if (constantColor[0].alpha == CMBALPHA)
				{
					constantColor[0].alpha = ENVALPHA;
					CombinerInputs[ENVALPHA].input = GL_CONSTANT_COLOR0_NV;
					CombinerInputs[ENVALPHA].usage = GL_ALPHA;
				}
				else if (constantColor[1].alpha == CMBALPHA)
				{
					constantColor[1].alpha = ENVALPHA;
					CombinerInputs[ENVALPHA].input = GL_CONSTANT_COLOR1_NV;
					CombinerInputs[ENVALPHA].usage = GL_ALPHA;
				}
				else if (combiner.vertex.alpha == CMBALPHA)
				{
					combiner.vertex.alpha = ENVALPHA;
					CombinerInputs[ENVALPHA].input = GL_PRIMARY_COLOR_NV;
					CombinerInputs[ENVALPHA].usage = GL_ALPHA;
				}
				else if (constantColor[0].color == COMBINED)
				{
					constantColor[0].color = ENVALPHA;
					CombinerInputs[ENVALPHA].input = GL_CONSTANT_COLOR0_NV;
					CombinerInputs[ENVALPHA].usage = GL_RGB;
				}
				else if (constantColor[1].color == COMBINED)
				{
					constantColor[1].color = ENVALPHA;
					CombinerInputs[ENVALPHA].input = GL_CONSTANT_COLOR1_NV;
					CombinerInputs[ENVALPHA].usage = GL_RGB;
				}
				else if (secondaryColor == COMBINED)
				{
					secondaryColor = ENVALPHA;
					CombinerInputs[ENVALPHA].input = GL_SECONDARY_COLOR_NV;
					CombinerInputs[ENVALPHA].usage = GL_RGB;
				}
				else if (combiner.vertex.color == COMBINED)
				{
					combiner.vertex.color = ENVALPHA;
					CombinerInputs[ENVALPHA].input = GL_PRIMARY_COLOR_NV;
					CombinerInputs[ENVALPHA].usage = GL_RGB;
				}
			}

			if (((color->stage[i].op[j].param1 == PRIMLODFRAC) ||
				(color->stage[i].op[j].param2 == PRIMLODFRAC) ||
				(color->stage[i].op[j].param3 == PRIMLODFRAC)) &&
				(CombinerInputs[PRIMLODFRAC].input == GL_ZERO))
			{
				if (constantColor[0].alpha == CMBALPHA)
				{
					constantColor[0].alpha = PRIMLODFRAC;
					CombinerInputs[PRIMLODFRAC].input = GL_CONSTANT_COLOR0_NV;
					CombinerInputs[PRIMLODFRAC].usage = GL_ALPHA;
				}
				else if (constantColor[1].alpha == CMBALPHA)
				{
					constantColor[1].alpha = PRIMLODFRAC;
					CombinerInputs[PRIMLODFRAC].input = GL_CONSTANT_COLOR1_NV;
					CombinerInputs[PRIMLODFRAC].usage = GL_ALPHA;
				}
				else if (combiner.vertex.alpha == CMBALPHA)
				{
					combiner.vertex.alpha = PRIMLODFRAC;
					CombinerInputs[PRIMLODFRAC].input = GL_PRIMARY_COLOR_NV;
					CombinerInputs[PRIMLODFRAC].usage = GL_ALPHA;
				}
				else if (constantColor[0].color == COMBINED)
				{
					constantColor[0].color = PRIMLODFRAC;
					CombinerInputs[PRIMLODFRAC].input = GL_CONSTANT_COLOR0_NV;
					CombinerInputs[PRIMLODFRAC].usage = GL_RGB;
				}
				else if (constantColor[1].color == COMBINED)
				{
					constantColor[1].color = PRIMLODFRAC;
					CombinerInputs[PRIMLODFRAC].input = GL_CONSTANT_COLOR1_NV;
					CombinerInputs[PRIMLODFRAC].usage = GL_RGB;
				}
				else if (secondaryColor == COMBINED)
				{
					secondaryColor = PRIMLODFRAC;
					CombinerInputs[PRIMLODFRAC].input = GL_SECONDARY_COLOR_NV;
					CombinerInputs[PRIMLODFRAC].usage = GL_RGB;
				}
				else if (combiner.vertex.color == COMBINED)
				{
					combiner.vertex.color = PRIMLODFRAC;
					CombinerInputs[PRIMLODFRAC].input = GL_PRIMARY_COLOR_NV;
					CombinerInputs[PRIMLODFRAC].usage = GL_RGB;
				}
			}

			RDP.useT0 |= (color->stage[i].op[j].param1 == TEXTURE0) || (color->stage[i].op[j].param2 == TEXTURE0) || (color->stage[i].op[j].param3 == TEXTURE0) || (color->stage[i].op[j].param1 == T0ALPHA) || (color->stage[i].op[j].param2 == T0ALPHA) || (color->stage[i].op[j].param3 == T0ALPHA);
			RDP.useT1 |= (color->stage[i].op[j].param1 == TEXTURE1) || (color->stage[i].op[j].param2 == TEXTURE1) || (color->stage[i].op[j].param3 == TEXTURE1) || (color->stage[i].op[j].param1 == T1ALPHA) || (color->stage[i].op[j].param2 == T1ALPHA) || (color->stage[i].op[j].param3 == T1ALPHA);
			RDP.useNoise |= (color->stage[i].op[j].param1 == NOISE) || (color->stage[i].op[j].param2 == NOISE) || (color->stage[i].op[j].param3 == NOISE);
			
			switch (color->stage[i].op[j].op)
			{
				case LOAD:
					if (regCombiners.color[curCombiner].A.used)
					{
						curCombiner++;

						if (curCombiner == OGL.maxGeneralCombiners)
							break;
					}

					SetColorCombinerInput( curCombiner, A, color->stage[i].op[j].param1, TRUE );
					SetColorCombinerInput( curCombiner, B, ONE, FALSE );
					regCombiners.color[curCombiner].output.sum = GL_SPARE0_NV;
					break;

				case SUB:
					if (regCombiners.color[curCombiner].C.used || regCombiners.color[curCombiner].D.used)
					{
						curCombiner++;

						if (curCombiner == OGL.maxGeneralCombiners)
							break;

						SetColorCombinerVariable( curCombiner, A, GL_SPARE0_NV, GL_SIGNED_IDENTITY_NV, GL_RGB, TRUE );
						SetColorCombinerInput( curCombiner, B, ONE, FALSE );
						regCombiners.color[curCombiner].output.sum = GL_SPARE0_NV;
					}

					SetColorCombinerVariable( curCombiner, C, CombinerInputs[color->stage[i].op[j].param1].input, GL_SIGNED_NEGATE_NV, GL_RGB, TRUE );
					SetColorCombinerInput( curCombiner, D, ONE, FALSE );
					break;

				case MUL:
					if (regCombiners.color[curCombiner].B.used || regCombiners.color[curCombiner].D.used)
					{
						curCombiner++;

						if (curCombiner == OGL.maxGeneralCombiners)
						{
							if ((!regCombiners.final.B.used) &&
								(!regCombiners.final.E.used) &&
								(!regCombiners.final.F.used))
							{
								SetFinalCombinerVariable( B, GL_E_TIMES_F_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB, TRUE );
								SetFinalCombinerInput( E, COMBINED, TRUE );
								SetFinalCombinerInput( F, color->stage[i].op[j].param1, TRUE );
							}
							break;
						}

						SetColorCombinerVariable( curCombiner, A, GL_SPARE0_NV, GL_SIGNED_IDENTITY_NV, GL_RGB, TRUE );
						regCombiners.color[curCombiner].output.sum = GL_SPARE0_NV;
					}

					SetColorCombinerInput( curCombiner, B, color->stage[i].op[j].param1, TRUE );

					if (regCombiners.color[curCombiner].C.used)
					{
						SetColorCombinerInput( curCombiner, D, color->stage[i].op[j].param1, TRUE );
					}
					break;
				case ADD:
					if (regCombiners.color[curCombiner].C.used || regCombiners.color[curCombiner].D.used)
					{
						curCombiner++;

						if (curCombiner == OGL.maxGeneralCombiners)
						{
							if (!regCombiners.final.D.used)
							{
								SetFinalCombinerInput( D, color->stage[i].op[j].param1, TRUE );
							}

							break;
						}

						SetColorCombinerVariable( curCombiner, A, GL_SPARE0_NV, GL_SIGNED_IDENTITY_NV, GL_RGB, TRUE );
						SetColorCombinerInput( curCombiner, B, ONE, FALSE );
						regCombiners.color[curCombiner].output.sum = GL_SPARE0_NV;
					}

					SetColorCombinerInput( curCombiner, C, color->stage[i].op[j].param1, TRUE );
					SetColorCombinerInput( curCombiner, D, ONE, FALSE );
					break;

				case INTER:
					if (regCombiners.color[curCombiner].A.used ||
						regCombiners.color[curCombiner].B.used ||
						regCombiners.color[curCombiner].C.used ||
						regCombiners.color[curCombiner].D.used)
					{
						curCombiner++;

						if (curCombiner == OGL.maxGeneralCombiners)
						{
							if (!regCombiners.final.A.used &&
								!regCombiners.final.B.used &&
								!regCombiners.final.C.used)
							{
								SetFinalCombinerInput( A, color->stage[i].op[j].param3, TRUE );
								SetFinalCombinerInput( B, color->stage[i].op[j].param1, TRUE );
								SetFinalCombinerInput( C, color->stage[i].op[j].param2, TRUE );
							}
							break;
						}
					}

					SetColorCombinerInput( curCombiner, A, color->stage[i].op[j].param1, TRUE );
					SetColorCombinerInput( curCombiner, B, color->stage[i].op[j].param3, TRUE );
					SetColorCombinerInput( curCombiner, C, color->stage[i].op[j].param2, TRUE );
					SetColorCombinerVariable( curCombiner, D, CombinerInputs[color->stage[i].op[j].param3].input, GL_UNSIGNED_INVERT_NV, CombinerInputs[color->stage[i].op[j].param3].usage, TRUE );

					regCombiners.color[curCombiner].output.sum = GL_SPARE0_NV;
					break;
			}
		}
	}

	numCombiners = max( min( curCombiner + 1, OGL.maxGeneralCombiners ), numCombiners );
	glCombinerParameteriNV( GL_NUM_GENERAL_COMBINERS_NV, numCombiners );

	for (int i = 0; i < numCombiners; i++)
	{
		glCombinerInputNV( GL_COMBINER0_NV + i, GL_RGB, GL_VARIABLE_A_NV, regCombiners.color[i].A.input, regCombiners.color[i].A.mapping, regCombiners.color[i].A.usage );
		glCombinerInputNV( GL_COMBINER0_NV + i, GL_RGB, GL_VARIABLE_B_NV, regCombiners.color[i].B.input, regCombiners.color[i].B.mapping, regCombiners.color[i].B.usage );
		glCombinerInputNV( GL_COMBINER0_NV + i, GL_RGB, GL_VARIABLE_C_NV, regCombiners.color[i].C.input, regCombiners.color[i].C.mapping, regCombiners.color[i].C.usage );
		glCombinerInputNV( GL_COMBINER0_NV + i, GL_RGB, GL_VARIABLE_D_NV, regCombiners.color[i].D.input, regCombiners.color[i].D.mapping, regCombiners.color[i].D.usage );
		glCombinerOutputNV( GL_COMBINER0_NV + i, GL_RGB, regCombiners.color[i].output.ab, regCombiners.color[i].output.cd, regCombiners.color[i].output.sum, GL_NONE, GL_NONE, GL_FALSE, GL_FALSE, GL_FALSE );

		glCombinerInputNV( GL_COMBINER0_NV + i, GL_ALPHA, GL_VARIABLE_A_NV, regCombiners.alpha[i].A.input, regCombiners.alpha[i].A.mapping, GL_ALPHA );
		glCombinerInputNV( GL_COMBINER0_NV + i, GL_ALPHA, GL_VARIABLE_B_NV, regCombiners.alpha[i].B.input, regCombiners.alpha[i].B.mapping, GL_ALPHA );
		glCombinerInputNV( GL_COMBINER0_NV + i, GL_ALPHA, GL_VARIABLE_C_NV, regCombiners.alpha[i].C.input, regCombiners.alpha[i].C.mapping, GL_ALPHA );
		glCombinerInputNV( GL_COMBINER0_NV + i, GL_ALPHA, GL_VARIABLE_D_NV, regCombiners.alpha[i].D.input, regCombiners.alpha[i].D.mapping, GL_ALPHA );
		glCombinerOutputNV( GL_COMBINER0_NV + i, GL_ALPHA, regCombiners.alpha[i].output.ab, regCombiners.alpha[i].output.cd, regCombiners.alpha[i].output.sum, GL_NONE, GL_NONE, GL_FALSE, GL_FALSE, GL_FALSE );
	}

	glFinalCombinerInputNV( GL_VARIABLE_A_NV, regCombiners.final.A.input, regCombiners.final.A.mapping, regCombiners.final.A.usage );
	glFinalCombinerInputNV( GL_VARIABLE_B_NV, regCombiners.final.B.input, regCombiners.final.B.mapping, regCombiners.final.B.usage );
	glFinalCombinerInputNV( GL_VARIABLE_C_NV, regCombiners.final.C.input, regCombiners.final.C.mapping, regCombiners.final.C.usage );
	glFinalCombinerInputNV( GL_VARIABLE_D_NV, regCombiners.final.D.input, regCombiners.final.D.mapping, regCombiners.final.D.usage );
	glFinalCombinerInputNV( GL_VARIABLE_E_NV, regCombiners.final.E.input, regCombiners.final.E.mapping, regCombiners.final.E.usage );
	glFinalCombinerInputNV( GL_VARIABLE_F_NV, regCombiners.final.F.input, regCombiners.final.F.mapping, regCombiners.final.F.usage );
	glFinalCombinerInputNV( GL_VARIABLE_G_NV, regCombiners.final.G.input, regCombiners.final.G.mapping, GL_ALPHA );
}