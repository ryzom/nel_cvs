
#include "stdafx.h"




#include "builderLogic.h"
#include "display.h"
#include "toolsLogic.h"
#include "../lib/primitive.h"

#include "nel/misc/vector.h"

#include "nel/misc/o_xml.h"
#include "nel/misc/i_xml.h"
#include "nel/misc/file.h"

#include "3d/nelu.h"
#include "3d/vertex_buffer.h"
#include "3d/primitive_block.h"
#include "3d/material.h"

using namespace NLLIGO;
using namespace NLMISC;
using namespace NL3D;
using namespace std;

// ***************************************************************************
// SPrimBuild
// ***************************************************************************

// ---------------------------------------------------------------------------
SPrimBuild::SPrimBuild()
{
	PRegion = NULL;
	Type = -1;		// 0 (Point),1 (Path), 2 (Zone),-1 (Not Valid)
	Created = false;
	Pos = 0;
	hidden = false;
}

// ***************************************************************************
// CBuilderLogic
// ***************************************************************************

// ---------------------------------------------------------------------------
CBuilderLogic::CBuilderLogic()
{
	_ItemSelected = NULL;
	_VertexSelected = -1;
	_RegionSelected = -1;
	_ToolsLogic = NULL;
}

// ---------------------------------------------------------------------------
void CBuilderLogic::setDisplay (CDisplay *pDisp)
{
	_Display = pDisp;
}

// ---------------------------------------------------------------------------
void CBuilderLogic::setToolsLogic (CToolsLogic *pTool)
{
	_ToolsLogic = pTool;
}

// ---------------------------------------------------------------------------
void CBuilderLogic::updateToolsLogic ()
{
	// Regenerate the toolsLogic with the data in the region
	_Primitives.clear ();
	if (_ToolsLogic != NULL)
		_ToolsLogic->reset();
	uint32 i, j;
	sint32 tempItem = -1;
	for (j = 0; j < _PRegions.size(); ++j)
	{
		CPrimRegion &rRegion = *_PRegions[j];
		sint32 nReg = j;

		
		if (_ToolsLogic != NULL)
			nReg = _ToolsLogic->createNewRegion (rRegion.Name);

		for (i = 0; i < rRegion.VPoints.size(); ++i)
		{
			HTREEITEM newItem = (HTREEITEM)tempItem;
			tempItem--;
			if (_ToolsLogic != NULL)
				newItem = _ToolsLogic->addPoint (nReg, rRegion.VPoints[i].Name.c_str());
			SPrimBuild pB;
			pB.Created = true;
			pB.Type = 0; // Point
			pB.Pos = i;
			pB.PRegion = _PRegions[j];
			_Primitives.insert (map<HTREEITEM, SPrimBuild>::value_type(newItem, pB));
		}

		for (i = 0; i < rRegion.VPaths.size(); ++i)
		{
			HTREEITEM newItem = (HTREEITEM)tempItem;
			tempItem--;
			if (_ToolsLogic != NULL)
				newItem = _ToolsLogic->addPath (nReg, rRegion.VPaths[i].Name.c_str());
			SPrimBuild pB;
			pB.Created = true;
			pB.Type = 1; // Path
			pB.Pos = i;
			pB.PRegion = _PRegions[j];
			_Primitives.insert (map<HTREEITEM, SPrimBuild>::value_type(newItem, pB));
		}

		for (i = 0; i < rRegion.VZones.size(); ++i)
		{
			HTREEITEM newItem = (HTREEITEM)tempItem;
			tempItem--;
			if (_ToolsLogic != NULL)
				newItem = _ToolsLogic->addZone (nReg, rRegion.VZones[i].Name.c_str());
			SPrimBuild pB;
			pB.Created = true;
			pB.Type = 2; // Path
			pB.Pos = i;
			pB.PRegion = _PRegions[j];
			_Primitives.insert (map<HTREEITEM, SPrimBuild>::value_type(newItem, pB));
		}
		if (_ToolsLogic != NULL)
			_ToolsLogic->expandAll (nReg);
	}
}

