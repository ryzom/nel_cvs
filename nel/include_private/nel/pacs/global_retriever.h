/** \file global_retriever.h
 * 
 *
 * $Id: global_retriever.h,v 1.8 2001/06/06 09:34:03 corvazier Exp $
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

#ifndef NL_GLOBAL_RETRIEVER_H
#define NL_GLOBAL_RETRIEVER_H

#include <vector>
#include <list>

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/file.h"

#include "nel/misc/vector.h"
#include "nel/misc/vectord.h"
#include "nel/misc/aabbox.h"

#include "nel/pacs/local_retriever.h"
#include "nel/pacs/retriever_instance.h"
#include "nel/pacs/u_global_retriever.h"


namespace NLPACS
{

class CRetrieverBank;


/**
 * A class that allows to retrieve surface in a large amount of zones (referred as instances.)
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class CGlobalRetriever : public UGlobalRetriever
{
public:

	/**
	 * The global position in the the global retriever.
	 * Contains an instance id and a local position within the instance.
	 * \author Benjamin Legros
	 * \author Nevrax France
	 * \date 2001
	 */
	class CGlobalPosition
	{
	public:
		/// The id of the instance referred by this global position.
		sint32							InstanceId;

		/// The local position within the CLocalRetriever referred by the instance (cf. InstanceId)
		CLocalRetriever::CLocalPosition	LocalPosition;
	public:
		/**
		 * Constuctor.
		 * Creates a CGlobalPosition from an instanceId and a local position.
		 */
		CGlobalPosition(sint32 instanceId=-1, 
					   const CLocalRetriever::CLocalPosition &localPosition=CLocalRetriever::CLocalPosition::CLocalPosition())
			: InstanceId(instanceId), LocalPosition(localPosition) { }

		/// Serialises the global position.
		void							serial(NLMISC::IStream &f) { f.serial(InstanceId, LocalPosition); }
	};

protected:

	/// The CRetrieverBank where the commmon retrievers are stored.
	const CRetrieverBank			*_RetrieverBank;

	/** 
	 * The instance grid that composes the global retriever.
	 * Please note that the grid is rows/lines ordered the way of an excel sheet, e.g.
	 * one row right means increasing x coordinate, and one line down means decreasing y coordinate.
	 */
	std::vector<CRetrieverInstance>	_Instances;

	/// The width of the grid of instances.
	uint16							_Width;

	/// The height of the grid of instances.
	uint16							_Height;

	/// The axis aligned bounding box of the global retriever.
	NLMISC::CAABBox					_BBox;

