/** \file mot.h
 * The Model / Observer / Traversal  (MOT) paradgim.
 *
 * $Id: mot.h,v 1.9 2003/03/20 14:54:35 berenguier Exp $
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

#ifndef NL_MOT_H
#define NL_MOT_H


#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/bit_set.h"
#include "nel/misc/class_id.h"
#include <vector>
#include <set>
#include <map>



namespace	NLMISC
{
	class	IStream;
}


namespace	NL3D
{


class	IModel;
class	IObs;
class	ITrav;


// ***************************************************************************
/**
 * A composant server, and a traversal container.
 * Register models and observers globally to the program with static registerModel() and registerObs(). This should be done
 *  at the begining of the program.
 *
 * Before using a CMOT object, Add any traversals you want to support to your scene via addTrav().
 *
 * Then, you can create any model for this scene with createModel(). The model will be valid for this scene only, 
 * since it has only the observers of scene's traversals.
 * \sa IModel IObs ITrav
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class	CMOT
{
public:
	// Ctor.
	CMOT();
	// Dtor.
	virtual ~CMOT();

	/// \name Models / Observers registration.
	//@{
	/**
	 * Register a model, indicating from which he derive. 
	 * By default, model's observer are those of his father (idModelBase).
	 * \param idModel the Unique Id of the registered model
	 * \param idModelBase the Unique Id of the base calss of the registered model
	 * \param creator the function which create the registered model.
	 */
	static	void	registerModel(const NLMISC::CClassId &idModel, const NLMISC::CClassId &idModelBase, IModel* (*creator)());
	/**
	 * Register an observer, for a given traversal and a given model. 
	 * If an observer was previously specified for the couple Trav/Model, he is replaced.
	 * \param idTrav the Unique Id of the observer's traversal
	 * \param idModel the Unique Id of the observer's model
	 * \param creator the function which create the registered observer.
	 */
	static	void	registerObs(const NLMISC::CClassId &idTrav, const NLMISC::CClassId &idModel, IObs* (*creator)());
	//@}


public:
	/// \name Traversals  Registration .
	//@{
	/**
	 * Register a traversal and add it to the scene.
	 * This is done by CSene object, and not globally. Hence, we can have different scene, which doesn't support the
	 * same traversals. Undefined result are excepted if you put two or more traversals of the same type in a scene.
	 * \param v the traversal to be added. CMOT will never delete it (so the user should do). v->getId() must be the 
	 * Unique ID of this traversal class.
	 */
	void	addTrav(ITrav *v);
	/// Get the number of registered traversal
	uint	getNumTrav() const { return Traversals.size(); }
	/// Get a traversal by its index
	ITrav  *getTrav(uint index) const 
	{ 
		nlassert(index < Traversals.size());
		return Traversals[index].Trav;
	}
	/**
	 * Get a traversal via its class id.
	 * \param idTrav the Trav Unique Id.
	 * \return the traversal. NULL, if not found.
	 */
	ITrav	*getTrav(const NLMISC::CClassId &idTrav) const;
	//@}

	
public:
	/// \name Model mgt.
	//@{
	/**
	 * Create a model according to his type id.
	 * Model must has been previously registered via registerModel(). This function create all necessary observers, according 
	 * to the traversals registered with addTrav(), and registerObs(). If a model has no osberver specified for a given 
	 * traversal Trav, then the father's one will be created. If no ancestor has defined an observer for this traversal, then
	 * Trav->createDefaultObs() is taken.
	 *
	 * Then, this function attach those observers to the model.
	 *
	 * Then, This function attach this model to the Root of all traversals (if not NULL).
	 *
	 * Model are deleted with the CMOT::deleteModel() or with CMOT::release() which delete all models ans traversals.
	 * NB: Since CMOT own the model, model MUST NOT be used with SmartPtrs (but CRefPtr always work...).
	 * \param idModel the Unique Id of the Model
	 * \return a valid model of the required type. NULL, if not found.
	 * \see deleteModel()
	 */
	IModel	*createModel(const NLMISC::CClassId &idModel);

	/** Delete a model via his pointer.
	 * The model is automatically unlinked from all other model in all traversals, and his observers are 
	 * automatically destroyed.
	 *
	 * Once a model is deleted, all pointer to him should have been deleted.
	 */
	void	deleteModel(IModel *model);

	/** Validate all models and observers. All dirty models are cleaned, and so their observers.
	 * This should be called, before any ITrav traversal.
	 */
	void	validateModels();

	//@}


	/** release all the models and all the traversals created/registred.
	 * Remind that Models are deleted, but not Traversals, since CMOT do not own traversals.
	 */
	virtual void	release();


