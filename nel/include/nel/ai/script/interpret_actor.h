#ifndef NL_INTERPRET_ACTOR_H
#define NL_INTERPRET_ACTOR_H
#include "script/interpret_object_agent.h"

namespace NLIASCRIPT
{
	class CActorClass: public CAgentClass
	{
	public:
		static const NLIAC::CIdentType IdActorClass;
	public:
		CActorClass(const NLIAAGENT::IVarName &);
		CActorClass(const NLIAC::CIdentType &);
		CActorClass(const NLIAAGENT::IVarName &, const NLIAAGENT::IVarName &);
		CActorClass(const CActorClass &);
		CActorClass();

		const NLIAC::IBasicType *clone() const;
		const NLIAC::IBasicType *newInstance() const;
		void getDebugString(char *t) const;

		virtual NLIAAGENT::IObjectIA *buildNewInstance() const;

		virtual ~CActorClass();
	};
}
#endif
