/** \file export_collision.cpp
 * Export from 3dsmax to NeL
 *
 * $Id: export_collision.cpp,v 1.3 2002/03/12 16:32:25 berenguier Exp $
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

#include <vector>
#include <map>

#include "stdafx.h"
#include "export_nel.h"
#include "export_lod.h"

#include "../nel_export/std_afx.h"
#include "../nel_export/nel_export.h"
#include "../nel_export/nel_export_scene.h"

#include "pacs/collision_mesh_build.h"
#include "pacs/retriever_bank.h"
#include "pacs/global_retriever.h"
#include "pacs/build_indoor.h"
#include "3d/quad_grid.h"

using namespace std;
using namespace NLMISC;
using namespace NL3D;
using namespace NLPACS;

#define NEL_OBJET_NAME_DATA 1970


// ***************************************************************************

typedef pair<uint, uint>	TFaceRootMeshInfo;
typedef pair<uint, bool>	TEdgeInfo;

CCollisionMeshBuild*	CExportNel::createCollisionMeshBuild(std::vector<INode *> &nodes, TimeValue tvTime)
{
	CCollisionMeshBuild *pCollisionMeshBuild = new CCollisionMeshBuild();

	uint	i, j, node;
	uint	totalVertices = 0,
			totalFaces = 0,
			totalSurfaces = 0;

	vector<uint>				rootMeshVertices;
	vector<TFaceRootMeshInfo>	facesRootMeshesInfo;
	vector<string>				rootMeshNames;

	// merge all ondes into one CCollisionMeshBuild
	for (node=0; node<nodes.size(); ++node)
	{
		// Get a pointer on the object's node
		Object *obj = nodes[node]->EvalWorldState(tvTime).obj;

		// Check if there is an object
		if (obj)
		{		
			// Object can be converted in triObject ?
			if (obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0))) 
			{ 
				// Get a triobject from the node
				TriObject *tri = (TriObject*)obj->ConvertToType(tvTime, Class_ID(TRIOBJ_CLASS_ID, 0));

				// get the mesh name
				uint	meshId = rootMeshNames.size();
				rootMeshNames.push_back(nodes[node]->GetName());

				// Note that the TriObject should only be deleted
				// if the pointer to it is not equal to the object
				// pointer that called ConvertToType()
				bool deleteIt=false;
				if (obj != tri) 
					deleteIt = true;

				uint	i;
				Mesh	&mesh = tri->GetMesh();

				// Get the object matrix
				CMatrix ToWorldSpace;
				Matrix3 verticesToWorld = nodes[node]->GetObjectTM(tvTime);
				convertMatrix (ToWorldSpace, verticesToWorld);

				// Convert the vertices
				for (i=0; i<(uint)mesh.numVerts; ++i)
				{
					Point3 v=mesh.verts[i];
					CVector vv=ToWorldSpace*CVector (v.x, v.y, v.z);
					pCollisionMeshBuild->Vertices.push_back(vv);
					rootMeshVertices.push_back(node);
				}

				uint	maxMatId = 0;

				// Convert the faces
				for (i=0; i<(uint)mesh.numFaces; ++i)
				{
					facesRootMeshesInfo.push_back(make_pair(meshId, i));

					pCollisionMeshBuild->Faces.resize(pCollisionMeshBuild->Faces.size()+1);
					pCollisionMeshBuild->Faces.back().V[0] = mesh.faces[i].v[0]+totalVertices;
					pCollisionMeshBuild->Faces.back().V[1] = mesh.faces[i].v[1]+totalVertices;
					pCollisionMeshBuild->Faces.back().V[2] = mesh.faces[i].v[2]+totalVertices;

					pCollisionMeshBuild->Faces.back().Visibility[0] = ((mesh.faces[i].flags & EDGE_B) != 0);
					pCollisionMeshBuild->Faces.back().Visibility[1] = ((mesh.faces[i].flags & EDGE_C) != 0);
					pCollisionMeshBuild->Faces.back().Visibility[2] = ((mesh.faces[i].flags & EDGE_A) != 0);

					/// \todo get the real surface id
					uint32	maxMaterialId = mesh.faces[i].getMatID();
					if (maxMaterialId != 665 && maxMaterialId > maxMatId)
						maxMatId = maxMaterialId;
					sint32	sid = (maxMaterialId == 665) ? -1 : totalSurfaces+maxMaterialId;

					pCollisionMeshBuild->Faces.back().Surface = sid;
					pCollisionMeshBuild->Faces.back().Material = maxMaterialId;
				}

				totalVertices = pCollisionMeshBuild->Vertices.size();
				totalFaces = pCollisionMeshBuild->Faces.size();
				totalSurfaces += maxMatId+1;

				// Delete the triObject if we should...
				if (deleteIt)
					delete tri;
			}
		}
	}

	// Weld identical vertices.
	// using a grid to store indexes of vertices
	const sint		GridSize = 64;
	const float		GridWidth = 1.0f;
	const float		WeldThreshold = 0.005f;

	CQuadGrid<uint>	grid;
	vector<uint>	remapIds;
	vector<CVector>	remapVertices;
	vector<CVector>	&vertices = pCollisionMeshBuild->Vertices;

	vector<CVector>			previousVertices = pCollisionMeshBuild->Vertices;
	vector<CCollisionFace>	previousFaces = pCollisionMeshBuild->Faces;

	grid.create(GridSize, GridWidth);
	remapIds.resize(totalVertices);

	for (i=0; i<totalVertices; ++i)
	{
		remapIds[i] = i;

		CAABBox	box;
		box.setCenter(pCollisionMeshBuild->Vertices[i]);
		box.setHalfSize(CVector(WeldThreshold, WeldThreshold, 0.0f));

		grid.insert(box.getMin(), box.getMax(), i);
	}


	for (i=0; i<totalVertices; ++i)
	{
		if (remapIds[i] != i)
			continue;

		CVector		weldTo = vertices[i];

		// select close vertices
		grid.select(vertices[i], vertices[i]);

		// for each selected vertex, remaps it to the current vertex
		CQuadGrid<uint>::CIterator	it;
		for (it=grid.begin(); it!=grid.end(); ++it)
		{
			uint	weldedId = *it;
			CVector		welded = vertices[weldedId];

			if (weldedId <= i || rootMeshVertices[i] == rootMeshVertices[weldedId] ||
				remapIds[weldedId] != weldedId || (welded-weldTo).norm() > WeldThreshold)
				continue;

			remapIds[weldedId] = i;
		}
	}

	for (i=0; i<totalVertices; ++i)
	{
		if (remapIds[i] > i)
			nlerror("found a greater remap id");

		if (remapIds[i] == i)
		{
			uint	newId = remapVertices.size();
			remapVertices.push_back(vertices[i]);
			remapIds[i] = newId;
		}
		else
		{
			remapIds[i] = remapIds[remapIds[i]];
		}
	}

	for (i=0; i<totalFaces; ++i)
		for (j=0; j<3; ++j)
			pCollisionMeshBuild->Faces[i].V[j] = remapIds[pCollisionMeshBuild->Faces[i].V[j]];

	// check for errors
	vector<string>	warnings;
	for (i=0; i<totalFaces; ++i)
	{
		if (pCollisionMeshBuild->Faces[i].V[0] == pCollisionMeshBuild->Faces[i].V[1] ||
			pCollisionMeshBuild->Faces[i].V[1] == pCollisionMeshBuild->Faces[i].V[2] ||
			pCollisionMeshBuild->Faces[i].V[2] == pCollisionMeshBuild->Faces[i].V[0])
		{
			warnings.push_back(string("mesh:") + rootMeshNames[facesRootMeshesInfo[i].first] + string(" face:") + toString(facesRootMeshesInfo[i].second));
		}
	}

	// and clean up the mesh if some errors appear
	vector<CCollisionFace>::iterator	it;
	for (it=pCollisionMeshBuild->Faces.begin(); it!=pCollisionMeshBuild->Faces.end(); )
	{
		if ((*it).V[0] == (*it).V[1] ||
			(*it).V[1] == (*it).V[2] ||
			(*it).V[2] == (*it).V[0])
		{
			it = pCollisionMeshBuild->Faces.erase(it);
		}
		else
		{
			++it;
		}
	}

	pCollisionMeshBuild->Vertices = remapVertices;

	// check bbox size
	CAABBox	box;
	if (!pCollisionMeshBuild->Vertices.empty())
	{
		box.setCenter(pCollisionMeshBuild->Vertices[0]);
		for (i=1; i<pCollisionMeshBuild->Vertices.size(); ++i)
			box.extend(pCollisionMeshBuild->Vertices[i]);
	}

	CVector	hs = box.getHalfSize();
	if (hs.x > 255.0f || hs.y > 255.0f)
		MessageBox (theCNelExport.ip->GetMAXHWnd(), 
					"The bounding box of the selection exceeds 512 meters large!", 
					"NeL export collision", MB_OK|MB_ICONEXCLAMATION);

	// report warnings
	if (!warnings.empty())
	{
		string	message = "Warning(s) occured during collision export\n(defective links may result)";
		for (i=0; i<warnings.size(); ++i)
			message += string("\n")+warnings[i];

		MessageBox (theCNelExport.ip->GetMAXHWnd(), 
					(message+"\n\n(This message was copied in the clipboard)").c_str(), 
					"NeL export collision", MB_OK|MB_ICONEXCLAMATION);

		if (OpenClipboard (NULL))
		{
			HGLOBAL mem = GlobalAlloc (GHND|GMEM_DDESHARE, message.size()+1);
			if (mem)
			{
				char *pmem = (char *)GlobalLock (mem);
				strcpy (pmem, message.c_str());
				GlobalUnlock (mem);
				EmptyClipboard ();
				SetClipboardData (CF_TEXT, mem);
			}
			CloseClipboard ();
		}
	}

	vector<string>	errors;
	pCollisionMeshBuild->link(false, errors);
	pCollisionMeshBuild->link(true, errors);
	// report warnings
	if (!errors.empty())
	{
		string	message = "Error(s) occured during collision export\n(edge issues)";
		for (i=0; i<errors.size(); ++i)
			message += string("\n")+errors[i];

		MessageBox (theCNelExport.ip->GetMAXHWnd(), 
					(message+"\n\n(This message was copied in the clipboard)").c_str(), 
					"NeL export collision", MB_OK|MB_ICONEXCLAMATION);

		if (OpenClipboard (NULL))
		{
			HGLOBAL mem = GlobalAlloc (GHND|GMEM_DDESHARE, message.size()+1);
			if (mem)
			{
				char *pmem = (char *)GlobalLock (mem);
				strcpy (pmem, message.c_str());
				GlobalUnlock (mem);
				EmptyClipboard ();
				SetClipboardData (CF_TEXT, mem);
			}
			CloseClipboard ();
		}

		delete pCollisionMeshBuild;
		pCollisionMeshBuild = NULL;
	}

	// Return the shape pointer or NULL if an error occured.
	return pCollisionMeshBuild;
}


// ***************************************************************************
bool	CExportNel::createCollisionMeshBuildList(std::vector<INode *> &nodes, Interface& ip, TimeValue time,
	std::vector<std::pair<std::string, NLPACS::CCollisionMeshBuild*> > &meshBuildList)
{
	nlassert(meshBuildList.size()==0);
	// Result to return
	bool bRet=false;

	// Eval the objects a time
	uint	i, j;

	for (i=0; i<nodes.size(); ++i)
	{
		ObjectState os = nodes[i]->EvalWorldState(time);
		if (!os.obj)
			return bRet;
	}

	std::vector<std::pair<std::string, std::vector<INode *> > >	igs;
	for (i=0; i<nodes.size(); ++i)
	{
		// Object is flagged as a collision?
		int	bCol= getScriptAppData(nodes[i], NEL3D_APPDATA_COLLISION, BST_UNCHECKED);
		if(bCol == BST_CHECKED)
		{
			// If yes, add it to list
			std::string	ig = CExportNel::getScriptAppData(nodes[i], NEL3D_APPDATA_IGNAME, "");
			if (ig == "")
				ig = "unknown_ig";

			for (j=0; j<igs.size() && ig!=igs[j].first; ++j)
				;
			if (j == igs.size())
			{
				igs.push_back();
				igs[j].first = ig;
			}

			igs[j].second.push_back(nodes[i]);
		}
	}


	for (i=0; i<igs.size(); ++i)
	{
		std::string				igname = igs[i].first;
		std::vector<INode *>	&ignodes = igs[i].second;
		// Object exist ?
		CCollisionMeshBuild	*pCmb = CExportNel::createCollisionMeshBuild(ignodes, time);

		// Conversion success ?
		if (pCmb)
		{
			meshBuildList.push_back(make_pair(igname, pCmb));
			
			// All is good
			bRet=true;
		}
	}

	return bRet;
}


// ***************************************************************************
void	CExportNel::computeCollisionRetrieverFromScene(Interface& ip, TimeValue time, 
	CRetrieverBank *&retrieverBank, CGlobalRetriever *&globalRetriever,
	const char *igNamePrefix, const char *igNameSuffix, std::string &retIgName)
{
	// Default: empty retrieverBank/globalRetriever
	retrieverBank= NULL;
	globalRetriever= NULL;
	retIgName= "";

	// get list of nodes from scene
	std::vector<INode*>	nodes;
	getObjectNodes (nodes, time, ip);

	// build list of cmb.
	std::vector<std::pair<std::string, NLPACS::CCollisionMeshBuild*> >	meshBuildList;
	if( createCollisionMeshBuildList(nodes, ip, time, meshBuildList) && meshBuildList.size()>0 )
	{
		// create a retriverBnak and a global retrevier.
		retrieverBank= new CRetrieverBank;
		globalRetriever= new CGlobalRetriever;

		// list of valid instance to create.
		vector<pair<uint32, CVector> >	retrieverInstances;

		// fill the retrieverBank
		uint	i;
		for(i=0; i<meshBuildList.size();i++)
		{
			std::string				igname = meshBuildList[i].first;
			CCollisionMeshBuild		*pCmb = meshBuildList[i].second;

			// compute a localRetriever 
			CLocalRetriever		lr;
			CVector				translation;
			string				error;
			if( NLPACS::computeRetriever(*pCmb, lr, translation, error ) )
			{
				// set his id to the igname.
				lr.setIdentifier(igname);

				// Add to the retrieverBank
				uint32	lrId= retrieverBank->addRetriever(lr);

				// add this valid retrieverInstnace.
				retrieverInstances.push_back(make_pair(lrId, translation));
			}

			// free the CCollisionMeshBuild.
			delete pCmb;

			// does igname match prefix/suffix???
			if(igname.find(igNamePrefix)==0)
			{
				uint	lenPrefix= strlen(igNamePrefix);
				sint	pos= igname.find(igNameSuffix, lenPrefix);
				if(pos!=string::npos)
				{
					// Yes => setup the name between prefix/suffix
					retIgName= igname.substr(lenPrefix, pos-lenPrefix);
				}
			}
		}

		// fill the globalRetriever with all instances created.
		globalRetriever->setRetrieverBank(retrieverBank);
		for(i=0; i<retrieverInstances.size();i++)
		{
			// must set -translation
			globalRetriever->makeInstance(retrieverInstances[i].first, 0, -retrieverInstances[i].second);
		}
		// compile the globalRetriever.
		globalRetriever->initQuadGrid();
		globalRetriever->makeAllLinks();
	}

}

