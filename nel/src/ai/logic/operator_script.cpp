#include "nel/ai/logic/operator_script.h"
#include "nel/ai/agent/agent_script.h"
#include "nel/ai/agent/object_type.h"
#include "nel/ai/script/codage.h"

namespace NLAIAGENT
{
	static CGroupType listBidon;

	COperatorScript::COperatorScript(const COperatorScript &a) : CAgentScript(a)
	{
	}

	COperatorScript::COperatorScript(IAgentManager *manager, 
							   IBasicAgent *father,
							   std::list<IObjectIA *> &components,	
							   NLAISCRIPT::COperatorClass *actor_class )
	: CAgentScript(manager, father, components, actor_class )
	{	
	}	

	COperatorScript::COperatorScript(IAgentManager *manager, bool stay_alive) : CAgentScript( manager )
	{
	}

	COperatorScript::~COperatorScript()
	{
	}

	const NLAIC::IBasicType *COperatorScript::clone() const
	{		
		COperatorScript *m = new COperatorScript(*this);
		return m;
	}		

	const NLAIC::IBasicType *COperatorScript::newInstance() const
	{	
		COperatorScript *instance;
		if ( _AgentClass )
		{
			instance = (COperatorScript *) _AgentClass->buildNewInstance();
		}
		else 
		{			
			instance = new COperatorScript(NULL);
		}
		return instance;
	}
	
	void COperatorScript::getDebugString(char *t) const
	{
		strcpy(t,"COperatorScript ");
	}

	bool COperatorScript::isEqual(const IBasicObjectIA &a) const
	{
		return true;
	}

	const NLAIC::CIdentType &COperatorScript::getType() const
	{		
		return IdOperatorScript;
	}

	void COperatorScript::save(NLMISC::IStream &os)
	{
		CAgentScript::save(os);
		// TODO
	}

	void COperatorScript::load(NLMISC::IStream &is)
	{
		CAgentScript::load(is);
		// TODO
	}

	sint32 COperatorScript::getMethodIndexSize() const
	{
		return CAgentScript::getBaseMethodCount();
	}

	IObjectIA::CProcessResult COperatorScript::runMethodBase(int index,int heritance, IObjectIA *params)
	{		
		IObjectIA::CProcessResult r;

/*		if ( index == fid_activate )
		{
			activate();
			IObjectIA::CProcessResult r;
			r.ResultState =  NLAIAGENT::processIdle;
			r.Result = NULL;
		}
*/
		return CAgentScript::runMethodeMember(heritance,index,params);
	}



	IObjectIA::CProcessResult COperatorScript::runMethodBase(int index,IObjectIA *params)
	{	

		index = index - IAgent::getMethodIndexSize();
/*

		if ( index < getBaseMethodCount() )
			return CAgentScript::runMethodeMember(index, params);
*/
		IObjectIA::CProcessResult r;

		char buf[1024];
		getDebugString(buf);

//		index = index - getBaseMethodCount();
		
/*		if ( index == fid_activate )
		{
			activate();
			IObjectIA::CProcessResult r;
			r.ResultState =  NLAIAGENT::processIdle;
			r.Result = NULL;
		}

		if ( index == fid_onActivate )
		{
			onActivate();
			IObjectIA::CProcessResult r;
			r.ResultState =  NLAIAGENT::processIdle;
			r.Result = NULL;
		}

		if ( index == fid_unActivate )
		{
			unActivate();
			IObjectIA::CProcessResult r;
			r.ResultState =  NLAIAGENT::processIdle;
			r.Result = NULL;
		}

		if ( index == fid_onUnActivate )
		{
			onUnActivate();
			IObjectIA::CProcessResult r;
			r.ResultState =  NLAIAGENT::processIdle;
			r.Result = NULL;
		}

		if ( index == fid_switch )
		{
			std::vector<CStringType *> handles;
			if ( ( (NLAIAGENT::IBaseGroupType *) params)->size() )
			{
#ifdef _DEBUG
				const char *dbg_param_type = (const char *) params->getType();
				char dbg_param_string[1024 * 8];
				params->getDebugString(dbg_param_string);
#endif
				const IObjectIA *fw = ( ((NLAIAGENT::IBaseGroupType *)params) )->getFront();
#ifdef _DEBUG
				const char *dbg_param_front_type = (const char *) fw->getType();
#endif

				( ((NLAIAGENT::IBaseGroupType *)params))->popFront();
//				while ( fw->size() )
//				{
					handles.push_back( (CStringType *) fw);
//					fw->popFront();
//				}

				std::vector<CComponentHandle *> switched;
				for ( int i = 0; i < (int) handles.size(); i++)
					switched.push_back( new CComponentHandle( handles[ i ]->getStr(), (IAgent *) getParent() ) );
				switchActor( switched, false );
			}
			IObjectIA::CProcessResult r;
			r.ResultState =  NLAIAGENT::processIdle;
			r.Result = NULL;
		}
		*/
		return r;
	}

	int COperatorScript::getBaseMethodCount() const
	{
		return CAgentScript::getBaseMethodCount();
	}


	tQueue COperatorScript::isMember(const IVarName *className,const IVarName *name,const IObjectIA &param) const
	{		

		const char *txt = name->getString();

		tQueue result = CAgentScript::isMember( className, name, param);

		if ( result.size() )
			return result;
/*
		if ( *name == CStringVarName("activate") )
		{
			NLAIAGENT::CObjectType *r_type = new NLAIAGENT::CObjectType( new NLAIC::CIdentType( NLAIC::CIdentType::VoidType ) );
			result.push( NLAIAGENT::CIdMethod(  IAgent::getMethodIndexSize() + fid_activate, 0.0,NULL, r_type ) );
		}

		if ( *name == CStringVarName("onActivate") )
		{
			NLAIAGENT::CObjectType *r_type = new NLAIAGENT::CObjectType( new NLAIC::CIdentType( NLAIC::CIdentType::VoidType ) );
			result.push( NLAIAGENT::CIdMethod( IAgent::getMethodIndexSize() + fid_onActivate , 0.0,NULL, r_type ) );
		}


		if ( *name == CStringVarName("unActivate") )
		{
			CObjectType *r_type = new CObjectType( new NLAIC::CIdentType( NLAIC::CIdentType::VoidType ) );
			result.push( NLAIAGENT::CIdMethod( IAgent::getMethodIndexSize() + fid_onUnActivate, 0.0,NULL, r_type ) );
		}

		if ( *name == CStringVarName("onUnActivate") )
		{
			CObjectType *r_type = new CObjectType( new NLAIC::CIdentType( NLAIC::CIdentType::VoidType ) );
			result.push( NLAIAGENT::CIdMethod( IAgent::getMethodIndexSize() + fid_unActivate, 0.0,NULL, r_type ) );
		}

		if ( *name == CStringVarName("switch") )
		{
			CObjectType *r_type = new CObjectType( new NLAIC::CIdentType( NLAIC::CIdentType::VoidType ) );
			result.push( NLAIAGENT::CIdMethod( IAgent::getMethodIndexSize() + fid_switch, 0.0, NULL, r_type ) );
		}

		if(_AgentClass != NULL)
		{
			tQueue r = _AgentClass->isMember(className, name, param);
			if(r.size() != 0) return r;
		}
		*/
		return result;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////

}
