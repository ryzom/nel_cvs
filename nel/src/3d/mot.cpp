/** \file mot.cpp
 * The Model / Observer / Traversal  (MOT) paradgim.
 *
 * $Id: mot.cpp,v 1.12 2001/04/24 10:22:22 berenguier Exp $
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

#include "nel/3d/mot.h"
#include "nel/misc/debug.h"
#include "nel/misc/stream.h"
#include <algorithm>
#include <list>
using namespace std;
using namespace NLMISC;


namespace	NL3D
{


// ***************************************************************************
// ***************************************************************************
// CMOT static.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
set<CMOT::CModelEntry>		CMOT::RegModels;
set<CMOT::CObsEntry>		CMOT::RegObservers;

// ***************************************************************************
void	CMOT::registerModel(const CClassId &idModel, const CClassId &idModelBase, IModel* (*creator)())
{
	nlassert(idModel!=CClassId::Null);
	nlassert(creator);
	// idModelBase may be Null...

	CModelEntry		e;
	e.BaseModelId= idModelBase;
	e.ModelId= idModel;
	e.Creator= creator;

	// Insert/replace e.
	RegModels.erase(e);
	RegModels.insert(e);
}
// ***************************************************************************
void	CMOT::registerObs(const CClassId &idTrav, const CClassId &idModel, IObs* (*creator)())
{
	nlassert(idTrav!=CClassId::Null);
	nlassert(idModel!=CClassId::Null);
	nlassert(creator);

	CObsEntry		e;
	e.ModelId= idModel;
	e.TravId= idTrav;
	e.Creator= creator;

	// Insert/replace e.
	RegObservers.erase(e);
	RegObservers.insert(e);
}


// ***************************************************************************
// ***************************************************************************
// CMOT.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
CMOT::CMOT()
{
}
// ***************************************************************************
CMOT::~CMOT()
{
	release();
}

// ***************************************************************************
void	CMOT::addTrav(ITrav *v)
{
	nlassert(v);
	CClassId	idTrav= v->getClassId();
	nlassert(idTrav!=CClassId::Null);

	CTravEntry	e;
	e.TravId= idTrav;
	e.Trav= v;

	Traversals.push_back(e);
}
// ***************************************************************************
ITrav	*CMOT::getTrav(const CClassId &idTrav) const 
{
	std::vector<CTravEntry>::const_iterator	it;

	for(it= Traversals.begin(); it!= Traversals.end(); it++)
	{
		if(idTrav== (*it).TravId)
			return (*it).Trav;
	}

	return NULL;
}
// ***************************************************************************
void	CMOT::release()
{
	// First, release all the models.
	set<IModel*>::iterator	it;
	it= Models.begin();
	while( it!=Models.end())
	{
		deleteModel(*it);
		it= Models.begin();
	}

	// Then release the traversals ptrs.
	Traversals.clear();
}


// ***************************************************************************
IModel	*CMOT::createModel(const CClassId &idModel)
{
	nlassert(idModel!=CClassId::Null);

	CModelEntry	e;
	e.ModelId= idModel;
	set<CModelEntry>::iterator	itModel;
	itModel= RegModels.find(e);

	if(itModel==RegModels.end())
	{
		nlstop;			// Warning, CScene::registerBasics () has not been called !
		return NULL;
	}
	else
	{
		IModel	*m= (*itModel).Creator();
		if(!m)	return NULL;

		std::vector<CTravEntry>::const_iterator	itTrav;
		for(itTrav= Traversals.begin(); itTrav!=Traversals.end(); itTrav++)
		{
			// Create observer.
			IObs	*obs= createObs((*itTrav).Trav, idModel);
			nlassert(obs);
			// Init model.
			obs->Model= m;
			obs->Trav= (*itTrav).Trav;
			// Attach it to Model.
			m->Observers.insert(IModel::CObsMap::value_type((*itTrav).TravId, obs));
			// Attach them to traversal's root.
			if((*itTrav).Trav->getRoot())
				(*itTrav).Trav->link(NULL, m);
		}

		// After m finished, init the observers.
		IModel::CObsMap::const_iterator	itObs;
		for(itObs= m->Observers.begin(); itObs!=m->Observers.end(); itObs++)
		{
			IObs	*o= (*itObs).second;
			o->init();
		}

		// Insert the model into the list.
		Models.insert(m);

		return m;
	}
}
// ***************************************************************************
void	CMOT::deleteModel(IModel *model)
{
	if(model==NULL)
		return;
	set<IModel*>::iterator	it= Models.find(model);
	if(it!=Models.end())
	{
		delete *it;
		Models.erase(it);
	}
}


// ***************************************************************************
void	CMOT::validateModels()
{
	// check all the models.
	set<IModel*>::iterator	itmodel;
	itmodel= Models.begin();
	while( itmodel!=Models.end())
	{
		IModel*		model= (*itmodel);

		// chek / validate the model.
		model->validate();

		itmodel++;
	}
}


// ***************************************************************************
IObs	*CMOT::createObs(const ITrav *trav, const CClassId &idModel) const
{
	nlassert(trav);
	CClassId idTrav= trav->getClassId();
	nlassert(idTrav!=CClassId::Null);

	if(idModel==CClassId::Null)
	{
		// Use trav to create default observer.
		return trav->createDefaultObs();
	}

	CObsEntry	e;
	e.TravId= idTrav;
	e.ModelId= idModel;
	std::set<CObsEntry>::iterator	it;
	it= RegObservers.find(e);

	if(it==RegObservers.end())
	{
		// Try the father of the model.
		CModelEntry	e;
		e.ModelId= idModel;
		set<CModelEntry>::iterator	it;
		it= RegModels.find(e);

		nlassert(it!=RegModels.end());

		return createObs(trav, (*it).BaseModelId);
	}
	else
	{
		// Register with the specified observer.
		return (*it).Creator();
	}
}


// ***************************************************************************
IObs	*CMOT::getModelObs(IModel *m, const NLMISC::CClassId &idTrav) const
{
	if(!m)
		return NULL;
	return m->getObs(idTrav);
}


// ***************************************************************************
// ***************************************************************************
// ITrav.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void	ITrav::setRoot(IModel	*root)
{
	if(root)
		Root= root->getObs(getClassId());
	else
		Root= NULL;
}
// ***************************************************************************
IModel	*ITrav::getRoot() const
{
	if(Root)
		return Root->Model;
	else
		return NULL;
}

// ***************************************************************************
void	ITrav::link(IModel *m1, IModel *m2) const
{
	IObs	*o1,*o2;

	nlassert(m2);
	CClassId	travId= getClassId();

	if(m1)
	{
		o1= m1->getObs(travId);
		nlassert(o1);
	}
	else
	{
		o1= Root;
		// If his one is NULL, return.
		// NB: this may not be an error. eg: in CScene::release(), when all models are deleted, 
		// a ~CSkeletonModel() call link() to Root, but Root model may have been deleted first!!!
		if(!o1)
			return;
	}
	o2= m2->getObs(travId);
	nlassert(o1);
	nlassert(o2);
	o2->addParent(o1);
	o1->addChild(o2);
}
// ***************************************************************************
void	ITrav::unlink(IModel *m1, IModel *m2) const
{
	IObs	*o1,*o2;

	nlassert(m2);
	CClassId	travId= getClassId();

	if(m1)
	{
		o1= m1->getObs(travId);
		nlassert(o1);
	}
	else
	{
		o1= Root;
		// If his one is NULL, return.
		// NB: this may not be an error. eg: in CScene::release(), when all models are deleted, 
		// a ~CSkeletonModel() call link() to Root, but Root model may have been deleted first!!!
		if(!o1)
			return;
	}
	o2= m2->getObs(travId);
	nlassert(o1);
	nlassert(o2);
	o2->delParent(o1);
	o1->delChild(o2);
}

// ***************************************************************************
void	ITrav::moveChildren(IModel *parentFrom, IModel *parentTo) const
{
	// Make a local list of children (since link() modify the list).
	list<IModel	*>	children;
	for(IModel	*c= getFirstChild(parentFrom); c!=NULL; c= getNextChild(parentFrom))
		children.push_back(c);

	for(list<IModel	*>::iterator it= children.begin(); it!= children.end();it++)
	{
		unlink(parentFrom, *it);
		link(parentTo, *it);
	}
}
// ***************************************************************************
void	ITrav::copyChildren(IModel *parentFrom, IModel *parentTo) const
{
	// Make a local list of children (since link() modify the list).
	list<IModel	*>	children;
	for(IModel	*c= getFirstChild(parentFrom); c!=NULL; c= getNextChild(parentFrom))
		children.push_back(c);

	for(list<IModel	*>::iterator it= children.begin(); it!= children.end();it++)
	{
		link(parentTo, *it);
	}
}


// ***************************************************************************
sint	ITrav::getNumChildren(IModel *m) const
{
	IObs	*o= m->getObs(getClassId());
	return o->getNumChildren();
}
// ***************************************************************************
IModel	*ITrav::getFirstChild(IModel *m) const
{
	IObs	*o= m->getObs(getClassId());
	return o->getFirstChild()->Model;
}
// ***************************************************************************
IModel	*ITrav::getNextChild(IModel *m) const
{
	IObs	*o= m->getObs(getClassId());
	return o->getNextChild()->Model;
}

// ***************************************************************************
sint	ITrav::getNumParents(IModel *m) const
{
	IObs	*o= m->getObs(getClassId());
	return o->getNumParents();
}
// ***************************************************************************
IModel	*ITrav::getFirstParent(IModel *m) const
{
	IObs	*o= m->getObs(getClassId());
	return o->getFirstParent()->Model;
}
// ***************************************************************************
IModel	*ITrav::getNextParent(IModel *m) const
{
	IObs	*o= m->getObs(getClassId());
	return o->getNextParent()->Model;
}




// ***************************************************************************
// ***************************************************************************
// IModel.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
IModel::IModel()
{
	TouchObs.resize(Last);
	LastClassId= 0;
	LastObs= NULL;
}
// ***************************************************************************
IModel::~IModel()
{
	CObsMap::iterator	it;
	for(it=Observers.begin();it!=Observers.end();it++)
	{
		delete (*it).second;
		(*it).second=NULL;
	}
}
// ***************************************************************************
IObs	*IModel::getObs(const CClassId &idTrav) const 
{
	CObsMap::const_iterator	it;

	if(idTrav==LastClassId)
		return LastObs;

	LastClassId= idTrav;
	it= Observers.find(idTrav);
	if(it==Observers.end())
		LastObs= NULL;
	else
		LastObs= (*it).second;

	return LastObs;
}


// ***************************************************************************
void	IModel::validate()
{
	update();

	// If the model is newer than observers.
	if(TouchObs[IModel::Dirty])
	{
		// update all model's observers.
		IModel::CObsMap::iterator	it;
		for(it= Observers.begin(); it!= Observers.end(); it++)
		{
			IObs	*o= (*it).second;
			o->update();
		}

		// Must clear all dirty falgs.
		cleanTouch();
	}
}


// ***************************************************************************
// ***************************************************************************
// IObs.
// ***************************************************************************
// ***************************************************************************



// ***************************************************************************
IObs::IObs()
{
	Model= NULL;
	Trav= NULL;
	SonIt=Sons.end();
}
// ***************************************************************************
IObs::~IObs()
{
	set<IObs*>::iterator	it;

	// Delete link from fathers.
	for(it= Fathers.begin(); it!=Fathers.end(); it++)
	{
		IObs	*father= (*it);
		// Must use delChild() since don't know what father is.
		father->delChild(this);
	}

	// Delete link from sons.
	for(it= Sons.begin(); it!=Sons.end(); it++)
	{
		IObs	*son= (*it);
		// Must use delParent() since don't know what son is.
		son->delParent(this);
	}
}
// ***************************************************************************
IObs	*IObs::getObs(const CClassId &idTrav) const
{
	return Model->getObs(idTrav);
}


// ***************************************************************************
void	IObs::addChild(IObs *son)
{
	nlassert(son);

	// insert (if not exist).
	Sons.insert(son);
}
// ***************************************************************************
void	IObs::delChild(IObs *son)
{
	nlassert(son);

	// Just erase (if possible).
	Sons.erase(son);
}
// ***************************************************************************
void	IObs::addParent(IObs *father)
{
	nlassert(father);

	if(isTreeNode())
	{
		// Must test if father is already linked.
		set<IObs*>::iterator	it;
		it= Fathers.find(father);
		if(it!=Fathers.end())
			return;		// father is already a parent of this.

		// Tree node, so delete fathers, and fathers links to me.
		for(it= Fathers.begin(); it!=Fathers.end();it++)
		{
			// Must use delChild() since don't know what father is.
			(*it)->delChild(this);
		}
		Fathers.clear();
	}

	// insert (if not exist).
	Fathers.insert(father);
}
// ***************************************************************************
void	IObs::delParent(IObs *father)
{
	nlassert(father);

	// Just erase (if possible).
	Fathers.erase(father);
}



// ***************************************************************************
sint	IObs::getNumChildren() const
{
	return Sons.size();
}
// ***************************************************************************
IObs	*IObs::getFirstChild() const
{
	SonIt= Sons.begin();
	if(SonIt==Sons.end())
		return NULL;
	else
		return (*SonIt);
}
// ***************************************************************************
IObs	*IObs::getNextChild() const
{
	nlassert(SonIt!=Sons.end());
	SonIt++;
	if(SonIt==Sons.end())
		return NULL;
	else
		return (*SonIt);
}


// ***************************************************************************
sint	IObs::getNumParents() const
{
	return Fathers.size();
}
// ***************************************************************************
IObs	*IObs::getFirstParent() const
{
	FatherIt= Fathers.begin();
	if(FatherIt==Fathers.end())
		return NULL;
	else
		return (*FatherIt);
}
// ***************************************************************************
IObs	*IObs::getNextParent() const
{
	nlassert(FatherIt!=Fathers.end());
	FatherIt++;
	if(FatherIt==Fathers.end())
		return NULL;
	else
		return (*FatherIt);
}





}
