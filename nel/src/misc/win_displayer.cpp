/** \file win_displayer.cpp
 * <File description>
 *
 * $Id: win_displayer.cpp,v 1.4 2001/08/23 14:32:16 lecroart Exp $
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

#include <windows.h>
#include <windowsx.h>
#include <winuser.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <richedit.h>
#include <commctrl.h>
#include <time.h>

#include "nel/misc/debug.h"
#include "nel/misc/common.h"
#include "nel/misc/path.h"
#include "nel/misc/command.h"
#include "nel/misc/win_displayer.h"

namespace NLMISC {


CWinDisplayer::~CWinDisplayer ()
{
	if (_Init)
	{
		DeleteObject (_HFont);
		DestroyWindow (_HWnd);
		DestroyWindow (_HEdit);
		FreeLibrary (_HLibModule);
	}
}


LRESULT CALLBACK WndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static MSGFILTER *pmf;
	switch (message) 
	{
	case WM_SIZE:
		{
			CWinDisplayer *cwd=(CWinDisplayer *)GetWindowLong (hWnd, GWL_USERDATA);
			if (cwd != NULL)
			{
				int w = lParam & 0xFFFF;
				int h = (lParam >> 16) & 0xFFFF;
				SetWindowPos (cwd->_HEdit, NULL, 0, cwd->_ToolBarHeight, w, h-cwd->_ToolBarHeight-cwd->_InputEditHeight, SWP_NOZORDER | SWP_NOACTIVATE );
				SetWindowPos (cwd->_HInputEdit, NULL, 0, h-cwd->_InputEditHeight, w, cwd->_InputEditHeight, SWP_NOZORDER | SWP_NOACTIVATE );
				cwd->resizeLabel ();
			}
		}
		break;
	case WM_DESTROY:	PostQuitMessage (0); break;
	case WM_CLOSE:		raise (SIGINT); break;
	case WM_COMMAND:
		{
			if (HIWORD(wParam) == BN_CLICKED)
			{
				// clear button clicked
				CWinDisplayer *cwd=(CWinDisplayer *)GetWindowLong (hWnd, GWL_USERDATA);
				cwd->clear ();
			}
		}
		break;
	case WM_NOTIFY:
		switch (((NMHDR*)lParam)->code)
		{
		case EN_MSGFILTER:
			pmf = (MSGFILTER *)lParam;
			if (pmf->msg == WM_CHAR)
			{
				if (pmf->wParam == VK_RETURN)
				{
					char TextSend[20000];
					CWinDisplayer *cwd=(CWinDisplayer *)GetWindowLong (hWnd, GWL_USERDATA);
					TextSend[0] = TextSend[1] = (char)0xFF;
					SendMessage (cwd->_HInputEdit, WM_GETTEXT, (WPARAM)20000-1, (LPARAM)TextSend);
					SendMessage (cwd->_HInputEdit, WM_SETTEXT, (WPARAM)0, (LPARAM)"");
					char *pos1 = TextSend, *pos2 = TextSend;
					string str;
					while (*pos2 != '\0')
					{
						str = "";
						while (*pos2 != '\0' && *pos2 != 13)
						{
							if (*pos2 != 10)
							{
								str += *pos2;
							}
							*pos2++;
						}
						if (!str.empty())
						{
							ICommand::execute (str, *InfoLog);
						}
						*pos2++;
					}
				}
			}
		}
   }
   return DefWindowProc (hWnd, message, wParam, lParam);
}

void CWinDisplayer::resizeLabel ()
{
	RECT Rect;
	GetClientRect (_HWnd, &Rect);
	sint delta = Rect.right / (_HLabels.size () + 1);

	SetWindowPos (_HClearBtn, NULL, 0, 0, delta, _ToolBarHeight, SWP_NOZORDER | SWP_NOACTIVATE );
	
	for (uint i = 0; i< _HLabels.size (); i++)
	{
		SetWindowPos (_HLabels[i], NULL, (i+1)*delta, 0, delta, _ToolBarHeight, SWP_NOZORDER | SWP_NOACTIVATE );
	}
}

uint CWinDisplayer::createLabel (const char *Name)
{
	HWND label = CreateWindow ("STATIC", Name, WS_CHILD | WS_VISIBLE | SS_SIMPLE, 0, 0, 0, 0, _HWnd, (HMENU) NULL, (HINSTANCE) GetWindowLong(_HWnd, GWL_HINSTANCE), NULL);
	SendMessage (label, WM_SETFONT, (LONG) _HFont, TRUE);
	_HLabels.push_back (label);
	resizeLabel();
	return (uint)label;
}

void CWinDisplayer::setLabel (uint label, const char *Name)
{
	SendMessage ((HWND)label, WM_SETTEXT, 0, (LONG) Name);
}

void CWinDisplayer::setWindowParams (sint x, sint y, sint w, sint h)
{
	LONG flag = SWP_NOZORDER | SWP_NOACTIVATE;
	
	if (x == -1 && y == -1) flag |= SWP_NOMOVE;
	if (w == -1 && h == -1) flag |= SWP_NOSIZE;

	SetWindowPos (_HWnd, NULL, x, y, w, h, flag);
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
	wc.hbrBackground	= (HBRUSH)COLOR_WINDOW;
	wc.lpszClassName	= "NLClass";
	wc.lpszMenuName		= NULL;
	if ( !RegisterClass(&wc) ) return;

	ULONG	WndFlags;
	RECT	WndRect;
	WndFlags = WS_OVERLAPPEDWINDOW /*| WS_CLIPCHILDREN | WS_CLIPSIBLINGS*/;
	WndRect.left = 0;
	WndRect.top = 0;
	WndRect.right = w;
	WndRect.bottom = h;
	AdjustWindowRect(&WndRect,WndFlags,FALSE);
	string wn = "Nel Service Console (compiled " __DATE__ " " __TIME__ ") ";
	if (!wn.empty())
	{
		wn += ": ";
		wn += WindowNameEx;
	}

	// create the window
	_HWnd = CreateWindow ("NLClass", wn.c_str(), WndFlags, CW_USEDEFAULT,CW_USEDEFAULT, WndRect.right,WndRect.bottom, NULL, NULL, GetModuleHandle(NULL), NULL);
	SetWindowLong (_HWnd, GWL_USERDATA, (LONG)this);
	
	// resize the window
	RECT rc;
	SetRect (&rc, 0, 0, w, h);
	AdjustWindowRectEx (&rc, GetWindowStyle (_HWnd), GetMenu (_HWnd) != NULL, GetWindowExStyle (_HWnd));
	SetWindowPos (_HWnd, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE );

	ShowWindow(_HWnd,SW_SHOW);

	// create the font
	_HFont = CreateFont (-13, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");

	// create the edit control
	_HEdit = CreateWindow ("EDIT", NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL, 0, _ToolBarHeight, w, h-_ToolBarHeight-_InputEditHeight, _HWnd, (HMENU) NULL, (HINSTANCE) GetWindowLong(_HWnd, GWL_HINSTANCE), NULL);
	SendMessage (_HEdit, WM_SETFONT, (LONG) _HFont, TRUE);

	// create the clear buttton control
	_HClearBtn = CreateWindow ("BUTTON", "Clear", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 0, 0, 100, _ToolBarHeight, _HWnd, (HMENU) NULL, (HINSTANCE) GetWindowLong(_HWnd, GWL_HINSTANCE), NULL);
	SendMessage (_HClearBtn, WM_SETFONT, (LONG) _HFont, TRUE);
	
	// set the edit to read only
	SendMessage (_HEdit, EM_SETREADONLY, TRUE, 0);

	// set the edit text limit to lot of :)
	SendMessage (_HEdit, EM_LIMITTEXT, -1, 0);

	// create the input edit control
	_HLibModule = LoadLibrary("RICHED20.DLL");
	if (_HLibModule == NULL)
	{
		nlerror ("RichEdit 2.0 library not found!");
	}

	_HInputEdit = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW, RICHEDIT_CLASS, "", WS_CHILD | WS_VISIBLE
		| ES_MULTILINE | ES_WANTRETURN | ES_NOHIDESEL | ES_AUTOHSCROLL, 0, h-_InputEditHeight, w, _InputEditHeight,
		_HWnd, NULL, (HINSTANCE) GetWindowLong(_HWnd, GWL_HINSTANCE), NULL);
	SendMessage (_HInputEdit, WM_SETFONT, (LONG) _HFont, TRUE);

	DWORD dwEvent = SendMessage(_HInputEdit, EM_GETEVENTMASK, (WPARAM)0, (LPARAM)0);
	dwEvent |= ENM_MOUSEEVENTS | ENM_KEYEVENTS | ENM_CHANGE;
	SendMessage(_HInputEdit, EM_SETEVENTMASK, (WPARAM)0, (LPARAM)dwEvent);

	SetFocus(_HInputEdit);
	
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

