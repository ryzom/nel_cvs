#include "nel/ai/agent/goal_stack.h"
#include "nel/ai/agent/object_type.h"
#include "nel/ai/agent/agent_digital.h"

namespace NLAILOGIC
{
	CGoalStack::CGoalStack()
	{
		_MaxGoals = 1;
	}

	CGoalStack::CGoalStack(const CGoalStack &g)
	{
		int i;
		for( i = 0; i < (int) g._Goals.size(); i++ )
		{
			_Goals.push_back( (CGoal *) g._Goals[i]->clone() );
		}
		_MaxGoals = g._MaxGoals;
	}	

	CGoalStack::~CGoalStack()
	{
		removeGoal();
	}

	NLAIAGENT::IObjectIA::CProcessResult CGoalStack::runActivity()
	{
		sint32 i;
		if ( _Goals.size() > 1)
		{
			std::sort(_Goals.begin(), _Goals.end(), greater());
		}

#ifdef NL_DEBUG
		/*std::string dbg_stack;
		for ( i = 0; i < (sint32) _Goals.size(); i++ )
		{
			std::string tmp;
			 _Goals[i]->getDebugString( tmp );
			dbg_stack += tmp;
		}
		const char *dbg_str = dbg_stack.c_str();*/
#endif

		i = 0;
		std::vector<CGoal *>::iterator it_g = _Goals.begin();
		while ( ( it_g != _Goals.end() ) )
		{
			if (  (*it_g)->priority() > 0 )
			{		
				if ( (*it_g)->isExclusive() )
				{
					if ( i < _MaxGoals )		// Exclusive operators
					{
						(*it_g)->select();
						i++;
					}
					else
						(*it_g)->unSelect();
				}
				else
					(*it_g)->select();			// Background operators
			}
			else
				(*it_g)->unSelect();
			it_g++;
		}
		return NLAIAGENT::IObjectIA::CProcessResult();
	}


	void CGoalStack::addGoal(CGoal *g) 
	{
		_Goals.push_back( g );
	}

	void CGoalStack::removeGoal()
	{
		std::vector<CGoal *>::iterator it_g = _Goals.begin();
		while ( it_g != _Goals.end() )
		{
			(*it_g)->cancel();
			(*it_g)->release();
			it_g++;
		}
		_Goals.clear();
	}

	void CGoalStack::removeGoal(CGoal *g)
	{
		std::vector<CGoal *>::iterator it_g = _Goals.begin();
		while ( it_g != _Goals.end() )
		{
			if ( (**it_g) == *g )
			{
				(*it_g)->cancel();
				//Samos
				(*it_g)->release();
				_Goals.erase( it_g );
				return;
			}
			it_g++;
		}
	}

	CGoal *CGoalStack::getTopGoal()
	{
		if ( _Goals.size() )
			return _Goals.front();	
		else
			return NULL;
	}

	CGoal *CGoalStack::operator[](sint32 index)
	{
		return _Goals[index];
	}

	void CGoalStack::getDebugString(std::string &t) const
	{
		for (int i = 0; i < (int) _Goals.size(); i++ )
		{
			t += _Goals[i]->getName().getString();
			std::string text;
			text = NLAIC::stringGetBuild(" %f ", _Goals[i]->priority() );
			t += text;
			if ( _Goals[i]->isSelected() )
				t += "Selected";
			else
				t += "Not selected";
			t += '\n';
		}
	}

	const NLAIC::CIdentType &CGoalStack::getType() const
	{
		return IdGoalStack;
	}

	const NLAIC::IBasicType *CGoalStack::clone() const
	{
		return new CGoalStack( *this );
	}

	const NLAIC::IBasicType *CGoalStack::newInstance() const
	{
		return new CGoalStack();
	}

	void CGoalStack::save(NLMISC::IStream &os)
	{
	}

	void CGoalStack::load(NLMISC::IStream &is)
	{
	}

	bool CGoalStack::isTrue() const
	{
		// TODO
		return false;
	}

	float CGoalStack::truthValue() const
	{
		// TODO
		return 0.0;
	}

	const NLAIAGENT::IObjectIA::CProcessResult &CGoalStack::run()
	{
		return NLAIAGENT::IObjectIA::ProcessRun;
	}

	bool CGoalStack::isEqual(const CGoal &a) const
	{
		// TODO:
		return false;
	}

	bool CGoalStack::isEqual(const NLAIAGENT::IBasicObjectIA &a) const
	{
		// TODO:
		return false;
	}

	const std::vector<CGoal *> &CGoalStack::getStack()
	{
		return _Goals;
	}

	void CGoalStack::setMaxGoals(sint32 max)
	{
		_MaxGoals = max;
	}

	NLAIAGENT::tQueue CGoalStack::isMember(const NLAIAGENT::IVarName *className,const NLAIAGENT::IVarName *funcName,const NLAIAGENT::IObjectIA &params) const
	{

#ifdef NL_DEBUG	
		std::string nameP;
		std::string nameM;
		funcName->getDebugString(nameM);
		params.getDebugString(nameP);

		const char *dbg_class_name = (const char *) getType();
#endif
		static NLAIAGENT::CStringVarName maxgoals_name("setMaxGoals");
		NLAIAGENT::tQueue r;
		if(className == NULL)
		{
			if( (*funcName) == maxgoals_name )
			{					
				NLAIAGENT::CObjectType *c = new NLAIAGENT::CObjectType( new NLAIC::CIdentType( CGoal::IdGoal ) );					
				r.push( NLAIAGENT::CIdMethod( 0 + IObjectIA::getMethodIndexSize(), 0.0, NULL, c) );					
			}
		}

		if ( r.empty() )
			return IObjectIA::isMember(className, funcName, params);
		else
			return r;
	}

	///\name Some IObjectIA method definition.
	//@{		
	NLAIAGENT::IObjectIA::CProcessResult CGoalStack::runMethodeMember(sint32, sint32, NLAIAGENT::IObjectIA *)
	{
		return IObjectIA::CProcessResult();
	}

	NLAIAGENT::IObjectIA::CProcessResult CGoalStack::runMethodeMember(sint32 index, NLAIAGENT::IObjectIA *p)
	{
		NLAIAGENT::IBaseGroupType *param = (NLAIAGENT::IBaseGroupType *)p;

		switch ( index - IObjectIA::getMethodIndexSize() )
		{
			case 0:
				{					

					NLAIAGENT::DigitalType *maxgoals = (NLAIAGENT::DigitalType *) param->getFront();
					param->popFront();
#ifdef NL_DEBUG
					std::string buf;
					maxgoals->getDebugString(buf);
#endif
					_MaxGoals = (sint32) maxgoals->getNumber();
					return IObjectIA::CProcessResult();		
				}
				break;
		}
		return IObjectIA::CProcessResult();
	}

	sint32 CGoalStack::getMethodIndexSize() const
	{
		return IObjectIA::getMethodIndexSize() + 1;
	}
} // NLAILOGIC
