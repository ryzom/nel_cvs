/** \file build_surf.h
 * 
 *
 * $Id: build_surf.h,v 1.9 2003/08/27 09:23:07 legros Exp $
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

#ifndef NL_BUILD_SURF_H
#define NL_BUILD_SURF_H

#include <vector>

#include "nel/misc/debug.h"
#include "nel/misc/file.h"

#include "3d/zone.h"
#include "3d/patch.h"
#include "3d/mesh.h"
#include "3d/landscape.h"

#include "nel/3d/quad_tree.h"
#include "3d/quad_grid.h"

#include "nel/misc/vector.h"
#include "nel/misc/aabbox.h"
#include "nel/misc/geom_ext.h"
#include "nel/misc/polygon.h"

#include "pacs/surface_quad.h"
#include "pacs/retrievable_surface.h"

#include "prim_checker.h"



extern std::string				OutputRootPath;
extern std::string				OutputDirectory;
extern std::string				OutputPath;
extern std::string				TessellationPath;
extern std::string				IGBoxes;
extern uint						TessellateLevel;
extern bool						ReduceSurfaces;
extern bool						SmoothBorders;
extern bool						ComputeElevation;
extern bool						ComputeLevels;
extern std::vector<std::string>	ZoneNames;
extern std::string				ZoneExt;
extern std::string				ZoneLookUpPath;

extern bool						TessellateZones;
extern bool						MoulineZones;
extern bool						ProcessRetrievers;
extern std::string				PreprocessDirectory;

extern bool						UseZoneSquare;
extern std::string				ZoneUL;
extern std::string				ZoneDR;

extern std::string				GlobalRetriever;
extern std::string				RetrieverBank;
extern std::string				GlobalUL;
extern std::string				GlobalDR;
extern bool						ProcessGlobal;

extern CPrimChecker				PrimChecker;

std::string			getZoneNameById(uint16 id);
uint16				getZoneIdByName(std::string &name);
NLMISC::CAABBox		getZoneBBoxById(uint16 id);
uint16				getZoneIdByPos(NLMISC::CVector &pos);
NL3D::CMesh			*generateMeshFromBBox(const NLMISC::CAABBox &bbox, NLMISC::CRGBA color = NLMISC::CRGBA(255, 128, 0));

namespace NLPACS
{

class CSurfElement;
class CComputableSurfaceBorder;
class CComputableSurface;
class CPatchTessellation;
class CZoneTessellation;


/**
 * CStats. Used for statistics...
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class CStats
{
public:
	class CHisto : public std::vector< std::pair<float, uint> >
	{
	public:
		double	Total;

	public:
		CHisto() : Total(0.0) {}

		void	add(float val)
		{
			uint	i;
			for (i=0; i<size(); ++i)
			{
				if (val < operator[](i).first)
				{
					operator[](i).second++;
					break;
				}
			}

			Total += val;
		}

		void	dump(char *msg)
		{
			uint	i, t = 0;

			nlinfo(msg);

			for (i=1; i<size(); ++i)
				if (operator[](i).second != 0)
					nlinfo("[%.4f,%.4f]: %d elements", operator[](i-1).first, operator[](i).first, operator[](i).second);

			for (i=0; i<size(); ++i)
				t += operator[](i).second;

			nlinfo("number of elements: %d", t);
			nlinfo("mean: %f", Total/(double)t);
		}
	};

public:
	CHisto		XBBSpanList;
	CHisto		YBBSpanList;
	CHisto		XBBSpan;
	CHisto		YBBSpan;

	uint		TotalSpanList;
	uint		TotalSpan;

public:

	void init();

};

extern	CStats	StatsSurfaces;

/**/

const sint32	UnaffectedSurfaceId = -1;











