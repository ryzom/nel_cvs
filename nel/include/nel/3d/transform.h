/** \file transform.h
 * <File description>
 *
 * $Id: transform.h,v 1.4 2000/10/25 13:39:13 lecroart Exp $
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
 * A basic node which provide orientation / translation / scale.
 * May be derived for each node who want to support such a scheme (CCamera, CLight, CInstance ... )
 *
 * No observer is provided for LightTrav and RenderTrav (not lightable, nor renderable => use default).
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CTransform : public IModel
{
private:
	// Add our own dirty states.
	enum	TDirty
	{
		TransformDirty= IModel::Last,	// The matrix or the visibility state is modified.
		Last
	};

private:
	CHrcTrav::TVisibility	Visibility;
	CVector					Pos, Rot, Scale;
	CMatrix::TRotOrder		RotOrder;
	bool					PosRotScaleMode;
	CMatrix					LocalMatrix;
	bool					BadLocalMatrix;		// The posrotscale has been modified.


	void	foul()
	{
		IModel::foul();
		Touch.set(TransformDirty);
	}
	void	indirect()
	{
		PosRotScaleMode= true;
		BadLocalMatrix= true;
		foul();
	}

	// update the matrix (usefull in indirect mode).
	void	updateLocalMatrix()
	{
		if(BadLocalMatrix)
		{
			if(PosRotScaleMode)
			{
				// Recompute the local Matrix.
				LocalMatrix.identity();
				LocalMatrix.translate(Pos);
				LocalMatrix.rotate(Rot, RotOrder);
				LocalMatrix.scale(Scale);
			}
			BadLocalMatrix= false;
		}
	}

public:
	/// Call at the begining of the program, to register the model, and the basic observers.
	static	void	registerBasic();


public:

	/// Constructor
	CTransform();

	/** \name Pos/Rot/Scale Matrix operations.
	 * The set() method enter the transform in a indirect mode: the localmatrix is computed from the following method.
	 * The get() method work only in indirect mode.
	 */
	//@{
	/// Just reset the indirect mode to default.
	void		reset() { indirect(); LocalMatrix.identity(); Pos= Rot= CVector::Null; Scale.set(1,1,1); RotOrder= CMatrix::XYZ;}
	void		setPos(const CVector &pos) { indirect(); Pos= pos;}
	void		setRot(const CVector &rot) { indirect(); Rot= rot;}
	void		addPos(const CVector &pos) { indirect(); Pos+= pos;}
	void		addRot(const CVector &rot) { indirect(); Rot+= rot;}
	void		setScale(const CVector &scale) { indirect(); Scale= scale;}
	void		setRotOrder(CMatrix::TRotOrder order) { indirect(); RotOrder= order;}
	void		getPos(CVector &pos) const {pos= Pos;}
	void		getRot(CVector &rot) const {rot= Rot;}
	void		getScale(CVector &scale) const {scale= Scale;}
	void		getRotOrder(CMatrix::TRotOrder &order) const { order= RotOrder;}
	CVector		getPos() const {return Pos;}
	CVector		getRot() const {return Rot;}
	CVector		getScale() const {return Scale;}
	CMatrix::TRotOrder	getRotOrder() const {return RotOrder;}
	// TODO: set/get pivots, set/get shear.
	//@}


	/// \name Direct Matrix operations.
	//@{
	/** The setMatrix() enter the transform in a Direct mode: LocalMatrix= mat.
	 * The direct get*() methods won't work after this.
	 */
	void		setMatrix(const CMatrix &mat);
	void		getMatrix(CMatrix &mat) const;
	CMatrix		getMatrix() const
	{
		CMatrix	ret; getMatrix(ret); return ret;
	}
	//@}


	/// Hide the object and his sons.
	void		hide();
	/// Show the objet and his sons.
	void		show();
	/// herit the visibility from his father. (default behavior).
	void		heritVisibility();
	/// Get the local visibility state.
	CHrcTrav::TVisibility	getVisibility() {return Visibility;}


protected:
	/// Implement the clean method.
	virtual void	clean()
	{
		IModel::clean();

		// Clean up the model.
		updateLocalMatrix();
		Touch.clear(TransformDirty);
	}

private:
	friend class	CTransformHrcObs;
	friend class	CTransformClipObs;

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
