/** \file baseai.h
 * Sevral class for the ia objects fonctionality.
 *
 * $Id: baseai.h,v 1.26 2002/06/17 14:16:54 chafik Exp $
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

#ifndef NL_BASEIA_H
#define NL_BASEIA_H
#include <iostream.h>
#include <fstream.h>
#include <vector>

#include <queue>

#include "nel/ai/agent/ident.h"


/**
this is the agents objects name space.
*/
namespace NLAIAGENT
{


	/**
	TProcessStatement define the statement of an agent process.
	* \author Chafik sameh	 	
	* \author Nevrax France
	* \date 2000
	*/
	enum TProcessStatement {
		processIdle,	/// The agent is in idle state.
		processLocked,	/// The agent is locked, so that we can't run it.
		processBuzzy,	/// The agent is in the buzzy state (curently executing itself)
		processEnd,		/// The agent has completed its execution
		processToKill,	/// The agent has completed its execution and have to be delete.
		processPresKill, /// The agent has completed its execution and have to be remove from exection list.
		processNotComplete,/// The agent have still something to do.
		processError	/// The agent has encountered problems during its excecution.
	};

	/**
	Removes an object from an stl list.
	The comparison is done using the value of the pointer.
	* \author Chafik sameh	 	
	* \author Nevrax France
	* \date 2000
	*/
	template <class T>
	bool eraseFromList(std::list<T> *l,T a)
	{
		std::list<T>::iterator i = l->begin();
		while(i != l->end())
		{		
			std::list<T>::iterator j = i++;
			if(*j == a)
			{
				l->erase(j);
				return true;
			}			
		}
		return false;
	}
	
	/**
	IBasicObjectIA is the base class for all AI classes.
	It defines some basic functionalities as pure virtual functions.
	* \author Chafik sameh	 	
	* \author Nevrax France
	* \date 2000
	*/
	class IBasicObjectIA : public NLAIC::IBasicInterface
	{
		public:
			IBasicObjectIA();
			IBasicObjectIA(const IBasicObjectIA &a);
		
			/**
			Lot of algo use this function to determine the equality between 2 class, in particular the extraction from stl list. 
			the == operator call the bool isEqual(const IBasicObjectIA &a) const member method to determine the equality between tow class.
			*/
			bool operator == (const IBasicObjectIA &classType) const;

			/**
			The bool operator == (const IBasicObjectIA &a) const member method call this function when class 'classType' have the same type of this class,
			programme have to assume the equality between the memebers of this class and the memeber of the class 'classType'.
			*/
			virtual bool isEqual(const IBasicObjectIA &a) const = 0;
			virtual ~IBasicObjectIA();
	};
	
	class IBaseGroupType;
	class IBasicType;
	class IObjectIA;
	class IMessageBase;
	
	struct CIdMethod
	{
		///Index of the method in the class.
		sint32 Index;
		///Hwo far the argument of method is nearest of the method argument that we need.
		double Weight;
		///The scrip op code of the method to run.
		IObjectIA *Method;
		///Return type of the method.
		IObjectIA *ReturnType;

		CIdMethod(const CIdMethod &a);		
		
		CIdMethod(sint32 i, double w,IObjectIA *m,IObjectIA *r);		

		CIdMethod();		

		virtual ~CIdMethod();		

		bool operator < (const CIdMethod &) const;		
		const CIdMethod &operator = (const CIdMethod &);
	};
	

	typedef std::priority_queue<CIdMethod> tQueue;

	/** IObjectIA is the basic class for all IA surpport.
		In this level an IObjectIA objects is a basic agent where it define all element for basic functionality. Among other things the functionality is 
		the definition of static memeber components and the memthods defined in the agent. For exemple we need that for the external call method and
		object library on the agent script, or we need that for the agent remote call method in the server for synchronizing run objects.		

		* \author Chafik sameh	 
		* \author Portier Pierre
		* \author Nevrax France
		* \date 2000

	*/
	class IObjectIA: public IBasicObjectIA
	{	
	protected:
		/** 
		The copy constructor for the object.
		Because the object need an unique reserved ident in the word for objects distribution in the server, we protected the copy constructor.
		*/
		IObjectIA(const IObjectIA &a);

	public:
		/**
		This structure define the return result of run method.		
		*/
		struct CProcessResult
		{
			CProcessResult()
			{				
				ResultState = processIdle;
				Result = NULL;
			}

			CProcessResult(TProcessStatement state)
			{				
				ResultState = state;
				Result = NULL;
			}

			/**
			The statement of the run result.
			*/
			TProcessStatement ResultState;
			/**
			The ruturn value of a method.
			*/
			IObjectIA *Result;
		};		

	public:

