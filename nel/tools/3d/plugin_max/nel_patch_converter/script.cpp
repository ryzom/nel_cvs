/** \file script.cpp
 * <File description>
 *
 * $Id: script.cpp,v 1.7 2002/06/20 17:17:05 corvazier Exp $
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

#include "PO2RPO.h"
#include "..\nel_patch_lib\rpo.h"

#include "3d/zone.h"
#include "3d/tile_bank.h"

#include "nel/misc/file.h"

#include "nel/misc/common.h"

using namespace NLMISC;
using namespace NL3D;
using namespace std;

#include "MAXScrpt.h"
#include "3dmath.h"

// Various MAX and MXS includes
#include "Numbers.h"
#include "MAXclses.h"
#include "Streams.h"
#include "MSTime.h"
#include "MAXObj.h"
#include "Parser.h"
#include "modstack.h"

#include "max.h"
#include "stdmat.h"

// define the new primitives using macros from SDK
#include "definsfn.h"

/*===========================================================================*\
 |	Define our new functions
 |	These will turn on texture map display at all levels for a given object
\*===========================================================================*/

def_visible_primitive( set_steps,				"SetRykolPatchSteps");
def_visible_primitive( set_tile_steps,			"SetRykolTileSteps");
def_visible_primitive( get_tile_count,			"GetRykolTileCount");
def_visible_primitive( get_patch_count,			"GetRykolPatchCount");
def_visible_primitive( get_sel_edge,			"GetRykolSelEdges");
def_visible_primitive( get_edge_vert1,			"GetRykolEdgesVert1");
def_visible_primitive( get_edge_vert2,			"GetRykolEdgesVert2");
def_visible_primitive( get_edge_vect1,			"GetRykolEdgesVect1");
def_visible_primitive( get_edge_vect2,			"GetRykolEdgesVect2");
def_visible_primitive( get_vertex_pos,			"GetRykolVertexPos");
def_visible_primitive( set_vertex_pos,			"SetRykolVertexPos");
def_visible_primitive( get_vector_pos,			"GetRykolVectorPos");
def_visible_primitive( set_vector_pos,			"SetRykolVectorPos");
def_visible_primitive( set_vector_count,		"GetRykolVectorCount");
def_visible_primitive( set_vertex_count,		"GetRykolVertexCount");
def_visible_primitive( set_interior_mode,		"SetRykolInteriorMode");
def_visible_primitive( set_compute_interior,	"RykolComputeInterior");
def_visible_primitive( set_tile_mode,			"SetRykolTileMode");
def_visible_primitive( get_selected_vertex,		"GetRykolSelVertex");
def_visible_primitive( get_selected_patch,		"GetRykolSelPatch");
def_visible_primitive( get_selected_tile,		"GetRykolSelTile");
def_visible_primitive( get_patch_vertex,		"NeLGetPatchVertex");


def_visible_primitive( get_tile_tile_number,	"NelGetTileTileNumber");
def_visible_primitive( get_tile_noise_number,	"NelGetTileNoiseNumber");
def_visible_primitive( set_tile_noise_number,	"NelSetTileNoiseNumber");

def_visible_primitive( load_bank,				"NelLoadBank");
def_visible_primitive( get_tile_set,			"NelGetTileSet");

def_visible_primitive( set_tile_bank,			"NelSetTileBank");

def_visible_primitive( export_zone,				"ExportRykolZone");
def_visible_primitive( import_zone,				"NeLImportZone");

/* permettre l'acces à auto/manual intrior edges
faire une methode pour interfacer la fonction compute interior edge
donner un acces à tiledmode/patchmode (on/off)
faire un getselectedvertex
faire un getselectedpatch
faire un getselectedtile */

/*def_visible_primitive( set_interior_mode,		"SetRykolInteriorMode");
def_visible_primitive( set_vertex_count,		"GetRykolVertexCount");*/

/*===========================================================================*\
 |	Implimentations of our new function calls
 |	notice the appended '_cf' to show its the function implimentation
\*===========================================================================*/

/*def_visible_primitive( get_sel_edge,			"GetRykolSelEdges");
def_visible_primitive( get_edge_vertex,			"GetRykolEdgesVertex");
def_visible_primitive( get_vertex_pos,			"GetRykolVertexPos");
def_visible_primitive( set_vertex_pos,			"SetRykolVertexPos");
def_visible_primitive( get_vector_pos,			"GetRykolVectorPos");
def_visible_primitive( set_vector_pos,			"SetRykolVectorPos");*/

void errorMessage (const char *msg, const char *title, Interface& it, bool dialog)
{
	// Text or dialog ?
	if (dialog)
	{
		// Dialog message
		MessageBox (it.GetMAXHWnd(), msg, title, MB_OK|MB_ICONEXCLAMATION);
	}
	else
	{
		// Text message
		mprintf ((string(msg) + "\n").c_str());
	}
}

