/** \file u_global_retriever.h
 * A class that allows to retrieve surface in a large amount of zones (referred as instances.)
 *
 * $Id: u_global_retriever.h,v 1.3 2001/06/06 10:17:44 corvazier Exp $
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


namespace NLPACS 
{


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

	/// Constructor
	UGlobalRetriever();

	/// Make a raytrace test. For the time, always return false.
	virtual bool			testRaytrace (const NLMISC::CVectorD &v0, const NLMISC::CVectorD &v1) =0;

	/**
	  * Create a global retriever.
	  *
	  * \param globalRetriver is the global retriver path file name.
	  * \param retriverBank is the global retriver bank path file name.
	  *
	  */
	static UGlobalRetriever	*createGlobalRetriever (const char* globalRetriver, const char* retriverBank);

	/**
	  * Delete a global retriever.
	  *
	  */
	static void				deleteRetriever (UGlobalRetriever *retriever);
};


} // NLPACS


#endif // NL_U_GLOBAL_RETRIEVER_H

/* End of u_global_retriever.h */
