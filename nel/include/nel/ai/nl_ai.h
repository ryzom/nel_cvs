#ifndef _IA_NEL_H
#define _IA_NEL_H

#include "nel/ai/script/compilateur.h"

namespace NLAIAGENT
{
	class IMainAgent;
}
namespace NLAILINK
{
	void initIALib();
	void releaseIALib();
	void setLocalServerID(uint8 );
	uint8 getLocalServerID();
	void setMainManager(NLAIAGENT::IMainAgent *);
	void releaseMainManager();
	void buildScript(const std::string &scriptSrc, const std::string &name);
}

#endif
