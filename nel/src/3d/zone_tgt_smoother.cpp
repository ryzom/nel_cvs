/** \file zone_tgt_smoother.cpp
 * <File description>
 *
 * $Id: zone_tgt_smoother.cpp,v 1.1 2001/01/16 08:34:57 berenguier Exp $
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

#include "nel/3d/zone_tgt_smoother.h"
#include "nel/misc/plane.h"
using namespace std;
using namespace NLMISC;

namespace NL3D
{



// ***************************************************************************
void		CZoneTgtSmoother::makeVerticesCoplanar(std::vector<CZoneInfo>  &zones)
{
	sint	i,j, numZone;
	sint	centerZoneId;

	nlassert(zones.size()>=1);
	centerZoneId= zones[0].ZoneId;

	// 0. CenterZone.
	//===============
	// First, make connectivity patch/vertex
	for(i=0;i<(sint)zones[0].Patchs.size();i++)
	{
		CPatchInfo		&pa= zones[0].Patchs[i];

		for(j=0;j<4;j++)
		{
			sint	vtx= pa.BaseVertices[j];
			CPatchId	pid;
			pid.ZoneId= centerZoneId;
			pid.PatchId= i;
			pid.Patch= &pa;
			pid.IdVert= j;
			VertexMap[vtx].Patchs.push_back(pid);
		}
	}
	// Second, what vertices of this zone are one border?
	for(i=0;i<(sint)zones[0].BorderVertices.size();i++)
	{
		CBorderVertex	&bv= zones[0].BorderVertices[i];
		sint	vtx= bv.CurrentVertex;
		VertexMap[vtx].OnBorder= true;
	}

	// 1. Neighbor zones.
	//===================
	map<sint, sint>		tempMap;
	for(numZone= 1; numZone<(sint)zones.size(); numZone++)
	{
		sint	adjZoneId= zones[numZone].ZoneId;

		tempMap.clear();
		// Tests which vertices points on the center zone.
		for(i=0;i<(sint)zones[numZone].BorderVertices.size();i++)
		{
			CBorderVertex	&bv= zones[numZone].BorderVertices[i];

			if(bv.NeighborZoneId== centerZoneId)
			{
				tempMap[bv.CurrentVertex]= bv.NeighborVertex;
			}
		}
		// Tests patchs which points on center zone.
		for(i=0;i<(sint)zones[numZone].Patchs.size();i++)
		{
			CPatchInfo		&pa= zones[numZone].Patchs[i];

			for(j=0;j<4;j++)
			{
				sint	vtx= pa.BaseVertices[j];
				if(tempMap.find(vtx)!=tempMap.end())
				{
					CPatchId	pid;
					pid.ZoneId= adjZoneId;
					pid.PatchId= i;
					pid.Patch= &pa;
					pid.IdVert= j;
					// Fill the vertex of the center zone.
					VertexMap[tempMap[vtx]].Patchs.push_back(pid);
				}
			}
		}

	}

	// 2. Process each vertex.
	//========================
	ItVertexMap		itVert;
	for(itVert= VertexMap.begin(); itVert!=VertexMap.end(); itVert++)
	{
		CVertexInfo		&vert= itVert->second;

		// a. verify if coplanar is possible.
		//===================================

		// TODO later: do it too on non border vertices if wanted (with a normal threshold...).
		if(!vert.OnBorder)
			continue;
		// TODO later: formula with 3, 5 ... patchs around the vertex.
		if(vert.Patchs.size()!=4)
			continue;

		// Test if there is no bind 1/x on this patch, around this vertex.
		// TODO later: binds should works...
		std::list<CPatchId>::iterator	itPatch;
		for(itPatch= vert.Patchs.begin(); itPatch!= vert.Patchs.end(); itPatch++)
		{
			// Tests the two edges around the vertex.
			sint	e0= itPatch->IdVert;
			sint	e1= (itPatch->IdVert+4-1)%4;

			if(itPatch->Patch->BindEdges[e0].NPatchs!= 1)
				continue;
			if(itPatch->Patch->BindEdges[e1].NPatchs!= 1)
				continue;
		}

		// b. build the plane.
		//====================
		CVector		planeNormal(0,0,0);
		CVector		planeCenter;
		for(itPatch= vert.Patchs.begin(); itPatch!= vert.Patchs.end(); itPatch++)
		{
			CPatchInfo		&pa= *(itPatch->Patch);
			CVector			a,b,c, pvect;
			sint			t0= (itPatch->IdVert*2+8-1)%8;
			sint			t1= itPatch->IdVert*2;

			// CCW order.
			a= pa.Patch.Tangents[t0];
			b= pa.Patch.Vertices[itPatch->IdVert];
			c= pa.Patch.Tangents[t1];
			pvect= (b-a)^(c-b);
			planeNormal+= pvect.normed();

			// yes, done 4 times... :(
			planeCenter= b;
		}
		// Average of all normals...
		planeNormal.normalize();
		CPlane		plane;
		plane.make(planeNormal, planeCenter);


		// c. projects the tangents, rebuild interior.
		//============================================
		for(itPatch= vert.Patchs.begin(); itPatch!= vert.Patchs.end(); itPatch++)
		{
			CPatchInfo		&pa= *(itPatch->Patch);
			sint			t0= (itPatch->IdVert*2+8-1)%8;
			sint			t1= itPatch->IdVert*2;
			pa.Patch.Tangents[t0]= plane.project(pa.Patch.Tangents[t0]);
			pa.Patch.Tangents[t1]= plane.project(pa.Patch.Tangents[t1]);

			// Setup the coplanared interior. just the sum of 2 vector tangents.
			pa.Patch.Interiors[itPatch->IdVert]= pa.Patch.Tangents[t0] + pa.Patch.Tangents[t1] - planeCenter;
		}

	}

}


/*
		// CODE TEST TO DO BETTER COPLANAR THAN MAX. BUG!!!  (maybe in part b...)

		// b. maps patchs on tangents.
		//=========================
		vector<CTangentId>	tangents;
		for(itPatch= vert.Patchs.begin(); itPatch!= vert.Patchs.end(); itPatch++)
		{
			CPatchInfo		&pa= *(itPatch->Patch);
			sint	e0= itPatch->IdVert;
			sint	e1= (itPatch->IdVert+4-1)%4;
			sint	tgt, patchId, zoneId;

			// Edge0.
			//=======
			// get neighbor patch id.
			zoneId= pa.BindEdges[e0].ZoneId;
			patchId= pa.BindEdges[e0].Next[0];
			// If tangent already inserted, mapped to this neighbor, just map ME to it...
			for(tgt= 0; tgt<(sint)tangents.size();tgt++)
			{
				if(tangents[tgt].ZoneId==zoneId && tangents[tgt].PatchId==patchId)
					break;
			}
			// If not found, add the tangent.
			if(tgt==(sint)tangents.size())
			{
				CTangentId	tangent;
				// Set OUR patch Id.
				tangent.ZoneId= itPatch->ZoneId;
				tangent.PatchId= itPatch->PatchId;
				// Get the tangent, just after the vertex.
				tangent.Tangent= pa.Patch.Tangents[itPatch->IdVert*2];
				tangent.p0= &pa;
				tangents.push_back(tangent);
			}
			else
			{
				tangents[tgt].p1= &pa;
			}
			// Map the patch to this tangent.
			itPatch->tgt0= tgt;


			// Edge1.
			//=======
			zoneId= pa.BindEdges[e1].ZoneId;
			patchId= pa.BindEdges[e1].Next[0];
			// If tangent already inserted, mapped to a neighbor, just map to it...
			for(tgt= 0; tgt<(sint)tangents.size();tgt++)
			{
				if(tangents[tgt].ZoneId==zoneId && tangents[tgt].PatchId==patchId)
					break;
			}
			// If not found, add the tangent.
			if(tgt==(sint)tangents.size())
			{
				CTangentId	tangent;
				// Set OUR patch Id.
				tangent.ZoneId= itPatch->ZoneId;
				tangent.PatchId= itPatch->PatchId;
				// Get the tangent, just before the vertex.
				tangent.Tangent= pa.Patch.Tangents[(itPatch->IdVert*2+8-1)%8];
				tangent.p0= &pa;
				tangents.push_back(tangent);
			}
			else
			{
				tangents[tgt].p1= &pa;
			}
			// Map the patch to this tangent.
			itPatch->tgt1= tgt;

		}

		// There should be 4 tangents.
		//nlassert(tangents.size()==4);


		// c. build the plane.
		//====================
		CVector		planeNormal(0,0,0);
		CVector		planeCenter;
		for(itPatch= vert.Patchs.begin(); itPatch!= vert.Patchs.end(); itPatch++)
		{
			CPatchInfo		&pa= *(itPatch->Patch);
			CVector			a,b,c, pvect;

			// CCW order.
			a= tangents[itPatch->tgt1].Tangent;
			b= pa.Patch.Vertices[itPatch->IdVert];
			c= tangents[itPatch->tgt0].Tangent;
			pvect= (b-a)^(c-b);
			planeNormal+= pvect.normed();

			// yes, done 4 times... :(
			planeCenter= b;
		}
		planeNormal.normalize();
		CPlane		plane;
		plane.make(planeNormal, planeCenter);


		// d. project the tangents.
		//=========================
		// TODO: better coplanar than Max... (with orthogonal angles: use p0/p1).
		for(i=0;i<(sint)tangents.size();i++)
		{
			//tangents[i].Tangent= plane.project(tangents[i].Tangent);
		}


		// e. assign tangents to patchs, rebuild interior.
		//==============================
		for(itPatch= vert.Patchs.begin(); itPatch!= vert.Patchs.end(); itPatch++)
		{
			CPatchInfo		&pa= *(itPatch->Patch);
			sint			t0= itPatch->IdVert*2;
			sint			t1= (itPatch->IdVert*2+8-1)%8;
			pa.Patch.Tangents[t0]= tangents[itPatch->tgt0].Tangent;
			pa.Patch.Tangents[t1]= tangents[itPatch->tgt1].Tangent;

			// Setup the coplanared interior. just the sum of 2 vector tangents.
			pa.Patch.Interiors[itPatch->IdVert]= pa.Patch.Tangents[t0] + pa.Patch.Tangents[t1] - planeCenter;
		}
*/


} // NL3D
