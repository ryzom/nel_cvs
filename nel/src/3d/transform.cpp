/** \file transform.cpp
 * <File description>
 *
 * $Id: transform.cpp,v 1.25 2001/08/28 11:44:22 berenguier Exp $
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
#include "3d/skip_model.h"


using namespace NLMISC;


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

	_ClusterSystem = NULL;

	_FreezeHRCState= FreezeHRCStateDisabled;

	_QuadGridClipManagerEnabled= false;
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
void			CTransform::freeze()
{
	// First, validate the model, and all his observers.
	// Frozen state is disabled here (in CTransformHrcObs::update()).
	validate();

	// Then flag the frozen state.
	// Get the HrcObs.
	CTransformHrcObs	*hrcObs= (CTransformHrcObs*)getObs(HrcTravId);
	hrcObs->Frozen= true;
}

// ***************************************************************************
void			CTransform::setDontUnfreezeChildren(bool val)
{
	CTransformHrcObs	*hrcObs= (CTransformHrcObs*)getObs(HrcTravId);
	hrcObs->DontUnfreezeChildren = val;
}


// ***************************************************************************
const CMatrix& CTransform::getWorldMatrix()
{
	CTransformHrcObs *pObs = (CTransformHrcObs*)getObs(HrcTravId);
	return pObs->WorldMatrix;
}


// ***************************************************************************
void		CTransform::freezeHRC()
{
	// if disabled, say we are ready to validate our worldMatrix for long.
	if(_FreezeHRCState==FreezeHRCStateDisabled)
	{
		_FreezeHRCState= FreezeHRCStateRequest;
		_QuadGridClipManagerEnabled= true;
	}
}


// ***************************************************************************
void		CTransform::unfreezeHRC()
{
	// if this model is no HRC frozen disabled
	if(_FreezeHRCState!=FreezeHRCStateDisabled)
	{
		// if model correctly frozen.
		if(_FreezeHRCState == CTransform::FreezeHRCStateEnabled )
		{
			// Trick: get the traversal via the HrcObs.
			IObs		*hrcObs= getObs(HrcTravId);
			CHrcTrav	*hrcTrav= static_cast<CHrcTrav*>(hrcObs->Trav);
			// if linked to SkipModelRoot, link this model to root of HRC.
			if( hrcTrav->getFirstParent(this) == hrcTrav->SkipModelRoot )
				hrcTrav->link(NULL, this);

			// Link this object to the validateList.
			linkToValidateList();
		}

		_FreezeHRCState= FreezeHRCStateDisabled;
		_QuadGridClipManagerEnabled= false;
	}
}


// ***************************************************************************
void		CTransform::update()
{
	IModel::update();

	// test if the matrix has been changed in ITransformable.
	if(ITransformable::compareMatrixDate(_LastTransformableMatrixDate))
	{
		_LastTransformableMatrixDate= ITransformable::getMatrixDate();
		foul(TransformDirty);
	}

	// update the freezeHRC state.
	if(_FreezeHRCState != CTransform::FreezeHRCStateDisabled)
	{
		// if the model request to be frozen in HRC
		if(_FreezeHRCState == CTransform::FreezeHRCStateRequest )
		{
			// Wait for next Hrc traversal to compute good WorldMatrix for this model and his sons.
			_FreezeHRCState = CTransform::FreezeHRCStateReady;
		}
		// if the model is ready to be frozen in HRC, then do it!!
		else if( _FreezeHRCState == CTransform::FreezeHRCStateReady )
		{
			// Trick: get the traversal via the HrcObs.
			IObs		*hrcObs= getObs(HrcTravId);
			CHrcTrav	*hrcTrav= static_cast<CHrcTrav*>(hrcObs->Trav);
			// if linked to root of HRC, link this model to SkipModelRoot.
			if( hrcTrav->getFirstParent(this) == hrcTrav->getRoot() )
				hrcTrav->link(hrcTrav->SkipModelRoot, this);

			// unLink this object from the validateList. NB: the list will still be correclty parsed.
			unlinkFromValidateList();

			// Now this model won't be tested for validation nor for worldMatrix update. End!!
			_FreezeHRCState = CTransform::FreezeHRCStateEnabled;
		}
	}
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

		// The transform has been modified. Hence, it is no more frozen.
		Frozen= false;
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

		// If father is not Frozen, so do I.
		CTransformHrcObs	*hrcTransCaller= dynamic_cast<CTransformHrcObs*>(caller);

		// if caller is a CTransformHrcObs
		//  and if it is not frozen (for any reason), disable us!

		if (hrcTransCaller && !hrcTransCaller->Frozen && !hrcTransCaller->DontUnfreezeChildren)
			Frozen= false;
	}
	// else, default!!
	else
	{
		pFatherWM= &(CMatrix::Identity);
		visFather= true;

		// NB: Root is Frozen by essence :), so don't modify the frozen state here.
	}

	// Combine matrix
	if(LocalDate>WorldDate || (caller && caller->WorldDate>WorldDate) )
	{
		// Must recompute the world matrix.  ONLY IF I AM NOT SKINNED/STICKED TO A SKELETON!
		if( ((CTransform*)Model)->_FatherSkeletonModel==NULL)
		{
			WorldMatrix=  *pFatherWM * LocalMatrix;
			WorldDate= static_cast<CHrcTrav*>(Trav)->CurrentDate;

			// Add the model to the moving object list
			if (!Frozen)
				static_cast<CHrcTrav*>(Trav)->_MovingObjects.push_back (Model);
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

	CClipTrav		*clipTrav= safe_cast<CClipTrav*>(Trav);
	IBaseClipObs	*callerClipObs= static_cast<IBaseClipObs*>(caller);

	if ((Date == clipTrav->CurrentDate) && Visible)
		return;
	Date = clipTrav->CurrentDate;

	// clip: update Visible flag.
	Visible= false;
	// if at least visible.
	if(HrcObs->WorldVis)
	{
		if(clipTrav->ForceNoFrustumClip)
		{
			Visible= true;
		}
		else
		{
			// If linked to a SkeletonModel, don't clip, and use skeleton model clip result.
			// This works because we are sons of the SkeletonModel in the Clip traversal...
			bool	skeletonClip= false;
			if( ((CTransform*)Model)->_FatherSkeletonModel!=NULL )
				skeletonClip= callerClipObs->Visible;

			// clip.
			if( skeletonClip  ||  clip(callerClipObs)  )
				Visible= true;
		}
	}

	// if visible, add to list.
	if(Visible)
	{
		// add this observer to the visibility list.
		clipTrav->addVisibleObs(this);

		// Insert the model in the render list.
		if(isRenderable())
		{
			clipTrav->RenderTrav->addRenderObs(RenderObs);
		}
	}

	// DoIt the sons.
	traverseSons();
}


// ***************************************************************************
void	CTransformAnimDetailObs::traverse(IObs *caller)
{
	// AnimDetail behavior: animate only if not clipped.
	// NB: no need to test because of VisibilityList use.

	// test if the refptr is NULL or not (RefPtr).
	CChannelMixer	*chanmix= static_cast<CTransform*>(Model)->_ChannelMixer;
	if(chanmix)
	{
		// eval detail!!
		chanmix->eval(true, static_cast<CAnimDetailTrav*>(Trav)->CurrentDate);
	}

	// no need to traverseSons. No graph here.
}



}
