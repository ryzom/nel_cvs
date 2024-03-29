/** \file landscapeig_manager.cpp
 * TODO: File description
 *
 * $Id: landscapeig_manager.cpp,v 1.18 2005/02/22 10:19:10 besson Exp $
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

#include "std3d.h"

#include "nel/3d/landscapeig_manager.h"
#include "scene_user.h"
#include "instance_group_user.h"
#include "shape.h"
#include "nel/misc/common.h"
#include "nel/misc/debug.h"
#include "nel/misc/path.h"
#include "nel/misc/file.h"
#include "nel/misc/hierarchical_timer.h"

// std.
#include <fstream>


using namespace NLMISC;
using namespace std;

H_AUTO_DECL ( NL3D_Load_Zone_IG )
H_AUTO_DECL ( NL3D_Unload_Zone_IG )

#define	NL3D_HAUTO_LAND_MNGR_LOAD_ZONEIG	H_AUTO_USE( NL3D_Load_Zone_IG )
#define	NL3D_HAUTO_LAND_MNGR_UNLOAD_ZONEIG	H_AUTO_USE( NL3D_Unload_Zone_IG )

namespace NL3D 
{


// ***************************************************************************
CLandscapeIGManager::CInstanceGroupElement::CInstanceGroupElement(UInstanceGroup *ig, const char *fileName)
{
	NL3D_MEM_LANDSCAPE_IG
	Ig = ig;
	AddedToScene = false;
	if (fileName != NULL)
		FileName = fileName;
}

// ***************************************************************************
void	CLandscapeIGManager::CInstanceGroupElement::release()
{
	NL3D_MEM_LANDSCAPE_IG
	delete Ig;
	Ig= NULL;
}


// ***************************************************************************
CLandscapeIGManager::CLandscapeIGManager()
{
	NL3D_MEM_LANDSCAPE_IG
	_Scene=NULL;
}
// ***************************************************************************
CLandscapeIGManager::~CLandscapeIGManager()
{
	NL3D_MEM_LANDSCAPE_IG
	// reset should have been called.
	if(_Scene!=NULL)
		throw Exception("CLandscapeIGManager not reseted");
}
// ***************************************************************************
void	CLandscapeIGManager::initIG(UScene *scene, const std::string &igDesc, UDriver *driver, uint selectedTexture, 
									NLMISC::IProgressCallback *callBack)
{
	NL3D_MEM_LANDSCAPE_IG
	nlassert(scene);
	_Scene= scene;

	// Load the file.
	if(igDesc.empty())
		return;

	string igFile = CPath::lookup(igDesc);

	//ifstream file(igFile.c_str(), ios::in);

	CIFile file;

	// Shape to add should be empty !
	nlassert(_ShapeAdded.empty ());
	
	// if loading ok.
	//if(file.is_open())
	if (file.open (igFile))
	{
		char tmpBuff[260];
		char delimiterBox[] = "\t";
		// While the end of the file is not reached.
		while(!file.eof())
		{
			// Get a line
			file.getline(tmpBuff, 260);
			char *token = strtok(tmpBuff, delimiterBox);
			// create the instance group.
			if(token != NULL)
			{
				if( _ZoneInstanceGroupMap.find(token)!=_ZoneInstanceGroupMap.end() )
					throw Exception("CLandscapeIGManager::initIG() found 2 igs with same name in %s", igFile.c_str());
				else
				{
					// create the instanceGroup.
					UInstanceGroup	*ig = UInstanceGroup::createInstanceGroup(token);
					if (ig)
					{
						// add it to the map.
						string	tokId= toUpper(string(token));
						_ZoneInstanceGroupMap[tokId]= CInstanceGroupElement(ig, token);

						// Add a reference on the shapes
						CInstanceGroup &_ig = static_cast<CInstanceGroupUser*>(ig)->getInternalIG();
						CScene &_scene = static_cast<CSceneUser*>(scene)->getScene();
						uint i;
						for (i=0; i<_ig.getNumInstance(); i++)
						{
							// Get the instance name
							string shapeName;
							_ig.getShapeName(i, shapeName);
							if (!shapeName.empty ())
							{
								if (toLower(CFile::getExtension(shapeName)) != "pacs_prim")
								{
									// Insert a new shape ?
									if (_ShapeAdded.find(shapeName) == _ShapeAdded.end())
									{
										// Shape present ?
										CShapeBank *shapeBank = _scene.getShapeBank();
										IShape *shape = NULL;
										if (shapeBank->getPresentState (shapeName) == CShapeBank::NotPresent)
											shapeBank->load (shapeName);
										if (shapeBank->getPresentState (shapeName) == CShapeBank::Present)
											shape = shapeBank->addRef(shapeName);

										// Shape loaded ?
										if (shape)
										{
											// Insert the shape
											CSmartPtr<IShape> *smartPtr = new CSmartPtr<IShape>;
											*smartPtr = shape;
											_ShapeAdded.insert (TShapeMap::value_type (shapeName, smartPtr));

											// Flush the shape
											IDriver	*_driver = static_cast<CDriverUser*>(driver)->getDriver();
											shape->flushTextures(*_driver, selectedTexture);
										}
									}
								}
							}
						}
					}
					else
					{
						nlwarning ("CLandscapeIGManager::initIG() Can't load instance group '%s' in '%s'", token, igFile.c_str());
					}
				}
			}
		}
		file.close();
	}
	else
	{
		nlwarning ("Couldn't load '%s'", igFile.c_str());
	}
}
// ***************************************************************************
UInstanceGroup *CLandscapeIGManager::loadZoneIG(const std::string &name)
{
	NL3D_MEM_LANDSCAPE_IG
	NL3D_HAUTO_LAND_MNGR_LOAD_ZONEIG

	if(name=="") 
		return NULL;

	// try to find this InstanceGroup.
	ItZoneInstanceGroupMap	it;
	it= _ZoneInstanceGroupMap.find( translateName(name) );

	// if found.
	if( it!= _ZoneInstanceGroupMap.end() )
	{
		// if not already added to the scene.
		if( !it->second.AddedToScene )
		{
			// add to the scene.
			if (it->second.Ig != NULL)
			{
				it->second.Ig->addToScene(*_Scene);
				it->second.AddedToScene= true;
			}
		}
		return it->second.Ig;
	}
	else
	{
		return NULL;
	}
}
// ***************************************************************************
void	CLandscapeIGManager::loadArrayZoneIG(const std::vector<std::string> &names, std::vector<UInstanceGroup *> *dest /*= NULL*/)
{
	NL3D_MEM_LANDSCAPE_IG
	if (dest)
	{
		dest->clear();
		dest->reserve(names.size());
	}
	for(uint i=0; i<names.size(); i++)
	{
		UInstanceGroup *ig = loadZoneIG(names[i]);
		if (dest && ig)
		{
			dest->push_back(ig);
		}
	}
}

