
#include <windows.h>

#include "debug.h"
#include "application.h"
#include "windowdevice.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
LRESULT CALLBACK MainWndProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   PAINTSTRUCT ps;
   HDC hDC;

   switch( msg ) {
      case WM_PAINT:
         hDC = BeginPaint( hWnd, &ps );

         TextOut( hDC, 10, 10, "Hello, World!", 13 );

         EndPaint( hWnd, &ps );
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

   WNDCLASS wc;
   MSG msg;
   HWND hWnd;

   if (!app->Data->HPrevInstance)
   {
      wc.lpszClassName = "GenericAppClass";
      wc.lpfnWndProc = MainWndProc;
      wc.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
      wc.hInstance = hInstance;
      wc.hIcon = LoadIcon( NULL, IDI_APPLICATION );
      wc.hCursor = LoadCursor( NULL, IDC_ARROW );
      wc.hbrBackground = (HBRUSH)( COLOR_WINDOW+1 );
      wc.lpszMenuName = "GenericAppMenu";
      wc.cbClsExtra = 0;
      wc.cbWndExtra = 0;

      RegisterClass( &wc );
   }

   ghInstance = hInstance;

   hWnd = CreateWindow( "GenericAppClass",
      "Generic Application",
      WS_OVERLAPPEDWINDOW|WS_HSCROLL|WS_VSCROLL,
      0,
      0,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      NULL,
      NULL,
      app->Data->HInstance,
      NULL
   );

   ShowWindow( hWnd, nCmdShow );

}
*/
class MyApplication : virtual public Application
{
public:
	
	void onStart ()
	{
		OutputDebugString ("onStart() called\n");

		WindowDevice *wd = static_cast<WindowDevice *>(Device::create ("WINDOW_DEVICE"));
//		create (this);
	}

	bool onIdle ()
	{
		OutputDebugString ("onIdle() called\n");

		MSG msg;
		while( GetMessage( &msg, NULL, 0, 0 ) ) {
		   TranslateMessage( &msg );
		   DispatchMessage( &msg );
		}
	}

	void onStop ()
	{
		OutputDebugString ("onStop() called\n");
	}
};

Application *Application::Create ()
{
	return new MyApplication;
}
