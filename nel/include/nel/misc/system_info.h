/** \file system_info.h
 * Retrieves system information (OS, processor, memory, etc...)
 *
 * $Id: system_info.h,v 1.5 2002/08/23 12:18:13 lecroart Exp $
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

#ifndef NL_SYSTEM_INFO_H
#define NL_SYSTEM_INFO_H

#include "nel/misc/types_nl.h"

#include <string>

namespace NLMISC {


/**
 * <Class description>
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2000
 */
class CSystemInfo
{
public:

	static std::string getOS ();
	static std::string getProc ();
	static std::string getMem ();

	/** test wether the cpuid instruction is supported
	  * (always false on non intel architectures)
	  */
	static bool hasCPUID ();

	/** helps to know wether the processor features mmx instruction set 
	  * This is initialized at started, so its fast
	  * (always false on non 0x86 architecture ...)
	  */	  
	static bool hasMMX ();

	/** helps to know wether the processor has streaming SIMD instructions (the OS must supports it)
	  * This is initialized at started, so its fast
	  * (always false on non 0x86 architecture ...)
	  */
	static bool hasSSE ();

	static bool isNT();
};


} // NLMISC


#endif // NL_SYSTEM_INFO_H

/* End of system_info.h */
