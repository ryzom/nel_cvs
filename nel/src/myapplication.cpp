
#include <windows.h>
#include <strstream>
using namespace std;

#include "debug.h"
#include "application.h"
#include "applicationdata.h"

#include "device.h"
#include "windowdevice.h"
#include "inputdevice.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
bool Quit = false;

int a=-1, b=-1;

bool tab[256];
LRESULT CALLBACK MainWndProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hDC;

	switch (msg)
	{
	case WM_KEYUP:
		if (wParam < 256) tab[wParam] = false;
		if (wParam == 27) Quit = true;
		InvalidateRect (hWnd, NULL, TRUE);
		break;
	case WM_KEYDOWN:
		if (wParam < 256) tab[wParam] = true;
		InvalidateRect (hWnd, NULL, TRUE);
		break;
	case WM_PAINT:
		{
			hDC = BeginPaint( hWnd, &ps );
//			TextOut( hDC, 10, 10, "Hello, World!", 13 );
			for (int y = 0; y < 16; y++)
			{
				for (int x = 0; x < 16; x++)
				{
					char toto[2];
					toto[0]=y*16+x;
					toto[1]='\0';
					TextOut (hDC, 10 + 20 * x, 10 + 20 * y, (tab[y*16+x])?"*":toto, 1);
				}
			}
			EndPaint( hWnd, &ps );
		}
		break;
	case WM_DESTROY:
		PostQuitMessage( 0 );
		break;
	default:
		return( DefWindowProc( hWnd, msg, wParam, lParam ));
	}
	return 0;
}

void create (Application *app)
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

   app->Data->hWnd = CreateWindow( "GenericAppClass",
      "Generic Application",
      WS_OVERLAPPEDWINDOW,
      0,
      0,
      400,
      400,
      NULL,
      NULL,
      CurrentApplication->Data->HInstance,
      NULL
   );

   ShowWindow (CurrentApplication->Data->hWnd, CurrentApplication->Data->nShowCmd);
}
*/

const char *ScanCodeString[] = {
"SC_ESC", "SC_F1", "SC_F2", "SC_F3", "SC_F4", "SC_F5", "SC_F6", "SC_F7", "SC_F8", "SC_F9", "SC_F10", "SC_F11", "SC_F12", "SC_PRTSCR", "SC_SCRLCK", "SC_PAUSE",
"SC_SQUARE", "SC_1", "SC_2", "SC_3", "SC_4", "SC_5", "SC_6", "SC_7", "SC_8", "SC_9", "SC_0", "SC_DEGREE", "SC_PLUS", "SC_BCKSPC", "SC_INSERT", "SC_HOME", "SC_PGUP", "SC_VERNUM", "SC_PADDIV", "SC_PADMULT", "SC_PADMINUS",
"SC_TAB", "SC_A", "SC_Z", "SC_E", "SC_R", "SC_T", "SC_Y", "SC_U", "SC_I", "SC_O", "SC_P", "SC_POW", "SC_DOLLAR", "SC_ENTER", "SC_SUPPR", "SC_END", "SC_PGDWN", "SC_PAD7", "SC_PAD8", "SC_PAD9", "SC_PADPLUS",
"SC_CAPSLCK", "SC_Q", "SC_S", "SC_D", "SC_F", "SC_G", "SC_H", "SC_J", "SC_K", "SC_L", "SC_M", "SC_UMLAUT", "SC_MULT", "SC_PAD4", "SC_PAD5", "SC_PAD6",
"SC_LSHIFT", "SC_INF", "SC_W", "SC_X", "SC_C", "SC_V", "SC_B", "SC_N", "SC_VIRG", "SC_SEMICOL", "SC_COL", "SC_ESCL", "SC_RSHIFT", "SC_UP", "SC_PAD1", "SC_PAD2", "SC_PAD3", "SC_PADENTER",
"SC_LCTRL", "SC_LWIN", "SC_ALT", "SC_SPACE", "SC_ALTGR", "SC_RWIN", "SC_MENU", "SC_RCTRL", "SC_LEFT", "SC_DOWN", "SC_RIGHT", "SC_PAD0", "SC_DOT"
};

const uint32 nbw = 1;
class MyApplication : virtual public Application
{
public:
	WindowDevice *wd[nbw];
	InputDevice *id[nbw];

	void onStart ()
	{
		OutputDebugString ("onStart() called\n");

		for (uint nb = 0; nb < nbw; nb++)
		{
			wd[nb] = static_cast<WindowDevice *>(Device::create ("WINDOW_DEVICE"));
			wd[nb]->init (nb*20, nb*20, 400, 400);
			wd[nb]->open ();
			id[nb] = static_cast<InputDevice *>(Device::create ("INPUT_DEVICE"));
			id[nb]->connect (wd[nb]);
		}
	}

	bool onIdle ()
	{
		OutputDebugString ("onIdle() called\n");

		for (uint nb = 0; nb < nbw; nb++)
		{
			wd[nb]->clear ();
			uint y = 0;
			for (vector <InputEvent>::iterator iei = id[nb]->InputEvents.begin (); iei != id[nb]->InputEvents.end (); iei++)
			{
				if ((*iei).pressure > 0.0f)
				{
					char str[128];
					if ((*iei).scanCode < 0)
					{
							sprintf(str, "%d '%c' '%c'", -((*iei).scanCode), -((*iei).scanCode), (*iei).asciiCode);
					}
					else
					{
							sprintf(str, "%s '%c' %d", ScanCodeString[((*iei).scanCode)], (*iei).asciiCode, (*iei).asciiCode);
					}
					if ((*iei).asciiCode != -1)
						wd[nb]->printf(10, 10 + 20 * y, str);
					y++;
				}
			}
		}

		return false;	// is finish?
	}

	void onStop ()
	{
		OutputDebugString ("onStop() called\n");
		for (uint nb = 0; nb < nbw; nb++)
		{
			wd[nb]->close ();
		}
	}
};

Application *Application::Create ()
{
	return new MyApplication;
}
