/** \file zone.h
 * <File description>
 *
 * $Id: zone.h,v 1.1 2000/10/27 14:30:33 berenguier Exp $
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

#ifndef NL_ZONE_H
#define NL_ZONE_H

#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"
#include "nel/3d/tessellation.h"
#include <vector>


namespace NL3D {


class CZone;


// ***************************************************************************
struct	CBorderVertex
{
	// The neighbor  zone.
	CZone			*Other;
	// The neighbor zone Id.
	sint32			ZoneId;
	// The index of vertex in the neighbor zone.
	sint32			Vertex;
};



// ***************************************************************************
/**
 * A landscape zone.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CZone
{
public:
	float			PatchBias, PatchScale;


	// A Zone has:
	// The number of vertices she access (maybe on border).
	sint			NumVertices;
	// The list of border vetices. Patch vertex indices are sorted so that the border one are all at the begining.
	std::vector<CBorderVertex>	BorderVertices;
	// TODO: problem on corners ??? Have a special "World zone" for this??? :)


public:

	/// Constructor
	CZone();


// Private part.
private:
/*********************************/
	// A smartptrisable vertex.
	struct	CTessBaseVertex : public NLMISC::CRefCount
	{
		CTessVertex		Vert;
	};


private:
	// The smartptr on zone vertices.
	typedef	NLMISC::CSmartPtr<CTessBaseVertex>	PBaseVertex;
	typedef	std::vector<PBaseVertex>					TBaseVerticesVec;
	TBaseVerticesVec	BaseVertices;

	
private:
	friend	class CTessFace;

	// Local info for CTessFace tiles. CZone must setup them at the begining at refine()/render().
	// Should we compute the error metric part for tile?? Stored by Zone. By patch, it is Too slow, regarding
	// the cost of computing this errormetric in CTessFace::updateErrorMetric().
	bool			ComputeTileErrorMetric;
};


} // RK3D


#endif // NL_ZONE_H

/* End of zone.h */