// ---------------------------------------------------------------------------
bool CBuilderLogic::load (const char *fileName, const char *path)
{
	string sTmp = fileName;
	for (uint32 i = 0; i < _PRegions.size(); ++i)
	if (_PRegions[i]->Name == sTmp)
	{
		_RegionSelected = i;
		updateToolsLogic ();
		if (_Display)
			_Display->OnDraw (NULL);
		return true;
	}

	uint32 nPos = _PRegions.size ();
	_PRegions.push_back (new CPrimRegion);
	_FullNames.push_back("");
	_PRegions[nPos]->Name = "__New_Region__";
	_FullNames[nPos] = path;
	_FullNames[nPos] += "\\";
	_FullNames[nPos] += fileName;
	_RegionSelected = nPos;
	try
	{
		CIFile fileIn;
		fileIn.open (fileName);
		CIXml input;
		input.init (fileIn);
		_PRegions[_RegionSelected]->serial (input);
	}
	catch (Exception& e)
	{
		MessageBox (NULL, e.what(), "Warning", MB_OK);
	}

	updateToolsLogic ();

	if (_Display)
		_Display->OnDraw (NULL);
	return true;
}

// ---------------------------------------------------------------------------
bool CBuilderLogic::save (uint32 nPos, const char *fileName)
{
	COFile file;
	file.open (fileName);
	COXml output;
	output.init (&file, "1.0");
	_PRegions[nPos]->Name = fileName;
	_PRegions[nPos]->serial (output);
	output.flush ();
	file.close ();
	updateToolsLogic ();
	return true;
}

// ---------------------------------------------------------------------------
void CBuilderLogic::autoSaveAll()
{
	for (uint32 i = 0; i < _PRegions.size(); ++i)
	{
		COFile file;
		file.open (_FullNames[i]);
		COXml output;
		output.init (&file, "1.0");
		_PRegions[i]->serial (output);
		output.flush ();
		file.close ();
	}
}

// ---------------------------------------------------------------------------
void CBuilderLogic::newZone ()
{
	uint32 nPos = _PRegions.size ();
	_PRegions.push_back (new CPrimRegion);
	_PRegions[nPos]->Name = "__New_Region__";
	_FullNames.push_back ("");
	_RegionSelected = nPos;
	updateToolsLogic ();
}

// ---------------------------------------------------------------------------
void CBuilderLogic::unload (uint32 pos)
{
	if (_PRegions.size() == 0)
		return;
	delete _PRegions[pos];
	for (uint32 i = pos; i < _PRegions.size()-1; ++i)
	{
		_PRegions[i] = _PRegions[i+1];
		_FullNames[i] = _FullNames[i+1];
	}
	_PRegions.resize (_PRegions.size()-1);
	_FullNames.resize (_FullNames.size()-1);
	updateToolsLogic ();
}

// ---------------------------------------------------------------------------
uint32 CBuilderLogic::getNbZoneRegion ()
{
	return _PRegions.size();
}

// ---------------------------------------------------------------------------
const string &CBuilderLogic::getZoneRegionName (uint32 nPos)
{
	return _PRegions[nPos]->Name;
}


// ---------------------------------------------------------------------------
void CBuilderLogic::insertPoint (uint32 pos, HTREEITEM item, const char *Name, const char *LayerName)
{
	CPrimPoint pp;
	pp.LayerName = LayerName;
	pp.Name = Name;
	pp.Point = CVector(0.0f, 0.0f, 0.0f);
	_PRegions[pos]->VPoints.push_back(pp);
	SPrimBuild pB;
	pB.Type = 0; // Point
	pB.Pos = _PRegions[pos]->VPoints.size()-1;
	pB.PRegion = _PRegions[pos];
	_Primitives.insert (map<HTREEITEM, SPrimBuild>::value_type(item, pB));
}

// ---------------------------------------------------------------------------
void CBuilderLogic::insertPath (uint32 pos, HTREEITEM item, const char *Name, const char *LayerName)
{
	CPrimPath pp;
	pp.LayerName = LayerName;
	pp.Name = Name;
	_PRegions[pos]->VPaths.push_back (pp);
	SPrimBuild pB;
	pB.Type = 1; // Path
	pB.Pos = _PRegions[pos]->VPaths.size()-1;
	pB.PRegion = _PRegions[pos];
	_Primitives.insert (map<HTREEITEM, SPrimBuild>::value_type(item, pB));
}

