/** \file path.h
 * CPath
 *
 * $Id: path.h,v 1.1 2000/12/11 17:49:08 cado Exp $
 */

/* Copyright, 2000 Nevrax Ltd.
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

#ifndef NL_PATH_H
#define NL_PATH_H

#include "nel/misc/types_nl.h"
#include <string>
#include <vector>


namespace NLMISC {


/// Vectors of strings
typedef std::vector<std::string> CStringVector;


// EPathNotFound
class EPathNotFound : public Exception
{
public:
	EPathNotFound( const std::string& filename )
	{
		_Reason = "Path not found for "+filename;
	}

	virtual const char	*what() const throw() { return _Reason.c_str(); }

private:
	
	std::string	_Reason;
};


/**
 * Utility class for search paths
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2000
 */
class CPath
{
public:

	/// Adds a search path. The separator for directories is '/'.
	static void			addSearchPath( const std::string& path );

	/** Returns the long name (path and filename) for the specified file, using search paths
	 * stored by addSearchPath in the same order as they were added.
	 * If no path is found where path/file exists, an exception EPathNotFound is raised.
	 */
	static std::string	lookup( const std::string& filename );

private:

	static CStringVector	_SearchPaths;
};


} // NLMISC


#endif // NL_PATH_H

/* End of path.h */
