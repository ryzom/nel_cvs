/** \file export_mesh_interface.cpp
 *
 * $Id: export_mesh_interface.cpp,v 1.1 2002/04/23 16:30:04 vizerie Exp $
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


#include "stdafx.h"
#include "export_nel.h"

#include <set>
#include <vector>


#include "nel/misc/line.h"
#include "nel/misc/polygon.h"
#include "nel/misc/path.h"
#include "export_lod.h"




using namespace NLMISC;
using namespace NL3D;




///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// a vertice of a mesh interface, private use
struct CMeshInterfaceVertex
{
	CVector Pos;
	CVector Normal;

	// test wether the given vertex can be welded to one of this interface vertices
	bool	canWeld(const NLMISC::CVector &vert, float threshold) const
	{
		NLMISC::CVector distV = vert - Pos;
		return distV.norm() < threshold;
	}

	// try snapping a vertex to this mesh Interface
	bool	snapVert(NLMISC::CVector &vert, NLMISC::CVector &normal, const NLMISC::CMatrix &toWorldMat, float threshold)
	{					
		if (canWeld(toWorldMat * vert, threshold))
		{
			// snap to pos and normal
			// The pos of the interface vertex is in world space, so we put it back in object space			
			NLMISC::CMatrix invMat = toWorldMat.inverted();
			vert   = invMat * Pos; 
			normal = invMat.mulVector(Normal);
			return true;
		}				
		return false;
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// a mesh Interface, private use
struct CMeshInterface
{
	std::vector<CMeshInterfaceVertex> Verts;
	// try to snap a vertex to one of this Interface vertex
	bool	snapVert(NLMISC::CVector &vert, NLMISC::CVector &normal, const NLMISC::CMatrix &toWorldMat, float threshold)
	{
			for(uint k = 0; k < Verts.size(); ++k)
			{
				if (Verts[k].snapVert(vert, normal, toWorldMat, threshold)) return true;
			}		
			return false;
	}

	// test wether a vertex can be welded to that interface
	bool	canWeld(const NLMISC::CVector &pos, float threshold) const
	{
		for(uint k = 0; k < Verts.size(); ++k)
		{
			if (Verts[k].canWeld(pos, threshold)) return true;
		}
		return false;
	}

	// build this Interface from a max mesh (usually a polygon converted to a mesh)
	bool buildFromMaxMesh(INode &node, TimeValue tvTime)
	{
		// Get a pointer on the object's node
		Object *obj = node.EvalWorldState(tvTime).obj;

		// Check if there is an object
		if (!obj) return false;
		
		if (obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0))) 
		{ 
			// Get a triobject from the node
			TriObject *tri = (TriObject*)obj->ConvertToType(tvTime, Class_ID(TRIOBJ_CLASS_ID, 0));

			// Note that the TriObject should only be deleted
			// if the pointer to it is not equal to the object
			// pointer that called ConvertToType()
			bool deleteIt=false;
			if (obj != tri) 
				deleteIt = true;
			Mesh &mesh = tri->GetMesh();
			//
			CPolygon poly;			
			CExportNel::maxPolygonMeshToOrderedPoly(mesh, poly.Vertices);
			// build a local to world matrix
			Matrix3 localToWorld = node.GetObjectTM(tvTime);
			CMatrix nelMatLocalToWorld;
			CExportNel::convertMatrix(nelMatLocalToWorld, localToWorld);
			//			
			uint numVerts = poly.Vertices.size();
			Verts.resize(numVerts);
			uint k;
			for(k = 0; k < numVerts; ++k)
			{
				// puts vertex in world
				Verts[k].Pos = nelMatLocalToWorld * poly.Vertices[k];
			}
			// compute normals
			for(k = 0; k < numVerts; ++k)
			{
				CLine line(Verts[k == 0 ? (numVerts - 1) : k - 1].Pos, Verts[(k == numVerts - 1) ? 0 : k + 1].Pos);
				CVector proj;
				line.project(Verts[k].Pos, proj);
				Verts[k].Normal = (Verts[k].Pos - proj).normed();
			}
			//
			if (deleteIt)
			{
				delete tri;
			}
			return true;						
		}
		return false;
	}
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// private func to apply a set of mesh Interface to a meshbuild
static void ApplyMeshInterfaces(std::vector<CMeshInterface> &interfaces, CMesh::CMeshBuild &mbuild, const NLMISC::CMatrix &toWorldMat, float threshold)
{	
	// for every faces
	for(uint k = 0; k < mbuild.Faces.size(); ++k)
	{
		// test each corner
		for(uint l = 0; l < 3; ++l)
		{
			// against each Interface
			for(uint m = 0; m < interfaces.size(); ++m)
			{
				interfaces[m].snapVert(mbuild.Vertices[mbuild.Faces[k].Corner[l].Vertex],
									  mbuild.Faces[k].Corner[l].Normal,
									  toWorldMat,
									  threshold
									 );			
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** private func : Select vertices in a max mesh that match a CMeshInterface
  * It returns true if the operation was done properly
  */
