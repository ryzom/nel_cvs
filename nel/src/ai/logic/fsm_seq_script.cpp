#include "nel/ai/logic/fsm_seq_script.h"
#include "nel/ai/agent/agent.h"
#include "nel/ai/agent/agent_script.h"
#include "nel/ai/agent/object_type.h"
#include "nel/ai/script/codage.h"
#include "nel/ai/script/interpret_fsm.h"
#include "nel/ai/agent/agent_method_def.h"
#include "nel/ai/agent/comp_handle.h"

namespace NLAIAGENT
{
	static CGroupType listBidon;

	CSeqFsmScript::CSeqFsmScript(const CSeqFsmScript &a) : CFsmScript(a)
	{
		_ActiveChild = NULL;
		_IdActiveChild = 0;
	}

	CSeqFsmScript::CSeqFsmScript(IAgentManager *manager, 
							   IBasicAgent *father,
							   std::list<IObjectIA *> &components,	
							   NLAISCRIPT::CFsmClass *actor_class )
	: CFsmScript(manager, father, components, actor_class )
	{	
		_ActiveChild = NULL;
		_IdActiveChild = 0;
	}	

	CSeqFsmScript::CSeqFsmScript(IAgentManager *manager, bool stay_alive) : CFsmScript( manager )
	{
		_ActiveChild = NULL;
		_IdActiveChild = 0;
	}

	CSeqFsmScript::~CSeqFsmScript()
	{
	}

	const NLAIC::IBasicType *CSeqFsmScript::clone() const
	{		
		CSeqFsmScript *m = new CSeqFsmScript(*this);
		return m;
	}		

	const NLAIC::IBasicType *CSeqFsmScript::newInstance() const
	{	
		CSeqFsmScript *instance;
		if ( _AgentClass )
		{
			instance = (CSeqFsmScript *) _AgentClass->buildNewInstance();
		}
		else 
		{			
			instance = new CSeqFsmScript(NULL);
		}
		return instance;
	}
	
	void CSeqFsmScript::getDebugString(std::string &t) const
	{
		if ( _AgentClass )
		{
			std::string buf;
			_AgentClass->getDebugString(buf);
			t += buf;
		}
		else
			t += "<CSeqFsmScript>";
	}

	bool CSeqFsmScript::isEqual(const IBasicObjectIA &a) const
	{
		return true;
	}

	const NLAIC::CIdentType &CSeqFsmScript::getType() const
	{		
		return IdSeqFsmScript;
	}

	void CSeqFsmScript::save(NLMISC::IStream &os)
	{
		CAgentScript::save(os);
		// TODO
	}

	void CSeqFsmScript::load(NLMISC::IStream &is)
	{
		CAgentScript::load(is);
		// TODO
	}

/*	int CSeqFsmScript::getBaseMethodCount() const
	{
		return CFsmScript::getBaseMethodCount() + 2;
	}
*/

	IObjectIA::CProcessResult CSeqFsmScript::runMethodBase(int index,int heritance, IObjectIA *params)
	{
		return IObjectIA::CProcessResult();
	}


	IObjectIA::CProcessResult CSeqFsmScript::runMethodBase(int index,IObjectIA *params)
	{	

		IBaseGroupType *param = (IBaseGroupType *) params;

		int i = index - IAgent::getMethodIndexSize();


		if ( i < getBaseMethodCount() )
			return CFsmScript::runMethodBase(index, params);

		switch(index - CFsmScript::getMethodIndexSize())
		{
		case 0:
			{	
				while ( param->size() )
				{
#ifdef NL_DEBUG
					const IVarName *state_name = (NLAIAGENT::IVarName *) param->get()->clone();
#endif
					param->popFront();
#ifdef NL_DEBUG
					const char *dbg_state_name = state_name->getString();
#endif
//					_Steps.push_back( state_name );
				}
			}
			break;
		}

		IObjectIA::CProcessResult r;

#ifdef NL_DEBUG
		std::string buf;
		getDebugString(buf);
#endif

		return r;
	}

/*
	int CSeqFsmScript::getBaseMethodCount() const
	{
		return CAgentScript::getBaseMethodCount();
	}
*/	
	tQueue CSeqFsmScript::isMember(const IVarName *className,const IVarName *name,const IObjectIA &param) const
	{		

		tQueue result = CFsmScript::isMember( className, name, param);

		if ( result.size() )
			return result;

		if ( *name == CStringVarName("Constructor") )
		{
			NLAIAGENT::CObjectType *r_type = new NLAIAGENT::CObjectType( new NLAIC::CIdentType( NLAIC::CIdentType::VoidType ) );
			result.push( NLAIAGENT::CIdMethod(  CFsmScript::getMethodIndexSize() , 0.0,NULL, r_type ) );
		}
		return result;
	}

/*	const IObjectIA::CProcessResult &CSeqFsmScript::run()
	{
		return CAgentScript::run();
	}
	*/

	void CSeqFsmScript::activate(sint32 no_child)
	{
		if ( _ActiveChild != NULL )
		{
			_ActiveChild->release();
			_ActiveChild = NULL;
		}

		_IdActiveChild = no_child;
//		IBasicAgent *tmp = get

		if ( _ActiveChild != NULL )
			_ActiveChild->incRef();
	}


/*	void CSeqFsmScript::unactivate(IBasicAgent *child)
	{
		if ( _ActiveChild != NULL)
		{
			_ActiveChild->release();
			_ActiveChild = NULL;
		}
	}
	*/

	void CSeqFsmScript::runChildren()
	{
#ifdef NL_DEBUG	
	const char *classBase = (const char *)getType();
#endif
		if ( _ActiveChild )
			_ActiveChild->run();
	}

	void CSeqFsmScript::onSuccess()
	{

	}

	void CSeqFsmScript::onFailure()
	{
	}

	void CSeqFsmScript::onSuccess( IObjectIA *)
	{
		activate( _IdActiveChild + 1 );
	}

	void CSeqFsmScript::onFailure( IObjectIA *)
	{
	}
} // NLAIAGENT
