/** \file mrm_builder.cpp
 * <File description>
 *
 * $Id: mrm_builder.cpp,v 1.1 2000/12/21 09:45:22 berenguier Exp $
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

#include "nel/3d/mrm_builder.h"
using namespace NLMISC;
using namespace std;


namespace NL3D 
{


// ***************************************************************************
// ***************************************************************************
// Tools Methods.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
static bool	findElement(vector<sint>	&array, sint elt)
{
	return find(array.begin(), array.end(), elt) != array.end();
}
// ***************************************************************************
static bool	deleteElement(vector<sint>	&array, sint elt)
{
	bool	found=false;
	vector<sint>::iterator	it=array.begin();

	while(  (it=find(array.begin(), array.end(), elt)) != array.end() )
		found=true, array.erase(it);

	return found;
	// Must not use remove<> since it do not modify size ... (???)
	// This seems not to work.
	//return remove(array.begin(), array.end(), elt)!=array.end();
}


// ***************************************************************************
// ***************************************************************************
// Edge Cost methods.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
bool	CMRMBuilder::vertexHasOneWedge(sint numvertex)
{
	CMRMVertex	&vert= TmpVertices[numvertex];
	for(sint attId=0;attId<NumAttributes;attId++)
	{
		sint	numwedge=-1;
		for(sint i=0;i<(sint)vert.SharedFaces.size();i++)
		{
			sint	w= TmpFaces[vert.SharedFaces[i]].getAssociatedWedge(attId, numvertex);
			if(numwedge>=0 && numwedge!=w)	return false;
			else	numwedge=w;
		}
	}
	return true;
}
// ***************************************************************************
bool	CMRMBuilder::vertexHasOneMaterial(sint numvertex)
{
	sint	matId=-1;
	CMRMVertex	&vert= TmpVertices[numvertex];
	for(sint i=0;i<(sint)vert.SharedFaces.size();i++)
	{
		sint	m= TmpFaces[vert.SharedFaces[i]].MaterialId;
		if(matId>=0 && matId!=m)	return false;
		else	matId=m;
	}
	return true;
}
// ***************************************************************************
bool	CMRMBuilder::vertexContinue(sint numvertex)
{
	return vertexHasOneWedge(numvertex) && vertexHasOneMaterial(numvertex);
}
// ***************************************************************************
bool	CMRMBuilder::vertexClosed(sint numvertex)
{
	CMRMVertex	&vert= TmpVertices[numvertex];
	map<CMRMEdge, sint>		EdgeShare;
	// Init to 0.
	for(sint i=0;i<(sint)vert.SharedFaces.size();i++)
	{
		CMRMFaceBuild		&f=TmpFaces[vert.SharedFaces[i]];
		EdgeShare[f.getEdge(0)]= 0;
		EdgeShare[f.getEdge(1)]= 0;
		EdgeShare[f.getEdge(2)]= 0;
	}
	// Inc count.
	for(i=0;i<(sint)vert.SharedFaces.size();i++)
	{
		CMRMFaceBuild		&f=TmpFaces[vert.SharedFaces[i]];
		EdgeShare[f.getEdge(0)]++;
		EdgeShare[f.getEdge(1)]++;
		EdgeShare[f.getEdge(2)]++;
	}
	// Test open edges.
	for(i=0;i<(sint)vert.SharedFaces.size();i++)
	{
		CMRMFaceBuild		&f=TmpFaces[vert.SharedFaces[i]];
		sint	v0= f.Corner[0].Vertex;
		sint	v1= f.Corner[1].Vertex;
		sint	v2= f.Corner[2].Vertex;
		if(EdgeShare[f.getEdge(0)]<2 && (v0==numvertex || v1==numvertex)) return false;
		if(EdgeShare[f.getEdge(1)]<2 && (v1==numvertex || v2==numvertex)) return false;
		if(EdgeShare[f.getEdge(2)]<2 && (v0==numvertex || v2==numvertex)) return false;
	}
	return true;
}
// ***************************************************************************
float	CMRMBuilder::getDeltaFaceNormals(sint numvertex)
{
	// return a positive value of Somme(|DeltaNormals|) / NNormals.
	CMRMVertex	&vert= TmpVertices[numvertex];
	float	delta=0;
	CVector	refNormal;
	sint	nfaces=vert.SharedFaces.size();
	for(sint i=0;i<nfaces;i++)
	{
		CVector	normal;
		CVector	&v0= TmpVertices[TmpFaces[i].Corner[0].Vertex].Current;
		CVector	&v1= TmpVertices[TmpFaces[i].Corner[1].Vertex].Current;
		CVector	&v2= TmpVertices[TmpFaces[i].Corner[2].Vertex].Current;
		normal= (v1-v0)^(v2-v0);
		normal.normalize();
		if(i==0)
			refNormal=normal;
		else
			delta+=(1-refNormal*normal);
	}
	if(nfaces<2)
		return 0;
	else
		return delta/(nfaces-1);
}
// ***************************************************************************
bool	CMRMBuilder::edgeContinue(const CMRMEdge &edge)
{
	sint v0= edge.v0;
	sint v1= edge.v1;
	CMRMVertex	&Vertex1=TmpVertices[v0], &Vertex2=TmpVertices[v1];

	// build list sharing edge.
	vector<sint>	deletedFaces;
	for(sint i=0;i<(sint)Vertex1.SharedFaces.size();i++)
	{
		sint	numFace= Vertex1.SharedFaces[i];
		if(TmpFaces[numFace].hasVertex(v1))
			deletedFaces.push_back(numFace);
	}

	// TODODODO.
/*	sint	matId=-1;
	sint	numwedge1=-1,numwedge2=-1;
	// test if faces have same behavior.
	for(i=0;i<(sint)deletedFaces.size();i++)
	{
		sint	m,w;
		m= TmpFaces[deletedFaces[i]].MaterialId;
		if(matId>=0 && matId!=m)	return false;
		else	matId=m;
		w= TmpFaces[deletedFaces[i]].getAssociatedWedge(v0);
		if(numwedge1>=0 && numwedge1!=w)	return false;
		else	numwedge1=w;
		w= TmpFaces[deletedFaces[i]].getAssociatedWedge(v1);
		if(numwedge2>=0 && numwedge2!=w)	return false;
		else	numwedge2=w;
	}
*/
	return true;
}
// ***************************************************************************
bool	CMRMBuilder::edgeNearUniqueMatFace(const CMRMEdge &edge)
{
	sint v0= edge.v0;
	sint v1= edge.v1;
	CMRMVertex	&Vertex1=TmpVertices[v0], &Vertex2=TmpVertices[v1];

	// build list sharing edge.
	vector<sint>	deletedFaces;
	for(sint i=0;i<(sint)Vertex1.SharedFaces.size();i++)
	{
		sint	numFace= Vertex1.SharedFaces[i];
		if(TmpFaces[numFace].hasVertex(v1))
			deletedFaces.push_back(numFace);
	}

	// test if faces are not isolated OneMaterial faces.
	for(i=0;i<(sint)deletedFaces.size();i++)
	{
		CMRMFaceBuild	&f=TmpFaces[deletedFaces[i]];
		if( !edgeContinue(f.getEdge(0)) && 
			!edgeContinue(f.getEdge(1)) && 
			!edgeContinue(f.getEdge(2)))
			return true;
	}

	return false;
}

