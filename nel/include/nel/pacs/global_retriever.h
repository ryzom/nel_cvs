/** \file global_retriever.h
 * 
 *
 * $Id: global_retriever.h,v 1.2 2001/05/15 08:03:09 legros Exp $
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



	// A* methods
public:
	/// Finds an A* path from a given global position to another.
	void								findAStarPath(const CGlobalPosition &begin, const CGlobalPosition &end);

private:
	CRetrieverInstance::CAStarNodeInfo	&getNode(CRetrieverInstance::CAStarNodeAccess &access)
	{
		return _Instances[access.InstanceId]._NodesInformation[access.NodeId];
	}


};

}; // NLPACS

#endif // NL_GLOBAL_RETRIEVER_H

/* End of global_retriever.h */