Value*
export_zone_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count(export_zone, 3, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check(arg_list[0], MAXNode, "ExportRykolZone [Object]");
	//type_check(arg_list[1], Integer, "SetRykolPatchSteps [Object]");

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// Get a INode pointer from the argument passed to us
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// Get a Object pointer
	ObjectState os=node->EvalWorldState(ip->GetTime()); 

	// ok ?
	Boolean *ret=&false_value;

	if (os.obj)
	{
		// Get class id
		RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), 
			RYKOLPATCHOBJ_CLASS_ID);
		if (tri)
		{
			int nZone=arg_list[2]->to_int ();

			// Create a zone
			CZone zone;
			if (tri->rpatch->exportZone (node, &tri->patch, zone, nZone))
			{
				// Export path 
				const char* sPath=arg_list[1]->to_string();

				COFile file;
				if (file.open (sPath))
				{
					zone.serial (file);
					ret=&true_value;
				}
			}
		}
	}

	return ret;
}

Value* import_zone_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count (export_zone, 2, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	char *help = "NeLImportZone filename dialogError";
	type_check (arg_list[0], String, help);
	type_check (arg_list[1], Boolean, help);

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// Get the filename
	string filename = arg_list[0]->to_string();

	// Get the flip
	bool dialog = arg_list[1]->to_bool ()!=FALSE;

	// Return value
	Value *ret = &undefined;
		
	// Load the zone
	CIFile input;
	if (input.open (filename))
	{
		try
		{
			// The zone
			CZone zone;
			input.serial (zone);

			// Create an instance of NeL path mesh
			RPO* rpo = (RPO*) CreateInstance (GEOMOBJECT_CLASS_ID, RYKOLPATCHOBJ_CLASS_ID);
			rpo->rpatch = new RPatchMesh ();
			rpo->rpatch->rTess.TileTesselLevel = -5;

			// Convert the zone
			int zoneId;
			rpo->rpatch->importZone (&rpo->patch, zone, zoneId);

			// Create a derived object that references the rpo
			IDerivedObject *dobj = CreateDerivedObject(rpo);

			// Create a node in the scene that references the derived object
			INode *node = ip->CreateObjectNode (dobj);

    		// Return the result
			ret = new MAXNode (node);

			// Redraw the viewports
			ip->RedrawViews(ip->GetTime());
       	}
		catch (Exception& e)
		{
			// Error message
			errorMessage (("Error when loading file "+filename+": "+e.what()).c_str(), "NeL import zone", *ip, dialog);
		}
	}
	else
	{
		// Error message
		errorMessage (("Can't open the file "+filename+" for reading.").c_str(), "NeL import zone", *ip, dialog);
	}

	return ret;
}

Value*
get_selected_tile_cf(Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count(get_selected_tile, 1, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check(arg_list[0], MAXNode, "GetRykolSeltile [Object]");
	//type_check(arg_list[1], Integer, "SetRykolPatchSteps [Object]");

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// Get a INode pointer from the argument passed to us
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// Get a Object pointer
	ObjectState os=node->EvalWorldState(ip->GetTime()); 

	// ok ?
	Array *array=NULL;

	if (os.obj)
	{
		// Get class id
		RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), 
			RYKOLPATCHOBJ_CLASS_ID);
		if (tri)
		{
			int nSize=tri->rpatch->tileSel.NumberSet();
			array=new Array (nSize);
			int j=0;
			for (int i=0; i<tri->rpatch->tileSel.GetSize(); i++)
			{
				if (tri->rpatch->tileSel[i])
					array->append(Integer::intern(i+1));
			}
		}
	}

	return array;
}

Value*
get_selected_patch_cf(Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count(get_selected_patch, 1, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check(arg_list[0], MAXNode, "GetRykolSelPatch [Object]");
	//type_check(arg_list[1], Integer, "SetRykolPatchSteps [Object]");

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// Get a INode pointer from the argument passed to us
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// Get a Object pointer
	ObjectState os=node->EvalWorldState(ip->GetTime()); 

	// ok ?
	Array *array=NULL;

	if (os.obj)
	{
		// Get class id
		RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), 
			RYKOLPATCHOBJ_CLASS_ID);
		if (tri)
		{
			int nSize=tri->patch.patchSel.NumberSet();
			array=new Array (nSize);
			int j=0;
			for (int i=0; i<tri->patch.patchSel.GetSize(); i++)
			{
				if (tri->patch.patchSel[i])
					array->append(Integer::intern(i+1));
			}
		}
	}

	return array;
}

