/** \file collision_desc.h
 * Description of the contact of a collision
 *
 * $Id: collision_desc.h,v 1.1 2001/05/04 14:36:59 corvazier Exp $
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

#ifndef NL_COLLISION_DESC_H
#define NL_COLLISION_DESC_H

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"

namespace NLPACS 
{

/**
 * Description of the contact of a collision
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class CCollisionDesc
{
public:
	NLMISC::CVector		ContactPosition;
	NLMISC::CVector		ContactNormal;
	float				ContactTime;
};


} // NLPACS


#endif // NL_COLLISION_DESC_H

/* End of collision_desc.h */
