/** \file particle_system_manager.h
 * <File description>
 *
 * $Id: particle_system_manager.h,v 1.2 2003/03/04 18:16:30 vizerie Exp $
 */

/* Copyright, 2000 - 2002 Nevrax Ltd.
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

#ifndef NL_PARTICLE_SYSTEM_MANAGER_H
#define NL_PARTICLE_SYSTEM_MANAGER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/plane.h"
#include "nel/3d/animation_time.h"


#include <list>
#include <vector>

namespace NLMISC
{
	class CVector;
}

namespace NL3D {


class CParticleSystemModel;

/**
 * This class list all the particle systems that have resources allocated at a given time, so that 
 * we can remove the resource of those who are too far from the viewer.
 * This has been added because objects that are in a cluster that is not 
 * visible are not parsed during traversals, so their resources couldn't just be released
 * Only a few systems are parsed each time (because they can be numerous).
 * Call to that class should be made by CScene, so you don't need to use it
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2002
 */
class CParticleSystemManager
{
public:
	enum { NumProcessToRefresh  = 3 }; // the number of systems that are refreshed at each call

	/// default ctor
	CParticleSystemManager();	

	/// call this to refresh systems. (check those whose data should be released)
	void	refreshModels(const std::vector<NLMISC::CPlane>	&worldFrustumPyramid,  const NLMISC::CVector &viewerPos);

	/// perform animation on systems that should be animated even if not parsed (temporary spells for example)
	void	processAnimate(TAnimationTime deltaT);

private:
	friend class CParticleSystemModel;
	friend class CParticleSystemClipObs;
	
	typedef std::list<CParticleSystemModel *> TModelList;

	struct TModelHandle
	{
		TModelList::iterator Iter;
		bool				 Valid;
		TModelHandle() : Valid(false) {}
	};

	
	/** Should be called when a new p.s. model has resources attached to it.
	  * \see removeSystemModel
	  */
	TModelHandle addSystemModel(CParticleSystemModel *);

	/// Should called when a p.s model has resources detached from it.
	void			removeSystemModel(TModelHandle &handle);

	/// Should be called to attach a system that must always be animated
	TModelHandle addPermanentlyAnimatedSystem(CParticleSystemModel *);

	/// Remove a permanenlty animated system
	void			removePermanentlyAnimatedSystem(TModelHandle &handle);
	
private:

	TModelList::iterator _CurrListIterator; /// the current element being processed
	TModelList			 _ModelList;
	TModelList			 _PermanentlyAnimatedModelList;
	uint			     _NumModels;
};


} // NL3D


#endif // NL_PARTICLE_SYSTEM_MANAGER_H

/* End of particle_system_manager.h */
