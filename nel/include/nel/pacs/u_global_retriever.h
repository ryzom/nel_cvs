/** \file u_global_retriever.h
 * A class that allows to retrieve surface in a large amount of zones (referred as instances.)
 *
 * $Id: u_global_retriever.h,v 1.7 2001/07/12 14:15:09 legros Exp $
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

#ifndef NL_U_GLOBAL_RETRIEVER_H
#define NL_U_GLOBAL_RETRIEVER_H

#include "nel/misc/types_nl.h"

#include "nel/pacs/u_retriever_bank.h"

namespace NLMISC
{
class CVector;
class CVectorD;
class CAABBox;
}

namespace NLPACS 
{

class UGlobalPosition;

/**
 * A class that allows to retrieve surface in a large amount of zones (referred as instances.)
 *
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2001
 */
class UGlobalRetriever
{
public:

	/// Make a raytrace test. For the time, always return false.
	virtual bool					testRaytrace (const NLMISC::CVectorD &v0, const NLMISC::CVectorD &v1) =0;

	/**
	  * Return the bounding box of the global retriever.
	  */
	virtual const NLMISC::CAABBox	&getBBox() const=0;

	/**
	  * Return the average height for a global position
	  */
	virtual float					getMeanHeight(const UGlobalPosition &pos) =0;

	/**
	  * Retrieves the position of an estimated point in the global retriever.
	  */
	virtual UGlobalPosition			retrievePosition(const NLMISC::CVector &estimated) const =0;

	/**
	  * Retrieves the position of an estimated point in the global retriever (double instead.)
	  */
	virtual UGlobalPosition			retrievePosition(const NLMISC::CVectorD &estimated) const =0;

	/**
	  * Converts a global position object into a 'human-readable' CVector.
	  */
	virtual NLMISC::CVector			getGlobalPosition(const UGlobalPosition &global) const =0;

	/**
	  * Converts a global position object into a 'human-readable' CVector (double instead.)
	  */
	virtual NLMISC::CVectorD		getDoubleGlobalPosition(const UGlobalPosition &global) const =0;

	/**
	  * Create a global retriever.
	  *
	  * \param globalRetriver is the global retriver path file name. This method use the CPath to find the file.
	  * \param retriverBank is the global retriver bank associated to the global retriever.
	  * \return the pointer on the global retriver or NULL if the file is not found.
	  */
	static UGlobalRetriever	*		createGlobalRetriever (const char* globalRetriever, const URetrieverBank* retrieverBank);

	/**
	  * Delete a global retriever.
	  */
	static void						deleteGlobalRetriever (UGlobalRetriever *retriever);
};


} // NLPACS


#endif // NL_U_GLOBAL_RETRIEVER_H

/* End of u_global_retriever.h */
