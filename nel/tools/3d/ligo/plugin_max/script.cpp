/** \file script.cpp
 * MaxScript extension for ligo plugins
 *
 * $Id: script.cpp,v 1.2 2001/10/29 09:35:15 corvazier Exp $
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
#include "../lib/material.h"
#include "../lib/transition.h"

// From nel patch lib
#include "../../plugin_max/nel_patch_lib/rpo.h"

// From nel 3d
#include <3d/zone.h>

// From nel misc
#include <nel/misc/file.h>
#include <nel/misc/o_xml.h>
#include <nel/misc/i_xml.h>

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

// Visual
#include <direct.h>

using namespace std;
using namespace NLMISC;
using namespace NLLIGO;

// ***************************************************************************

/// Definition of new MAXScript functions
def_visible_primitive( export_material,						"NeLLigoExportMaterial");
def_visible_primitive( export_transition,					"NeLLigoExportTransition");
def_visible_primitive( get_error_zone_template,				"NeLLigoGetErrorZoneTemplate");
def_visible_primitive( get_snap ,							"NeLLigoGetSnap");
def_visible_primitive( get_cell_size ,						"NeLLigoGetCellSize");
def_visible_primitive( check_zone_with_template ,			"NeLLigoCheckZoneWithTemplate");
def_visible_primitive( get_error_string,					"NeLLigoGetErrorString");
def_visible_primitive( set_directory,						"NeLLigoSetDirectory");

// ***************************************************************************

/// Zone template
CLigoError ScriptErrors[10];

// ***************************************************************************

/// Export a material
Value* export_material_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (4)
	check_arg_count(export_material, 4, count);

	// Check to see if the arguments match up to what we expect
	char *message = "NeLLigoExportMaterial [Object] [Filename] [CheckOnly] [Error in dialog]";
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
				CMaxToLigo::errorMessage ("Error: can't load the config file ligoscape.cfg", "NeL Ligo export material", 
					*MAXScript_interface, errorInDialog);
			}
			else
			{
				// The zone template
				CZoneTemplate zoneTemplate;

				// Build the zone template
				bool res = CMaxToLigo::buildZoneTemplate (node, tri->patch, zoneTemplate, config, ScriptErrors[0], ip->GetTime());

				// Success ?
				if (res)
				{
					// Build a zone
					NL3D::CZone zone;
					if (tri->rpatch->exportZone (node, &tri->patch, zone, 0))
					{
						// Build a material
						CMaterial material;

						// No error ?
						if (res = material.build (zoneTemplate, config, ScriptErrors[0]))
						{
							// Save it ?
							if (!checkOnly)
							{

								// Make a name for the zone
								char drive[512];
								char dir[512];
								char name[512];
								char path[512];
								_splitpath (fileName, drive, dir, name, NULL);
								_makepath (path, drive, dir, name, ".zone");

								// Ok ?
								bool ok = true;

								// Catch exception
								try
								{
									// Open a stream file
									COFile outputfile;
									if (outputfile.open (fileName))
									{
										// Create an xml stream
										COXml outputXml;
										nlverify (outputXml.init (&outputfile));

										// Serial the class
										material.serial (outputXml);

										// Another the stream
										COFile outputfile2;

										// Open another file
										if (outputfile2.open (path))
										{
											// Serial the zone
											zone.serial (outputfile2);
										}
										else
										{
											// Error message
											char tmp[512];
											smprintf (tmp, 512, "Can't open the file %s for writing.", path);
											CMaxToLigo::errorMessage (tmp, "NeL Ligo export material", *MAXScript_interface, errorInDialog);
											ok = false;
										}
									}
									else
									{
										// Error message
										char tmp[512];
										smprintf (tmp, 512, "Can't open the file %s for writing.", fileName);
										CMaxToLigo::errorMessage (tmp, "NeL Ligo export material", *MAXScript_interface, errorInDialog);
										ok = false;
									}
								}
								catch (Exception &e)
								{
									// Error message
									char tmp[512];
									smprintf (tmp, 512, "Error while save the file %s : %s", fileName, e.what());
									CMaxToLigo::errorMessage (tmp, "NeL Ligo export material", *MAXScript_interface, errorInDialog);
									ok = false;
								}

								// Remove the files
								if (!ok)
								{
									remove (fileName);
									remove (path);
								}
							}
						}
					}
					else
					{
						// Error, zone can't be exported
						CMaxToLigo::errorMessage ("Error: can't export the Nel zone, check bind errors.", "NeL Ligo export material", 
							*MAXScript_interface, errorInDialog);
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
				"NeL Ligo export material", *MAXScript_interface, errorInDialog);
		}
	}

	// Return false
	return &false_value;
}

// ***************************************************************************

/// Export a transition
Value* export_transition_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (6)
	check_arg_count(export_transition, 6, count);

	// Check to see if the arguments match up to what we expect
	char *message = "NeLLigoExportTransition [Object array (count=9)] [Output filename] [First material filename] [Second material filename] [CheckOnly] [Error in dialog]";
	type_check(arg_list[0], Array, message);
	type_check(arg_list[1], String, message);
	type_check(arg_list[2], String, message);
	type_check(arg_list[3], String, message);
	type_check(arg_list[4], Boolean, message);
	type_check(arg_list[5], Boolean, message);

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// The first arg
	Array *nodes = (Array*)arg_list[0];
	nlassert (is_array(nodes));

	// The second arg
	const char *fileName = arg_list[1]->to_string();

	// The second arg
	string matFilename[2];
	matFilename[0] = arg_list[2]->to_string();
	matFilename[1] = arg_list[3]->to_string();

	// The third arg
	bool checkOnly = (arg_list[4]->to_bool() != FALSE);

	// The fourth arg
	bool errorInDialog = (arg_list[5]->to_bool() != FALSE);

	// Ok ?
	bool ok = true;

	// All zone are present ?
	bool allPresent = true;

	// Clear error message
	for (uint error=0; error<10; error++)
		ScriptErrors[error].clear ();

	// Load the ligofile
	CLigoConfig config;
	if (!CMaxToLigo::loadLigoConfigFile (config, *MAXScript_interface, errorInDialog))
	{
		// Output an error
		CMaxToLigo::errorMessage ("Error: can't load the config file ligoscape.cfg", "NeL Ligo export transition", 
			*MAXScript_interface, errorInDialog);
	}
	else
	{
		// CTransition::TransitionZoneCount elements in the array ?
		if (nodes->size == CTransition::TransitionZoneCount)
		{
			// Build a node array
			std::vector<CZoneTemplate>			arrayTemplate (CTransition::TransitionZoneCount);
			std::vector<const CZoneTemplate*>	arrayTemplatePtr (CTransition::TransitionZoneCount, NULL);
			std::vector<RPO*>					arrayTri (CTransition::TransitionZoneCount, NULL);
			std::vector<INode*>					arrayNode (CTransition::TransitionZoneCount, NULL);

			for (uint i=0; i<(uint)nodes->size; i++)
			{
				// Get a node zone
				if (nodes->get (i+1) != &undefined)
				{
					arrayNode[i] = nodes->get (i+1)->to_node();
					// Node builded ?
					bool builded = false;
					
					// Get a Object pointer
					ObjectState os=arrayNode[i]->EvalWorldState(ip->GetTime()); 

					// Ok ?
					if (os.obj)
					{
						// Convert in 3ds NeL patch mesh
						arrayTri[i] = (RPO *) os.obj->ConvertToType(ip->GetTime(), RYKOLPATCHOBJ_CLASS_ID);
						if (arrayTri[i])
						{
							// Build a zone template
							CZoneTemplate zoneTemplate;

							// Build the zone template
							if (CMaxToLigo::buildZoneTemplate (arrayNode[i], arrayTri[i]->patch, arrayTemplate[i], config, ScriptErrors[i], ip->GetTime()))
							{
								// Success, put the pointer
								arrayTemplatePtr[i] = &arrayTemplate[i];
								builded = true;
							}
						}
					}

					// Ok ?
					if (!builded)
						ok = false;
				}
				else
					allPresent = false;
			}

			// Ok, continue
			if (ok)
			{
				// Load the materials
				CMaterial materials[2];

				// For each material
				for (uint mat=0; mat<2; mat++)
				{
					// Inputfile 0
					CIFile input;
					if (input.open (matFilename[mat]))
					{
						// Catch some errors
						try
						{
							// XML stream
							CIXml inputXml;
							nlverify (inputXml.init (input));

							// Serial
							materials[mat].serial (inputXml);
						}
						catch (Exception &e)
						{
							// Error message
							char tmp[2048];
							smprintf (tmp, 2048, "Error while loading material file %s : %s", matFilename[mat], e.what());
							CMaxToLigo::errorMessage (tmp, "NeL Ligo export transition", *MAXScript_interface, errorInDialog);
							ok = false;
						}
					}
					else
					{
						// Error message
						char tmp[512];
						smprintf (tmp, 512, "Can't open the file %s for reading.", matFilename[mat]);
						CMaxToLigo::errorMessage (tmp, "NeL Ligo export transition", *MAXScript_interface, errorInDialog);
						ok = false;
					}
				}

				// Ok ?
				if (ok)
				{
					// Build the transition
					CTransition transition;
					if (ok = transition.build (materials[0], materials[1], arrayTemplatePtr, config, ScriptErrors, ScriptErrors[CTransition::TransitionZoneCount]))
					{
						// Export ?
						if (!checkOnly)
						{
							// All transitions are ok ?
							if (allPresent)
							{
								// Build the zones
								NL3D::CZone zones[CTransition::TransitionZoneCount];
								uint zone;
								for (zone=0; zone<CTransition::TransitionZoneCount; zone++)
								{
									// Build the zone
									if (!arrayTri[zone]->rpatch->exportZone (arrayNode[zone], &arrayTri[zone]->patch, zones[zone], 0))
									{
										// Error, zone can't be exported
										CMaxToLigo::errorMessage ("Error: can't export the Nel zone, check bind errors.", "NeL Ligo export material", 
											*MAXScript_interface, errorInDialog);
										ok = false;
									}
								}

								// Catch exceptions
								if (ok)
								{
									// File names
									vector<string> createdfiles;
									try
									{
										// Open a stream file
										COFile outputfile;
										if (outputfile.open (fileName))
										{
											// Add the filename
											createdfiles.push_back (fileName);

											// Create an xml stream
											COXml outputXml;
											nlverify (outputXml.init (&outputfile));

											// Serial the class
											transition.serial (outputXml);

											// Make a name for the zone
											char drive[512];
											char dir[512];
											char name[512];

											// Export each zones
											for (zone=0; zone<CTransition::TransitionZoneCount; zone++)
											{
												// Final path
												char path[512];
												_splitpath (fileName, drive, dir, path, NULL);
												sprintf (name, "%s_%d", path, zone);
												_makepath (path, drive, dir, name, ".zone");

												// Another the stream
												COFile outputfile2;

												// Open another file
												if (outputfile2.open (path))
												{
													// Add the filename
													createdfiles.push_back (path);

													// Serial the zone
													zones[zone].serial (outputfile2);
												}
												else
												{
													// Error message
													char tmp[512];
													smprintf (tmp, 512, "Can't open the file %s for writing.", path);
													CMaxToLigo::errorMessage (tmp, "NeL Ligo export transition", *MAXScript_interface, errorInDialog);
													ok = false;
												}
											}
										}
										else
										{
											// Error message
											char tmp[512];
											smprintf (tmp, 512, "Can't open the file %s for writing.", fileName);
											CMaxToLigo::errorMessage (tmp, "NeL Ligo export transition", *MAXScript_interface, errorInDialog);
											ok = false;
										}
									}
									catch (Exception &e)
									{
										// Error message
										char tmp[512];
										smprintf (tmp, 512, "Error while save the file %s : %s", fileName, e.what());
										CMaxToLigo::errorMessage (tmp, "NeL Ligo export transition", *MAXScript_interface, errorInDialog);
										ok = false;
									}

									// Not ok ?
									if (!ok)
									{
										// Erase the files
										for (uint file=0; file<createdfiles.size(); file++)
										{
											// Removing files
											remove (createdfiles[file].c_str ());
										}
									}
								}
							}
							else
							{
								// Error message
								char tmp[512];
								smprintf (tmp, 512, "All transition are not present. Can't export..");
								CMaxToLigo::errorMessage (tmp, "NeL Ligo export transition", *MAXScript_interface, errorInDialog);
								ok = false;
							}
						}
					}
				}
			}
		}
		else
		{
			// Output an error
			CMaxToLigo::errorMessage ("Error: must have 9 cell in the node array", "NeL Ligo export transition", 
				*MAXScript_interface, errorInDialog);
		}
	}

	// Ok ?
	return ok?&true_value:&false_value;
}

// ***************************************************************************
 
Value* get_error_zone_template_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (4)
	check_arg_count(get_error_zone_template, 4, count);

	// Check to see if the arguments match up to what we expect
	char *message = "NeLLigoExportZoneTemplate [Array error codes] [Array vertex id] [Array messages] [Error index]";
	type_check(arg_list[0], Array, message);
	type_check(arg_list[1], Array, message);
	type_check(arg_list[2], Array, message);
	type_check(arg_list[3], Integer, message);

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

	// The third arg
	int errorIndex = arg_list[3]->to_int() - 1;
	clamp (errorIndex, 0, 8);

	// Num error
	uint numError = ScriptErrors[errorIndex].numVertexError ();

	// For each error
	for (uint i=0; i<numError; i++)
	{
		// Id and edge
		uint id, edge;
		uint error = (uint) ScriptErrors[errorIndex].getVertexError (i, id, edge);

		// Append error code
		errorCodes->append (Integer::intern (error+1));

		// Append vertex id
		vertexId->append (Integer::intern (id+1));

		// Append messages
		messages->append (new String("[LIGO DEBUG] Opened edge"));
	}

	// Return the main error message
	return Integer::intern ((int)ScriptErrors[errorIndex].MainError+1);
}

// ***************************************************************************
 
Value* get_snap_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (0)
	check_arg_count(get_snap, 0, count);

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
 
Value* get_cell_size_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (0)
	check_arg_count(get_cell_size, 0, count);

	// Load the ligofile
	CLigoConfig config;
	if (CMaxToLigo::loadLigoConfigFile (config, *MAXScript_interface, false))
	{
		return Float::intern ((float)config.CellSize);
	}
	else
		return &undefined;
}

// ***************************************************************************
 
/// Check a ligo zone with a ligo template
Value* check_zone_with_template_cf (Value** arg_list, int count)
{
/*	// Make sure we have the correct number of arguments (3)
	check_arg_count(check_zone_with_template, 3, count);

	// Check to see if the arguments match up to what we expect
	char *message = "NeLLigoExportZoneTemplate [Object] [Template filename] [Error in dialog]";
	type_check(arg_list[0], MAXNode, message);
	type_check(arg_list[1], String, message);
	type_check(arg_list[2], Boolean, message);

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// The first arg
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// The second arg
	const char *fileName = arg_list[1]->to_string();

	// The fourth arg
	bool errorInDialog = (arg_list[2]->to_bool() != FALSE);

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
				CMaxToLigo::errorMessage ("Error: can't load the config file ligoscape.cfg", "NeL Ligo check zone", 
					*MAXScript_interface, errorInDialog);
			}
			else
			{
				// The zone template
				CZoneTemplate zoneTemplate;

				// Build the zone template
				bool res = CMaxToLigo::buildZoneTemplate (node, tri->patch, zoneTemplate, config, ScriptErrors[0], ip->GetTime());

				// Success ?
				if (res)
				{
					// The zone template
					CZoneTemplate orignalZoneTemplate;

					// Read the template
					CIFile inputfile;

					// Catch exception
					try
					{
						// Open the selectec template file
						if (inputfile.open (fileName))
						{
							// Create an xml stream
							CIXml inputXml;
							inputXml.init (inputfile);

							// Serial the class
							orignalZoneTemplate.serial (inputXml);

							// Check it
							res=orignalZoneTemplate.checkTemplate (zoneTemplate, config, ScriptErrors);
						}
						else
						{
							// Error message
							char tmp[512];
							smprintf (tmp, 512, "Can't open the skeleton template file %s for reading.", fileName);
							CMaxToLigo::errorMessage (tmp, "NeL Ligo check zone", *MAXScript_interface, errorInDialog);
						}
					}
					catch (Exception &e)
					{
						// Error message
						char tmp[512];
						smprintf (tmp, 512, "Error while loading the file %s : %s", fileName, e.what());
						CMaxToLigo::errorMessage (tmp, "NeL Ligo check zone", *MAXScript_interface, errorInDialog);
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
				"NeL Ligo check zone", *MAXScript_interface, errorInDialog);
		}
	}
*/
	// Return false
	return &false_value;
}

// ***************************************************************************

Value* get_error_string_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (1)
	check_arg_count(get_error_string, 1, count);

	// Checks arg
	char *message = "NeLLigoGetErrorString [error code]";
	type_check(arg_list[0], Integer, message);

	// The first arg
	int errorCode = arg_list[0]->to_int()-1;

	// Error code
	return new String ((char*)CLigoError::getStringError ((CLigoError::TError)errorCode));
}

// ***************************************************************************

Value* set_directory_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (1)
	check_arg_count(set_directory, 1, count);

	// Checks arg
	char *message = "NeLLigoDirectory [path]";
	type_check(arg_list[0], String, message);

	// The first arg
	const char *dir = arg_list[0]->to_string();

	// Set the directory
	return (chdir (dir)==0)?&true_value:&false_value;
}

// ***************************************************************************

/// MAXScript Plugin Initialization

// ***************************************************************************

__declspec( dllexport ) void LibInit() { }

// ***************************************************************************

