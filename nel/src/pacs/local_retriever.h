/** \file local_retriever.h
 * 
 *
 * $Id: local_retriever.h,v 1.22 2002/12/18 14:57:14 legros Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
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

#ifndef NL_LOCAL_RETRIEVER_H
#define NL_LOCAL_RETRIEVER_H

#include <vector>
#include <string>
#include <list>

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/file.h"

#include "nel/misc/aabbox.h"
#include "nel/misc/polygon.h"

#include "pacs/vector_2s.h"
#include "pacs/surface_quad.h"
#include "pacs/chain.h"
#include "pacs/retrievable_surface.h"
#include "pacs/chain_quad.h"
#include "pacs/exterior_mesh.h"
#include "pacs/face_grid.h"

#include "nel/pacs/u_global_position.h"



namespace NLPACS
{

class CCollisionSurfaceTemp;

/**
 * A surface retriever, located by its bounding box.
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class CLocalRetriever
{
public:
	/**
	 * A tip of several chains. A CTip can contain more than 2 chains.
	 * \author Benjamin Legros
	 * \author Nevrax France
	 * \date 2001
	 */
	class CTip
	{
	public:
		/**
		 * A chain tip. Contains the id of the chain and wether this tip is the start of the chain.
		 * \author Benjamin Legros
		 * \author Nevrax France
		 * \date 2001
		 */
		struct CChainTip
		{
			/// The id of the chain.
			sint32	Chain;

			/// True if this tip is the beginning of the chain.
			bool	Start;

			/// Constructor.
			CChainTip(sint32 chainId = 0, bool start = 0) : Chain(chainId), Start(start) {}

			/// Serielaises the CChainTip
			void	serial(NLMISC::IStream &f) { f.serial(Chain, Start); }
		};

		CTip() : Point(NLMISC::CVector::Null) {}

		/// The position of the tip.x
		NLMISC::CVector					Point;

		// The chains linked to that tip.
		std::vector<CChainTip>			Chains;

	public:
		/// Serialises the CTip.
		void	serial(NLMISC::IStream &f)
		{
			f.serial(Point);
			f.serialCont(Chains);
		}

		/// Translates the CTip by the translation vector.
		void	translate(const NLMISC::CVector &translation)
		{
			Point += translation;
		}
	};


	/**
	 * A topology. It's a set of surfaces which are definning a connex master surface.
	 * \author Benjamin Legros
	 * \author Nevrax France
	 * \date 2001
	 */
	class CTopology : public std::vector<sint32>
	{
	public:
		void	serial(NLMISC::IStream &f)	{ f.serialCont(*this); }
	};


	/**
	 * An estimation of the position of a point on a specified surface.
	 * The estimated position is LOCAL reference to the retriever axis.
	 * \author Benjamin Legros
	 * \author Nevrax France
	 * \date 2001
	 */
	class CLocalPosition : public ULocalPosition
	{
	public:
		/// Constructor.
		CLocalPosition(sint32 surface=-1, const NLMISC::CVector &estimation=NLMISC::CVector::Null) 
		{
			Surface=surface;
			Estimation=estimation;
		}

		/// Serialises the CLocalPosition.
		//void							serial(NLMISC::IStream &f) { f.serial(Surface, Estimation); }
	};

	/**
	 * The different types of retriever (landscape or interior.)
	 * \author Benjamin Legros
	 * \author Nevrax France
	 * \date 2001
	 */
	enum EType
	{
		Landscape = 0,
		Interior
	};


	/**
	 * The faces used for snapping in interior retrievers.
	 * \author Benjamin Legros
	 * \author Nevrax France
	 * \date 2001
	 */
	class CInteriorFace
	{
	public:
		uint32	Verts[3];
		uint32	Surface;
	public:
		void	serial(NLMISC::IStream &f) { f.serial(Verts[0], Verts[1], Verts[2], Surface); }
	};


protected:
	friend class	CRetrieverInstance;
	
	/// The chains insinde the zone.
	std::vector<COrderedChain>			_OrderedChains;
	std::vector<COrderedChain3f>		_FullOrderedChains;

	/// The chains insinde the zone.
	std::vector<CChain>					_Chains;

	/// The surfaces inside the zone.
	std::vector<CRetrievableSurface>	_Surfaces;

	/// The bbox of the local retriever
	NLMISC::CAABBox						_BBox;

	/// The tips making links between different chains.
	std::vector<CTip>					_Tips;

	/// The chains on the edges of the zone.
	std::vector<uint16>					_BorderChains;

	/// The topologies within the zone.
	std::vector<CTopology>				_Topologies[NumCreatureModels];

	/// The tip recognition threshold
	static const float					_TipThreshold;

	/// The tip recognition threshold
	static const float					_EdgeTipThreshold;

	/// For collisions, the chainquad.
	CChainQuad							_ChainQuad;

	/// The type of the retriever
	EType								_Type;

	/// \name Internior retriever specific
	// @{
	/// The exterior mesh, for collisions
	CExteriorMesh						_ExteriorMesh;

	/// The vertices of the collision mesh
	std::vector<NLMISC::CVector>		_InteriorVertices;

	/// The faces of the collision mesh
	std::vector<CInteriorFace>			_InteriorFaces;

	/// The face selection grid
	CFaceGrid							_FaceGrid;

	/// An human readable identifier of the retriever.
	std::string							_Id;

	// @}

	/// Tells if retriever is loaded
	bool								_Loaded;

