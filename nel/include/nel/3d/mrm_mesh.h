/** \file mrm_mesh.h
 * Internal header for CMRMBuilder.
 *
 * $Id: mrm_mesh.h,v 1.3 2001/06/14 13:37:27 berenguier Exp $
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

#ifndef NL_MRM_MESH_H
#define NL_MRM_MESH_H

#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"
#include "nel/misc/vector_h.h"
#include "nel/misc/vector.h"
#include <vector>


namespace NL3D
{


using NLMISC::CVector;
using NLMISC::CVectorH;


// ***************************************************************************
#define	NL3D_MRM_MAX_ATTRIB		12


// ***************************************************************************
/**
 * An internal mesh corner Index representation for MRM.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
struct	CMRMCorner
{
	// The id of the vertex.
	sint	Vertex;
	// The ids of the wedges. Points on Attributes of the mesh.
	sint	Attributes[NL3D_MRM_MAX_ATTRIB];

	// For CMRMMeshFinal computing, wedge ids.
	sint	WedgeStartId;
	sint	WedgeEndId;
	sint	WedgeGeomId;
};


// ***************************************************************************
/**
 * An internal mesh face representation for MRM.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
struct	CMRMFace
{
public:
	// The 3 corner indices of the face.
	CMRMCorner		Corner[3];
	// The id of the material. Used for material boundaries.
	sint			MaterialId;
};


// ***************************************************************************
/**
 * An internal mesh representation for MRM. USER DO NOT USE IT!!
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 * \see CMRMBuilder
 */
class CMRMMesh
{
public:
	// The vertices of the MRMMesh.
	std::vector<CVector>		Vertices;
	// The attributes of the MRMMesh.
	std::vector<CVectorH>		Attributes[NL3D_MRM_MAX_ATTRIB];
	// The number of used attributes of the MRMMesh.
	sint						NumAttributes;
	// The faces of the MRMMesh.
	std::vector<CMRMFace>		Faces;


public:
	/// Constructor
	CMRMMesh();

};


// ***************************************************************************
/**
 * An internal mesh representation for MRM, with geomoprh information. USER DO NOT USE IT.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 * \see CMRMBuilder
 */
class CMRMMeshGeom : public CMRMMesh
{
public:
	/// Same size than Faces, but points onto coarser Mesh verices. NB: MaterialId means nothing here.
	std::vector<CMRMFace>		CoarserFaces;

	CMRMMeshGeom	&operator=(const CMRMMesh &o)
	{
		(CMRMMesh&)(*this)= o;
		// copy faces into CoarserFaces.
		CoarserFaces= Faces;
	}

public:
	/// Constructor
	CMRMMeshGeom();
};


// ***************************************************************************
/**
 * A geomoprh information.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 * \see CMRMBuilder
 */
struct	CMRMWedgeGeom
{
	/// The start wedge index of the geomorph.
	sint	Start;
	/// The end wedge index of the geomorph.
	sint	End;
	/// where to store the result in the Wedge array. (NB: always in beginning of array).
	sint	Dest;
};


// ***************************************************************************
/**
 * An internal MRM mesh representation for MRM, with All lods information. USER DO NOT USE IT.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 * \see CMRMBuilder
 */
class	CMRMMeshFinal
{
public:

	// An wedge value (vertex + all attribs).
	struct	CWedge
	{
		CVector		Vertex;
		CVectorH	Attributes[NL3D_MRM_MAX_ATTRIB];
		static		uint NumAttributesToCompare;

		bool	operator<(const CWedge &o) const
		{
			if(Vertex!=o.Vertex)
				return Vertex<o.Vertex;
			else
			{
				nlassert(NumAttributesToCompare<=NL3D_MRM_MAX_ATTRIB);
				for(uint i=0; i<NumAttributesToCompare; i++)
				{
					if(Attributes[i]!=o.Attributes[i])
						return Attributes[i]<o.Attributes[i];
				}
			}
			// they are equal.
			return false;
		}
	};

	// a face.
	struct	CFace
	{
		/// Three index on the wedegs.
		sint		WedgeId[3];
		/// the material id.
		sint		MaterialId;
	};


	// A LOD information for the final MRM representation.
	struct	CLod
	{
		/// this tells how many wedges in the Wedges array this lod requires. this is usefull for partial loading.
		sint						NWedges;
		/// This is the face list for this LOD.
		std::vector<CFace>			Faces;
		/// List of geomorphs.
		std::vector<CMRMWedgeGeom>	Geomorphs;
	};


public:
	/** The wedges of the final mesh. Contains all Wedges for all lods, sorted from LOD0 to LODN, 
	 * with additional empty wedges, for geomorph.
	 */
	std::vector<CWedge>			Wedges;
	/// The number of used attributes of the MRMMesh.
	sint						NumAttributes;
	/// the finals Lods of the MRM.
	std::vector<CLod>			Lods;


	CMRMMeshFinal()
	{
		NumAttributes= 0;
	}


	/// add a wedge to this mesh, or return id if exist yet.
	sint	findInsertWedge(const CWedge &w);


	void	reset()
	{
		Wedges.clear();
		_WedgeMap.clear();
		Lods.clear();
		NumAttributes= 0;
	}


private:
	// The map of wedges to wedges index.
	typedef std::map<CWedge, sint>		TWedgeMap;
	TWedgeMap			_WedgeMap;

};


} // NL3D


#endif // NL_MRM_MESH_H

/* End of mrm_mesh.h */