Value*
get_selected_vertex_cf(Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count(get_selected_vertex, 1, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check(arg_list[0], MAXNode, "GetRykolSelVertex [Object]");
	//type_check(arg_list[1], Integer, "SetRykolPatchSteps [Object]");

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// Get a INode pointer from the argument passed to us
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// Get a Object pointer
	ObjectState os=node->EvalWorldState(ip->GetTime()); 

	// ok ?
	Array *array=NULL;

	if (os.obj)
	{
		// Get class id
		RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), 
			RYKOLPATCHOBJ_CLASS_ID);
		if (tri)
		{
			int nSize=tri->patch.vertSel.NumberSet();
			array=new Array (nSize);
			int j=0;
			for (int i=0; i<tri->patch.vertSel.GetSize(); i++)
			{
				if (tri->patch.vertSel[i])
					array->append(Integer::intern(i+1));
			}
		}
	}

	return array;
}

Value*
set_tile_mode_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count(set_interior_mode, 2, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check(arg_list[0], MAXNode, "SetRykolTileMode [Object]");

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// Get a INode pointer from the argument passed to us
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// Get a Object pointer
	ObjectState os=node->EvalWorldState(ip->GetTime()); 

	// ok ?
	bool bRet=false;

	if (os.obj)
	{
		// Get class id
		if (os.obj->CanConvertToType(Class_ID(PATCHOBJ_CLASS_ID, 0))) 
		{
			bRet=true;
			RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), 
				RYKOLPATCHOBJ_CLASS_ID);
			if (tri)
			{
				tri->rpatch->rTess.ModeTile=(arg_list[1]->to_int()!=0);
			}
			// Note that the TriObject should only be deleted
			// if the pointer to it is not equal to the object
			// pointer that called ConvertToType()
			if (os.obj != tri)
				delete tri;

			// redraw and update
			node->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE); 
			ip->RedrawViews(ip->GetTime());
		}
	}

	return bRet?&true_value:&false_value;
}

Value*
set_compute_interior_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count(set_compute_interior, 1, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check(arg_list[0], MAXNode, "RykolComputeInterior [Object]");

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// Get a INode pointer from the argument passed to us
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// Get a Object pointer
	ObjectState os=node->EvalWorldState(ip->GetTime()); 

	// ok ?
	bool bRet=false;

	if (os.obj)
	{
		// Get class id
		if (os.obj->CanConvertToType(RYKOLPATCHOBJ_CLASS_ID)) 
		{
			bRet=true;
			RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), 
				RYKOLPATCHOBJ_CLASS_ID);
			if (tri)
				tri->patch.computeInteriors();
			// Note that the TriObject should only be deleted
			// if the pointer to it is not equal to the object
			// pointer that called ConvertToType()
			if (os.obj != tri)
				delete tri;
		
			// redraw and update
			node->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE); 
			ip->RedrawViews(ip->GetTime());
		}
	}

	return bRet?&true_value:&false_value;
}

Value*
set_interior_mode_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count(set_interior_mode, 3, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check(arg_list[0], MAXNode, "SetRykolInteriorMode [Object]");
	//type_check(arg_list[1], Integer, "SetRykolPatchSteps [Object]");

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// Get a INode pointer from the argument passed to us
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// Get a Object pointer
	ObjectState os=node->EvalWorldState(ip->GetTime()); 

	// ok ?
	bool bRet=false;

	if (os.obj)
	{
		// Get class id
		if (os.obj->CanConvertToType(RYKOLPATCHOBJ_CLASS_ID)) 
		{
			bRet=true;
			RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), 
				RYKOLPATCHOBJ_CLASS_ID);
			if (tri)
			{
				if (arg_list[1]->to_int()<=tri->patch.numPatches)
					tri->patch.patches[arg_list[1]->to_int()-1].SetAuto (arg_list[2]->to_int());
			}
			// Note that the TriObject should only be deleted
			// if the pointer to it is not equal to the object
			// pointer that called ConvertToType()
			if (os.obj != tri)
				delete tri;
		
			// redraw and update
			node->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE); 
			ip->RedrawViews(ip->GetTime());
		}
	}

	return bRet?&true_value:&false_value;
}

Value*
set_vertex_count_cf(Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count(set_vertex_count, 1, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check(arg_list[0], MAXNode, "GetRykolVertexCount [Object]");
	//type_check(arg_list[1], Integer, "SetRykolPatchSteps [Object]");

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// Get a INode pointer from the argument passed to us
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// Get a Object pointer
	ObjectState os=node->EvalWorldState(ip->GetTime()); 

	// ok ?
	int nRet=-1;

	if (os.obj)
	{
		// Get class id
		RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), 
			RYKOLPATCHOBJ_CLASS_ID);
		if (tri)
		{
			nRet=tri->patch.numVerts;
		}
	}

	return Integer::intern(nRet);
}

Value*
set_vector_count_cf(Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count(set_vector_count, 1, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check(arg_list[0], MAXNode, "GetRykolVectorCount [Object]");
	//type_check(arg_list[1], Integer, "SetRykolPatchSteps [Object]");

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// Get a INode pointer from the argument passed to us
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// Get a Object pointer
	ObjectState os=node->EvalWorldState(ip->GetTime()); 

	// ok ?
	int nRet=-1;

	if (os.obj)
	{
		// Get class id
		RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), 
			RYKOLPATCHOBJ_CLASS_ID);
		if (tri)
		{
			nRet=tri->patch.numVecs;
		}
	}

	return Integer::intern(nRet);
}