public:
	///
	bool								LoadCheckFlag;

private:
	/// The intersection between an ordered chain and the path.
	struct CIntersectionMarker
	{
		float	Position;
		uint16	OChain;
		uint16	Edge;
		bool	In;

		CIntersectionMarker() {}
		CIntersectionMarker(float position, uint16 ochain, uint16 edge, bool in) : Position(position), OChain(ochain), Edge(edge), In(in) {}

		bool	operator< (const CIntersectionMarker &comp) const { return Position < comp.Position; }
	};

public:
	/// @name Constructors
	// @{

	CLocalRetriever();

	// @}

	/// @name Selectors
	// @{

	/// Returns the chain tips inside the local retrievers.
	const std::vector<CTip>				&getTips() const { return _Tips; }
	/// Returns the nth tip in the retriever.
	const CTip							&getTip(uint n) const { return _Tips[n]; }

	/// Returns the ordered chains.
	const std::vector<COrderedChain>	&getOrderedChains() const { return _OrderedChains; }
	/// Returns the nth ordered chain.
	const COrderedChain					&getOrderedChain(uint n) const { return _OrderedChains[n]; }

	/// Returns the full ordered chains.
	const std::vector<COrderedChain3f>	&getFullOrderedChains() const { return _FullOrderedChains; }
	/// Returns the nth full ordered chain.
	const COrderedChain3f				&getFullOrderedChain(uint n) const { return _FullOrderedChains[n]; }

	/// Returns the chains.
	const std::vector<CChain>			&getChains() const { return _Chains; }
	/// retruns the nth chain.
	const CChain						&getChain(uint n) const { return _Chains[n]; }

	/// Returns the ids of the chains on the edges of the retriever.
	const std::vector<uint16>			&getBorderChains() const { return _BorderChains; }
	/// Returns the id of the nth chain on the edges of the retriever.
	uint16								getBorderChain(uint n) const { return _BorderChains[n]; }

	/// Returns the surfaces.
	const std::vector<CRetrievableSurface>	&getSurfaces() const { return _Surfaces; }
	/// Returns the nth surface.
	const CRetrievableSurface			&getSurface(uint n) const { return _Surfaces[n]; }

	/// Returns the type of the retriever
	EType								getType() const { return _Type; }

	/// Returns the bbox
	const NLMISC::CAABBox				&getBBox() const { return _BBox; }

	/// Returns the exterior mesh of the retriever
	const CExteriorMesh					&getExteriorMesh() const { return _ExteriorMesh; }

	/// Returns the interior vertices
	const std::vector<NLMISC::CVector>	&getInteriorVertices() const { return _InteriorVertices; }

	/// Returns the interior faces
	const std::vector<CInteriorFace>	&getInteriorFaces() const { return _InteriorFaces; }

	/// Returns the identifier of the retriever.
	const std::string					&getIdentifier() const { return _Id; }

	/// Is loaded ?
	bool								isLoaded() const { return _Loaded; }

	/// build BBoxes of interior surfaces in surfaceBBoxes (cleared first)
	void								buildInteriorSurfaceBBoxes(std::vector<NLMISC::CAABBox>	&surfaceBBoxes) const;

	/// Builds the polygons (loops) of a given surface
	void								buildSurfacePolygons(uint32 surface, std::list<NLMISC::CPolygon> &polygons) const;

	/// Builds the polygons (loops) of all surfaces in the retriever
	void								buildSurfacePolygons(std::list< std::list<NLMISC::CPolygon> > &polygons) const
	{
		uint	i;
		for (i=0; i<_Surfaces.size(); ++i)
		{
			polygons.push_back();
			buildSurfacePolygons(i, polygons.back());
		}
	}

	/// Builds the polygons (loops) of a given surface
	void								build3dSurfacePolygons(uint32 surface, std::list<NLMISC::CPolygon> &polygons) const;

	/// Builds the polygons (loops) of all surfaces in the retriever
	void								build3dSurfacePolygons(std::list< std::list<NLMISC::CPolygon> > &polygons) const
	{
		uint	i;
		for (i=0; i<_Surfaces.size(); ++i)
		{
			polygons.push_back();
			build3dSurfacePolygons(i, polygons.back());
		}
	}
	// @}


	/// @name Mutators
	//@{

	/// Adds a surface to the local retriever, using its features. Returns the id of the newly created surface.
	sint32								addSurface(uint8 normalq, uint8 orientationq,
												   uint8 mat, uint8 charact, uint8 level,
												   bool isUnderWater, float waterHeight,
												   const NLMISC::CVector &center,
												   const CSurfaceQuadTree &quad);

	/**
	 * Adds a chain to the local retriever, using the vertices of the chain, 
	 * the left and right surfaces id and the edge on which the chain is stuck
	 */
	sint32								addChain(const std::vector<NLMISC::CVector> &vertices,
												 sint32 left, sint32 right);

	/// Set the type of the retriever (see EType)
	void								setType(EType type) { _Type = type; }

	/// Sets the exterior mesh
	void								setExteriorMesh(const CExteriorMesh &em) { _ExteriorMesh = em; }

	/// Sets the bbox of the retriever
	void								setBBox(const NLMISC::CAABBox &bbox) { _BBox = bbox; }


	/// Returns the interior vertices
	std::vector<NLMISC::CVector>		&getInteriorVertices() { return _InteriorVertices; }

	/// Returns the interior faces
	std::vector<CInteriorFace>			&getInteriorFaces() { return _InteriorFaces; }

	/// Returns the identifier of the retriever.
	void								setIdentifier(const std::string &id) { _Id = id; }


	/// Returns the chain quad
	CChainQuad							&getChainQuad() { return _ChainQuad; }


	/// Inits the face grid
	void								initFaceGrid();


	/// Builds topologies tables.
	void								computeTopologies();

	/// Builds tips
	void								computeLoopsAndTips();

	/// Found chains on the edges of the retriever and fills _BorderChains tables.
	void								findBorderChains();

	/// Updates surfaces links from the links contained in the chains...
	void								updateChainIds();


	/// Sorts chains references inside the tips. NOT IMPLEMENTED YET.
	void								sortTips();


	/// Translates the local retriever by the translation vector.
	void								translate(const NLMISC::CVector &translation);


	///
	void								flushFullOrderedChains() { _FullOrderedChains.clear(); }

	///
	void								setFullOrderedChains(const std::vector<COrderedChain3f> &foc)	{ _FullOrderedChains = foc; }


	/// Serialises the CLocalRetriever.
	void								serial(NLMISC::IStream &f);

	/// Clear
	void								clear();

	// @}

	/// \name  Collisions part.
	// @{
	/// compute the chain quad, used for collisions. the ChainQuad is serialised in serial(). _OrderedChains must be OK.
	void			computeCollisionChainQuad();
	/** add possible collisions chains to the temp result.
	 * \param cst the temp result to store collision chains. they are appened to cst.CollisionChains.
	 * \param bboxMove the bbox which bounds the movement of the entity.
	 * \param transBase the vector we use to translate local position of edge.
	 */
	void			testCollision(CCollisionSurfaceTemp &cst, const NLMISC::CAABBox &bboxMove, const NLMISC::CVector2f &transBase) const;

	// @}

