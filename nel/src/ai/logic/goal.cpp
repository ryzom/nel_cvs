#include "nel/ai/logic/goal.h"
#include "nel/ai/logic/var.h"
#include "nel/ai/agent/object_type.h"
#include "nel/ai/logic/operator_script.h"

namespace NLAILOGIC
{
	CGoal::CGoal() : IBaseBoolType()
	{
		_Name = NULL;
		_Sender = NULL;
		_Receiver = NULL;
		_Selected = false;
		_Mode = achieveOnce;
	}
		
	CGoal::CGoal(const NLAIAGENT::IVarName &name, TTypeOfGoal mode) : IBaseBoolType()
	{
		_Name = (NLAIAGENT::IVarName *) name.clone();
		_Sender = NULL;
		_Receiver = NULL;
		_Mode = mode;
		_Selected = false;
	}

	CGoal::CGoal(const NLAIAGENT::IVarName &name, std::list<const NLAIAGENT::IObjectIA *> &args, TTypeOfGoal mode)
	{
		_Name = (NLAIAGENT::IVarName *) name.clone();
		while ( !args.empty() )
		{
			_Args.push_back( (NLAIAGENT::IObjectIA *) args.front()->clone() );
			args.pop_front();
		}
		_Sender = NULL;
		_Receiver = NULL;
		_Mode = mode;
		_Selected = false;
	}

	CGoal::CGoal(const CGoal &c) : IBaseBoolType()
	{
		if ( c._Name )
			_Name = (NLAIAGENT::IVarName *) c._Name->clone();
		else
			_Name = NULL;
		_Sender =c._Sender;
		_Receiver = c._Receiver;
		_Mode = c._Mode;

		for ( int i = 0; i < (int) c._Args.size(); i++ )
			_Args.push_back( (NLAIAGENT::IObjectIA *) c._Args[i]->clone() );

		_Selected = c._Selected;
	}

	CGoal::~CGoal()
	{
		if ( _Name )
			_Name->release();

		int i;		
		for ( i = 0; i < (int) _Args.size(); i++ )
			_Args[i]->release();
		
		std::vector<NLAIAGENT::IBasicAgent *>::iterator it_s = _Successors.begin();
		while ( it_s != _Successors.end() )
		{			
			(*it_s ++)->release();			
		}
	}

	void CGoal::failure()
	{
		for ( int i = 0; i < (int) _Successors.size(); i++ )
		{
			// TODO: Envoi de message "failed"
		}
	}

	void CGoal::success()
	{
		for ( int i = 0; i < (int) _Successors.size(); i++ )
		{
			// TODO: Envoi de message "success"
		}
	}

	void CGoal::operatorSuccess(NLAIAGENT::IBasicAgent *op)
	{
/*
#ifdef NL_DEBUG
		std::string buffer;
		op->getDebugString(buffer);
#endif
*/

		nlinfo("operatorSuccess: 0x%0x, %d, (0x%0x, %s)", this, _Successors.size(),op,(const char *)op->getType());
		std::vector<NLAIAGENT::IBasicAgent *>::iterator it_s = _Successors.begin();
		while ( it_s != _Successors.end() )
		{			
/*			if ( op != *it_s )
			{
				nlinfo("Problème!!!");
			}
			nlinfo("\t0x%0x",*it_s);
			nlinfo("\t\t%s", (const char *)(*it_s)->getType());
*/
			if ( (**it_s) == *op )
			{
				(*it_s)->release();
				_Successors.erase( it_s );
				break;
			}
			it_s++;
		}

		switch ( _Mode )
		{
			case achieveOnce:
				( (NLAIAGENT::CAgentScript *) _Receiver)->removeGoal( this );
				break;

			case achieveForever:
				break;
		}

	}

	void CGoal::operatorFailure(NLAIAGENT::IBasicAgent *op)
	{
		std::vector<NLAIAGENT::IBasicAgent *>::iterator it_s = _Successors.begin();
		while ( it_s != _Successors.end() )
		{
			if ( (**it_s) == *op )
			{
				(*it_s)->release();
				_Successors.erase( it_s );
				break;
			}
			it_s++;
		}

		switch ( _Mode )
		{
			case achieveOnce:
				( (NLAIAGENT::CAgentScript *) _Receiver )->removeGoal( this );
				break;

			case achieveForever:
				break;
		}
	}

