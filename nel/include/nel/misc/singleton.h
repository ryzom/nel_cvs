/** \file singleton.h
 * This class is used to create singleton class following the singleton design pattern
 *
 * $Id: singleton.h,v 1.3 2005/06/23 16:27:15 boucher Exp $
 */

/* Copyright, 2004 Nevrax Ltd.
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

#ifndef NL_SINGLETON_H
#define NL_SINGLETON_H

#include "nel/misc/common.h"
#include "nel/misc/thread.h"
#include "nel/misc/app_context.h"

namespace NLMISC {
	
/**
 * Example:
 * \code
	struct CFooSingleton : public CSingleton<CFooSingleton>
	{
		void foo() { nlinfo("foo!"); }
	};	

	// call the foo function:
	CFooSingleton::getInstance().foo();

 * \endcode
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2004
 */

template<class T>
class CSingleton
{
public:

	/// returns a reference and not a pointer to be sure that the user
	/// doesn't have to test the return value and can directly access the class
	static T &getInstance()
	{
		if(!Instance)
		{
			Instance = new T;
			nlassert(Instance);
		}
		return *Instance;
	}

protected:

	/// no public ctor to be sure that the user can't create an instance
	CSingleton()
	{
	}

	static T *Instance;
};

template <class T>
T* CSingleton<T>::Instance = 0;




} // NLMISC

#endif // NL_SINGLETON_H

/* End of singleton.h */
