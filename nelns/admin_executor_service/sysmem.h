/** \file sysmem.h
 *
 * $Id: sysmem.h,v 1.1 2001/04/18 13:54:25 valignat Exp $
 */

/* Copyright, 2000 Nevrax Ltd.
 *
 * This file is part of NEVRAX NeL Network Services
 * NEVRAX NeL Network Services is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * NEVRAX NeL Network Services is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NeL Network Services; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */


#ifndef NL_SYSMEM_H
#define NL_SYSMEM_H


class CSysMemory
{
public:
	CSysMemory  ( void );
	~CSysMemory ( void );

	/// Returns the percentage of used memory
	sint getMemoryUsage ( void );
};


#endif // NL_SYSMEM_H

// End of sysmem.h
