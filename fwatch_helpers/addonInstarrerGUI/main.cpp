// addonInstarrerGUI.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "common.h"
#include "installer.h"
#include "main.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
HFONT system_font;
const int window_w = 800;
const int window_h = 600;

GLOBAL_VARIABLES global = {
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	true,
	false,
	false,
	-1,
	0,
	0,
	0,
	0,
	0,
	0,
	0.61,
	0,
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L"",
	L""
};

std::wstring mod_subfolders[] = {
	L"",
	L"Addons",
	L"Bin",
	L"Campaigns",
	L"Dta",
	L"Missions",
	L"MPMissions",
	L"Templates",
	L"SPTemplates",
	L"MissionsUsers",
	L"MPMissionsUsers",
	L"IslandCutscenes"
};

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(nCmdShow);
	global.program_arguments = lpCmdLine;

	NONCLIENTMETRICS ncMetrics;
	ncMetrics.cbSize = sizeof(NONCLIENTMETRICS);
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncMetrics, 0);
	system_font = CreateFontIndirect(&ncMetrics.lfMessageFont);

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_ADDONINSTARRERGUI, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);
	
	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow)) {
		return FALSE;
	}
	
	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_ADDONINSTARRERGUI));

	// Main message loop:
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

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

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ADDONINSTARRERGUI));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_ADDONINSTARRERGUI);
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
   HWND hWnd;
   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, window_w, window_h, NULL, NULL, hInstance, NULL);

   if (!hWnd)
      return FALSE;

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
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

	switch (message) {
		case WM_CREATE: {
			#define WINDOW_H_RATIO 1.5f
			#define DIVIDE(arg) (int)((double)arg/WINDOW_H_RATIO)

			DWORD style = WS_CHILD | WS_VISIBLE | SS_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | ES_READONLY | WS_BORDER | WS_VSCROLL;
			global.control_generallog = CreateWindow(L"Edit", L"", style, 0, 0, window_w, DIVIDE(window_h), hWnd, NULL, NULL, NULL);
			global.control_detaillog  = CreateWindow(L"Edit", L"", style, 0, DIVIDE(window_h), window_w, DIVIDE(window_h), hWnd, NULL, NULL, NULL);

			SendMessage(global.control_generallog, WM_SETFONT, (WPARAM)system_font, TRUE);
			SendMessage(global.control_detaillog, WM_SETFONT, (WPARAM)system_font, TRUE);

			global.window_menu = GetMenu(hWnd);
			EnableMenuItem(global.window_menu, ID_PROCESS_RETRY, MF_BYCOMMAND | MF_GRAYED);

			DWORD threadID1    = 0;
			CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)addonInstallerWrapper, 0, 0,&threadID1);
		} break;

		case WM_SIZE: {
			int new_window_width  = LOWORD(lParam);
			int new_window_height = HIWORD(lParam);
			SetWindowPos(global.control_generallog, NULL, 0, 0, new_window_width, DIVIDE(new_window_height), SWP_SHOWWINDOW);
			SetWindowPos(global.control_detaillog , NULL, 0, DIVIDE(new_window_height), new_window_width, DIVIDE(new_window_height), SWP_SHOWWINDOW);
		} break;

		case WM_COMMAND: {
			wmId       = LOWORD(wParam);
			wmEvent    = HIWORD(wParam);

			// Parse the menu selections:
			switch (wmId) {
				case ID_PROCESS_ABORT: {
					global.abort_installer = true;
					DisableMenu();
				} break;

				case ID_OPTIONS_RESTARTGAME: {
					global.restart_game = !global.restart_game;
					CheckMenuItem(global.window_menu, wmId, global.restart_game ? MF_CHECKED : MF_UNCHECKED);
				} break;

				case ID_PROCESS_PAUSE: {
					global.pause_installer = !global.pause_installer;
					CheckMenuItem(global.window_menu, wmId, global.pause_installer ? MF_CHECKED : MF_UNCHECKED);
				} break;

				case ID_PROCESS_RETRY: {
					global.retry_installer = true;
				} break;

				case IDM_ABOUT: {
					DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
				} break;

				case IDM_EXIT:
					DestroyWindow(hWnd);
				break;

				default:
					return DefWindowProc(hWnd, message, wParam, lParam);
			}
			break;
		} break;

		case WM_PAINT: {
			hdc = BeginPaint(hWnd, &ps);
			// TODO: Add any drawing code here...
			EndPaint(hWnd, &ps);
		} break;

		case WM_CLOSE:
		case WM_DESTROY: {
			if (global.thread_installer==NULL || (global.thread_installer!=NULL && WaitForSingleObject(global.thread_installer, 0) == WAIT_OBJECT_0))
				PostQuitMessage(0);
			else
				if (!global.abort_installer) {
					global.abort_installer = true;
					DisableMenu();
				}
		} break;

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