// ***************************************************************************
void	CLandscapeIGManager::unloadArrayZoneIG(const std::vector<std::string> &names)
{
	NL3D_MEM_LANDSCAPE_IG
	for(uint i=0; i<names.size(); i++)
	{
		unloadZoneIG(names[i]);
	}
}

// ***************************************************************************
void	CLandscapeIGManager::unloadZoneIG(const std::string &name)
{
	NL3D_MEM_LANDSCAPE_IG
	NL3D_HAUTO_LAND_MNGR_UNLOAD_ZONEIG
	if(name=="")
		return;

	// try to find this InstanceGroup.
	ItZoneInstanceGroupMap	it;
	it= _ZoneInstanceGroupMap.find( translateName(name) );

	// if found.
	if( it!= _ZoneInstanceGroupMap.end() )
	{
		// if really added to the scene.
		if( it->second.AddedToScene )
		{
			// remove from the scene.
			it->second.Ig->removeFromScene(*_Scene);
			it->second.AddedToScene= false;
		}
	}
}

// ***************************************************************************
bool	CLandscapeIGManager::isIGAddedToScene(const std::string &name) const
{
	NL3D_MEM_LANDSCAPE_IG
	if(name=="")
		return false;

	// try to find this InstanceGroup.
	ConstItZoneInstanceGroupMap	it;
	it= _ZoneInstanceGroupMap.find( translateName(name) );

	// if found.
	if( it!= _ZoneInstanceGroupMap.end() )
		return	it->second.AddedToScene;
	else
		return false;
}

// ***************************************************************************
UInstanceGroup	*CLandscapeIGManager::getIG(const std::string &name) const
{
	NL3D_MEM_LANDSCAPE_IG
	if(name=="")
		return NULL;

	// try to find this InstanceGroup.
	ConstItZoneInstanceGroupMap	it;
	it= _ZoneInstanceGroupMap.find( translateName(name) );

	// if found.
	if( it!= _ZoneInstanceGroupMap.end() )
		return it->second.Ig;
	else
		return NULL;
}