		static CProcessResult ProcessRun;
		static CProcessResult ProcessNotComplit;		
		
		/**
		Define the an idle state for an agent all time in this state.
		*/
		static TProcessStatement ProcessIdle;

		/**
		Define the an locked state for an agent all time in this state.
		*/
		static TProcessStatement ProcessLocked;

		/**
		Define the an buzzy state for an agent all time in this state.
		*/
		static TProcessStatement ProcessBuzzy;

		/**
		Define the an end state for an agent all time in this state.
		*/
		static TProcessStatement ProcessEnd;

		/**
		Define the an error state for an agent all time in this state.
		*/
		static TProcessStatement ProcessError;
		

	public:
		IObjectIA();

		/**
		Init method alows to init the attribut of an object. the maparmetre is in more time a list of IObjectIA. 
		*/
		virtual void init(IObjectIA *);

		/**
		Equality operator.
		*/
		virtual IObjectIA &operator = (const IObjectIA &a);

		/**
		Some where Run method define the process of the agen. 
		*/
		virtual const CProcessResult &run() = 0;		

		/**
		Some where Run method define the process of the agen.
		*/
		virtual const CProcessResult &runStep();

		/**
		Static member is the component that the agent is create and die with. getStaticMemberSize to get the size of the static member components
		*/
		virtual sint32 getStaticMemberSize() const;
		/**
		to get the index of a given components name.
		*/
		virtual sint32 getStaticMemberIndex(const IVarName &) const;
		/**
		to get the pointer of a given components referened by an index.
		*/
		virtual const IObjectIA *getStaticMember(sint32) const;

		/**
		to get the pointer of a given components referened by an string.
		*/
		const IObjectIA *getStaticMember(const IVarName &compName) const
		{
			return getStaticMember(getStaticMemberIndex(compName));
		}
		/**
		to cahnge a given components given by an index.
		*/
		bool setStaticMember(const IVarName &compName,IObjectIA *change)
		{
			return setStaticMember(getStaticMemberIndex(compName),change);
		}

		/**
		to cahnge a given components given by a string.
		*/
		virtual bool setStaticMember(sint32,IObjectIA *);

		/**
		getMethodIndexSize define the nomber of method define in the class.
		*/
		virtual sint32 getMethodIndexSize() const;
		/**
		The method isMember(nameSubClass,methodName,param) return the best method correspondent to the search.
		remember we use the overloadind method.

		NameSubClass: if this pointer is not NULL then NameSubClass is the name of the base class were the method are defined.

		methodName: is the name of the class.

		param: is an IObjectIA where it defined the parametre of the method. 
		lot of method use a IObjectIA vector object with an NLAIC::CIdentType as the the template argm.
		*/
		virtual tQueue isMember(const IVarName *,const IVarName *,const IObjectIA &) const;
		/**
		isClassInheritedFrom(className) allow us to known if this class have a base class with the name className.

		If true the method return the range of the base class, this range is an arbitrary reference id define by the programmer or bay the scrypt.
		the false return is define by -1.
		*/
		virtual sint32 isClassInheritedFrom(const IVarName &) const;

		/**
		The methode runMethodeMember(sint32 heritance, sint32 index,  IObjectIA *param) allow us to run a member method.

		heritance: is the id returned by the methodisClassInheritedFrom(const IVarName &).
		index: is the method id return by the isMember(const IVarName *,const IVarName *,const IObjectIA &) const method.
		param: is the parametre for the method.
		*/
		virtual	CProcessResult runMethodeMember(sint32, sint32, IObjectIA *);

		/**
		The methode getMethodeMemberDebugString(sint32 heritance, sint32 index)
		return a string that define the name and the argument of the method
		*/
		virtual	std::string getMethodeMemberDebugString(sint32, sint32) const;

		/**
		The methode runMethodeMember(sint32 index,IObjectIA *param) allow us to run a own member method.
		
		index: is the method id return by the isMember(const IVarName *,const IVarName *,const IObjectIA &) const method.
		param: is the parametre for the method.
		*/
		virtual	CProcessResult runMethodeMember(sint32 index,IObjectIA *);

		/**
		The method sendMessage(const IObjectIA &msg) allow us to achive a message defined in msg.
		*/		
		virtual	CProcessResult sendMessage(IObjectIA *);

		/**
		This method allow to send a message to an compoment given by an string IVarName.
		*/
		virtual	CProcessResult sendMessage(const IVarName &,IObjectIA *);
		
		/**
		The method canProcessMessage(const IVarName &msgName) allow us to know if agent can preocess given by msgName.

		tQueue contain the liste of method (same as a run(msg)) that coud process the msg.
		*/
		virtual	tQueue canProcessMessage(const IVarName &);

