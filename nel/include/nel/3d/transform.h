/** \file transform.h
 * <File description>
 *
 * $Id: transform.h,v 1.9 2001/02/28 14:21:00 berenguier Exp $
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

#include "nel/3d/mot.h"
#include "nel/3d/hrc_trav.h"
#include "nel/3d/clip_trav.h"
#include "nel/3d/track.h"
#include "nel/3d/animatable.h"
#include "nel/3d/animated_value.h"
#include "nel/misc/matrix.h"


namespace	NL3D
{


using NLMISC::CVector;
using NLMISC::CPlane;
using NLMISC::CMatrix;


class	CTransformHrcObs;
class	CTransformClipObs;


// ***************************************************************************
// ClassIds.
const NLMISC::CClassId		TransformId=NLMISC::CClassId(0x174750cb, 0xf952024);


// ***************************************************************************
/**
 * A basic node which provide a matrix.
 * May be derived for each node who want to support such a scheme (CCamera, CLight, CInstance ... )
 *
 * No observer is provided for LightTrav and RenderTrav (not lightable, nor renderable => use default).
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CTransform : public IModel, IAnimatable
{
public:
	/// Call at the begining of the program, to register the model, and the basic observers.
	static	void	registerBasic();

public:


	/// \name Direct Matrix operations.
	//@{
	void			setMatrix(const CMatrix &mat);
	const CMatrix	&getMatrix() const	{return LocalMatrix;}
	//@}


	/// Hide the object and his sons.
	void		hide();
	/// Show the objet and his sons.
	void		show();
	/// herit the visibility from his father. (default behavior).
	void		heritVisibility();
	/// Get the local visibility state.
	CHrcTrav::TVisibility	getVisibility() {return Visibility;}


	/// \name Misc
	// @{
	/** 
	  * Setup Matrix by the lookAt method.
	  * 
	  * \param eye is the coordinate of the object.
	  * \param target is the point the object look at.
	  * \param roll is the roll angle in radian along the object's Y axis.
	  */
	void		lookAt (const CVector& eye, const CVector& target, float roll=0.f);
	// @}


public:
	/// \name Herited from IAnimatable

	/// From IAnimatable
	virtual uint getValueCount () const;

	/// From IAnimatable
	virtual IAnimatedValue* getValue (uint valueId);

	/// From IAnimatable
	virtual const std::string& getValueName (uint valueId) const;

	/// From IAnimatable
	virtual ITrack* getDefaultTrack (uint valueId);

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
	CMatrix					LocalMatrix;


	void	foul()
	{
		IModel::foul();
		Touch.set(TransformDirty);
	}

protected:
	/// Constructor
	CTransform();
	/// Destructor
	virtual ~CTransform() {}

	/// Implement the clean method.
	virtual void	clean()
	{
		IModel::clean();
		// Clean up the model.
		Touch.clear(TransformDirty);
	}

private:
	static IModel	*creator() {return new CTransform;}
	static const std::string CTransform::valueNames [];
	friend class	CTransformHrcObs;
	friend class	CTransformClipObs;

	// For animation, Pos, rot scale pivot animated values
	CAnimatedValueVector	_Pos;
	CAnimatedValueVector	_RotEuler;
	CAnimatedValueQuat		_RotQuat;
	CAnimatedValueVector	_Scale;
	CAnimatedValueVector	_Pivot;

protected:
	// For animation, default tracks pointers, must be set by the derived model.
	CTrackDefaultVector		*_PosDefault;
	CTrackDefaultVector		*_RotEulerDefault;
	CTrackDefaultQuat		*_RotQuatDefault;
	CTrackDefaultVector		*_ScaleDefault;
	CTrackDefaultVector		*_PivotDefault;
};


// ***************************************************************************
/**
 * This observer:
 * - implement the notification system (just the clean() method).
 *
 * \sa CHrcTrav IBaseHrcObs
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class	CTransformHrcObs : public IBaseHrcObs
{
public:


	virtual	void	clean()
	{
		IBaseHrcObs::clean();

		if(Touch[CTransform::TransformDirty])
		{
			Touch.clear(CTransform::TransformDirty);
			// update the local matrix.
			LocalMatrix= static_cast<CTransform*>(Model)->LocalMatrix;
			IBaseHrcObs::LocalVis= static_cast<CTransform*>(Model)->Visibility;
			// update the date of the local matrix.
			updateLocal();
		}
	}


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

	/// Don't clip, but don't render.
	virtual	bool	clip(IBaseClipObs *caller, bool &renderable) 
	{
		renderable= false; 
		return true;
	}

};



}


#endif // NL_TRANSFORM_H

/* End of transform.h */