/**
 * CSurfElement is an element of an iso-criteria surface. It is basically a CTriangle, and
 * contains the various criteria values such as incline class, landscape material ...
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class CSurfElement
{
public:
	/**
	 *
	 */
	uint32							ElemId;

	/**
	 * The support of the surface element.
	 * The index to the 3 vertices of the triangle.
	 */
	uint32							Tri[3];

	/**
	 * The element normal vector
	 */
	NLMISC::CVector					Normal;

	/**
	 * The area of the element
	 */
	float							Area;

	/**
	 * The root tessellation patch
	 */
	CPatchTessellation				*Root;

	/**
	 * The tessellation vertices
	 */
	std::vector<NLMISC::CVector>	*Vertices;


	/* Here the surface criteria.
	   Probably some normal quantization, material, flags ... */
	uint8							NormalQuanta;
	uint8							OrientationQuanta;
	uint8							Material;
	uint8							Character;
	uint8							Level;
	uint8							WaterShape;
	uint8							QuantHeight;

	uint32							ForceMerge;
	
	bool							IsBorder;
	bool							IsHorizontal;
	bool							IsValid;
	bool							IsMergable;
	bool							ClusterHint;

	enum
	{
		NumNormalQuantas = 4,
		NumOrientationQuantas = 4
	};


	/** 
	 * The links to the neighboring elements.
	 * Each edge is related to the opposite vertex in the triangle */
	CSurfElement		*EdgeLinks[3];

	/**
	 * The id of the zone linked to each edge (-1 if none).
	 */
	sint32				ZoneLinks[3];

	/**
	 * A flag for each edge, set if the edge has already been evaluated (in
	 * the surface border computation.
	 */
	bool				EdgeFlag[3];

	/**
	 * The Id of the surface container.
	 */
	sint32				SurfaceId;

	/**
	 * The Id of the no level surface container.
	 */
	sint32				NoLevelSurfaceId;

	/**
	 * Set if the element has already be cut on a border of the zone
	 */
	uint8				CutFlag;

public:
	/**
	 * Constructor.
	 * Creates a simple CSurfElement.
	 */
	CSurfElement()
	{
		ElemId = 0;
		Root = NULL;
		EdgeLinks[0] = NULL;
		EdgeLinks[1] = NULL;
		EdgeLinks[2] = NULL;
		ZoneLinks[0] = -1;
		ZoneLinks[1] = -1;
		ZoneLinks[2] = -1;
		EdgeFlag[0] = false;
		EdgeFlag[1] = false;
		EdgeFlag[2] = false;
		SurfaceId = UnaffectedSurfaceId;
		NoLevelSurfaceId = UnaffectedSurfaceId;
		NormalQuanta = 0;
		OrientationQuanta = 0;
		Material = 0;
		IsBorder = false;
		IsHorizontal = false;
		IsValid = false;
		IsMergable = true;
		ClusterHint = false;
		CutFlag = 0;
		WaterShape = 255;
		QuantHeight = 0;
		ForceMerge = 0;
	}

	/// Computes the bbox of the surface element.
	NLMISC::CAABBox	getBBox() const;


	/**
	 * Computes the various criteria values (associated to quantas)
	 */
	void	computeQuantas();

	/**
	 * Computes the level number of the surface element
	 */
	void	computeLevel(NL3D::CQuadGrid<CSurfElement *> &grid);

	/**
	 * Returns true if both elements have the same topological properties.
	 */
	bool	checkProperties(CSurfElement &e)
	{
		return	Material == e.Material &&
				OrientationQuanta == e.OrientationQuanta &&
				NormalQuanta == e.NormalQuanta;
	}

	/**
	 * Removes properly all links to the CSurfElement.
	 */
	void	removeLinks()
	{
		uint	i, j;
		for (i=0; i<3; ++i)
		{
			if (EdgeLinks[i] != NULL)
				for (j=0; j<3; ++j)
					if (EdgeLinks[i]->EdgeLinks[j] == this)
						EdgeLinks[i]->EdgeLinks[j] = NULL;
			EdgeLinks[i] = NULL;
		}

	}

private:
	void	computeElevation(std::vector<NLMISC::CPlane> &elevation, float radius, float height, float floorThreshold);
};










