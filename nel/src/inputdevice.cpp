#include <windows.h>

#include "application.h"
#include "applicationdata.h" 
#include "inputdevice.h"
#include "windowdevice.h"

void InputDevice::connect (WindowDevice *wd)
{
	// connecte le window device a l'input device
	hWnd = wd->hWnd;
}

void InputDevice::newEvent (const InputEvent &ie)
{
	for (vector <InputEvent>::iterator iei = InputEvents.begin (); iei != InputEvents.end (); iei++)
	{
		if ((*iei).scanCode == ie.scanCode)
		{
			// on a trouve la touche, on met a jour si necessaire
			if (ie.pressure == 0.0f)
				InputEvents.erase (iei);
			else
				(*iei).pressure = ie.pressure;
			return;
		}
	}
	InputEvents.push_back (ie);
//	OutputDebugString ("et une de plus\n");
}

ScanCode InputDevice::convertEvent (WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case VK_ESCAPE:		return SC_ESC;
	case VK_F1:			return SC_F1;
	case VK_F2:			return SC_F2;
	case VK_F3:			return SC_F3;
	case VK_F4:			return SC_F4;
	case VK_F5:			return SC_F5;
	case VK_F6:			return SC_F6;
	case VK_F7:			return SC_F7;
	case VK_F8:			return SC_F8;
	case VK_F9:			return SC_F9;
	case VK_F10:		return SC_F10;
	case VK_F11:		return SC_F11;
	case VK_F12:		return SC_F12;
	case VK_PRINT:		return SC_PRTSCR;
//	case VK_:			return SC_SCRLOCK;
	case VK_PAUSE:		return SC_PAUSE;

//	case VK_:return SC_SQUARE;
	case '1':	return SC_1;
	case '2':	return SC_2;
	case '3':	return SC_3;
	case '4':	return SC_4;
	case '5':	return SC_5;
	case '6':	return SC_6;
	case '7':	return SC_7;
	case '8':	return SC_8;
	case '9':	return SC_9;
	case '0':	return SC_0;
/*	case VK_:	return SC_DEGREE;
	case VK_:	return SC_PLUS;
*/	case VK_BACK:		return SC_BCKSPC;
	case VK_INSERT:		return SC_INSERT;
	case VK_HOME:		return SC_HOME;
	case VK_PRIOR:		return SC_PGUP;
	case VK_NUMLOCK:	return SC_NUMLOCK;
	case VK_DIVIDE:		return SC_PADDIV;
	case VK_MULTIPLY:	return SC_PADMUL;
	case VK_SUBTRACT:	return SC_PADSUB;

	case VK_TAB:	return SC_TAB;
	case 'A':return SC_A;
	case 'Z':return SC_Z;
	case 'E':return SC_E;
	case 'R':return SC_R;
	case 'T':return SC_T;
	case 'Y':return SC_Y;
	case 'U':return SC_U;
	case 'I':return SC_I;
	case 'O':return SC_O;
	case 'P':return SC_P;
/*	case VK_:return SC_POW;
	case VK_:return SC_DOLLAR;
*/	case VK_RETURN:
		{
			if ((lParam>>24&1) == 1) return SC_PADENTER;
			else return SC_ENTER;
		}
	case VK_DELETE:	return SC_DEL;
	case VK_END:	return SC_END;
	case VK_NEXT:	return SC_PGDWN;
	case VK_NUMPAD7:return SC_PAD7;
	case VK_NUMPAD8:return SC_PAD8;
	case VK_NUMPAD9:return SC_PAD9;
	case VK_ADD:	return SC_PADADD;

	case VK_CAPITAL:return SC_CAPSLCK;
	case 'Q':return SC_Q;
	case 'S':return SC_S;
	case 'D':return SC_D;
	case 'F':return SC_F;
	case 'G':return SC_G;
	case 'H':return SC_H;
	case 'J':return SC_J;
	case 'K':return SC_K;
	case 'L':return SC_L;
	case 'M':return SC_M;
/*	case VK_:return SC_UMLAUT;
	case VK_:return SC_MULT;
*/	case VK_NUMPAD4:	return SC_PAD4;
	case VK_NUMPAD5:	return SC_PAD5;
	case VK_NUMPAD6:	return SC_PAD6;

	case VK_SHIFT:
		{
			if ((lParam>>16&0xFF) == 42) return SC_LSHIFT;
			else if ((lParam>>16&0xFF) == 54) return SC_RSHIFT;
			else return SC_UNKNOWN;
		}
//	case VK_:return SC_INF;
	case 'W':return SC_W;
	case 'X':return SC_X;
	case 'C':return SC_C;
	case 'V':return SC_V;
	case 'B':return SC_B;
	case 'N':return SC_N;
/*	case VK_:return SC_VIRG;
	case VK_:return SC_SEMICOL;
	case VK_:return SC_COL;
	case VK_:return SC_ESCL;
*/
	case VK_UP:			return SC_UP;
	case VK_NUMPAD1:	return SC_PAD1;
	case VK_NUMPAD2:	return SC_PAD2;
	case VK_NUMPAD3:	return SC_PAD3;

	case VK_CONTROL:
		{
			if ((lParam>>24&1) == 1) return SC_RCTRL;
			else return SC_LCTRL;
		}
	case VK_LWIN:		return SC_LWIN;
	case VK_MENU:		return SC_ALT;
	case VK_SPACE:		return SC_SPACE;
//	case VK_:return SC_ALTGR;
	case VK_RWIN:		return SC_RWIN;
	case VK_APPS:		return SC_MENU;
	case VK_LEFT:		return SC_LEFT;
	case VK_DOWN:		return SC_DOWN;
	case VK_RIGHT:		return SC_RIGHT;
	case VK_NUMPAD0:	return SC_PAD0;
	case VK_DECIMAL:	return SC_DOT;
	default: break;
	}
	return (ScanCode) -wParam;
}

