/** \file mini_col.cpp
 * <File description>
 *
 * $Id: mini_col.cpp,v 1.6 2001/01/02 10:22:02 berenguier Exp $
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

#include "nel/3d/mini_col.h"
#include "nel/3d/aabbox.h"
using namespace NLMISC;
using namespace std;


namespace NL3D 
{


static const	sint	QuadDepth= 10;


// ***************************************************************************
CMiniCol::CMiniCol()
{
	CMatrix		tmp;
	CVector		I(1,0,0);
	CVector		J(0,0,-1);
	CVector		K(0,1,0);

	tmp.identity();
	tmp.setRot(I,J,K, true);
	_QuadTree.changeBase (tmp);
	_Radius= 100;
	_Inited= false;
}


// ***************************************************************************
void			CMiniCol::addFaces(const std::vector<CTriangle> &faces)
{
	for(sint i=0;i<(sint)faces.size();i++)
	{
		const CTriangle	&f= faces[i];
		CAABBox	box;
		// Add, relative to center.
		box.setCenter(f.V0-_Center);
		box.extend(f.V1-_Center);
		box.extend(f.V2-_Center);
		CNode	node;
		node.Face= f;
		node.Plane.make(f.V0, f.V1, f.V2);
		_QuadTree.insert(box.getMin(), box.getMax(), node);
	}
}


// ***************************************************************************
void			CMiniCol::addLandscapePart(CLandscape &land, float size)
{
	vector<CTriangle> faces;
	CAABBoxExt	bb;
	bb.setCenter(_Center);
	bb.setSize(CVector(size, size, size));
	land.buildCollideFaces(bb,faces, false);
	addFaces(faces);
}


// ***************************************************************************
void			CMiniCol::init(CLandscape *land, float radius)
{
	_Landscape= land;
	_Radius= radius;
	float	size= 2* _Radius;
	_QuadTree.clear();
	// For security, add a delta (2*50: 2*maxsize of a patch).
	_QuadTree.create(QuadDepth, CVector::Null, 2*size+100);
}


// ***************************************************************************
void			CMiniCol::setCenter(const CVector& center)
{
	bool	reset= false;
	if(!_Inited)
	{
		_Inited= true;
		reset= true;
	}
	else
	{
		if((center-_Center).norm()>_Radius)
			reset= true;
	}

	if(reset)
	{
		_Center= center;
		// delete all elements, but not the quadtree.
		_QuadTree.eraseAll();
		if(_Landscape)
		{
			float	size= 2* _Radius;
			// init the col landscape of 2*size, so we can go on sphere limit, and still have good collision around us.
			addLandscapePart(*_Landscape, 2*size);
		}
	}
}


// ***************************************************************************
bool			CMiniCol::snapToGround(CVector &pos, float hup, float hbot)
{
	CVector	b1,b2;
	bool	found=false;
	float	height;


	// Select quad nodes which contains pos.
	b1=b2=pos;
	b1.z-= hbot;
	b2.z+= hup;
	// Select, relative to center.
	_QuadTree.select(b1-_Center,b2-_Center);

	// For each face, test if it is under pos, then test if height is correct.
	CQuadTree<CNode>::CIterator	iFace;
	for(iFace= _QuadTree.begin();iFace!=_QuadTree.end();iFace++)
	{
		CTriangle	&pFace= (*iFace).Face;
		CPlane		&pPlane= (*iFace).Plane;
		// Order is important.
		CVector		&p0= pFace.V0;
		CVector		&p1= pFace.V1;
		CVector		&p2= pFace.V2;

		// TOIMP: This is VERY SLOW!!! (hope that the quadtree will help, but it still very slow...).

		// Yoyo Debug, test, if the point may be IN the bbox.
		CAABBox		bbFace;
		bbFace.setCenter(p0);
		bbFace.extend(p1);
		bbFace.extend(p2);
		CVector		bext=p0;
		bext.z= maxof(p0.z, p1.z, p2.z)+hbot;
		bbFace.extend(bext);
		bext.z= minof(p0.z, p1.z, p2.z)-hup;
		bbFace.extend(bext);
		if(!bbFace.include(pos))
			continue;

		// Test if the face enclose the pos in X/Y plane.
		// NB: compute and using a BBox to do a rapid test is not a very good idea, since it will 
		// add an overhead which is NOT negligeable compared to the following test.
		float		a,b,c;		// 2D cartesian coefficients of line in plane X/Y.
		// Line p0-p1.
		a= -(p1.y-p0.y);
		b= (p1.x-p0.x);
		c= -(p0.x*a + p0.y*b);
		if( (a*pos.x + b*pos.y + c) < 0)	continue;
		// Line p1-p2.
		a= -(p2.y-p1.y);
		b= (p2.x-p1.x);
		c= -(p1.x*a + p1.y*b);
		if( (a*pos.x + b*pos.y + c) < 0)	continue;
		// Line p2-p0.
		a= -(p0.y-p2.y);
		b= (p0.x-p2.x);
		c= -(p2.x*a + p2.y*b);
		if( (a*pos.x + b*pos.y + c) < 0)	continue;


		// Compute the possible height.
		CVector		tmp;
		// intersect the vertical line with the plane.
		tmp= pPlane.intersect(pos, pos-CVector(0,0,100));
		float		h= tmp.z;
		// Test if it would fit in the wanted field.
		if(h>pos.z+hup)	continue;
		if(h<pos.z-hbot)	continue;

		// OK!!
		if(!found)
		{
			found=true;
			height=h;
		}
		else
		{
			height= max(height,h);
		}
	}

	if(found)
		pos.z= height;

	return found;
}


// ***************************************************************************
bool			CMiniCol::testMove(const CVector &prec, CVector &cur)
{
	CVector	dir= cur-prec;
	dir.normalize();

	// Must not go to near of a wall.
	CVector	test= cur+dir*0.5;
	float	norm= (test-prec).norm();
	norm*=3;
	if(!snapToGround(test, norm, norm))
	{
		cur= prec;
		return false;
	}
	else
	{
		// Must test and snap the current position.
		norm= (cur-prec).norm();
		norm*=3;
		if(!snapToGround(cur, norm, norm))
		{
			cur= prec;
			return false;
		}
	}
	return true;
}


} // NL3D
