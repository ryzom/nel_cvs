/** \file win_displayer.cpp
 * <File description>
 *
 * $Id: win_displayer.cpp,v 1.1 2001/06/15 10:00:07 lecroart Exp $
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

#include "nel/misc/types_nl.h"

#ifdef NL_OS_WINDOWS

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <signal.h>

using namespace std;

#include <windowsx.h>

#include "nel/misc/common.h"
#include "nel/misc/path.h"
#include "nel/misc/win_displayer.h"

namespace NLMISC {

CWinDisplayer::~CWinDisplayer ()
{
	if (_Init)
	{
		DeleteObject (_HFont);
		DestroyWindow (_HWnd);
		DestroyWindow (_HEdit);
	}
}


LRESULT CALLBACK WndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) 
	{
	case WM_SIZE:
		{
			CWinDisplayer *cwd=(CWinDisplayer *)GetWindowLong (hWnd, GWL_USERDATA);
			if (cwd != NULL)
			{
				int w = lParam & 0xFFFF;
				int h = (lParam >> 16) & 0xFFFF;
				SetWindowPos (cwd->_HEdit, NULL, 0, 0, w, h, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE );
			}
			break;
		}
	case WM_DESTROY:	PostQuitMessage (0); break;
	case WM_CLOSE:		raise (SIGINT); break;
	default: return DefWindowProc (hWnd, message, wParam, lParam);
   }
   return 0;
}

void CWinDisplayer::create (string WindowNameEx, uint w, uint h, sint hs)
{
	_HistorySize = hs;

	WNDCLASS wc;
	memset (&wc,0,sizeof(wc));
	wc.style			= CS_HREDRAW | CS_VREDRAW ;//| CS_DBLCLKS;
	wc.lpfnWndProc		= (WNDPROC)WndProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hInstance		= GetModuleHandle(NULL);
	wc.hIcon			= NULL;
	wc.hCursor			= LoadCursor(NULL,IDC_ARROW);
	wc.hbrBackground	= WHITE_BRUSH;
	wc.lpszClassName	= "NLClass";
	wc.lpszMenuName		= NULL;
	if ( !RegisterClass(&wc) ) return;

	_HFont = CreateFont (15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEVICE_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH|FF_DONTCARE, "Arial");

	ULONG	WndFlags;
	RECT	WndRect;
	WndFlags=WS_OVERLAPPEDWINDOW+WS_CLIPCHILDREN+WS_CLIPSIBLINGS;
	WndRect.left=0;
	WndRect.top=0;
	WndRect.right=w;
	WndRect.bottom=h;
	AdjustWindowRect(&WndRect,WndFlags,FALSE);
	string wn = "Nel Service Console (compiled " __DATE__ " " __TIME__ ") ";
	if (!wn.empty())
	{
		wn += ": ";
		wn += WindowNameEx;
	}

	_HWnd = CreateWindow ("NLClass", wn.c_str(), WndFlags, CW_USEDEFAULT,CW_USEDEFAULT, WndRect.right,WndRect.bottom, NULL, NULL, GetModuleHandle(NULL), NULL);
	SetWindowLong (_HWnd, GWL_USERDATA, (LONG)this);
	// resize the window
	RECT rc;
	SetRect (&rc, 0, 0, w, h);
	AdjustWindowRectEx (&rc, GetWindowStyle (_HWnd), GetMenu (_HWnd) != NULL, GetWindowExStyle (_HWnd));
	SetWindowPos (_HWnd, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE );

	ShowWindow(_HWnd,SW_SHOW);

	_HEdit = CreateWindow("EDIT",NULL,WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL, 0, 0, w, h, _HWnd, (HMENU) NULL, (HINSTANCE) GetWindowLong(_HWnd, GWL_HINSTANCE), NULL);
	SendMessage (_HEdit, WM_SETFONT, (long) _HFont, TRUE);

	_Thread = getThreadId ();
	update ();
}

void CWinDisplayer::update ()
{
	MSG	msg;
	while (PeekMessage(&msg,NULL,0,0,PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

/** Sends the string to the logging server
 * \warning If not connected, tries to connect to the logging server each call. It can slow down your program a lot.
 */
void CWinDisplayer::doDisplay (const NLMISC::TDisplayInfo &args, const char *message)
{
	if (getThreadId() != _Thread) return;

	bool needSpace = false;
	stringstream ss;

	if (args.LogType != CLog::LOG_NO)
	{
		ss << logTypeToString(args.LogType);
		needSpace = true;
	}

	// Write thread identifier
	if ( args.ThreadId != 0 )
	{
		ss << setw(5) << args.ThreadId;
	}

	if (args.Filename != NULL)
	{
		if (needSpace) { ss << " "; needSpace = false; }
		ss << CFile::getFilename(args.Filename);
		needSpace = true;
	}

	if (args.Line != -1)
	{
		if (needSpace) { ss << " "; needSpace = false; }
		ss << args.Line;
		needSpace = true;
	}

	if (needSpace) { ss << " : "; needSpace = false; }

	uint nbl = 1;

	char *npos, *pos = const_cast<char *>(message);
	while (npos = strchr (pos, '\n'))
	{
		*npos = '\0';
		ss << pos;
		ss << "\r\n";
		*npos = '\n';
		pos = npos+1;
		nbl++;
	}
	ss << pos;

	sint nLine = SendMessage (_HEdit, EM_GETLINECOUNT, 0, 0) - 1;
	sint nIndex = SendMessage (_HEdit, EM_LINEINDEX, nLine, 0);
	SendMessage (_HEdit, EM_SETSEL, nIndex, nIndex);
	SendMessage (_HEdit, EM_LIMITTEXT, 0, 0);
	SendMessage (_HEdit, EM_REPLACESEL, TRUE, (long) ss.str().c_str());
	SendMessage (_HEdit, EM_SETREADONLY, TRUE, 0);

	if (_HistorySize > 0 && nLine > _HistorySize)
	{
		sint oldIndex1 = SendMessage (_HEdit, EM_LINEINDEX, 0, 0);
		sint oldIndex2 = SendMessage (_HEdit, EM_LINEINDEX, nbl, 0);
		SendMessage (_HEdit, EM_SETSEL, oldIndex1, oldIndex2);
		SendMessage (_HEdit, EM_REPLACESEL, TRUE, (long) "");
	}

	update ();
}


} // NLMISC

#endif // NL_OS_WINDOWS
