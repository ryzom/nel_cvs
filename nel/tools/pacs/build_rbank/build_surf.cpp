/** \file build_surf.cpp
 *
 *
 * $Id: build_surf.cpp,v 1.24 2004/06/21 15:33:06 legros Exp $
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

#include <math.h>
#include <float.h>

#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"
#include "nel/misc/path.h"
#include "nel/misc/file.h"

#include "nel/misc/plane.h"
#include "nel/misc/triangle.h"
#include "nel/misc/polygon.h"

#include "3d/landscape.h"
#include "3d/mesh.h"
#include "3d/quad_grid.h"

#include "pacs/vector_2s.h"

#include "build_surf.h"

#include <deque>
#include <hash_map>

using namespace std;
using namespace NLMISC;
using namespace NL3D;


// Misc functions...

uint16	getZoneIdByPos(CVector &pos)
{
	uint		x, y;
	const float	zdim = 160.0f;

	x = (uint)(pos.x/zdim);
	y = (uint)(-pos.y/zdim);

	return x+y*256;
}

string getZoneNameById(uint16 id)
{
	uint	x = id%256;
	uint	y = id/256;

	char ych[32];
	sprintf(ych,"%d_%c%c", y+1, 'A'+x/26, 'A'+x%26);
	return string(ych);
}

uint16 getZoneIdByName(string &name)
{
	string upperName = strupr (name);
	sint		y = 0, x = 0;
	const char	*str = upperName.c_str();

	while (*str != '_')
		y = y*10 + *(str++)-'0';

	++str;

	x = (str[0]-'A')*26+(str[1]-'A');

	return (y-1)*256+x;
}

CAABBox	getZoneBBoxById(uint16 id)
{
	CAABBox		bbox;
	uint		x, y;
	const float	zdim = 160.0f;

	x = id%256;
	y = id/256;
	bbox.setMinMax(CVector(zdim*x,		-zdim*(y+1),	-10000.0f),
				   CVector(zdim*(x+1),	-zdim*y,		+10000.0f));


	return bbox;
}


bool	relativeEquals(CVector &a, CVector &b, float epsilon)
{
	float	n = (a-b).norm();
	float	r = a.norm();


	return (n/r <= epsilon);
}

bool	absoluteEquals(CVector &a, CVector &b, float epsilon)
{
	float	n = (a-b).norm();
	return n <= epsilon;
}

inline CVector	vmin(const CVector &a, const CVector &b, const CVector &c)
{
	return CVector(std::min(a.x, std::min(b.x, c.x)),
				   std::min(a.y, std::min(b.y, c.y)),
				   std::min(a.z, std::min(b.z, c.z)));
}

inline CVector	vmax(const CVector &a, const CVector &b, const CVector &c)
{
	return CVector(std::max(a.x, std::max(b.x, c.x)),
				   std::max(a.y, std::max(b.y, c.y)),
				   std::max(a.z, std::max(b.z, c.z)));
}

inline CVector	vmin(const CVector &a, const CVector &b)
{
	return CVector(std::min(a.x, b.x),
				   std::min(a.y, b.y),
				   std::min(a.z, b.z));
}

inline CVector	vmax(const CVector &a, const CVector &b)
{
	return CVector(std::max(a.x, b.x),
				   std::max(a.y, b.y),
				   std::max(a.z, b.z));
}

static sint64	float2Fixed(float f)
{
	return (sint64)(floor(f*NLPACS::Vector2sAccuracy));
}

static float	fixed2Float(sint64 s)
{
	return ((float)s)/NLPACS::Vector2sAccuracy;
}

static void		snapAccuracyBit(float &f)
{
	f = fixed2Float(float2Fixed(f));
}

static void		snapAccuracyBit(CVector &v)
{
	snapAccuracyBit(v.x);
	snapAccuracyBit(v.y);
	snapAccuracyBit(v.z);
}

static CAABBox	getSnappedBBox(CVector v0, CVector v1, CVector v2, const CAABBox &bbox)
{
	snapAccuracyBit(v0);
	snapAccuracyBit(v1);
	snapAccuracyBit(v2);

	CAABBox	box;

	box.setCenter(v0);
	box.extend(v1);
	box.extend(v2);

	return box;
}

static float	alg2dArea(const CVector &v0, const CVector &v1, const CVector &v2)
{
	float	ux = v1.x-v0.x,
			uy = v1.y-v0.y,
			vx = v2.x-v0.x,
			vy = v2.y-v0.y;

	return ux*vy - uy*vx;
}

static double	alg2dArea(const CVectorD &v0, const CVectorD &v1, const CVectorD &v2)
{
	double	ux = v1.x-v0.x,
			uy = v1.y-v0.y,
			vx = v2.x-v0.x,
			vy = v2.y-v0.y;

	return ux*vy - uy*vx;
}

template<class A>
class CHashPtr
{
public:
	typedef	A	*ptrA;
	size_t	operator() (const ptrA &a) const
	{
		return ((uint32)a)/sizeof(A);
	}
};

bool	isInside(const CPolygon &poly, const NLPACS::CSurfElement &elem)
{
	uint	i, j;

	if (poly.getNumVertices() <= 2)
		return false;
	
	CVector		pnorm = (poly.Vertices[0]-poly.Vertices[1])^(poly.Vertices[2]-poly.Vertices[1]);
	pnorm.normalize();

	for (i=0; i<3; ++i)
	{
		const CVector	&v = (*elem.Vertices)[elem.Tri[i]];
		bool			inside = true;

		for (j=0; j<poly.Vertices.size(); ++j)
		{
			const CVector	&v0 = poly.Vertices[j],
							&v1 = (j == poly.Vertices.size()-1) ? poly.Vertices[0] : poly.Vertices[j+1];

			if ((pnorm^(v1-v0)) * (v-v1) > 0.0f)
			{
				inside = false;
				break;
			}
		}

		if (inside)
			return true;
	}
	return false;
}


/*
 */