// ---------------------------------------------------------------------------
void CBuilderLogic::insertZone (uint32 pos, HTREEITEM item, const char *Name, const char *LayerName)
{
	CPrimZone pz;
	pz.LayerName = LayerName;
	pz.Name = Name;
	_PRegions[pos]->VZones.push_back (pz);
	SPrimBuild pB;
	pB.Type = 2; // Zone
	pB.Pos = _PRegions[pos]->VZones.size()-1;
	pB.PRegion = _PRegions[pos];
	_Primitives.insert (map<HTREEITEM, SPrimBuild>::value_type(item, pB));
}

// ---------------------------------------------------------------------------
void CBuilderLogic::del (HTREEITEM item)
{
	map<HTREEITEM, SPrimBuild>::iterator it = _Primitives.find (item);
	if (it != _Primitives.end())
	{
		SPrimBuild &rPB = it->second;
		CPrimRegion &PRegion = *rPB.PRegion;
		// Delete the entry in the document
		uint32 i;
		switch (rPB.Type)
		{
			case 0:
			{
				for (i = rPB.Pos+1; i < PRegion.VPoints.size(); ++i)
					PRegion.VPoints[i-1] = PRegion.VPoints[i];
				PRegion.VPoints.resize (PRegion.VPoints.size()-1);
			}
			break;
			case 1:
			{
				for (i = rPB.Pos+1; i < PRegion.VPaths.size(); ++i)
					PRegion.VPaths[i-1] = PRegion.VPaths[i];
				PRegion.VPaths.resize (PRegion.VPaths.size()-1);
			}
			break;
			case 2:
				for (i = rPB.Pos+1; i < PRegion.VZones.size(); ++i)
					PRegion.VZones[i-1] = PRegion.VZones[i];
				PRegion.VZones.resize (PRegion.VZones.size()-1);
			break;
		}

		map<HTREEITEM, SPrimBuild>::iterator it2 = _Primitives.begin ();
		while (it2 != _Primitives.end())
		{
			SPrimBuild &rPB2 = it2->second;

			if (rPB2.Type == rPB.Type)
				if (rPB2.Pos > rPB.Pos)
					rPB2.Pos -= 1;
				
			++it2;
		}
		// Delete the entry in the map
		_Primitives.erase (it);
	}

	if (_Display)
		_Display->OnDraw (NULL);
}

// ---------------------------------------------------------------------------
void CBuilderLogic::hide (HTREEITEM item)
{
	map<HTREEITEM, SPrimBuild>::iterator it = _Primitives.find (item);
	if (it != _Primitives.end())
	{
		SPrimBuild &rPB = it->second;
		rPB.hidden = !rPB.hidden;
	}
	if (_Display)
		_Display->OnDraw (NULL);
}

// ---------------------------------------------------------------------------
void CBuilderLogic::hideAll (uint32 nPos, sint32 nID, bool bHide)
{
	map<HTREEITEM, SPrimBuild>::iterator it = _Primitives.begin ();
	while (it != _Primitives.end())
	{
		SPrimBuild &rPB = it->second;

		if (rPB.PRegion == _PRegions[nPos])
		{
			if (rPB.Type == nID)
				rPB.hidden = bHide;
		}
		
		++it;
	}
	if (_Display)
		_Display->OnDraw (NULL);
}

// ---------------------------------------------------------------------------
void CBuilderLogic::regionHideAll (uint32 nPos, bool bHide)
{
	map<HTREEITEM, SPrimBuild>::iterator it = _Primitives.begin ();
	while (it != _Primitives.end())
	{
		SPrimBuild &rPB = it->second;

		if (rPB.PRegion == _PRegions[nPos])
			rPB.hidden = bHide;
		
		++it;
	}
	if (_Display)
		_Display->OnDraw (NULL);
}

