/** \file water_user.h
 * class that implements the user interface for water
 *
 * $Id: water_user.h,v 1.3 2002/10/28 17:32:13 corvazier Exp $
 */

/* Copyright, 2000, 2001 Nevrax Ltd.
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

#include "nel/3d/u_water.h"
#include "3d/instance_user.h"

#define NL3D_MEM_WATER						NL_ALLOC_CONTEXT( 3dWater )

namespace NL3D 
{


/// implementation of UWaterInstance methods
class CWaterInstanceUser : public CInstanceUser, public UWaterInstance
{
public:		
	CWaterInstanceUser(CScene *scene, IModel *trans) : CInstanceUser(scene, trans) 
	{
		NL3D_MEM_WATER
	}
	virtual uint32	getWaterHeightMapID() const;
	virtual float	getHeightFactor() const;	
	virtual float   getHeight(const NLMISC::CVector2f &pos);
	virtual float   getAttenuatedHeight(const NLMISC::CVector2f &pos, const NLMISC::CVector &viewer);
};


} // NL3D