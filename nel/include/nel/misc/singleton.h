/** \file singleton.h
 * This class is used to create singleton class following the singleton design pattern
 *
 * $Id: singleton.h,v 1.3.6.3 2006/01/27 12:59:36 mitchell Exp $
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

namespace NLMISC 
{
		
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
		virtual ~CSingleton() {}

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



	/** A variant of the singleton, not fully compliant with the standard design pattern
	 *	It is more appropriate for object built from a factory but that must
	 *	be instanciate only once.
	 *	The singleton paradigm allow easy access to the unique instance but
	 *	I removed the automatic instanciation of getInstance().
	 *
	 *	Consequently, the getInstance return a pointer that can be NULL
	 *	if the singleton has not been build yet.
	 *
	 * Example:
	 * \code
		struct CFooSingleton : public CManualSingleton<CFooSingleton>
		{
			void foo() { nlinfo("foo!"); }
		};	

		// create an instance by any mean
		CFooSingleton	mySingleton

		// call the foo function:
		CFooSingleton::getInstance()->foo();

  		// create another instance is forbiden
		CFooSingleton	otherInstance;		// ASSERT !


	 * \endcode
	 * \author Boris 'sonix' Boucher
	 * \author Nevrax France
	 * \date 2005
	 */

	template <class T>
	class CManualSingleton
	{
		static T *&_instance()
		{
			static T *instance = NULL;

			return instance;
		}

	protected:


		CManualSingleton()
		{
			nlassert(_instance() == NULL);
			_instance() = static_cast<T*>(this);
		}

		~CManualSingleton()
		{
			nlassert(_instance() == this);
			_instance() = NULL;
		}

	public:
		
		static T* getInstance()
		{
			nlassert(_instance() != NULL);

			return _instance();
		}
	};


} // NLMISC

#endif // NL_SINGLETON_H

/* End of singleton.h */