class CAABBoxPred
{
public:
	bool	operator () (const CAABBox &a, const CAABBox &b) const
	{
		return a.getCenter().z < b.getCenter().z;
	}
};








/*
 * CSurfElement methods
 *
 *
 *
 *
 *
 *
 *
 *
 */


void	NLPACS::CSurfElement::computeQuantas(CZoneTessellation *zoneTessel)
{
	CVector		v0 = (*Vertices)[Tri[0]],
				v1 = (*Vertices)[Tri[1]],
				v2 = (*Vertices)[Tri[2]];

	CVector		nv0 = v0,
				nv1 = v1,
				nv2 = v2;

	nv0.z = 0.0f;
	nv1.z = 0.0f;
	nv2.z = 0.0f;

	CVector	n = (nv1-nv0) ^ (nv2-nv0);

	double	hmin = std::min(v0.z, std::min(v1.z, v2.z));
	//QuantHeight = ((uint8)(floor((v0.z+v1.z+v2.z)/6.0f)))%255;
	QuantHeight = ((uint8)floor(hmin/2.0))%255;

	Area = 0.5f*n.norm();

	IsValid = (Normal.z > 0.707f);

	uint8	bits0 = PrimChecker.get((uint)v0.x, (uint)v0.y);
	uint8	bits1 = PrimChecker.get((uint)v1.x, (uint)v1.y);
	uint8	bits2 = PrimChecker.get((uint)v2.x, (uint)v2.y);

	uint16	ws0 = PrimChecker.index((uint)v0.x, (uint)v0.y);
	uint16	ws1 = PrimChecker.index((uint)v1.x, (uint)v1.y);
	uint16	ws2 = PrimChecker.index((uint)v2.x, (uint)v2.y);

	uint8	bits = bits0|bits1|bits2;

	if (bits & CPrimChecker::Include)
	{
		IsValid = true;
	}

	if (bits & CPrimChecker::Exclude)
	{
		ForceInvalid = true;
		IsValid = false;
	}

	if (bits & CPrimChecker::ClusterHint && IsValid)
	{
		ClusterHint = true;
	}

//	if ((bits & CPrimChecker::Cliff) && (bits & CPrimChecker::Water))
//		IsValid = false;

	if (Normal.z <= 0.30f)
	{
		ForceInvalid = true;
		IsValid = false;
	}
	else if ((bits & CPrimChecker::Water) != 0 && !ForceInvalid)
	{
		bool	w0 = ((bits0&CPrimChecker::Water) != 0);
		bool	w1 = ((bits1&CPrimChecker::Water) != 0);
		bool	w2 = ((bits2&CPrimChecker::Water) != 0);

		uint	ws = 0xff;

		if ((w0 && w1 && ws0 == ws1) || (w0 && w2 && ws0 == ws2))
			ws = ws0;
		else if (w1 && w2 && ws1 == ws2)
			ws = ws1;
		else if (w0)
			ws = ws0;
		else if (w1)
			ws = ws1;
		else if (w2)
			ws = ws2;
		else
		{
			nlwarning("No WaterShape found for element %d, whereas water detected...", ElemId);
		}

		WaterShape = ws;

		bool	exists;
		float	wh = PrimChecker.waterHeight(ws, exists);
		if (exists && ((*Vertices)[Tri[0]].z < wh || (*Vertices)[Tri[1]].z < wh || (*Vertices)[Tri[2]].z < wh))
		{
			if (bits & CPrimChecker::Cliff)
			{
				ForceInvalid = true;
				IsValid = false;
				return;
			}

			IsValid = true;
			WaterShape = ws;
			IsUnderWater = true;
		}
		else
		{
			ForceInvalid = true;
			IsValid = false;
		}
	}

	if (ForceInvalid)
		IsValid = false;
}

CAABBox	NLPACS::CSurfElement::getBBox() const
{
	CAABBox	box;
	box.setCenter((*Vertices)[Tri[0]]);
	box.extend((*Vertices)[Tri[1]]);
	box.extend((*Vertices)[Tri[2]]);
	return box;
}






/*
 * CComputableSurfaceBorder methods implementation
 *
 *
 *
 *
 *
 *
 *
 *
 */

void	NLPACS::CComputableSurfaceBorder::dump()
{
	sint	i;

	nldebug("dump border between %d and %d", Left, Right);
	for (i=0; i<(sint)Vertices.size(); ++i)
	{
		nldebug("  v[%d]={%g,%g,%g}", i, Vertices[i].x, Vertices[i].y, Vertices[i].z);
	}
}