// ******************
private:
	struct	CModelEntry
	{
		NLMISC::CClassId	ModelId, BaseModelId;
		IModel* (*Creator)();
		bool	operator<(const CModelEntry& o) const {return ModelId<o.ModelId;}
		bool	operator==(const CModelEntry& o) const {return ModelId==o.ModelId;}
		bool	operator!=(const CModelEntry& o) const {return !(*this==o);}
	};

	struct	CObsEntry
	{
		NLMISC::CClassId	TravId, ModelId;
		IObs* (*Creator)();
		bool	operator<(const CObsEntry& o) const 
		{
			if(TravId!=o.TravId)
				return TravId<o.TravId;
			else
				return ModelId<o.ModelId;
		}
		bool	operator==(const CObsEntry& o) const 
		{
			return ModelId==o.ModelId && TravId==o.TravId;
		}
		bool	operator!=(const CObsEntry& o) const {return !(*this==o);}
	};

	struct	CTravEntry
	{
		NLMISC::CClassId	TravId;
		ITrav	*Trav;
	};

private:
	// must do this for _ValidateModelList access.
	friend	class IModel;

	std::vector<CTravEntry>			Traversals;
	std::set<IModel*>				Models;
	IModel							*_ValidateModelList;

	static std::set<CModelEntry>	RegModels;
	static std::set<CObsEntry>		RegObservers;

private:
	// Create an observer, obeying the hierachy/observer system explained in createModel().
	IObs	*createObs(const ITrav *trav, const NLMISC::CClassId &idModel) const;

protected:
	// For hertied class, to access observer result of a model.
	IObs	*getModelObs(IModel *m, const NLMISC::CClassId &idTrav) const;


private:
	// a dummy method, usefull for dynamic_cast only.
	virtual	void	dummyForDynamicCast() {}

};


// ***************************************************************************
/**
 * A base model.
 * A model is the base structure for any node. The user directly manipulates Models implemented by the deriver.
 *
 * The deriver must implement a notification system so observers can know if they must update themselves. This is done with
 * a NLMISC::CBitSet TouchObs. Deriver may add Additional flags, or additional info which may serves just as hint, to not 
 * compute everything (eg: vertex interval...). In CMOT::validateModels(), for each model, do:
 *		- update() the model.
 *		- if the model is dirty:
 *			- update() the observers
 *			- cleanTouch() the model.
 *
 *
 * \b DERIVERS \b RULES:
 * - Possibly Add his own TDirty state (see TDirty and TouchObs), and resize TouchObs (see IModel()).
 * - Implement the notification system (see update()/cleanTouch()), as descripted above.
 *
 * The deriver may choose how to  foul() himself: either automatic (on any mutator function), or by user (which may call a
 * foul function).
 * \sa CMOT IObs ITrav
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class	IModel : public NLMISC::CRefCount
{
protected:
	/**
	 * Init a model.
	 * The user must create a Model only with CMOT::createModel().
	 * This is required since, CMOT::createModel() create observers for his traversals and link them to this created model.
	 *
	 * The deriver \b should do a \c TouchObs.resize(Last), to ensure he resize the BitSet correctly.
	 * The dervier \b should keep/declare ctor and dtor protected, to avoid user error (new and delete).
	 */
	IModel();

	/**
	 *	Extra init for a model. this method is called by the framework at the very end of CMOT::createModel()
	 *	Warning! if the model is a CTransformShape, then when initModel() is called, Shape and other related member/setup
	 *	of IShape::createInstance() are not yet done (because createModel() is called at the begining in createInstance()).
	 *
	 *	Because initModel() is called at the very end, deriver could implement anything like creating other models, 
	 *	but not deleting this model...
	 *
	 *	Default behavior is to do nothing.
	 */
	virtual	void	initModel()
	{
	}

	/** Destrutor. Model's observers are deleted automatically.
	 * The user must delete a Model only with CMOT::deleteModel(). This ensure that model validity is correct during the 
	 * life of CMOT.
	 *
	 * The dervier \b should keep/declare ctor and dtor protected, to avoid user error (new and delete).
	 */
	virtual	~IModel();