Value*
set_vertex_pos_cf(Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count(set_vertex_pos, 3, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check(arg_list[0], MAXNode, "SetRykolVertexPos [Object]");
	//type_check(arg_list[1], Integer, "SetRykolPatchSteps [Object]");

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// Get a INode pointer from the argument passed to us
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// Get a Object pointer
	ObjectState os=node->EvalWorldState(ip->GetTime()); 

	// ok ?
	bool bRet=false;

	if (os.obj)
	{
		// Get class id
		RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), 
			RYKOLPATCHOBJ_CLASS_ID);
		if (tri)
		{
			int nVertex=arg_list[1]->to_int()-1;
			Point3 point=arg_list[2]->to_point3();
			if ((nVertex>=0)&&(nVertex<tri->patch.numVerts))
			{
				bRet=true;
				tri->patch.verts[nVertex].p=point;
				tri->patch.InvalidateGeomCache();
				tri->rpatch->InvalidateChannels(PART_ALL);
				node->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE); 
				ip->RedrawViews(ip->GetTime());
			}
		}
	}

	return bRet?&true_value:&false_value;
}

Value*
set_vector_pos_cf(Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count(set_vector_pos, 3, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check(arg_list[0], MAXNode, "SetRykolVectorPos [Object]");
	//type_check(arg_list[1], Integer, "SetRykolPatchSteps [Object]");

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// Get a INode pointer from the argument passed to us
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// Get a Object pointer
	ObjectState os=node->EvalWorldState(ip->GetTime()); 

	// ok ?
	bool bRet=false;

	if (os.obj)
	{
		// Get class id
		RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), 
			RYKOLPATCHOBJ_CLASS_ID);
		if (tri)
		{
			int nVertex=arg_list[1]->to_int()-1;
			Point3 point=arg_list[2]->to_point3();
			if ((nVertex>=0)&&(nVertex<tri->patch.numVecs))
			{
				bRet=true;
				tri->patch.vecs[nVertex].p=point;
				tri->patch.InvalidateGeomCache();
				tri->rpatch->InvalidateChannels(PART_ALL);
				node->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE); 
				ip->RedrawViews(ip->GetTime());
			}
		}
	}

	return bRet?&true_value:&false_value;
}

Value*
get_vertex_pos_cf(Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count(get_vertex_pos, 2, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check(arg_list[0], MAXNode, "GetRykolVertexPos [Object]");
	//type_check(arg_list[1], Integer, "SetRykolPatchSteps [Object]");

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// Get a INode pointer from the argument passed to us
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// Get a Object pointer
	ObjectState os=node->EvalWorldState(ip->GetTime()); 

	// ok ?
	Value *vRet=NULL;

	if (os.obj)
	{
		// Get class id
		RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), 
			RYKOLPATCHOBJ_CLASS_ID);
		if (tri)
		{
			int nVertex=arg_list[1]->to_int()-1;
			if ((nVertex>=0)&&(nVertex<tri->patch.numVerts))
			{
				vRet=new Point3Value (tri->patch.verts[nVertex].p);
			}
		}
	}

	return vRet;
}

Value*
get_vector_pos_cf(Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count(get_vector_pos, 2, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check(arg_list[0], MAXNode, "GetRykolVectorPos [Object]");
	//type_check(arg_list[1], Integer, "SetRykolPatchSteps [Object]");

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// Get a INode pointer from the argument passed to us
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// Get a Object pointer
	ObjectState os=node->EvalWorldState(ip->GetTime()); 

	// ok ?
	Value *vRet=NULL;

	if (os.obj)
	{
		// Get class id
		RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), 
			RYKOLPATCHOBJ_CLASS_ID);
		if (tri)
		{
			int nVertex=arg_list[1]->to_int()-1;
			if ((nVertex>=0)&&(nVertex<tri->patch.numVecs))
			{
				vRet=new Point3Value (tri->patch.vecs[nVertex].p);
			}
		}
	}

	return vRet;
}


Value*
get_edge_vect1_cf(Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count(get_edge_vect1, 2, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check(arg_list[0], MAXNode, "GetRykolEdgesVect1 [Object]");
	//type_check(arg_list[1], Integer, "SetRykolPatchSteps [Object]");

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// Get a INode pointer from the argument passed to us
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// Get a Object pointer
	ObjectState os=node->EvalWorldState(ip->GetTime()); 

	// ok ?
	int nVert=-1;

	if (os.obj)
	{
		// Get class id
		RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), 
			RYKOLPATCHOBJ_CLASS_ID);
		if (tri)
		{
			int nEdge=arg_list[1]->to_int()-1;
			if ((nEdge>=0)&&(nEdge<tri->patch.numEdges))
			{
				nVert=tri->patch.edges[nEdge].vec12;
			}
		}
	}

	return Integer::intern(nVert);
}

