/** \file retriever_instance.h
 * 
 *
 * $Id: retriever_instance.h,v 1.1 2001/05/04 14:50:49 legros Exp $
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

#ifndef NL_RETRIEVER_INSTANCE_H
#define NL_RETRIEVER_INSTANCE_H

#include <vector>
#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/file.h"

#include "nel/pacs/local_retriever.h"

namespace NLPACS
{

class CRetrieverInstance
{
public:
	class CSurfaceEdge
	{
	public:
		sint32							From;
		sint32							To;
	public:
		CSurfaceEdge(sint32 from=0, sint32 to=0) : From(from), To(to) {}
	};

protected:
	/// The id of this instance.
	sint32								_InstanceId;

	/// The id of the retrievable surface pattern.
	sint32								_RetrieverId;

	/// @name Instance displacement.
	//@{
	uint8								_Orientation;
	NLMISC::CVector						_Origin;
	//@}

	/// The instance ids of the neighbors.
	sint32								_Neighbors[4];

	/// The neighbor tips on each edge (in the corresponding neighbor.)
	std::vector<uint16>					_EdgeTipLinks[4];

	/// The neighbor  chains on each edge (cf tips.)
	std::vector<uint16>					_EdgeChainLinks[4];

public:
	CRetrieverInstance();

	void								make(sint32 instanceId, sint32 retrieverId,
											 uint8 orientation, const NLMISC::CVector &origin);

	void								link(const CRetrieverInstance &neighbor, uint8 edge,
											 const std::vector<CLocalRetriever> &retrievers);

	void								serial(NLMISC::IStream &f);
};

}; // NLPACS

#endif // NL_RETRIEVER_INSTANCE_H

/* End of retriever_instance.h */
