/** \file shared_memory.h
 * Encapsulation of shared memory APIs
 *
 * $Id: shared_memory.h,v 1.3 2002/08/08 14:42:05 cado Exp $
 */

/* Copyright, 2000-2002 Nevrax Ltd.
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

#ifndef NL_SHARED_MEMORY_H
#define NL_SHARED_MEMORY_H

#include "nel/misc/types_nl.h"

#ifdef NL_OS_WINDOWS
#include <map>
#endif

namespace NLMISC {


#ifdef NL_OS_WINDOWS
typedef const char *TSharedMemId;
#else
typedef key_t TSharedMemId;
#endif


/**
 * Encapsulation of shared memory APIs.
 * Using file mapping under Windows, System V shared memory (shm) under Linux.
 *
 * Note: under Linux, an option could be added to prevent a segment to be swapped out.
 *
 * \author Olivier Cado
 * \author Nevrax France
 * \date 2002
 */
class CSharedMemory
{
public:

	// Constructor
	//CSharedMemory();

	/** Create a shared memory segment and get access to it. The id must not be used.
	 * \return Access address of the segment of the choosen size
	 */
	static void *		createSharedMemory( TSharedMemId sharedMemId, uint32 size );

	/** Get access to an existing shared memory segment. The id must be used.
	 * \return Access address of the segment. Its size was determined at the creation.
	 */
	static void *		accessSharedMemory( TSharedMemId sharedMemId );

	/** Close a shared memory segment, given the address returned by createSharedMemory() or
	 * accessSharedMemory(). Must be called by each process that called createSharedMemory()
	 * or accessSharedMemory().
	 */
	static bool			closeSharedMemory( void * accessAddress );

  /**
   * Destroy a shared memory segment (must be call by the process that created the segment,
   * not by the accessors).
   *
   * "Rescue feature": set "force" to true if a segment was created and left out of
   * control (meaning a new createSharedMemory() with the same sharedMemId fails), but
   * before, make sure the segment really belongs to you!
   * 
   * Note: this method does nothing under Windows, destroying is automatic.
   */
  static void destroySharedMemory( TSharedMemId sharedMemId, bool force=false );
};


} // NLMISC


#endif // NL_SHARED_MEMORY_H

/* End of shared_memory.h */