	const NLAIC::IBasicType *CGoal::clone() const
	{
		NLAIC::IBasicInterface *m = new CGoal( *this );
		return m;
	}

	const NLAIC::IBasicType *CGoal::newInstance() const
	{
		return clone();
	}


	void CGoal::save(NLMISC::IStream &os)
	{			
	}

	void CGoal::load(NLMISC::IStream &is)
	{
		IObjectIA::load( is );
	}

	void CGoal::getDebugString(std::string &text) const
	{
		text += "<CGoal> (";
		if ( _Name ) 
			text += _Name->getString();
		std::string buf;
		int i;
		for ( i = 0; i < (int) _Args.size(); i++ )
		{
			_Args[i]->getDebugString(buf);
			text += " ";
			text += buf;
		}
		text += ") - ";

		text += NLAIC::stringGetBuild(" PRI = %f", priority() );
		text += " PRE = ";

		for ( i = 0; i < (int) _Predecessors.size(); i++ )
		{
			_Predecessors[i]->getDebugString( buf );
			text += buf;
		}
		text += " POST = ";
		for ( i = 0; i < (int) _Successors.size(); i++ )
		{
			_Successors[i]->getDebugString( buf );
			text += buf;
		}
	}

	bool CGoal::isTrue() const
	{
		return false;
	}

	float CGoal::truthValue() const
	{
		return 1.0;
	}

	const NLAIAGENT::IObjectIA::CProcessResult &CGoal::run()
	{
		return IObjectIA::ProcessRun;
	}

	bool CGoal::isEqual(const CGoal &a) const
	{
		return false;
	}

	bool CGoal::isEqual(const NLAIAGENT::IBasicObjectIA &a) const
	{
		return false;
	}

	const std::vector<NLAIAGENT::IObjectIA *> &CGoal::getArgs()
	{
		return _Args;
	}

	const NLAIC::CIdentType &CGoal::getType() const
	{
		return IdGoal;
	}

	void CGoal::setArgs(std::list<NLAIAGENT::IObjectIA *> &args)
	{
		std::list<NLAIAGENT::IObjectIA *>::iterator it_var = args.begin();
		while ( it_var != args.end() )
		{
			_Args.push_back( (NLAIAGENT::IObjectIA *) (*it_var)->clone() );
			it_var++;
		}
	}

	NLAIAGENT::tQueue CGoal::isMember(const NLAIAGENT::IVarName *className,const NLAIAGENT::IVarName *funcName,const NLAIAGENT::IObjectIA &params) const
	{

#ifdef NL_DEBUG	
	std::string nameP;
	std::string nameM;
	funcName->getDebugString(nameM);
	params.getDebugString(nameP);
	const char *dbg_class_name = (const char *) getType();
#endif
		static NLAIAGENT::CStringVarName constructor_name("Constructor");
		static NLAIAGENT::CStringVarName mode_once_name("SetModeOnce");
		static NLAIAGENT::CStringVarName mode_repeat_name("SetModeRepeat");
		NLAIAGENT::tQueue r;
		if(className == NULL)
		{
			if( (*funcName) == constructor_name )
			{					
				NLAIAGENT::CObjectType *c = new NLAIAGENT::CObjectType( new NLAIC::CIdentType( CGoal::IdGoal ) );					
				r.push( NLAIAGENT::CIdMethod( 0 + IObjetOp::getMethodIndexSize(), 0.0, NULL, c) );					
			}

			if( (*funcName) == mode_once_name )
			{					
				NLAIAGENT::CObjectType *c = new NLAIAGENT::CObjectType( new NLAIC::CIdentType( CGoal::IdGoal ) );					
				r.push( NLAIAGENT::CIdMethod( 1 + IObjetOp::getMethodIndexSize(), 0.0, NULL, c) );					
			}

			if( (*funcName) == mode_repeat_name )
			{					
				NLAIAGENT::CObjectType *c = new NLAIAGENT::CObjectType( new NLAIC::CIdentType( CGoal::IdGoal ) );					
				r.push( NLAIAGENT::CIdMethod( 2 + IObjetOp::getMethodIndexSize(), 0.0, NULL, c) );					
			}

		}

		if ( r.empty() )
			return IBaseBoolType::isMember(className, funcName, params);
		else
			return r;
	}

