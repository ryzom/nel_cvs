/** \file retrievable_surface.cpp
 *
 *
 * $Id: retrievable_surface.cpp,v 1.10 2001/12/28 15:37:02 lecroart Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
 *
 * This file is part of NEVRAX NEL.
 * NEVRAX NEL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX NEL is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NEL; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "stdpacs.h"

#include "pacs/retrievable_surface.h"

using namespace std;
using namespace NLMISC;


float			NLPACS::Models[NumCreatureModels][NumModelCharacteristics] =
{
	{ 0.5f, 1.0f, -1.0f },
	{ 0.8f, 2.0f, -0.5f },
	{ 2.0f, 4.0f, +0.5f },
	{ 4.0f, 8.0f, +0.707f }
};

void	NLPACS::CRetrievableSurface::serial(IStream &f)
{
	/*
	Version 0:
		- base version.
	Version 1:
		- absolute water height and flag
	*/
	sint	ver= f.serialVersion(1);

	uint	i;
	f.serial(_NormalQuanta);
	f.serial(_OrientationQuanta);
	f.serial(_Material);
	f.serial(_Character);
	f.serial(_Level);
	f.serialCont(_Chains);
	f.serialCont(_Loops);
	f.serial(_Quad);
	for (i=0; i<NumCreatureModels; ++i)
		f.serial(_Topologies[i]);
	f.serial(_Center);
	f.serial(_IsFloor, _IsCeiling);
	f.serial(_Flags);

	if (ver >= 1)
	{
		f.serial(_WaterHeight);
	}
}

void	NLPACS::CRetrievableSurface::TLoop::serial(IStream &f)
{ 
	f.serialCont(*this); 
	f.serial(Length); 
}
