/** \file global_retriever.h
 * 
 *
 * $Id: global_retriever.h,v 1.3 2001/05/16 15:17:12 berenguier Exp $
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
#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/file.h"

#include "nel/misc/vector.h"
#include "nel/misc/vectord.h"
#include "nel/misc/aabbox.h"

#include "nel/pacs/local_retriever.h"
#include "nel/pacs/retriever_instance.h"


namespace NLPACS
{

class CRetrieverBank;

/**
 * A class that allows to retrieve surface in a large amount of zones (referred as instances.)
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class CGlobalRetriever
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
		sint32							InstanceId;
		CLocalRetriever::CLocalPosition	LocalPosition;
	public:
		CGlobalPosition(sint32 instanceId=-1, 
					   const CLocalRetriever::CLocalPosition &localPosition=CLocalRetriever::CLocalPosition::CLocalPosition())
			: InstanceId(instanceId), LocalPosition(localPosition) { }
		void							serial(NLMISC::IStream &f) { f.serial(InstanceId, LocalPosition); }
	};

protected:

	const CRetrieverBank			*_RetrieverBank;

	std::vector<CRetrieverInstance>	_Instances;

	uint16							_Width;
	uint16							_Height;

	NLMISC::CAABBox					_BBox;

public:
	// Constructors

	CGlobalRetriever(uint width=0, uint height=0, const CRetrieverBank *bank=NULL) 
		: _Width(width), _Height(height), _RetrieverBank(bank), _Instances(width*height)
	{ }


	// Selectors

	uint16							getWidth() const { return _Width; }
	uint16							getHeight() const { return _Height; }

	const NLMISC::CAABBox			&getBBox() const { return _BBox; }

	const std::vector<CRetrieverInstance>	getInstances() const { return _Instances; }

	const CRetrieverInstance		&getInstance(uint id) const { return getInstance(id); }
	const CRetrieverInstance		&getInstance(uint x, uint y) const { return getInstance(x, y); }
	const CRetrieverInstance		&getInstance(const NLMISC::CVector &p) const { return getInstance(p); }

	const CRetrieverBank			*getRetrieverBank() const { return _RetrieverBank; }

	CGlobalPosition					retrievePosition(const NLMISC::CVector &estimated);
	NLMISC::CVector					getGlobalPosition(const CGlobalPosition &global) const;
	NLMISC::CVectorD				getDoubleGlobalPosition(const CGlobalPosition &global) const;

	void							convertId(uint id, uint &x, uint &y) const
	{
		x = id % _Width;
		y = id / _Width;
		nlassert(y < _Height);
	}
	uint							convertId(uint x, uint y) const	{ return y*_Width+x; }

	NLMISC::CVector					getInstanceCenter(uint x, uint y) const;
	NLMISC::CVector					getInstanceCenter(uint id) const
	{
		uint	x, y;
		convertId(id, x, y);
		return getInstanceCenter(x, y);
	}


	/// get instances which intersect the bbox. result: any instance(x,y), with  x0<=x<x1  &&  y0<=y<y1.
	void							getInstanceBounds(sint32 &x0, sint32 &y0, sint32 &x1, sint32 &y1, const NLMISC::CAABBox &bbox) const;


	// Mutators

	void							setWidth(uint16 width) { _Width = width; _Instances.resize(_Width*_Height); }
	void							setHeight(uint16 height) { _Height = height; _Instances.resize(_Width*_Height); }

	void							setBBox(const NLMISC::CAABBox &bbox) { _BBox = bbox; }

	CRetrieverInstance				&makeInstance(uint x, uint y, uint32 retriever, uint8 orientation, const NLMISC::CVector &origin);

	CRetrieverInstance				&getInstanceFullAccess(uint id) { return _Instances[id]; }
	CRetrieverInstance				&getInstanceFullAccess(uint x, uint y)
	{
		nlassert(x < _Width);
		nlassert(y < _Height);
		return _Instances[convertId(x, y)];
	}
	CRetrieverInstance				&getInstanceFullAccess(const NLMISC::CVector &position);

	void							setRetrieverBank(const CRetrieverBank *bank) { _RetrieverBank = bank; }

	void							makeLinks(uint n);
	void							resetAllLinks();
	void							makeAllLinks();


	// Others

	void							serial(NLMISC::IStream &f);


	/// \name  Collisions part.
	// @{
	/** Test a movement of a cylinder against surface world.
	 * \param start is the start position of the movement.
	 * \param delta is the requested movement.
	 * \param radius is the radius of the vertical cylinder.
	 * \param cst is the CCollisionSurfaceTemp object used as temp copmputing (one per thread).
	 * \return list of collision against surface, ordered by increasing time. NB: this array may modified by CGlobalRetriever on
	 *	any collision call.
	 */
	const	std::vector<CCollisionSurfaceDesc>	
		&testCylinderMove(const CGlobalPosition &start, const NLMISC::CVector &delta, float radius, CCollisionSurfaceTemp &cst) const;
	/** apply a movement of a point against surface world. This must be called after test???Move().
	 * NB: It's up to you to give good t, relative to result of test???Move(). Else, undefined results...
	 * NB: if you don't give same start/delta as in preceding call to testMove(), start is returned.
	 *
	 * \param start is the start position of the movement. (must be same as passed in test???Move()).
	 * \param delta is the requested movement (must be same as passed in test???Move()).
	 * \param t must be in [0,1]. t*delta is the actual requested movement.
	 * \param cst is the CCollisionSurfaceTemp object used as temp computing (one per thread). (must be same as passed in test???Move()).
	 * \return new position of the entity.
	 */
	CGlobalPosition		doMove(const CGlobalPosition &start, const NLMISC::CVector &delta, float t, CCollisionSurfaceTemp &cst) const;
	// @}



	// A* methods
public:
	/// Finds an A* path from a given global position to another.
	void								findAStarPath(const CGlobalPosition &begin, const CGlobalPosition &end);

private:
	CRetrieverInstance::CAStarNodeInfo	&getNode(CRetrieverInstance::CAStarNodeAccess &access)
	{
		return _Instances[access.InstanceId]._NodesInformation[access.NodeId];
	}


	/// \name  Collisions part.
	// @{
	// TODO_BBOX.
	enum	TCollisionType { Circle, Point };
	/** reset and fill cst.CollisionChains with possible collisions in bboxMove+origin.
	 * result: collisionChains, computed localy to origin.
	 */
	void	findCollisionChains(CCollisionSurfaceTemp &cst, const NLMISC::CAABBox &bboxMove, const NLMISC::CVector &origin) const;
	/** reset and fill cst.CollisionDescs with effective collisions against current cst.CollisionChains.
	 * result: new collisionDescs in cst.
	 * NB: with colType==Point, normal in cst.CollisionDescs are undefined.
	 */
	void	testCollisionWithCollisionChains(CCollisionSurfaceTemp &cst, const CVector2f &startCol, const CVector2f &deltaCol,
		CSurfaceIdent startSurface, float radius, TCollisionType colType) const;
	// @}


};

}; // NLPACS

#endif // NL_GLOBAL_RETRIEVER_H

/* End of global_retriever.h */
