/* mot.h
 *
 * Copyright (C) 2000 Nevrax. All rights reserved.
 *
 * The redistribution, use and modification in source or binary forms of
 * this software is subject to the conditions set forth in the copyright
 * document ("Copyright") included with this distribution.
 */

/*
 * $Id: mot.h,v 1.1 2000/10/02 12:56:15 berenguier Exp $
 *
 * The Model / Observer / Traversal  (MOT) paradgim.
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


/*
	// THIS IS NOT TRADUCED BECAUSE NOT FIXED YET.

  Le moteur est basé sur un modèle Model / Observer / Traversal. (MOT).

  Graph de classe Model / Observer / Traversal  (Model, Obs, Trav)

  CMOT contient N traversals de type différents. Typiquement, les 4 traversals de base sont:

CHandleTrav:	la hierarchie d'objet. Le graph est un arbre. Le Traverser donne toutes les WorldMatrix des objets,
	necessaire aux autres traversals.
CCullTrav:	le graph de visibilité. Ce traversal affecte directement CRenderTrav, en ajoutant les objets visibles.
CLightTrav:	le graph de dépendance Light/Objet. Ce graph est aussi un arbre. Il indique quels objets sont illuminés par 
	quelle source lumineuse. Le traversal déduit par noeud une liste des lights qui l'ilumine.
CRenderTrav: la liste triée en Z des objets à rendre.


  On a un SceneGraph en organisant les observers. ie, on a des graph d' observer (on a un graph par Traversal). Les models
  n'ont pas forcément un observer dans chaque graph. Typiquement, ni un cluster ni une light ne se render.

  
  Exemple de models

CLight:		light. Pass: NODE & CULL & LIGHT.
CInstance:	instance d'un mesh quelconque. Pass: NODE & CULL & LIGHT & RENDER
CCluster:	Un cluster représente un partitionnement du monde. NODE & CULL
CPortal:	un portal est un lien entre 2 clusters.  NODE & CULL



Le client n'utilise QUE ITrav et IModel.
Il ne peut accéder à ses Observeurs. Un observeur ici est juste un objet "algorithm" qui travaille sur des modeles, pour un 
traversal. AUCUN setup ne doit donc lui être appliqué. A la question "oui mais j'ai envie de donner des hints pour le calcul 
de la radiosité", la réponse est: Il faut dériver du Model à étendre, pour lui rajouter ces fameux hints, puis implémenter
l'observeur qui va prendre en compte ces hints. Un nouveau model est crée, et c'est donc celui là que le client devra utiliser 
pour setter ses hints pour le traversal de radiosité. De cette facon, on garde l'encapsulation Model/Traversal.


Le deriver peut créer des Traversals, de nouveaux models (soit totalement nouveaux, soit par héritage)
et implémente les IObs, et doit les registerer dans CMOT.


Fonctionnement du Système d'instance (Shapes). Ce modèle ne s'occupe pas des instances, mais il faut donner une ligne de
conduite pour une utilisation correcte. En fait, les règles Model/Observer ne doivent pas être transgressées: le model est
la structure, l'observeur est l'algorithm pour le traversal donné. Cela veut dire que la fonction render() doit être
implémentée dans l'observeur, et non dans le shape. En procédant de cette façon, on est sur que l'observeur pourra accéder 
à toutes les infos dont il a besoin (comme la WorldMatrix de l'observeur Handle par exemple). Cela veut dire aussi qu'il 
faudra que l'interface du shape soit bien définie vis à vis de l'observeur. Faire un nouveau shape demandera donc parfois
de créer un nouveau model (un CInstance particulier, qui n'a pas forcément de nouvelles méthodes), ainsi qu'un nouvel 
observeur qui suportera ce shape.


NOTE: le fait de bien séparer Models/Observers n'interdit pas le stockage de données particulières à l'observeur dans Model.
Ceci est particulièrement vrai pour Shape, qui gardera les liste de vertices pré-computées pour render() etc...


PROBLEME: interaction entre les traversals (clip / cull). ClipObs et RenderObs doit demander le résultat de HandleObs. 
ClipObs doit linker les RenderObs dans le RenderTrav. => pitet obligé, en partie, de fournir une interface de base IObs, 
par Traversal (CBaseRenderObs, CBaseCullObs, ...) pour ainsi fournir une même interface pour les autrres observeurs (eg: 
l'observeur de Cull récupère la WorlMatrix de l'observeur de base Handle). Ceci est à spécifier par Traversal.



Le core du moteur (ie nous :) ) a le droit de changer modèles, Traversals et observeurs, à la condition de garder
une compatibilité ascendante, pour que les modèles/observeurs/traversals écrit par les Deriver marchent encore, bien qu'il 
soit alors nécessaire de recompiler.

*/



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
 * Then, create any model for this scene with createModel(). The model will be valid for this scene only, since it has only
 * the observers of scene's traversals.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class	CMOT
{
public:
	/// \name Models / Observers / Traversals  Registration .
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

	/**
	 * Register a traversal and add it to the scene.
	 * This is done by CSene object, and not globally. Hence, we can have different scene, which doesn't support the
	 * same traversals. Undefined result are excepted if you put two or more traversals of the same type in a scene.
	 * \param v the traversal to be added. CMOT will never delete it (so the user should do). v->getId() must be the 
	 * Unique ID of this traversal class.
	 */
	void	addTrav(ITrav *v);
	/**
	 * Get a traversal via its class id.
	 * \param idTrav the Trav Unique Id.
	 * \return the traversal. NULL, if not found.
	 */
	ITrav	*getTrav(const NLMISC::CClassId &idTrav) const;
	//@}


public:
	/**
	 * Create a model according to his id.
	 * Model must has been previously registered via registerModel(). This function create all necessary observers, according 
	 * to the traversals registered with addTrav(), and registerObs(). If a model has no osberver specified for a given 
	 * traversal Trav, then the father's one will be created. If no ancestor has defined an observer for this traversal, then
	 * Trav->createDefaultObs() is taken.
	 *
	 * Then, this function attach those observers to the model.
	 *
	 * Then, This function attach those observers to the Root of their respective traversals (if not NULL).
	 * 
	 * \param idModel the Unique Id of the Model
	 * \return a valid model of the required type. NULL, if not found.
	 */
	IModel	*createModel(const NLMISC::CClassId &idModel) const;


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
	std::vector<CTravEntry>			Traversals;
	static std::set<CModelEntry>	Models;
	static std::set<CObsEntry>		Observers;

private:
	// Create an observer, obeying the hierachy/observer system explained in createModel().
	IObs	*createObs(const ITrav *trav, const NLMISC::CClassId &idModel) const;
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
 * - methods for building the graph of observers. setRoot(), getRoot(), link(), unlink().
 * No traverse() method is provided. The deriver may use their own function.
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
	 * This method must create a default observer for this traversal. Any model which doesn't provide (by registerObs() or by inheritance)
	 * an observer for this view will be linked with this default observer.
	 */
	virtual	IObs	*createDefaultObs() const =0;
	/// This function must return the Unique Ident for this traversal class.
	virtual	NLMISC::CClassId	getClassId() const =0;
	//@}


	/// \name Graph Methods.
	/**
	 * Specify a root Object. Must be called before link it to a CMOT.
	 * CMOT::createModel() will assert if a traversal in the CMOT object has not specified a root. \n
	 * Also, ITrav::link(NULL, ...) will assert, if no root defined.
	 */
	void	setRoot(IObs	*root);
	/// Get the root of the traversal.
	IObs	*getRoot() const;
	/**
	 * Link 2 models via their IObs for this traversal.
	 * m2 becomes a child of m1.
	 * If m1==NULL, m2 will be linked to the Root of this traversal (or assert() if this one is NULL).
	 * if m2 was already a son of m1, no-op.
	 */
	void	link(IModel *m1, IModel *m2) const;
	/**
	 * Unlink 2 models via their IObs for this traversal.
	 * If m1==NULL, m2 will be unlinked from the Root of this traversal (or assert() if this one is NULL).
	 * if m2 was not a son of m1, no-op.
	 */
	void	unlink(IModel *m1, IModel *m2) const;


private:
	// The root observer.
	IObs	*Root;

};


