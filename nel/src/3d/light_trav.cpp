/* light_trav.cpp
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: light_trav.cpp,v 1.2 2000/10/09 14:53:44 lecroart Exp $
 *
 * <Replace this by a description of the file>
 */

#include "nel/3d/light_trav.h"
#include "nel/3d/hrc_trav.h"
#include "nel/3d/clip_trav.h"
using namespace std;
using namespace NLMISC;


namespace	NL3D
{

using namespace	NLMISC;

// ***************************************************************************
CLightTrav::CLightTrav()
{
}

// ***************************************************************************
IObs		*CLightTrav::createDefaultObs() const
{
	return	new CDefaultLightObs;
}



// ***************************************************************************
void		IBaseLightObs::init()
{
	IObs::init();
	nlassert( dynamic_cast<IBaseHrcObs*> (getObs(HrcTravId)) );
	HrcObs= static_cast<IBaseHrcObs*> (getObs(HrcTravId));
	nlassert( dynamic_cast<IBaseClipObs*> (getObs(ClipTravId)) );
	ClipObs= static_cast<IBaseClipObs*> (getObs(ClipTravId));
}


}