public:
	
	/// \name Notification system
	//@{
	/** 
	 * The Dirty states. Derived models may add flags with similar enum. The first enum element must begin at 
	 * CBaseClass::Last (where CBaseClass is the base class), so falg compatibility is maintained.
	 */
	enum	TDirty
	{
		Dirty=0,		// First bit, to say that the Model is dirty.
		Last
	};
	/// TouchObs say what part of the model has changed, so Observers can deal with this in Obs::update().
	NLMISC::CBitSet		TouchObs;
	/// The derived model should call foul() in update() or other mutator functions.
	void	foul(uint flag) { TouchObs.set(Dirty); TouchObs.set(flag); }

	/// check if the model is modified, and if yes, update him and his observers.
	void	validate();

	//@}


protected:
	// Observers created and linked by CMOT::createModel()
	friend	class	CMOT;
	friend	class	IObs;
	friend	class	ITrav;
	typedef	std::map<NLMISC::CClassId, IObs*>		CObsMap;
	CObsMap	Observers;

	/// Get an observer according to his Traversal Id. NULL, if not found.
	IObs	*getObs(const NLMISC::CClassId &idTrav) const;


	// A link to the CMOT which created us, filled in createModel().
	CMOT			*_OwnerMot;

	// linked list of models to validate.
	IModel			*_PrecModelToValidate;
	IModel			*_NextModelToValidate;


protected:


	/// \name Notification system specification.
	//@{
	/** This function must update the model (called by CMOT::validateModels()), and foul() necessary flags.
	 * Must :
	 *	- call BaseClass::update() (eg: IModel::update()).
	 *	- test if something is different (eg: animation modification). update Model information (eg compute new Matrix).
	 *	- foul() good bits. (eg: foul(TransformDirty)).
	 *	- maybe set other Touch information (vertex intervals...).
	 *
	 * The default behavior is to do nothing.
	 *
	 * NB: Touch information is reseted after observers validation in CMOT::validateModels(), using Model::cleanTouch()
	 *
	 */
	virtual	void	update()
	{
	}

	/** This function must clean the Touch information of the model (called by CMOT::validateModels())
	 * It is called AFTER his observers are validated according to him.
	 *
	 * This function Must :
	 *	- call BaseClass::cleanTouch() (eg: IModel::cleanTouch()).
	 *	- maybe clean other Touch information (vertex intervals...).
	 *
	 * NB: Touch bits are leared in validateModels().
	 *
	 * The default behavior is just to clear flags of TouchObs, which may be sufficient, except if you have special 
	 * touch information (eg: a vertex interval).
	 */
	virtual	void	cleanTouch()
	{
		TouchObs.clearAll();
	}

	// for CMOT::createModel() and for CTransform::freezeHRC() only.
	void	linkToValidateList();
	void	unlinkFromValidateList();

	//@}


};