// ---------------------------------------------------------------------------
const char* CBuilderLogic::getName (HTREEITEM item)
{
	map<HTREEITEM, SPrimBuild>::iterator it = _Primitives.find (item);
	if (it == _Primitives.end())
		return NULL;
	else
	{
		SPrimBuild &rPB = it->second;
		CPrimRegion &PRegion = *rPB.PRegion;

		switch (rPB.Type)
		{
			case 0:
				return PRegion.VPoints[rPB.Pos].Name.c_str();
			break;
			case 1:
				return PRegion.VPaths[rPB.Pos].Name.c_str();
			break;
			case 2:
				return PRegion.VZones[rPB.Pos].Name.c_str();
			break;
		}
		return NULL;
	}
}

// ---------------------------------------------------------------------------
const char* CBuilderLogic::getLayerName (HTREEITEM item)
{
	map<HTREEITEM, SPrimBuild>::iterator it = _Primitives.find (item);
	if (it == _Primitives.end())
		return NULL;
	else
	{
		SPrimBuild &rPB = it->second;
		CPrimRegion &PRegion = *rPB.PRegion;

		switch (rPB.Type)
		{
			case 0:
				return PRegion.VPoints[rPB.Pos].LayerName.c_str();
			break;
			case 1:
				return PRegion.VPaths[rPB.Pos].LayerName.c_str();
			break;
			case 2:
				return PRegion.VZones[rPB.Pos].LayerName.c_str();
			break;
		}
		return NULL;
	}
}

// ---------------------------------------------------------------------------
bool CBuilderLogic::isHidden (HTREEITEM item)
{
	map<HTREEITEM, SPrimBuild>::iterator it = _Primitives.find (item);
	if (it == _Primitives.end())
	{
		return true;
	}
	else
	{
		SPrimBuild &rPB = it->second;
		return rPB.hidden;
	}
}

// ---------------------------------------------------------------------------
void CBuilderLogic::setName (HTREEITEM item, const char* pStr)
{
	map<HTREEITEM, SPrimBuild>::iterator it = _Primitives.find (item);
	if (it == _Primitives.end())
		return;
	else
	{
		SPrimBuild &rPB = it->second;
		CPrimRegion &PRegion = *rPB.PRegion;

		switch (rPB.Type)
		{
			case 0:
				PRegion.VPoints[rPB.Pos].Name = pStr;
			break;
			case 1:
				PRegion.VPaths[rPB.Pos].Name = pStr;
			break;
			case 2:
				PRegion.VZones[rPB.Pos].Name = pStr;
			break;
		}
	}
}

// ---------------------------------------------------------------------------
void CBuilderLogic::setLayerName (HTREEITEM item, const char* pStr)
{
	map<HTREEITEM, SPrimBuild>::iterator it = _Primitives.find (item);
	if (it == _Primitives.end())
		return;
	else
	{
		SPrimBuild &rPB = it->second;
		CPrimRegion &PRegion = *rPB.PRegion;

		switch (rPB.Type)
		{
			case 0:
				PRegion.VPoints[rPB.Pos].LayerName = pStr;
			break;
			case 1:
				PRegion.VPaths[rPB.Pos].LayerName = pStr;
			break;
			case 2:
				PRegion.VZones[rPB.Pos].LayerName = pStr;
			break;
		}
	}
}

// ---------------------------------------------------------------------------
HTREEITEM CBuilderLogic::getSelPB ()
{
	return _ItemSelected;
}

// ---------------------------------------------------------------------------
void CBuilderLogic::setSelPB (HTREEITEM item)
{
	_ItemSelected = NULL;
	map<HTREEITEM, SPrimBuild>::iterator it = _Primitives.find (item);
	if (it != _Primitives.end())
		_ItemSelected = item;

	if (_Display)
		_Display->OnDraw (NULL);
}