		///If the result is true the object is loclal else its a representation of an object on the server. By the method return true.
		virtual bool isLocal();

		virtual ~IObjectIA();
	};	
	
	/**
	Class IRefrence is the main class for an agent it define the word refrence of an agents.
	* \author Chafik sameh	 	
	* \author Nevrax France
	* \date 2000
	*/
	class IRefrence: public IObjectIA
	{	
	private:
		///_NumRef is the word refrence of the agents.
		IWordNumRef *_NumRef;
		///_Parent is the word refrence of the parent agents if existe.
		IWordNumRef *_Parent;

	protected:
		IRefrence(const IRefrence &A);
		///the loader of the class.
		void refLoadStream(NLMISC::IStream &is);
		void setNumRef(const IWordNumRef &ref);

	public:
		///default constructor, when it's called an new reference is created.
		IRefrence();
		///We can call this constructor when agent have a parent.
		IRefrence(const IWordNumRef *parent);
		///We can call this constructor when we want to load a new agents from a stream.
		IRefrence(NLMISC::IStream &is);

		///to get a word num ref of the agents.
		operator const IWordNumRef &() const;
		///to get a word num ref pointer of the agents.
		operator const IWordNumRef *() const ;

		virtual const CIdent getIdentType() const;

		///to get the agent parents
		IRefrence *getParent() const;

		/// to set the parent of the agents.
		virtual void setParent(const IWordNumRef *parent);
		/// To get the top level parent of this agent.
		virtual const IRefrence *getOwner() const;
		
		/// \name IBasicInterface method.
		/**
		We have to force programmer to implement the save and load method, but class have to save own attributs.
		*/
		//@{
		virtual void save(NLMISC::IStream &os) = 0;
		virtual void load(NLMISC::IStream &is) = 0;
		//@}


		///That function allow user to change the 7 bit of the type field in the sid agent number;
		void setTypeAt(uint64 t);		

		virtual ~IRefrence();
	};

	/**
	In general agents create a lot of connection with ather agent and for exemple when he die, he have to close all this connections. 
	IConnectIA allow programmer to manage connection. the IConnectIA class work with tow list the connection list and the connected list.
	Connection list: is the list of agent connected to this object.
	Connectioned list: is the list of objects where this agent is connected to.

	* \author Chafik sameh	 
	* \author Portier Pierre
	* \author Nevrax France
	* \date 2000
	*/
	class IConnectIA: public IRefrence
	{
	private:
		typedef std::list<const IConnectIA *> tListiBasic;
		typedef std::list<const IConnectIA *>::iterator tListiBasicItr;
		typedef std::list<const IConnectIA *>::const_iterator tListiBasicCstItr;

	protected:
		tListiBasic _Connection;
		tListiBasic _Connected;		

	protected:
		IConnectIA(const IConnectIA &A);
		void connectLoadStream(NLMISC::IStream &is);

	public:

		IConnectIA();
		IConnectIA(const IWordNumRef *parent);
		IConnectIA(NLMISC::IStream &is);
		IConnectIA *getParent() const;
				
		///When agent die and when he is in the Connectioned lis, he call this method to inform this state to the objects.
		virtual void onKill(IConnectIA *);	
		///\name State of agent.
		///State is a CProcessResult objects it define the state of agent after the run method.
		//@{
		virtual const CProcessResult &getState() const = 0;
		virtual void setState(TProcessStatement s, IObjectIA *result) = 0;		
		//@}		

		///sendMessage allow agent to send a message to an ather agent.
		///The message is a IMessageBase.
		virtual	IObjectIA::CProcessResult sendMessage(IObjectIA *m)
		{
			return sendMessage((IMessageBase *)m);
		}
		virtual IObjectIA::CProcessResult sendMessage(IMessageBase *msg) = 0;
		
		/// \name IBasicInterface method.
		/**
		programmer have to call this method to load or save internal attribut.
		*/
		//@{
		virtual void save(NLMISC::IStream &os) = 0;
		virtual void load(NLMISC::IStream &is) = 0;
		//@}

		///When agent want to make connection he have to call this method to connect to an agent.
		///When it call the B object is added in the connection list.
		void connect(IConnectIA *b);
		///To release a connection.
		void removeConnection(IConnectIA *Agent);

		virtual void setParent(const IWordNumRef *parent);

		///this method define wat agent have to whene he die.
		virtual void Kill(); 

		virtual ~IConnectIA();

	protected:		
		
		/// \name list manipulation method.
		//@{
		void addInConnectionList(const IConnectIA *a);
		void removeInConnectionList(IConnectIA *a);
		void addInConnectedList(const IConnectIA *a);
		void removeInConnectedList(const IConnectIA *a);
		//@}

	};
}
#endif
