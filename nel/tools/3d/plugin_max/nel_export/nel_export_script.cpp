/** \file nel_export_script.cpp
 * <File description>
 *
 * $Id: nel_export_script.cpp,v 1.7 2001/10/10 15:39:11 besson Exp $
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

#include "std_afx.h"
#include "nel_export.h"
#include "../nel_mesh_lib/export_nel.h"
#include "../nel_mesh_lib/export_lod.h"


#define EXPORT_GET_ALLOCATOR

#include "nel/misc/debug.h"
#include "nel/misc/file.h"

using namespace NLMISC;

/*===========================================================================*\
 |	Define our new functions
 |	These will turn on texture map display at all levels for a given object
\*===========================================================================*/

def_visible_primitive ( export_shape,	"NelExportShape");
def_visible_primitive ( export_animation,	"NelExportAnimation");
def_visible_primitive ( export_ig,		"NelExportInstanceGroup");
def_visible_primitive ( view_shape,		"NelViewShape");

char *sExportShapeErrorMsg = "NeLExportShape [Object] [Filename.shape]";
char *sExportAnimationErrorMsg = "NelExportAnimation [node array] [Filename.anim] [bool_scene_animation]";

Value* export_shape_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count(export_shape, 2, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check (arg_list[0], MAXNode, sExportShapeErrorMsg);
	type_check (arg_list[1], String, sExportShapeErrorMsg);

	// Get a INode pointer from the argument passed to us
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// Export path 
	const char* sPath=arg_list[1]->to_string();

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// Ok ?
	Boolean *ret=&false_value;

	// Is the flag dont export set ?
	if (CExportNel::getScriptAppData (node, NEL3D_APPDATA_DONTEXPORT, 0))
		return ret;

	// For the moment load the default config file.
	CExportNelOptions opt;
	char sConfigFileName[512];
	strcpy( sConfigFileName, ip->GetDir(APP_PLUGCFG_DIR) );
	strcat( sConfigFileName, "\\NelExportScene.cfg" );
	if( CNelExport::FileExists(sConfigFileName) )
	{
		// Serial the configuration
		try {
			CIFile inputFile;
			if( inputFile.open(sConfigFileName) )
			{
				opt.serial( inputFile );
			}
		}
		catch(...)
		{
		}
	}
	
	// Export
	CExportNel::deleteLM( *node, opt );
	if (CNelExport::exportMesh (sPath, *node, *ip, ip->GetTime(), opt))
		ret = &true_value;

	return ret;
}


Value* export_animation_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count(export_shape, 3, count);

	// Check to see if the arguments match up to what we expect
	type_check (arg_list[0], Array, sExportAnimationErrorMsg);
	type_check (arg_list[1], String, sExportAnimationErrorMsg);
	type_check (arg_list[2], Boolean, sExportAnimationErrorMsg);

	// Export path 
	const char* sPath=arg_list[1]->to_string();

	// Get time
	TimeValue time=MAXScript_interface->GetTime();
	
	// Get array
	Array* array=(Array*)arg_list[0];

	// Check each value in the array
	uint i;
	for (i=0; i<(uint)array->size; i++)
		type_check (array->get (i+1), MAXNode, sExportAnimationErrorMsg);

	// Ok ?
	Boolean *ret=&false_value;

	// Save all selected objects
	if (array->size)
	{
		// Make a list of nodes
		std::vector<INode*> vectNode;
		for (i=0; i<(uint)array->size; i++)
			vectNode.push_back (array->get (i+1)->to_node());

		// Scene anim ?
		BOOL scene=arg_list[2]->to_bool();

		// Export the zone
		if (theCNelExport.exportAnim (sPath, vectNode, *MAXScript_interface, time, scene!=FALSE))
		{
			// Ok
			ret=&true_value;
		}
		else
		{
			// Error message
			mprintf ("Error exporting animation %s in the file\n%s\n", (*vectNode.begin())->GetName(), sPath);
		}
	}

	return ret;
}


Value* export_ig_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count(export_shape, 2, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check (arg_list[0], Array, "NelExportInstanceGroup [Object array] [Filename]");
	type_check (arg_list[1], String, "NelExportInstanceGroup [Object array] [Filename]");

	// Ok ?
	Boolean *ret=&false_value;

	if (is_array (arg_list[0]))
	{
		// Get array
		Array* array=(Array*)arg_list[0];

		// Check each value in the array
		uint i;
		for (i=0; i<(uint)array->size; i++)
			type_check (array->get (i+1), MAXNode, "NelExportInstanceGroup [Object array] [Filename]");

		// Create a STL array
		if (array->size)
		{
			std::vector<INode*> vect;
			for (uint i=0; i<(uint)array->size; i++)
				vect.push_back (array->get (i+1)->to_node());
				
			// Export path 
			const char* sPath=arg_list[1]->to_string();

			// Get a good interface pointer
			Interface *ip = MAXScript_interface;

			// Export
			if (CNelExport::exportInstanceGroup (sPath, vect, *ip))
				ret=&true_value;
		}
	}

	return ret;
}

Value* view_shape_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count(view_shape, 0, count);

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// View
	// For the moment load the default options for lightmap
	CExportNelOptions opt;
	char sConfigFileName[512];
	strcpy( sConfigFileName, ip->GetDir(APP_PLUGCFG_DIR) );
	strcat( sConfigFileName, "\\NelExportScene.cfg" );
	if( CNelExport::FileExists(sConfigFileName) )
	{
		// Serial the configuration
		try {
			CIFile inputFile;
			if( inputFile.open(sConfigFileName) )
			{
				opt.serial( inputFile );
			}
		}
		catch(...)
		{
		}
	}
	CNelExport::viewMesh (*ip, ip->GetTime(), opt);

	return &true_value;
}

/*===========================================================================*\
 |	MAXScript Plugin Initialization
\*===========================================================================*/

__declspec( dllexport ) void
LibInit() { 
}