// ---------------------------------------------------------------------------
void CBuilderLogic::createVertexOnSelPB (CVector &v, uint32 pos)
{
	map<HTREEITEM, SPrimBuild>::iterator it = _Primitives.find (_ItemSelected);
	if (it == _Primitives.end())
		return;

	SPrimBuild &rPB = it->second;
	CPrimRegion &PRegion = *rPB.PRegion;
	switch (rPB.Type)
	{
		case 0:
			if (!rPB.Created)
			{
				PRegion.VPoints[rPB.Pos].Point = v;
				rPB.Created = true;
			}
		break;
		case 1:
			if (!rPB.Created)
			{
				rPB.Created = true;
			}
			PRegion.VPaths[rPB.Pos].VPoints.push_back(v);
		break;
		case 2:
			if (!rPB.Created)
			{
				rPB.Created = true;
			}
			PRegion.VZones[rPB.Pos].VPoints.push_back(v);
		break;
	}
	
}

// ---------------------------------------------------------------------------
bool CBuilderLogic::selectVertexOnSelPB (CVector &selMin, CVector &selMax)
{
	_VertexSelected = -1;

	map<HTREEITEM, SPrimBuild>::iterator it = _Primitives.find (_ItemSelected);
	if (it == _Primitives.end())
		return false;

	SPrimBuild &rPB = it->second;
	CPrimRegion &PRegion = *rPB.PRegion;

	if (!rPB.Created)
		return false;

	CVector *pV = NULL;
	uint32 nNbV = 0;
	switch (rPB.Type)
	{
		case 0:
			nNbV = 1;
			pV = &(PRegion.VPoints[rPB.Pos].Point);
		break;
		case 1:
			nNbV = PRegion.VPaths[rPB.Pos].VPoints.size();
			pV = &(PRegion.VPaths[rPB.Pos].VPoints[0]);
		break;
		case 2:
			nNbV = PRegion.VZones[rPB.Pos].VPoints.size();
			pV = &(PRegion.VZones[rPB.Pos].VPoints[0]);
		break;
	}

	for (uint32 i = 0; i < nNbV; ++i)
	{
		if ((pV[i].x > selMin.x) && (pV[i].x < selMax.x) &&
			(pV[i].y > selMin.y) && (pV[i].y < selMax.y))
		{
			_VertexSelected = i;
			break;
		}
	}
	if (_VertexSelected == -1)
		return false;
	return true;
}

// ---------------------------------------------------------------------------
void CBuilderLogic::setSelVertexOnSelPB (NLMISC::CVector &v)
{
	if ((_VertexSelected == -1) || (_ItemSelected == NULL))
		return;
	map<HTREEITEM, SPrimBuild>::iterator it = _Primitives.find (_ItemSelected);
	if (it == _Primitives.end())
		return;

	SPrimBuild &rPB = it->second;
	CPrimRegion &PRegion = *rPB.PRegion;
	
	switch (rPB.Type)
	{
		case 0:
			PRegion.VPoints[rPB.Pos].Point = v;
		break;
		case 1:
			PRegion.VPaths[rPB.Pos].VPoints[_VertexSelected] = v;
		break;
		case 2:
			PRegion.VZones[rPB.Pos].VPoints[_VertexSelected] = v;
		break;
	}
}

// ---------------------------------------------------------------------------
void CBuilderLogic::delSelVertexOnSelPB ()
{
	if ((_VertexSelected == -1) || (_ItemSelected == NULL))
		return;
	map<HTREEITEM, SPrimBuild>::iterator it = _Primitives.find (_ItemSelected);
	if (it == _Primitives.end())
		return;

	SPrimBuild &rPB = it->second;
	CPrimRegion &PRegion = *rPB.PRegion;

	uint32 i;
	switch (rPB.Type)
	{
		case 0:
			rPB.Created = false;
		break;
		case 1:
			if (PRegion.VPaths[rPB.Pos].VPoints.size() == 1)
				rPB.Created = false;

			for (i = _VertexSelected+1; i < PRegion.VPaths[rPB.Pos].VPoints.size(); ++i)
				PRegion.VPaths[rPB.Pos].VPoints[i-1] = PRegion.VPaths[rPB.Pos].VPoints[i];
			PRegion.VPaths[rPB.Pos].VPoints.resize (PRegion.VPaths[rPB.Pos].VPoints.size()-1);
		break;
		case 2:
			if (PRegion.VZones[rPB.Pos].VPoints.size() == 1)
				rPB.Created = false;

			for (i = _VertexSelected+1; i < PRegion.VZones[rPB.Pos].VPoints.size(); ++i)
				PRegion.VZones[rPB.Pos].VPoints[i-1] = PRegion.VZones[rPB.Pos].VPoints[i];
			PRegion.VZones[rPB.Pos].VPoints.resize (PRegion.VZones[rPB.Pos].VPoints.size()-1);
		break;
	}

}