/**
 * CComputableSurfaceBorder separates geometrically 2 distinct CComputableSurface objects
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class CComputableSurfaceBorder
{
public:
	std::vector<NLMISC::CVector>	Vertices;

	sint32							Left;
	sint32							Right;

	float							Length;

	sint8							Edge;

	bool							DontSmooth;

public:
	/// Constructor.
	CComputableSurfaceBorder(sint32 left = 0, sint32 right = 0, sint edge=-1) : Left(left), Right(right), Edge(edge), DontSmooth(false) {}

	/// Dump the vertices that constitue the border.
	void	dump();

	/// Smoothes the border (and so reduces the number of vertices).
	void	smooth(float val);

	/// Computes the length of the border
	void	computeLength()
	{
		sint	n;
		Length = 0.0;
		for (n=0; n<(sint)Vertices.size()-1; ++n)
		{
			Length += (Vertices[n+1]-Vertices[n]).norm();
		}
	}
};










/**
 * CComputableSurface is a compact connex set of CSurfElement.
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class CComputableSurface
{
public:

public:
	/// The Id of the surface
	sint32									SurfaceId;

	/// No level surf id
	sint32									NoLevelSurfaceId;

	/// The references on the elements that belong to the surface
	std::vector<CSurfElement *>				Elements;

	/// The object that stores all the borders used in the computed area
	std::vector<CComputableSurfaceBorder>	*BorderKeeper;

	/// The characteristics of the surface
	uint8									NormalQuanta;
	uint8									OrientationQuanta;
	uint8									Material;
	uint8									Character;
	uint8									Level;

	bool									IsHorizontal;
	bool									IsUnderWater;
	bool									ClusterHint;

	float									Area;
	float									WaterHeight;
	uint8									QuantHeight;

	/// The BBox of the whole zone (in which the surface should be contained.)
	NLMISC::CAABBox							BBox;

	/// The height storage quad tree
	CSurfaceQuadTree						HeightQuad;

	/// The center of the surface
	NLMISC::CVector							Center;

public:
	/**
	 * Constructor.
	 * Builds an empty surface.
	 */
	CComputableSurface() : SurfaceId(UnaffectedSurfaceId), NormalQuanta(0), OrientationQuanta(0), Material(0), BorderKeeper(NULL), ClusterHint(false)	{}

	/**
	 * Flood fills the surface elements to find iso-criteria surfaces.
	 * Every linked surface element which has the same quantas values and a surfaceid == -1
	 * are marked and recursively called.
	 */
	void	floodFill(CSurfElement *first, sint32 surfId);

	/// Builds the border of the CComputableSurface.
	void	buildBorders();

	///
	void	computeHeightQuad();

private:
	void	followBorder(CSurfElement *first, uint edge, uint sens, std::vector<NLMISC::CVector> &vstore, bool &loop);
};









/**
 * CPatchRetriever.
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
struct CPatchRetriever
{
	const NL3D::CZone					*Zone;
	uint16								ZoneId;
	NLMISC::CAABBox						BBox;
	std::vector<NL3D::CPatchInfo>		PatchInfos;
	std::vector<NL3D::CBorderVertex>	BorderVertices;
	uint32								MaxVertex;
	std::vector<CPatchTessellation>		Patches;

	std::vector<sint32>					PatchRemap;
	uint32								TotalNew;

	CPatchRetriever(uint16 zoneId) : ZoneId(zoneId) {}
};

struct CRetrieverSort
{
	bool	operator() (const CPatchRetriever &a, const CPatchRetriever &b) const
	{
		return a.ZoneId < b.ZoneId;
	}
};









/**
 * CPatchTessellation contains information related to the tessellation of a single patch.
 * Used to link surface elements together.
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class CPatchTessellation
{
private:
	uint16							_NT, _NS;

public:
	const NL3D::CPatch				*RootPatch;
	const NL3D::CPatchInfo			*RootPatchInfo;
	CZoneTessellation				*RootZoneTessellation;
	CPatchRetriever					*RootRetriever;
	std::vector<CSurfElement>		Elements;
	uint16							PatchId;
	uint16							ZoneId;
	bool							Valid;

	std::vector<NLMISC::CVector>	Vertices;
	std::vector<uint8>				BorderSnapped;
	NLMISC::CAABBox					BBox;
	NLMISC::CAABBox					OriginalBBox;

public:
	/**
	 * Constructor.
	 * Creates an empty patch tessellation, with no patch nor zone information.
	 * The tessellation will be built using build().
	 */
	CPatchTessellation() : RootPatch(NULL), RootPatchInfo(NULL), RootZoneTessellation(NULL), PatchId(0), _NT(0), _NS(0), Valid(false) {}

	/**
	 * Sets patch and zone values up.
	 */
	void	setup(const NL3D::CPatch *rootPatch, 
				  const NL3D::CPatchInfo *rootPatchInfo,
				  CZoneTessellation *rootZone,
				  CPatchRetriever *rootRetriever,
				  uint16 patchId,
				  uint16 zoneId);

	/**
	 * Selects faces inside the patch elevation.
	 */