// ***************************************************************************
/**
 * A base observer.
 * An observer is a link beetween a Model and a Traversal. It may implement the behavior of the model for this 
 * particular traversal. Only deriver have access to /implement  observers. Users don't manipulate observers.
 *
 * IObs define:
 * - graph methods to link/unlink to observers. Read carefully this section.
 * - a notification system (see IModel for an explanation)
 * - an interface for traversal to traverse() this observer. 
 *
 * \b DERIVERS \b RULES:
 * - Implement the notification system: Just extend update().
 * - Implement the traverse() method
 * - Possibly Extend/Modify the graph methods (see isTreeNode(), addParent() ...).
 * - Possibly Extend/Modify the init() method.
 *
 * Since Models are created with CMOT::createModel(), an observer is never alone, he always has a Model and a ITrav.
 * \sa CMOT IModel ITrav
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class	IObs : public NLMISC::CRefCount
{
public:
	IModel	*Model;		// The model for this observer.
	ITrav	*Trav;		// The traversal for this observer.

public:
	IObs();
	/// ~IObs() must destroy correclty the father/son links  (with call to virtual delChild() delParent()).
	virtual			~IObs();
	/** This is called at the end of createModel(). So the model is correctly constructed and linked to his observers.
	 * The default behavior is to do nothing. Warning! all IObs::init() are called BEFORE IModel::initModel().
	 */
	virtual	void	init() {}


	/**
	 * \name Graph methods. USED ONLY BY ITrav*.
	 * BASIC EXTENSION: Although All those methods may be extended, it may be a hard work. So if an observer just want to 
	 * choose beetween being a \b tree \b Node (which has only one parent) or being a \b Graph \b node, it may just implement 
	 * the isTreeNode() method which return true by default.
	 *
	 * ADVANCED EXTENSION: the deriver may implement the way this observer is linked to the traversal graph. \n
	 * The deriver must use list<> IObs::SonList and IObs::FatherList and set<> IObs::SonMap and IObs::FatherMap 
	 *	in the same way the default implementation use them. \n
	 * We cut the behavior into two ways: addParent and addChild() (and their respective del*() function). We must do this
	 * since some observers may link to sons in a particualr way (Z-list ...), and some others may link to parents in
	 * a particular way (such as tree node which want to delete their old parent).
	 *
	 * ITrav::link() and ITrav::unlink() will call the correct functions:\n
	 * o1->addChild(o2); o2->addParent(o1);
	 *
	 * If the deriver store the links to sons / fathers in a particular way, it must destroy them in his destructor.
	 *	NB: in all case, get*() are NOT virtual because of speed consideration.
	 *
	 */
	//@{
	/// For standard behavior, just modify this method, to have a TreeNode behavior or a graphNode behavior. Default: true.
	virtual bool	isTreeNode() {return true;}
	/// This function SHOULD JUST add son to the son list. If \c son was already a son, no-op.
	virtual	void	addChild(IObs *son);
	/// This function SHOULD JUST delete son from the son list. If \c son is not a son, no-op.
	virtual	void	delChild(IObs *son);
	/** This function SHOULD JUST add father to the parent list. If \c father was already a parent, no-op.
     * The default behavior is to call isTreeNode(). If true, unlink from his current father, and call 
	 * precFather->delChild(this). Hence this observer has a behavior of a Tree node (can have only one parent).
	 */
	virtual	void	addParent(IObs *father);
	/// This function SHOULD JUST delete father from the parent list. If \c father is not a parent, no-op.
	virtual	void	delParent(IObs *father);

	/// Get the number of children.
	sint	getNumChildren() const {return NumSons;}
	/// Return the first child of the observer. NULL returned if not found.
	IObs	*getFirstChild() const
	{
		if(NumSons==0)
			return 0;
		CurSonIt= SonList.begin();
		nlassert(CurSonIt!=SonList.end());
		return (*CurSonIt);
	}
	/// Return the next child of the observer. NULL returned if not found. Unpredictible results if insertions/deletions are made between a getFirstChild() / getNextChild().
	IObs	*getNextChild() const
	{
		nlassert(CurSonIt!=SonList.end());
		CurSonIt++;
		if(CurSonIt==SonList.end())
			return NULL;
		else
			return (*CurSonIt);
	}

	/// Get the number of parent.
	sint	getNumParents() const {return NumFathers;}
	/// Return the first parent of the observer. NULL returned if not found.
	IObs	*getFirstParent() const
	{
		if(NumFathers==0)
			return 0;
		CurFatherIt= FatherList.begin();
		nlassert(CurFatherIt!=FatherList.end());
		return (*CurFatherIt);
	}
	/// Return the next parent of the observer. NULL returned if not found. Unpredictible results if insertions/deletions are made between a getFirstParent() / getNextParent().
	IObs	*getNextParent() const
	{
		nlassert(CurFatherIt!=FatherList.end());
		CurFatherIt++;
		if(CurFatherIt==FatherList.end())
			return NULL;
		else
			return (*CurFatherIt);
	}

	//@}


	/// \name DoIt method. USED ONLY BY ITrav*.
	//@{
	/**
	 * Traverse this observer.
	 * This function "justdoit" must do all the traversal thing:
	 *	- Get info from caller, and DoIt the observer (completly observer depedent).
	 *	- should traverse() his sons (or simply call traverseSons()).
	 * \param caller the father of the observer which have called traverse(). WARNING: this is NULL, for the ROOT.
	 */
	virtual	void	traverse(IObs *caller)=0;
	//@}


	/// \name Notification system.
	//@{

	/**
	 * This function must update the observer, according to his model's Touch information and the Model data.
	 * NB: do not modify here Touch information of the model (because they may be used by other Observers).
	 *
	 * The default behavior is to do nothing.
	 */
	virtual	void	update()
	{
	}
	//@}


	/// \name Utility methods.
	//@{
	/// Do traverse() for all sons of this observer.
	void	traverseSons()
	{
		for(IObs *c= getFirstChild(); c!=NULL; c= getNextChild())
		{
			c->traverse(this);
		}
	}
	/// Get the observer for an other view, via IModel.
	IObs			*getObs(const NLMISC::CClassId &idTrav) const;
	//@}


protected:
	typedef	std::list<IObs*>	TObsList;
	typedef	TObsList::iterator	ItObsList;
	typedef	TObsList::const_iterator	ConstItObsList;
	typedef	std::map<IObs *, ItObsList>	TObsMap;
	typedef	TObsMap::iterator			ItObsMap;

	// separate in list/set, to have o(1) lookup, and o(logn) insertion/erase.
	TObsList					SonList;
	TObsList					FatherList;
	TObsMap						SonMap;
	TObsMap						FatherMap;
	sint						NumFathers;
	sint						NumSons;

	mutable	ConstItObsList		CurSonIt;
	mutable	ConstItObsList		CurFatherIt;

};



