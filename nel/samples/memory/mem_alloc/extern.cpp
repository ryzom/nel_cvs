#include "stdafx.h"
#include <stdlib.h>

void *operator new (unsigned int i)
{
	return malloc (i);
}