void	NLPACS::CComputableSurfaceBorder::smooth(float val)
{
	float						minArea;
	vector<CVector>::iterator	it, minIt;
	uint						i;

	uint						before, after;

	bool						allowMoveLeft = (Left != -1);
	bool						allowMoveRight = (Right != -1);

	// filtering passes
	uint	numPass = 3;
	for (; numPass>0 && Vertices.size()>3; --numPass)
	{
		CVector	previous = Vertices[0];
		for (i=1; i<Vertices.size()-1; ++i)
		{
			CVector	newVert = (Vertices[i]*2.0+previous+Vertices[i+1])/4.0f;

			if (!allowMoveLeft || !allowMoveRight)
			{
				float	area1 = alg2dArea(previous, Vertices[i], Vertices[i+1]);
				float	area2 = alg2dArea(previous, newVert, Vertices[i+1]);

				previous = Vertices[i];

				if ((!allowMoveLeft && area2 > area1) || (!allowMoveRight && area2 < area1))
					Vertices[i] = newVert;
			}
			else
			{
				previous = Vertices[i];
				Vertices[i] = newVert;
			}
		}
	}

	before = Vertices.size();
	while (Vertices.size()>3)	// don't smooth blow 3 vertices to avoid degenrated surfaces
	{
		minArea = val;
		minIt = Vertices.end();
		it = Vertices.begin();
		++it;

		for (i=1; i<Vertices.size()-1; ++i, ++it)
		{
			float	area;

			area = 0.5f*((Vertices[i+1]-Vertices[i])^(Vertices[i-1]-Vertices[i])).norm();
			if (area < minArea)
			{
				minArea = area;
				minIt = it;
			}
		}

		if (minIt != Vertices.end())
		{
			Vertices.erase(minIt);
		}
		else
		{
			break;
		}
	}
	after = Vertices.size();

	if (Verbose)
		nlinfo("smoothed border %d-%d: %d -> %d", Left, Right, before, after);
}



















/*
 * CComputableSurface methods implementation
 *
 *
 *
 *
 *
 *
 *
 *
 */
void	NLPACS::CComputableSurface::followBorder(CZoneTessellation *zoneTessel, CSurfElement *first, uint edge, uint sens, vector<CVector> &vstore, bool &loop)
{
	CSurfElement	*current = first;
	CSurfElement	*next = current->EdgeLinks[edge];
	current->EdgeFlag[edge] = true;

	const sint32	currentSurfId = current->SurfaceId;
	const sint32	oppositeSurfId = (next != NULL) ? next->SurfaceId : UnaffectedSurfaceId;
	const sint32	oppositeZid = current->getZoneIdOnEdge(edge);
	sint			oedge;

	sint			pivot = (edge+sens)%3;
	sint			nextEdge = edge;

	bool			allowThis = true;

	// adds the pivot to the border and its normal
	vstore.push_back((*current->Vertices)[current->Tri[pivot]]);

	uint	loopCount = 0;

	while (true)
	{
		++loopCount;

		current->IsBorder = true;

		if ((oppositeSurfId != UnaffectedSurfaceId && (next == NULL || (next->SurfaceId != oppositeSurfId && next->SurfaceId != currentSurfId))) ||
			(oppositeSurfId == UnaffectedSurfaceId && (next != NULL && next->SurfaceId != currentSurfId || next == NULL && current->getZoneIdOnEdge(nextEdge) != oppositeZid)) ||
			((current->EdgeFlag[nextEdge] || zoneTessel->VerticesFlags[current->Tri[pivot]]!=0) && !allowThis))
		{
			// if reaches the end of the border, then quits.
			loop = (absoluteEquals(vstore.front(), vstore.back(), 1e-2f) && loopCount != 1);
			break;
		}
		else if ((oppositeSurfId != UnaffectedSurfaceId && next->SurfaceId == oppositeSurfId) ||
				 (oppositeSurfId == UnaffectedSurfaceId && next == NULL))
		{
			// if the next edge belongs to the border, then go on the same element
			current->EdgeFlag[nextEdge] = true;
			if (oppositeSurfId != UnaffectedSurfaceId)
			{
				for (oedge=0; oedge<3 && next->EdgeLinks[oedge]!=current; ++oedge)
					;
				nlassert(oedge != 3);
				nlassert(allowThis || !next->EdgeFlag[oedge]);
				next->EdgeFlag[oedge] = true;
			}
			pivot = (pivot+sens)%3;
			nextEdge = (nextEdge+sens)%3;
			next = current->EdgeLinks[nextEdge];
			vstore.push_back((*current->Vertices)[current->Tri[pivot]]);
		}
		else 
		{
			// if the next element is inside the surface, then go to the next element
			nlassert(next->SurfaceId == currentSurfId);

			for (oedge=0; oedge<3 && next->EdgeLinks[oedge]!=current; ++oedge)
				;
			nlassert(oedge != 3);
			current = next;
			pivot = (oedge+3-sens)%3;
			nextEdge = (oedge+sens)%3;
			next = current->EdgeLinks[nextEdge];
		}

		allowThis = false;
	}
}

