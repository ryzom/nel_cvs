/** \file transform.h
 * <File description>
 *
 * $Id: transform.h,v 1.1 2001/06/15 16:24:45 corvazier Exp $
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

#ifndef NL_TRANSFORM_H
#define NL_TRANSFORM_H

#include "3d/mot.h"
#include "3d/hrc_trav.h"
#include "3d/clip_trav.h"
#include "3d/render_trav.h"
#include "3d/track.h"
#include "3d/transformable.h"
#include "3d/animated_value.h"
#include "3d/anim_detail_trav.h"
#include "3d/channel_mixer.h"
#include "nel/misc/matrix.h"


namespace	NL3D
{


using NLMISC::CVector;
using NLMISC::CPlane;
using NLMISC::CMatrix;


class	CTransformHrcObs;
class	CTransformClipObs;
class	CSkeletonModel;
class	CSkeletonModelAnimDetailObs;

// ***************************************************************************
// ClassIds.
const NLMISC::CClassId		TransformId=NLMISC::CClassId(0x174750cb, 0xf952024);


// ***************************************************************************
/**
 * A basic node which provide an animatable matrix (ITransformable).
 * May be derived for each node who want to support such a scheme (CCamera, CLight, CInstance ... )
 *
 * CTransform ALWAYS herit scale from fathers! (joints skeleton may not...) (nbyoyo: else, this breaks the touch system with observers).
 *
 * CTransform Default tracks are identity (derived class may change this).
 *
 * No observer is provided for LightTrav and RenderTrav (not lightable, nor renderable => use default).
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CTransform : public IModel, public ITransformable
{
public:
	/// Call at the begining of the program, to register the model, and the basic observers.
	static	void	registerBasic();

public:


	/// Hide the object and his sons.
	void		hide();
	/// Show the objet and his sons.
	void		show();
	/// herit the visibility from his father. (default behavior).
	void		heritVisibility();
	/// Get the local visibility state.
	CHrcTrav::TVisibility	getVisibility() {return Visibility;}
	/// Get the skeleton model. Returnr NULL in normal mode.
	CSkeletonModel*			getSkeletonModel () const {return _FatherSkeletonModel;}


	/// \name Derived from ITransformable.
	// @{
	/// Default Track Values are identity (pos,pivot= 0, scale= 1, rots=0).
	virtual ITrack* getDefaultTrack (uint valueId);
	/// register transform channels (in global anim mode).
	virtual void	registerToChannelMixer(CChannelMixer *chanMixer, const std::string &prefix);
	// @}



// ********
private:
	// Add our own dirty states.
	enum	TDirty
	{
		TransformDirty= IModel::Last,	// The matrix or the visibility state is modified.
		Last
	};

private:
	CHrcTrav::TVisibility	Visibility;

	static	CTrackDefaultVector		DefaultPos;
	static	CTrackDefaultVector		DefaultPivot;
	static	CTrackDefaultVector		DefaultRotEuler;
	static	CTrackDefaultQuat		DefaultRotQuat;
	static	CTrackDefaultVector		DefaultScale;

	void	foulTransform()
	{
		IModel::foul(TransformDirty);
	}

protected:
	/// Constructor
	CTransform();
	/// Destructor
	virtual ~CTransform();

	/// Implement the update method.
	virtual void	update()
	{
		IModel::update();
		// test if the matrix has been changed in ITransformable.
		if(ITransformable::compareMatrixDate(_LastTransformableMatrixDate))
		{
			_LastTransformableMatrixDate= ITransformable::getMatrixDate();
			foul(TransformDirty);
		}
	}


	// The SkeletonModel, root of us (skinning or sticked object). NULL , if normal mode.
	CSkeletonModel	*_FatherSkeletonModel;


private:
	static IModel	*creator() {return new CTransform;}
	friend class	CTransformHrcObs;
	friend class	CTransformClipObs;
	friend class	CTransformAnimDetailObs;
	friend class	CSkeletonModel;
	friend class	CSkeletonModelAnimDetailObs;

	// For Skeleton Object Stick.
	void			updateWorldMatrixFromSkeleton(const CMatrix &parentWM);


	// For anim detail.
	NLMISC::CRefPtr<CChannelMixer>		_ChannelMixer;

	// Last date of ITransformable matrix.
	uint64			_LastTransformableMatrixDate;

};


// ***************************************************************************
/**
 * This observer:
 * - implement the notification system (just the update() method).
 * - implement the traverse() method.
 *
 * \sa CHrcTrav IBaseHrcObs
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class	CTransformHrcObs : public IBaseHrcObs
{
public:


	virtual	void	update()
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


	/// \name Utility methods.
	//@{
	/// Update the world state according to the parent world state and the local states.
	void	updateWorld(IBaseHrcObs *caller)
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
	//@}


	/// \name The base doit method.
	//@{
	/// The base behavior is to update() the observer, updateWorld() states, and traverseSons().
	virtual	void	traverse(IObs *caller)
	{
		// Recompute the matrix, according to caller matrix mode, and local matrix.
		nlassert(!caller || dynamic_cast<IBaseHrcObs*>(caller));
		updateWorld(static_cast<IBaseHrcObs*>(caller));
		// DoIt the sons.
		traverseSons();
	}
	//@}

};


// ***************************************************************************
/**
 * This observer:
 * - leave the notification system to DO NOTHING.
 * - implement the clip() method to return true (not renderable)
 * - leave the traverse() method as IBaseClipObs.
 *
 * \sa CHrcTrav IBaseClipObs
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class	CTransformClipObs : public IBaseClipObs
{
public:

	/// don't render.
	virtual	bool	isRenderable() const {return false;}

	/// Don't clip.
	virtual	bool	clip(IBaseClipObs *caller) 
	{
		return true;
	}

	/** The base doit method.
	 * The behavior is to:
	 *	- test if HrcObs->WorldVis is visible.
	 *	- test if the observer is clipped with clip() OR IF SKELETON MODEL, USE SKELETON MODEL clip!!
	 *	- if visible and not clipped, set \c Visible=true (else false).
	 *	- if Visible==true, and renderable, add it to the RenderTraversal: \c RenderTrav->addRenderObs(RenderObs);
	 *	- always traverseSons(), to clip the sons.
	 */
	virtual	void	traverse(IObs *caller)
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

};


// ***************************************************************************
/**
 * This observer:
 * - leave the notification system to DO NOTHING.
 * - implement the traverse method.
 *
 * \sa CHrcTrav IBaseHrcObs
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class	CTransformAnimDetailObs : public IBaseAnimDetailObs
{
public:

	/** this do all the good things:
	 *	- animdetail if the model channelmixer is not NULL, AND if model not clipped!!
	 *	- traverseSons().
	 */
	virtual	void	traverse(IObs *caller)
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

};


}


#endif // NL_TRANSFORM_H

/* End of transform.h */
