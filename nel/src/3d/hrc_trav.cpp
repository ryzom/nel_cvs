/** \file hrc_trav.cpp
 * <File description>
 *
 * $Id: hrc_trav.cpp,v 1.8 2002/02/28 12:59:49 besson Exp $
 */

/* Copyright, 2000 Nevrax Ltd.
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

#include "std3d.h"

#include "3d/hrc_trav.h"
#include "3d/skip_model.h"
#include "3d/clip_trav.h"
#include "3d/anim_detail_trav.h"

using namespace std;
using namespace NLMISC;


namespace	NL3D
{


// ***************************************************************************
IObs				*CHrcTrav::createDefaultObs() const
{
	return new CDefaultHrcObs;
}


// ***************************************************************************
void				CHrcTrav::setSkipModelRoot(CSkipModel *m)
{
	SkipModelRoot= m;
}


// ***************************************************************************
void IBaseHrcObs::init()
{
	IObs::init();

	ClipObs= safe_cast<IBaseClipObs*> (getObs(ClipTravId));
	AnimDetailObs= safe_cast<IBaseAnimDetailObs*> (getObs(AnimDetailTravId));

}


// ***************************************************************************
void	IBaseHrcObs::addParent(IObs *father)
{
	IObs	*oldFather= getFirstParent();

	// Default bheavior.
	IObs::addParent(father);

	// Our parent has changed ??
	IObs	*newFather= getFirstParent();
	if( oldFather != newFather )
	{
		// Since our parent has changed, must recompute WM.
		WorldDate=-1;
	}
}


}
