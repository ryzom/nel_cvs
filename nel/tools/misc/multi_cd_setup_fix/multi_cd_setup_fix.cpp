// multi_cd_setup_fix.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "resource.h"

INT_PTR CALLBACK MyDialogProc(
  HWND hwndDlg,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{
	return FALSE;
}

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	// Windows
	HWND hwnd = CreateDialog (hInstance, MAKEINTRESOURCE(IDD_WAIT), NULL, MyDialogProc);
	RECT rect;
	RECT rectDesktop;
	GetWindowRect (hwnd, &rect);
	GetWindowRect (GetDesktopWindow (), &rectDesktop);
	SetWindowPos (hwnd, HWND_TOPMOST, (rectDesktop.right-rectDesktop.left-rect.right+rect.left)/2, (rectDesktop.bottom-rectDesktop.top-rect.bottom+rect.top)/2 , 0, 0, SWP_NOSIZE);
	ShowWindow (hwnd, SW_SHOW);

	// Display the window
	MSG	msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

 	// Get the temp directory
	char tempPath[1024];
	if (GetTempPath(1024, tempPath))
	{
		CreateDirectory(tempPath, NULL);
		strcat (tempPath, "Ryzom\\");
		CreateDirectory(tempPath, NULL);

		// Copy the files
		
		// Setup.dat
		char setupFile[1024];
		strcpy (setupFile, tempPath);
		strcat (setupFile, "setup.exe");
		SetFileAttributes(setupFile, GetFileAttributes(setupFile)&~FILE_ATTRIBUTE_READONLY);
		BOOL deleted = DeleteFile (setupFile);
		if (!CopyFile ("setup.dat", setupFile, FALSE) && deleted)
			MessageBox (NULL, "Not enough disk space", "Setup", MB_OK|MB_ICONERROR);

		// Ryzom.msi
		char msiFile[1024];
		strcpy (msiFile, tempPath);
		strcat (msiFile, "Ryzom.msi");
		SetFileAttributes(msiFile, GetFileAttributes(msiFile)&~FILE_ATTRIBUTE_READONLY);
		deleted = DeleteFile (msiFile); 
		if (!CopyFile ("Ryzom.msi", msiFile, FALSE) && deleted)
			MessageBox (NULL, "Not enough disk space", "Setup", MB_OK|MB_ICONERROR);

		// Execute the setup
		STARTUPINFO         si;
		PROCESS_INFORMATION pi;
		memset(&si, 0, sizeof(si));
		memset(&pi, 0, sizeof(pi));
		si.cb = sizeof(si);
		if (CreateProcess (setupFile, NULL, NULL, NULL, FALSE, 0, NULL, tempPath, &si, &pi))
			return 0;
	}

	return -1;
}