void	NLPACS::CComputableSurface::buildBorders(CZoneTessellation *zoneTessel)
{
	sint	elem, edge;

	if (Verbose)
		nlinfo("generate borders for the surface %d (%d elements) - water=%d", SurfaceId, Elements.size(), (IsUnderWater ? 1 : 0));

	for (elem=0; elem<(sint)Elements.size(); ++elem)
	{
		// for each element,
		// scan for a edge that points to a different surface

		nlassert(Elements[elem]->SurfaceId == SurfaceId);

		for (edge=0; edge<3; ++edge)
		{

			if ((Elements[elem]->EdgeLinks[edge] == NULL || Elements[elem]->EdgeLinks[edge]->SurfaceId != SurfaceId) &&
				!Elements[elem]->EdgeFlag[edge])
			{
				BorderKeeper->resize(BorderKeeper->size()+1);
				CComputableSurfaceBorder	&border = BorderKeeper->back();

				border.Left = Elements[elem]->SurfaceId;

				// ????
				//border.DontSmooth = (Elements[elem]->EdgeLinks[edge] != NULL && Elements[elem]->NoLevelSurfaceId == Elements[elem]->EdgeLinks[edge]->SurfaceId);

				if (Elements[elem]->EdgeLinks[edge] != NULL && Elements[elem]->EdgeLinks[edge]->ZoneId != Elements[elem]->ZoneId)
				{
					// link on a neighbor zone
					border.Right = -2;
				}
				if (Elements[elem]->EdgeLinks[edge] == NULL || Elements[elem]->EdgeLinks[edge]->SurfaceId == UnaffectedSurfaceId)
				{
					// no link at all
					border.Right = -1;
				}
				else
				{
					border.Right = Elements[elem]->EdgeLinks[edge]->SurfaceId;
				}

				if (Verbose)
					nlinfo("generate border %d (%d-%d)", BorderKeeper->size()-1, border.Left, border.Right);

				bool				loop;
				vector<CVector>		bwdVerts;
				vector<CVector>		&fwdVerts = border.Vertices;

				followBorder(zoneTessel, Elements[elem], edge, 2, bwdVerts, loop);

				sint	i;

				fwdVerts.reserve(bwdVerts.size());
				fwdVerts.clear();

				for (i=(sint)(bwdVerts.size()-1); i>=0; --i)
				{
					fwdVerts.push_back(bwdVerts[i]);
				}

				if (loop)
				{
					fwdVerts.push_back(fwdVerts.front());
				}
				else
				{
					fwdVerts.resize(fwdVerts.size()-2);
					followBorder(zoneTessel, Elements[elem], edge, 1, fwdVerts, loop);
				}
			}
		}
	}
}













/*
 * CZoneTessellation constructors and methods implementation
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */

bool	NLPACS::CZoneTessellation::setup(uint16 zoneId, sint16 refinement, const CVector &translation)
{
	CentralZoneId = zoneId;
	Refinement = refinement;
//	Translation = translation;

	// the zone bbox is hard coded for accuracy improvement...
	OriginalBBox = getZoneBBoxById(zoneId);
	BBox = OriginalBBox;
	Translation = -BBox.getCenter();
	Translation.x = (float)floor(Translation.x+0.5f);
	Translation.y = (float)floor(Translation.y+0.5f);
	Translation.z = (float)floor(Translation.z+0.5f);
	BBox.setCenter(CVector::Null);
	BBox.setHalfSize(CVector(80.0f, 80.0f, BBox.getHalfSize().z));

	// if zone doesn't exist, don't even setup tessellation
	try
	{
		if (CPath::lookup(getZoneNameById(zoneId)+ZoneExt, false, false) == "")
			return false;
	}
	catch (EPathNotFound &)
	{
		return false;
	}

	// setup the square of 9 zones...
	if (Verbose)
		nlinfo("setup zone tessellation %d %s", zoneId, getZoneNameById(zoneId).c_str());
	{
		sint	zx = zoneId%256, zy = zoneId/256;

		for (zy=(zoneId/256)-1; zy<=(zoneId/256)+1; ++zy)
		{
			for (zx=(zoneId%256)-1; zx<=(zoneId%256)+1; ++zx)
			{
				if (zx >= 0 && zx <= 255 && zy >= 0 && zy <= 255)
				{
					uint	zid = (zy<<8) + (zx);
					string	filename = getZoneNameById(zid)+ZoneExt;
					if (CPath::lookup(filename, false, false) != "")
					{
						_ZoneIds.push_back(zid);
					}
				}
			}
		}
	}
	
	// sort zones
	sort(_ZoneIds.begin(), _ZoneIds.end());

	return true;
}