// ---------------------------------------------------------------------------
void CBuilderLogic::render (CVector &viewMin, CVector &viewMax)
{
	// Accelerate rendering with vertex buffer
	CVertexBuffer VB, VBSel;
	CPrimitiveBlock PB, PBSel;
	CMaterial Mat;
	CRGBA Col, ColSel;

	Col = CRGBA(255, 255, 255, 192);
	ColSel = CRGBA(255, 0, 0, 192);

	Mat.initUnlit ();
	Mat.setSrcBlend(CMaterial::srcalpha);
	Mat.setDstBlend(CMaterial::invsrcalpha);
	Mat.setBlend (true);
	VB.setVertexFormat (CVertexBuffer::PositionFlag);
	VBSel.setVertexFormat (CVertexBuffer::PositionFlag);
	
	// Parse the map
	map<HTREEITEM, SPrimBuild>::iterator it = _Primitives.begin();
	while (it != _Primitives.end())
	{
		HTREEITEM curItem = it->first;
		SPrimBuild &curPB = it->second;
		CPrimRegion &PRegion = *curPB.PRegion;

		CRGBA col;
		uint32 i;
		
		// If not created do not display or hidden
		if ((!curPB.Created)||(curPB.hidden))
		{
			++it;
			continue;
		}

		CVector *pVec = NULL;
		uint32 nNbVec = 0;

		// Clip

		if (curPB.Type == 0) // Point
		{
			pVec = &PRegion.VPoints[curPB.Pos].Point;
			nNbVec = 1;
		}

		if (curPB.Type == 1) // Path
		{
			pVec = &PRegion.VPaths[curPB.Pos].VPoints[0];
			nNbVec = PRegion.VPaths[curPB.Pos].VPoints.size();
		}

		if (curPB.Type == 2) // Zone
		{
			pVec = &PRegion.VZones[curPB.Pos].VPoints[0];
			nNbVec = PRegion.VZones[curPB.Pos].VPoints.size();
		}

		if (clip(pVec, nNbVec, viewMin, viewMax))
		{
			++it;
			continue;
		}	
			

		// Draw all interiors
		
		if (curPB.Type == 2) // For Zones
		{
			vector<sint32> vRef;
			uint32 nStart, VBStart = VBSel.getNumVertices(), PBStart = PBSel.getNumTri();
			vRef.resize(nNbVec);
			for(i = 0; i < vRef.size(); ++i)
				vRef[i] = i;

			nStart = 0;
			while (vRef.size() > 2)
			{
				// Is triangle (nStart, nStart+1, nStart+2) back face ?
				sint32 nP1 = vRef[nStart];
				sint32 nP2 = vRef[(nStart+1)%vRef.size()];
				sint32 nP3 = vRef[(nStart+2)%vRef.size()];
				CVector pos1 = pVec[nP1];
				CVector pos2 = pVec[nP2];
				CVector pos3 = pVec[nP3];
				if (((pos2.x-pos1.x) * (pos3.y-pos1.y) - (pos2.y-pos1.y) * (pos3.x-pos1.x)) < 0.0f)
				{
					// Yes -> next triangle
					nStart++;
					//nlassert(nStart != vRef.size());
					if (nStart == vRef.size())
					{
						VBSel.setNumVertices (VBStart);
						PBSel.setNumTri(PBStart);
						break;
					}
					continue;
				}
				// Is triangle (nStart, nStart+1, nStart+2) contains the other point ?
				bool bInside = false;
				for (i = 0; i < vRef.size(); ++i)
				{
					if ((vRef[i] != nP1) && (vRef[i] != nP2) && (vRef[i] != nP3))
					{
						if (isInTriangleOrEdge(	pVec[vRef[i]].x, pVec[vRef[i]].y, 
												pos1.x, pos1.y,
												pos2.x, pos2.y,
												pos3.x, pos3.y ))
						{
							bInside = true;
							break;
						}
					}
				}
				if (bInside)
				{
					// Yes -> next triangle
					nStart++;
					//nlassert(nStart != vRef.size());
					if (nStart == vRef.size())
					{
						VBSel.setNumVertices (VBStart);
						PBSel.setNumTri(PBStart);
						break;
					}
					continue;
				}

				// Draw the triangle
				convertToScreen (&pos1, 1, viewMin, viewMax);
				convertToScreen (&pos2, 1, viewMin, viewMax);
				convertToScreen (&pos3, 1, viewMin, viewMax);
				if (curItem == _ItemSelected)
					renderDrawTriangle(pos1, pos2, pos3, &VBSel, &PBSel);
				else
					renderDrawTriangle(pos1, pos2, pos3, &VB, &PB);
				
				// Erase the point in the middle
				for (i = 1+((nStart+1)%vRef.size()); i < vRef.size(); ++i)
					vRef[i-1] = vRef[i];
				vRef.resize (vRef.size()-1);
				nStart = 0;
			}
		}

		// Draw all lines
		
		if ((curPB.Type == 1) || (curPB.Type == 2)) // For Pathes and Zones
		for (i = 0; i < nNbVec-1; ++i)
		{
			CVector pos = pVec[i];
			CVector pos2 = pVec[i+1];

			convertToScreen (&pos, 1, viewMin, viewMax);
			convertToScreen (&pos2, 1, viewMin, viewMax);
			if (curItem == _ItemSelected)
				renderDrawLine (pos, pos2, &VBSel, &PBSel);
			else
				renderDrawLine (pos, pos2, &VB, &PB);
		}

		// Draw all points

		for (i = 0; i < nNbVec; ++i)		// For Points, Pathes and Zones
		{
			CVector pos = pVec[i];
			convertToScreen (&pos, 1, viewMin, viewMax);
			if (curItem == _ItemSelected)
			{
				if (_VertexSelected == (sint32)i)
				{
					col = CRGBA (255, 255, 0, 192);
					renderDrawPoint (pos, col, NULL, NULL);
				}
				else
				{
					renderDrawPoint (pos, col, &VBSel, &PBSel);
				}
			}
			else
			{
				renderDrawPoint (pos, col, &VB, &PB);
			}
		}

		++it;
	}

	// Flush the Vertex Buffer
	
	CMatrix mtx;
	mtx.identity();
	CNELU::Driver->setupViewport (CViewport());
	CNELU::Driver->setupViewMatrix (mtx);
	CNELU::Driver->setupModelMatrix (mtx);
	CNELU::Driver->setFrustum (0.f, 1.f, 0.f, 1.f, -1.f, 1.f, false);
	Mat.setColor (Col);
	CNELU::Driver->activeVertexBuffer(VB);
	CNELU::Driver->render(PB, Mat);
	Mat.setColor (ColSel);
	CNELU::Driver->activeVertexBuffer(VBSel);
	CNELU::Driver->render(PBSel, Mat);
}

