#ifndef INPUTDEVICE_H
#define INPUTDEVICE_H

#ifdef OS_WINDOWS
#include <windows.h>
#endif

#include "device.h"

class WindowDevice;

enum ScanCode {
SC_UNKNOWN=0xFFFFFFFF, SC_ESC=0, SC_F1, SC_F2, SC_F3, SC_F4, SC_F5, SC_F6, SC_F7, SC_F8, SC_F9, SC_F10, SC_F11, SC_F12, SC_PRTSCR, SC_SCRLCK, SC_PAUSE,
SC_SQUARE, SC_1, SC_2, SC_3, SC_4, SC_5, SC_6, SC_7, SC_8, SC_9, SC_0, SC_DEGREE, SC_PLUS, SC_BCKSPC, SC_INSERT, SC_HOME, SC_PGUP, SC_NUMLOCK, SC_PADDIV, SC_PADMUL, SC_PADSUB,
SC_TAB, SC_A, SC_Z, SC_E, SC_R, SC_T, SC_Y, SC_U, SC_I, SC_O, SC_P, SC_POW, SC_DOLLAR, SC_ENTER, SC_DEL, SC_END, SC_PGDWN, SC_PAD7, SC_PAD8, SC_PAD9, SC_PADADD,
SC_CAPSLCK, SC_Q, SC_S, SC_D, SC_F, SC_G, SC_H, SC_J, SC_K, SC_L, SC_M, SC_UMLAUT, SC_MUL, SC_PAD4, SC_PAD5, SC_PAD6,
SC_LSHIFT, SC_INF, SC_W, SC_X, SC_C, SC_V, SC_B, SC_N, SC_VIRG, SC_SEMICOL, SC_COL, SC_ESCL, SC_RSHIFT, SC_UP, SC_PAD1, SC_PAD2, SC_PAD3, SC_PADENTER,
SC_LCTRL, SC_LWIN, SC_ALT, SC_SPACE, SC_ALTGR, SC_RWIN, SC_MENU, SC_RCTRL, SC_LEFT, SC_DOWN, SC_RIGHT, SC_PAD0, SC_DOT,
};

struct InputEvent
{
	ScanCode scanCode;
	float pressure;
	int   asciiCode;
};

class InputDevice : public Device
{
public:
	vector <InputEvent> InputEvents;
	vector <char>		InputBuffer;

	static DeviceType getDeviceType () { return DeviceType("INPUT_DEVICE"); }

	void connect (WindowDevice *wd);

	void newEvent (const InputEvent &ie);

#ifdef OS_WINDOWS
	HWND hWnd;
	ScanCode convertEvent (WPARAM wParam, LPARAM lParam);
#endif
};

#endif // INPUTDEVICE_H
