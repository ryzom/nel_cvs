/** \file primitive_utils.cpp
 * Utility to work with primitives tree
 *
 *	Some of this utilities use the concept of 'predicate' applyed to 
 *	primitive node.
 *	As defined in the stl, a predicate is some form of functor that
 *	receive a parameter, apply some test on it, then return the result
 *	of the test.
 *	In this case, predicate functor receive a IPrimitive pointer and 
 *	return a boolean.
 *	The utilities come with 4 predefined predicate :
 *		- primitive class property test
 *		- primitive name property test
 *		- primitive class and name property test
 *		- primitive specific property value test.
 *
 *	You can define you own predicate. To do so, just create a class or struct
 *	with any data member/constructor you need. The unique requirement is that
 *	your class declare a method
 *		bool operator() (const IPrimitive *) const
 *
 *	You can also 'combine' the predefined predicate by using the 
 *	class CPrimitiveSetFilter that refine a set of primitive
 *	by applying another predicate.
 *
 *	Note that this work is inspired in some way by the functionality found
 *	in XPath for xml.
 *
 *	Boris.
 *
 * $Id: primitive_utils.cpp,v 1.4.4.1 2004/09/13 15:56:41 boucher Exp $
 */

#include <nel/ligo/primitive_utils.h>


namespace NLLIGO
{

std::string buildPrimPath(const IPrimitive *prim)
{
	std::string path;

	while (prim != NULL)
	{
		std::string name;
		prim->getPropertyByName("name", name);
		if (path.empty())
			path = name;
		else
			path = name + "." + path;

		prim = prim->getParent();
	}
	return path;
}

void selectPrimByPath(IPrimitive *rootNode, const std::string &path, TPrimitiveSet &result)
{
	std::vector<std::string>	parts;
	NLMISC::explode(path, ".", parts, false);
	IPrimitive * tmpChild;

	result.clear();

	if (parts.empty())
		return;

	TPrimitiveSet	candidats, nextStep;
	candidats.push_back(rootNode);


	for (uint i=0; i<parts.size(); ++i)
	{
		for (uint j=0; j<candidats.size(); ++j)
		{
			std::string tmpName;
			std::vector<std::string> name;
			candidats[j]->getPropertyByName("name", tmpName);
			NLMISC::explode(tmpName,".",name);

			bool test=false;
			for(uint k=0;k<name.size();k++)
			{				
				if (name.at(k)==parts[i+k])
					test=true;	
				else
				{
					test=false;
					break;
				}
			}
			if (test) 
			{
				for(uint k=0;k<candidats[j]->getNumChildren();k++)
				{
					candidats[j]->getChild(tmpChild,k);
					nextStep.push_back(tmpChild);
				}
				result.clear();
				result.push_back(candidats[j]);
				i+=name.size()-1;
				break;
			}
			
		}
		
		candidats.swap(nextStep);
		nextStep.clear();

		if (candidats.empty())
			return;
	}

	// store the result
	//result.push_back(candidats.at(0)->getParent());
}

} // namespace NLLIGO