void	NLPACS::CZoneTessellation::build()
{
	sint	el;
	uint	i, j;

	NL3D::CLandscape	landscape;
	landscape.init();

	vector<CVector>				normals;

	vector<CVector>				vectorCheck;
	bool						useNoHmZones = true;

	{
		NL3D::CLandscape	landscapeNoHm;
		landscapeNoHm.init();

		//
		// load the 9 landscape zones
		//
		for (i=0; i<_ZoneIds.size(); ++i)
		{
			string	filename = getZoneNameById(_ZoneIds[i])+ZoneExt;
			CIFile	file(CPath::lookup(filename));
			CZone	zone;
			zone.serial(file);
			file.close();

			if (Verbose)
				nlinfo("use zone %s %d", filename.c_str(), zone.getZoneId());

			if (zone.getZoneId() != _ZoneIds[i])
			{
				nlwarning ("Zone %s ID is wrong. Abort.", filename.c_str());
				return;
			}
			landscape.addZone(zone);

			if (useNoHmZones)
			{
				string	filenameNH = getZoneNameById(_ZoneIds[i])+ZoneNHExt;
				string	loadZ = CPath::lookup(filenameNH, false, false);
				if (!loadZ.empty())
				{
					CIFile	fileNH(loadZ);
					CZone	zoneNH;
					zoneNH.serial(fileNH);
					fileNH.close();
					if (zoneNH.getZoneId() != _ZoneIds[i])
					{
						nlwarning ("Zone %s ID is wrong. Abort.", filenameNH.c_str());
						return;
					}
					landscapeNoHm.addZone(zoneNH);
				}
				else
				{
					useNoHmZones = false;
				}
			}

			_ZonePtrs.push_back(landscape.getZone(_ZoneIds[i]));
		}

		landscape.setNoiseMode(false);
		landscape.checkBinds();

		if (useNoHmZones)
		{
			landscapeNoHm.setNoiseMode(false);
			landscapeNoHm.checkBinds();
		}

		BestFittingBBox.setCenter(CVector::Null);
		BestFittingBBox.setHalfSize(CVector::Null);

		// Compute best fitting bbox
		for (i=0; i<_ZoneIds.size(); ++i)
			if (_ZoneIds[i] == CentralZoneId)
				BestFittingBBox = _ZonePtrs[i]->getZoneBB().getAABBox();

		CAABBox	enlBBox = BestFittingBBox;
		enlBBox.setHalfSize(enlBBox.getHalfSize()+CVector(8.0f, 8.0f, 1000.0f));

		// Add neighbor patch
		for (i=0; i<_ZoneIds.size(); ++i)
		{
			if (_ZoneIds[i] == CentralZoneId)
			{
				for (j=0; (sint)j<_ZonePtrs[i]->getNumPatchs(); ++j)
				{
					landscape.excludePatchFromRefineAll(_ZoneIds[i], j, false);
					if (useNoHmZones)
						landscapeNoHm.excludePatchFromRefineAll(_ZoneIds[i], j, false);
				}
				if (Verbose)
					nlinfo(" - selected %d/%d patches for zone %d", _ZonePtrs[i]->getNumPatchs(), _ZonePtrs[i]->getNumPatchs(), _ZoneIds[i]);
			}
			else
			{
				uint	nump = 0;
				for (j=0; (sint)j<_ZonePtrs[i]->getNumPatchs(); ++j)
				{
					CAABBox	pbox = _ZonePtrs[i]->getPatch(j)->buildBBox();
					bool	inters = enlBBox.intersect(pbox);

					if (inters)
					{
						landscape.excludePatchFromRefineAll(_ZoneIds[i], j, false);
						if (useNoHmZones)
							landscapeNoHm.excludePatchFromRefineAll(_ZoneIds[i], j, false);
						++nump;
					}
					else
					{
						landscape.excludePatchFromRefineAll(_ZoneIds[i], j, true);
						if (useNoHmZones)
							landscapeNoHm.excludePatchFromRefineAll(_ZoneIds[i], j, true);
					}
				}
				if (Verbose)
					nlinfo(" - selected %d/%d patches for zone %d", nump, _ZonePtrs[i]->getNumPatchs(), _ZoneIds[i]);
			}
		}

		// tessellate the landscape, get the leaves (the tessellation faces), and convert them
		// into surf elements
		if (Verbose)
			nlinfo("Compute landscape tessellation");

		if (Verbose)
			nlinfo("   - tessellate landscape");

		if (useNoHmZones)
		{
			landscapeNoHm.setThreshold(0.0f);
			landscapeNoHm.setTileMaxSubdivision(TessellateLevel);
			landscapeNoHm.refineAll(CVector::Null);
			landscapeNoHm.averageTesselationVertices();

			// get the faces
			vector<const CTessFace *>	leavesNoHm;
			landscapeNoHm.getTessellationLeaves(leavesNoHm);

			for (el=0; el<(sint)leavesNoHm.size(); ++el)
			{
				const CTessFace	*face = leavesNoHm[el];
				const CVector	*v[3];

				// get the vertices of the face
				v[0] = &(face->VBase->EndPos);
				v[1] = &(face->VLeft->EndPos);
				v[2] = &(face->VRight->EndPos);

				normals.push_back( ((*(v[1])-*(v[0])) ^ (*(v[2])-*(v[0]))).normed() );

				vectorCheck.push_back(*(v[0]));
				vectorCheck.push_back(*(v[1]));
				vectorCheck.push_back(*(v[2]));
			}
		}
	}

	landscape.setThreshold(0.0f);
	landscape.setTileMaxSubdivision(TessellateLevel);
	landscape.refineAll(CVector::Null);
	landscape.averageTesselationVertices();

	vector<const CTessFace *>	leaves;
	landscape.getTessellationLeaves(leaves);
	if (Verbose)
	{
		if (useNoHmZones)
			nlinfo("      - used no height map zones");
		nlinfo("      - generated %d leaves", leaves.size());
	}

	if (!useNoHmZones)
	{
		for (el=0; el<(sint)leaves.size(); ++el)
		{
			const CTessFace	*face = leaves[el];
			const CVector	*v[3];

			// get the vertices of the face
			v[0] = &(face->VBase->EndPos);
			v[1] = &(face->VLeft->EndPos);
			v[2] = &(face->VRight->EndPos);

			normals.push_back( ((*(v[1])-*(v[0])) ^ (*(v[2])-*(v[0]))).normed() );

			vectorCheck.push_back(*(v[0]));
			vectorCheck.push_back(*(v[1]));
			vectorCheck.push_back(*(v[2]));
		}
	}

	nlassert(normals.size() == leaves.size());

	// generate a vector of vertices and of surf element
	hash_map<const CVector *, uint32, CHashPtr<const CVector> >				vremap;
	hash_map<const CVector *, uint32, CHashPtr<const CVector> >::iterator	vremapit;
	hash_map<const CTessFace *, CSurfElement *, CHashPtr<const CTessFace> >	fremap;
	hash_map<const CTessFace *, CSurfElement *, CHashPtr<const CTessFace> >::iterator	fremapit;
	_Vertices.clear();
	_Tessellation.resize(leaves.size());

	if (Verbose)
		nlinfo("   - make and remap surface elements");

	for (el=0; el<(sint)leaves.size(); ++el)
		fremap[leaves[el]] = &(_Tessellation[el]);

	uint	check = 0;

	float	dist, maxdist = 0.0f;

	for (el=0; el<(sint)leaves.size(); ++el)
	{
		const CTessFace	*face = leaves[el];
		const CVector	*v[3];

		CSurfElement	&element = _Tessellation[el];

		// setup zone id
		element.ZoneId = face->Patch->getZone()->getZoneId();

		// get the vertices of the face
		v[0] = &(face->VBase->EndPos);
		v[1] = &(face->VLeft->EndPos);
		v[2] = &(face->VRight->EndPos);

		{
			CVector	vcheck;

			vcheck = vectorCheck[check++] - *(v[0]);
			vcheck.z = 0;
			dist = vcheck.norm();
			if (dist > maxdist)	maxdist = dist;
			//nlassert(vcheck.norm() < 0.1f);

			vcheck = vectorCheck[check++] - *(v[1]);
			vcheck.z = 0;
			dist = vcheck.norm();
			if (dist > maxdist)	maxdist = dist;
			//nlassert(vcheck.norm() < 0.1f);

			vcheck = vectorCheck[check++] - *(v[2]);
			vcheck.z = 0;
			dist = vcheck.norm();
			if (dist > maxdist)	maxdist = dist;
			//nlassert(vcheck.norm() < 0.1f);
		}

		//element.Normal = ((*(v[1])-*(v[0])) ^ (*(v[2])-*(v[0]))).normed();
		element.Normal = normals[el];


		// search the vertices in the map
		for (i=0; i<3; ++i)
		{
			// if doesn't exist, create a new vertex
			if ((vremapit = vremap.find(v[i])) == vremap.end())
			{
				element.Tri[i] = _Vertices.size();
				_Vertices.push_back(*(v[i]));
				vremap.insert(make_pair(v[i], element.Tri[i]));
			}
			// else use previous
			else
			{
				element.Tri[i] = vremapit->second;
			}
		}

		// setup the vertices pointer
		element.Vertices = &_Vertices;

		CTessFace		*edge[3];

		edge[0] = face->FBase;
		edge[1] = face->FRight;
		edge[2] = face->FLeft;

		for (i=0; i<3; ++i)
		{
			fremapit = fremap.find(edge[i]);
			element.EdgeLinks[i] = (fremapit != fremap.end() ? fremapit->second : NULL);
		}
	}

	for (el=0; el<(sint)_Tessellation.size(); ++el)
	{
		// add the element to the list of valid elements
		Elements.push_back(&(_Tessellation[el]));
	}

	landscape.clear();
}