public:
	/**
	 * Constructor.
	 * Creates a global retriever with given width, height and retriever bank.
	 */
	CGlobalRetriever(uint width=0, uint height=0, const CRetrieverBank *bank=NULL) 
		: _Width(width), _Height(height), _RetrieverBank(bank), _Instances(width*height)
	{ }


	/// @name Selectors
	//@{

	/// Gets the width (in number of rows) of the global retriever.
	uint16							getWidth() const { return _Width; }

	/// Get the height (in number of lines) of the global retriever.
	uint16							getHeight() const { return _Height; }


	/// Gets the BBox of the global retriever.
	const NLMISC::CAABBox			&getBBox() const { return _BBox; }


	/// Gets the vector of retriever instances that compose the global retriever.
	const std::vector<CRetrieverInstance>	getInstances() const { return _Instances; }

	/// Gets the retriever instance referred by its id.
	const CRetrieverInstance		&getInstance(uint id) const { return _Instances[id]; }
	/// Gets the retriever instance referred by its row and line position.
	const CRetrieverInstance		&getInstance(uint x, uint y) const { return _Instances[convertId(x, y)]; }
	/// Gets the retriever instance referred by its global position.
	const CRetrieverInstance		&getInstance(const NLMISC::CVector &p) const;


	/// Get the retriever bank associated to this global retriever.
	const CRetrieverBank			*getRetrieverBank() const { return _RetrieverBank; }

	//@}

	/// @name Position retrieving methods.
	//@{

	/// Retrieves the position of an estimated point in the global retriever.
	CGlobalPosition					retrievePosition(const NLMISC::CVector &estimated);

	/// Converts a global position object into a 'human-readable' CVector.
	NLMISC::CVector					getGlobalPosition(const CGlobalPosition &global) const;

	/// Converts a global position object into a 'human-readable' CVector (double instead.)
	NLMISC::CVectorD				getDoubleGlobalPosition(const CGlobalPosition &global) const;

	//@}

	/// @name Misc. safe methods.
	//@{

	/// Converts an id into a couple of (row,line) coordinates.
	void							convertId(uint id, uint &x, uint &y) const
	{
		x = id % _Width;
		y = id / _Width;
		nlassert(y < _Height);
	}

	/// Converts a couple of (row,line) coordinates into an id.
	uint							convertId(uint x, uint y) const	{ return y*_Width+x; }

	/// Returns the center of an instance of the retriever specified by its (row,line) coordinates.
	NLMISC::CVector					getInstanceCenter(uint x, uint y) const;

	/// Returns the center of an instance of the retriever specified by its id.
	NLMISC::CVector					getInstanceCenter(uint id) const
	{
		uint	x, y;
		convertId(id, x, y);
		return getInstanceCenter(x, y);
	}

	/// get instances which intersect the bbox. result: any instance(x,y), with  x0<=x<x1  &&  y0<=y<y1.
	void							getInstanceBounds(sint32 &x0, sint32 &y0, sint32 &x1, sint32 &y1, const NLMISC::CAABBox &bbox) const;

	//@}



	/// @name Mutators
	//@{

	/** 
	 * Sets the width of the global retriever.
	 * The previous instances are NOT reset! Set width/height/center before any instance addition.
	 */
	void							setWidth(uint16 width) { _Width = width; _Instances.resize(_Width*_Height); }
	/** 
	 * Sets the height of the global retriever.
	 * The previous instances are NOT reset! Set width/height/center before any instance addition.
	 */
	void							setHeight(uint16 height) { _Height = height; _Instances.resize(_Width*_Height); }
	/** 
	 * Sets the center of the global retriever.
	 * The previous instances are NOT translated! Set width/height/center before any instance addition.
	 */
	void							setCenter(const NLMISC::CVector &center) { _BBox.setCenter(center); }

	/**
	 * Updates the bbox of the global retriever.
	 * The previous instances are NOT updated! Set width/height/center before any instance addition.
	 */
	void							updateBBox() { _BBox.setSize(NLMISC::CVector(_Width*160.0f, _Height*160.0f, 20000.0f)); }

	/** 
	 * Sets directly the bbox of the global retriever.
	 * Better use setWidth(), setHeight(), setCenter() and updateBBox().
	 */
	void							setBBox(const NLMISC::CAABBox &bbox) { _BBox = bbox; }

	/// Creates an instance of local retriever at position (x,y), with the given retriever and orientation.
	CRetrieverInstance				&makeInstance(uint x, uint y, uint32 retriever, uint8 orientation);
	/**
	 * Creates an instance of local retriever at position (x,y), with the given retriever and orientation.
	 * The instance is eventually placed at the given origin.
	 */
	CRetrieverInstance				&makeInstance(uint x, uint y, uint32 retriever, uint8 orientation, const NLMISC::CVector &origin);

	/// Gets the instance by its id, with full read/write access.
	CRetrieverInstance				&getInstanceFullAccess(uint id) { return _Instances[id]; }

	/// Gets the instance by its (row,line) coordinates, with full read/write access.
	CRetrieverInstance				&getInstanceFullAccess(uint x, uint y)
	{
		nlassert(x < _Width);
		nlassert(y < _Height);
		return _Instances[convertId(x, y)];
	}
	/// Gets the instance by a position inside the instance, with full read/write access.
	CRetrieverInstance				&getInstanceFullAccess(const NLMISC::CVector &position);

	/// Sets the retriever bank.
	void							setRetrieverBank(const CRetrieverBank *bank) { _RetrieverBank = bank; }


	/// Resets all links within the global retriever.
	void							resetAllLinks();

	/// Links the instance referred by its id to its 4 neighbors.
	void							makeLinks(uint n);
	/// Links all the instances inside the global retriever.
	void							makeAllLinks();

	/// Serialises the global retriever.
	void							serial(NLMISC::IStream &f);

	//@}


	/// \name  Collisions part.
	// @{
	/** Test a movement of a cylinder against surface world.
	 * \param start is the start position of the movement.
	 * \param delta is the requested movement.
	 * \param radius is the radius of the vertical cylinder.
	 * \param cst is the CCollisionSurfaceTemp object used as temp copmputing (one per thread).
	 * \return list of collision against surface, ordered by increasing time. this is a synonym for
	 * cst.CollisionDescs. NB: this array may be modified by CGlobalRetriever on any collision call.
	 */
	const TCollisionSurfaceDescVector	&testCylinderMove(const CGlobalPosition &start, const NLMISC::CVector &delta, 
		float radius, CCollisionSurfaceTemp &cst) const;
	/** Test a movement of a bbox against surface world.
	 * \param start is the start position of the movement.
	 * \param delta is the requested movement.
	 * \param locI is the oriented I vector of the BBox.  I.norm()== Width/2.
	 * \param locJ is the oriented J vector of the BBox.  J.norm()== Height/2.
	 * \param cst is the CCollisionSurfaceTemp object used as temp copmputing (one per thread).
	 * \return list of collision against surface, ordered by increasing time. this is a synonym for
	 * cst.CollisionDescs. NB: this array may be modified by CGlobalRetriever on any collision call.
	 */
	const TCollisionSurfaceDescVector	&testBBoxMove(const CGlobalPosition &start, const NLMISC::CVector &delta, 
		const NLMISC::CVector &locI, const NLMISC::CVector &locJ, CCollisionSurfaceTemp &cst) const;
	/** apply a movement of a point against surface world. This should be called after test???Move().
	 * NB: It's up to you to give good t, relative to result of test???Move(). Else, undefined results...
	 * NB: if you don't give same start/delta as in preceding call to testMove(), and rebuildChains==false,
	 *	start is returned (nlstop in debug).
	 *
	 * \param start is the start position of the movement. (must be same as passed in test???Move()).
	 * \param delta is the requested movement (must be same as passed in test???Move()).
	 * \param t must be in [0,1]. t*delta is the actual requested movement.
	 * \param cst is the CCollisionSurfaceTemp object used as temp computing (one per thread). (must be same as passed in test???Move()).
	 * \param rebuildChains true if doMove() is not called just after the testMove(). Then CGlobalRetriever must recompute some part
	 *	of the data needed to performing his task.
	 * \return new position of the entity.
	 */
	CGlobalPosition		doMove(const CGlobalPosition &start, const NLMISC::CVector &delta, float t, CCollisionSurfaceTemp &cst, bool rebuildChains=false) const;
	/** retrieve a surface by its Id. NULL if not found or if -1.
	 */
	const CRetrievableSurface	*getSurfaceById(const CSurfaceIdent &surfId);
	/** Test a rotation of a BBox against the surfaces.
	 * NB: this function is not perfect because a ContactSurface may appears 2+ times in the returned array.
	 * \param start is the center of the bbox.
	 * \param locI is the new oriented I vector of the BBox.  I.norm()== Width/2.
	 * \param locJ is the new oriented J vector of the BBox.  J.norm()== Height/2.  NB : must have locI^locJ== aK (a>0)
	 * \param cst is the CCollisionSurfaceTemp object used as temp copmputing (one per thread).
	 * \return list of collision against surface (ContactTime and ContactNormal has no means). this is a synonym for
	 * cst.CollisionDescs. NB: this array may be modified by CGlobalRetriever on any collision call.
	 */
	const TCollisionSurfaceDescVector	&testBBoxRot(const CGlobalPosition &start, 
		const NLMISC::CVector &locI, const NLMISC::CVector &locJ, CCollisionSurfaceTemp &cst) const;

	/** return the mean height of the surface under pos..
	 *
	 */
	float				getMeanHeight(const CGlobalPosition &pos);
	// @}


	/// \name  A* part.
	// @{

	/// Finds an A* path from a given global position to another.
	// TO DO: secure search to avoid crashes...
	void							findAStarPath(const CGlobalPosition &begin, const CGlobalPosition &end, std::list<CRetrieverInstance::CAStarNodeAccess> &path);

	// @}

