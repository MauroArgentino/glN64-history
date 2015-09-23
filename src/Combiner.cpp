#include <windows.h>
#include "OpenGL.h"
#include "Combiner.h"
#include "NV_register_combiners.h"
#include "ARB_texture_env_combine.h"
#include "Debug.h"
#include "RDP.h"
#include "RSP.h"

CombinerInfo combiner;

void Combiner_Init()
{
	switch (OGL.combiner)
	{
		case NV_REGISTER_COMBINERS:
			glActiveTextureARB( GL_TEXTURE0_ARB );
			glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );
			glActiveTextureARB( GL_TEXTURE1_ARB );
			glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );

			Init_NV_register_combiners();
			break;

		case ARB_TEXTURE_ENV_COMBINE:
			glDisable( GL_REGISTER_COMBINERS_NV );

			Init_ARB_texture_env_combine();
			break;
	}
}

void Combiner_UpdateCombineColors()
{
	switch (OGL.combiner)
	{
		case NV_REGISTER_COMBINERS:
			Update_NV_register_combiners_Colors();
			break;

		case ARB_TEXTURE_ENV_COMBINE:
			Update_ARB_texture_env_combine_Colors();
			break;
	}

	RSP.update &= ~UPDATE_COMBINE_COLORS;
}

void Combiner_SimplifyCycle( CombinerCycle *cc, CombinerStage *stage )
{
	// Load the first operand
	stage->op[0].op = LOAD;
	stage->op[0].param1 = cc->A;
	stage->numOps = 1;

	// If we're just subtracting zero, skip it
	if (cc->B != ZERO)
	{
		// Subtracting a number from itself is zero
		if (cc->B == stage->op[0].param1)
			stage->op[0].param1 = ZERO;
		else
		{
			stage->op[1].op = SUB;
			stage->op[1].param1 = cc->B;
			stage->numOps++;
		}
	}

	// If we either subtracted, or didn't load a zero
	if ((stage->numOps > 1) || (stage->op[0].param1 != ZERO))
	{
		// Multiplying by zero is zero
		if (cc->C == ZERO)
		{
			stage->numOps = 1;
			stage->op[0].op = LOAD;
			stage->op[0].param1 = ZERO;
		}
		else
		{
			// Multiplying by one, so just do a load
			if ((stage->numOps == 1) && (stage->op[0].param1 == ONE))
				stage->op[0].param1 = cc->C;
			else
			{
				stage->op[stage->numOps].op = MUL;
				stage->op[stage->numOps].param1 = cc->C;
				stage->numOps++;
			}
		}
	}

	// Don't bother adding zero
	if (cc->D != ZERO)
	{
		// If all we have so far is zero, then load this instead
		if ((stage->numOps == 1) && (stage->op[0].param1 == ZERO))
			stage->op[0].param1 = cc->D;
		else
		{
			stage->op[stage->numOps].op = ADD;
			stage->op[stage->numOps].param1 = cc->D;
			stage->numOps++;
		}
	}

	// Handle interpolation
	if ((stage->numOps == 4) && (stage->op[1].param1 == stage->op[3].param1))
	{
		stage->op[0].op = INTER;
		stage->op[0].param2 = stage->op[1].param1;
		stage->op[0].param3 = stage->op[2].param1;
		stage->numOps = 1;
	}
}

void Combiner_MergeStages( Combiner *c )
{
	// If all we have is a load in the first stage we can just replace
	// each occurance of COMBINED in the second stage with it
	if ((c->stage[0].numOps == 1) && (c->stage[0].op[0].op == LOAD))
	{
		int combined = c->stage[0].op[0].param1;

		for (int i = 0; i < c->stage[1].numOps; i++)
		{
			c->stage[0].op[i].op = c->stage[1].op[i].op;
			c->stage[0].op[i].param1 = (c->stage[1].op[i].param1 == COMBINED) ? combined : c->stage[1].op[i].param1;
			c->stage[0].op[i].param2 = (c->stage[1].op[i].param2 == COMBINED) ? combined : c->stage[1].op[i].param2;
			c->stage[0].op[i].param3 = (c->stage[1].op[i].param3 == COMBINED) ? combined : c->stage[1].op[i].param3;
		}

		c->stage[0].numOps = c->stage[1].numOps;
		c->numStages = 1;
	}
	// We can't do any merging on an interpolation
	else if (c->stage[1].op[0].op != INTER)
	{
		int numCombined = 0;

		// See how many times the first stage is used in the second one
		for (int i = 0; i < c->stage[1].numOps; i++)
			if (c->stage[1].op[i].param1 == COMBINED)
				numCombined++;

		// If it's not used, just replace the first stage with the second
		if (numCombined == 0)
		{
			for (int i = 0; i < c->stage[1].numOps; i++)
			{
				c->stage[0].op[i].op = c->stage[1].op[i].op;
				c->stage[0].op[i].param1 = c->stage[1].op[i].param1;
				c->stage[0].op[i].param2 = c->stage[1].op[i].param2;
				c->stage[0].op[i].param3 = c->stage[1].op[i].param3;
			}
			c->stage[0].numOps = c->stage[1].numOps;

			c->numStages = 1;
		}
		// If it's only used once
		else if (numCombined == 1)
		{
			// It's only used in the load, so tack on the ops from stage 2 on stage 1
			if (c->stage[1].op[0].param1 == COMBINED)
			{
				for (int i = 1; i < c->stage[1].numOps; i++)
				{
					c->stage[0].op[c->stage[0].numOps].op = c->stage[1].op[i].op;
					c->stage[0].op[c->stage[0].numOps].param1 = c->stage[1].op[i].param1;
					c->stage[0].numOps++;
				}

				c->numStages = 1;
			}
			// Otherwise, if it's used in the second op, and that op isn't SUB
			// we can switch the parameters so it works out to tack the ops onto stage 1
			else if ((c->stage[1].op[1].param1 == COMBINED) && (c->stage[1].op[1].op != SUB))
			{
				c->stage[0].op[c->stage[0].numOps].op = c->stage[1].op[1].op;
				c->stage[0].op[c->stage[0].numOps].param1 = c->stage[1].op[0].param1;
				c->stage[0].numOps++;

				// If there's another op, tack it onto stage 1 too
				if (c->stage[1].numOps > 2)
				{
					c->stage[0].op[c->stage[0].numOps].op = c->stage[1].op[2].op;
					c->stage[0].op[c->stage[0].numOps].param1 = c->stage[1].op[2].param1;
					c->stage[0].numOps++;
				}

				c->numStages = 1;
			}
		}
	}
}

