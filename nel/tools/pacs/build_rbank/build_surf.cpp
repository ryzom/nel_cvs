/** \file build_surf.cpp
 *
 *
 * $Id: build_surf.cpp,v 1.14 2003/08/27 09:23:07 legros Exp $
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



NLPACS::CStats	NLPACS::StatsSurfaces;




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
bool	isInside(vector<CPlane> &top, NLPACS::CSurfElement &element)
{
	static CVector	t1[32], t2[32];

	CVector	*a, *b;
	uint	na, nb;
	uint	i;

	a = t1;
	b = t2;

	a[0] = (*element.Vertices)[element.Tri[0]];
	a[1] = (*element.Vertices)[element.Tri[1]];
	a[2] = (*element.Vertices)[element.Tri[2]];
	na = 3;

	for (i=0; i<top.size(); ++i)
	{
		nb = top[i].clipPolygonBack(a, b, na);
		if (nb < 3)
			return false;

		swap(a, b);
		swap(na, nb);
	}
	return true;
}

void	cut(vector<CPlane> &top, NLPACS::CSurfElement &element, vector<CVector> &res)
{
	static CVector	t1[32], t2[32];

	CVector	*a, *b;
	uint	na, nb;
	uint	i;

	a = t1;
	b = t2;

	a[0] = (*element.Vertices)[element.Tri[0]];
	a[1] = (*element.Vertices)[element.Tri[1]];
	a[2] = (*element.Vertices)[element.Tri[2]];
	na = 3;

	for (i=0; i<top.size(); ++i)
	{
		nb = top[i].clipPolygonBack(a, b, na);
		swap(a, b);
		swap(na, nb);
	}

	res.clear();
	for (i=0; i<na; ++i)
		res.push_back(a[i]);
}

void	cut(CPlane &plane, const vector<CVector> &source, vector<CVector> &destination)
{
	destination.clear();
	if (source.empty())
		return;

	bool	lastInside = (plane*source[0] <= 0.0f);
	bool	thisInside;

	if (lastInside)
		destination.push_back(source[0]);

	uint	i;

	for (i=1; i<source.size(); ++i)
	{
		thisInside = (plane*source[i] <= 0.0f);

		if (thisInside && !lastInside || !thisInside && lastInside)
		{
			destination.push_back(plane.intersect(source[i-1], source[i]));
			float	d = ((destination.back()-source[i-1])*(source[i]-source[i-1]))/(source[i]-source[i-1]).sqrnorm();
			nlassert(d>=0.0f);
			nlassert(d<=1.0f);
		}

		if (thisInside)
			destination.push_back(source[i]);

		lastInside = thisInside;
	}
}

bool	cut(CPlane &plane, const vector<CVector> &source, vector<vector<CVector> > &destination)
{
	if (source.empty())
		return true;

	bool	lastInside = (plane*source[0] <= 0.0f);
	bool	thisInside;
	bool	cutFlag= false;

	if (lastInside)
	{
		destination.resize(destination.size()+1);
		destination.back().push_back(source[0]);
	}

	uint	i;

	for (i=1; i<source.size(); ++i)
	{
		thisInside = (plane*source[i] <= 0.0f);

		if (thisInside && !lastInside || !thisInside && lastInside)
		{
			if (thisInside && !lastInside)
				destination.resize(destination.size()+1);
			destination.back().push_back(plane.intersect(source[i-1], source[i]));
			float	d = ((destination.back().back()-source[i-1])*(source[i]-source[i-1]))/(source[i]-source[i-1]).sqrnorm();
			nlassert(d>=0.0f);
			nlassert(d<=1.0f);
			cutFlag = true;
		}

		if (thisInside)
			destination.back().push_back(source[i]);

		lastInside = thisInside;
	}

	return cutFlag;
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
 * CStats methods...
 */

void NLPACS::CStats::init()
{
	uint	i;

	for (i=0; i<255; ++i)
	{
		XBBSpanList.push_back(pair<float,uint>((float)pow(1.04,(double)i)-1.0f,0));
		YBBSpanList.push_back(pair<float,uint>((float)pow(1.04,(double)i)-1.0f,0));
		XBBSpan.push_back(pair<float,uint>((float)pow(1.04,(double)i)-1.0f,0));
		YBBSpan.push_back(pair<float,uint>((float)pow(1.04,(double)i)-1.0f,0));
	}

	XBBSpanList.push_back(pair<float,uint>(FLT_MAX,0));
	YBBSpanList.push_back(pair<float,uint>(FLT_MAX,0));
	XBBSpan.push_back(pair<float,uint>(FLT_MAX,0));
	YBBSpan.push_back(pair<float,uint>(FLT_MAX,0));

	TotalSpanList = 0;
	TotalSpan = 0;
}














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

void	NLPACS::CSurfElement::computeElevation(vector<CPlane> &elevation, float radius, float height, float floorThreshold)
{
	uint		i;
	CVector		tri[3];
	CVector		pos[6];
	CVector		normals[6];

	for (i=0; i<3; ++i)
		tri[i] = (*Vertices)[Tri[i]];

	for (i=0; i<3; ++i)
		normals[i] = ((tri[(i+2)%3]-tri[(i+1)%3])^Normal).normed();

	for (i=3; i<6; ++i)
		normals[i] = (normals[(i+2)%3]+normals[(i+1)%3]).normed();

	for (i=0; i<3; ++i)
		pos[i] = tri[(i+1)%3]+normals[i]*radius;

	for (i=3; i<6; ++i)
		pos[i] = tri[i-3]+normals[i]*radius;

	elevation.resize(8);
	for (i=0; i<6; ++i)
		elevation[i].make(normals[i], pos[i]);

	elevation[i++].make(-Normal, tri[0]+Normal*floorThreshold);
	elevation[i++].make(Normal, tri[0]+Normal*height);
}