private:
	/// \name  A* part.
	// @{

	/// Gets the CAStarNodeInfo referred by its access.
	CRetrieverInstance::CAStarNodeInfo	&getNode(CRetrieverInstance::CAStarNodeAccess &access)
	{
		return _Instances[access.InstanceId]._NodesInformation[access.NodeId];
	}

	// @}


	/// \name  Collisions part.
	// @{
	enum	TCollisionType { Circle, BBox };
	/** reset and fill cst.CollisionChains with possible collisions in bboxMove+origin.
	 * result: collisionChains, computed localy to origin.
	 */
	void	findCollisionChains(CCollisionSurfaceTemp &cst, const NLMISC::CAABBox &bboxMove, const NLMISC::CVector &origin) const;
	/** reset and fill cst.CollisionDescs with effective collisions against current cst.CollisionChains.
	 * result: new collisionDescs in cst.
	 */
	void	testCollisionWithCollisionChains(CCollisionSurfaceTemp &cst, const CVector2f &startCol, const CVector2f &deltaCol,
		CSurfaceIdent startSurface, float radius, const CVector2f bbox[4], TCollisionType colType) const;
	/** reset and fill cst.MoveDescs with effective collisions of a point movement against current cst.CollisionChains.
	 * result: the surfaceIdent where we stop. -1 if we traverse a Wall, which should not happen because of collision test.
	 * NB: for precision pb, startCol and deltaCol should be snapped on a grid of 1/1024 meters, using snapVector().
	 * NB: for precision pb (stop on edge etc....), return a "Precision problem ident", ie (-2,-2).
	 */
	CSurfaceIdent	testMovementWithCollisionChains(CCollisionSurfaceTemp &cst, const CVector2f &startCol, const CVector2f &deltaCol,
		CSurfaceIdent startSurface) const;
	/** reset and fill cst.CollisionDescs with effective collisions against current cst.CollisionChains.
	 * result: new collisionDescs in cst.
	 */
	void	testRotCollisionWithCollisionChains(CCollisionSurfaceTemp &cst, const CVector2f &startCol, CSurfaceIdent startSurface, const CVector2f bbox[4])  const;
	// @}


};

}; // NLPACS

#endif // NL_GLOBAL_RETRIEVER_H

/* End of global_retriever.h */
