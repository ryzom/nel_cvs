#ifndef NL_INTERPRET_ACTOR_H
#define NL_INTERPRET_ACTOR_H
#include "nel/ai/script/interpret_object_agent.h"

namespace NLAISCRIPT
{
	class CActorClass: public CAgentClass
	{
	public:
		static const NLAIC::CIdentType IdActorClass;

	public:
		CActorClass(const NLAIAGENT::IVarName &);
		CActorClass(const NLAIC::CIdentType &);
		CActorClass(const NLAIAGENT::IVarName &, const NLAIAGENT::IVarName &);
		CActorClass(const CActorClass &);
		CActorClass();

		const NLAIC::IBasicType *clone() const;
		const NLAIC::IBasicType *newInstance() const;
		virtual void getDebugString(std::string &) const;

		virtual NLAIAGENT::IObjectIA *buildNewInstance() const;

		virtual ~CActorClass();
	};
}
#endif