// ***************************************************************************
std::string		CLandscapeIGManager::translateName(const std::string &name) const
{
	NL3D_MEM_LANDSCAPE_IG
	std::string		ret;
	ret= toUpper(name + ".ig");
	return ret;
}


// ***************************************************************************
void	CLandscapeIGManager::reset()
{
	NL3D_MEM_LANDSCAPE_IG
	while( _ZoneInstanceGroupMap.begin() != _ZoneInstanceGroupMap.end() )
	{
		string	name= _ZoneInstanceGroupMap.begin()->first;
		// first remove from scene
		unloadZoneIG( name.substr(0, name.find('.')) );

		// then delete this entry.
		_ZoneInstanceGroupMap.begin()->second.release();
		_ZoneInstanceGroupMap.erase(_ZoneInstanceGroupMap.begin());
	}

	// For all shape reference
	TShapeMap::iterator ite = _ShapeAdded.begin ();
	while (ite != _ShapeAdded.end())
	{
		// Unreference shape
		CScene &_scene = static_cast<CSceneUser*>(_Scene)->getScene();
		CSmartPtr<IShape> *smartPtr = (CSmartPtr<IShape> *)(ite->second);
		IShape *shapeToRelease = *smartPtr;
		*smartPtr = NULL;
		_scene.getShapeBank()->release(shapeToRelease);
		delete smartPtr;
	
		// Next
		ite++;
	}
	_ShapeAdded.clear ();
	
	_Scene=NULL;
}


// ***************************************************************************
void	CLandscapeIGManager::reloadAllIgs()
{
	NL3D_MEM_LANDSCAPE_IG
	vector<std::string>		bkupIgFileNameList;
	vector<bool>			bkupIgAddedToScene;

	// First, erase all igs.
	while( _ZoneInstanceGroupMap.begin() != _ZoneInstanceGroupMap.end() )
	{
		string	name= _ZoneInstanceGroupMap.begin()->first;

		// bkup the state of this ig.
		bkupIgFileNameList.push_back(_ZoneInstanceGroupMap.begin()->second.FileName);
		bkupIgAddedToScene.push_back(_ZoneInstanceGroupMap.begin()->second.AddedToScene);

		// first remove from scene
		unloadZoneIG( name.substr(0, name.find('.')) );

		// then delete this entry.
		_ZoneInstanceGroupMap.begin()->second.release();
		_ZoneInstanceGroupMap.erase(_ZoneInstanceGroupMap.begin());
	}

	// Then reload all Igs.
	for(uint i=0; i<bkupIgFileNameList.size(); i++)
	{
		const	char	*token= bkupIgFileNameList[i].c_str();
		UInstanceGroup	*ig = UInstanceGroup::createInstanceGroup(token);
		// add it to the map.
		string	tokId= toUpper(token);
		_ZoneInstanceGroupMap[tokId]= CInstanceGroupElement(ig, token);

		// If was addedToScene before, re-add to scene now.
		if(bkupIgAddedToScene[i])
		{
			loadZoneIG( tokId.substr(0, tokId.find('.')) );
		}
	}
}


// ***************************************************************************
void CLandscapeIGManager::getAllIG(std::vector<UInstanceGroup *> &dest) const
{	
	NL3D_MEM_LANDSCAPE_IG
	dest.clear();
	dest.reserve(_ZoneInstanceGroupMap.size());
	// add the instances
	for(TZoneInstanceGroupMap::const_iterator it = _ZoneInstanceGroupMap.begin(); it != _ZoneInstanceGroupMap.end(); ++it)
	{			
		dest.push_back(it->second.Ig);			
	}
}

// ***************************************************************************
void CLandscapeIGManager::getAllIGWithNames(std::vector<std::pair<UInstanceGroup *, std::string> > &dest) const
{
	NL3D_MEM_LANDSCAPE_IG
	dest.clear();
	dest.reserve(_ZoneInstanceGroupMap.size());
	// add the instances
	for(TZoneInstanceGroupMap::const_iterator it = _ZoneInstanceGroupMap.begin(); it != _ZoneInstanceGroupMap.end(); ++it)
	{			
		dest.push_back(std::make_pair(it->second.Ig, it->second.FileName));			
	}
}


} // NL3D
