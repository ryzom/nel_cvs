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
	}
		
	CGoal::CGoal(const NLAIAGENT::IVarName &name) : IBaseBoolType()
	{
		_Name = (NLAIAGENT::IVarName *) name.clone();
		_Sender = NULL;
		_Receiver = NULL;
	}

	CGoal::CGoal(const NLAIAGENT::IVarName &name, std::list<const NLAIAGENT::IObjectIA *> &args)
	{
		_Name = (NLAIAGENT::IVarName *) name.clone();
		while ( !args.empty() )
		{
			_Args.push_back( (NLAIAGENT::IObjectIA *) args.front() );
			args.pop_front();
		}
		_Sender = NULL;
		_Receiver = NULL;
	}

	CGoal::CGoal(const CGoal &c) : IBaseBoolType()
	{
		if ( c._Name )
			_Name = (NLAIAGENT::IVarName *) c._Name->clone();
		else
			_Name = NULL;
		_Sender = NULL;
		_Receiver = NULL;

		for ( int i = 0; i < (int) c._Args.size(); i++ )
			_Args.push_back( (NLAIAGENT::IObjectIA *) c._Args[i]->clone() );
	}

	CGoal::~CGoal()
	{
		if ( _Name )
			_Name->release();

		int i;		for ( i = 0; i < (int) _Args.size(); i++ )
			_Args[i]->release();
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

	void CGoal::operatorSucces(NLAIAGENT::IBasicAgent *)
	{

	}

	void CGoal::operatorFailure(NLAIAGENT::IBasicAgent *)
	{

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

	void CGoal::getDebugString(char *text) const
	{
		strcpy( text ,"<CGoal> (");
		if ( _Name ) 
			strcat( text, _Name->getString() );
		char buf[1024];
		int i;
		for ( i = 0; i < (int) _Args.size(); i++ )
		{
			_Args[i]->getDebugString(buf);
			strcat(text," ");
			strcat(text, buf);
		}
		strcat(text,") - ");

		sprintf(buf," PRI = %f", priority() );
		strcat(text, buf);
		strcat(text," PRE = ");

		for ( i = 0; i < (int) _Predecessors.size(); i++ )
		{
			_Predecessors[i]->getDebugString( buf );
			strcat(text, buf);
		}
		strcat(text," POST = ");
		for ( i = 0; i < (int) _Successors.size(); i++ )
		{
			_Successors[i]->getDebugString( buf );
			strcat(text, buf);
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
		return false; //( a._Value == _Value );
	}

	bool CGoal::isEqual(const NLAIAGENT::IBasicObjectIA &a) const
	{
		return false;//( ((CGoal &)a)._Value == _Value );
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
	char nameP[1024*4];
	char nameM[1024*4];
	funcName->getDebugString(nameM);
	params.getDebugString(nameP);

	const char *dbg_class_name = (const char *) getType();
#endif
		NLAIAGENT::tQueue r;
		if(className == NULL)
		{
			if( (*funcName) == NLAIAGENT::CStringVarName( "Constructor" ) )
			{					
				NLAIAGENT::CObjectType *c = new NLAIAGENT::CObjectType( new NLAIC::CIdentType( CGoal::IdGoal ) );					
				r.push( NLAIAGENT::CIdMethod( 0 + IObjetOp::getMethodIndexSize(), 0.0, NULL, c) );					
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

				NLAIAGENT::CStringType *name = (NLAIAGENT::CStringType *) param->getFront()->clone();
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
					_Args.push_back( (NLAIAGENT::IObjectIA *) param->getFront() );
					param->popFront();
				}
				return IObjectIA::CProcessResult();		
			}
			break;
		}

		return IObjectIA::CProcessResult();
	}

	sint32 CGoal::getMethodIndexSize() const
	{
		return IBaseBoolType::getMethodIndexSize() + 1;
	}
	//@}

	void CGoal::addSuccessor(NLAIAGENT::IBasicAgent *s)
	{
		_Successors.push_back(s);
	}

	void CGoal::addPredecessor(NLAIAGENT::IBasicAgent *p)
	{
		_Predecessors.push_back(p);
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
		for ( int i = 0; i < (int) _Successors.size(); i++ )
		{
			( (NLAIAGENT::COperatorScript *)_Successors[i] )->cancel();
		}
	}

	float CGoal::priority() const
	{
		return 1.0;
	}
}