/*
	void	selectElevation(std::vector<CSelectTriangle *> &selection);
	std::vector<CSelectTriangle *>		Selected;
*/
	void	selectElevation(std::vector<CSurfElement *> &selection);
	std::vector<CSurfElement *>		Selected;


};











/**
 * CZoneTessellation is the whole tessellation of a given CZone.
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class CZoneTessellation
{
private:
	NL3D::CLandscape						_Landscape;
	std::vector<CSurfElement>				_Tessellation;
	std::vector<NLMISC::CVector>			_Vertices;

protected:
	friend class CPatchTessellation;

	std::vector<CPatchRetriever>			_Zones;
	
	CPatchRetriever	*retrieveZone(uint16 id)
	{
		uint	i;
		for (i=0; i<_Zones.size(); ++i)
			if (_Zones[i].ZoneId == id)
				return &(_Zones[i]);
		return NULL;
	}
public:
	class CMergeForceBox
	{
	public:
		NLMISC::CAABBox						MergeBox;
		uint32								MergeId;
		void	serial(NLMISC::IStream &f)		{ f.serial(MergeBox, MergeId); }
	};

public:
	/// The zone valid tessellation elements.
	std::vector<CSurfElement *>				Elements;

	///
	NLMISC::CAABBox							BBox;
	NLMISC::CAABBox							OriginalBBox;
	NLMISC::CAABBox							BestFittingBBox;

	///
	NLMISC::CVector							Translation;

	///
	sint32									CentralZoneId;

	std::vector<NLMISC::CPolygon>			WaterShapes;
	NL3D::CQuadGrid<uint32>					WaterGrid;

	/** 
	 * The tessellation refinement. The size of the tessellation is equal to 2m/Refinement
	 * (say, for instance, a refinement of 2 means a 1m large tessellation.)
	 */
	sint16									Refinement;

	/**
	 * The surfaces composing the tessellation.
	 */
	std::vector<CComputableSurface>			Surfaces;
	std::vector<CComputableSurface>			ExtSurfaces;

	/**
	 * The borders for the whole CZone.
	 */
	std::vector<CComputableSurfaceBorder>	Borders;

	/**
	 * The tessellation container/selector
	 */
	NL3D::CQuadTree<CSurfElement *>			Container;
	NLMISC::CAABBox							ContBBox;

	/**
	 * The box that force merge into surface
	 */
	std::vector<CMergeForceBox>				ForceMerge;

public:
	/**
	 * Constructor
	 * Creates an empty tessellation.
	 */
	CZoneTessellation() {}

	/**
	 * Clear
	 */
	void	clear();

	/**
	 * Sets a zone tessellation up for building later.
	 */
	bool	setup(uint16 zoneId, sint16 refinement, const NLMISC::CVector &translation);

	/**
	 * Adds a zone light tessellation to the quad tree container.
	 */
	void	addToContainer(const NL3D::CZone &zone);
	NL3D::CMesh	*generateCollisionMesh();

	/**
	 * Builds the whole zone tessellation (with linkage) from the given zone.
	 */
	void	build();

	/**
	 * Sets the water polygons up.
	 */
	void	addWaterShape(const NLMISC::CPolygon &poly)
	{
		WaterShapes.push_back(poly);
	}

	/**
	 * Compile the whole zone tessellation and creates surfaces
	 */
	void	compile();

	/**
	 * Generates a CMesh from the tessellation.
	 */
	NL3D::CMesh	*generateMesh();

	/**
	 * Generates borders for the whole zone tessellation.
	 * \param smooth how much to smooth the borders
	 */
	void	generateBorders(float smooth);

	/**
	 *
	 */
	NLMISC::CAABBox	computeBBox() const;

	/**
	 * Generates Stats...
	 */
	void	generateStats();

	/**
	 * Save tessellation
	 */
	void	saveTessellation(NLMISC::COFile &output);

	/**
	 * Load tessellation
	 */
	void	loadTessellation(NLMISC::CIFile &input);
};

}; // NLPACS

#endif // NL_BUILD_SURF_H

/* End of build_surf.h */