void	NLPACS::CSurfElement::computeQuantas()
{
	CVector		v0 = (*Vertices)[Tri[0]],
				v1 = (*Vertices)[Tri[1]],
				v2 = (*Vertices)[Tri[2]];
	
	CVector	n = (v1-v0) ^ (v2-v0);
	Normal = n.normed();

	CAABBox		zbbox = Root->RootZoneTessellation->OriginalBBox;
	zbbox.setHalfSize(zbbox.getHalfSize()-CVector(1.0e-2f, 1.0e-2f, 0.0f));

//	bool	hasInside, hasOutside;
//	hasInside = zbbox.include(v0) || zbbox.include(v1) || zbbox.include(v2);
//	hasOutside = !zbbox.include(v0) || !zbbox.include(v1) || !zbbox.include(v2);
//	IsMergable = hasInside && !hasOutside;

	QuantHeight = ((uint8)(floor((v0.z+v1.z+v2.z)/6.0f)))%255;
	
	Area = 0.5f*n.norm();

/*
	uint	i;
	for (i=0; i<NumCreatureModels-1; ++i)
		if (Normal.z>=Models[i][ModelInclineThreshold] &&
			Normal.z<Models[i+1][ModelInclineThreshold])
			break;

	NormalQuanta = NumCreatureModels-1-i;
*/

	NormalQuanta = (Normal.z > 0.707f ? 0 : 1);

/*
	if (NormalQuanta == 0 || NormalQuanta == NumNormalQuantas-1)
	{
		OrientationQuanta = 0;
	}
	else
	{
		float	r = (float)sqrt(Normal.x*Normal.x+Normal.y*Normal.y);
		float	psi = (float)atan2(Normal.x/r, Normal.y/r);

		OrientationQuanta = (int)(NumOrientationQuantas*psi/6.283186f);
	}

	if (NormalQuanta<0)							NormalQuanta = 0;
	if (NormalQuanta>=NumNormalQuantas)			NormalQuanta = NumNormalQuantas-1;
	if (OrientationQuanta<0)					OrientationQuanta = 0;
	if (OrientationQuanta>=NumNormalQuantas)	OrientationQuanta = NumOrientationQuantas-1;
*/

	OrientationQuanta = 0;
	IsHorizontal = (NormalQuanta == 0);
	IsValid = IsHorizontal;

	uint8	bits = 0;
	bits |= PrimChecker.get((uint)v0.x, (uint)v0.y);
	bits |= PrimChecker.get((uint)v1.x, (uint)v1.y);
	bits |= PrimChecker.get((uint)v2.x, (uint)v2.y);

	if (bits & CPrimChecker::Include)
	{
		IsValid = true;
		IsHorizontal = true;
		NormalQuanta = 0;
	}

	if (bits & CPrimChecker::Exclude)
	{
		IsValid = false;
		IsHorizontal = false;
		NormalQuanta = 1;
	}

	if (bits & CPrimChecker::ClusterHint && IsValid)
		ClusterHint = true;

	Material = 0;
	Level = 0;
	Character = 0;

	if (ComputeElevation)
	{
		/* the maximal cosine of the angle between the normal of the element and the 
		 * normal of the potential obstacle. Beyond that value, the obstacle isn't considered
		 * as an obstacle.
		 */
		const float	normalThreshold = 0.5f;

		if (Root->Selected.size() > 0)
		{
			vector<CPlane>	elevation;
			sint			model;
			uint			sel;
			bool			found = false;

			for (model=NumCreatureModels-1; model>=0; --model)
			{
				if (Normal.z < Models[model][ModelInclineThreshold])
					continue;

				computeElevation(elevation,
								 Models[model][ModelRadius],
								 Models[model][ModelHeight],
								 0.2f);

				for (sel=0; sel<Root->Selected.size(); ++sel)
				{
					if (isInside(elevation, *(Root->Selected[sel])) &&
						Normal*Root->Selected[sel]->Normal < normalThreshold)
					{
						found = true;
						break;
					}
				}

				if (!found)
					break;
			}

			Character = model;
		}
		else
		{
			Character = NumCreatureModels-1;
		}
	}

	CVector	vmin;
	CVector	vmax;
	vmin.minof((*Vertices)[Tri[0]], (*Vertices)[Tri[1]]);
	vmin.minof(vmin, (*Vertices)[Tri[2]]);
	vmax.maxof((*Vertices)[Tri[0]], (*Vertices)[Tri[1]]);
	vmax.maxof(vmax, (*Vertices)[Tri[2]]);

	Root->RootZoneTessellation->WaterGrid.select(vmin, vmax);

	CQuadGrid<uint32>::CIterator	it = Root->RootZoneTessellation->WaterGrid.begin();
	bool	inWater = false;

	for (; it!=Root->RootZoneTessellation->WaterGrid.end(); ++it)
	{
		uint	shape = (*it);
		CPolygon	&poly = Root->RootZoneTessellation->WaterShapes[shape];
		if (isInside(poly, *this) &&
			((*Vertices)[Tri[0]].z < poly.Vertices[0].z || (*Vertices)[Tri[0]].z < poly.Vertices[1].z || (*Vertices)[Tri[0]].z < poly.Vertices[2].z))
		{
			WaterShape = shape;
			break;
		}
	}
}

CAABBox	NLPACS::CSurfElement::getBBox() const
{
	CAABBox	box;
	box.setCenter((*Vertices)[Tri[0]]);
	box.extend((*Vertices)[Tri[1]]);
	box.extend((*Vertices)[Tri[2]]);
	return box;
}


