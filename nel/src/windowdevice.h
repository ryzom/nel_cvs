#ifndef WINDOWDEVICE_H
#define WINDOWDEVICE_H

#ifdef OS_WINDOWS
#include <windows.h>
#endif

#include "device.h"

class WindowDevice : public Device
{
public:
	void init (sint16 x, sint16 y, uint16 width, uint16 height);
	void open ();
	void close ();
	void clear ();
	void printf(uint16 x, uint16 y, const string Text);

	static DeviceType getDeviceType () { return DeviceType("WINDOW_DEVICE"); }

#ifdef OS_WINDOWS
	HWND hWnd;
#endif

	struct entry
	{
		sint16 x, y;
		string text;
	};
	vector <entry> Texts;
};

#endif // WINDOWDEVICE_H