static bool SelectVerticesInMeshFromInterfaces(const std::vector<CMeshInterface> &inters, float threshold, INode &node, TimeValue tvTime)
{	
	Object *obj = node.EvalWorldState(tvTime).obj;		
	// Check if there is an object
	if (obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0))) 
	{ 
		// Get a triobject from the node
		TriObject *tri = (TriObject*)obj->ConvertToType(tvTime, Class_ID(TRIOBJ_CLASS_ID, 0));					
		if (obj != tri) 
		{
			// not a mesh object, so do nothing
			delete tri;			
			return false;
		}
		
		Mesh &mesh = tri->GetMesh();
		// unselect all vertices
		mesh.VertSel().ClearAll();

		// build the local to wordl matrix
		Matrix3 localToWorld = node.GetObjectTM(tvTime);
		CMatrix nelMatLocalToWorld;
		CExportNel::convertMatrix(nelMatLocalToWorld, localToWorld);

		mesh.selLevel = DISP_SELVERTS ;
				
		// test each vertices against the junctions
		for(uint k = 0; k < (uint) mesh.getNumVerts(); ++k)
		{
			CVector pos;
			Point3  maxPos = mesh.getVert(k);
			CExportNel::convertVector(pos, maxPos);			

			for(uint l = 0; l < inters.size(); ++l)
			{
				if (inters[l].canWeld(nelMatLocalToWorld * pos, threshold))
				{
					mesh.VertSel().Set(k);
					break;
				}
			}
		}
		mesh.SetDispFlag(DISP_SELVERTS);	
	}
	return true;				
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** private func to build a vector of set interface from a max file  
  * The max file is merged to this one 
  */
static bool BuildMeshInterfaces(const char *cMaxFileName, std::vector<CMeshInterface> &meshInterfaces, CExportNel &exportNel, TimeValue tvTime)
{
	std::string maxFileName(cMaxFileName);
	// make a set of current scene nodes	
	std::vector<INode *> nodes;
	exportNel.getObjectNodes(nodes, tvTime);	
	std::set<INode *> firstNodes(nodes.begin(), nodes.end());

	if  (CFile::getExtension(maxFileName).empty())
	{
		maxFileName += ".max";
	}

	/** don't know why, but a call to Interface::MergeFromFile freeze the application, so we
	  * use a call to maxscript instead
	  */
	std::string command = std::string("mergeMAXFile \"" + maxFileName + "\" #noRedraw #mergeDups");
	if (CExportNel::scriptEvaluate(command.c_str(), NULL, scriptNothing) == false)
	{
		nlwarning("Unable to merge %s", maxFileName.c_str());
		return false;
	}

	
	// make a set of current scene nodes + merged nodes
	nodes.clear();
	exportNel.getObjectNodes(nodes, tvTime);		
	std::set<INode *> allNodes(nodes.begin(), nodes.end());
	std::set<INode *> mergedNodes;


	// compute difference to get merged nodes
	std::set_difference(allNodes.begin(), allNodes.end(),
						firstNodes.begin(), firstNodes.end(),						
						std::inserter(mergedNodes, mergedNodes.begin())
					   );

	if (mergedNodes.size() == 0)
	{
		nlwarning("Couldn't find interface : %s", maxFileName.c_str());
	}
	
	// build meshs interfaces set from merged nodes
	std::set<INode *>::iterator it;
	for(it = mergedNodes.begin(); it != mergedNodes.end(); ++it)
	{
		CMeshInterface meshInterface;
		if (meshInterface.buildFromMaxMesh(**it, tvTime))
		{
			// well we could avoid a vector copy
			meshInterfaces.push_back(meshInterface);
		}
		else
		{
			nlwarning("unable to build interface from %s", exportNel.getNelObjectName(**it).c_str());
		}
	}
	

	// remove loaded nodes from scene
	for(it = mergedNodes.begin(); it != mergedNodes.end(); ++it)
	{
		exportNel.getInterface()->DeleteNode(*it, FALSE);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CExportNel::applyInterfaceToMeshBuild(INode &node, CMesh::CMeshBuild &mbuild, const NLMISC::CMatrix &toWorldMat, TimeValue time)
{
	std::string interfaceFile = CExportNel::getScriptAppData(&node, NEL3D_APPDATA_INTERFACE_FILE, "");
	if (interfaceFile.empty()) return;

	// get the threshold
	float threshold = CExportNel::getScriptAppData(&node, NEL3D_APPDATA_INTERFACE_THRESHOLD, -1.f);

	if (threshold < 0.f)
	{
		nlwarning("Invalid threshold used for interface merging, in node %s", CExportNel::getNelObjectName(node).c_str() );
		return;
	}

	std::vector<CMeshInterface> meshInterface;
	if (! ::BuildMeshInterfaces(interfaceFile.c_str(), meshInterface, *this, time))
	{
		return;
	}
	
	
	// process the mesh build	
	::ApplyMeshInterfaces(meshInterface, mbuild, toWorldMat, threshold);
	
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool	CExportNel::useInterfaceMesh(INode &node)
{
	std::string interfaceFile = CExportNel::getScriptAppData(&node, NEL3D_APPDATA_INTERFACE_FILE, "");
	return !interfaceFile.empty();
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool	CExportNel::selectInterfaceVertices(INode &node, TimeValue time)
{
	std::string interfaceFile = CExportNel::getScriptAppData(&node, NEL3D_APPDATA_INTERFACE_FILE, "");
	if (interfaceFile.empty()) 
	{
		nlwarning("SelectInterfaceVertices : This node has no mesh interface : %s", this->getNelObjectName(node).c_str() );
		return false;
	}

	// get the threshold
	float threshold = CExportNel::getScriptAppData(&node, NEL3D_APPDATA_INTERFACE_THRESHOLD, -1.f);

	if (threshold < 0.f)
	{
		nlwarning("Invalid threshold used for interface merging, in node %s", CExportNel::getNelObjectName(node).c_str() );
		return false;
	}

	std::vector<CMeshInterface> meshInterfaces;
	if (!::BuildMeshInterfaces(interfaceFile.c_str(), meshInterfaces, *this, time))
	{
		return false;
	}

	return ::SelectVerticesInMeshFromInterfaces(meshInterfaces, threshold, node, time );			
}