// ***************************************************************************
float	CMRMBuilder::computeEdgeCost(const CMRMEdge &edge)
{
	sint	v1= edge.v0;
	sint	v2= edge.v1;
	// more expensive is the edge, later it will collapse.
	float	cost=(TmpVertices[v1].Current-TmpVertices[v2].Current).norm();

	float	faceCost= (getDeltaFaceNormals(v1)+getDeltaFaceNormals(v2));
	// Must minimize.
	faceCost= max(faceCost, 0.01f);
	cost*= faceCost;

	// Like hope, add a weight on discontinuities..
	if( !vertexContinue(v1) && !vertexContinue(v2) )
	{
		// Nb: don't do this on discontinuities edges, unless the unique material face will collapse (fiou!!).
		if( edgeContinue(edge) || edgeNearUniqueMatFace(edge) )
			cost*=4;
	}

	return cost;
}




// ***************************************************************************
// ***************************************************************************
// Collapse Methods.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
bool	CMRMBuilder::faceShareWedges(CMRMFaceBuild *face, sint attribId, sint numVertex1, sint numVertex2)
{
	sint	numWedge1= face->getAssociatedWedge(attribId, numVertex1);
	sint	numWedge2= face->getAssociatedWedge(attribId, numVertex2);
	if(numWedge1<0) return false;
	if(numWedge2<0) return false;

	CMRMAttribute	&w1= TmpAttributes[attribId][numWedge1];
	CMRMAttribute	&w2= TmpAttributes[attribId][numWedge2];
	return w1.Shared && w2.Shared && w1.NbSharedFaces>0 && w2.NbSharedFaces>0;
}



// ***************************************************************************
// ***************************************************************************
// Mesh Level method.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CMRMBuilder::CMRMBuilder()
{
}


} // NL3D