void	NLPACS::CZoneTessellation::compile()
{
	sint	el;
	uint	i;

	CAABBox	tbox = computeBBox();

	bool	HasInvertedUnderWater = false;

	// setup cliffs
	for (el=0; el<(sint)Elements.size(); ++el)
	{
		CSurfElement	&element = *(Elements[el]);

		// a cliff ?
		if (element.Normal.z < 0.0)
		{
			CVector		&v0 = _Vertices[element.Tri[0]],
						&v1 = _Vertices[element.Tri[1]],
						&v2 = _Vertices[element.Tri[2]];

			uint8		bits0 = PrimChecker.get((uint)v0.x, (uint)v0.y);
			uint8		bits1 = PrimChecker.get((uint)v1.x, (uint)v1.y);
			uint8		bits2 = PrimChecker.get((uint)v2.x, (uint)v2.y);

			bool		w0 = ((bits0&CPrimChecker::Water) != 0);
			bool		w1 = ((bits1&CPrimChecker::Water) != 0);
			bool		w2 = ((bits2&CPrimChecker::Water) != 0);

			if ((bits0 & CPrimChecker::Water)!=0 || (bits1 & CPrimChecker::Water)!=0 || (bits2 & CPrimChecker::Water)!=0)
			{
				uint		ws;

				uint16		ws0 = PrimChecker.index((uint)v0.x, (uint)v0.y);
				uint16		ws1 = PrimChecker.index((uint)v1.x, (uint)v1.y);
				uint16		ws2 = PrimChecker.index((uint)v2.x, (uint)v2.y);

				if ((w0 && w1 && ws0 == ws1) || (w0 && w2 && ws0 == ws2))
					ws = ws0;
				else if (w1 && w2 && ws1 == ws2)
					ws = ws1;
				else if (w0)
					ws = ws0;
				else if (w1)
					ws = ws1;
				else if (w2)
					ws = ws2;

				float		minz = std::min(_Vertices[element.Tri[0]].z, 
								   std::min(_Vertices[element.Tri[1]].z,
											_Vertices[element.Tri[2]].z));

				bool		exists;
				float		wh = PrimChecker.waterHeight(ws, exists)+WaterThreshold;

				// 
				if (minz <= wh)
				{
					CPolygon	p(v0, v1, v2);
					PrimChecker.renderBits(p, CPrimChecker::Cliff);

					HasInvertedUnderWater = true;
				}
			}
		}
	}

	if (HasInvertedUnderWater)
	{
		nlwarning("zone '%s' has reversed landscape under water", (getZoneNameById((uint16)CentralZoneId)+ZoneExt).c_str());
	}


	// compute elements features
	if (Verbose)
		nlinfo("compute elements quantas");
	for (el=0; el<(sint)Elements.size(); ++el)
	{
		CSurfElement	&element = *(Elements[el]);

		element.ElemId = el;
		element.computeQuantas(this);
	}

	if (ReduceSurfaces)
	{
		// optimizes the number of generated segments
		// it also smoothes a bit the surface border
		// it seems that 3 consecutive passes are optimal to reduce
		// nasty granularity
		if (Verbose)
			nlinfo("reduce surfaces");
		uint	i;
		sint	p;

		for (i=0; i<3; ++i)
		{
			for (p=0; p<(sint)Elements.size(); ++p)
			{
				CSurfElement	&e = *(Elements[p]);
				CSurfElement	&e0 = *e.EdgeLinks[0],
								&e1 = *e.EdgeLinks[1],
								&e2 = *e.EdgeLinks[2];

				if (e.IsMergable && &e0 != NULL && &e1 != NULL && &e2 != NULL &&
					e.ZoneId == e0.ZoneId &&
					e.ZoneId == e1.ZoneId &&
					e.ZoneId == e2.ZoneId &&
					!e.ForceInvalid)
				{
					// Strong optimization
					// merge the element quantas to the neighbors' quantas which are the most numerous
					// quantas are evaluated individually
					if (e0.IsValid && e1.IsValid)						e.IsValid = true;
					if (e1.IsValid && e2.IsValid)						e.IsValid = true;
					if (e0.IsValid && e2.IsValid)						e.IsValid = true;

					if (e0.QuantHeight == e1.QuantHeight)				e.QuantHeight = e0.QuantHeight;
					if (e1.QuantHeight == e2.QuantHeight)				e.QuantHeight = e1.QuantHeight;
					if (e0.QuantHeight == e2.QuantHeight)				e.QuantHeight = e2.QuantHeight;
/*
					if (e0.WaterShape == e1.WaterShape && e0.IsValid && e1.IsValid)	e.WaterShape = e0.WaterShape;
					if (e1.WaterShape == e2.WaterShape && e1.IsValid && e2.IsValid)	e.WaterShape = e1.WaterShape;
					if (e0.WaterShape == e2.WaterShape && e0.IsValid && e2.IsValid)	e.WaterShape = e2.WaterShape;
*/
				}
			}
		}

		for (p=0; p<(sint)Elements.size(); ++p)
		{
			CSurfElement	&e = *(Elements[p]);
			CSurfElement	&e0 = *e.EdgeLinks[0],
							&e1 = *e.EdgeLinks[1],
							&e2 = *e.EdgeLinks[2];

			if (&e != NULL && &e0 != NULL && &e1 != NULL && &e2 != NULL &&
				e.IsValid && e0.IsValid && e1.IsValid && e2.IsValid &&
				!e.IsUnderWater && e0.IsUnderWater && e1.IsUnderWater && e2.IsUnderWater)
			{
				nlwarning("isolated submerged element '%d' !", p);
			}
		}
	}

	// translates vertices to the local axis
	sint64	vx, vy, vz, tx, ty, tz;
	tx = float2Fixed(Translation.x);
	ty = float2Fixed(Translation.y);
	tz = float2Fixed(Translation.z);

	uint	p;
	for (i=0; i<(sint)_Vertices.size(); ++i)
	{
		vx = float2Fixed(_Vertices[i].x) + tx;
		vy = float2Fixed(_Vertices[i].y) + ty;
		vz = float2Fixed(_Vertices[i].z) + tz;
		_Vertices[i] = CVector(fixed2Float(vx), fixed2Float(vy), fixed2Float(vz));
	}

	BestFittingBBox.setCenter(BestFittingBBox.getCenter()+Translation);


	//
	//if (false)
	{
		//
		// first pass of flood fill
		// allow detecting landscape irregularities
		//

		if (Verbose)
			nlinfo("build and flood fill surfaces -- pass 1");
		uint32	surfId = 0; // + (ZoneId<<16);

		for (p=0; p<(sint)Elements.size(); ++p)
		{
			if (Elements[p]->SurfaceId == UnaffectedSurfaceId)
			{
				Surfaces.push_back(CComputableSurface());
				CComputableSurface	&surf = Surfaces.back();

				surf.BorderKeeper = &Borders;
				surf.floodFill(Elements[p], surfId++, CSurfElemCompareSimple(), this);
				surf.BBox = BestFittingBBox;

				bool	force = false;

				if (surf.Area < 30.0f && surf.Elements.size() > 0)
				{
					uint		i;
					CAABBox		aabbox;

					aabbox.setCenter((*surf.Elements[0]->Vertices)[surf.Elements[0]->Tri[0]]);

					for (i=0; i<surf.Elements.size(); ++i)
					{
						aabbox.extend((*surf.Elements[i]->Vertices)[surf.Elements[i]->Tri[0]]);
						aabbox.extend((*surf.Elements[i]->Vertices)[surf.Elements[i]->Tri[1]]);
						aabbox.extend((*surf.Elements[i]->Vertices)[surf.Elements[i]->Tri[2]]);
					}

					// swap all suface elements validity
					if (!surf.Elements[0]->ForceInvalid && aabbox.getHalfSize().z < 1.5f)
					{
						for (i=0; i<surf.Elements.size(); ++i)
						{
							surf.Elements[i]->IsValid = !surf.Elements[i]->IsValid;
						}
						if (Verbose)
							nlinfo("Reverted surface %d (%d elements, water=%d)", surfId-1, surf.Elements.size(), (surf.IsUnderWater ? 1 : 0));
					}
				}
			}
		}

		Surfaces.clear();
		ExtSurfaces.clear();
	}

	vector<CSurfElement*>	elDup;
	for (el=0; el<(sint)Elements.size(); ++el)
		if (Elements[el]->IsValid)
			elDup.push_back(Elements[el]);
	Elements = elDup;
	elDup.clear();

	for (el=0; el<(sint)Elements.size(); ++el)
	{
		CSurfElement	&element = *(Elements[el]);
		element.SurfaceId = UnaffectedSurfaceId;
		uint	i;
		for (i=0; i<3; ++i)
			if (element.EdgeLinks[i] != NULL && !element.EdgeLinks[i]->IsValid)
				element.EdgeLinks[i] = NULL;
	}

	//
	{
		if (Verbose)
			nlinfo("build and flood fill surfaces");
		uint32	surfId = 0; // + (ZoneId<<16);
		uint	totalSurf = 0;
		sint32	extSurf = -1024;

		for (p=0; p<(sint)Elements.size(); ++p)
		{
			if (Elements[p]->SurfaceId == UnaffectedSurfaceId)
			{
				bool	elInCentral = (Elements[p]->ZoneId == CentralZoneId);

				++totalSurf;
				sint32	thisSurfId = (elInCentral) ? surfId++ : extSurf--;
				if (elInCentral)
					Surfaces.push_back(CComputableSurface());
				else
					ExtSurfaces.push_back(CComputableSurface());

				CComputableSurface	&surf = (elInCentral) ? Surfaces.back() : ExtSurfaces.back();

				surf.BorderKeeper = &Borders;
				surf.floodFill(Elements[p], thisSurfId, CSurfElemCompareNormal(), this);
				surf.BBox = BestFittingBBox;
			}
		}

		if (Verbose)
		{
			nlinfo("%d surfaces generated", totalSurf);

			for (p=0; p<Surfaces.size(); ++p)
			{
				nlinfo("surf %d: %d elements", p, Surfaces[p].Elements.size());

				if (Surfaces[p].Elements.size() == 1)
				{
					nlinfo("elm: %d", Surfaces[p].Elements[0]->ElemId);
				}
			}
		}
	}

	// flag vertices that are pointed by more than 2 surfaces
	VerticesFlags.resize(_Vertices.size(), 0);

	for (p=0; p<(sint)Elements.size(); ++p)
	{
		CSurfElement	*elem = Elements[p];

		sint32		s = elem->SurfaceId;
		sint32		s0 = (elem->EdgeLinks[0] != NULL ? elem->EdgeLinks[0]->SurfaceId : UnaffectedSurfaceId);
		sint32		s1 = (elem->EdgeLinks[1] != NULL ? elem->EdgeLinks[1]->SurfaceId : UnaffectedSurfaceId);
		sint32		s2 = (elem->EdgeLinks[2] != NULL ? elem->EdgeLinks[2]->SurfaceId : UnaffectedSurfaceId);

		if (s != s0 && s != s1 && s0 != s1)
		{
			VerticesFlags[elem->Tri[2]] = 1;
		}

		if (s != s1 && s != s2 && s1 != s2)
		{
			VerticesFlags[elem->Tri[0]] = 1;
		}

		if (s != s2 && s != s0 && s2 != s0)
		{
			VerticesFlags[elem->Tri[1]] = 1;
		}
	}
}




