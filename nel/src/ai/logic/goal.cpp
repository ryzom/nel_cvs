#include "nel/ai/logic/goal.h"

namespace NLAILOGIC
{

	CGoal::CGoal(const NLAIAGENT::IVarName &name)
	{
		_Name = (NLAIAGENT::IVarName *) name.clone();
	}

	CGoal::CGoal(const CGoal &c)
	{
		if ( c._Name )
			_Name = (NLAIAGENT::IVarName *) c._Name->clone();
	}


	CGoal::~CGoal()
	{
		if ( _Name )
			_Name->release();
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

	void CGoal::operatorSucces(IBaseOperator *)
	{

	}

	void CGoal::operatorFailure(IBaseOperator *)
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
//		is >> _Value;	
	}

	void CGoal::getDebugString(char *text) const
	{
		strcpy( text ,"CGoal");
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

	std::vector<IBaseVar *> *CGoal::getVars()
	{
		return NULL;
	}

	const NLAIC::CIdentType &CGoal::getType() const
	{
		return IdGoal;
	}
}