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
	}

	CGoalStack::~CGoalStack()
	{
/*		std::vector<CGoal *>::iterator it_g = _Goals.begin();
		while ( it_g != _Goals.end() )
		{
			(*it_g)->cancel();
			it_g++;
		}
		*/
	}
/*
	NLAIAGENT::IObjectIA::CProcessResult CGoalStack::runActivity()
	{
		int i;
		if ( _Goals.size() > 1)
		{
			CGoal *old_top = _Goals.front();

			std::sort(_Goals.begin(), _Goals.end(), greater());


			CGoal *new_top = _Goals.front();

#ifdef NL_DEBUG
			std::string dbg_stack;
			for ( i = 0; i < (int) _Goals.size(); i++ )
			{
				std::string tmp;
				 _Goals[i]->getDebugString( tmp );
				dbg_stack += tmp;
			}
			const char *dbg_str = dbg_stack.c_str();
#endif


		if ( ! _MultiTask )
		{
				new_top->select();
				
				if ( ! ( (*old_top) == (*new_top) ) )
				{
					old_top->unSelect();
				}
			}
			else
			{
				if ( _Goals.size() == 1 )
				{
#ifdef NL_DEBUG
					std::string tmp;
					 _Goals[0]->getDebugString( tmp );
#endif
					_Goals.front()->select();
				}
			}
		}
		else
		{
			for ( i = 0; i < (int) _Goals.size(); i++ )
			{
				if ( _Goals[i]->priority() > 0 )
					_Goals[i]->select();
				else
					_Goals[i]->unSelect();
			}
		}
		return NLAIAGENT::IObjectIA::CProcessResult();
	}
	*/

	NLAIAGENT::IObjectIA::CProcessResult CGoalStack::runActivity()
	{
		sint32 i;
		if ( _Goals.size() > 1)
		{
			CGoal *old_top = _Goals.front();
			std::sort(_Goals.begin(), _Goals.end(), greater());
		}

#ifdef NL_DEBUG
		std::string dbg_stack;
		for ( i = 0; i < (sint32) _Goals.size(); i++ )
		{
			std::string tmp;
			 _Goals[i]->getDebugString( tmp );
			dbg_stack += tmp;
		}
		const char *dbg_str = dbg_stack.c_str();
#endif

		i = 0;
		std::vector<CGoal *>::iterator it_g = _Goals.begin();
		while ( ( it_g != _Goals.end() ) )
		{
			if ( ( i < _MaxGoals ) && ( (*it_g)->priority() > 0 ) )
			{		
				_Goals[i]->select();
				i++;
			}
			else
				_Goals[i]->unSelect();
			it_g++;
		}
		return NLAIAGENT::IObjectIA::CProcessResult();
	}


	void CGoalStack::addGoal(CGoal *g) 
	{
		_Goals.push_back( g );
	}

	void CGoalStack::removeGoal(CGoal *g)
	{
		std::vector<CGoal *>::iterator it_g = _Goals.begin();
		while ( it_g != _Goals.end() )
		{
			if ( (**it_g) == *g )
			{
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
			char buf[1024];
			sprintf( buf, " %f ", _Goals[i]->priority() );
			t += buf;
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