Value*
get_edge_vect2_cf(Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count(get_edge_vect2, 2, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check(arg_list[0], MAXNode, "GetRykolEdgesVect2 [Object]");
	//type_check(arg_list[1], Integer, "SetRykolPatchSteps [Object]");

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// Get a INode pointer from the argument passed to us
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// Get a Object pointer
	ObjectState os=node->EvalWorldState(ip->GetTime()); 

	// ok ?
	int nVert=-1;

	if (os.obj)
	{
		// Get class id
		RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), 
			RYKOLPATCHOBJ_CLASS_ID);
		if (tri)
		{
			int nEdge=arg_list[1]->to_int()-1;
			if ((nEdge>=0)&&(nEdge<tri->patch.numEdges))
			{
				nVert=tri->patch.edges[nEdge].vec21;
			}
		}
	}

	return Integer::intern(nVert);
}

Value*
get_edge_vert1_cf(Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count(get_edge_vert1, 2, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check(arg_list[0], MAXNode, "GetRykolEdgesVert1 [Object]");
	//type_check(arg_list[1], Integer, "SetRykolPatchSteps [Object]");

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// Get a INode pointer from the argument passed to us
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// Get a Object pointer
	ObjectState os=node->EvalWorldState(ip->GetTime()); 

	// ok ?
	int nVert=-1;

	if (os.obj)
	{
		// Get class id
		RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), 
			RYKOLPATCHOBJ_CLASS_ID);
		if (tri)
		{
			int nEdge=arg_list[1]->to_int()-1;
			if ((nEdge>=0)&&(nEdge<tri->patch.numEdges))
			{
				nVert=tri->patch.edges[nEdge].v1;
			}
		}
	}

	return Integer::intern(nVert);
}


Value*
get_edge_vert2_cf(Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count(get_edge_vert2, 2, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check(arg_list[0], MAXNode, "GetRykolEdgesVert2 [Object]");
	//type_check(arg_list[1], Integer, "SetRykolPatchSteps [Object]");

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// Get a INode pointer from the argument passed to us
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// Get a Object pointer
	ObjectState os=node->EvalWorldState(ip->GetTime()); 

	// ok ?
	int nVert=-1;

	if (os.obj)
	{
		// Get class id
		RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), 
			RYKOLPATCHOBJ_CLASS_ID);
		if (tri)
		{
			int nEdge=arg_list[1]->to_int()-1;
			if ((nEdge>=0)&&(nEdge<tri->patch.numEdges))
			{
				nVert=tri->patch.edges[nEdge].v2;
			}
		}
	}

	return Integer::intern(nVert);
}

Value*
get_sel_edge_cf(Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count(set_tile_steps, 1, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check(arg_list[0], MAXNode, "SetRykolSelEdges [Object]");
	//type_check(arg_list[1], Integer, "SetRykolPatchSteps [Object]");

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// Get a INode pointer from the argument passed to us
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// Get a Object pointer
	ObjectState os=node->EvalWorldState(ip->GetTime()); 

	// ok ?
	Array *array=NULL;

	if (os.obj)
	{
		// Get class id
		RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), 
			RYKOLPATCHOBJ_CLASS_ID);
		if (tri)
		{
			int nSize=tri->patch.edgeSel.NumberSet();
			array=new Array (nSize);
			int j=0;
			for (int i=0; i<tri->patch.edgeSel.GetSize(); i++)
			{
				if (tri->patch.edgeSel[i])
					array->append(Integer::intern(i+1));
					//array->data[j++]=;
			}
		}
	}

	return array;
}

Value*
set_steps_cf(Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count(set_steps, 2, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check(arg_list[0], MAXNode, "SetRykolPatchSteps [Object]");
	//type_check(arg_list[1], Integer, "SetRykolPatchSteps [Object]");

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// Get a INode pointer from the argument passed to us
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// Get a Object pointer
	ObjectState os=node->EvalWorldState(ip->GetTime()); 

	// ok ?
	bool bRet=false;

	if (os.obj)
	{
		// Get class id
		Class_ID classId=os.obj->ClassID();
		if (classId==RYKOLPATCHOBJ_CLASS_ID)
		{
			bRet=true;
			static_cast<RPO*>(os.obj)->SetMeshSteps(arg_list[1]->to_int());
		}
		if (os.obj->CanConvertToType(Class_ID(PATCHOBJ_CLASS_ID, 0))) 
		{
			bRet=true;
			PatchObject *tri = (PatchObject *) os.obj->ConvertToType(ip->GetTime(), 
				Class_ID(PATCHOBJ_CLASS_ID, 0));
			if (tri)
				tri->SetMeshSteps(arg_list[1]->to_int());
			// Note that the TriObject should only be deleted
			// if the pointer to it is not equal to the object
			// pointer that called ConvertToType()
			if (os.obj != tri)
				delete tri;
		}
		if (classId==Class_ID(PATCHOBJ_CLASS_ID,0))
		{
			bRet=true;
			static_cast<PatchObject*>(os.obj)->SetMeshSteps(arg_list[1]->to_int());
		}
		if (bRet)
		{
			// redraw and update
			node->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE); 
			ip->RedrawViews(ip->GetTime());
		}
	}

	return bRet?&true_value:&false_value;
}