void	NLPACS::CZoneTessellation::generateBorders(float smooth)
{
	sint	surf;

	if (Verbose)
		nlinfo("generate tessellation borders");
	// for each surface, build its border
	for (surf=0; surf<(sint)Surfaces.size(); ++surf)
		Surfaces[surf].buildBorders(this);

	// then, for each border, link the related surfaces...
	if (Verbose)
		nlinfo("smooth borders");
	sint	border;
	sint	totalBefore = 0,
			totalAfter = 0;
	for (border=0; border<(sint)Borders.size(); ++border)
	{
		float	smScale = (Borders[border].Right < 0) ? 0.2f : 1.0f;
		uint	before = Borders[border].Vertices.size();
		if (SmoothBorders && !Borders[border].DontSmooth)
		{
			Borders[border].smooth(smooth*smScale);
		}
		Borders[border].computeLength();
		uint	after = Borders[border].Vertices.size();
		totalBefore += before;
		totalAfter += after;
	}
	if (Verbose)
		nlinfo("smooth process: %d -> %d (%.1f percent reduction)", totalBefore, totalAfter, 100.0*(1.0-(double)totalAfter/(double)totalBefore));
}





void	NLPACS::CZoneTessellation::saveTessellation(COFile &output)
{
	output.serialCont(_Vertices);

	uint	i;

	for (i=0; i<_Tessellation.size(); ++i)
		_Tessellation[i].ElemId = i;

	uint32	numTessel = _Tessellation.size();
	output.serial(numTessel);

	for (i=0; i<_Tessellation.size(); ++i)
	{
		_Tessellation[i].serial(output, _Tessellation);
	}
}




void	NLPACS::CZoneTessellation::loadTessellation(CIFile &input)
{
	input.serialCont(_Vertices);

	uint	i;

	uint32	numTessel;
	input.serial(numTessel);
	_Tessellation.resize(numTessel);

	for (i=0; i<_Tessellation.size(); ++i)
	{
		_Tessellation[i].serial(input, _Tessellation);
	}

	Elements.resize(_Tessellation.size());
	for (i=0; i<(sint)_Tessellation.size(); ++i)
	{
		Elements[i] = &_Tessellation[i];
	}
}

void	NLPACS::CZoneTessellation::clear()
{
	_ZoneIds.clear();
	_ZonePtrs.clear();
	_Tessellation.clear();
	_Vertices.clear();
	Elements.clear();
	Surfaces.clear();
	Borders.clear();
}

CAABBox	NLPACS::CZoneTessellation::computeBBox() const
{
	CAABBox		zbox;
	bool		set = false;
	uint		i;

	if (_Vertices.size() == 0)
		return zbox;

	zbox.setCenter(_Vertices[0]);

	for (i=1; i<_Vertices.size(); ++i)
		zbox.extend(_Vertices[i]);

	return zbox;
}