// ---------------------------------------------------------------------------
void CBuilderLogic::renderDrawPoint (CVector &pos, CRGBA &col, CVertexBuffer *pVB, CPrimitiveBlock *pPB)
{
	if (pVB == NULL)
	{
		CDRU::drawLine (pos.x-0.01f, pos.y, pos.x+0.01f, pos.y, *CNELU::Driver, col);
		CDRU::drawLine (pos.x, pos.y-0.01f, pos.x, pos.y+0.01f, *CNELU::Driver, col);
	}
	else
	{
		sint32 nVBPos = pVB->getNumVertices();
		pVB->setNumVertices (nVBPos+4);
		pVB->setVertexCoord (nVBPos+0, pos.x-0.01f, pos.z, pos.y);
		pVB->setVertexCoord (nVBPos+1, pos.x+0.01f, pos.z, pos.y);
		pVB->setVertexCoord (nVBPos+2, pos.x, pos.z, pos.y-0.01f);
		pVB->setVertexCoord (nVBPos+3, pos.x, pos.z, pos.y+0.01f);
		sint32 nPBPos = pPB->getNumLine();
		pPB->setNumLine (nPBPos+2);
		pPB->setLine (nPBPos+0, nVBPos+0, nVBPos+1);
		pPB->setLine (nPBPos+1, nVBPos+2, nVBPos+3);
	}
}

