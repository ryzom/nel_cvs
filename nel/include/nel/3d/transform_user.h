/** \file transform_user.h
 * <File description>
 *
 * $Id: transform_user.h,v 1.4 2001/03/19 15:38:51 berenguier Exp $
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

#ifndef NL_TRANSFORM_USER_H
#define NL_TRANSFORM_USER_H

#include "nel/misc/types_nl.h"
#include "nel/3d/tmp/u_transform.h"
#include "nel/3d/transform.h"
#include "nel/3d/scene.h"


namespace NL3D 
{


// ***************************************************************************
/**
 * UTransform implementation.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CTransformUser : virtual public UTransform
{
protected:
	// The Scene.
	CScene				*_Scene;
	// The object.
	CTransform			*_Transform;

public:


	/// \name Object
	// @{
	/// Give a Scene Instance. CTransformUser owns it, and will delete it.
	CTransformUser(CScene *scene, IModel *trans)
	{
		nlassert(scene && trans);
		_Scene= scene;
		nlassert(dynamic_cast<CTransform*>(trans));
		_Transform= (CTransform*)trans;

		// Same enums!!
		nlassert((uint)UTransform::VisibilityCount == (uint)CHrcTrav::VisibilityCount);
		nlassert((uint)UTransform::TransformModeCount == (uint)ITransformable::TransformModeCount);
	}
	virtual	~CTransformUser()
	{
		// Delete this model!!
		_Scene->deleteModel(_Transform);
		_Transform= NULL;
	}
	// @}


	/// \name Space manipulation
	// @{
	virtual	void			setMatrix(const CMatrix &mat)
	{
		_Transform->setMatrix(mat);
	}
	virtual	const CMatrix	&getMatrix() const	
	{
		return _Transform->getMatrix();
	}
	/// unlink this from oldparent, and make this be a son of newFather.
	virtual	void			parent(UTransform *newFather)
	{
		if(newFather)
		{
			// link me to other.
			CTransformUser	*other= dynamic_cast<CTransformUser*>(newFather);
			if(other->_Scene!=_Scene)
				nlerror("Try to parent 2 object from 2 differnet scenes!!");
			_Scene->getTrav(HrcTravId)->link(other->_Transform, _Transform);
		}
		else
		{
			// link me to Root.
			_Scene->getTrav(HrcTravId)->link(NULL, _Transform);
		}
	}
	// @}


	/// \name space set/get.
	// @{

	virtual	void			setTransformMode(TTransformMode mode, CMatrix::TRotOrder ro= CMatrix::ZXY)
	{
		_Transform->setTransformMode((ITransformable::TTransformMode)(uint)mode, ro);
	}
	virtual	void			setPos(const CVector &pos)
	{
		_Transform->setPos(pos);
	}
	virtual	void			setRotEuler(const CVector &rot)
	{
		_Transform->setRotEuler(rot);
	}
	virtual	void			setRotQuat(const CQuat &quat)
	{
		_Transform->setRotQuat(quat);
	}
	virtual	void			setScale(const CVector &scale)
	{
		_Transform->setScale(scale);
	}
	virtual	void			setPivot(const CVector &pivot)
	{
		_Transform->setPivot(pivot);
	}

	virtual	TTransformMode		getTransformMode()
	{
		return (TTransformMode)(uint)_Transform->getTransformMode();
	}
	virtual	CMatrix::TRotOrder	getRotOrder()
	{
		return _Transform->getRotOrder();
	}

	virtual	void			getPos(CVector &pos)
	{
		_Transform->getPos(pos);
	}
	virtual	void			getRotEuler(CVector &rot)
	{
		_Transform->getRotEuler(rot);
	}
	virtual	void			getRotQuat(CQuat &quat)
	{
		_Transform->getRotQuat(quat);
	}
	virtual	void			getScale(CVector &scale)
	{
		_Transform->getScale(scale);
	}
	virtual	void			getPivot(CVector &pivot)
	{
		_Transform->getPivot(pivot);
	}

	virtual	CVector			getPos()
	{
		return _Transform->getPos();
	}
	virtual	CVector			getRotEuler()
	{
		return _Transform->getRotEuler();
	}
	virtual	CQuat			getRotQuat()
	{
		return _Transform->getRotQuat();
	}
	virtual	CVector			getScale()
	{
		return _Transform->getScale();
	}
	virtual	CVector			getPivot()
	{
		return _Transform->getPivot();
	}

	// @}


	/// \name visibility
	// @{
	/// Hide the object and his sons.
	virtual	void			hide()
	{
		_Transform->hide();
	}
	/// Show the objet and his sons.
	virtual	void			show()
	{
		_Transform->show();
	}
	/// herit the visibility from his father. (default behavior).
	virtual	void			heritVisibility()
	{
		_Transform->heritVisibility();
	}
	/// Get the local visibility state.
	virtual	TVisibility		getVisibility()
	{
		return (UTransform::TVisibility)(uint32)_Transform->getVisibility();
	}
	// @}


	/// \name Misc
	// @{
	/** 
	  * Setup Matrix by the lookAt method. Work only in DirectMatrix mode and RotQuat mode (not euler...).
	  * 
	  * \param eye is the coordinate of the object.
	  * \param target is the point the object look at.
	  * \param roll is the roll angle in radian along the object's Y axis.
	  */
	virtual	void			lookAt (const CVector& eye, const CVector& target, float roll=0.f)
	{
		_Transform->lookAt(eye, target, roll);
	}
	// @}

public:
	/// \name Accessor for CSeneUser.
	// @{
	CScene		*getScene()
	{
		return _Scene;
	}
	// @}

};


} // NL3D


#endif // NL_TRANSFORM_USER_H

/* End of transform_user.h */
