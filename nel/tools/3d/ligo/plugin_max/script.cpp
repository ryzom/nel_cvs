/** \file script.cpp
 * MaxScript extension for ligo plugins
 *
 * $Id: script.cpp,v 1.1 2001/10/12 13:26:01 corvazier Exp $
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

#define EXPORT_GET_ALLOCATOR

#include "max_to_ligo.h"

// From ligo library
#include "../lib/zone_template.h"
#include "../lib/ligo_config.h"
#include "../lib/ligo_error.h"

// From nel patch lib
#include "../../plugin_max/nel_patch_lib/rpo.h"

// From nel misc
#include <nel/misc/file.h>
#include <nel/misc/o_xml.h>

using namespace NLMISC;
using namespace NLLIGO;

#include "MAXScrpt.h"
#include "3dmath.h"

// Various MAX and MXS includes
#include "Numbers.h"
#include "MAXclses.h"
#include "Streams.h"
#include "MSTime.h"
#include "MAXObj.h"
#include "Parser.h"
#include "max.h"
#include "stdmat.h"
#include "definsfn.h"

// ***************************************************************************

/// Definition of new MAXScript functions
def_visible_primitive( export_zone_template,				"NeLLigoExportZoneTemplate");
def_visible_primitive( get_error_zone_template,				"NeLLigoGetErrorZoneTemplate");
def_visible_primitive( get_snap ,							"NeLLigoGetSnap");

// ***************************************************************************

/// Zone template
CLigoError ScriptErrors;

// ***************************************************************************

/// Export a zone template
Value* export_zone_template_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (4)
	check_arg_count(export_zone, 4, count);

	// Check to see if the arguments match up to what we expect
	char *message = "NeLLigoExportZoneTemplate [Object] [Filename] [CheckOnly] [Error in dialog]";
	type_check(arg_list[0], MAXNode, message);
	type_check(arg_list[1], String, message);
	type_check(arg_list[2], Boolean, message);
	type_check(arg_list[3], Boolean, message);

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// The first arg
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// The second arg
	const char *fileName = arg_list[1]->to_string();

	// The third arg
	bool checkOnly = (arg_list[2]->to_bool() != FALSE);

	// The fourth arg
	bool errorInDialog = (arg_list[3]->to_bool() != FALSE);

	// Get a Object pointer
	ObjectState os=node->EvalWorldState(ip->GetTime()); 

	// Ok ?
	if (os.obj)
	{
		// Convert in 3ds NeL patch mesh
		RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), RYKOLPATCHOBJ_CLASS_ID);
		if (tri)
		{
			// Load the ligofile
			CLigoConfig config;
			if (!CMaxToLigo::loadLigoConfigFile (config, *MAXScript_interface, errorInDialog))
			{
				// Output an error
				CMaxToLigo::errorMessage ("Error: can't load the config file ligoscape.cfg", "NeL Ligo export zone template", 
					*MAXScript_interface, errorInDialog);
			}
			else
			{
				// The zone template
				CZoneTemplate zoneTemplate;

				// Build the zone template
				bool res = CMaxToLigo::buildZoneTemplate (node, tri->patch, zoneTemplate, config, ScriptErrors, ip->GetTime());

				// Success ?
				if (res)
				{
					// Save it ?
					if (!checkOnly)
					{
						// Catch exception
						try
						{
							// Open a stream file
							COFile outputfile;
							if (outputfile.open (fileName))
							{
								// Create an xml stream
								COXml outputXml;
								outputXml.init (&outputfile);

								// Serial the class
								zoneTemplate.serial (outputXml);
							}
						}
						catch (Exception &e)
						{
							// Error message
							char tmp[512];
							smprintf (tmp, 512, "Error while save the file %s : %s", fileName, e.what());
							CMaxToLigo::errorMessage (tmp, "NeL Ligo export zone template", *MAXScript_interface, errorInDialog);
						}
					}
				}

				// Return the result
				return res?&true_value:&false_value;
			}
		}
		else
		{
			// Output an error
			CMaxToLigo::errorMessage ("Error: can't convert the object in 3ds NeL patch mesh object", 
				"NeL Ligo export zone template", *MAXScript_interface, errorInDialog);
		}
	}

	// Return false
	return &false_value;
}

// ***************************************************************************
 
Value* get_error_zone_template_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (3)
	check_arg_count(export_zone, 3, count);

	// Check to see if the arguments match up to what we expect
	char *message = "NeLLigoExportZoneTemplate [Array error codes] [Array vertex id] [Array messages]";
	type_check(arg_list[0], Array, message);
	type_check(arg_list[1], Array, message);
	type_check(arg_list[2], Array, message);

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// The first arg
	Array *errorCodes = (Array *)arg_list[0];
	nlassert (is_array(errorCodes));

	// The second arg
	Array *vertexId = (Array *)arg_list[1];
	nlassert (is_array(vertexId));

	// The third arg
	Array *messages = (Array *)arg_list[2];
	nlassert (is_array(messages));

	// Num error
	uint numError = ScriptErrors.numVertexError ();

	// For each error
	for (uint i=0; i<numError; i++)
	{
		// Id and edge
		uint id, edge;
		uint error = (uint) ScriptErrors.getVertexError (i, id, edge);

		// Append error code
		errorCodes->append (Integer::intern (error+1));

		// Append vertex id
		vertexId->append (Integer::intern (id+1));

		// Append messages
		messages->append (new String("[LIGO DEBUG] Opened edge"));
	}

	// Return the main error message
	return Integer::intern ((int)ScriptErrors.MainError);
}
// ***************************************************************************
 
Value* get_snap_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (3)
	check_arg_count(export_zone, 0, count);

	// Load the ligofile
	CLigoConfig config;
	if (CMaxToLigo::loadLigoConfigFile (config, *MAXScript_interface, false))
	{
		return Float::intern ((float)config.Snap);
	}
	else
		return &undefined;
}

// ***************************************************************************

/// MAXScript Plugin Initialization

// ***************************************************************************

__declspec( dllexport ) void LibInit() { }

// ***************************************************************************

