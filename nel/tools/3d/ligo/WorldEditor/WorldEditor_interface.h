/** \file WorldEditor_interface.h
 *
 * $Id: WorldEditor_interface.h,v 1.4 2002/01/16 15:22:32 besson Exp $
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

#include <vector>
#include <string>

// ***************************************************************************
// IMasterCB
// ***************************************************************************
// This class is the master callbacks. This is a group of events when the 
// worldeditor makes special actions. The application that use the world editor interface
// can have a class overrided from the IMasterCB to receive some notifications of the WorldEditor.
// -- for the moment its used by the Master tool to link Worldeditor and Georges --
class IMasterCB
{
public:
	// fill the vector passed in argument with the new list of primZone when the list changes
	virtual void setAllPrimZoneNames (std::vector<std::string> &primZoneList) = 0;
};

// ***************************************************************************
// IWorldEditor
// ***************************************************************************
class IWorldEditor
{
public:
	virtual ~IWorldEditor() {};

	// Init the UI
	virtual void initUI (HWND parent=NULL)=0;

	// Init the UI Light version
	virtual void initUILight (int x, int y, int cx, int cy)=0;

	// Go
	virtual void go ()=0;

	// Release the UI
	virtual void releaseUI ()=0;

	// Get the main frame
	virtual void*getMainFrame ()=0;

	// Set the root path directory
	virtual void setRootDir (const char *sPathName)=0;

	// Set the root path directory
	virtual void setMasterCB (IMasterCB *pMCB)=0;

	// Create the default files given the base name (add extension)
	virtual void createDefaultFiles (const char *fileBaseName)=0;

	// Create a default .prim file
	virtual void createEmptyPrimFile (const char *fullName)=0;

	// Load a specific file and make it by default
	virtual void loadFile(const char *fileName)=0;

	// Save all files opened
	virtual void saveOpenedFiles()=0;

	// Get instance
	static WORLDEDITOR_EXPORT IWorldEditor* getInterface (int version=WORLDEDITOR_VERSION);

	// Release instance
	static WORLDEDITOR_EXPORT void releaseInterface (IWorldEditor* view);
};

// To export the names in a good format that can be human readable and not with the heavy style
// of the MFC we have to do it in 'old-school' mode
extern "C" 
{
	WORLDEDITOR_EXPORT IWorldEditor* IWorldEditorGetInterface (int version=WORLDEDITOR_VERSION);
	WORLDEDITOR_EXPORT void IWorldEditorReleaseInterface (IWorldEditor* pWE);
} 

#endif OBJECT_VIEWER_INTERFACE
