/** \file win_displayer.h
 * Implementation of the CDisplayer (look at displayer.h) that display on a Win32 Windows
 *
 * $Id: win_displayer.h,v 1.5 2001/08/24 09:45:36 lecroart Exp $
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

#ifndef NL_WIN_DISPLAYER_H
#define NL_WIN_DISPLAYER_H

#include "nel/misc/types_nl.h"

#ifdef NL_OS_WINDOWS

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>

#include "nel/misc/displayer.h"

namespace NLMISC {


/**
 * this displayer displays on a win32 windows. it can display only messages from the thread that created the displayer
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2001
 */
class CWinDisplayer : public NLMISC::IDisplayer
{
public:

	CWinDisplayer (const char *displayerName = "") : IDisplayer(displayerName), _Init (false), _ToolBarHeight (25), _InputEditHeight (25) { }

	/// Destructor
	virtual ~CWinDisplayer ();

	void	create (std::string WindowNameEx = "", uint w = 700, uint h = 300, sint hs = 10000);
	void	update ();
	void	clear ();

	uint	createLabel (const char *Name);
	void	setLabel (uint Label, const std::string &Name);

	void	setWindowParams (sint x, sint y, sint w, sint h);

protected:

	/** Sends the string to the logging server
	 * \warning If not connected, tries to connect to the logging server each call. It can slow down your program a lot.
	 */
	virtual void doDisplay (const NLMISC::TDisplayInfo &args, const char *message);

private:
	uint _Thread;
	bool _Init;

	HWND _HEdit, _HWnd, _HInputEdit;
	HFONT _HFont;
	HMODULE _HLibModule;

	sint _HistorySize;

	HWND _HClearBtn;
	HWND _HLabel1;
	sint _ToolBarHeight;
	sint _InputEditHeight;

	std::vector<HWND>	_HLabels;

	void resizeLabel ();

	friend LRESULT CALLBACK WndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

};

} // NLMISC

#endif // NL_OS_WINDOWS

#endif // NL_WIN_DISPLAYER_H

/* End of win_displayer.h */