/*
protected:
*/

	/// Assures a position is really inside a surface (and independent from any accuracy issue) and returns true if the position was moved.
	bool								insurePosition(ULocalPosition &local) const;

	/// Retrieves a position inside the retriever (from the local position), returns true if the position is close to a border
	void								retrievePosition(NLMISC::CVector estimated, /*std::vector<uint8> &retrieveTable,*/ CCollisionSurfaceTemp &cst) const;

	/// Snaps on the ground
	void								snapToInteriorGround(ULocalPosition &position, bool &snapped) const;

	///
	float								getHeight(const ULocalPosition &position) const;

	/// Finds a path in a given surface, from the point A to the point B.
	void								findPath(const CLocalPosition &A, const CLocalPosition &B, std::vector<CVector2s> &path, NLPACS::CCollisionSurfaceTemp &cst) const;


	///
	void								unify();

public:
	const NLMISC::CVector				&getStartVector(uint32 chain) const;
	const NLMISC::CVector				&getStopVector(uint32 chain) const;

	const NLMISC::CVector				&getStartVector(uint32 chain, sint32 surface) const;
	const NLMISC::CVector				&getStopVector(uint32 chain, sint32 surface) const;

	uint16								getStartTip(uint32 chain, sint32 surface) const;
	uint16								getStopTip(uint32 chain, sint32 surface) const;
	
	void								setStartTip(uint32 chain, sint32 surface, uint16 startTip);
	void								setStopTip(uint32 chain, sint32 surface, uint16 stopTip);

	uint32								getPreviousChain(uint32 chain, sint32 surface) const;
	uint32								getNextChain(uint32 chain, sint32 surface) const;

public:
	void								dumpSurface(uint surf, const NLMISC::CVector &vect = NLMISC::CVector::Null) const;

	float								distanceToBorder(const ULocalPosition &pos) const;
};

}; // NLPACS

#endif // NL_LOCAL_RETRIEVER_H

/* End of local_retriever.h */
