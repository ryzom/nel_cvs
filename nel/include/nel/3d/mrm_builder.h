/** \file mrm_builder.h
 * <File description>
 *
 * $Id: mrm_builder.h,v 1.3 2001/01/30 13:44:16 lecroart Exp $
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

#ifndef NL_MRM_BUILDER_H
#define NL_MRM_BUILDER_H

#include "nel/misc/types_nl.h"
#include "nel/3d/mrm_mesh.h"
#include <map>


namespace NL3D 
{


// ***************************************************************************
/**
 * An internal mesh vertex representation for MRM building.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
struct	CMRMVertex
{
public:
	CVector				Current,Original;
	std::vector<sint>	SharedFaces;
	sint				CollapsedTo;

public:
	CMRMVertex() {CollapsedTo=-1;}
};


// ***************************************************************************
/**
 * An internal mesh vertex attribute (UV, color, normal...) representation for MRM building.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
struct	CMRMAttribute
{
public:
	CVectorH		Current,Original;
	sint			CollapsedTo;		// -2 <=> "must interpolate from Current to Original".

public:
	// temporary data in construction:
	sint			InterpolatedFace;
	sint			NbSharedFaces;
	bool			Shared;
	// A wedge is said "shared" if after edge is collapsed, he lost corners.
	// If NbSharedFaces==0, this wedge has been entirely destroyed.

public:
	CMRMAttribute() {CollapsedTo=-1;}
};


// ***************************************************************************
/**
 * An internal mesh edge Index representation for MRM building.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
struct	CMRMEdge
{
	sint	v0,v1;
	CMRMEdge() {}
	CMRMEdge(sint a, sint b) {v0= a; v1=b;}
	bool operator==(const CMRMEdge &o) const
	{
		// Order means nothing  ( (v0,v1) == (v1,v0) ).... Kick it.
		return (v0==o.v0 && v1==o.v1) || (v0==o.v1 && v1==o.v0);
	}
	bool operator<(const CMRMEdge &o) const
	{
		// Order means nothing  ( (v0,v1) == (v1,v0) ).... Kick it.
		sint max0= std::max(v0,v1);
		sint min0= std::min(v0,v1);
		sint max1= std::max(o.v0,o.v1);
		sint min1= std::min(o.v0,o.v1);
		if(max0!=max1)
			return max0<max1;
		else
			return min0<min1;
	}
};


// ***************************************************************************
struct	CMRMFaceBuild;
/**
 * A tuple Edge/Face.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
struct	CMRMEdgeFace : public CMRMEdge
{
	CMRMFaceBuild		*Face;
	CMRMEdgeFace();
	CMRMEdgeFace(sint a, sint b, CMRMFaceBuild *f) 
	{
		v0=a; v1=b;
		Face= f;
	}
	CMRMEdgeFace(const CMRMEdge &e, CMRMFaceBuild *f) : CMRMEdge(e)
	{
		Face= f;
	}

};


// ***************************************************************************
/**
 * The map of edge collapses.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
typedef	std::multimap<float, CMRMEdgeFace>	TEdgeMap;
typedef	TEdgeMap::iterator					ItEdgeMap;


// ***************************************************************************
/**
 * An internal mesh extended face representation for MRM building.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
struct	CMRMFaceBuild : public CMRMFace
{
public:
	// temporary data in construction:
	// The interpolated attrbutes of the face.
	CVectorH		InterpolatedAttributes[NL3D_MRM_MAX_ATTRIB];
	// Is this face deleted in the current MRM collapse?
	bool			Deleted;
	// The iterator of the edges in the EdgeCollapse list.
	ItEdgeMap		It0, It1, It2;
	// The mirror value of iterator: are they valid???
	bool			ValidIt0, ValidIt1, ValidIt2;


public:
	CMRMFaceBuild()
	{
		Deleted=false;
		ValidIt0= ValidIt1= ValidIt2= false;
	}
	CMRMFaceBuild &operator=(const CMRMFace &f)
	{
		(CMRMFace &)(*this)=f;
		return *this;
	}

	// Edges.
	//=======
	sint	getAssociatedEdge(const CMRMEdge &edge) const
	{
		sint v0= edge.v0;
		sint v1= edge.v1;
		if(Corner[0].Vertex==v0 && Corner[1].Vertex==v1)	return 0;
		if(Corner[0].Vertex==v1 && Corner[1].Vertex==v0)	return 0;
		if(Corner[1].Vertex==v0 && Corner[2].Vertex==v1)	return 1;
		if(Corner[1].Vertex==v1 && Corner[2].Vertex==v0)	return 1;
		if(Corner[0].Vertex==v0 && Corner[2].Vertex==v1)	return 2;
		if(Corner[0].Vertex==v1 && Corner[2].Vertex==v0)	return 2;
		return -1;
	}
	bool	hasEdge(const CMRMEdge &edge) const
	{
		return getAssociatedEdge(edge)!=-1;
	}
	CMRMEdge	getEdge(sint eId) const
	{
		if(eId==0) return CMRMEdge(Corner[0].Vertex, Corner[1].Vertex);
		if(eId==1) return CMRMEdge(Corner[1].Vertex, Corner[2].Vertex);
		if(eId==2) return CMRMEdge(Corner[2].Vertex, Corner[0].Vertex);
		nlstop;
		return CMRMEdge(-1,-1);
	}
	void	invalidAllIts()
	{
		ValidIt0= ValidIt1= ValidIt2= false;
	}
	void	invalidEdgeIt(const CMRMEdge &e)
	{
		if(e== getEdge(0)) ValidIt0= false;
		else if(e== getEdge(1)) ValidIt1= false;
		else if(e== getEdge(2)) ValidIt2= false;
		else nlstop;
	}
	bool	validEdgeIt(const CMRMEdge &e)
	{
		if(e== getEdge(0)) return ValidIt0;
		if(e== getEdge(1)) return ValidIt1;
		if(e== getEdge(2)) return ValidIt2;
		nlstop;
		return false;
	}

	// Vertices.
	//==========
	bool	hasVertex(sint	numvertex)
	{
		return Corner[0].Vertex==numvertex || Corner[1].Vertex==numvertex || Corner[2].Vertex==numvertex;
	}

	
	// Wedges.
	//==========
	bool	hasWedge(sint attribId, sint numwedge)
	{
		return Corner[0].Attributes[attribId]==numwedge ||
			Corner[1].Attributes[attribId]==numwedge ||
			Corner[2].Attributes[attribId]==numwedge;
	}
	sint	getAssociatedWedge(sint attribId, sint numvertex)
	{
		if(Corner[0].Vertex==numvertex)	return Corner[0].Attributes[attribId];
		if(Corner[1].Vertex==numvertex)	return Corner[1].Attributes[attribId];
		if(Corner[2].Vertex==numvertex)	return Corner[2].Attributes[attribId];
		return -1;
	}
};



// ***************************************************************************
/**
 * The class for building MRMs.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class CMRMBuilder
{
public:

	/// Constructor
	CMRMBuilder();

	// Make.
	//void	setupMesh(const CMeshBuild &baseMesh);
	//void	setupVertexWeightsFlags....
	//void	setupInterfaces....
	//void	compileMRM(CMeshBuildMRM &mrmMesh, ...);


// ****************************
private:

	// The vertices of the MRMMesh.
	std::vector<CMRMVertex>		TmpVertices;
	// The attributes of the MRMMesh.
	std::vector<CMRMAttribute>	TmpAttributes[NL3D_MRM_MAX_ATTRIB];
	// The number of used attributes of the MRMMesh.
	sint						NumAttributes;
	// The faces of the MRMMesh.
	std::vector<CMRMFaceBuild>	TmpFaces;
	// Ordered list of Edge collapse.
	TEdgeMap					EdgeCollapses;


	/// \name Edge Cost methods.
	// @{
	bool	vertexHasOneWedge(sint numvertex);
	bool	vertexHasOneMaterial(sint numvertex);
	bool	vertexContinue(sint numvertex);
	bool	vertexClosed(sint numvertex);
	float	getDeltaFaceNormals(sint numvertex);	// return a positive value of Sum(|DeltaNormals|) / NNormals.
	bool	edgeContinue(const CMRMEdge &edge);
	bool	edgeNearUniqueMatFace(const CMRMEdge &edge);
	float	computeEdgeCost(const CMRMEdge &edge);
	// @}


	/// \name Collapse methods.
	// @{
	bool	faceShareWedges(CMRMFaceBuild *face, sint attribId, sint numVertex1, sint numVertex2);
	void	insertFaceIntoEdgeList(CMRMFaceBuild &tmpf);
	void	removeFaceFromEdgeList(CMRMFaceBuild &f);
	sint	collapseEdge(const CMRMEdge &edge);	// return num of deleted faces.
	sint	followVertex(sint i);
	sint	followWedge(sint attribId, sint i);
	// @}


	/// \name Mesh Level methods.
	// @{
	void	init(const CMRMMesh &baseMesh);
	void	collapseEdges(sint nWantedFaces);
	//void	makeLODMesh(LSingleLODMesh &lodMesh);
	void	saveCoarserMesh(CMRMMesh &coarserMesh);
	//void	makeFromMesh(const CMRMMesh &baseMesh, LSingleLODMesh &lodMesh, CMRMMesh &coarserMesh, sint nWantedFaces);
	// @}


};


} // NL3D


#endif // NL_MRM_BUILDER_H

/* End of mrm_builder.h */
