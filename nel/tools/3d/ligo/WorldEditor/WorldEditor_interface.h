/** \file WorldEditor_interface.h
 *
 * $Id: WorldEditor_interface.h,v 1.1 2001/10/24 14:35:53 besson Exp $
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

#ifndef WORLDEDITOR_INTERFACE
#define WORLDEDITOR_INTERFACE

#ifndef WORLDEDITOR_EXPORT
#define WORLDEDITOR_EXPORT __declspec( dllimport ) 
#endif // WORLDEDITOR_EXPORT

// Increment this version number each time you distribute a new version of the dll.
#define WORLDEDITOR_VERSION 1

class IWorldEditor
{
public:
	virtual ~IWorldEditor() {};

	// Init the UI
	virtual void initUI (HWND parent=NULL)=0;

	// Go
	virtual void go ()=0;

	// Release the UI
	virtual void releaseUI ()=0;

	virtual void*getMainFrame ()=0;

	// Get instance
	static WORLDEDITOR_EXPORT IWorldEditor* getInterface (int version=WORLDEDITOR_VERSION);

	// Release instance
	static WORLDEDITOR_EXPORT void releaseInterface (IWorldEditor* view);
};

#endif OBJECT_VIEWER_INTERFACE