// ---------------------------------------------------------------------------
void CBuilderLogic::renderDrawLine (CVector &pos, CVector &pos2, CVertexBuffer *pVB, CPrimitiveBlock *pPB)
{
	sint32 nVBPos = pVB->getNumVertices();
	pVB->setNumVertices (nVBPos+2);
	pVB->setVertexCoord (nVBPos+0, pos.x, pos.z, pos.y);
	pVB->setVertexCoord (nVBPos+1, pos2.x, pos2.z, pos2.y);
	sint32 nPBPos = pPB->getNumLine();
	pPB->setNumLine (nPBPos+1);
	pPB->setLine (nPBPos+0, nVBPos+0, nVBPos+1);
}

// ---------------------------------------------------------------------------
void CBuilderLogic::renderDrawTriangle (CVector &pos, CVector &pos2, CVector &pos3, CVertexBuffer *pVB, CPrimitiveBlock *pPB)
{
	sint32 nVBPos = pVB->getNumVertices();
	pVB->setNumVertices (nVBPos+3);
	pVB->setVertexCoord (nVBPos+0, pos.x, pos.z, pos.y);
	pVB->setVertexCoord (nVBPos+1, pos2.x, pos2.z, pos2.y);
	pVB->setVertexCoord (nVBPos+2, pos3.x, pos3.z, pos3.y);
	sint32 nPBPos = pPB->getNumTri();
	pPB->setNumTri (nPBPos+1);
	pPB->setTri (nPBPos+0, nVBPos+0, nVBPos+1, nVBPos+2);
}

// ---------------------------------------------------------------------------
bool CBuilderLogic::clip (CVector *pVec, uint32 nNbVec, CVector &viewMin, CVector &viewMax)
{
	uint32 i;
	for (i = 0; i < nNbVec; ++i)
		if (pVec[i].x > viewMin.x)
			break;
	if (i == nNbVec)
		return true; // Entirely clipped

	for (i = 0; i < nNbVec; ++i)
		if (pVec[i].x < viewMax.x)
			break;
	if (i == nNbVec)
		return true;

	for (i = 0; i < nNbVec; ++i)
		if (pVec[i].y > viewMin.y)
			break;
	if (i == nNbVec)
		return true;

	for (i = 0; i < nNbVec; ++i)
		if (pVec[i].y < viewMax.y)
			break;
	if (i == nNbVec)
		return true;
	return false; // Not entirely clipped
}

// ---------------------------------------------------------------------------
void CBuilderLogic::convertToScreen (CVector* pVec, sint nNbVec, CVector &viewMin, CVector &viewMax)
{
	for (sint i = 0; i < nNbVec; ++i)
	{
		pVec[i].x = (pVec[i].x-viewMin.x)/(viewMax.x-viewMin.x);
		pVec[i].y = (pVec[i].y-viewMin.y)/(viewMax.y-viewMin.y);
		pVec[i].z = 0.0f;
	}
}

// ---------------------------------------------------------------------------
bool CBuilderLogic::isInTriangleOrEdge(	double x, double y, 
												double xt1, double yt1, 
												double xt2, double yt2, 
												double xt3, double yt3 )
{
	// Test vector T1X and T1T2
	double sign1 = ((xt2-xt1)*(y-yt1) - (yt2-yt1)*(x-xt1));
	// Test vector T2X and T2T3
	double sign2 = ((xt3-xt2)*(y-yt2) - (yt3-yt2)*(x-xt2));
	// Test vector T3X and T3T1
	double sign3 = ((xt1-xt3)*(y-yt3) - (yt1-yt3)*(x-xt3));
	if( (sign1 <= 0.0)&&(sign2 <= 0.0)&&(sign3 <= 0.0) )
		return true;
	if( (sign1 >= 0.0)&&(sign2 >= 0.0)&&(sign3 >= 0.0) )
		return true;
	return false;
}
