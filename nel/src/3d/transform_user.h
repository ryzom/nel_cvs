/** \file transform_user.h
 * <File description>
 *
 * $Id: transform_user.h,v 1.22 2003/08/07 08:49:13 berenguier Exp $
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

#define NL3D_MEM_TRANSFORM						NL_ALLOC_CONTEXT( 3dTrans )

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
	CScene				*_Scene;	// Can be NULL if deleteIt is false.
	// The object.
	CTransform			*_Transform;
	UInstanceGroup		*_pIG;

	// Must delete the transform shape ?, if false, scene can be NULL.
	bool				_DeleteIt;
public:


	/// \name Object
	// @{
	/** Give a Scene Instance. CTransformUser owns it, and will delete it.
	  * \param deleteIt is true if the CTransformUser must delete the transform shape in the destructor, else false.
	  */
	CTransformUser(CScene *scene, CTransform *trans, bool deleteIt) : CTransformableUser( static_cast<ITransformable*>((CTransform*)trans) )
	{
		NL3D_MEM_TRANSFORM
		nlassert(trans);
		_Scene= scene;
		_DeleteIt = deleteIt;
		// NB: _Transform is "same" pointer as ITransformable, but correclty casted.
		_Transform= NLMISC::safe_cast<CTransform*>(trans);
		_pIG = NULL;
		// Same enums!!
		nlassert((uint)UTransform::VisibilityCount == (uint)CHrcTrav::VisibilityCount);
	}
	virtual	~CTransformUser()
	{
		NL3D_MEM_TRANSFORM
		if (_Transform && _DeleteIt)
		{
			// Scene must not be NULL if _DeleteIt == true
			nlassert (_Scene);

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
		NL3D_MEM_TRANSFORM
		nlassert(_Transform) ; // object invalid now ...
		if(newFather)
		{
			// link me to other.
			CTransformUser	*other= dynamic_cast<CTransformUser*>(newFather);
			if(other->_Scene!=_Scene)
				nlerror("Try to parent 2 object from 2 differnet scenes!!");
			other->_Transform->hrcLinkSon( _Transform );
		}
		else
		{
			// link me to Root.
			_Scene->getRoot()->hrcLinkSon( _Transform );
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
		NL3D_MEM_TRANSFORM
		nlassert(_Transform) ; // object invalid now ...
		_Transform->hide();
	}
	/// Show the objet and his sons.
	virtual	void			show()
	{
		NL3D_MEM_TRANSFORM
		nlassert(_Transform) ; // object invalid now ...
		_Transform->show();
	}
	virtual void			setUserClipping(bool enable)
	{
		NL3D_MEM_TRANSFORM
		nlassert(_Transform) ; // object invalid now ...
		_Transform->setUserClipping(enable);
	}
	virtual bool			getUserClipping() const
	{
		NL3D_MEM_TRANSFORM
		nlassert(_Transform) ; // object invalid now ...
		return _Transform->getUserClipping();
	}
	/// herit the visibility from his father. (default behavior).
	virtual	void			heritVisibility()
	{
		NL3D_MEM_TRANSFORM
		nlassert(_Transform) ; // object invalid now ...
		_Transform->heritVisibility();
	}
	/// Get the local visibility state.
	virtual	TVisibility		getVisibility()
	{
		NL3D_MEM_TRANSFORM
		nlassert(_Transform) ; // object invalid now ...
		return (UTransform::TVisibility)(uint32)_Transform->getVisibility();
	}
	// @}


	
	/// \name freezeHRC
	// @{
	virtual	void			freezeHRC();
	virtual	void			unfreezeHRC();
	// @}

	virtual void  setOrderingLayer(uint layer) 
	{ 
		NL3D_MEM_TRANSFORM
		_Transform->setOrderingLayer(layer); 
	}

	/// Get the ordering layer
	virtual uint getOrderingLayer() const 
	{ 
		NL3D_MEM_TRANSFORM
		return _Transform->getOrderingLayer(); 
	}


	/// name Lighting Behavior.
	// @{
	virtual	void			setUserLightable(bool enable) 
	{
		NL3D_MEM_TRANSFORM
		_Transform->setUserLightable(enable);
	}
	virtual	bool			getUserLightable() const  
	{
		NL3D_MEM_TRANSFORM
		return  _Transform->getUserLightable();
	}
	// @}


	virtual void			setLogicInfo(ILogicInfo *logicInfo) 
	{
		NL3D_MEM_TRANSFORM
		_Transform->setLogicInfo(logicInfo);
	}


	virtual bool	getLastWorldVisState() const
	{
		NL3D_MEM_TRANSFORM
		return _Transform->isHrcVisible();
	}

	virtual bool	getLastClippedState() const 
	{
		NL3D_MEM_TRANSFORM
		return _Transform->isClipVisible();
	}

	virtual void	getLastParentClusters(std::vector<CCluster*> &clusters) const;


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

	/// name Accessors for opacity/transparency
	// @{	
	virtual void			setTransparency(bool v)
	{
		NL3D_MEM_TRANSFORM
		_Transform->setTransparency(v);
	}
	virtual void			setOpacity(bool v)
	{
		NL3D_MEM_TRANSFORM
		_Transform->setOpacity(true);
	}
	virtual uint32			isOpaque()
	{
		NL3D_MEM_TRANSFORM
		return _Transform->isOpaque();
	}
	virtual uint32			isTransparent()
	{
		NL3D_MEM_TRANSFORM
		return _Transform->isTransparent();
	}
	// @}

	/// \name ShadowMapping
	// @{
	virtual void			enableCastShadowMap(bool state);
	virtual bool			canCastShadowMap() const;
	virtual void			enableReceiveShadowMap(bool state);
	virtual bool			canReceiveShadowMap() const;
	// @}

public:
	/// \name Accessor for CSeneUser / Other.
	// @{
	CScene		*getScene()
	{
		NL3D_MEM_TRANSFORM
		return _Scene;
	}
	CTransform	*getTransform()
	{
		NL3D_MEM_TRANSFORM
		return _Transform;
	}

	// @}



};


} // NL3D


#endif // NL_TRANSFORM_USER_H

/* End of transform_user.h */
