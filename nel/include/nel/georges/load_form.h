/** \file load_form.h
 * quick load of values from georges sheet (using a fast load with compacted file)
 *
 * $Id: load_form.h,v 1.11 2002/09/04 12:14:25 coutelas Exp $
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

#ifndef NL_LOAD_FORM_H
#define NL_LOAD_FORM_H

#include "nel/misc/types_nl.h"

#include <map>
#include <string>
#include <vector>

#include "nel/misc/path.h"
#include "nel/misc/file.h"

#include "nel/georges/u_form_loader.h"
#include "nel/misc/sheet_id.h"

/** This function is used to load values from georges sheet in a quick way.
 * The first time it loads the sheet and parse it with the readGeorges function
 * provided by the user to read the value he wants. It'll generate a packed file
 * that contains this values (using serialCont). The next launch, the function will
 * only load the packed file and if some sheet have changed, it'll automatically regenerate
 * the packed file.
 *
 * To use the loadForm(), you first have to create a class that will contains values for one sheet.
 * This class must also implements 2 functions (readGeorges() and serial()) and 1 static functions (getVersion())
 *
 * Extension file name for the packedFilename must be ".packed_sheets"
 *
 * Classical use (copy/paste this in your code):

	class CContainerEntry
	{
	public:
		CContainerEntry () : WalkSpeed(1.3f), RunSpeed(6.0f) {}

		float WalkSpeed, RunSpeed;

		// load the values using the george sheet
		void readGeorges (const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const NLMISC::CSheetId &sheetId)
		{
			// the form was found so read the true values from George
			form->getRootNode ().getValueByName (WalkSpeed, "Basics.MovementSpeeds.WalkSpeed");
			form->getRootNode ().getValueByName (RunSpeed, "Basics.MovementSpeeds.RunSpeed");
		}

		// load/save the values using the serial system
		void serial (NLMISC::IStream &s)
		{
			s.serial (WalkSpeed, RunSpeed);
		}

		// return the version of this class, increments this value when the content hof this class changed
		static uint getVersion () { return 1; }
	};

	// this structure is fill by the loadForm() function and will contain all you need
	std::map<NLMISC::CSheetId,CContainerEntry> Container;

	void init ()
	{
		// load the values using the george sheet or packed file and fill the container
		loadForm(".creature", "test.packed_sheets", Container);
	}

 * Now you can access the Container (using the CSheedId) to know the WalkSpeed and RunSpeed of all creatures.
 *
 */

/** This function is used to load values from georges sheet in a quick way.
 * \param sheetFilter a string to filter the sheet (ie: ".item")
 * \param packedFilename the name of the file that this function will generate (extension must be "packed_sheets")
 * \param container the map that will be filled by this function
 */
template <class T>
void loadForm (const std::string &sheetFilter, const std::string &packedFilename, std::map<NLMISC::CSheetId, T> &container)
{
	std::vector<std::string> vs;
	vs.push_back(sheetFilter);
	loadForm(vs, packedFilename, container);
}

/** This function is used to load values from georges sheet in a quick way.
 * \param sheetFilter a vector of string to filter the sheet in the case you need more than one filter
 * \param packedFilename the name of the file that this function will generate (extension must be "packed_sheets")
 * \param container the map that will be filled by this function
 */