	///\name Some IObjectIA method definition.
	//@{		
	NLAIAGENT::IObjectIA::CProcessResult CGoal::runMethodeMember(sint32, sint32, NLAIAGENT::IObjectIA *)
	{
		return IObjectIA::CProcessResult();
	}

	NLAIAGENT::IObjectIA::CProcessResult CGoal::runMethodeMember(sint32 index, NLAIAGENT::IObjectIA *p)
	{
		NLAIAGENT::IBaseGroupType *param = (NLAIAGENT::IBaseGroupType *)p;

		switch(index - IObjetOp::getMethodIndexSize())
		{
			case 0:
				{					

					NLAIAGENT::CStringType *name = (NLAIAGENT::CStringType *) param->getFront();
					param->popFront();
#ifdef NL_DEBUG
					const char *dbg_name = name->getStr().getString();
#endif
					// If the constructor() function is explicitely called and the object has already been initialised
					if ( _Name )
						_Name->release();
					_Args.clear();

					_Name = (NLAIAGENT::IVarName *) name->getStr().clone();
					std::list<const NLAIAGENT::IObjectIA *> args;
					while ( param->size() )
					{
						_Args.push_back( (NLAIAGENT::IObjectIA *) param->getFront()->clone() );
						param->popFront();
					}
					name->release();
					return IObjectIA::CProcessResult();		
				} 
				break;
			
			case 1:
				_Mode = achieveOnce;
				break;

			case 2:
				_Mode = achieveForever;
				break;
		}

		return IObjectIA::CProcessResult();
	}

	sint32 CGoal::getMethodIndexSize() const
	{
		return IBaseBoolType::getMethodIndexSize() + 3;
	}
	//@}

	void CGoal::addSuccessor(NLAIAGENT::IBasicAgent *s)
	{		
		nlinfo("addSuccessor: 0x%0x, %d, (0x%0x, %s)", this, _Successors.size(),s,(const char *)s->getType());
		_Successors.push_back(s);
		s->incRef();
	}

	void CGoal::addPredecessor(NLAIAGENT::IBasicAgent *p)
	{
		_Predecessors.push_back(p);
		p->incRef();
	}

	bool CGoal::operator==(const CGoal &g)
	{
		if ( (*g._Name) == (*_Name) && _Args.size() == g._Args.size() )
			return true;

		return false;
	}

	void CGoal::setSender(NLAIAGENT::IBasicAgent *s)
	{
		_Sender = s;
	}

	void CGoal::setReceiver(NLAIAGENT::IBasicAgent *r)
	{
		_Receiver = r;
	}

	NLAIAGENT::IBasicAgent *CGoal::getSender()
	{
		return _Sender;
	}

	NLAIAGENT::IBasicAgent *CGoal::getReceiver()
	{
		return _Receiver;
	}

	void CGoal::cancel()
	{
		std::vector<NLAIAGENT::IBasicAgent *>::iterator i, end = _Successors.end();
		for ( i = _Successors.begin(); i != end; i++ )		
		{
			( (NLAIAGENT::COperatorScript *)*i )->cancel();
		}
	}

	float CGoal::priority() const
	{
		if ( _Successors.empty() )
			return 0.0;

		float pri = 256;		
		std::vector<NLAIAGENT::IBasicAgent *>::const_iterator i, end = _Successors.end();
		for ( i = _Successors.begin(); i != end; i++ )
		{
			float suc_pri = ( (NLAIAGENT::COperatorScript *)*i )->priority();
			if ( suc_pri < pri )
			{
				pri = suc_pri;
			}
		}
		return pri;
	}

	bool CGoal::isExclusive()
	{
		if ( _Successors.empty() )
			return false;
		else
			return ( (NLAIAGENT::COperatorScript *)_Successors.front())->isExclusive();
	}
}
