/** \file load_form.h
 * quick load of values from georges sheet (using a fast load with compacted file)
 *
 * $Id: load_form.h,v 1.1 2002/06/03 10:00:15 lecroart Exp $
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
#include "nel/georges/sheet_id.h"

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
		void readGeorges (const NLMISC::CSmartPtr<NLGEORGES::UForm> &form, const CSheetId &sheetId)
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
	std::map<CSheetId,CContainerEntry> Container;

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
void loadForm (const std::string &sheetFilter, const std::string &packedFilename, std::map<CSheetId, T> &container)
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
void loadForm (const std::vector<std::string> &sheetFilters, const std::string &packedFilename, std::map<CSheetId, T> &container)
{
	// check the extension (i know that file like "foo.packed_sheetsbar" will be accepted but this check is enough...)
	nlassert (packedFilename.find (".packed_sheets") != string::npos);

	// make sure the CSheetId singleton has been properly initialised
	CSheetId::init();

	// build a vector of the sheetFilters sheet ids (".item")
	std::vector<CSheetId> sheetIds;
	std::vector<std::string> filenames;
	for (uint i = 0; i < sheetFilters.size(); i++)
		CSheetId::buildIdVector(sheetIds, filenames, sheetFilters[i]);

	// check if we need to create a new .pitems or just read it
	uint32 packedFiledate = NLMISC::CFile::getFileModificationDate(packedFilename);

	bool needRebuild = false;
	for (uint j = 0; j < filenames.size(); j++)
	{
		std::string f = filenames[j];
		std::string p = NLMISC::CPath::lookup (f, false);
		if (p.empty()) continue;
		uint32 d = NLMISC::CFile::getFileModificationDate(p);
		//nlinfo ("%s %s %d %d",f.c_str(), p.c_str(), d, packedFiledate);
		if( d > packedFiledate)
		{
			needRebuild = true;
			break;
		}
	}

	if (needRebuild)
	{
NeedRebuild:
		nlinfo ("loadForm(): Packed file '%s' is out of date, reconstruct it", packedFilename.c_str());
		// setup a form loader to read our forms with
		NLGEORGES::UFormLoader *formLoader = NLGEORGES::UFormLoader::createLoader ();

		// iterate through the vector of sheet ids
		for(std::vector<CSheetId>::iterator it = sheetIds.begin(); it != sheetIds.end(); ++it)
		{
			// Load the form with given sheet id
			NLMISC::CSmartPtr<NLGEORGES::UForm> form = formLoader->loadForm ((*it).toString().c_str ());
			if (form)
			{
				// add the new creature
				std::pair<std::map<CSheetId, T>::iterator, bool> res = container.insert(std::make_pair(*it,T()));

				if (!res.second)
				{
					nlwarning ("There's 2 sheet with the same id (sheetId=%s), overwrite the old one", (*it).toString().c_str());
				}

				(*res.first).second.readGeorges (form, *it);
			}
		}
		NLGEORGES::UFormLoader::releaseLoader (formLoader);

		// now, save the new container in the packedfile
		std::string path = NLMISC::CPath::lookup (packedFilename, false);
		if (path.empty())
		{
			path = packedFilename;
		}
		NLMISC::COFile ofile;
		ofile.open(path);
		uint ver = T::getVersion ();
		ofile.serialVersion(ver);
		ofile.serialCont(container);
		ofile.close ();
	}
	else
	{
		nlinfo ("loadForm(): Loading packed file '%s'", packedFilename.c_str());

		// don't need to rebuild, only read the packedFilename
		try
		{
			NLMISC::CIFile ifile;
			ifile.setCacheFileOnOpen(true);
			ifile.open (NLMISC::CPath::lookup(packedFilename));
			// an exception will be launch if the file is not the good version
			ifile.setVersionException (true, true);
			uint ver = T::getVersion ();
			ifile.serialVersion(ver);
			ifile.serialCont (container);
			ifile.close ();
		}
		catch (NLMISC::EStream &e)
		{
			nlinfo ("loadForm(): Exception during reading the packed file, reconstruct it (%s)", e.what());
			goto NeedRebuild;
		}
	}

	// housekeeping
	sheetIds.clear ();
	filenames.clear ();
}

#endif // NL_LOAD_FORM_H

/* End of load_form.h */