template <class T>
void loadForm (const std::vector<std::string> &sheetFilters, const std::string &packedFilename, std::map<NLMISC::CSheetId, T> &container)
{
	// check the extension (i know that file like "foo.packed_sheetsbar" will be accepted but this check is enough...)
	nlassert (packedFilename.find (".packed_sheets") != std::string::npos);

	// make sure the CSheetId singleton has been properly initialised
	NLMISC::CSheetId::init();

	// build a vector of the sheetFilters sheet ids (".item")
	std::vector<NLMISC::CSheetId> sheetIds;
	std::vector<std::string> filenames;
	for (uint i = 0; i < sheetFilters.size(); i++)
		NLMISC::CSheetId::buildIdVector(sheetIds, filenames, sheetFilters[i]);

	// if there s no file, nothing to do
	if (sheetIds.empty())
		return;

	std::string packedFilenamePath = NLMISC::CPath::lookup(packedFilename, false);
	if (packedFilenamePath.empty())
	{
		packedFilenamePath = packedFilename;
	}

	bool olde, newe;
	NLMISC::CIFile::getVersionException(olde, newe);
	// load the packed sheet if exists
	try
	{
		NLMISC::CIFile ifile;
		ifile.setCacheFileOnOpen(true);
		ifile.open (packedFilenamePath);
		// an exception will be launch if the file is not the good version or if the file is not found

		nlinfo ("loadForm(): Loading packed file '%s'", packedFilename.c_str());
		
		uint32 nbEntries;
		ifile.serial (nbEntries);
		ifile.setVersionException (true, true);
		uint ver = T::getVersion ();
		ifile.serialVersion(ver);
		ifile.serialCont (container);
		ifile.close ();
	}
	catch (NLMISC::Exception &e)
	{
		nlinfo ("loadForm(): Exception during reading the packed file, I'll reconstruct it (%s)", e.what());
		// clear the container because it can contains partially loaded sheet so we must clean it before continue
		container.clear ();
	}
	NLMISC::CIFile::setVersionException(olde, newe);

	// set up the current sheet in container to remove sheet that are in the container and not in the directory anymore
	std::map<NLMISC::CSheetId, bool> sheetToRemove;
	for (typename std::map<NLMISC::CSheetId, T>::iterator it = container.begin(); it != container.end(); it++)
	{
		sheetToRemove.insert (make_pair((*it).first, true));
	}

	// check if we need to create a new .pitems or just read it
	uint32 packedFiledate = NLMISC::CFile::getFileModificationDate(packedFilenamePath);

	bool containerChanged = false;

	NLGEORGES::UFormLoader *formLoader = NULL;

	std::vector<uint> NeededToRecompute;

	for (uint k = 0; k < filenames.size(); k++)
	{
		std::string p = NLMISC::CPath::lookup (filenames[k], false, false);
		if (p.empty()) continue;
		uint32 d = NLMISC::CFile::getFileModificationDate(p);

		// no need to remove this sheet
		sheetToRemove[sheetIds[k]] = false;

		if( d > packedFiledate || container.find (sheetIds[k]) == container.end())
		{
			NeededToRecompute.push_back(k);
		}
	}

	nlinfo ("%d sheets checked, %d need to be recomputed", filenames.size(), NeededToRecompute.size());

	NLMISC::TTime last = NLMISC::CTime::getLocalTime ();
	NLMISC::TTime start = NLMISC::CTime::getLocalTime ();

	NLMISC::CSmartPtr<NLGEORGES::UForm> form;

	for (uint j = 0; j < NeededToRecompute.size(); j++)
	{
		if(NLMISC::CTime::getLocalTime () > last + 5000)
		{
			last = NLMISC::CTime::getLocalTime ();
			if(j>0)
				nlinfo ("%.0f%% completed (%d/%d), %d seconds remaining", (float)j*100.0/NeededToRecompute.size(),j,NeededToRecompute.size(), (NeededToRecompute.size()-j)*(last-start)/j/1000);
		}

		// create the georges loader if necessary
		if (formLoader == NULL)
		{
			WarningLog->addNegativeFilter("CFormLoader: Can't open the form file");
			formLoader = NLGEORGES::UFormLoader::createLoader ();
		}

		// Load the form with given sheet id
		form = formLoader->loadForm (sheetIds[NeededToRecompute[j]].toString().c_str ());
		if (form)
		{
/*			if (packedFiledate > 0)
			{
				if (d > packedFiledate)
					nlinfo ("loadForm(): the sheet '%s' is newer than the packed one, I reload it", p.c_str());
				else
					nlinfo ("loadForm(): the sheet '%s' is not in the packed sheets, I load it", p.c_str());
			}*/
			
			// add the new creature, it could be already loaded by the packed sheets but will be overwrite with the new one
			typename std::pair<std::map<NLMISC::CSheetId, T>::iterator, bool> res = container.insert(std::make_pair(sheetIds[NeededToRecompute[j]],T()));

			(*res.first).second.readGeorges (form, sheetIds[NeededToRecompute[j]]);
			containerChanged = true;
		}
	}

	nlinfo ("%d seconds to recompute %d sheets", (uint32)(NLMISC::CTime::getLocalTime()-start)/1000, NeededToRecompute.size());

	// free the georges loader if necessary
	if (formLoader != NULL)
	{
		NLGEORGES::UFormLoader::releaseLoader (formLoader);
		WarningLog->removeFilter ("CFormLoader: Can't open the form file");
	}

	// we have now to remove sheet that are in the container and not exist anymore in the sheet directories
	for (std::map<NLMISC::CSheetId, bool>::iterator it2 = sheetToRemove.begin(); it2 != sheetToRemove.end(); it2++)
	{
		if((*it2).second)
		{
			nlinfo ("the sheet '%s' is not in the directory, remove it from container", (*it2).first.toString().c_str());
			container.erase((*it2).first);
			containerChanged = true;
		}
	}

	// now, save the new container in the packedfile
	try
	{
		if(containerChanged)
		{
			NLMISC::COFile ofile;
			ofile.open(packedFilenamePath);
			uint ver = T::getVersion ();
			uint32 nbEntries = sheetIds.size();
			ofile.serial (nbEntries);
			ofile.serialVersion(ver);
			ofile.serialCont(container);
			ofile.close ();
		}
	}
	catch (NLMISC::Exception &e)
	{
		nlinfo ("loadForm(): Exception during saving the packed file, it will be recreated next launch (%s)", e.what());
	}

	// housekeeping
	sheetIds.clear ();
	filenames.clear ();
}

#endif // NL_LOAD_FORM_H

/* End of load_form.h */
