/** \file root_model.h
 * <File description>
 *
 * $Id: root_model.h,v 1.2 2002/06/26 16:48:58 berenguier Exp $
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

#ifndef NL_ROOT_MODEL_H
#define NL_ROOT_MODEL_H

#include "nel/misc/types_nl.h"
#include "3d/mot.h"
#include "3d/hrc_trav.h"
#include "3d/clip_trav.h"
#include "3d/anim_detail_trav.h"
#include "3d/load_balancing_trav.h"
#include "3d/light_trav.h"
#include "3d/render_trav.h"


namespace NL3D {


// ***************************************************************************
// ClassIds.
const NLMISC::CClassId		RootModelId=NLMISC::CClassId(0x25f0505d, 0x75c69f9);


// ***************************************************************************
/**
 * The purpose of this model is to have Observers which do nothing in traverse, but "traverseSons".
 *	This is made only for Hrc, Clip, AnimDetail, LoadBalancing, Lighting and render Trav.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CRootModel : public IModel
{
public:
	/// Call at the begining of the program, to register the model, and the basic observers.
	static	void	registerBasic();


	/// Direct accessor to the observers.
	IObs			*getObs(const NLMISC::CClassId &idTrav);


protected:
	/// Constructor
	CRootModel() {}
	/// Destructor
	virtual ~CRootModel() {}

private:
	static IModel	*creator() {return new CRootModel;}
	friend class	CRootModelHrcObs;
	friend class	CRootModelClipObs;
	friend class	CRootModelAnimDetailObs;
	friend class	CRootModelLoadBalancingObs;
	friend class	CRootModelLightObs;
	friend class	CRootModelRenderObs;

};


// ***************************************************************************
class	CRootModelHrcObs : public IBaseHrcObs
{
public:
	CRootModelHrcObs() {}

	virtual	void	traverse(IObs *caller);

	static IObs	*creator() {return new CRootModelHrcObs;}

};
// ***************************************************************************
class	CRootModelClipObs : public IBaseClipObs
{
public:
	CRootModelClipObs() {}

	virtual	bool	clip(IBaseClipObs *caller)	{return true;}
	virtual	void	traverse(IObs *caller);

	static IObs	*creator() {return new CRootModelClipObs;}

};
// ***************************************************************************
class	CRootModelAnimDetailObs : public IBaseAnimDetailObs
{
public:
	CRootModelAnimDetailObs() {}

	virtual	void	traverse(IObs *caller);

	static IObs	*creator() {return new CRootModelAnimDetailObs;}

};
// ***************************************************************************
class	CRootModelLoadBalancingObs : public IBaseLoadBalancingObs
{
public:
	CRootModelLoadBalancingObs() {}

	virtual	void	traverse(IObs *caller);

	static IObs	*creator() {return new CRootModelLoadBalancingObs;}

};
// ***************************************************************************
class	CRootModelLightObs : public IBaseLightObs
{
public:
	CRootModelLightObs() {}

	virtual	void	traverse(IObs *caller);

	static IObs	*creator() {return new CRootModelLightObs;}

};
// ***************************************************************************
class	CRootModelRenderObs : public IBaseRenderObs
{
public:
	CRootModelRenderObs() {}

	virtual	void	traverse(IObs *caller);

	static IObs	*creator() {return new CRootModelRenderObs;}

};


} // NL3D


#endif // NL_ROOT_MODEL_H

/* End of root_model.h */
