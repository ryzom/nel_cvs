/** \file debug_displayer.h
 * <File description>
 *
 * $Id: debug_displayer.h,v 1.1 2001/12/12 15:34:20 legros Exp $
 */

/* Copyright, 2000, 2001 Nevrax Ltd.
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

#ifndef NL_DEBUG_DISPLAYER_H
#define NL_DEBUG_DISPLAYER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/displayer.h"

#include <string>


namespace NLMISC {


/**
 * <Class description>
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class CDebugDisplayer : public IDisplayer
{
public:
	/// Constructor
	CDebugDisplayer (const std::string &filename, bool eraseLastLog = false, const char *displayerName = "");

	/// Constructor
	CDebugDisplayer (const char *displayerName = "");

	/// Destructor
	virtual ~CDebugDisplayer ();

	/// Set Parameter of the displayer if not set at the ctor time
	void			setParam (const std::string &filename, bool eraseLastLog = false);

	///
	void			flush();

protected:
	/// Put the string into the file.
    virtual void	doDisplay ( const TDisplayInfo& args, const char *message );

private:
	std::string					_FileName;

	bool						_NeedHeader;

	std::string					_String;
};


} // NLMISC


#endif // NL_DEBUG_DISPLAYER_H

/* End of debug_displayer.h */