void	NLPACS::CSurfElement::computeLevel(CQuadGrid<CSurfElement *> &grid)
{
	if (!IsHorizontal)
	{
		Level = 0;
		return;
	}

	CPolygon		tri1;
	CPolygon		tri2;

	tri1.Vertices.push_back((*Vertices)[Tri[0]]);
	tri1.Vertices.push_back((*Vertices)[Tri[1]]);
	tri1.Vertices.push_back((*Vertices)[Tri[2]]);

	vector<CPlane>	planes;
	planes.resize(3);

	CVector	norm;

	norm = ((tri1.Vertices[1]-tri1.Vertices[0])^CVector::K).normed();
	planes[0].make(norm, tri1.Vertices[0]+norm*0.05f);
	norm = ((tri1.Vertices[2]-tri1.Vertices[1])^CVector::K).normed();
	planes[1].make(norm, tri1.Vertices[1]+norm*0.05f);
	norm = ((tri1.Vertices[0]-tri1.Vertices[2])^CVector::K).normed();
	planes[2].make(norm, tri1.Vertices[2]+norm*0.05f);

	uint			level = 0;
	CVector			center = ((*Vertices)[Tri[0]]+(*Vertices)[Tri[1]]+(*Vertices)[Tri[2]])/3.0f;

	CAABBox			centerBox = getBBox();
	centerBox.setHalfSize(centerBox.getHalfSize()+CVector(0.1f, 0.1f, 0.40f));

	grid.select(centerBox.getMin(), centerBox.getMax());
	CQuadGrid<CSurfElement *>::CIterator	it;

	CVector		emin = centerBox.getMin(),
				emax = centerBox.getMax();
	
	vector<CAABBox>	checkedBoxes;
	uint			i;

	for (it=grid.begin(); it!=grid.end(); ++it)
	{
		CSurfElement	&el = *(*it);
		if (&el == this)
			continue;

		const CVector	&V0 = (*(el.Vertices))[el.Tri[0]],
						&V1 = (*(el.Vertices))[el.Tri[1]],
						&V2 = (*(el.Vertices))[el.Tri[2]];

		CAABBox			box = el.getBBox();

		box.setHalfSize(box.getHalfSize()+CVector(0.1f, 0.1f, 0.1f));

		CVector			bmin = box.getMin(),
						bmax = box.getMax();

		// only keep elements in surroundings
		if (bmin.x > emax.x || bmin.y > emax.y || bmax.x < emin.x || bmax.y < emin.y)
			continue;

		// drop patch neighbors
		if (centerBox.intersect(box))
			continue;
/*
		// drop higher elements
		if (bmin.z > centerBox.getCenter().z+centerBox.getHalfSize().z)
			continue;
*/
		for (i=0; i<checkedBoxes.size(); ++i)
			if (checkedBoxes[i].intersect(V0, V1, V2))
				break;

		if (i<checkedBoxes.size())
		{
			checkedBoxes[i].extend(V0);
			checkedBoxes[i].extend(V1);
			checkedBoxes[i].extend(V2);
			continue;
		}

		// clip elemnts by elevation
		tri2.Vertices.clear();
		tri2.Vertices.push_back(V0);
		tri2.Vertices.push_back(V1);
		tri2.Vertices.push_back(V2);
		tri2.clip(planes);

		// if intersection not empty, keep box
		if (!tri2.Vertices.empty())
			checkedBoxes.push_back(box);
	}

	bool	merge;
	do
	{
		merge = false;
		vector<CAABBox>::iterator	it1, it2;
		for (it1=checkedBoxes.begin(); it1!=checkedBoxes.end(); ++it1)
		{
			it2 = it1;
			++it2;
			for (; it2!=checkedBoxes.end(); )
			{
				if ((*it1).intersect(*it2))
				{
					(*it1).extend((*it2).getMin());
					(*it1).extend((*it2).getMax());
					it2 = checkedBoxes.erase(it2);
					merge = true;
				}
				else
				{
					++it2;
				}
			}
		}
	}
	while (merge);

//	Level = level;
	Level = checkedBoxes.size();
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

	nldebug("smoothed border %d-%d: %d -> %d", Left, Right, before, after);
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

void	NLPACS::CComputableSurface::floodFill(CSurfElement *first, sint32 surfId)
{
	nldebug("flood fill surface %d", surfId);
	
	vector<CSurfElement *>	stack;
	sint					i;

	stack.push_back(first);
	first->SurfaceId = surfId;

	SurfaceId = surfId;
	Material = first->Material;
	OrientationQuanta = first->OrientationQuanta;
	NormalQuanta = first->NormalQuanta;
	Character = first->Character;
	Level = first->Level;
	IsHorizontal = first->IsHorizontal;
	ClusterHint = first->ClusterHint;
	QuantHeight = first->QuantHeight;
	uint	waterShape = first->WaterShape;

	IsUnderWater = (first->WaterShape != 255);
	WaterHeight = IsUnderWater ? first->Root->RootZoneTessellation->WaterShapes[first->WaterShape].Vertices[0].z : 123456.0f;


	uint32	currentZoneId = first->Root->ZoneId;

	Area = 0.0;

	while (!stack.empty())
	{
		CSurfElement	*pop = stack.back();
		stack.pop_back();
		Elements.push_back(pop);
		Area += pop->Area;

		for (i=0; i<3; ++i)
		{
			if (pop->EdgeLinks[i] != NULL &&
				pop->EdgeLinks[i]->IsValid &&
				pop->EdgeLinks[i]->ClusterHint == ClusterHint &&
				pop->EdgeLinks[i]->SurfaceId == UnaffectedSurfaceId &&
				pop->EdgeLinks[i]->Material == Material &&
				pop->EdgeLinks[i]->OrientationQuanta == OrientationQuanta &&
				pop->EdgeLinks[i]->NormalQuanta == NormalQuanta &&
				pop->EdgeLinks[i]->Character == Character &&
				pop->EdgeLinks[i]->Level == Level &&
				pop->EdgeLinks[i]->Root->ZoneId == currentZoneId &&
				pop->EdgeLinks[i]->WaterShape == waterShape &&
				pop->EdgeLinks[i]->QuantHeight == QuantHeight)
			{
				pop->EdgeLinks[i]->SurfaceId = SurfaceId;
				stack.push_back(pop->EdgeLinks[i]);
			}
		}
	}

	if (Elements.size() == 1 &&
		Elements[0]->EdgeLinks[0] != NULL && Elements[0]->EdgeLinks[0]->NoLevelSurfaceId == NoLevelSurfaceId &&
		Elements[0]->EdgeLinks[1] != NULL && Elements[0]->EdgeLinks[1]->NoLevelSurfaceId == NoLevelSurfaceId &&
		Elements[0]->EdgeLinks[2] != NULL && Elements[0]->EdgeLinks[2]->NoLevelSurfaceId == NoLevelSurfaceId)
	{
		nlwarning("1 seperated element surface found");
	}

	nldebug("%d elements added", Elements.size());

	Center = CVector::Null;
	for (i=0; i<(sint)Elements.size(); ++i)
	{
		vector<CVector>	&vertices = *Elements[i]->Vertices;
		Center += (vertices[Elements[i]->Tri[0]]+vertices[Elements[i]->Tri[1]]+vertices[Elements[i]->Tri[2]]);
	}
	Center /= (float)(Elements.size()*3);
}

void	NLPACS::CComputableSurface::followBorder(CSurfElement *first, uint edge, uint sens, vector<CVector> &vstore, bool &loop)
{
	CSurfElement	*current = first;
	CSurfElement	*next = current->EdgeLinks[edge];
	current->EdgeFlag[edge] = true;

	const sint32	currentSurfId = current->SurfaceId;
	const sint32	oppositeSurfId = (next != NULL) ? next->SurfaceId : UnaffectedSurfaceId;
	const sint32	oppositeZid = current->ZoneLinks[edge];
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
			(oppositeSurfId == UnaffectedSurfaceId && (next != NULL && next->SurfaceId != currentSurfId || next == NULL && current->ZoneLinks[nextEdge] != oppositeZid)) ||
			(current->EdgeFlag[nextEdge] && !allowThis))
		{
			// if reaches the end of the border, then quits.
			loop = absoluteEquals(vstore.front(), vstore.back(), 1e-2f);
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

void	NLPACS::CComputableSurface::buildBorders()
{
	sint	elem, edge;

	nldebug("generate borders for the surface %d", SurfaceId);

	for (elem=0; elem<(sint)Elements.size(); ++elem)
	{
		// for each element,
		// scan for a edge that points to a different surface

		for (edge=0; edge<3; ++edge)
		{

			if ((Elements[elem]->EdgeLinks[edge] == NULL || Elements[elem]->EdgeLinks[edge]->SurfaceId != SurfaceId) &&
				!Elements[elem]->EdgeFlag[edge])
			{
				BorderKeeper->resize(BorderKeeper->size()+1);
				CComputableSurfaceBorder	&border = BorderKeeper->back();

				border.Left = Elements[elem]->SurfaceId;

				border.DontSmooth = (Elements[elem]->EdgeLinks[edge] != NULL && Elements[elem]->NoLevelSurfaceId == Elements[elem]->EdgeLinks[edge]->SurfaceId);

				if (Elements[elem]->EdgeLinks[edge] != NULL && Elements[elem]->EdgeLinks[edge]->Root->ZoneId != Elements[elem]->Root->ZoneId)
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

				nldebug("generate border %d (%d-%d)", BorderKeeper->size()-1, border.Left, border.Right);

				bool				loop;
				vector<CVector>		bwdVerts;
				vector<CVector>		&fwdVerts = border.Vertices;

				followBorder(Elements[elem], edge, 2, bwdVerts, loop);

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
					followBorder(Elements[elem], edge, 1, fwdVerts, loop);
				}
			}
		}
	}
}

void	NLPACS::CComputableSurface::computeHeightQuad()
{
	nldebug("generate height quad for surface %d", SurfaceId);
	HeightQuad.clear();
	HeightQuad.init(4.0f, 6, BBox.getCenter(), std::max(BBox.getHalfSize().x, BBox.getHalfSize().y));
	
	uint	i;
	for (i=0; i<Elements.size(); ++i)
	{
		CSurfElement	&element = *(Elements[i]);
		HeightQuad.addVertex((*element.Vertices)[element.Tri[0]]);
		HeightQuad.addVertex((*element.Vertices)[element.Tri[1]]);
		HeightQuad.addVertex((*element.Vertices)[element.Tri[2]]);
	}

	HeightQuad.compile();
}


















/*
 * CPatchTessellation constructors and methods implementation
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

void	NLPACS::CPatchTessellation::setup(const NL3D::CPatch *rootPatch, 
										const NL3D::CPatchInfo *rootPatchInfo,
										CZoneTessellation *rootZone,
										CPatchRetriever *rootRetriever,
										uint16 patchId,
										uint16 zoneId)
{
	RootPatch = rootPatch;
	RootPatchInfo = rootPatchInfo;
	RootZoneTessellation = rootZone;
	RootRetriever = rootRetriever;
	PatchId = patchId;
	ZoneId = zoneId;
	_NS = RootZoneTessellation->Refinement*RootPatch->getOrderS();
	_NT = RootZoneTessellation->Refinement*RootPatch->getOrderT();
	Valid = false;
	OriginalBBox = RootPatch->buildBBox();
	BBox = OriginalBBox;
	BBox.setCenter(BBox.getCenter()+rootZone->Translation);
}


// Selects triangle for later intersection with polytop's creature models
void	NLPACS::CPatchTessellation::selectElevation(vector<CSurfElement *> &selection)
{
	vector<CPlane>	top = vector<CPlane>(6);

	CVector			normal;
	CVector			verts[5] = 
	{
		RootPatchInfo->Patch.eval(0.0f, 0.0f),
		RootPatchInfo->Patch.eval(0.0f, 1.0f),
		RootPatchInfo->Patch.eval(1.0f, 1.0f),
		RootPatchInfo->Patch.eval(1.0f, 0.0f),
		RootPatchInfo->Patch.eval(0.5f, 0.5f)
	};

	/* enlarge the selection box */
	{
		CVector		overts[4] = { verts[0], verts[1], verts[2], verts[3] };
		CVector		directs[4];

		directs[0] = (verts[1]-verts[0]).normed();
		directs[1] = (verts[2]-verts[1]).normed();
		directs[2] = (verts[3]-verts[2]).normed();
		directs[3] = (verts[0]-verts[3]).normed();

		float		largest = Models[NumCreatureModels-1][ModelRadius];

		verts[0] = verts[0] + directs[3]*largest - directs[0]*largest;
		verts[1] = verts[1] + directs[0]*largest - directs[1]*largest;
		verts[2] = verts[2] + directs[1]*largest - directs[2]*largest;
		verts[3] = verts[3] + directs[2]*largest - directs[3]*largest;
	}

	normal = (RootPatchInfo->Patch.evalNormal(0.0f, 0.0f)+
			  RootPatchInfo->Patch.evalNormal(1.0f, 0.0f)+
			  RootPatchInfo->Patch.evalNormal(1.0f, 1.0f)+
			  RootPatchInfo->Patch.evalNormal(0.0f, 1.0f)+
			  RootPatchInfo->Patch.evalNormal(0.5f, 0.5f)*2.0).normed();

	/* use vertical normal for 'walk' landscape
	 * the threshold value we consider to be quasi vertical.
	 * it is actually the cosine of the angle between the normal and the vertical
	 */
	const float		normalThreshold = 0.707f;
	CVector			useNormal = normal;
	if (useNormal.z > normalThreshold)
		useNormal = CVector(0.0f, 0.0f, 1.0f);

	uint	i;

	for (i=0; i<4; ++i)
		top[i].make((verts[(i+1)%4]-verts[i])^useNormal, verts[i]);

	CVector	minv = verts[0];

	for (i=1; i<5; ++i)
		if ((verts[i]-minv)*normal < 0.0)
			minv = verts[i];

	top[4].make(-normal, minv);
	top[5].make(normal, minv+useNormal*20.0f);

	selection.clear();

	RootZoneTessellation->Container.select(top);

	CQuadTree<CSurfElement *>::CIterator	it;
	for (it=RootZoneTessellation->Container.begin(); it!=RootZoneTessellation->Container.end(); ++it)
	{
		CSurfElement	&t = **it;

		/* The threshold value for a triangle to be considered as a possible obstacle
		 * It is the cosine of the angle between the triangle normal and the whole patch normal
		 */
		const float		normalSkipThreshold = 0.5f;

		if (t.Normal*normal < normalSkipThreshold && isInside(top, t))
		{
			selection.push_back(*it);
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

	_Landscape.init();

	CMatrix		tmp;
	CVector		I(1,0,0);
	CVector		J(0,0,-1);
	CVector		K(0,1,0);
	tmp.identity();
	tmp.setRot(I,J,K, true);
	Container.changeBase (tmp);

	float		sz = std::max(OriginalBBox.getSize().x, OriginalBBox.getSize().y)+20.0f;
	ContBBox.setCenter(OriginalBBox.getCenter());
	ContBBox.setSize(CVector(sz, sz, OriginalBBox.getSize().z));
	Container.create(7, OriginalBBox.getCenter(), sz);

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


	uint	i, j;
	// setup the square of 9 zones...
	nldebug("setup zone tessellation %d %s", zoneId, getZoneNameById(zoneId).c_str());
	{
		uint	zx = zoneId%256, zy = zoneId/256;
		if (zx>0 && zy>0)		_Zones.push_back(CPatchRetriever(zoneId-257));
		if (zy>0)				_Zones.push_back(CPatchRetriever(zoneId-256));
		if (zx<255 && zy>0)		_Zones.push_back(CPatchRetriever(zoneId-255));
		if (zx>0)				_Zones.push_back(CPatchRetriever(zoneId-1));
		_Zones.push_back(CPatchRetriever(zoneId));
		if (zx<255)				_Zones.push_back(CPatchRetriever(zoneId+1));
		if (zx>0 && zy<255)		_Zones.push_back(CPatchRetriever(zoneId+255));
		if (zy<255)				_Zones.push_back(CPatchRetriever(zoneId+256));
		if (zx<255 && zy<255)	_Zones.push_back(CPatchRetriever(zoneId+257));

		// remove zones that don't exist
		vector<CPatchRetriever>::iterator	rit;
		for (rit=_Zones.begin(); rit!=_Zones.end(); )
		{
			string	filename = getZoneNameById(rit->ZoneId)+ZoneExt;
			try
			{
				if (CPath::lookup(filename, false, false) == "")
					rit = _Zones.erase(rit);
				else
					++rit;
			}
			catch (EPathNotFound &)
			{
				rit = _Zones.erase(rit);
			}
		}
	}
	
	// sort zones
	sort(_Zones.begin(), _Zones.end(), CRetrieverSort());

	// load the zones and select patchs that are inside the extended bbox
	for (i=0; i<_Zones.size(); ++i)
	{
		string	filename = getZoneNameById(_Zones[i].ZoneId)+ZoneExt;
		CIFile	file(CPath::lookup(filename));
		CZone	zone;
		zone.serial(file);
		file.close();
		if (zone.getZoneId() != _Zones[i].ZoneId)
		{
			nlwarning ("Zone %s ID is wrong. Abort.");
			return false;
		}
		else
		{
			nldebug("use zone %s %d", filename.c_str(), zone.getZoneId());
			_Landscape.addZone(zone);
			CPatchRetriever	&retriever = _Zones[i];
			retriever.Zone = _Landscape.getZone(_Zones[i].ZoneId);
			retriever.BBox = retriever.Zone->getZoneBB().getAABBox();
			retriever.BBox.setCenter(retriever.BBox.getCenter()+Translation);
			const_cast<CZone *>(retriever.Zone)->retrieve(retriever.PatchInfos, retriever.BorderVertices);


			// compute number of vertex used by this zone.
			retriever.MaxVertex= 0;
			for(j=0;j<(sint)retriever.PatchInfos.size();j++)
			{
				const CPatchInfo	&pi= retriever.PatchInfos[j];

				for(sint k=0;k<4;k++)
				{
					retriever.MaxVertex= max((uint32)pi.BaseVertices[k], retriever.MaxVertex);
				}
			}

			// init the remap table
			for (j=0; j<retriever.PatchInfos.size(); ++j)
				retriever.PatchRemap.push_back(-1);

			retriever.TotalNew = 0;
		}
	}

	_Landscape.setNoiseMode(false);

	_Landscape.checkBinds();

	BestFittingBBox.setCenter(CVector::Null);
	BestFittingBBox.setHalfSize(CVector::Null);

	// Compute best fitting bbox
	for (i=0; i<_Zones.size(); ++i)
		if (_Zones[i].ZoneId == CentralZoneId)
			BestFittingBBox = _Zones[i].Zone->getZoneBB().getAABBox();

	// Add neighbor patch
	for (i=0; i<_Zones.size(); ++i)
	{
		CPatchRetriever	&retriever = _Zones[i];
/*
		CAABBox	selectBBox;

		selectBBox.setCenter(CVector(17442, -24484, 4));
		selectBBox.setHalfSize(CVector(16, 16, 80));
*/
		if (_Zones[i].ZoneId == CentralZoneId)
		{
			for (j=0; j<retriever.PatchInfos.size(); ++j)
			{
				retriever.PatchRemap[j] = retriever.TotalNew++;
				_Landscape.excludePatchFromRefineAll(_Zones[i].ZoneId, j, false);
			}
		}
		else
		{
			CAABBox	enlBBox = BestFittingBBox;
			enlBBox.setHalfSize(enlBBox.getHalfSize()+CVector(8.0f, 8.0f, 0.0f));
			for (j=0; j<retriever.PatchInfos.size(); ++j)
			{
				CAABBox	pbox = retriever.Zone->getPatch(j)->buildBBox();
				if (enlBBox.intersect(pbox) && retriever.PatchRemap[j] == -1)
				{
					retriever.PatchRemap[j] = retriever.TotalNew++;
					_Landscape.excludePatchFromRefineAll(_Zones[i].ZoneId, j, false);
				}
				else
				{
					_Landscape.excludePatchFromRefineAll(_Zones[i].ZoneId, j, true);
				}
			}
		}
	}



	// setup patches
	uint	zone;
	for (zone=0; zone<_Zones.size(); ++zone)
	{
		CPatchRetriever	&retriever = _Zones[zone];
		sint	p;

		retriever.Patches.resize(retriever.Zone->getNumPatchs());

		for (p=0; p<(sint)retriever.Patches.size(); ++p)
		{
			const CPatch	*patch = retriever.Zone->getPatch(p);
			retriever.Patches[p].setup(patch, &retriever.PatchInfos[p], this, &retriever, p, retriever.ZoneId);
		}
	}

	return true;
}





void	NLPACS::CZoneTessellation::build()
{
	uint	zone;
	sint	el;

	// tessellate the landscape, get the leaves (the tessellation faces), and convert them
	// into surf elements
	{
		sint												i, j;
		nldebug("Compute landscape tessellation");

		nldebug("   - tessellate landscape");
		_Landscape.setThreshold(0.0f);
		_Landscape.setTileMaxSubdivision(TessellateLevel);
		_Landscape.refineAll(CVector::Null);
		_Landscape.averageTesselationVertices();

		// get the faces
		vector<const CTessFace *>	leaves;
		_Landscape.getTessellationLeaves(leaves);
		nldebug("      - generated %d leaves", leaves.size());

		// remap zone links and patch links
		map<const CPatch *, CPatchTessellation *>			premap;
		map<const CPatch *, CPatchTessellation *>::iterator	premapit;

		// setup the map to find the patch references
		for (zone=0; zone<(sint)_Zones.size(); ++zone)
		{
			CPatchRetriever	&retriever = _Zones[zone];
			for (j=0; j<retriever.Zone->getNumPatchs(); ++j)
				premap.insert(make_pair(retriever.Zone->getPatch(j), &retriever.Patches[j]));
		}

		// generate a vector of vertices and of surf element
		hash_map<const CVector *, uint32, CHashPtr<const CVector> >				vremap;
		hash_map<const CVector *, uint32, CHashPtr<const CVector> >::iterator	vremapit;
		hash_map<const CTessFace *, CSurfElement *, CHashPtr<const CTessFace> >	fremap;
		hash_map<const CTessFace *, CSurfElement *, CHashPtr<const CTessFace> >::iterator	fremapit;
		_Vertices.clear();
		_Tessellation.resize(leaves.size());

		nldebug("   - make and remap surface elements");

		for (el=0; el<(sint)leaves.size(); ++el)
		{
			const CTessFace	*face = leaves[el];
			const CVector	*v[3];

			CSurfElement	&element = _Tessellation[el];

			// add a new face in the map
			fremap.insert(make_pair(face, &element));

			// get the vertices of the face
			v[0] = &(face->VBase->EndPos);
			v[1] = &(face->VLeft->EndPos);
			v[2] = &(face->VRight->EndPos);

			element.Normal = ((*(v[1])-*(v[0])) ^ (*(v[2])-*(v[0]))).normed();

			// set the root patch
			premapit = premap.find(face->Patch);
			if (premapit != premap.end())
			{
				element.Root = premapit->second;
			}
			else
			{
				nlwarning("Couldn't remap root patch in CSurElement");
				element.Root = NULL;
			}

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

			for (i=0; i<3; ++i)
			{
				element.EdgeLinks[i] = NULL;
				element.ZoneLinks[i] = -1;
			}
		}

		// remap links to zone and neighbor elements
		nldebug("   - remap surface elements links");
		for (el=0; el<(sint)_Tessellation.size(); ++el)
		{
			CSurfElement	&element = _Tessellation[el];
			const CTessFace	*face = leaves[el];
			CTessFace		*edge[3];

			edge[0] = face->FBase;
			edge[1] = face->FRight;
			edge[2] = face->FLeft;

			for (i=0; i<3; ++i)
			{
				fremapit = fremap.find(edge[i]);
				if (fremapit != fremap.end())
				{
					element.EdgeLinks[i] = fremapit->second;
					element.ZoneLinks[i] = element.EdgeLinks[i]->Root->ZoneId;
				}
				else
				{
					element.EdgeLinks[i] = NULL;
				}
			}
		}
	}

	for (el=0; el<(sint)_Tessellation.size(); ++el)
	{
		// add the element to the list of valid elements
		CSurfElement	&element = _Tessellation[el];
		Elements.push_back(&element);
	}

	_Landscape.clear();
}





void	NLPACS::CZoneTessellation::compile()
{
	uint	zone;
	sint	el;

	nldebug("prepare elevation selection");
	for (zone=0; zone<_Zones.size(); ++zone)
	{
		CPatchRetriever	&retriever = _Zones[zone];
		sint	p;

		for (p=0; p<(sint)retriever.Patches.size(); ++p)
		{
			const CPatch	*patch = retriever.Zone->getPatch(p);
			retriever.Patches[p].setup(patch, &retriever.PatchInfos[p], this, &retriever, p, retriever.ZoneId);
			retriever.Patches[p].selectElevation(retriever.Patches[p].Selected);
			retriever.Patches[p].Valid = true;
		}
	}

	_Landscape.clear();

	for (el=0; el<(sint)_Tessellation.size(); ++el)
	{
		CSurfElement	&element = _Tessellation[el];
		const CVector	*v[3];

		// get the vertices of the face
		v[0] = &_Vertices[element.Tri[0]];
		v[1] = &_Vertices[element.Tri[1]];
		v[2] = &_Vertices[element.Tri[2]];

		// if the element is in the container, add it to the quadtree for later selection
		if (ContBBox.intersect(*v[0], *v[1], *v[2]))
			Container.insert(vmin(*v[0], *v[1], *v[2]), vmax(*v[0], *v[1], *v[2]), &element);
	}

	// setup water quad grid
	WaterGrid.create(512, 0.5f);
	uint	i, j;
	for (i=0; i<WaterShapes.size(); ++i)
	{
		CVector	vmin = WaterShapes[i].Vertices[0];
		CVector	vmax = vmin;
		for (j=1; j<WaterShapes[i].Vertices.size(); ++j)
		{
			vmin.minof(vmin, WaterShapes[i].Vertices[j]);
			vmax.maxof(vmax, WaterShapes[i].Vertices[j]);
		}

		WaterGrid.insert(vmin, vmax, i);
	}

	// compute elements features
	nldebug("compute elements quantas");
	for (el=0; el<(sint)Elements.size(); ++el)
	{
		CSurfElement	&element = *(Elements[el]);
		element.computeQuantas();
		element.ElemId = el;
	}

	if (ReduceSurfaces)
	{
		// optimizes the number of generated segments
		// it also smoothes a bit the surface border
		// it seems that 3 consecutive passes are optimal to reduce
		// nasty granularity
		nldebug("reduce surfaces");
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
					e.Root->ZoneId == e0.Root->ZoneId &&
					e.Root->ZoneId == e1.Root->ZoneId &&
					e.Root->ZoneId == e2.Root->ZoneId)
				{
					// Strong optimization
					// merge the element quantas to the neighbors' quantas which are the most numerous
					// quantas are evaluated individually
					if (e0.Material == e1.Material)						e.Material = e0.Material;
					if (e1.Material == e2.Material)						e.Material = e1.Material;
					if (e0.Material == e2.Material)						e.Material = e2.Material;

					if (e0.NormalQuanta == e1.NormalQuanta)				e.NormalQuanta = e0.NormalQuanta;
					if (e1.NormalQuanta == e2.NormalQuanta)				e.NormalQuanta = e1.NormalQuanta;
					if (e0.NormalQuanta == e2.NormalQuanta)				e.NormalQuanta = e2.NormalQuanta;

					if (e0.OrientationQuanta == e1.OrientationQuanta)	e.OrientationQuanta = e0.OrientationQuanta;
					if (e1.OrientationQuanta == e2.OrientationQuanta)	e.OrientationQuanta = e1.OrientationQuanta;
					if (e0.OrientationQuanta == e2.OrientationQuanta)	e.OrientationQuanta = e2.OrientationQuanta;

					if (e0.Character == e1.Character)					e.Character = e0.Character;
					if (e1.Character == e2.Character)					e.Character = e1.Character;
					if (e0.Character == e2.Character)					e.Character = e2.Character;

					if (e0.Level == e1.Level)							e.Level = e0.Level;
					if (e1.Level == e2.Level)							e.Level = e1.Level;
					if (e0.Level == e2.Level)							e.Level = e2.Level;

					if (e0.QuantHeight == e1.QuantHeight)				e.QuantHeight = e0.QuantHeight;
					if (e1.QuantHeight == e2.QuantHeight)				e.QuantHeight = e1.QuantHeight;
					if (e0.QuantHeight == e2.QuantHeight)				e.QuantHeight = e2.QuantHeight;
				}
			}
		}
	}

	for (el=0; el<(sint)Elements.size(); ++el)
	{
		Elements[el]->IsHorizontal = (Elements[el]->NormalQuanta == 0);
		Elements[el]->IsValid = Elements[el]->IsHorizontal;
	}

	vector<CSurfElement*>	elDup;
	for (el=0; el<(sint)Elements.size(); ++el)
		if (Elements[el]->IsValid)
			elDup.push_back(Elements[el]);
	Elements = elDup;
	elDup.clear();

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

	for (el=0; el<(sint)Elements.size(); ++el)
	{
		CSurfElement	&element = *(Elements[el]);
		uint	i;
		for (i=0; i<3; ++i)
			if (element.EdgeLinks[i] != NULL && !element.EdgeLinks[i]->IsValid)
				element.EdgeLinks[i] = NULL;
	}

	// flood fills the tessellation to get surfaces
	{
		nldebug("build and flood fill surfaces -- pass 1");
		uint32	surfId = 0; // + (ZoneId<<16);
		uint	totalSurf = 0;
		sint32	extSurf = -1024;

		CPlane	planes[6];
		BBox.makePyramid(planes);

		for (p=0; p<(sint)Elements.size(); ++p)
		{
			if (Elements[p]->SurfaceId == UnaffectedSurfaceId)
			{
				bool	elInCentral = (Elements[p]->Root->ZoneId == CentralZoneId);
//				bool	elInCentral = true;

				++totalSurf;
				sint32	thisSurfId = (elInCentral) ? surfId++ : extSurf--;
				if (elInCentral)
					Surfaces.push_back(CComputableSurface());
				else
					ExtSurfaces.push_back(CComputableSurface());

			
				CComputableSurface	&surf = (elInCentral) ? Surfaces.back() : ExtSurfaces.back();
				surf.BorderKeeper = &Borders;
				surf.floodFill(Elements[p], thisSurfId);
				surf.BBox = BestFittingBBox;
				if (surf.IsHorizontal && elInCentral)
					surf.computeHeightQuad();
			}
		}
	}

	// compute elements level
	if (ComputeLevels)
	{
		nldebug("compute elements levels");
		// Insert all elements into a CQuadGrid

		CQuadGrid<CSurfElement *>	quadGrid;
		quadGrid.create(1024, 0.25f);

		sint	s;
		for (s=0; s<(sint)Surfaces.size(); ++s)
		{
			CComputableSurface	&surf = Surfaces[s];
			if (!surf.IsHorizontal)
				continue;

			sint	i;			

			for (i=0; i<(sint)surf.Elements.size(); ++i)
			{
				CAABBox	box = surf.Elements[i]->getBBox();
				quadGrid.insert(box.getMin(), box.getMax(), surf.Elements[i]);
			}

			for (i=0; i<(sint)surf.Elements.size(); ++i)
			{
				surf.Elements[i]->computeLevel(quadGrid);
			}

			quadGrid.clear();
		}
	}

	//
	Surfaces.clear();
	ExtSurfaces.clear();

	//
	for (i=0; i<(sint)Elements.size(); ++i)
	{
		Elements[i]->NoLevelSurfaceId = Elements[i]->SurfaceId;
		Elements[i]->SurfaceId = UnaffectedSurfaceId;
	}

	//
	{
		nldebug("build and flood fill surfaces -- pass 2");
		uint32	surfId = 0; // + (ZoneId<<16);
		uint	totalSurf = 0;
		sint32	extSurf = -1024;

		CPlane	planes[6];
		BBox.makePyramid(planes);

		for (p=0; p<(sint)Elements.size(); ++p)
		{
			if (Elements[p]->SurfaceId == UnaffectedSurfaceId)
			{
				bool	elInCentral = (Elements[p]->Root->ZoneId == CentralZoneId);
//				bool	elInCentral = true;

				++totalSurf;
				sint32	thisSurfId = (elInCentral) ? surfId++ : extSurf--;
				if (elInCentral)
					Surfaces.push_back(CComputableSurface());
				else
					ExtSurfaces.push_back(CComputableSurface());

			
				CComputableSurface	&surf = (elInCentral) ? Surfaces.back() : ExtSurfaces.back();

				surf.NoLevelSurfaceId = Elements[p]->NoLevelSurfaceId;

				surf.BorderKeeper = &Borders;
				surf.floodFill(Elements[p], thisSurfId);
				surf.BBox = BestFittingBBox;
				if (surf.IsHorizontal && elInCentral)
					surf.computeHeightQuad();
			}
		}

		nldebug("%d surfaces generated", totalSurf);
	}
}




