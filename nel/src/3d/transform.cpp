/** \file transform.cpp
 * <File description>
 *
 * $Id: transform.cpp,v 1.20 2001/07/05 09:38:49 besson Exp $
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

#include "3d/transform.h"
#include "3d/skeleton_model.h"


namespace	NL3D
{


// ***************************************************************************
void	CTransform::registerBasic()
{
	CMOT::registerModel( TransformId, 0, CTransform::creator);
	CMOT::registerObs( HrcTravId,			TransformId, CTransformHrcObs::creator			);
	CMOT::registerObs( ClipTravId,			TransformId, CTransformClipObs::creator			);
	CMOT::registerObs( AnimDetailTravId,	TransformId, CTransformAnimDetailObs::creator	);
	CMOT::registerObs( RenderTravId,		TransformId, CTransformRenderObs::creator		);
}


// ***************************************************************************
CTransform::CTransform()
{
	TouchObs.resize(Last);

	Visibility= CHrcTrav::Herit;

	_LastTransformableMatrixDate= 0;

	_FatherSkeletonModel= NULL;

	_Transparent = false;
	_Opaque = true;
}

// ***************************************************************************
CTransform::~CTransform()
{
	if(_FatherSkeletonModel)
	{
		// detach me from the skeleton.
		// Observers hierarchy is modified.
		_FatherSkeletonModel->detachSkeletonSon(this);
		_FatherSkeletonModel= NULL;
	}
}

// ***************************************************************************
void		CTransform::hide()
{
	// Optim: do nothing if already set (=> not foulTransform() -ed).
	if(Visibility!= CHrcTrav::Hide)
	{
		foulTransform();
		Visibility= CHrcTrav::Hide;
	}
}
// ***************************************************************************
void		CTransform::show()
{
	// Optim: do nothing if already set (=> not foulTransform() -ed).
	if(Visibility!= CHrcTrav::Show)
	{
		foulTransform();
		Visibility= CHrcTrav::Show;
	}
}
// ***************************************************************************
void		CTransform::heritVisibility()
{
	// Optim: do nothing if already set (=> not foulTransform() -ed).
	if(Visibility!= CHrcTrav::Herit)
	{
		foulTransform();
		Visibility= CHrcTrav::Herit;
	}
}


// ***************************************************************************
CTrackDefaultVector		CTransform::DefaultPos( CVector::Null );
CTrackDefaultVector		CTransform::DefaultRotEuler( CVector::Null );
CTrackDefaultQuat		CTransform::DefaultRotQuat( NLMISC::CQuat::Identity );
CTrackDefaultVector		CTransform::DefaultScale( CVector(1,1,1) );
CTrackDefaultVector		CTransform::DefaultPivot( CVector::Null );

ITrack* CTransform::getDefaultTrack (uint valueId)
{
	// Cyril: prefer do it here in CTransform, because of CCamera, CLight etc... (which may not need a default value too!!)

	// what value ?
	switch (valueId)
	{
	case PosValue:			return &DefaultPos;
	case RotEulerValue:		return &DefaultRotEuler;
	case RotQuatValue:		return &DefaultRotQuat;
	case ScaleValue:		return &DefaultScale;
	case PivotValue:		return &DefaultPivot;
	}

	// No, only ITrnasformable values!
	nlstop;
	// Deriver note: else call BaseClass::getDefaultTrack(valueId);

	return NULL;

}

// ***************************************************************************
void	CTransform::registerToChannelMixer(CChannelMixer *chanMixer, const std::string &prefix)
{
	// Hey!! we are animated!!
	_ChannelMixer= chanMixer;

	// For CTransfom, channels are not detailled.
	addValue(chanMixer, PosValue, OwnerBit, prefix, false);
	addValue(chanMixer, RotEulerValue, OwnerBit, prefix, false);
	addValue(chanMixer, RotQuatValue, OwnerBit, prefix, false);
	addValue(chanMixer, ScaleValue, OwnerBit, prefix, false);
	addValue(chanMixer, PivotValue, OwnerBit, prefix, false);

	// Deriver note: if necessary, call	BaseClass::registerToChannelMixer(chanMixer, prefix);
}



// ***************************************************************************
void			CTransform::updateWorldMatrixFromSkeleton(const CMatrix &parentWM)
{
	// Get the HrcObs.
	CTransformHrcObs	*hrcObs= (CTransformHrcObs*)getObs(HrcTravId);

	// Compute the HRC WorldMatrix.
	hrcObs->WorldMatrix= parentWM*hrcObs->LocalMatrix;
}



// ***************************************************************************
// ***************************************************************************
// Observers.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void	CTransformHrcObs::update()
{
	IBaseHrcObs::update();

	if(Model->TouchObs[CTransform::TransformDirty])
	{
		// update the local matrix.
		LocalMatrix= static_cast<CTransform*>(Model)->getMatrix();
		IBaseHrcObs::LocalVis= static_cast<CTransform*>(Model)->Visibility;
		// update the date of the local matrix.
		LocalDate= static_cast<CHrcTrav*>(Trav)->CurrentDate;
	}
}


// ***************************************************************************
void	CTransformHrcObs::updateWorld(IBaseHrcObs *caller)
{
	const	CMatrix		*pFatherWM;
	bool				visFather;


	// If not root case, link to caller.
	if(caller)
	{
		pFatherWM= &(caller->WorldMatrix);
		visFather= caller->WorldVis;
	}
	// else, default!!
	else
	{
		pFatherWM= &(CMatrix::Identity);
		visFather= true;
	}

	// Combine matrix
	if(LocalDate>WorldDate || (caller && caller->WorldDate>WorldDate) )
	{
		// Must recompute the world matrix.  ONLY IF I AM NOT SKINNED/STICKED TO A SKELETON!
		if( ((CTransform*)Model)->_FatherSkeletonModel==NULL)
		{
			WorldMatrix=  *pFatherWM * LocalMatrix;
			WorldDate= static_cast<CHrcTrav*>(Trav)->CurrentDate;
		}
	}

	// Combine visibility.
	switch(LocalVis)
	{
		case CHrcTrav::Herit: WorldVis= visFather; break;
		case CHrcTrav::Hide: WorldVis= false; break;
		case CHrcTrav::Show: WorldVis= true; break;
	}
}

// ***************************************************************************
void	CTransformHrcObs::traverse(IObs *caller)
{
	// Recompute the matrix, according to caller matrix mode, and local matrix.
	nlassert(!caller || dynamic_cast<IBaseHrcObs*>(caller));
	updateWorld(static_cast<IBaseHrcObs*>(caller));
	// DoIt the sons.
	traverseSons();
}


// ***************************************************************************
void	CTransformClipObs::traverse(IObs *caller)
{
	nlassert(!caller || dynamic_cast<IBaseClipObs*>(caller));

	Visible= false;

	// If linked to a SkeletonModel, don't clip, and use skeleton model clip result.
	// This works because we are sons of the SkeletonModel in the Clip traversal...
	bool	skeletonClip= false;
	if( ((CTransform*)Model)->_FatherSkeletonModel!=NULL )
	{
		skeletonClip= static_cast<IBaseClipObs*>(caller)->Visible;
	}

	// Test visibility or clip.
	if(HrcObs->WorldVis && ( skeletonClip  ||  clip( static_cast<IBaseClipObs*>(caller)) )  )
	{
		Visible= true;

		// Insert the model in the render list.
		if(isRenderable())
		{
			nlassert(dynamic_cast<CClipTrav*>(Trav));
			static_cast<CClipTrav*>(Trav)->RenderTrav->addRenderObs(RenderObs);
		}
	}

	// DoIt the sons.
	traverseSons();
}


// ***************************************************************************
void	CTransformAnimDetailObs::traverse(IObs *caller)
{
	// AnimDetail behavior: animate only if not clipped.
	if(ClipObs->Visible)
	{
		// test if the refptr is NULL or not (RefPtr).
		CChannelMixer	*chanmix= static_cast<CTransform*>(Model)->_ChannelMixer;
		if(chanmix)
		{
			// eval detail!!
			chanmix->eval(true, static_cast<CAnimDetailTrav*>(Trav)->CurrentDate);
		}
	}

	// important for the root only. Else, There is no reason to do a hierarchy for AnimDetail.
	traverseSons();
}



}
