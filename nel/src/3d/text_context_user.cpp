/** \file text_context_user.cpp
 * <File description>
 *
 * $Id: text_context_user.cpp,v 1.4 2002/02/28 12:59:51 besson Exp $
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

#include "std3d.h"

#include "3d/text_context_user.h"


namespace NL3D 
{


// ***************************************************************************
NLMISC::CVector UTextContext::CStringInfo::getHotSpotVector(UTextContext::THotSpot hotspot)
{
	NLMISC::CVector hotspotVector(0,0,0);

	if(hotspot==UTextContext::MiddleLeft)
		hotspotVector = CVector(0,0,-StringHeight/2);
	
	if(hotspot==UTextContext::TopLeft)
		hotspotVector = CVector(0,0,StringHeight);
	
	if(hotspot==UTextContext::MiddleBottom)
		hotspotVector = CVector(-StringWidth/2,0,0);
	
	if(hotspot==UTextContext::MiddleMiddle)
		hotspotVector = CVector(-StringWidth/2,0,-StringHeight/2);
	
	if(hotspot==UTextContext::MiddleTop)
		hotspotVector = CVector(-StringWidth/2,0,-StringHeight);
	
	if(hotspot==UTextContext::BottomRight)
		hotspotVector = CVector(-StringWidth,0,0);
	
	if(hotspot==UTextContext::MiddleRight)
		hotspotVector = CVector(-StringWidth,0,-StringHeight/2);
	
	if(hotspot==UTextContext::TopRight)
		hotspotVector = CVector(-StringWidth,0,-StringHeight);

	return hotspotVector;
}




} // NL3D