// ***************************************************************************
/**
 * A base model.
 * A model is the base structure for any node. The user directly manipulates Models implemented by the deriver.
 *
 * The deriver must implement a notification system so observers can know if they must update themselves. This is done with
 * a NLMISC::CBitSet Touch. Deriver may add Additional flags, or additional info which may serves just as hint, to not 
 * compute everything. When an observer function is called, it may check if the model has been modified via this BitSet.
 * If this is the case, then all observers will be "touched", via a IObs::foul() virtual function, and then the model is 
 * clean() -ed. Then, a dirty observer may update himself with help of the model.
 *
 * The deriver may choose how to  foul() himself: either automatic (on any mutator function), or by user (which may call a
 * foul function).
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class	IModel : public NLMISC::CRefCount
{
protected:
	/**
	 * Init a model.
	 * The client must create a Model only with CMOT::createModel().
	 * This is required since, CMOT::createModel() create observers for his traversals and link them to this created model.
	 *
	 * The deriver should do a \c Touch.resize(Last), to ensure he resize the BitSet correctly.
	 */
	IModel();


public:
	/// Destrutor. Model's observers are deleted automatically.
	virtual	~IModel();

	
	/// \name Notification system
	//@{
	/** 
	 * The Dirty states. Derived models may add flags with similar enum. The first enum element must begin at 
	 * CBase::Last (where CBase is the base class), so falg compatibility is maintained.
	 */
	enum	TDirty
	{
		Dirty=0,		// First bit, to say that the Model is dirty.
		Last
	};
	NLMISC::CBitSet	Touch;
	/// Is the model dirty?
	bool	isDirty() const { return Touch[Dirty];}
	/// The derived model must set "Dirty" flag to force the dirty state.
	void	foul() { Touch.set(Dirty, true);}

	/** This function must clean the model (called by model's observers). 
	 * Must set Touch[Dirty] to 0, and reset all other special information of the model.
	 */
	virtual	void	clean()=0;
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
};


