#ifndef _PYTHON_SUPPORT_H_
#define _PYTHON_SUPPORT_H_

struct PyMethodDef;
namespace NLAIPYSERVER
{	
	class IPythonModule
	{
	public:
		virtual void addModule(char *moduleName,const PyMethodDef *baseModule) = 0;
	};
}
#endif