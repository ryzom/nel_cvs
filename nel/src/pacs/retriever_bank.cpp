/** \file retriever_bank.cpp
 *
 *
 * $Id: retriever_bank.cpp,v 1.2 2001/06/08 15:38:28 legros Exp $
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

#include <vector>
#include <list>
#include <map>

#include "nel/misc/types_nl.h"
#include "nel/misc/file.h"
#include "nel/misc/debug.h"

#include "pacs/retriever_bank.h"

using namespace std;
using namespace NLMISC;

// CRetrieverBank methods implementation

NLPACS::URetrieverBank *NLPACS::URetrieverBank::createRetrieverBank (const char *retrieverBank)
{
	CIFile	file;
	file.open(retrieverBank);
	CRetrieverBank	*bank = new CRetrieverBank();
	file.serial(*bank);

	return static_cast<URetrieverBank *>(bank);
}


void	NLPACS::URetrieverBank::deleteRetrieverBank (NLPACS::URetrieverBank *retrieverBank)
{
	// Cast
//	nlassert (dynamic_cast<NLPACS::CRetrieverBank*>(retrieverBank));
	NLPACS::CRetrieverBank* r=static_cast<NLPACS::CRetrieverBank*>(retrieverBank);

	// Delete
	delete r;
}

// end of CRetrieverBank methods implementation