// ***************************************************************************
/**
 * A base observer.
 * An observer is a link beetween a Model and a Traversal. It may implement the behavior of the model for this 
 * particular traversal. Only deriver have access to /implement  observers. Users don't manipulate observers.
 *
 * IObs define:
 * - graph methods to link/unlink to observers.
 * - a notification system (see IModel for an explanation)
 * - an interface for traversal to traverse() this observer. 
 *
 * \sa IModel
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
	/// The deriver should do a \c Touch.resize(Last), to ensure he resize the BitSet correctly.
	IObs();
	// ~IObs() must destroy correclty the father/son links  (with call to virtual delChild() delParent()).
	virtual			~IObs();

	// Get the observer for an other view, via IModel.
	IObs			*getObs(const NLMISC::CClassId &idTrav) const;


	/**
	 * \name Graph methods.
	 * USED ONLY BY ITrav*. The deriver must implement the way this observer is linked to the traversal graph. \n
	 * The default behavior of all those function is to use the set<> IObs::Sons and IObs::Fathers. \n
	 * We cut the behavior into two ways: addParent and addChild() (and their respective del*() function). We must do this
	 * since some observers may link to sons in a particualr way (Z-list ...), and some others may link to parents in
	 * a particular way (such as tree node which want to delete their old parent).
	 *
	 * ITrav::link() and ITrav::unlink() will call the correct functions:\n
	 * o1->addChild(o2); o2->addParent(o1);
	 *
	 */
	//@{
	/// This function SHOULD JUST add son to the son list. If \c son was already a son, no-op.
	virtual	void	addChild(IObs *son);
	/// This function SHOULD JUST delete son from the son list. If \c son is not a son, no-op.
	virtual	void	delChild(IObs *son);
	/** This function SHOULD JUST add father to the parent list. If \c father was already a parent, no-op.
     * The default behavior is to unlink from his current father, and call precFather->delChild(this). Hence this observer
	 * has a behavior of a Tree node (can have only one parent).
	 */
	virtual	void	addParent(IObs *father);
	/// This function SHOULD JUST delete father from the parent list. If \c father is not a parent, no-op.
	virtual	void	delParent(IObs *father);

	/// Get the number of children.
	virtual	sint	getNumChildren() const;
	/// Return the first child of the observer. NULL returned if not found.
	virtual	IObs	*getFirstChild() const;
	/// Return the next child of the observer. NULL returned if not found. Unpredictible results if insertions are made between a getFirstChild() / getNextChild().
	virtual	IObs	*getNextChild() const;
	//@}


	/// \name DoIt method. USED ONLY BY ITrav*.
	//@{
	/**
	 * Traverse this observer.
	 * This function "justdoit" must do all the traversal thing: Get info from caller, call the observers sons traverse() 
	 * function, depending on what she want to do etc...
	 */
	virtual	void	traverse(IObs *caller)=0;
	//@}


	/// \name Notification system.
	//@{
	/** 
	 * The Dirty states. Derived models may add flags with similar enum. The first enum element must begin at 
	 * CBase::Last (where CBase is the base class), so falg compatibility is maintained.
	 */
	enum	TDirty
	{
		Dirty=0,		// First bit, to say that the observer is dirty
		Last
	};
	NLMISC::CBitSet	Touch;
	/// Is the model dirty?
	bool	isDirty() const { return Touch[Dirty];}

	/**
	 * This function must foul the observer, according to the state of the model. The observer must keep any usefull
	 * dirty information, since the model will be cleaned soon.
	 */
	virtual	void	foul()=0;
	/**
	 * This function must clean the observer, using his Touch information. She can't use the state of the model too see
	 * what to clean, since the model has been cleaned (see IObs::foul()). But she must use the model to know how to compute 
	 * the observer.
	 */
	virtual	void	clean()=0;
	/**
	 * This function should be called on any function which want to use a valid observer, particularly the traverse() function.
	 * This function first test if the model is dirty. If this is the case, she do o->foul() for any observer o of model Model,
	 * then Model->clean().
	 * Then this function test if the observer himself is dirty, and call clean() if this is the case.
	 */
	void	update();
	//@}


protected:
	std::set<IObs*>	Sons;
	std::set<IObs*>	Fathers;

	mutable	std::set<IObs*>::const_iterator	SonIt;

};



}


#endif // NL_MOT_H

/* End of mot.h */

