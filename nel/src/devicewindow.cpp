/*
#include <windows.h>
#include <strstream>
using namespace std;

#include "debug.h"
#include "application.h"
#include "applicationdata.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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

void Createw ()
{
	Application *app
   if (!app->Data->HPrevInstance)
   {
		WNDCLASS wc;
		wc.lpszClassName = "GenericAppClass";
		wc.lpfnWndProc = MainWndProc;
		wc.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
		wc.hInstance = app->Data->HInstance;
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
      app->Data->HInstance,
      NULL
   );

   ShowWindow (app->Data->hWnd, app->Data->nShowCmd);
}
*/