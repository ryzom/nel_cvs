/* debug.cpp
 *
 * Copyright, 2000 Nevrax Ltd.
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

/*
 * $Id: debug.cpp,v 1.1 2000/10/04 15:03:28 cado Exp $
 *
 * <Replace this by a description of the file>
 */

#include "nel/misc/debug.h"


namespace NLMISC {

CLog ErrorLog( LOG_ERROR, true );
CLog WarningLog( LOG_WARNING, true );
CLog InfoLog( LOG_INFO, true );
CLog DebugLog( LOG_DEBUG, false );
CLog StatLog( LOG_STAT, true );

} // NLMISC
