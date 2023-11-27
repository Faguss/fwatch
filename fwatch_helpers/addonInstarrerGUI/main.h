#pragma once

#include "resource.h"
#pragma comment(lib, "comctl32.lib")
#include <Commctrl.h>

ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK ConvertDownloadLink(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK EditScript(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
DWORD WINAPI UpdateLineNumber(__in LPVOID lpParameter);
void CalculateWindowSizes(HWND window);