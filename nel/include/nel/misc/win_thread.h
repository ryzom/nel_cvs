/** \file win_thread.h
 * Windows implementation of CThread class (look at thread.h)
 *
 * $Id: win_thread.h,v 1.6 2001/06/21 12:35:16 lecroart Exp $
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

#ifndef NL_WIN_THREAD_H
#define NL_WIN_THREAD_H

#include "nel/misc/types_nl.h"
#include "nel/misc/thread.h"

#ifdef NL_OS_WINDOWS

namespace NLMISC {


/**
 * Windows implementation of CThread class (look thread.h)
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2000
 */
class CWinThread : public IThread
{
public:

	/// Constructor
	CWinThread(IRunnable *runnable);

	virtual ~CWinThread();
	
	virtual void start();
	virtual void terminate();
	virtual void wait();

	virtual IRunnable *getRunnable()
	{
		return Runnable;
	}

	/// private use
	IRunnable	*Runnable;

private:

	void		*ThreadHandle;	// HANDLE	don't put it to avoid including windows.h
	uint32		ThreadId;		// DWORD	don't put it to avoid including windows.h
};


} // NLMISC

#endif // NL_OS_WINDOWS

#endif // NL_WIN_THREAD_H

/* End of win_thread.h */