void	NLPACS::CZoneTessellation::generateBorders(float smooth)
{
	sint	surf;

	nldebug("generate tessellation borders");
	// for each surface, build its border
	for (surf=0; surf<(sint)Surfaces.size(); ++surf)
		Surfaces[surf].buildBorders();

	// then, for each border, link the related surfaces...
	nldebug("smooth borders");
	sint	border;
	sint	totalBefore = 0,
			totalAfter = 0;
	for (border=0; border<(sint)Borders.size(); ++border)
	{
		float	smScale = (Borders[border].Right < 0) ? 0.5f : 1.0f;
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
	nldebug("smooth process: %d -> %d (%.1f percent reduction)", totalBefore, totalAfter, 100.0*(1.0-(double)totalAfter/(double)totalBefore));
}


void	NLPACS::CZoneTessellation::generateStats()
{
	uint	border, span;
	for (border=0; border<Borders.size(); ++border)
	{
		CComputableSurfaceBorder	&spanList = Borders[border];

		CAABBox	listBBox;
		bool	listSet = false;

		for (span=0; span<spanList.Vertices.size()-1; ++span)
		{
			CAABBox	spanBBox;

			spanBBox.setCenter(spanList.Vertices[span]);
			spanBBox.extend(spanList.Vertices[span+1]);

			StatsSurfaces.XBBSpan.add(spanBBox.getSize().x);
			StatsSurfaces.YBBSpan.add(spanBBox.getSize().y);

			if (!listSet)
				listBBox.setCenter(spanList.Vertices[span]);
			else
				listBBox.extend(spanList.Vertices[span]);

			listBBox.extend(spanList.Vertices[span+1]);
			listSet = true;
		}

		StatsSurfaces.XBBSpanList.add(listBBox.getSize().x);
		StatsSurfaces.YBBSpanList.add(listBBox.getSize().y);

		StatsSurfaces.TotalSpan += spanList.Vertices.size()-1;
	}

	StatsSurfaces.TotalSpanList += Borders.size();
}


void	NLPACS::CZoneTessellation::saveTessellation(COFile &output)
{
	output.serialCont(_Vertices);

	hash_map<const CSurfElement*, sint32, CHashPtr<const CSurfElement> >							elementRemap;
	hash_map<const CPatchTessellation*, pair<sint32,sint32>, CHashPtr<const CPatchTessellation> >	patchRemap;

	uint	i, j;

	elementRemap[NULL] = -1;
	for (i=0; i<_Tessellation.size(); ++i)
		elementRemap[&_Tessellation[i]] = i;

	patchRemap[NULL] = make_pair(-1, -1);
	for (i=0; i<_Zones.size(); ++i)
		for (j=0; j<_Zones[i].Patches.size(); ++j)
			patchRemap[&_Zones[i].Patches[j]] = make_pair(_Zones[i].Patches[j].ZoneId, _Zones[i].Patches[j].PatchId);

	uint32	numTessel = _Tessellation.size();
	output.serial(numTessel);

	for (i=0; i<_Tessellation.size(); ++i)
	{
		pair<sint32, sint32>	p;

		p = patchRemap[_Tessellation[i].Root];
		output.serial(p.first);
		output.serial(p.second);

		sint32	s;

		s = elementRemap[_Tessellation[i].EdgeLinks[0]]; output.serial(s);
		s = elementRemap[_Tessellation[i].EdgeLinks[1]]; output.serial(s);
		s = elementRemap[_Tessellation[i].EdgeLinks[2]]; output.serial(s);

		output.serial(_Tessellation[i].Tri[0]);
		output.serial(_Tessellation[i].Tri[1]);
		output.serial(_Tessellation[i].Tri[2]);
	}
}




void	NLPACS::CZoneTessellation::loadTessellation(CIFile &input)
{
	input.serialCont(_Vertices);

	uint	i, j;

	uint32	numTessel;
	input.serial(numTessel);
	_Tessellation.resize(numTessel);

	for (i=0; i<_Tessellation.size(); ++i)
	{
		sint32	s;

		input.serial(s);
		if (s == -1)
		{
			_Tessellation[i].Root = NULL;
			input.serial(s);
		}
		else
		{
			CPatchRetriever	*retriever = retrieveZone((uint16)s);

			input.serial(s);
			_Tessellation[i].Root = (s == -1) ? NULL : &(retriever->Patches[s]);
		}

		input.serial(s); _Tessellation[i].EdgeLinks[0] = (s == -1) ? NULL : &_Tessellation[s];
		input.serial(s); _Tessellation[i].EdgeLinks[1] = (s == -1) ? NULL : &_Tessellation[s];
		input.serial(s); _Tessellation[i].EdgeLinks[2] = (s == -1) ? NULL : &_Tessellation[s];

		input.serial(_Tessellation[i].Tri[0]);
		input.serial(_Tessellation[i].Tri[1]);
		input.serial(_Tessellation[i].Tri[2]);

		_Tessellation[i].Vertices = &_Vertices;
	}

	Elements.resize(_Tessellation.size());
	for (i=0; i<(sint)_Tessellation.size(); ++i)
	{
		CSurfElement	&element = _Tessellation[i];

		Elements[i] = &element;

		for (j=0; j<3; ++j)
			if (element.EdgeLinks[j] != NULL)
				element.ZoneLinks[j] = element.EdgeLinks[j]->Root->ZoneId;
	}
}

void	NLPACS::CZoneTessellation::clear()
{
	_Zones.clear();
	_Landscape.clear();
	_Tessellation.clear();
	_Vertices.clear();
	Elements.clear();
	Surfaces.clear();
	Borders.clear();
	Container.clear();
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