/** \file retriever_bank.h
 * 
 *
 * $Id: retriever_bank.h,v 1.9 2003/03/24 16:38:54 legros Exp $
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
#include <set>

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/file.h"
#include "nel/misc/common.h"
#include "nel/misc/path.h"

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
	friend class URetrieverBank;

protected:
	/// The retrievers stored in the retriever bank.
	std::vector<CLocalRetriever>		_Retrievers;

	/// All loaded ?
	bool								_AllLoaded;

	/// Bank name prefix
	std::string							_NamePrefix;

	/// The loaded retrievers, if the retriever bank is not in loadAll mode
	std::set<uint>						_LoadedRetrievers;

public:
	/// Constructor
	CRetrieverBank(bool allLoaded = true) : _AllLoaded(allLoaded) {}

	/// Returns the vector of retrievers.
	const std::vector<CLocalRetriever>	&getRetrievers() const { return _Retrievers; }

	/// Returns the number of retrievers in the bank.
	uint								size() const { return _Retrievers.size(); }

	/// Gets nth retriever.
	const CLocalRetriever				&getRetriever(uint n) const
	{
		nlassert(n < _Retrievers.size());
		/* if (!_Retrievers[n].isLoaded())
			nlwarning("Trying to access rbank '%s', retriever %d not loaded", _NamePrefix.c_str(), n); */
		return _Retrievers[n];
	}

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

		if (!_AllLoaded)
		{
			if (f.isReading())
			{
				uint32	num = 0;
				f.serial(num);
				nlinfo("Presetting RetrieverBank '%s', %d retriever slots allocated", _NamePrefix.c_str(), num);
				_Retrievers.resize(num);
			}
			else
			{
				nlwarning("Unable to write incomplete CRetrieverBank '%s' to stream, data might be lost!", _NamePrefix.c_str());
			}
		}
		else
		{
			f.serialCont(_Retrievers);
		}
	}

	/// Write separate retrievers using dynamic filename convention
	void								saveRetrievers(const std::string &path, const std::string &bankPrefix)
	{
		uint	i;
		for (i=0; i<_Retrievers.size(); ++i)
		{
			NLMISC::COFile	f(NLMISC::CPath::standardizePath(path) + bankPrefix + "_" + NLMISC::toString(i) + ".lr");
			f.serial(_Retrievers[i]);
		}
	}

	/// Write separate retrievers using dynamic filename convention
	void								saveShortBank(const std::string &path, const std::string &bankPrefix)
	{
		NLMISC::COFile	f(NLMISC::CPath::standardizePath(path) + "short_" + bankPrefix + ".rbank");

		f.serialVersion(0);

		uint32	num = _Retrievers.size();
		f.serial(num);
	}

	/// @name Dynamic retrieve loading
	// @{

	/// Diff loaded retrievers
	void		diff(const std::set<uint> &newlr, std::set<uint> &in, std::set<uint> &out)
	{
		std::set<uint>::iterator	it;

		for (it=_LoadedRetrievers.begin(); it!=_LoadedRetrievers.end(); ++it)
		{
			uint	n = *it;
			if (n >= _Retrievers.size())
				continue;
			_Retrievers[n].LoadCheckFlag = true;
		}

		for (it=newlr.begin(); it!=newlr.end(); ++it)
		{
			uint	n = *it;
			if (n >= _Retrievers.size())
				continue;
			if (!_Retrievers[n].LoadCheckFlag)
				in.insert(n);
			_Retrievers[n].LoadCheckFlag = false;
		}

		for (it=_LoadedRetrievers.begin(); it!=_LoadedRetrievers.end(); ++it)
		{
			uint	n = *it;
			if (n >= _Retrievers.size())
				continue;
			if (_Retrievers[n].LoadCheckFlag)
				out.insert(n);
			_Retrievers[n].LoadCheckFlag = false;
		}
	}

	/// Loads nth retriever from stream
	void		loadRetriever(uint n, NLMISC::IStream &s)
	{
		if (_AllLoaded || n >= _Retrievers.size() || _Retrievers[n].isLoaded())
		{
			nlwarning("RetrieverBank '%s' asked to load retriever %n whereas not needed, aborted", _NamePrefix.c_str(), n);
			return;
		}

		s.serial(_Retrievers[n]);
		_LoadedRetrievers.insert(n);
	}

	/// Insert a retriever in loaded list
	void		setRetrieverAsLoaded(uint n)
	{
		_LoadedRetrievers.insert(n);
	}

	/// Unload nth retriever
	void		unloadRetriever(uint n)
	{
		if (_AllLoaded || n >= _Retrievers.size() || !_Retrievers[n].isLoaded())
		{
			nlwarning("RetrieverBank '%s' asked to unload retriever %n whereas not needed, aborted", _NamePrefix.c_str(), n);
			return;
		}

		_Retrievers[n].clear();
		_LoadedRetrievers.erase(n);
	}

	///
	const std::string	&getNamePrefix() const	{ return _NamePrefix; }

	///
	bool		allLoaded() const { return _AllLoaded; }

	// @}
};

}; // NLPACS

#endif // NL_RETRIEVER_BANK_H

/* End of retriever_bank.h */
