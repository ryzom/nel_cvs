/** \file global_retriever.cpp
 *
 *
 * $Id: global_retriever.cpp,v 1.1 2001/05/10 12:19:02 legros Exp $
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

#include <vector>

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"

#include "nel/misc/debug.h"

#include "nel/pacs/global_retriever.h"
#include "nel/pacs/retriever_bank.h"

using namespace std;
using namespace NLMISC;

NLPACS::CRetrieverInstance	&NLPACS::CGlobalRetriever::getInstanceFullAccess(const CVector &position)
{
	CVector			offset = _BBox.getMin()-position;
	const float		zdim = 160.0f;
	return getInstanceFullAccess((uint)(offset.x/zdim), (uint)(offset.y/zdim));
}

void	NLPACS::CGlobalRetriever::serial(NLMISC::IStream &f)
{
	f.serialCont(_Instances);
	f.serial(_Width, _Height);
	f.serial(_BBox);
}

void	NLPACS::CGlobalRetriever::makeLinks(uint n)
{
	uint	x, y;
	convertId(n, x, y);

	if (x > 0 && _Instances[n-1].getInstanceId() >= 0)
	{
		_Instances[n].link(_Instances[n-1], 0, _RetrieverBank->getRetrievers());
		_Instances[n-1].link(_Instances[n], 2, _RetrieverBank->getRetrievers());
	}

	if (y < (uint)(_Height-1) && _Instances[n-_Width].getInstanceId() >= 0)
	{
		_Instances[n].link(_Instances[n-_Width], 1, _RetrieverBank->getRetrievers());
		_Instances[n-_Width].link(_Instances[n], 3, _RetrieverBank->getRetrievers());
	}

	if (x < (uint)(_Width-1) && _Instances[n+1].getInstanceId() >= 0)
	{
		_Instances[n].link(_Instances[n+1], 2, _RetrieverBank->getRetrievers());
		_Instances[n+1].link(_Instances[n], 0, _RetrieverBank->getRetrievers());
	}

	if (y > 0 && _Instances[n+_Width].getInstanceId() >= 0)
	{
		_Instances[n].link(_Instances[n+_Width], 3, _RetrieverBank->getRetrievers());
		_Instances[n+_Width].link(_Instances[n], 1, _RetrieverBank->getRetrievers());
	}
}

void	NLPACS::CGlobalRetriever::resetAllLinks()
{
	uint	n;
	for (n=0; n<_Instances.size(); ++n)
		_Instances[n].unlink(_Instances);
}

void	NLPACS::CGlobalRetriever::makeAllLinks()
{
	resetAllLinks();

	uint	n;
	for (n=0; n<_Instances.size(); ++n)
		makeLinks(n);
}

NLPACS::CGlobalRetriever::CGlobalPosition	NLPACS::CGlobalRetriever::retrievePosition(const CVector &estimated)
{
	CRetrieverInstance	&instance = getInstanceFullAccess(estimated);
	CLocalRetriever::CLocalPosition	localPosition = instance.retrievePosition(estimated, _RetrieverBank->getRetriever(instance.getRetrieverId()));
	return CGlobalPosition(instance.getInstanceId(), localPosition);
}

CVectorD	NLPACS::CGlobalRetriever::getGlobalPosition(const NLPACS::CGlobalRetriever::CGlobalPosition &global) const
{
	return _Instances[global.InstanceId].getDoubleGlobalPosition(global.LocalPosition.Estimation);
}

CVector		NLPACS::CGlobalRetriever::getInstanceCenter(uint x, uint y) const
{
	const float	zdim = 160.0f;
	CVector	bmin = _BBox.getMin();
	return CVector(bmin.x+zdim*((float)x+0.5f), bmin.y+zdim*((float)y+0.5f), 0.0f);
}