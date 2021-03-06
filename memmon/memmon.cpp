// Copyright (c) 2007,2008 Charles Bailey

// memmon.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "memmon.h"
#include "mmpainter.h"
#include "mmprefs.h"
#include "attachdlg.h"
#include "createprocdlg.h"

#include <sstream>
#include <crtdbg.h>

#define MAX_LOADSTRING 100

using std::wostringstream;

namespace
{
// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
MMPainter* pPaint;
MMPrefs* pPrefs;
UINT_PTR timerid = 0;
bool bRecMenuState = false;
}

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
void CreateShNotIcon(HWND hwnd);
void PollShNotIcon(HWND hwnd);
void RemoveShNotIcon(HWND hwnd);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	_CrtSetDbgFlag(_CrtSetDbgFlag(0) | _CRTDBG_LEAK_CHECK_DF);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_MEMMON, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	MMPrefs prefs;
	prefs.Load();

	pPrefs = &prefs;

	MMPainter painter(100, pPrefs);
	pPaint = &painter;

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MEMMON));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	pPaint = NULL;
	pPrefs = NULL;

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= 0;//CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MEMMON));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= NULL;//(HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_MEMMON);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd = NULL;

	hInst = hInstance; // Store instance handle in our global variable


	if (pPrefs != NULL && pPrefs->GetUIPrefs().topmost)
	{
		hWnd = CreateWindowEx(WS_EX_TOPMOST, szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);
	}
	else
	{
		hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);
	}

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	//CreateShNotIcon(hWnd);
	return TRUE;
}

namespace
{

void MyPaint(HDC hdc, PAINTSTRUCT* ps)
{
	if (RectVisible(hdc, &pPaint->GetRect()))
	{
		pPaint->Paint(hdc, ps);
	}
}

void CheckTopmostSetting(HWND hWnd, const MMPrefs::UIPrefs& uiprf)
{
	WINDOWINFO wi;
	wi.cbSize = sizeof(wi);
	if (::GetWindowInfo(hWnd, &wi))
	{
		bool bTopMost = ((wi.dwExStyle & WS_EX_TOPMOST) != 0);
		if (bTopMost != uiprf.topmost)
		{
			::SetWindowPos(hWnd, uiprf.topmost ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
		}
	}
}

void DoFlipMenuItem( HWND hWnd, bool& check )
{
	HMENU hMenu = GetMenu( hWnd );

	if (!hMenu )
		return;

	MENUITEMINFO mii;
	mii.cbSize = sizeof( mii );
	mii.fMask = MIIM_STATE;

	if( GetMenuItemInfo( hMenu, ID_FILE_RECORD, FALSE, &mii ) )
	{
		check = !check;

		if( check )
			mii.fState |= MFS_CHECKED;
		else
			mii.fState &= ~MFS_CHECKED;

		SetMenuItemInfo( hMenu, ID_FILE_RECORD, FALSE, &mii );
		DrawMenuBar( hWnd );
	}
}

}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	const RECT& rect = pPaint->GetRect();
	RECT tmp;
	MINMAXINFO* pMinMax;
	int xdff, ydff;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		case ID_PROC_ATTACH:
			if( AttachDialog::Run( hInst, hWnd, pPaint ) && timerid == 0 )
				timerid = SetTimer(hWnd, 1, 100, NULL);
			break;
		case IDM_EDIT_PREFERENCES:
			pPrefs->RunDialog(hInst, hWnd);
			CheckTopmostSetting(hWnd, pPrefs->GetUIPrefs());
			break;
		case ID_FILE_OPEN:
			pPaint->Read(hWnd);
			InvalidateRect(hWnd, &rect, TRUE);
			break;
		case ID_FILE_SAVEAS:
			pPaint->Snapshot(hWnd);
			break;
		case ID_PROC_RUN:
			if( CreateProcDialog::Run( hInst, hWnd, pPaint ) && timerid == 0 )
				timerid = SetTimer(hWnd, 1, 100, NULL);
			if( pPaint->IsRecording() != bRecMenuState )
				DoFlipMenuItem( hWnd, bRecMenuState );
			break;
		case ID_FILE_RECORD:
			if( !bRecMenuState )
				pPaint->Record( hWnd );
			else
				pPaint->StopRecording();

			if( pPaint->IsRecording() != bRecMenuState )
				DoFlipMenuItem( hWnd, bRecMenuState );

			break;

		case ID_FILE_PLAYBACK:
			if( pPaint->Playback( hWnd ) && timerid == 0 )
				timerid = SetTimer(hWnd, 1, 100, NULL);
			if( pPaint->IsRecording() != bRecMenuState )
				DoFlipMenuItem( hWnd, bRecMenuState );
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;

	case WM_TIMER:
		pPaint->Update();
		if( pPaint->IsRecording() != bRecMenuState )
			DoFlipMenuItem( hWnd, bRecMenuState );
		InvalidateRect(hWnd, &rect, TRUE);
		break;

	case WM_KEYDOWN:
		//PollShNotIcon(hWnd);
		break;

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		MyPaint(hdc, &ps);
		EndPaint(hWnd, &ps);
		break;

	case WM_GETMINMAXINFO:
		//RemoveShNotIcon(hWnd);
		tmp = pPaint->GetRect();

		AdjustWindowRectEx(&tmp, WS_OVERLAPPEDWINDOW & ~WS_OVERLAPPED, true, 0);

		xdff = tmp.right - tmp.left - pPaint->GetRect().right;
		ydff = tmp.bottom - tmp.top- pPaint->GetRect().bottom;

		pMinMax = (MINMAXINFO*)lParam;
		pMinMax->ptMaxSize.x = rect.right + xdff;
		pMinMax->ptMaxSize.y = rect.bottom + ydff;
		pMinMax->ptMaxTrackSize.x = rect.right + xdff;
		pMinMax->ptMaxTrackSize.y = rect.bottom + ydff;
		break;

	case WM_DESTROY:
		//RemoveShNotIcon(hWnd);
		if (timerid != 0) KillTimer(hWnd, timerid);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
/*
void CreateShNotIcon(HWND hwnd)
{
	NOTIFYICONDATA nid;
	nid.cbSize = sizeof(nid);
	nid.hWnd = hwnd;
	nid.uID = 1;
	nid.uFlags = NIF_ICON | NIF_TIP | NIF_INFO;
	nid.hIcon = LoadIcon(NULL, IDI_HAND);
	_tcscpy(nid.szTip, _T("Funky!"));
	_tcscpy(nid.szInfo, _T("Very important details"));
	_tcscpy(nid.szInfoTitle, _T("Read Me!"));
	nid.uTimeout = 15;
	nid.dwInfoFlags = NIIF_INFO;
	Shell_NotifyIcon(NIM_ADD, &nid);
}

void PollShNotIcon(HWND hwnd)
{
	NOTIFYICONDATA nid;
	nid.cbSize = sizeof(nid);
	nid.hWnd = hwnd;
	nid.uID = 1;
	nid.uFlags = NIF_INFO;
	_tcscpy(nid.szInfo, _T("Very important details"));
	_tcscpy(nid.szInfoTitle, _T("Read Me!"));
	nid.uTimeout = 15;
	nid.dwInfoFlags = NIIF_INFO;
	Shell_NotifyIcon(NIM_MODIFY, &nid);
}

void RemoveShNotIcon(HWND hwnd)
{
	NOTIFYICONDATA nid;
	nid.cbSize = sizeof(nid);
	nid.hWnd = hwnd;
	nid.uID = 1;
	nid.uFlags = 0;
	Shell_NotifyIcon(NIM_DELETE, &nid);
}
*/