Value*
set_tile_steps_cf(Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count(set_tile_steps, 2, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check(arg_list[0], MAXNode, "SetRykoltileSteps [Object]");
	//type_check(arg_list[1], Integer, "SetRykolPatchSteps [Object]");

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// Get a INode pointer from the argument passed to us
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// Get a Object pointer
	ObjectState os=node->EvalWorldState(ip->GetTime()); 

	// ok ?
	bool bRet=false;

	if (os.obj)
	{
		// Get class id
		RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), 
			RYKOLPATCHOBJ_CLASS_ID);
		if (tri)
		{
			bRet=true;
			int nTess=arg_list[1]->to_int();
			if (nTess<-5)
				nTess=-5;
			if (nTess>5)
				nTess=5;
			tri->rpatch->rTess.TileTesselLevel=nTess;
			tri->rpatch->InvalidateChannels (PART_ALL);
		}	
		if (bRet)
		{
			// redraw and update
			node->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE); 
			ip->RedrawViews(ip->GetTime());
		}
	}

	return bRet?&true_value:&false_value;
}

Value*
get_tile_count_cf(Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count(get_tile_count, 2, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check(arg_list[0], MAXNode, "GetRykolTileCount [Zone] [PatchNumber]");
	//type_check(arg_list[1], Integer, "SetRykolPatchSteps [Object]");

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// Get a INode pointer from the argument passed to us
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// Get a Object pointer
	ObjectState os=node->EvalWorldState(ip->GetTime()); 

	// ok ?
	int nRet=-1;

	if (os.obj)
	{
		// Get class id
		Class_ID classId=os.obj->ClassID();
		RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), 
			RYKOLPATCHOBJ_CLASS_ID);
		if (tri)
		{
			int nPatch=arg_list[1]->to_int();
			if ((nPatch<(int)tri->rpatch->getUIPatchSize())&&(nPatch>=0))
			{
				nRet=(1<<(tri->rpatch->getUIPatch (nPatch).NbTilesU))*
				(1<<(tri->rpatch->getUIPatch (nPatch).NbTilesV));
			}

			// Note that the TriObject should only be deleted
			// if the pointer to it is not equal to the object
			// pointer that called ConvertToType()
			if (os.obj != tri)
				delete tri;
		}


		if (nRet!=-1)
		{
			// redraw and update
			node->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE); 
			ip->RedrawViews(ip->GetTime());
		}
	}

	return Integer::intern(nRet);
}

Value* get_patch_vertex_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (3)
	check_arg_count(get_tile_count, 3, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	char *message="NeLGetPatchVertex [NeLPatchMesh] [PatchId] [VertexId]";
	type_check(arg_list[0], MAXNode, message);
	type_check(arg_list[1], Integer, message);
	type_check(arg_list[2], Integer, message);

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// Get a INode pointer from the argument passed to us
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// ok ?
	int nRet=-1;

	// Get the patch id
	int patchId = arg_list[1]->to_int() - 1;

	// Get the vertex id
	int vertId = arg_list[2]->to_int() - 1;

	// Valid vertex id ?
	if ((vertId>=0) && (vertId<4))
	{
		// Get a Object pointer
		ObjectState os=node->EvalWorldState(ip->GetTime()); 

		if (os.obj)
		{
			// Get class id
			Class_ID classId=os.obj->ClassID();
			RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), 
				RYKOLPATCHOBJ_CLASS_ID);
			if (tri)
			{
				if ((patchId<(int)tri->patch.numPatches)&&(patchId>=0))
				{
					// Get the vertex id
					nRet = tri->patch.patches[patchId].v[vertId] + 1;
				}

				// Note that the TriObject should only be deleted
				// if the pointer to it is not equal to the object
				// pointer that called ConvertToType()
				if (os.obj != tri)
					delete tri;
			}


			if (nRet!=-1)
			{
				// redraw and update
				node->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE); 
				ip->RedrawViews(ip->GetTime());
			}
		}
	}

	return Integer::intern(nRet);
}

