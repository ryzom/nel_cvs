/** \file report.cpp
 * This function display a custom message box to report something.
 * It is used in the debug system
 *
 * $Id: report.cpp,v 1.11.4.1 2004/09/03 09:16:09 corvazier Exp $
 */

/* Copyright, 2002 Nevrax Ltd.
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

#include "stdmisc.h"

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <nel/misc/common.h>

#include "nel/misc/report.h"

#ifdef NL_OS_WINDOWS

#include <windows.h>
#include <windowsx.h>
#include <winuser.h>

#ifdef min
#	undef min
#endif // min

#ifdef max
#	undef max
#endif // max

#endif // NL_OS_WINDOWS

using namespace std;

namespace NLMISC 
{

#ifdef NL_OS_WINDOWS
static HWND sendReport;
#endif

//old doesn't work on visual c++ 7.1 due to default parameter typedef bool (*TEmailFunction) (const std::string &smtpServer, const std::string &from, const std::string &to, const std::string &subject, const std::string &body, const std::string &attachedFile = "", bool onlyCheck = false);
typedef bool (*TEmailFunction) (const std::string &smtpServer, const std::string &from, const std::string &to, const std::string &subject, const std::string &body, const std::string &attachedFile, bool onlyCheck);

#define DELETE_OBJECT(a) if((a)!=NULL) { DeleteObject (a); a = NULL; }

static TEmailFunction EmailFunction = NULL;

void setReportEmailFunction (void *emailFunction)
{
	EmailFunction = (TEmailFunction)emailFunction;

#ifdef NL_OS_WINDOWS
	if (sendReport)
		EnableWindow(sendReport, FALSE);
#endif
}

#ifndef NL_OS_WINDOWS

// GNU/Linux, do nothing

void report ()
{
}

#else

// Windows specific version

static string Body;
static string Subject;
static string AttachedFile;

static HWND checkIgnore;
static HWND debug;
static HWND ignore;
static HWND quit;
static HWND dialog;

static bool NeedExit;
static TReportResult Result;
static bool IgnoreNextTime;

static bool DebugDefaultBehavior, QuitDefaultBehavior;

static LRESULT CALLBACK WndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//MSGFILTER *pmf;

	if (message == WM_COMMAND && HIWORD(wParam) == BN_CLICKED)
	{
		if ((HWND) lParam == checkIgnore)
		{
			IgnoreNextTime = !IgnoreNextTime;
		}
		else if ((HWND) lParam == debug)
		{
			NeedExit = true;
			Result = ReportDebug;
			if (DebugDefaultBehavior)
			{
				NLMISC_BREAKPOINT;
			}
		}
		else if ((HWND) lParam == ignore)
		{
			NeedExit = true;
			Result = ReportIgnore;
		}
		else if ((HWND) lParam == quit)
		{
			NeedExit = true;
			Result = ReportQuit;

			if (QuitDefaultBehavior)
			{
				exit(EXIT_SUCCESS);
			}
		}
		else if ((HWND) lParam == sendReport)
		{
			if (EmailFunction != NULL)
			{
				bool res = EmailFunction ("", "", "", Subject, Body, AttachedFile, false);
				if (res)
				{
					EnableWindow(sendReport, FALSE);
					MessageBox (dialog, "The email was successfully sent", "email", MB_OK);
					FILE *file = fopen ("report_sent", "wb");
					fclose (file);
				}
				else
				{
					MessageBox (dialog, "Failed to send the email", "email", MB_OK | MB_ICONERROR);
				}
			}
		}
	}
	else if (message == WM_CHAR)
	{
		if (wParam == 27)
		{
			// ESC -> ignore
			NeedExit = true;
			Result = ReportIgnore;
		}
	}

	return DefWindowProc (hWnd, message, wParam, lParam);
}

TReportResult report (const std::string &title, const std::string &header, const std::string &subject, const std::string &body, bool enableCheckIgnore, uint debugButton, bool ignoreButton, sint quitButton, bool sendReportButton, bool &ignoreNextTime, const string &attachedFile)
{
	// register the window
	static bool AlreadyRegister = false;
	if(!AlreadyRegister)
	{
		WNDCLASS wc;
		memset (&wc,0,sizeof(wc));
		wc.style			= CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc		= (WNDPROC)WndProc;
		wc.cbClsExtra		= 0;
		wc.cbWndExtra		= 0;
		wc.hInstance		= GetModuleHandle(NULL);
		wc.hIcon			= NULL;
		wc.hCursor			= LoadCursor(NULL,IDC_ARROW);
		wc.hbrBackground	= (HBRUSH)COLOR_WINDOW;
		wc.lpszClassName	= "NLReportWindow";
		wc.lpszMenuName		= NULL;
		if (!RegisterClass(&wc)) return ReportError;
		AlreadyRegister = true;
	}

	string formatedTitle = title.empty() ? "NeL report" : title;


	// create the window
	dialog = CreateWindow ("NLReportWindow", formatedTitle.c_str(), WS_DLGFRAME | WS_CAPTION /*| WS_THICKFRAME*/, CW_USEDEFAULT, CW_USEDEFAULT, 456, 375,  NULL, NULL, GetModuleHandle(NULL), NULL);

	// create the font
	HFONT font = CreateFont (-12, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");

	Subject = subject;
	AttachedFile = attachedFile;

	// create the edit control
	HWND edit = CreateWindow ("EDIT", NULL, WS_BORDER | WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | ES_READONLY | ES_LEFT | ES_MULTILINE, 7, 70, 429, 212, dialog, (HMENU) NULL, (HINSTANCE) GetWindowLong(dialog, GWL_HINSTANCE), NULL);
	SendMessage (edit, WM_SETFONT, (LONG) font, TRUE);

	// set the edit text limit to lot of :)
	SendMessage (edit, EM_LIMITTEXT, ~0U, 0);

	Body = addSlashR (body);

	// set the message in the edit text
	SendMessage (edit, WM_SETTEXT, (WPARAM)0, (LPARAM)Body.c_str());

	if (enableCheckIgnore)
	{
		// create the combo box control
		checkIgnore = CreateWindow ("BUTTON", "Don't display this report again", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | BS_CHECKBOX, 7, 290, 429, 18, dialog, (HMENU) NULL, (HINSTANCE) GetWindowLong(dialog, GWL_HINSTANCE), NULL);
		SendMessage (checkIgnore, WM_SETFONT, (LONG) font, TRUE);

		if(ignoreNextTime)
		{
			SendMessage (checkIgnore, BM_SETCHECK, BST_CHECKED, 0);
		}
	}

	// create the debug button control
	debug = CreateWindow ("BUTTON", "Debug", WS_CHILD | WS_VISIBLE, 7, 315, 75, 25, dialog, (HMENU) NULL, (HINSTANCE) GetWindowLong(dialog, GWL_HINSTANCE), NULL);
	SendMessage (debug, WM_SETFONT, (LONG) font, TRUE);

	if (debugButton == 0)
		EnableWindow(debug, FALSE);

	// create the ignore button control
	ignore = CreateWindow ("BUTTON", "Ignore", WS_CHILD | WS_VISIBLE, 75+7+7, 315, 75, 25, dialog, (HMENU) NULL, (HINSTANCE) GetWindowLong(dialog, GWL_HINSTANCE), NULL);
	SendMessage (ignore, WM_SETFONT, (LONG) font, TRUE);

	if (ignoreButton == 0)
		EnableWindow(ignore, FALSE);

	// create the quit button control
	quit = CreateWindow ("BUTTON", "Quit", WS_CHILD | WS_VISIBLE, 75+75+7+7+7, 315, 75, 25, dialog, (HMENU) NULL, (HINSTANCE) GetWindowLong(dialog, GWL_HINSTANCE), NULL);
	SendMessage (quit, WM_SETFONT, (LONG) font, TRUE);

	if (quitButton == 0)
		EnableWindow(quit, FALSE);

	// create the debug button control
	sendReport = CreateWindow ("BUTTON", "Send report", WS_CHILD | WS_VISIBLE, 429-80, 315, 90, 25, dialog, (HMENU) NULL, (HINSTANCE) GetWindowLong(dialog, GWL_HINSTANCE), NULL);
	SendMessage (sendReport, WM_SETFONT, (LONG) font, TRUE);

	string formatedHeader;
	if (header.empty())
	{
		formatedHeader = "This application stopped to display this report.";
	}
	else
	{
		formatedHeader = header;
	}

	// ace don't do that because it s slow to try to send a mail
	//bool canSendReport  = sendReportButton && EmailFunction != NULL && EmailFunction("", "", "", "", "", true);
	bool canSendReport  = sendReportButton && EmailFunction != NULL;

	if (canSendReport)
		formatedHeader += " Send report will only email the contents of the box below. Please, send it to help us (it could take few minutes to send the email, be patient).";
	else
		EnableWindow(sendReport, FALSE);


	// create the label control
	HWND label = CreateWindow ("STATIC", formatedHeader.c_str(), WS_CHILD | WS_VISIBLE /*| SS_WHITERECT*/, 7, 7, 429, 51, dialog, (HMENU) NULL, (HINSTANCE) GetWindowLong(dialog, GWL_HINSTANCE), NULL);
	SendMessage (label, WM_SETFONT, (LONG) font, TRUE);


	DebugDefaultBehavior = debugButton==1;
	QuitDefaultBehavior = quitButton==1;

	IgnoreNextTime = ignoreNextTime;

	// show until the cursor really show :)
	while (ShowCursor(TRUE) < 0) {};

	SetWindowPos (dialog, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);

	NeedExit = false;

	while(!NeedExit)
	{
		MSG	msg;
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		nlSleep (1);
	}

	// set the user result
	ignoreNextTime = IgnoreNextTime;

	ShowWindow(dialog, SW_HIDE);

	DELETE_OBJECT(sendReport)
	DELETE_OBJECT(quit)
	DELETE_OBJECT(ignore)
	DELETE_OBJECT(debug)
	DELETE_OBJECT(checkIgnore)
	DELETE_OBJECT(edit)
	DELETE_OBJECT(label)
	DELETE_OBJECT(dialog)

	return Result;
}

#endif


} // NLMISC