void CWinDisplayer::clear ()
{
	// get number of line
	sint nLine = SendMessage (_HEdit, EM_GETLINECOUNT, 0, 0) - 1;

	// get size of the last line
	sint nIndex = SendMessage (_HEdit, EM_LINEINDEX, nLine, 0);

	// select all the text
	SendMessage (_HEdit, EM_SETSEL, 0, nIndex);

	// clear all the text
	SendMessage (_HEdit, EM_REPLACESEL, TRUE, (LONG) "");
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

	// store old selection
	DWORD startSel, endSel;
	SendMessage (_HEdit, EM_GETSEL, (LONG)&startSel, (LONG)&endSel);

	// get number of line
	sint nLine = SendMessage (_HEdit, EM_GETLINECOUNT, 0, 0) - 1;

	// clear old line if history size is too big
	if (_HistorySize > 0 && nLine > _HistorySize)
	{
		sint oldIndex1 = SendMessage (_HEdit, EM_LINEINDEX, 0, 0);
		sint oldIndex2 = SendMessage (_HEdit, EM_LINEINDEX, nbl, 0);
		SendMessage (_HEdit, EM_SETSEL, oldIndex1, oldIndex2);
		SendMessage (_HEdit, EM_REPLACESEL, TRUE, (LONG) "");
	}

	// get number of line
	nLine = SendMessage (_HEdit, EM_GETLINECOUNT, 0, 0) - 1;
	
	// get size of the last line
	sint nIndex = SendMessage (_HEdit, EM_LINEINDEX, nLine, 0);

	// select the end of the last line
	SendMessage (_HEdit, EM_SETSEL, nIndex, nIndex);

	// add the string to the edit control
	SendMessage (_HEdit, EM_REPLACESEL, TRUE, (LONG) ss.str().c_str());

	// restore old selection
	SendMessage (_HEdit, EM_SETSEL, startSel, endSel);

	update ();
}

} // NLMISC

#endif // NL_OS_WINDOWS
