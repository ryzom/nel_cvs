#ifndef NL_INTERPRET_GOALPATH_H
#define NL_INTERPRET_GOALPATH_H
#include "nel/ai/script/interpret_object_agent.h"

namespace NLAISCRIPT
{
	class CGoalPathClass: public CAgentClass
	{
	public:
		static const NLAIC::CIdentType IdGoalPathClass;

	public:
		CGoalPathClass(const NLAIAGENT::IVarName &);
		CGoalPathClass(const NLAIC::CIdentType &);
		CGoalPathClass(const NLAIAGENT::IVarName &, const NLAIAGENT::IVarName &);
		CGoalPathClass(const CGoalPathClass &);
		CGoalPathClass();

		const NLAIC::IBasicType *clone() const;
		const NLAIC::IBasicType *newInstance() const;
		virtual void getDebugString(std::string &) const;

		virtual NLAIAGENT::IObjectIA *buildNewInstance() const;

		virtual ~CGoalPathClass();
	};
}
#endif
