
#include <windows.h>

class ApplicationData
{
public:
#ifdef OS_WINDOWS
	HINSTANCE HInstance, HPrevInstance;
	LPSTR lpCmdLine;
	int nShowCmd;
#endif
};
