/** \file transform_user.h
 * <File description>
 *
 * $Id: transform_user.h,v 1.14 2002/08/05 15:29:11 berenguier Exp $
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
#include "nel/misc/debug.h"
#include "nel/3d/u_transform.h"
#include "3d/transform.h"
#include "3d/transform_shape.h"
#include "3d/scene.h"
#include "3d/transformable_user.h"


namespace NL3D 
{

class UInstanceGroup;

// ***************************************************************************
/**
 * UTransform implementation.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CTransformUser : virtual public UTransform, public CTransformableUser
{
protected:
	// The Scene.
	CScene				*_Scene;
	// The object.
	CTransform			*_Transform;
	UInstanceGroup		*_pIG;
public:


	/// \name Object
	// @{
	/// Give a Scene Instance. CTransformUser owns it, and will delete it.
	CTransformUser(CScene *scene, IModel *trans) : CTransformableUser( static_cast<ITransformable*>((CTransform*)trans) )
	{
		nlassert(scene && trans);
		_Scene= scene;
		// NB: _Transform is "same" pointer as ITransformable, but correclty casted.
		_Transform= NLMISC::safe_cast<CTransform*>(trans);
		_pIG = NULL;
		// Same enums!!
		nlassert((uint)UTransform::VisibilityCount == (uint)CHrcTrav::VisibilityCount);
	}
	virtual	~CTransformUser()
	{
		if (_Transform)
		{
			// Must test if _Transform is a CTransfromShape. If yes, must call deleteInstance().
			CTransformShape	*pTrShp= dynamic_cast<CTransformShape*>(_Transform);
			if(pTrShp)
				_Scene->deleteInstance(pTrShp);
			else
				_Scene->deleteModel(_Transform);
			_Transform= NULL;
		}
	}
	// @}


	/// \name Space manipulation
	// @{
	/// unlink this from oldparent, and make this be a son of newFather.
	virtual	void			parent(UTransform *newFather)
	{
		nlassert(_Transform) ; // object invalid now ...
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

	virtual void			setClusterSystem (UInstanceGroup *pIG);
	virtual UInstanceGroup *getClusterSystem ();
	// @}


	/// \name visibility
	// @{
	/// Hide the object and his sons.
	virtual	void			hide()
	{
		nlassert(_Transform) ; // object invalid now ...
		_Transform->hide();
	}
	/// Show the objet and his sons.
	virtual	void			show()
	{
		nlassert(_Transform) ; // object invalid now ...
		_Transform->show();
	}
	/// herit the visibility from his father. (default behavior).
	virtual	void			heritVisibility()
	{
		nlassert(_Transform) ; // object invalid now ...
		_Transform->heritVisibility();
	}
	/// Get the local visibility state.
	virtual	TVisibility		getVisibility()
	{
		nlassert(_Transform) ; // object invalid now ...
		return (UTransform::TVisibility)(uint32)_Transform->getVisibility();
	}
	// @}


	
	/// \name freezeHRC
	// @{
	virtual	void			freezeHRC();
	virtual	void			unfreezeHRC();
	// @}

	virtual void  setOrderingLayer(uint layer) { _Transform->setOrderingLayer(layer); }

	/// Get the ordering layer
	virtual uint getOrderingLayer() const { return _Transform->getOrderingLayer(); }


	/// name Lighting Behavior.
	// @{
	virtual	void			setUserLightable(bool enable) {_Transform->setUserLightable(enable);}
	virtual	bool			getUserLightable() const  {return  _Transform->getUserLightable();}
	// @}


	virtual void			setLogicInfo(ILogicInfo *logicInfo) {_Transform->setLogicInfo(logicInfo);}


	virtual bool	getLastClippedState() const {return _Transform->getLastClippedState();}

	virtual	const CMatrix	&getLastWorldMatrixComputed() const;

	/// name Load Balancing Behavior.
	// @{
	virtual void			setLoadBalancingGroup(const std::string &group);
	virtual const std::string	&getLoadBalancingGroup() const;
	// @}

	/// name Misc
	// @{
	virtual	void			setMeanColor(NLMISC::CRGBA color);
	virtual	NLMISC::CRGBA	getMeanColor() const;
	// @}

public:
	/// \name Accessor for CSeneUser / Other.
	// @{
	CScene		*getScene()
	{
		return _Scene;
	}
	CTransform	*getTransform()
	{
		return _Transform;
	}

	// @}

};


} // NL3D


#endif // NL_TRANSFORM_USER_H

/* End of transform_user.h */
