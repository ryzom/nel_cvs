#include <windows.h>

#include "application.h"
#include "applicationdata.h" 
#include "inputdevice.h"
#include "windowdevice.h"
#include "device.h"

bool Quit = false;

int a=-1, b=-1;

bool tab[256];

LRESULT CALLBACK MainWndProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	vector <Device *> vd;
	Device::get ("WINDOW_DEVICE", vd);

	WindowDevice *wd;

	for (vector<Device *>::iterator vdi = vd.begin (); vdi != vd.end (); vdi++)
	{
		wd = static_cast<WindowDevice *>(*vdi);
		if (wd->hWnd == hWnd)
			break;
	}

	switch (msg)
	{
//	case WM_SYSKEYUP:
	case WM_KEYUP:
		{
			vector <Device *> vd;
			Device::get ("INPUT_DEVICE", vd);

			for (vector<Device *>::iterator vdi = vd.begin (); vdi != vd.end (); vdi++)
			{
				InputDevice *id = static_cast<InputDevice *>(*vdi);
				if (id->hWnd == hWnd)
				{
					// on appelle cette input device
					InputEvent ie;
					ie.pressure = 0.0f;
					
					WORD toto[256];
					PBYTE state[256];
					GetKeyboardState ((unsigned char *)state);
					int res = ToAscii (wParam, (lParam>>16)&0xFF, (unsigned char *)state, toto, 0);
					if (res == 1) ie.asciiCode = toto[0];
					else ie.asciiCode = -1;

					ie.scanCode = id->convertEvent (wParam, lParam);
					id->newEvent (ie);
				}
			}
			if (wParam == 27) Quit = true;
			break;
		}
//	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:
		{
			vector <Device *> vd;
			Device::get ("INPUT_DEVICE", vd);

			for (vector<Device *>::iterator vdi = vd.begin (); vdi != vd.end (); vdi++)
			{
				InputDevice *id = static_cast<InputDevice *>(*vdi);
				if (id->hWnd == hWnd)
				{
					// on appelle cette input device
					InputEvent ie;
					ie.pressure = 1.0f;
					ie.scanCode = id->convertEvent (wParam, lParam);
					id->newEvent (ie);
				}
			}
			if (wParam == 27) Quit = true;
			break;
		}
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hDC = BeginPaint (hWnd, &ps);
			for (vector<WindowDevice::entry>::iterator cei = wd->Texts.begin (); cei != wd->Texts.end (); cei++)
			{
				TextOut (hDC, (*cei).x, (*cei).y, (*cei).text.c_str (), (*cei).text.size ());
			}
			EndPaint (hWnd, &ps);
			ValidateRect (hWnd, NULL);
			break;
		}
	case WM_DESTROY:
		PostQuitMessage (0);
		break;
	default:
		return (DefWindowProc (hWnd, msg, wParam, lParam));
	}
	return 0;
}

void WindowDevice::init (sint16 x, sint16 y, uint16 width, uint16 height)
{
	if (!CurrentApplication->Data->HPrevInstance)
	{
		WNDCLASS wc;
		wc.lpszClassName = "GenericAppClass";
		wc.lpfnWndProc = MainWndProc;
		wc.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
		wc.hInstance = CurrentApplication->Data->HInstance;
		wc.hIcon = NULL;
		wc.hCursor = LoadCursor( NULL, IDC_ARROW );
		wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wc.lpszMenuName = "GenericAppMenu";
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;

		RegisterClass( &wc );
	}

	hWnd = CreateWindow( "GenericAppClass",
	  "Generic Application",
	  WS_OVERLAPPEDWINDOW,
	  x,
	  y,
	  width,
	  height,
	  NULL,
	  NULL,
	  CurrentApplication->Data->HInstance,
	  NULL
	);
}

void WindowDevice::open ()
{
	ShowWindow (hWnd, CurrentApplication->Data->nShowCmd);
}

void WindowDevice::close ()
{
}

void WindowDevice::clear ()
{
	Texts.clear ();
	InvalidateRect (hWnd, NULL, TRUE);
}

void WindowDevice::printf (uint16 x, uint16 y, const string Text)
{
	entry ne;
	ne.x = x;
	ne.y = y;
	ne.text = Text;

	Texts.push_back (ne);
	InvalidateRect (hWnd, NULL, TRUE);
}
