#include "nel/ai/logic/fsm_script.h"
#include "nel/ai/agent/msg_action.h"
#include "nel/ai/agent/agent.h"
#include "nel/ai/agent/agent_script.h"
#include "nel/ai/agent/object_type.h"
#include "nel/ai/script/codage.h"
#include "nel/ai/agent/gd_agent_script.h"
//#include "nel/ai/logic/interpret_object_operator.h"
#include "nel/ai/script/interpret_message_action.h"

namespace NLAIAGENT
{
	static CGroupType listBidon;

	/////////////////////////////////////////////////////////////
	// Succes and failure messages declaration

	static NLAISCRIPT::COperandSimpleListOr *ParamIdSuccessMsg = new NLAISCRIPT::COperandSimpleListOr(2,
															  new NLAIC::CIdentType(NLAIAGENT::CSuccessMsg::IdSuccessMsg),
															  new NLAIC::CIdentType(NLAISCRIPT::CSuccessMsgClass::IdSuccessMsgClass)	);

	static NLAISCRIPT::CParam *ParamSuccessMsg = new NLAISCRIPT::CParam(1,ParamIdSuccessMsg);

	
	

	static NLAISCRIPT::COperandSimpleListOr *ParamIdFailureMsg = new NLAISCRIPT::COperandSimpleListOr(2,
															  new NLAIC::CIdentType(NLAIAGENT::CFailureMsg::IdFailureMsg),
															  new NLAIC::CIdentType(NLAISCRIPT::CFailureMsgClass::IdFailureMsgClass)	);

	static NLAISCRIPT::CParam *ParamFailureMsg = new NLAISCRIPT::CParam(1,ParamIdFailureMsg);


	/////////////////////////////////////////////////////////////


	CFsmScript::CFsmScript(const CFsmScript &a) : CAgentScript(a)
	{
	}

	CFsmScript::CFsmScript(IAgentManager *manager, 
							   IBasicAgent *father,
							   std::list<IObjectIA *> &components,	
							   NLAISCRIPT::CFsmClass *actor_class )
	: CAgentScript(manager, father, components, actor_class )
	{	
	}	

	CFsmScript::CFsmScript(IAgentManager *manager, bool stay_alive) : CAgentScript( manager )
	{
	}

	CFsmScript::~CFsmScript()
	{
	}

	const NLAIC::IBasicType *CFsmScript::clone() const
	{		
		CFsmScript *m = new CFsmScript(*this);
		return m;
	}		

	const NLAIC::IBasicType *CFsmScript::newInstance() const
	{	
		CFsmScript *instance;
		if ( _AgentClass )
		{
			instance = (CFsmScript *) _AgentClass->buildNewInstance();
		}
		else 
		{			
			instance = new CFsmScript(NULL);
		}
		return instance;
	}
	
	void CFsmScript::getDebugString(char *t) const
	{
		if ( _AgentClass )
		{
			char buf[1024 * 4];
			_AgentClass->getDebugString(buf);
			strcpy(t,buf);
		}
		else
			strcpy(t,"<CFsmScript>");
	}

	bool CFsmScript::isEqual(const IBasicObjectIA &a) const
	{
		return true;
	}

	const NLAIC::CIdentType &CFsmScript::getType() const
	{		
		return IdFsmScript;
	}

	void CFsmScript::save(NLMISC::IStream &os)
	{
		CAgentScript::save(os);
		// TODO
	}

	void CFsmScript::load(NLMISC::IStream &is)
	{
		CAgentScript::load(is);
		// TODO
	}

	sint32 CFsmScript::getMethodIndexSize() const
	{
		return CAgentScript::getBaseMethodCount();
	}

/*	IObjectIA::CProcessResult CFsmScript::runMethodBase(int index,int heritance, IObjectIA *params)
	{		
		IObjectIA::CProcessResult r;
*/
/*		if ( index == fid_activate )
		{
			activate();
			IObjectIA::CProcessResult r;
			r.ResultState =  NLAIAGENT::processIdle;
			r.Result = NULL;
		}
*/
/*		return CAgentScript::runMethodeBase(heritance,index,params);
	}

*/

