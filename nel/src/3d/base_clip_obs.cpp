/** \file base_clip_obs.cpp
 * <File description>
 *
 * $Id: base_clip_obs.cpp,v 1.2 2003/03/20 15:01:24 berenguier Exp $
 */

/* Copyright, 2000-2002 Nevrax Ltd.
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

#include "nel/misc/debug.h"
#include "3d/base_clip_obs.h"
#include "3d/clip_trav.h"
#include "3d/render_trav.h"
#include "3d/anim_detail_trav.h"
#include "3d/load_balancing_trav.h"


using namespace std;
using namespace NLMISC;


namespace NL3D
{


// ***************************************************************************
// ***************************************************************************
// IBaseClipObs
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
IBaseClipObs::~IBaseClipObs()
{
	// I must remove me from _VisibleList.
	if(_IndexInVisibleList>=0)
	{
		CClipTrav	*clipTrav= (CClipTrav*) Trav;
		nlassert(_IndexInVisibleList < (sint)clipTrav->_VisibleList.size() );
		// Mark NULL. NB: faster than a CRefPtr.
		clipTrav->_VisibleList[_IndexInVisibleList]= NULL;
		_IndexInVisibleList= -1;
	}
}


// ***************************************************************************
void IBaseClipObs::init()
{
	IObs::init();
	nlassert( dynamic_cast<IBaseHrcObs*> (getObs(HrcTravId)) );
	HrcObs= static_cast<IBaseHrcObs*> (getObs(HrcTravId));
	nlassert( dynamic_cast<IBaseRenderObs*> (getObs(RenderTravId)) );
	RenderObs= static_cast<IBaseRenderObs*> (getObs(RenderTravId));

	AnimDetailObs= safe_cast<IBaseAnimDetailObs*> (getObs(AnimDetailTravId));
	LoadBalancingObs= safe_cast<IBaseLoadBalancingObs*> (getObs(LoadBalancingTravId));

}


} // NL3D
