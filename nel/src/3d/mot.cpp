/** \file mot.cpp
 * The Model / Observer / Traversal  (MOT) paradgim.
 *
 * $Id: mot.cpp,v 1.20 2003/03/20 14:54:35 berenguier Exp $
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

#include "std3d.h"

#include "3d/mot.h"
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
	_ValidateModelList= NULL;
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
	v->addedToMOT(this);
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
	// No models at all.
	_ValidateModelList= NULL;

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

		// Set the owner for the model.
		m->_OwnerMot= this;

		// create observer for each trav.
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

		// Insert the model into the set.
		Models.insert(m);

		// By default the model is validate() in CMOT::validateModels().
		m->linkToValidateList();

		// Once the model is correclty created, finish init him.
		m->initModel();

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
	// check all the models which must be checked.
	IModel	*model= _ValidateModelList;
	IModel	*next;
	while( model )
	{
		// next to validate. get next now, because model->validate() may remove model from the list.
		next= model->_NextModelToValidate;

		// chek / validate the model.
		model->validate();

		// next.
		model= next;
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
sint ITrav::getNumChildren (IModel *m) const
{
	IObs *o = m->getObs(getClassId());
	return o->getNumChildren();
}
// ***************************************************************************
IModel* ITrav::getFirstChild (IModel *m) const
{
	IObs *o = m->getObs(getClassId());
	IObs *child = o->getFirstChild();
	if (child != NULL)
		return child->Model;
	return NULL;
}
// ***************************************************************************
IModel* ITrav::getNextChild (IModel *m) const
{
	IObs *o = m->getObs(getClassId());
	IObs *child = o->getNextChild();
	if (child != NULL)
		return child->Model;
	return NULL;
}

// ***************************************************************************
sint ITrav::getNumParents (IModel *m) const
{
	IObs *o = m->getObs(getClassId());
	return o->getNumParents();
}
// ***************************************************************************
IModel* ITrav::getFirstParent (IModel *m) const
{
	IObs *o = m->getObs(getClassId());
	IObs *father = o->getFirstParent();
	if (father != NULL)
		return father->Model;
	return NULL;

}
// ***************************************************************************
IModel* ITrav::getNextParent (IModel *m) const
{
	IObs *o = m->getObs(getClassId());
	IObs *father = o->getNextParent();
	if (father != NULL)
		return father->Model;
	return NULL;
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

	_OwnerMot= NULL;
	_PrecModelToValidate= NULL;
	_NextModelToValidate= NULL;
}
// ***************************************************************************
IModel::~IModel()
{
	// ensure the model is no more linked to the validateList.
	unlinkFromValidateList();

	// delte observers.
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
void	IModel::linkToValidateList()
{
	if(!_OwnerMot)
		return;

	// If the model is not already inserted.
	if( ! (_PrecModelToValidate!=NULL  ||  _OwnerMot->_ValidateModelList==this) )
	{
		// insert it.
		_NextModelToValidate= _OwnerMot->_ValidateModelList;
		_PrecModelToValidate= NULL;
		if(_NextModelToValidate)
			_NextModelToValidate->_PrecModelToValidate= this;
		_OwnerMot->_ValidateModelList= this;
	}
}


// ***************************************************************************
void	IModel::unlinkFromValidateList()
{
	if(!_OwnerMot)
		return;

	// If the model is inserted.
	if( _PrecModelToValidate!=NULL  ||  _OwnerMot->_ValidateModelList==this )
	{
		// update prec.
		if(_PrecModelToValidate)
			_PrecModelToValidate->_NextModelToValidate= _NextModelToValidate;
		else
			_OwnerMot->_ValidateModelList= _NextModelToValidate;

		// update next.
		if(_NextModelToValidate)
			_NextModelToValidate->_PrecModelToValidate= _PrecModelToValidate;

		// End.
		_PrecModelToValidate= NULL;
		_NextModelToValidate= NULL;
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

	NumFathers= 0;
	NumSons= 0;
	CurSonIt= SonList.end();
	CurFatherIt= FatherList.end();
}
// ***************************************************************************
IObs::~IObs()
{
	ItObsList	it;

	// delete map of Its.
	SonMap.clear();
	FatherMap.clear();

	// Delete link from fathers.
	for(it= FatherList.begin(); it!=FatherList.end(); it++)
	{
		IObs	*father= (*it);
		// Must use delChild() since don't know what father is.
		father->delChild(this);
	}

	// Delete link from sons.
	for(it= SonList.begin(); it!=SonList.end(); it++)
	{
		IObs	*son= (*it);
		// Must use delParent() since don't know what son is.
		son->delParent(this);
	}

	// And so delete lists.
	SonList.clear();
	FatherList.clear();
	NumFathers= 0;
	NumSons= 0;
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
	ItObsMap	it= SonMap.find(son);
	if(it==SonMap.end())
	{
		// insert in the map and in the list.
		SonMap[son]= SonList.insert(SonList.end(), son);
		NumSons++;
	}
}
// ***************************************************************************
void	IObs::delChild(IObs *son)
{
	nlassert(son);

	// Just erase (if possible).
	ItObsMap	it= SonMap.find(son);
	if(it!=SonMap.end())
	{
		// erase from list, then from mapt
		SonList.erase(it->second);
		SonMap.erase(it);
		NumSons--;
	}
}
// ***************************************************************************
void	IObs::addParent(IObs *father)
{
	nlassert(father);

	if(isTreeNode())
	{
		// Must test if father is already linked.
		ItObsMap	itMap;
		itMap= FatherMap.find(father);
		if(itMap!=FatherMap.end())
			return;		// father is already a parent of this.

		// Tree node, so delete fathers, and fathers links to me.
		for(ItObsList it= FatherList.begin(); it!=FatherList.end();it++)
		{
			// Must use delChild() since don't know what father is.
			(*it)->delChild(this);
		}
		FatherMap.clear();
		FatherList.clear();
	}

	// insert (if not exist).
	ItObsMap	it= FatherMap.find(father);
	if(it==FatherMap.end())
	{
		// insert in the map and in the list.
		FatherMap[father]= FatherList.insert(FatherList.end(), father);
		NumFathers++;
	}
}
// ***************************************************************************
void	IObs::delParent(IObs *father)
{
	nlassert(father);

	// Just erase (if possible).
	ItObsMap	it= FatherMap.find(father);
	if(it!=FatherMap.end())
	{
		// erase from list, then from mapt
		FatherList.erase(it->second);
		FatherMap.erase(it);
		NumFathers--;
	}
}



}
