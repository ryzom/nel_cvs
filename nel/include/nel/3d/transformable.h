/** \file transformable.h
 * <File description>
 *
 * $Id: transformable.h,v 1.4 2001/03/21 11:14:03 berenguier Exp $
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

#ifndef NL_TRANSFORMABLE_H
#define NL_TRANSFORMABLE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/matrix.h"
#include "nel/misc/quat.h"
#include "nel/3d/animatable.h"
#include "nel/3d/animated_value.h"
#include "nel/3d/track.h"


namespace NL3D
{

class	CChannelMixer;

using NLMISC::CMatrix;
using NLMISC::CVector;
using NLMISC::CQuat;


/**
 * Something which can be transformed in 3D space / animated.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class ITransformable : public IAnimatable
{
public:

	enum	TTransformMode
	{
		DirectMatrix=0,		// DirectMatrixMode (default).
		RotEuler,			// Matrix is computed from sperated composantes, with euler rotation.
		RotQuat,			// Matrix is computed from sperated composantes, with quat rotation.

		TransformModeCount
	};


public:

	/// Constructor. By default, DirectMatrix mode.
	ITransformable();


	/// Get the matrix, compute her if necessary (work in all modes).
	const CMatrix	&getMatrix() const {updateMatrix(); return _LocalMatrix;}

	/// Tells if the matrix needs to be computed, either if data are modified, or if father is modified (if unheritScale enabled).
	bool			needCompute() const;



	/// \name Transform Mode.
	//@{
	/// Change the transform mode. Components or matrix are not reseted.
	void	setTransformMode(TTransformMode mode, CMatrix::TRotOrder ro= CMatrix::ZXY)
	{
		_Mode= mode;
		_RotOrder= ro;
		// just for information.
		touch(PosValue);
	}
	/// Herit the scale of father (default).
	void	heritScale()
	{
		_Father= NULL;
		// just for information.
		touch(PosValue);
	}
	/// Do not Herit the scale of father (default). => use scale part of father (if not DirectMatrix mode)
	void	unheritScale(ITransformable *father)
	{
		_Father= father;
		// just for information.
		touch(PosValue);
	}
	//@}


	/// \name Matrix operations.
	//@{

	/// Work only in Rot* mode(nlassert).
	void	setPos(const CVector &pos)
	{
		nlassert(_Mode==RotEuler || _Mode==RotQuat);
		_Pos.Value= pos;
		touch(PosValue);
	}
	/// Work only in RotEuler mode(nlassert).
	void	setRotEuler(const CVector &rot)
	{
		nlassert(_Mode==RotEuler);
		_RotEuler.Value= rot;
		touch(RotEulerValue);
	}
	/// Work only in RotQuat mode (nlassert).
	void	setRotQuat(const CQuat &quat)
	{
		nlassert(_Mode==RotQuat);
		_RotQuat.Value= quat;
		touch(RotQuatValue);
	}
	/// Work only in Rot* mode (nlassert).
	void	setScale(const CVector &scale)
	{
		nlassert(_Mode==RotEuler || _Mode==RotQuat);
		_Scale.Value= scale;
		touch(ScaleValue);
	}
	/// Work only in Rot* mode (nlassert).
	void	setPivot(const CVector &pivot)
	{
		nlassert(_Mode==RotEuler || _Mode==RotQuat);
		_Pivot.Value= pivot;
		touch(PivotValue);
	}
	/// Work only in DirecTMatrix mode (nlassert).
	void	setMatrix(const CMatrix &mat)
	{
		nlassert(_Mode==DirectMatrix);
		_LocalMatrix= mat;
		// just for information.
		touch(PosValue);
	}

	//@}


	/// \name Matrix Get operations.
	//@{

	/// get the current transform mode.
	TTransformMode	getTransformMode()
	{
		return _Mode;
	}
	/// get the current rotorder (information vlaid only when RotEuler mode).
	CMatrix::TRotOrder	getRotOrder()
	{
		return _RotOrder;
	}

	/// Work only in Rot* mode(nlassert).
	void	getPos(CVector &pos)
	{
		nlassert(_Mode==RotEuler || _Mode==RotQuat);
		pos= _Pos.Value;
	}
	/// Work only in RotEuler mode(nlassert).
	void	getRotEuler(CVector &rot)
	{
		nlassert(_Mode==RotEuler);
		rot= _RotEuler.Value;
	}
	/// Work only in RotQuat mode (nlassert).
	void	getRotQuat(CQuat &quat)
	{
		nlassert(_Mode==RotQuat);
		quat= _RotQuat.Value;
	}
	/// Work only in Rot* mode (nlassert).
	void	getScale(CVector &scale)
	{
		nlassert(_Mode==RotEuler || _Mode==RotQuat);
		scale= _Scale.Value;
	}
	/// Work only in Rot* mode (nlassert).
	void	getPivot(CVector &pivot)
	{
		nlassert(_Mode==RotEuler || _Mode==RotQuat);
		pivot= _Pivot.Value;
	}

	/// Work only in Rot* mode(nlassert).
	CVector	getPos()
	{
		nlassert(_Mode==RotEuler || _Mode==RotQuat);
		return _Pos.Value;
	}
	/// Work only in RotEuler mode(nlassert).
	CVector	getRotEuler()
	{
		nlassert(_Mode==RotEuler);
		return _RotEuler.Value;
	}
	/// Work only in RotQuat mode (nlassert).
	CQuat	getRotQuat()
	{
		nlassert(_Mode==RotQuat);
		return _RotQuat.Value;
	}
	/// Work only in Rot* mode (nlassert).
	CVector	getScale()
	{
		nlassert(_Mode==RotEuler || _Mode==RotQuat);
		return _Scale.Value;
	}
	/// Work only in Rot* mode (nlassert).
	CVector getPivot()
	{
		nlassert(_Mode==RotEuler || _Mode==RotQuat);
		return _Pivot.Value;
	}
	//@}


	/// \name Misc
	// @{
	/** 
	  * Setup Matrix by the lookAt method. Work only in DirectMatrix mode and RotQuat mode (not euler...).
	  * 
	  * \param eye is the coordinate of the object.
	  * \param target is the point the object look at.
	  * \param roll is the roll angle in radian along the object's Y axis.
	  */
	void		lookAt (const CVector& eye, const CVector& target, float roll=0.f);
	// @}



	/// \name Herited from IAnimatable
	// @{
	/// Added values.
	enum	TAnimValues
	{
		PosValue= IAnimatable::AnimValueLast,
		RotEulerValue,
		RotQuatValue,
		ScaleValue,
		PivotValue,
		AnimValueLast
	};

	/// From IAnimatable
	virtual IAnimatedValue* getValue (uint valueId);

	/// From IAnimatable
	virtual const char *getValueName (uint valueId) const;

	/// From IAnimatable. Deriver must implement this.
	virtual ITrack* getDefaultTrack (uint valueId) =0;

	/// From IAnimatable. Deriver must implement this (channels may be detail-ed or not).
	virtual	void	registerToChannelMixer(CChannelMixer *chanMixer, const std::string &prefix) =0;

	// @}


	

private:
	// The computed matrix.
	mutable CMatrix			_LocalMatrix;
	TTransformMode			_Mode;
	CMatrix::TRotOrder		_RotOrder;
	// For father scale un-inheritance.
	ITransformable			*_Father;
	mutable	uint64			_FatherScaleDate;
	mutable	uint64			_LocalScaleDate;

	// For animation, Pos, rot scale pivot animated values
	CAnimatedValueVector	_Pos;
	CAnimatedValueVector	_RotEuler;
	CAnimatedValueQuat		_RotQuat;
	CAnimatedValueVector	_Scale;
	CAnimatedValueVector	_Pivot;

	// just test transfrom fglas.
	bool	testTransformFlags() const;

	// clear transfrom fglas.
	void	clearTransformFlags() const;

	// compute the matrix.
	void	updateMatrix() const;

};


} // NL3D


#endif // NL_TRANSFORMABLE_H

/* End of transformable.h */
