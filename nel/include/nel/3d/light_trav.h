/* light_trav.h
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: light_trav.h,v 1.2 2000/10/10 16:12:04 berenguier Exp $
 *
 * <Replace this by a description of the file>
 */

#ifndef NL_LIGHT_TRAV_H
#define NL_LIGHT_TRAV_H

#include "nel/3d/trav_scene.h"


namespace	NL3D
{


using namespace NLMISC;

	
class	IBaseHrcObs;
class	IBaseClipObs;


// ***************************************************************************
// ClassIds.
const NLMISC::CClassId		LightTravId= NLMISC::CClassId(0x328f500a, 0x57600db9);


// ***************************************************************************
/**
 * The light traversal.
 * The purpose of this traversal is to compute lighting on lightable objects.
 *
 * TODO.
 *
 * NB: see CScene for 3d conventions (orthonormal basis...)
 * \sa CScene IBaseLightObs
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CLightTrav : public ITravScene
{
public:

	/// Constructor
	CLightTrav();

	// ITrav/ITravScene Implementation.
	IObs				*createDefaultObs() const;
	NLMISC::CClassId	getClassId() const {return LightTravId;}
	sint				getRenderOrder() const {return 3000;}
	void				traverse()
	{
		if(Root)
			Root->traverse(NULL);
	}

};



// ***************************************************************************
/**
 * The base interface for clip traversal.
 * Clip observers MUST derive from IBaseLightObs.
 * This observer:
 * - leave the notification system to DO NOTHING.
 * - leave the traverse() method as undefined.
 * - implement the init() method, to set shortcut to neighboor observers.
 *
 * \b DERIVER \b RULES:
 * - implement the notification system (see IObs and IObs() for details).
 * - implement the traverse() method.
 * - possibly modify/extend the graph methods (such as a graph behavior).
 *
 * \sa CLightTrav
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class IBaseLightObs : public IObs
{
public:
	/// Shortcut to observers.
	IBaseHrcObs		*HrcObs;
	IBaseClipObs	*ClipObs;
	

public:

	/// Constructor.
	IBaseLightObs()
	{
		HrcObs=NULL;
		ClipObs= NULL;
	}
	/// Build shortcut to HrcObs and RenderObs.
	virtual	void	init();
};


// ***************************************************************************
/**
 * The base interface for clip traversal.
 * Clip observers MUST derive from IBaseLightObs.
 * This observer:
 * - leave the notification system to DO NOTHING.
 * - implement the traverse() method to DO NOTHING (but update() and traverseSons()).
 *
 * \sa CLightTrav
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CDefaultLightObs : public IBaseLightObs
{
public:
	void	traverse(IObs * caller)
	{
		update();
		traverseSons();
	}
};


// For Visual 6.0 namespace bug.
namespace MSC_FAKE
{
using namespace	NLMISC;
}


}


#endif // NL_LIGHT_TRAV_H

/* End of light_trav.h */
