#include "nel/ai/agent/goal_stack.h"

namespace NLAILOGIC
{
	CGoalStack::CGoalStack()
	{
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

	NLAIAGENT::IObjectIA::CProcessResult CGoalStack::runActivity()
	{
		if ( _Goals.size() > 1)
		{
			CGoal *old_top = _Goals.front();

			std::sort(_Goals.begin(), _Goals.end(), greater());


			CGoal *new_top = _Goals.front();

#ifdef NL_DEBUG
			std::string dbg_stack;
			for (int i = 0; i < (int) _Goals.size(); i++ )
			{
				std::string tmp;
				 _Goals[i]->getDebugString( tmp );
				dbg_stack += tmp;
			}
			const char *dbg_str = dbg_stack.c_str();
#endif


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
} // NLAILOGIC
