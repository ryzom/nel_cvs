/** \file landscapeig_manager.cpp
 * <File description>
 *
 * $Id: landscapeig_manager.cpp,v 1.7 2002/05/24 16:28:49 besson Exp $
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
#include "nel/3d/u_scene.h"
#include "nel/3d/u_instance_group.h"
#include "nel/misc/common.h"
#include "nel/misc/debug.h"
#include "nel/misc/path.h"
// std.
#include <fstream>


using namespace NLMISC;
using namespace std;


namespace NL3D 
{


// ***************************************************************************
CLandscapeIGManager::CInstanceGroupElement::CInstanceGroupElement(UInstanceGroup *ig, const char *fileName)
{
	Ig = ig;
	AddedToScene = false;
	if (fileName != NULL)
		FileName = fileName;
}

// ***************************************************************************
void	CLandscapeIGManager::CInstanceGroupElement::release()
{
	delete Ig;
	Ig= NULL;
}


// ***************************************************************************
CLandscapeIGManager::CLandscapeIGManager()
{
	_Scene=NULL;
}
// ***************************************************************************
CLandscapeIGManager::~CLandscapeIGManager()
{
	// reset should have been called.
	if(_Scene!=NULL)
		throw Exception("CLandscapeIGManager not reseted");
}
// ***************************************************************************
void	CLandscapeIGManager::initIG(UScene *scene, const std::string &igDesc)
{
	nlassert(scene);
	_Scene= scene;

	// Load the file.
	if(igDesc=="")
		return;
	string igFile = CPath::lookup(igDesc);
	ifstream file(igFile.c_str(), ios::in);

	// if loading ok.
	if(file.is_open())
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
					// add it to the map.
					string	tokId= token;
					strupr(tokId);
					_ZoneInstanceGroupMap[tokId]= CInstanceGroupElement(ig, token);
				}
			}
		}

		file.close();
	}
}
// ***************************************************************************
UInstanceGroup *CLandscapeIGManager::loadZoneIG(const std::string &name)
{
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
	for(uint i=0; i<names.size(); i++)
	{
		unloadZoneIG(names[i]);
	}
}

// ***************************************************************************
void	CLandscapeIGManager::unloadZoneIG(const std::string &name)
{
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
	std::string		ret;
	ret= name + ".ig";
	strupr(ret);

	return ret;
}


// ***************************************************************************
void	CLandscapeIGManager::reset()
{
	while( _ZoneInstanceGroupMap.begin() != _ZoneInstanceGroupMap.end() )
	{
		string	name= _ZoneInstanceGroupMap.begin()->first;
		// first remove from scene
		unloadZoneIG( name.substr(0, name.find('.')) );

		// then delete this entry.
		_ZoneInstanceGroupMap.begin()->second.release();
		_ZoneInstanceGroupMap.erase(_ZoneInstanceGroupMap.begin());
	}

	_Scene=NULL;
}


// ***************************************************************************
void	CLandscapeIGManager::reloadAllIgs()
{
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
		string	tokId= token;
		strupr(tokId);
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
	dest.clear();
	dest.reserve(_ZoneInstanceGroupMap.size());
	// add the instances
	for(TZoneInstanceGroupMap::const_iterator it = _ZoneInstanceGroupMap.begin(); it != _ZoneInstanceGroupMap.end(); ++it)
	{			
		dest.push_back(it->second.Ig);			
	}
}



} // NL3D
