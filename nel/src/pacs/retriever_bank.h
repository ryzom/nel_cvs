/** \file retriever_bank.h
 * 
 *
 * $Id: retriever_bank.h,v 1.6 2002/08/21 09:41:34 lecroart Exp $
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

#ifndef NL_RETRIEVER_BANK_H
#define NL_RETRIEVER_BANK_H

#include <vector>
#include <string>

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/file.h"

#include "pacs/local_retriever.h"
#include "nel/pacs/u_retriever_bank.h"

namespace NLPACS
{

/**
 * A bank of retrievers, shared by several global retrievers.
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class CRetrieverBank : public URetrieverBank
{
protected:
	/// The retrievers stored in the retriever bank.
	std::vector<CLocalRetriever>		_Retrievers;

public:
	/// Returns the vector of retrievers.
	const std::vector<CLocalRetriever>	&getRetrievers() const { return _Retrievers; }

	/// Returns the number of retrievers in the bank.
	uint								size() const { return _Retrievers.size(); }

	/// Gets nth retriever.
	const CLocalRetriever				&getRetriever(uint n) const { return _Retrievers[n]; }

	/// Adds the given retriever to the bank.
	uint								addRetriever(const CLocalRetriever &retriever) { _Retrievers.push_back(retriever); return _Retrievers.size()-1; }

	/// Loads the retriever named 'filename' (using defined search paths) and adds it to the bank.
	uint								addRetriever(const std::string &filename)
	{
		NLMISC::CIFile	input;
		_Retrievers.resize(_Retrievers.size()+1);
		CLocalRetriever	&localRetriever = _Retrievers.back();
		nldebug("load retriever file %s", filename.c_str());
		input.open(filename);
		localRetriever.serial(input);
		input.close();

		return _Retrievers.size()-1;
	}

	/// Cleans the bank up.
	void								clean();

	/// Serialises this CRetrieverBank.
	void								serial(NLMISC::IStream &f)
	{
		/*
		Version 0:
			- base version.
		*/
		(void)f.serialVersion(0);

		f.serialCont(_Retrievers);
	}
};

}; // NLPACS

#endif // NL_RETRIEVER_BANK_H

/* End of retriever_bank.h */
