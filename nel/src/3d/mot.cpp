/* mot.cpp
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: mot.cpp,v 1.1 2000/10/02 12:56:29 berenguier Exp $
 *
 * The Model / Observer / Traversal  (MOT) paradgim.
 */


#include "nel/3d/mot.h"
#include "nel/misc/assert.h"
#include "nel/misc/stream.h"
#include <algorithm>
using namespace std;
using namespace NLMISC;


namespace	NL3D
{


// ***************************************************************************
// ***************************************************************************
// CMOT.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void	CMOT::registerModel(const CClassId &idModel, const CClassId &idModelBase, IModel* (*creator)())
{
	assert(idModel!=CClassId::Null);
	assert(creator);
	// idModelBase may be Null...

	CModelEntry		e;
	e.BaseModelId= idModelBase;
	e.ModelId= idModel;
	e.Creator= creator;

	// Insert/replace e.
	Models.erase(e);
	Models.insert(e);
}
// ***************************************************************************
void	CMOT::registerObs(const CClassId &idTrav, const CClassId &idModel, IObs* (*creator)())
{
	assert(idTrav!=CClassId::Null);
	assert(idModel!=CClassId::Null);
	assert(creator);

	CObsEntry		e;
	e.ModelId= idModel;
	e.TravId= idTrav;
	e.Creator= creator;

	// Insert/replace e.
	Observers.erase(e);
	Observers.insert(e);
}
// ***************************************************************************
void	CMOT::addTrav(ITrav *v)
{
	assert(v);
	CClassId	idTrav= v->getClassId();
	assert(idTrav!=CClassId::Null);

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
IModel	*CMOT::createModel(const CClassId &idModel) const
{
	assert(idModel!=CClassId::Null);

	CModelEntry	e;
	e.ModelId= idModel;
	set<CModelEntry>::iterator	it;
	it= Models.find(e);

	if(it==Models.end())
		return NULL;
	else
	{
		IModel	*m= (*it).Creator();
		if(!m)	return NULL;

		std::vector<CTravEntry>::const_iterator	it;
		for(it= Traversals.begin(); it!=Traversals.end(); it++)
		{
			// Create observer.
			IObs	*obs= createObs((*it).Trav, idModel);
			assert(obs);
			// Init model.
			obs->Model= m;
			obs->Trav= (*it).Trav;
			// Attach it to Model.
			m->Observers.insert(IModel::CObsMap::value_type((*it).TravId, obs));
			// Attach them to traversal's root.
			(*it).Trav->link(NULL, m);
		}

		return m;
	}
}
// ***************************************************************************
IObs	*CMOT::createObs(const ITrav *trav, const CClassId &idModel) const
{
	assert(trav);
	CClassId idTrav= trav->getClassId();
	assert(idTrav!=CClassId::Null);

	if(idModel==CClassId::Null)
	{
		// Use trav to create default observer.
		return trav->createDefaultObs();
	}

	CObsEntry	e;
	e.TravId= idTrav;
	e.ModelId= idModel;
	std::set<CObsEntry>::iterator	it;
	it= Observers.find(e);

	if(it==Observers.end())
	{
		// Try the father of the model.
		CModelEntry	e;
		e.ModelId= idModel;
		set<CModelEntry>::iterator	it;
		it= Models.find(e);

		assert(it!=Models.end());

		return createObs(trav, (*it).BaseModelId);
	}
	else
	{
		// Register with the specified observer.
		return (*it).Creator();
	}
}



// ***************************************************************************
// ***************************************************************************
// ITrav.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void	ITrav::setRoot(IObs	*root)
{
	Root= root;
}
// ***************************************************************************
IObs	*ITrav::getRoot() const
{
	return Root;
}

// ***************************************************************************
void	ITrav::link(IModel *m1, IModel *m2) const
{
	IObs	*o1,*o2;

	assert(m2);
	CClassId	travId= getClassId();

	if(m1)
	{
		o1= m1->getObs(travId);
		assert(o1);
	}
	else
	{
		o1= Root;
		assert(o1);
	}
	o2= m2->getObs(travId);
	assert(o1);
	assert(o2);
	o2->addParent(o1);
	o1->addChild(o2);
}
// ***************************************************************************
void	ITrav::unlink(IModel *m1, IModel *m2) const
{
	IObs	*o1,*o2;

	assert(m2);
	CClassId	travId= getClassId();

	if(m1)
	{
		o1= m1->getObs(travId);
		assert(o1);
	}
	else
	{
		o1= Root;
		assert(o1);
	}
	o2= m2->getObs(travId);
	assert(o1);
	assert(o2);
	o2->delParent(o1);
	o1->delChild(o2);
}



// ***************************************************************************
// ***************************************************************************
// IModel.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
IModel::IModel()
{
	Touch.resize(Last);
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

	it= Observers.find(idTrav);
	if(it==Observers.end())
		return NULL;
	else
		return (*it).second;
}



// ***************************************************************************
// ***************************************************************************
// IObs.
// ***************************************************************************
// ***************************************************************************



// ***************************************************************************
IObs::IObs()
{
	Touch.resize(Last);

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
void	IObs::update()
{
	// If the model is dirty. 
	if(Model->isDirty())
	{
		// Foul all model's observers.
		IModel::CObsMap::iterator	it;
		for(it= Model->Observers.begin(); it!= Model->Observers.end(); it++)
		{
			IObs	*o= (*it).second;
			o->foul();
		}
		// Clean the model.
		Model->clean();
	}

	// If I am dirty(), clean me.
	if(isDirty())
	{
		clean();
	}
}




// ***************************************************************************
void	IObs::addChild(IObs *son)
{
	assert(son);

	// insert (if not exist).
	Sons.insert(son);
}
// ***************************************************************************
void	IObs::delChild(IObs *son)
{
	assert(son);

	// Just erase (if possible).
	Sons.erase(son);
}
// ***************************************************************************
void	IObs::addParent(IObs *father)
{
	assert(father);

	// Must test if father is already linked.
	set<IObs*>::iterator	it;
	it= Fathers.find(father);
	if(it!=Fathers.end())
		return;		// father is already a parent of this.

	// Tree node, so delete fathers, and fathers link to me.
	for(it= Fathers.begin(); it!=Fathers.end();it++)
	{
		// Must use delChild() since don't know what father is.
		(*it)->delChild(this);
	}
	Fathers.clear();

	// insert.
	Fathers.insert(father);
}
// ***************************************************************************
void	IObs::delParent(IObs *father)
{
	assert(father);

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
	assert(SonIt!=Sons.end());
	SonIt++;
	if(SonIt==Sons.end())
		return NULL;
	else
		return (*SonIt);
}





}