	IObjectIA::CProcessResult CFsmScript::runMethodBase(int index,IObjectIA *params)
	{	


		int i = index - IAgent::getMethodIndexSize();

		if ( i < getBaseMethodCount() )
			return CAgentScript::runMethodBase(index, params);

		switch ( i )
		{
			case 1:
				onSuccess(params);
				break;
			case 2:
				onFailure(params);
				break;
		}

		IObjectIA::CProcessResult r;

#ifdef NL_DEBUG
		char buf[1024];
		getDebugString(buf);
#endif

		return r;
	}

	int CFsmScript::getBaseMethodCount() const
	{
		return CAgentScript::getBaseMethodCount();
	}


	tQueue CFsmScript::isMember(const IVarName *className,const IVarName *name,const IObjectIA &param) const
	{		
#ifdef NL_DEBUG
		const char *dbg_func_name = name->getString();
#endif

		tQueue result = CAgentScript::isMember( className, name, param);

		if ( result.size() )
			return result;

		// Processes succes and failure functions
		if ( *name == CStringVarName("RunTell") )
		{
			double d;
			d = ((NLAISCRIPT::CParam &)*ParamSuccessMsg).eval((NLAISCRIPT::CParam &)param);
			
			if ( d >= 0.0 )
			{
				NLAIAGENT::CObjectType *r_type = new NLAIAGENT::CObjectType( new NLAIC::CIdentType( NLAIC::CIdentType::VoidType ) );
				result.push( NLAIAGENT::CIdMethod(  IAgent::getMethodIndexSize() + 1, 0.0,NULL, r_type ) );
			}

			d = ((NLAISCRIPT::CParam &)*ParamFailureMsg).eval((NLAISCRIPT::CParam &)param);
			if ( d >= 0.0 )
			{
				NLAIAGENT::CObjectType *r_type = new NLAIAGENT::CObjectType( new NLAIC::CIdentType( NLAIC::CIdentType::VoidType ) );
				result.push( NLAIAGENT::CIdMethod(  IAgent::getMethodIndexSize() + 2, 0.0,NULL, r_type ) );
			}
		}

		// Returns a list of active states
		if ( *name == CStringVarName("getActive") )
		{
			
		}

		return result;
	}

/*	void CFsmScript::setParent(const IWordNumRef *parent)
	{
		// Gets the father's factbase
		const IRefrence *father = (const IRefrence *) *parent;
		NLAILOGIC::CFactBase &fact_base = ( (CAgentScript *)father )->getFactBase();
		
		// Adds the needed asserts to the factbase
		((NLAISCRIPT::COperatorClass *) _AgentClass)->initialiseFactBase( &fact_base );
				
		// Sets the parent
		IRefrence::setParent(parent);		
	}
*/
	const IObjectIA::CProcessResult &CFsmScript::run()
	{
		return CAgentScript::run();
	}

	void CFsmScript::activate(IBasicAgent *child)
	{
		_ActiveChilds.push_back(child);
	}

	void CFsmScript::unactivate(IBasicAgent *child)
	{
		std::vector<IBasicAgent *>::iterator it_c = _ActiveChilds.begin();
		while ( it_c != _ActiveChilds.end() )
		{
			if ( child == *it_c )
			{
				_ActiveChilds.erase( it_c );
				return;
			}
			it_c++;
		}
	}

	void CFsmScript::runChildren()
	{
#ifdef NL_DEBUG	
	const char *classBase = (const char *)getType();
#endif
		std::vector<IBasicAgent *>::iterator it_c = _ActiveChilds.begin();
		while ( it_c != _ActiveChilds.end() )
		{
			(*it_c)->run();
			it_c++;
		}

		// Activation des fils
//		IAgent::runChildren();
	}

	void CFsmScript::onSuccess( IObjectIA *)
	{
		int i = 10;
	}

	void CFsmScript::onFailure( IObjectIA *)
	{
		int i = 10;
	}
}