// ***************************************************************************
/**
 * A base Traversal.
 * A traversal represent a functionality, something which is be performed on a graph of model. Since, we may have
 * differents graph of models, dependent on the traversal, a traversal maintain rather a graph of observers. But this is
 * invisible for the user, since he calls link() methods with models.
 *
 * A traversal provide:
 * - interface to create a default observer for models which don't specify them. See createDefaultObs().
 * - interface to identify himself with a classId. getClassId()
 * - methods for building the graph of observers. setRoot(), getRoot(), link(), unlink()...
 *
 * \b DERIVERS \b RULES:
 * - Implement createDefaultObs()
 * - Implement getClassId()
 *
 * No traverse() method is provided. The deriver may use their own function.
 * \sa CMOT IModel IObs
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class	ITrav : public NLMISC::CRefCount
{
public:

	/// \name Object
	//@{
	/// Constructor. Root Must be created/linked by User via setRoot().
	ITrav()	{Root=NULL;}
	/// Destructor. ~ITrav() doesn't delete Root. Root Must be destructed by user.
	virtual ~ITrav() {Root= NULL;}
	//@}

	/// \name Misc.
	//@{
	/** 
	 * This method must create a default observer for this traversal. Any model which doesn't provide (by registerObs() 
	 * or by inheritance) an observer for this view will be linked with this default observer.
	 */
	virtual	IObs				*createDefaultObs() const =0;
	/// This function must return the Unique Ident for this traversal class.
	virtual	NLMISC::CClassId	getClassId() const =0;
	/// Called when this traversal has been added to MOT object. The default does nothing
	virtual	void				addedToMOT(CMOT *mot) {}
	//@}


	/// \name Graph Methods.
	//@{
	/**
	 * Specify a root Model for this traversal (via his IObs).
	 * The model must be created with a valid/final CMOT.
	 * CMOT::createModel() will don't link the model if a traversal in the CMOT object has not specified a root. \n
	 * ITrav::link(NULL, ...) will assert, if no root defined.
	 *
	 * You may specify a NULL root (this may lead to a disabled traversal).
	 */
	void	setRoot(IModel	*root);
	/// Get the root of the traversal (NULL if not defined).
	IModel	*getRoot() const;
	/**
	 * Link 2 models via their IObs for this traversal.
	 * m2 becomes a child of m1.
	 * If m1==NULL, m2 will be linked to the Root of this traversal (or do nothing if this one is NULL).
	 * if m2 was already a son of m1, no-op.
	 */
	void	link(IModel *m1, IModel *m2) const;
	/**
	 * Unlink 2 models via their IObs for this traversal.
	 * If m1==NULL, m2 will be unlinked from the Root of this traversal (or do nothing if this one is NULL).
	 * if m2 was not a son of m1, no-op.
	 */
	void	unlink(IModel *m1, IModel *m2) const;
	/// make the children of parentFrom unlinked, and become the children of parentTo.
	void	moveChildren(IModel *parentFrom, IModel *parentTo) const;
	/// make the children of parentFrom become the children of parentTo too (may works like moveChildren if children are TreeNode).
	void	copyChildren(IModel *parentFrom, IModel *parentTo) const;


	/// Get the number of children of the model for this traversal.
	sint	getNumChildren(IModel *m) const;
	/// Return the first child of the Model for this traversal. NULL returned if not found.
	IModel	*getFirstChild(IModel *m) const;
	/// Return the next child of the Model for this traversal. NULL returned if not found. Unpredictible results if link() / unlink() are made between a getFirstChild() / getNextChild().
	IModel	*getNextChild(IModel *m) const;

	/// Get the number of Parents of the model for this traversal.
	sint	getNumParents(IModel *m) const;
	/// Return the first Parent of the Model for this traversal. NULL returned if not found.
	IModel	*getFirstParent(IModel *m) const;
	/// Return the next Parent of the Model for this traversal. NULL returned if not found. Unpredictible results if link() / unlink() are made between a getFirstParent() / getNextParent().
	IModel	*getNextParent(IModel *m) const;

	//@}


protected:
	// The root observer.
	NLMISC::CRefPtr<IObs>	Root;

};




}


#endif // NL_MOT_H

/* End of mot.h */

