/** \file skip_model.h
 * <File description>
 *
 * $Id: skip_model.h,v 1.2 2002/07/08 10:00:09 berenguier Exp $
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

#ifndef NL_SKIP_MODEL_H
#define NL_SKIP_MODEL_H

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
const NLMISC::CClassId		SkipModelId=NLMISC::CClassId(0x143f5849, 0x2847496e);


// ***************************************************************************
/**
 * The purpose of this model is to have Observers which do nothing in traverse, even not "traverseSons".
 *	This is made only for Hrc, Clip, AnimDetail, LoadBalancing, Lighting and render Trav.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CSkipModel : public IModel
{
public:
	/// Call at the begining of the program, to register the model, and the basic observers.
	static	void	registerBasic();


protected:
	/// Constructor
	CSkipModel() {}
	/// Destructor
	virtual ~CSkipModel() {}

private:
	static IModel	*creator() {return new CSkipModel;}
	friend class	CSkipModelHrcObs;
	friend class	CSkipModelClipObs;
	friend class	CSkipModelAnimDetailObs;
	friend class	CSkipModelLoadBalancingObs;
	friend class	CSkipModelLightObs;
	friend class	CSkipModelRenderObs;

};


// ***************************************************************************
class	CSkipModelHrcObs : public IBaseHrcObs
{
public:
	CSkipModelHrcObs() {}

	virtual	void	traverse(IObs *caller);

	static IObs	*creator() {return new CSkipModelHrcObs;}

};
// ***************************************************************************
class	CSkipModelClipObs : public IBaseClipObs
{
public:
	CSkipModelClipObs() {}

	virtual	bool	clip(IBaseClipObs *caller)	{return true;}
	virtual	void	traverse(IObs *caller);

	static IObs	*creator() {return new CSkipModelClipObs;}

};
// ***************************************************************************
class	CSkipModelAnimDetailObs : public IBaseAnimDetailObs
{
public:
	CSkipModelAnimDetailObs() {}

	virtual	void	traverse(IObs *caller);

	static IObs	*creator() {return new CSkipModelAnimDetailObs;}

};
// ***************************************************************************
class	CSkipModelLoadBalancingObs : public IBaseLoadBalancingObs
{
public:
	CSkipModelLoadBalancingObs() {}

	virtual	void	traverse(IObs *caller);

	static IObs	*creator() {return new CSkipModelLoadBalancingObs;}

};
// ***************************************************************************
class	CSkipModelLightObs : public IBaseLightObs
{
public:
	CSkipModelLightObs() {}

	virtual	void	traverse(IObs *caller);

	static IObs	*creator() {return new CSkipModelLightObs;}

};
// ***************************************************************************
class	CSkipModelRenderObs : public IBaseRenderObs
{
public:
	CSkipModelRenderObs() {}

	virtual	void	traverse(IObs *caller);

	static IObs	*creator() {return new CSkipModelRenderObs;}

};


} // NL3D


#endif // NL_SKIP_MODEL_H

/* End of skip_model.h */
