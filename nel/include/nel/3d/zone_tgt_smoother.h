/** \file zone_tgt_smoother.h
 * <File description>
 *
 * $Id: zone_tgt_smoother.h,v 1.1 2001/01/16 08:35:14 berenguier Exp $
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

#ifndef NL_ZONE_TGT_SMOOTHER_H
#define NL_ZONE_TGT_SMOOTHER_H

#include "nel/misc/types_nl.h"
#include "nel/3d/zone.h"
#include "nel/misc/common.h"
#include <map>
#include <list>
#include <vector>


namespace NL3D
{


// ***************************************************************************
/**
 * A class used to make Vertices coplanar IN or/and across zones.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CZoneTgtSmoother
{
public:

	/// Constructor
	CZoneTgtSmoother() {}

	/// Doit method. zones are modified. zones[0] is the center zones. Other are border zones.
	void		makeVerticesCoplanar(std::vector<CZoneInfo>  &zones);


// *************************
private:
	struct	CPatchId
	{
		// The unique Id of the patch.
		uint16			ZoneId;
		uint16			PatchId;
		// a ptr on the patch.
		CPatchInfo*		Patch;
		// [0,3]. which vertex of this patch points on the vertex.
		sint			IdVert;

		// local index on tangentes.
		sint			tgt0;
		sint			tgt1;
	};
	struct	CTangentId
	{
		uint16			ZoneId;
		uint16			PatchId;
		CPatchInfo		*p0, *p1;
		CVector			Tangent;
	};

	struct	CVertexInfo
	{
		// neighbors patchs.
		std::list<CPatchId>	Patchs;
		bool				OnBorder;

		CVertexInfo()
		{
			OnBorder= false;
		}
	};

	typedef	std::map<sint, CVertexInfo>		TVertexMap;
	typedef	TVertexMap::iterator			ItVertexMap;

private:
	TVertexMap		VertexMap;

};


} // NL3D


#endif // NL_ZONE_TGT_SMOOTHER_H

/* End of zone_tgt_smoother.h */