Value*
get_patch_count_cf(Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (2)
	check_arg_count(get_patch_count, 1, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check(arg_list[0], MAXNode, "GetRykolPatchCount [Object]");
	//type_check(arg_list[1], Integer, "SetRykolPatchSteps [Object]");

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// Get a INode pointer from the argument passed to us
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// Get a Object pointer
	ObjectState os=node->EvalWorldState(ip->GetTime()); 

	// ok ?
	int nRet=-1;

	if (os.obj)
	{
		// Get class id
		Class_ID classId=os.obj->ClassID();
		RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), 
			RYKOLPATCHOBJ_CLASS_ID);
		if (tri)
		{
			nRet=tri->rpatch->getUIPatchSize();

			// Note that the TriObject should only be deleted
			// if the pointer to it is not equal to the object
			// pointer that called ConvertToType()
			if (os.obj != tri)
				delete tri;
		}


		if (nRet!=-1)
		{
			// redraw and update
			node->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			ip->RedrawViews(ip->GetTime());
		}
	}

	return Integer::intern(nRet);
}

Value*
get_tile_tile_number_cf(Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (4)
	check_arg_count(get_tile_count, 4, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check(arg_list[0], MAXNode, "NelGetTileTileNumber [Zone] [PatchNumber] [TileNumber] [Layer]");
	type_check(arg_list[1], Integer, "NelGetTileTileNumber [Zone] [PatchNumber] [TileNumber] [Layer]");
	type_check(arg_list[2], Integer, "NelGetTileTileNumber [Zone] [PatchNumber] [TileNumber] [Layer]");
	type_check(arg_list[3], Integer, "NelGetTileTileNumber [Zone] [PatchNumber] [TileNumber] [Layer]");

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// Get a INode pointer from the argument passed to us
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// Get a Object pointer
	ObjectState os=node->EvalWorldState(ip->GetTime()); 

	// ok ?
	int nRet=-1;

	// Get the layer
	uint layer=arg_list[3]->to_int()-1;
	if (layer>=3)
	{
		mprintf ("Error: layer must be 1, 2, or 3\n");
	}
	else
	{
		if (os.obj)
		{
			// Get class id
			Class_ID classId=os.obj->ClassID();
			RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), 
				RYKOLPATCHOBJ_CLASS_ID);
			if (tri)
			{
				// Get the patch number
				uint nPatch=arg_list[1]->to_int()-1;
				if (nPatch>=tri->rpatch->getUIPatchSize())
				{
					mprintf ("Error: patch index is invalid.\n");
				}
				else
				{
					// Number of tile in this patch
					uint nPatchCount=(1<<(tri->rpatch->getUIPatch (nPatch).NbTilesU))*
						(1<<(tri->rpatch->getUIPatch (nPatch).NbTilesV));

					// Get the tile number
					uint tile=arg_list[2]->to_int()-1;
					if (tile>=nPatchCount)
					{
						mprintf ("Error: tile index is invalid.\n");
					}
					else
					{
						// Tile index
						if (layer<(uint)tri->rpatch->getUIPatch (nPatch).getTileDesc (tile).getNumLayer())
						{
							nRet=tri->rpatch->getUIPatch (nPatch).getTileDesc (tile).getLayer (layer).Tile+1;
						}
						else
						{
							nRet=0;
						}
					}
				}

				// Note that the TriObject should only be deleted
				// if the pointer to it is not equal to the object
				// pointer that called ConvertToType()
				if (os.obj != tri)
					delete tri;
			}
		}
	}

	return Integer::intern(nRet);
}

Value*
get_tile_noise_number_cf(Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (4)
	check_arg_count(get_tile_count, 3, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check(arg_list[0], MAXNode, "NelGetTileTileNumber [Zone] [PatchNumber] [TileNumber]");
	type_check(arg_list[1], Integer, "NelGetTileTileNumber [Zone] [PatchNumber] [TileNumber]");
	type_check(arg_list[2], Integer, "NelGetTileTileNumber [Zone] [PatchNumber] [TileNumber]");

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// Get a INode pointer from the argument passed to us
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// Get a Object pointer
	ObjectState os=node->EvalWorldState(ip->GetTime()); 

	// ok ?
	int nRet=-1;

	if (os.obj)
	{
		// Get class id
		Class_ID classId=os.obj->ClassID();
		RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), 
			RYKOLPATCHOBJ_CLASS_ID);
		if (tri)
		{
			// Get the patch number
			uint nPatch=arg_list[1]->to_int()-1;
			if (nPatch>=tri->rpatch->getUIPatchSize())
			{
				mprintf ("Error: patch index is invalid.\n");
			}
			else
			{
				// Number of tile in this patch
				uint nPatchCount=(1<<(tri->rpatch->getUIPatch (nPatch).NbTilesU))*
					(1<<(tri->rpatch->getUIPatch (nPatch).NbTilesV));

				// Get the tile number
				uint tile=arg_list[2]->to_int()-1;
				if (tile>=nPatchCount)
				{
					mprintf ("Error: patch index is invalid.\n");
				}
				else
				{
					// Tile index
					nRet=tri->rpatch->getUIPatch (nPatch).getTileDesc (tile).getDisplace ()+1;
				}
			}

			// Note that the TriObject should only be deleted
			// if the pointer to it is not equal to the object
			// pointer that called ConvertToType()
			if (os.obj != tri)
				delete tri;
		}
	}

	return Integer::intern(nRet);
}