void Combiner_GenerateMode()
{
	int numCycles;

	Combiner color, alpha;

	if ((RDP.otherMode_H & CYCLETYPE) == CYCLETYPE_FILL)
	{
		color.numStages = 1;
		color.stage[0].numOps = 1;
		color.stage[0].op[0].op = LOAD;
		color.stage[0].op[0].param1 = SHADE;

		alpha.numStages = 1;
		alpha.stage[0].numOps = 1;
		alpha.stage[0].op[0].op = LOAD;
		alpha.stage[0].op[0].param1 = ONE;
	}
	else if ((RDP.otherMode_H & CYCLETYPE) == CYCLETYPE_COPY)
	{
		color.numStages = 1;
		color.stage[0].numOps = 1;
		color.stage[0].op[0].op = LOAD;
		color.stage[0].op[0].param1 = TEXTURE0;

		alpha.numStages = 1;
		alpha.stage[0].numOps = 1;
		alpha.stage[0].op[0].op = LOAD;
		alpha.stage[0].op[0].param1 = T0ALPHA;
	}
	else
	{
		if ((RDP.otherMode_H & CYCLETYPE) == CYCLETYPE_2CYCLE)
		{
			numCycles = 2;
			color.numStages = 2;
			alpha.numStages = 2;
		}
		else
		{
			numCycles = 1;
			color.numStages = 1;
			alpha.numStages = 1;
		}

		CombinerCycle cc[2];
		CombinerCycle ac[2];

		// Decode and expand the combine mode into a more general form
		cc[0].A = CCExpandA[(RDP.combine0 & 0x00F00000) >> 20];
		cc[0].B = CCExpandB[(RDP.combine1 & 0xF0000000) >> 28];
		cc[0].C = CCExpandC[(RDP.combine0 & 0x000F8000) >> 15];
		cc[0].D = CCExpandD[(RDP.combine1 & 0x00038000) >> 15];

		cc[1].A = CCExpandA[(RDP.combine0 & 0x000001E0) >> 5];
		cc[1].B = CCExpandB[(RDP.combine1 & 0x0F000000) >> 24];
		cc[1].C = CCExpandC[(RDP.combine0 & 0x0000001F)];
		cc[1].D = CCExpandD[(RDP.combine1 & 0x000001C0) >> 6];

		ac[0].A = ACExpandA[(RDP.combine0 & 0x00007000) >> 12];
		ac[0].B = ACExpandB[(RDP.combine1 & 0x00007000) >> 12];
		ac[0].C = ACExpandC[(RDP.combine0 & 0x00000E00) >>  9];
		ac[0].D = ACExpandD[(RDP.combine1 & 0x00000E00) >>  9];

		ac[1].A = ACExpandA[(RDP.combine1 & 0x00E00000) >> 20];
		ac[1].B = ACExpandB[(RDP.combine1 & 0x00000038) >>  3];
		ac[1].C = ACExpandC[(RDP.combine1 & 0x001C0000) >> 18];
		ac[1].D = ACExpandD[(RDP.combine1 & 0x00000007)];

		for (int i = 0; i < numCycles; i++)
		{
			// Simplify each RDP combiner cycle into a combiner stage
			Combiner_SimplifyCycle( &cc[i], &color.stage[i] );
			Combiner_SimplifyCycle( &ac[i], &alpha.stage[i] );
		}

		if (numCycles == 2)
		{
			// Attempt to merge the two stages into one
			Combiner_MergeStages( &color );
			Combiner_MergeStages( &alpha );
		}
	}

	// Send the simplified combiner to the hardware-specific compiler
	switch (OGL.combiner)
	{
		case ARB_TEXTURE_ENV_COMBINE:
			Set_ARB_texture_env_combine( &color, &alpha );
			break;
		case NV_REGISTER_COMBINERS:
			Set_NV_register_combiners( &color, &alpha );
			break;
	}
}

void Combiner_UpdateCombineMode()
{
	if (OGL.combiner != combiner.combiner)
	{
		combiner.combiner = OGL.combiner;
		Combiner_Init();
	}

	RDP.useT0 = FALSE;
	RDP.useT1 = FALSE;
	RDP.useNoise = FALSE;

	combiner.vertex.color = COMBINED;
	combiner.vertex.alpha = CMBALPHA;

	Combiner_GenerateMode(); // I should cache the recompiled mode

	Combiner_UpdateCombineColors();

	RSP.update &= ~UPDATE_COMBINE;
}