Value*
set_tile_noise_number_cf(Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (4)
	check_arg_count(get_tile_count, 4, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check(arg_list[0], MAXNode, "NelGetTileNoiseNumber [Zone] [PatchNumber] [TileNumber] [noise]");
	type_check(arg_list[1], Integer, "NelGetTileNoiseNumber [Zone] [PatchNumber] [TileNumber] [noise]");
	type_check(arg_list[2], Integer, "NelGetTileNoiseNumber [Zone] [PatchNumber] [TileNumber] [noise]");
	type_check(arg_list[3], Integer, "NelGetTileNoiseNumber [Zone] [PatchNumber] [TileNumber] [noise]");

	// Get a good interface pointer
	Interface *ip = MAXScript_interface;

	// Get a INode pointer from the argument passed to us
	INode *node = arg_list[0]->to_node();
	nlassert (node);

	// Get a Object pointer
	ObjectState os=node->EvalWorldState(ip->GetTime()); 

	// ok ?
	bool bRet=false;

	// Get noise number
	uint noise=arg_list[3]->to_int()-1;
	if (noise>=16)
	{
		mprintf ("Error: noise value must be 1~16\n");
	}
	else
	{
		if (os.obj)
		{
			// Get class id
			Class_ID classId=os.obj->ClassID();
			RPO *tri = (RPO *) os.obj->ConvertToType(ip->GetTime(), 
				RYKOLPATCHOBJ_CLASS_ID);
			if (tri)
			{
				// Get the patch number
				uint nPatch=arg_list[1]->to_int()-1;
				if (nPatch>=tri->rpatch->getUIPatchSize())
				{
					mprintf ("Error: patch index is invalid.\n");
				}
				else
				{
					// Number of tile in this patch
					uint nPatchCount=(1<<(tri->rpatch->getUIPatch (nPatch).NbTilesU))*
						(1<<(tri->rpatch->getUIPatch (nPatch).NbTilesV));

					// Get the tile number
					uint tile=arg_list[2]->to_int()-1;
					if (tile>=nPatchCount)
					{
						mprintf ("Error: patch index is invalid.\n");
					}
					else
					{
						// Tile index
						tri->rpatch->getUIPatch (nPatch).getTileDesc (tile).setDisplace (noise);
						bRet=true;
					}
				}

				// Note that the TriObject should only be deleted
				// if the pointer to it is not equal to the object
				// pointer that called ConvertToType()
				if (os.obj != tri)
					delete tri;
			}
		}
	}

	return bRet?&true_value:&false_value;
}

CTileBank scriptedBank;

Value*
load_bank_cf(Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (4)
	check_arg_count(get_tile_count, 0, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	std::string bankName = GetBankPathName ();
	if (bankName!="")
	{
		try
		{
			// Open a file
			CIFile file;
			if (file.open (bankName))
			{
				// Read the bank
				file.serial (scriptedBank);

				// Build xref
				scriptedBank.computeXRef ();

				// Ok
				return &true_value;
			}
			else
			{
				mprintf ("Error: can't open bank file %s\n", bankName.c_str());
			}
		}
		catch (Exception& e)
		{
			// Error message
			mprintf ("Error: %s\n", e.what());
		}
	}

	// Error
	return &false_value;
}

Value*
get_tile_set_cf(Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (4)
	check_arg_count(get_tile_count, 1, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check(arg_list[0], Integer, "NelGetTileSet [tileId]");

	// ok ?
	int nRet=-1;

	// Get tile number
	uint tile=arg_list[0]->to_int()-1;
	if (tile>=(uint)scriptedBank.getTileCount())
	{
		mprintf ("Error: tile number is wrong. (1 ~ %d)\n", scriptedBank.getTileCount());
	}
	else
	{
		// Get the XRef for this bank
		int tileSet;
		int number;
		CTileBank::TTileType type;
		scriptedBank.getTileXRef (tile, tileSet, number, type);

		// Return value
		nRet=tileSet+1;
	}

	// Error
	return Integer::intern(nRet);
}

Value* set_tile_bank_cf (Value** arg_list, int count)
{
	// Make sure we have the correct number of arguments (4)
	check_arg_count(get_tile_count, 1, count);

	// Check to see if the arguments match up to what we expect
	// We want to use 'TurnAllTexturesOn <object to use>'
	type_check(arg_list[0], String, "NelSetTileBank [tile bank pathname]");

	// ok ?
	const char *pathname = arg_list[0]->to_string();

	// Get tile number
	SetBankPathName (pathname);

	// Error
	return &true_value;
}


/*===========================================================================*\
 |	MAXScript Plugin Initialization
\*===========================================================================*/

__declspec( dllexport ) void
LibInit() { 
}

