// addonInstarrerGUI.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "common.h"
#include "functions.h"
#include "installer.h"
#include "resource.h"
#include "main.h"

#define MAX_LOADSTRING 100
HINSTANCE hInst;
TCHAR szTitle[MAX_LOADSTRING];
TCHAR szWindowClass[MAX_LOADSTRING];
HFONT system_font;
const int window_w = 800;
const int window_h = 600;
RECT controls_pos[CONTROLS_MAX] = {0};

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(nCmdShow);

	global.window                     = NULL;
	global.thread_installer           = NULL;
	global.thread_receiver            = NULL;
	global.window_menu                = NULL;
	global.test_mode                  = false;
	global.skip_modfolder             = false;
	global.restart_game               = false;
	global.run_voice_program          = true;
	global.last_download_attempt      = false;
	global.condition_index            = -1;
	global.installation_phase         = PHASE_WAITING;
	global.order                      = ORDER_NONE;
	global.command_line_num           = 0;
	global.installation_steps_max     = 0;
	global.saved_alias_array_size     = 0;
	global.download_iterator          = 0;
	global.instruction_index          = 0;
	global.current_mod_version_date   = 0;
	global.installer_version          = 0.61f;
	global.script_version             = 0;
	global.program_arguments          = lpCmdLine;
	global.buffer_log                 = L"";
	global.buffer_status              = L"";
	global.gamerestart_arguments      = L"";
	global.downloaded_filename        = L"";
	global.current_mod                = L"";
	global.missing_modfolders         = L"";
	global.last_pbo_file              = L"";
	global.working_directory          = L"";
	global.current_mod_new_name       = L"";
	global.current_mod_version        = L"";
	global.current_mod_id             = L"";
	global.current_mod_keepname       = L"";
	global.downloaded_filename_last   = L"";
	global.last_log_message           = L"";
	global.game_version               = L"1.99";
	global.lang_eng                   = stringtable[0];
	global.lang                       = stringtable[0];

	// Process arguments
	global.arguments_table.insert(std::pair<std::wstring,std::wstring>(L"gameversion",L"1.99"));
	global.arguments_table.insert(std::pair<std::wstring,std::wstring>(L"assignid",L""));
	global.arguments_table.insert(std::pair<std::wstring,std::wstring>(L"assignidpath",L""));
	global.arguments_table.insert(std::pair<std::wstring,std::wstring>(L"assignname",L""));
	global.arguments_table.insert(std::pair<std::wstring,std::wstring>(L"assignkeepname",L""));
	global.arguments_table.insert(std::pair<std::wstring,std::wstring>(L"installid",L""));
	global.arguments_table.insert(std::pair<std::wstring,std::wstring>(L"installdir",L""));
	global.arguments_table.insert(std::pair<std::wstring,std::wstring>(L"downloadscript",L""));
	global.arguments_table.insert(std::pair<std::wstring,std::wstring>(L"evoice",L""));
	global.arguments_table.insert(std::pair<std::wstring,std::wstring>(L"language",L"English"));

	// Separate arguments:
	// - arguments for this program go to the table
	// - arguments for the gameRestart.exe go to a separate string
	{
		std::vector<std::wstring> argv;
		Tokenize(global.program_arguments, L" ", argv);
	
		for (size_t i=0; i<argv.size(); i++) {
			bool found = false;
		
			for (std::map<std::wstring,std::wstring>::iterator it=global.arguments_table.begin(); it!=global.arguments_table.end(); ++it) {
				std::wstring table_argument = L"-" + it->first + L"=";
			
				if (Equals(argv[i].substr(0, table_argument.length()), table_argument)) {
					it->second = argv[i].substr(table_argument.length());
					found      = true;
					break;
				}
			}
		
			if (!found)
				global.gamerestart_arguments += argv[i] + L" ";
		}
	
		global.test_mode    = global.arguments_table[L"downloadscript"].empty();
		global.game_version = global.arguments_table[L"gameversion"];

		if (Equals(global.arguments_table[L"language"],L"Russian"))
			global.lang = stringtable[1];
	
		if (Equals(global.arguments_table[L"language"],L"Polish"))
			global.lang = stringtable[2];
	
		Tokenize(global.arguments_table[L"installid"] , L",", global.mod_id);
		Tokenize(global.arguments_table[L"installdir"], L",", global.mod_name);
	}

	// Find current directory
	{
		wchar_t pwd[MAX_PATH];
		GetCurrentDirectory(MAX_PATH, pwd);
		global.working_directory = (std::wstring)pwd;

		// When testing outside of the game change path to the game root dir
		if (global.test_mode) {
			global.working_directory = ReplaceAll(global.working_directory, L"\\fwatch\\data", L"");
			SetCurrentDirectory(global.working_directory.c_str());
		}
	}

	// If ordered to create id file for a mod
	if (!global.arguments_table[L"assignidpath"].empty() && !global.arguments_table[L"assignid"].empty()) {
		WriteModID(global.arguments_table[L"assignidpath"], global.arguments_table[L"assignname"], global.arguments_table[L"assignid"], global.arguments_table[L"assignkeepname"]);
		return 0;
	}

	// Get system font
	NONCLIENTMETRICS ncMetrics = {0};
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

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex = {0};
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style         = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc   = WndProc;
	wcex.cbClsExtra    = 0;
	wcex.cbWndExtra    = 0;
	wcex.hInstance     = hInstance;
	wcex.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ADDONINSTARRERGUI));
	wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName  = MAKEINTRESOURCE(IDC_ADDONINSTARRERGUI);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm       = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;
	hInst = hInstance;

	RECT desktop;
	GetWindowRect(GetDesktopWindow(), &desktop);
	int x = ((desktop.right - window_w) / 2);
	int y = ((desktop.bottom - window_h) / 2);

	hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, x, y, window_w, window_h, NULL, NULL, hInstance, NULL);
   if (!hWnd)
      return FALSE;

	global.window = hWnd;
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message) {
		case WM_CREATE: {
			global.window_menu = GetMenu(hWnd);
			EnableMenuItem(global.window_menu, ID_PROCESS_RETRY, MF_BYCOMMAND | MF_GRAYED);

			INITCOMMONCONTROLSEX icex = {};
            icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
            icex.dwICC = ICC_TAB_CLASSES;
            InitCommonControlsEx(&icex);

			CalculateWindowSizes(hWnd);

			struct WindowTemplate {
				wchar_t type[32];
				wchar_t title[32];
				DWORD style;
			};

			WindowTemplate list[CONTROLS_MAX] = {};
			#define initWT(ID, TYPE, TITLE, STYLE) wcscpy_s(list[ID].type, 32, TYPE); wcscpy_s(list[ID].title, 32, TITLE); list[ID].style=STYLE;
			
			initWT(TAB, WC_TABCONTROLW, L"", WS_TABSTOP);
			initWT(GRAY_BACKGROUND, WC_STATICW, L"", SS_LEFT);
			initWT(LOG_GENERAL, WC_EDITW, L"", SS_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | ES_READONLY | WS_BORDER | WS_VSCROLL);
			initWT(LOG_DETAIL, WC_EDITW, L"", SS_LEFT | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | ES_READONLY | WS_BORDER | WS_VSCROLL);

			initWT(TXT_MOD_NAME, WC_STATICW, L"Mod name:", SS_CENTERIMAGE);
			initWT(INPUT_MOD_NAME, WC_EDITW, L"@insttest", WS_DLGFRAME | ES_AUTOHSCROLL | WS_TABSTOP);
			initWT(TXT_DIR_NAME, WC_STATICW, L"Dir name:", SS_CENTERIMAGE);
			initWT(INPUT_DIR_NAME, WC_EDITW, L"", WS_DLGFRAME | ES_AUTOHSCROLL | WS_TABSTOP);
			initWT(TXT_GAME_VER, WC_STATICW, L"Game version:", SS_CENTERIMAGE);
			initWT(INPUT_GAME_VER, WC_EDITW, L"1.99", WS_DLGFRAME | WS_TABSTOP);
			initWT(TXT_COMMANDS, WC_STATICW, L"Commands:", SS_CENTERIMAGE);
			initWT(TXT_DL_SIZE, WC_STATICW, L"Total Download Size: 0 B", SS_RIGHT);
			initWT(LIST_COMMANDS, WC_LISTBOXW, L"", WS_DLGFRAME | LBS_NOTIFY | WS_VSCROLL | LBS_NOINTEGRALHEIGHT | LBS_OWNERDRAWFIXED | WS_TABSTOP);

			initWT(BUTTON_OPEN_MOD, WC_BUTTONW, L"Open mod folder", WS_TABSTOP);
			initWT(BUTTON_OPEN_TMP, WC_BUTTONW, L"Open fwatch\\tmp\\_extracted", WS_TABSTOP);
			initWT(BUTTON_REWIND, WC_BUTTONW, L"|<", WS_TABSTOP);
			initWT(BUTTON_BACK, WC_BUTTONW, L"<<", WS_TABSTOP);
			initWT(BUTTON_NEXT, WC_BUTTONW, L">>", WS_TABSTOP);
			initWT(BUTTON_PLAY, WC_BUTTONW, L">", WS_TABSTOP);

			initWT(TESTING_SEPARATOR, WC_STATICW, L"", SS_ETCHEDHORZ | SS_SUNKEN);
			initWT(TXT_COMMAND_INFO0, WC_STATICW, L"", SS_LEFT);
			initWT(TXT_COMMAND_INFO1, WC_STATICW, L"", SS_LEFT);
			initWT(TXT_COMMAND_INFO2, WC_STATICW, L"", SS_LEFT);
			initWT(TXT_COMMAND_INFO3, WC_STATICW, L"", SS_LEFT);

			initWT(TXT_DOWNLOADS, WC_STATICW, L"Download:", SS_LEFT);
			initWT(LIST_DOWNLOADS, WC_LISTBOXW, L"", WS_DLGFRAME | LBS_NOTIFY | WS_VSCROLL | LBS_NOINTEGRALHEIGHT | WS_TABSTOP);
			initWT(TXT_FILENAME, WC_STATICW, L"File name:", SS_LEFT);
			initWT(TXT_DL_ARGS, WC_STATICW, L"Intermediate pages:", SS_LEFT);
			initWT(LIST_DL_ARGS, WC_LISTBOXW, L"", WS_DLGFRAME | LBS_NOTIFY | WS_VSCROLL | LBS_NOINTEGRALHEIGHT | WS_TABSTOP);

			initWT(BUTTON_JUMP_TO_STEP, WC_BUTTONW, L"Jump to this step", WS_TABSTOP);
			initWT(BUTTON_JUMP_TO_LINE, WC_BUTTONW, L"Show in script", WS_TABSTOP);
			initWT(BUTTON_OPEN_DOC, WC_BUTTONW, L"Open Documentation", WS_TABSTOP);

			initWT(EDIT_SCRIPT, WC_EDITW, L"", SS_LEFT | ES_MULTILINE | WS_VSCROLL | WS_HSCROLL | WS_TABSTOP);
			initWT(TXT_LINE_NUMBER, WC_STATICW, L"Line:", SS_CENTERIMAGE);
			initWT(BUTTON_SAVETEST, WC_BUTTONW, L"Save and Test", WS_TABSTOP);
			initWT(BUTTON_RELOAD, WC_BUTTONW, L"Reload file", WS_TABSTOP);
			initWT(BUTTON_OPEN_DOC_GENERAL, WC_BUTTONW, L"Documentation", WS_TABSTOP);
			initWT(BUTTON_CONVERT_DL, WC_BUTTONW, L"Convert download link", WS_TABSTOP);
			initWT(BUTTON_INSERT_DTA, WC_BUTTONW, L"Insert DTA template", WS_TABSTOP);

			for (int i=0; i<CONTROLS_MAX; i++) {
				global.controls[i] = CreateWindow(list[i].type, list[i].title, WS_CHILD | WS_VISIBLE | list[i].style, controls_pos[i].left,controls_pos[i].top,controls_pos[i].right,controls_pos[i].bottom, hWnd, (HMENU)(UINT_PTR)(ID_BASE+i), NULL, NULL);
				SendMessage(global.controls[i], WM_SETFONT, (WPARAM)system_font, TRUE);
			}

			TCITEMW tie        = {};
			tie.mask           = TCIF_TEXT;
			wchar_t tabs[][34] = {L"Log", L"Testing", L"Edit Script"};
			int max            = global.test_mode ? sizeof(tabs)/sizeof(tabs[0]) : 1;

			for (int i=0; i<max; i++) {
				tie.pszText = tabs[i];
				SendMessageW(global.controls[TAB], TCM_INSERTITEMW, i, (LPARAM) (LPTCITEM) &tie);
			}

			SetWindowSubclass(global.controls[EDIT_SCRIPT], (SUBCLASSPROC)&EditScript, 1, 0);
			SwitchTab(global.test_mode ? INSTALLER_TAB_INSTRUCTIONS : INSTALLER_TAB_LOG);

			DWORD threadID1 = 0;
			CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)addonInstallerWrapper, 0, 0,&threadID1);

			if (global.test_mode) {
				DWORD threadID2 = 0;
				CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)UpdateLineNumber, 0, 0,&threadID2);
			}
		} break;

		case WM_SIZE: {
			CalculateWindowSizes(hWnd);

			for (int i=0; i<CONTROLS_MAX; i++)
				SetWindowPos(global.controls[i], NULL, controls_pos[i].left,controls_pos[i].top,controls_pos[i].right,controls_pos[i].bottom, SWP_NOZORDER);
		} break;

		case WM_COMMAND: {
			wmId       = LOWORD(wParam);
			wmEvent    = HIWORD(wParam);

			// Parse the menu selections:
			switch (wmId) {
				case (ID_BASE+BUTTON_BACK) : {
					if (global.order == ORDER_NONE)
					global.order = ORDER_PREV;
				} break;

				case (ID_BASE+BUTTON_NEXT) : {
					if (global.order == ORDER_NONE)
					global.order = ORDER_NEXT;
				} break;

				case (ID_BASE+BUTTON_REWIND) : {
					if (global.order == ORDER_NONE)
						global.order = ORDER_REWIND;
				} break;

				case (ID_BASE+BUTTON_PLAY) : {
					if (global.order == ORDER_NONE) {
						global.order = ORDER_PLAY;
						EnableWindow(global.controls[BUTTON_PLAY], 0);
					}
				} break;

				case (ID_BASE+BUTTON_SAVETEST) : {
					if (global.order == ORDER_NONE)
						global.order = ORDER_RELOAD;
				} break;

				case (ID_BASE+BUTTON_RELOAD) : {
					int pressed = MessageBox(global.window, L"You'll lose changes. Are you sure?", L"Script Editor", MB_ICONQUESTION | MB_YESNO);
					if (pressed == IDYES) {
						std::wstring script_file_content = GetFileContents(PATH_TO_TEST_SCRIPT);
						SetWindowText(global.controls[EDIT_SCRIPT], script_file_content.c_str());
					}
				} break;

				case (ID_BASE+BUTTON_OPEN_MOD) : {
					HINSTANCE result = ShellExecute(NULL, L"open", global.current_mod_new_name.c_str(), NULL, NULL, SW_SHOWDEFAULT);
					if ((INT_PTR)result < 32)
						ShellExecute(NULL, L"open", L".", NULL, NULL, SW_SHOWDEFAULT);
				} break;

				case (ID_BASE+BUTTON_OPEN_TMP) : {
					HINSTANCE result = ShellExecute(NULL, L"open", L"fwatch\\tmp\\_extracted", NULL, NULL, SW_SHOWDEFAULT);
					if ((INT_PTR)result < 32)
						ShellExecute(NULL, L"open", L"fwatch\\tmp", NULL, NULL, SW_SHOWDEFAULT);
				} break;

				case (ID_BASE+BUTTON_OPEN_DOC_GENERAL) : {
					ShellExecute(NULL, L"open", DOCUMENTATION_URL, NULL, NULL, SW_SHOWDEFAULT);
				} break;

				case (ID_BASE+BUTTON_CONVERT_DL) : {
					DialogBox(hInst, MAKEINTRESOURCE(IDD_CONVERT_DL), hWnd, ConvertDownloadLink);
				} break;

				case (ID_BASE+BUTTON_INSERT_DTA) : {
					wchar_t dta_template[] = L"IF_VERSION  <=  1.96\r\n\tUNPBO <game>\\Res\\Dta\\HWTL\\data.pbo  dta\\HWTL\r\n\tMOVE new_textures\\*.pa?  dta\\HWTL\\Data\r\n\tMAKEPBO\r\n\t\r\n\tUNPBO <game>\\Res\\Dta\\HWTL\\data3d.pbo  dta\\HWTL\r\n\tMOVE new_models\\*.p3d  dta\\HWTL\\data3d\r\n\tMAKEPBO\r\nELSE\r\n\tUNPBO <game>\\DTA\\Data.pbo  dta\r\n\tMOVE new_textures\\*.pa?  dta\\Data\r\n\tMAKEPBO\r\n\t\r\n\tUNPBO <game>\\DTA\\Data3D.pbo  dta\r\n\tMOVE new_models\\*.p3d  dta\\Data3D\r\n\tMAKEPBO\r\nENDIF";
					SendMessage(global.controls[EDIT_SCRIPT], EM_REPLACESEL, TRUE, (LPARAM)dta_template);
				} break;

				case (ID_BASE+BUTTON_OPEN_DOC) : {
					LRESULT result = SendMessage(global.controls[LIST_COMMANDS], LB_GETCURSEL, 0, 0);
					if (result != LB_ERR) {
						std::wstring anchor = L"";
						switch(global.commands[result].id) {
							case COMMAND_AUTO_INSTALL : anchor=L"auto_installation"; break;
							case COMMAND_DOWNLOAD : anchor=L"get"; break;
							case COMMAND_UNPACK : anchor=L"unpack"; break;
							case COMMAND_MOVE : 
							case COMMAND_COPY : anchor=L"move"; break;
							case COMMAND_MAKEDIR : anchor=L"makedir"; break;
							case COMMAND_ASK_RUN : anchor=L"ask_run"; break;
							case COMMAND_DELETE : anchor=L"delete"; break;
							case COMMAND_RENAME : anchor=L"rename"; break;
							case COMMAND_ASK_DOWNLOAD : anchor=L"ask_get"; break;
							case COMMAND_IF_VERSION : 
							case COMMAND_ELSE : 
							case COMMAND_ENDIF : anchor=L"if_version"; break;
							case COMMAND_MAKEPBO : anchor=L"makepbo"; break;
							case COMMAND_EXTRACTPBO : anchor=L"unpbo"; break;
							case COMMAND_EDIT : anchor=L"edit"; break;
							case COMMAND_ALIAS : anchor=L"alias"; break;
							case COMMAND_FILEDATE : anchor=L"filedate"; break;
							case COMMAND_EXIT : anchor=L"exit"; break;
						}
						if (!anchor.empty()) {
							anchor = (std::wstring)DOCUMENTATION_URL + L"#" + anchor;
							ShellExecute(NULL, L"open", anchor.c_str(), NULL, NULL, SW_SHOWDEFAULT);
						}					
					}
				} break;

				case (ID_BASE+BUTTON_JUMP_TO_LINE) : {
					LRESULT result = SendMessage(global.controls[LIST_COMMANDS], LB_GETCURSEL, 0, 0);
					if (result != LB_ERR) {
						SwitchTab(INSTALLER_TAB_SCRIPT);

						LRESULT start = SendMessage(global.controls[EDIT_SCRIPT], EM_LINEINDEX, global.commands[result].line_num-1, -1);
						LRESULT length = SendMessage(global.controls[EDIT_SCRIPT], EM_LINELENGTH, start, -1);
						if (start > -1) {
							SendMessage(global.controls[EDIT_SCRIPT], EM_SETSEL, start, start+length);
							SendMessage(global.controls[EDIT_SCRIPT], EM_SCROLLCARET , 0, 0);

							// refresh the window
							ShowWindow(global.controls[EDIT_SCRIPT], 0);
							ShowWindow(global.controls[EDIT_SCRIPT], 1);
							SetFocus(global.controls[EDIT_SCRIPT]);
						}
					}
				} break;

				case (ID_BASE+BUTTON_JUMP_TO_STEP) : {
					if (global.order == ORDER_NONE)
						global.order = ORDER_JUMP;
				} break;

				case (ID_BASE+LIST_COMMANDS) : {
					if (wmEvent == LBN_SELCHANGE) {
						ShowCommandInfo();
					}
				} break;

				case (ID_BASE+LIST_DOWNLOADS) : {
					if (wmEvent == LBN_SELCHANGE) {
						ShowDownloadInfo();
					}
				} break;

				case ID_PROCESS_ABORT: {
						global.order = ORDER_ABORT;
					EnableWindowMenu(false);
				} break;

				case ID_OPTIONS_RESTARTGAME: {
					global.restart_game = !global.restart_game;
					CheckMenuItem(global.window_menu, wmId, global.restart_game ? MF_CHECKED : MF_UNCHECKED);
				} break;

				case ID_PROCESS_PAUSE: {
					if (global.order == ORDER_NONE)
						global.order = ORDER_PAUSE;
					CheckMenuItem(global.window_menu, wmId, global.order==ORDER_PAUSE ? MF_CHECKED : MF_UNCHECKED);
				} break;

				case ID_PROCESS_RETRY: {
					if (global.order == ORDER_NONE)
						global.order = ORDER_RETRY;
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

        case WM_NOTIFY: { 
			wmEvent = ((LPNMHDR)lParam)->code;
			switch (wmEvent) {
				case TCN_SELCHANGE: {
					SwitchTab((INSTALLER_TAB)TabCtrl_GetCurSel(global.controls[TAB]));
				} break;
			} break;
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
				if (global.order != ORDER_ABORT) {
					global.order = ORDER_ABORT;
					EnableWindowMenu(false);
				}
		} break;

		case WM_DRAWITEM: {
			if (wParam == (ID_BASE+LIST_COMMANDS)) {
				DrawListofCommands(hWnd, (UINT)wParam, (DRAWITEMSTRUCT *)lParam);
			}
		} break;

		case WM_MEASUREITEM: {
			PMEASUREITEMSTRUCT pmis = (PMEASUREITEMSTRUCT) lParam; 
			pmis->itemHeight = 16; 
			return TRUE;
		} break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	switch (message) {
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}

	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK ConvertDownloadLink(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam) 
{
	UNREFERENCED_PARAMETER(lParam);

    switch (message) { 
		case WM_INITDIALOG: {
			ShowWindow(GetDlgItem(hwndDlg, IDC_TXT_FNAME), SW_HIDE);
			ShowWindow(GetDlgItem(hwndDlg, IDC_EDIT_FNAME), SW_HIDE);
			ShowWindow(GetDlgItem(hwndDlg, IDC_TXT_CONFIRM), SW_HIDE);
			ShowWindow(GetDlgItem(hwndDlg, IDC_CHECK_CONFIRM), SW_HIDE);
			ShowWindow(GetDlgItem(hwndDlg, IDC_TXT_CONFIRM2), SW_HIDE);
			ShowWindow(GetDlgItem(hwndDlg, IDOK), SW_HIDE);
			return (INT_PTR)TRUE;
		} break;

        case WM_COMMAND: 
            switch (LOWORD(wParam)) {
				case IDC_EDIT_URL: {
					if (HIWORD(wParam) == EN_CHANGE) {
						std::wstring sites[] = {
							L"drive.google.com",
							L"moddb.com/mods/",
							L"moddb.com/downloads/start",
							L"mediafire.com/file/",
							L"ds-servers.com",
							L"ofpec.com",
							L"sendspace.com",
							L"lonebullet.com",
							L"dropbox.com/s/"
						};
						enum INDEXES {
							GDRIVE,
							MODDB_1,
							MODDB_2,
							MEDIAFIRE,
							DSSERVERS,
							OFPEC,
							SENDSPACE,
							LONEBULLET,
							DROPBOX
						};
						std::wstring url = L"";
						WindowTextToString(GetDlgItem(hwndDlg, IDC_EDIT_URL), url);
						Trim(url);

						int index = -1;
						size_t domain_pos = std::wstring::npos;

						for (int i=0; i<sizeof(sites)/sizeof(sites[0]) && index==-1; i++) {
							domain_pos = url.find(sites[i]);
							if (domain_pos != std::wstring::npos)
								index = i;
						}

						ShowWindow(GetDlgItem(hwndDlg, IDC_TXT_FNAME), index>=GDRIVE ? SW_SHOW : SW_HIDE);
						ShowWindow(GetDlgItem(hwndDlg, IDC_EDIT_FNAME), index>=GDRIVE ? SW_SHOW : SW_HIDE);
						ShowWindow(GetDlgItem(hwndDlg, IDC_TXT_CONFIRM), index==GDRIVE ? SW_SHOW : SW_HIDE);
						ShowWindow(GetDlgItem(hwndDlg, IDC_CHECK_CONFIRM), index==GDRIVE ? SW_SHOW : SW_HIDE);
						ShowWindow(GetDlgItem(hwndDlg, IDC_TXT_CONFIRM2), index==GDRIVE ? SW_SHOW : SW_HIDE);
						ShowWindow(GetDlgItem(hwndDlg, IDOK), SW_HIDE);
						SendMessage(GetDlgItem(hwndDlg, IDC_CHECK_CONFIRM), BM_SETCHECK, BST_UNCHECKED, 0);

						std::wstring final_url       = L"";
						std::wstring name_start      = L"";
						std::wstring name_end        = L"";
						std::wstring token_file_name = L"fwatch\\tmp\\__downloadtoken";
						bool reverse                 = false;

						switch(index) {
							case GDRIVE : {
								size_t id_pos1 = url.find(L"id=");
								size_t id_pos2 = url.find(L"/d/");
								std::wstring id = L"";

								if (id_pos1 != std::wstring::npos)
									id = url.substr(id_pos1+3);
								else
									if (id_pos2 != std::wstring::npos) {
										size_t slash = url.find(L"/",id_pos2+4);
										if (slash != std::wstring::npos) {
											id_pos2+=3;
											id = url.substr(id_pos2, slash-id_pos2);
										}
									}

								if (!id.empty()) {
									name_start = L"<meta itemprop=\"name\" content=\"";
									name_end   = L"\">";
									final_url  = L"https://docs.google.com/uc?export=download&id=" + id;
									
									if (Download(L"-S --spider "+final_url, FLAG_OVERWRITE | FLAG_SILENT_MODE) == ERROR_NONE)
										if (GetFileContents(L"fwatch\\tmp\\schedule\\downloadLog.txt").find(L"Content-Type: text/html;") != std::wstring::npos)
											SendMessage(GetDlgItem(hwndDlg, IDC_CHECK_CONFIRM), BM_SETCHECK, BST_CHECKED, 0);
								}
							} break;

							case MODDB_1 : {
								final_url  = url + L" /downloads/start/ /downloads/mirror/";
								name_start = L"<h5>Filename</h5>";
								name_end   = L"</div>";
							} break;

							case MODDB_2 : {
								size_t query_pos = url.find(L'?');
			
								if (query_pos == std::wstring::npos)
									query_pos = url.length();
			
								final_url = url.substr(0, query_pos) + L" /downloads/mirror/ ";

								name_start = L">download ";
								name_end   = L"</a>";
							} break;

							case MEDIAFIRE : {
								std::wstring sub = url.substr(domain_pos+sites[index].length());
								size_t slash     = sub.find_last_of(L"/");
			
								if (slash != std::wstring::npos) {
									final_url = url.substr(0,slash);
								} else {
									final_url = url;
								}
			
								final_url += L" ://download ";
								name_start = L"<div class=\"filename\">";
								name_end   = L"</div>";
							} break;

							case DSSERVERS : {
								bool gf = url.find(L"files/gf/") != std::wstring::npos;
								final_url = url + (gf ? L" store.node " : L" files/gf/ store.node ");

								if (gf) {
									name_start = L"<title>";
									name_end   = L" &bull;";
								} else {
									name_start = L"<dt>File Name:</dt>";
									name_end   = L"<dt>";
								}
							} break;

							case OFPEC : {
								final_url = url + (url.find(L"download.php")!=std::wstring::npos ? L" " : L" download.php ");
							} break;

							case SENDSPACE : {
								final_url  = url + L" sendspace.com/dl ";
								name_start = L";\"><b>";
								name_end   = L"</b></h2>";
							} break;

							case LONEBULLET : {
								final_url = url + L" /file/ files.lonebullet.com ";
								break;
							} break;

							case DROPBOX : {
								final_url  = ReplaceAll(url, L"?dl=0", L"?dl=1") + L" ";
								name_start = L"\" property=\"og:title\"";
								name_end   = L"<meta content=\"";
								reverse    = true;
							} break;
						}

						if (!name_start.empty() && !name_end.empty()) {
							INSTALLER_ERROR_CODE result = Download(L" --output-document="+token_file_name+L" "+url, FLAG_OVERWRITE | FLAG_SILENT_MODE);
							if (result == ERROR_NONE) {
								std::wstring token_file_buffer = GetFileContents(token_file_name);
								size_t offset = 0;
								std::wstring html = GetTextBetween(token_file_buffer, name_start, name_end, offset, reverse);

								bool inside_tag = false;
								for (size_t i=0; i<html.size(); i++) {
									if (html[i] == L'<' && !inside_tag)
										inside_tag = true;

									if (html[i] == L'>' && inside_tag) {
										inside_tag = false;
										html[i] = ' ';
									}

									if (inside_tag)
										html[i] = L' ';
								}

								html = Trim(html);
								SetWindowText(GetDlgItem(hwndDlg, IDC_EDIT_FNAME), html.c_str());
							}
						}

						SetWindowText(GetDlgItem(hwndDlg, IDC_FINAL_URL), final_url.c_str());
					}
					return (INT_PTR)TRUE;
				} break;

				case IDC_EDIT_FNAME: {
					if (HIWORD(wParam) == EN_CHANGE) {
						std::wstring filename = L"";
						WindowTextToString(GetDlgItem(hwndDlg, IDC_EDIT_FNAME), filename);
						Trim(filename);
						ShowWindow(GetDlgItem(hwndDlg, IDOK), filename.empty() ? SW_HIDE : SW_SHOW);
					}
					return (INT_PTR)TRUE;
				} break;

                case IDOK: {
					std::wstring url = L"";
					WindowTextToString(GetDlgItem(hwndDlg, IDC_FINAL_URL), url);

					std::wstring filename = L"";
					WindowTextToString(GetDlgItem(hwndDlg, IDC_EDIT_FNAME), filename);
					Trim(filename);

					if (!url.empty() && !filename.empty()) {
						if (filename.find(L" ") != std::wstring::npos)
							filename = L"\"" + filename + L"\"";
			
						if (IsWindowVisible(GetDlgItem(hwndDlg, IDC_CHECK_CONFIRM)) && IsDlgButtonChecked(hwndDlg, IDC_CHECK_CONFIRM))
							url += L" /download";
			
						url += L" " + filename + L"\r\n";

						SendMessage(global.controls[EDIT_SCRIPT], EM_REPLACESEL, TRUE, reinterpret_cast<LPARAM>(url.c_str()));
						SetFocus(global.controls[EDIT_SCRIPT]);
						EndDialog(hwndDlg, wParam); 
					}

                    return (INT_PTR)TRUE;
				}

				case IDCANCEL: 
					EndDialog(hwndDlg, wParam); 
					return (INT_PTR)TRUE;
            } 
    } 
    return (INT_PTR)FALSE;
} 

LRESULT CALLBACK EditScript(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	UNREFERENCED_PARAMETER(uIdSubclass);
	UNREFERENCED_PARAMETER(dwRefData);

	switch (uMsg) {
		case WM_CHAR : {
			if (wParam == 1 || wParam == 4) //disable beeping for ctrl+a and ctrl+d
				return 0;
		} break;

		case WM_KEYDOWN : {
			if (GetKeyState(VK_CONTROL) & 0x8000) {
				switch(wParam) {
					// Select all
					case 'A': SendMessage(hWnd, EM_SETSEL, 0, -1); break;

					// Duplicate line
					case 'D': {
						DWORD sel_start = 0;
						DWORD sel_end   = 0;
						SendMessageW(global.controls[EDIT_SCRIPT], EM_GETSEL, (LPARAM)&sel_start, (LPARAM)&sel_end);

						DWORD line_num    = (DWORD)SendMessageW(global.controls[EDIT_SCRIPT], EM_LINEFROMCHAR, UINT_MAX, 0);
						DWORD line_index  = (DWORD)SendMessageW(global.controls[EDIT_SCRIPT], EM_LINEINDEX, UINT_MAX, 0);
						DWORD line_length = (DWORD)SendMessage(global.controls[EDIT_SCRIPT], EM_LINELENGTH, sel_start, -1);
						DWORD line_count  = (DWORD)SendMessage(global.controls[EDIT_SCRIPT], EM_GETLINECOUNT, 0, 0);

						std::wstring text;
						text.reserve(line_length+3);
						text.resize(line_length);
						*reinterpret_cast<WORD *>(&text[0]) = (WORD)text.size();
						SendMessage(global.controls[EDIT_SCRIPT], EM_GETLINE, line_num, (LPARAM)&text[0]);

						if (line_num < line_count-1) {
							text += L"\r\n";
							SendMessage(global.controls[EDIT_SCRIPT], EM_SETSEL, line_index+line_length+2, line_index+line_length+2); //move caret to the next line
							SendMessage(global.controls[EDIT_SCRIPT], EM_REPLACESEL, TRUE, (LPARAM)(LPCSTR)text.c_str()); //duplicate line
							DWORD new_selection = line_index+line_length+2 + (sel_start-line_index); // set caret to the same line column but line below the original pos
							SendMessage(global.controls[EDIT_SCRIPT], EM_SETSEL, new_selection, new_selection);
						} else {
							SendMessage(global.controls[EDIT_SCRIPT], EM_SETSEL, line_index+line_length, line_index+line_length); //move caret to the end of the current line
							wchar_t new_line[] = L"\r\n";
							SendMessage(global.controls[EDIT_SCRIPT], EM_REPLACESEL, TRUE, (LPARAM)new_line); //insert new line
							SendMessage(global.controls[EDIT_SCRIPT], EM_REPLACESEL, TRUE, (LPARAM)(LPCSTR)text.c_str()); //duplicate line
						}
					} break;

					// Save
					case 'S': {
						if (global.order == ORDER_NONE)
							global.order = ORDER_RELOAD;
					} break;
				}
			}
		} break;
	}

	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

DWORD WINAPI UpdateLineNumber(__in LPVOID lpParameter)
{
	UNREFERENCED_PARAMETER(lpParameter);
	for (;;) {
		int result = (int)SendMessageW(global.controls[EDIT_SCRIPT], EM_LINEFROMCHAR, UINT_MAX, 0);
		std::wstring temp = L"Line: " + Int2StrW(++result);
		SetWindowText(global.controls[TXT_LINE_NUMBER], temp.c_str());
		Sleep(10);
	}
}

void CalculateWindowSizes(HWND window)
{
	RECT dialogspace;
	GetClientRect(window, &dialogspace);

	controls_pos[TAB].left     = 0;
	controls_pos[TAB].top      = 0;
	controls_pos[TAB].right    = dialogspace.right;
	controls_pos[TAB].bottom   = 24;
	int dialogspace_h_aftertab = dialogspace.bottom - controls_pos[TAB].bottom;

	controls_pos[GRAY_BACKGROUND]        = dialogspace;
	controls_pos[GRAY_BACKGROUND].top    = controls_pos[TAB].bottom;
	controls_pos[GRAY_BACKGROUND].bottom = dialogspace.bottom - controls_pos[TAB].bottom;

	controls_pos[LOG_GENERAL]        = dialogspace;
	controls_pos[LOG_GENERAL].top    = controls_pos[TAB].bottom;
	controls_pos[LOG_GENERAL].bottom = (int)((double)dialogspace_h_aftertab / 1.3);

	controls_pos[LOG_DETAIL]        = dialogspace;
	controls_pos[LOG_DETAIL].top    = controls_pos[LOG_GENERAL].top + controls_pos[LOG_GENERAL].bottom;
	controls_pos[LOG_DETAIL].bottom = dialogspace_h_aftertab - controls_pos[LOG_GENERAL].bottom;

	controls_pos[TXT_MOD_NAME].left   = 5;
	controls_pos[TXT_MOD_NAME].top    = controls_pos[LOG_GENERAL].top + 5;
	controls_pos[TXT_MOD_NAME].right  = 80;
	controls_pos[TXT_MOD_NAME].bottom = 20;

	controls_pos[INPUT_MOD_NAME].left   = controls_pos[TXT_MOD_NAME].left + controls_pos[TXT_MOD_NAME].right;
	controls_pos[INPUT_MOD_NAME].top    = controls_pos[TXT_MOD_NAME].top;
	controls_pos[INPUT_MOD_NAME].right  = 150;
	controls_pos[INPUT_MOD_NAME].bottom = controls_pos[TXT_MOD_NAME].bottom;

	controls_pos[TXT_DIR_NAME]      = controls_pos[TXT_MOD_NAME];
	controls_pos[TXT_DIR_NAME].top += controls_pos[TXT_MOD_NAME].bottom;

	controls_pos[INPUT_DIR_NAME]      = controls_pos[INPUT_MOD_NAME];
	controls_pos[INPUT_DIR_NAME].top += controls_pos[INPUT_MOD_NAME].bottom;		

	controls_pos[TXT_GAME_VER]      = controls_pos[TXT_DIR_NAME];
	controls_pos[TXT_GAME_VER].top += controls_pos[TXT_DIR_NAME].bottom;

	controls_pos[INPUT_GAME_VER]      = controls_pos[INPUT_DIR_NAME];
	controls_pos[INPUT_GAME_VER].top += controls_pos[INPUT_DIR_NAME].bottom;

	controls_pos[BUTTON_OPEN_MOD]       = controls_pos[INPUT_MOD_NAME];
	controls_pos[BUTTON_OPEN_MOD].left  = controls_pos[INPUT_MOD_NAME].left + controls_pos[INPUT_MOD_NAME].right + 20;
	controls_pos[BUTTON_OPEN_MOD].right = controls_pos[INPUT_MOD_NAME].right + 20;
	controls_pos[BUTTON_OPEN_MOD].top  += controls_pos[INPUT_MOD_NAME].bottom / 2;
	
	controls_pos[BUTTON_OPEN_TMP]     = controls_pos[BUTTON_OPEN_MOD];
	controls_pos[BUTTON_OPEN_TMP].top = controls_pos[BUTTON_OPEN_MOD].top + controls_pos[BUTTON_OPEN_MOD].bottom;

	int button_size = 25;
	int button_y    = (controls_pos[BUTTON_OPEN_MOD].bottom + controls_pos[BUTTON_OPEN_TMP].bottom - button_size) / 2;
	controls_pos[BUTTON_REWIND].left   = controls_pos[BUTTON_OPEN_TMP].left + controls_pos[BUTTON_OPEN_TMP].right + 20;
	controls_pos[BUTTON_REWIND].right  = button_size;
	controls_pos[BUTTON_REWIND].top    = controls_pos[BUTTON_OPEN_MOD].top + button_y;
	controls_pos[BUTTON_REWIND].bottom = button_size;
			
	for (int i=BUTTON_REWIND, index=0; i<=BUTTON_PLAY; i++, index++) {
		if (i < BUTTON_PLAY) {
			controls_pos[i+1]      = controls_pos[i];
			controls_pos[i+1].left = controls_pos[i].left + controls_pos[i].right + 5;
		}
	}

	controls_pos[TESTING_SEPARATOR].left   = 10;
	controls_pos[TESTING_SEPARATOR].top    = controls_pos[INPUT_GAME_VER].top + controls_pos[INPUT_GAME_VER].bottom + 10;
	controls_pos[TESTING_SEPARATOR].right  = dialogspace.right - 20;
	controls_pos[TESTING_SEPARATOR].bottom = 2;

	controls_pos[TXT_COMMANDS]       = controls_pos[TXT_MOD_NAME];
	controls_pos[TXT_COMMANDS].top   = controls_pos[TESTING_SEPARATOR].top + 10;

	controls_pos[TXT_DL_SIZE]       = controls_pos[TXT_COMMANDS];
	controls_pos[TXT_DL_SIZE].right = 200;
	controls_pos[TXT_DL_SIZE].left  = dialogspace.right - 10 - controls_pos[TXT_DL_SIZE].right;

	controls_pos[LIST_COMMANDS]        = controls_pos[TXT_COMMANDS];
	controls_pos[LIST_COMMANDS].top   += controls_pos[TXT_COMMANDS].bottom;
	controls_pos[LIST_COMMANDS].right  = (int)((double)dialogspace.right / 3.5);
	controls_pos[LIST_COMMANDS].bottom = controls_pos[LOG_DETAIL].top - controls_pos[LIST_COMMANDS].top;

	controls_pos[TXT_COMMAND_INFO0] = controls_pos[TXT_COMMANDS];
	controls_pos[TXT_COMMAND_INFO0].top   = controls_pos[LIST_COMMANDS].top;
	controls_pos[TXT_COMMAND_INFO0].left += controls_pos[LIST_COMMANDS].right + 10;
	controls_pos[TXT_COMMAND_INFO0].right = dialogspace.right - controls_pos[TXT_COMMAND_INFO0].left;

	for (int i=TXT_COMMAND_INFO0; i<=TXT_COMMAND_INFO3; i++) {
		controls_pos[i+1]     = controls_pos[i];
		controls_pos[i+1].top = controls_pos[i].top + controls_pos[i].bottom;
	}	

	controls_pos[LIST_DOWNLOADS]        = controls_pos[TXT_DOWNLOADS];
	controls_pos[LIST_DOWNLOADS].top    = controls_pos[TXT_DOWNLOADS].top + controls_pos[TXT_DOWNLOADS].bottom;
	controls_pos[LIST_DOWNLOADS].bottom = 50;
	controls_pos[LIST_DOWNLOADS].right  = dialogspace.right - controls_pos[TXT_COMMAND_INFO0].left - 10;

	controls_pos[TXT_FILENAME]     = controls_pos[TXT_DOWNLOADS];
	controls_pos[TXT_FILENAME].top = controls_pos[LIST_DOWNLOADS].top + controls_pos[LIST_DOWNLOADS].bottom;

	controls_pos[TXT_DL_ARGS]     = controls_pos[TXT_FILENAME];
	controls_pos[TXT_DL_ARGS].top = controls_pos[TXT_FILENAME].top + controls_pos[TXT_FILENAME].bottom;

	controls_pos[LIST_DL_ARGS]        = controls_pos[LIST_DOWNLOADS];
	controls_pos[LIST_DL_ARGS].top    = controls_pos[TXT_DL_ARGS].top + controls_pos[TXT_DL_ARGS].bottom;
	controls_pos[LIST_DL_ARGS].bottom = 40;

	controls_pos[BUTTON_JUMP_TO_STEP]        = controls_pos[LIST_DL_ARGS];
	controls_pos[BUTTON_JUMP_TO_STEP].top    = controls_pos[LIST_DL_ARGS].top + controls_pos[LIST_DL_ARGS].bottom + 10;
	controls_pos[BUTTON_JUMP_TO_STEP].right  = 140;
	controls_pos[BUTTON_JUMP_TO_STEP].bottom = 20;

	controls_pos[BUTTON_JUMP_TO_LINE]      = controls_pos[BUTTON_JUMP_TO_STEP];
	controls_pos[BUTTON_JUMP_TO_LINE].left = controls_pos[BUTTON_JUMP_TO_STEP].left + controls_pos[BUTTON_JUMP_TO_STEP].right + 10;

	controls_pos[BUTTON_OPEN_DOC]      = controls_pos[BUTTON_JUMP_TO_LINE];
	controls_pos[BUTTON_OPEN_DOC].left = controls_pos[BUTTON_JUMP_TO_LINE].left + controls_pos[BUTTON_JUMP_TO_LINE].right + 10;

	controls_pos[EDIT_SCRIPT].left   = 0;
	controls_pos[EDIT_SCRIPT].top    = controls_pos[TAB].bottom;
	controls_pos[EDIT_SCRIPT].right  = dialogspace.right;
	controls_pos[EDIT_SCRIPT].bottom = dialogspace_h_aftertab - 20;

	controls_pos[TXT_LINE_NUMBER]        = controls_pos[EDIT_SCRIPT];
	controls_pos[TXT_LINE_NUMBER].left   = 10;
	controls_pos[TXT_LINE_NUMBER].right  = 50;
	controls_pos[TXT_LINE_NUMBER].top    = controls_pos[EDIT_SCRIPT].top + controls_pos[EDIT_SCRIPT].bottom;
	controls_pos[TXT_LINE_NUMBER].bottom = dialogspace.bottom - controls_pos[TXT_LINE_NUMBER].top;

	controls_pos[BUTTON_SAVETEST]       = controls_pos[TXT_LINE_NUMBER];
	controls_pos[BUTTON_SAVETEST].left  = controls_pos[TXT_LINE_NUMBER].left + controls_pos[TXT_LINE_NUMBER].right + 10;
	controls_pos[BUTTON_SAVETEST].right = 100;

	controls_pos[BUTTON_RELOAD]      = controls_pos[BUTTON_SAVETEST];
	controls_pos[BUTTON_RELOAD].left = controls_pos[BUTTON_SAVETEST].left + controls_pos[BUTTON_SAVETEST].right + 20;

	controls_pos[BUTTON_OPEN_DOC_GENERAL]      = controls_pos[BUTTON_RELOAD];
	controls_pos[BUTTON_OPEN_DOC_GENERAL].left = controls_pos[BUTTON_RELOAD].left + controls_pos[BUTTON_RELOAD].right + 20;

	controls_pos[BUTTON_CONVERT_DL]      = controls_pos[BUTTON_OPEN_DOC_GENERAL];
	controls_pos[BUTTON_CONVERT_DL].left = controls_pos[BUTTON_OPEN_DOC_GENERAL].left + controls_pos[BUTTON_OPEN_DOC_GENERAL].right + 20;
	controls_pos[BUTTON_CONVERT_DL].right = 140;

	controls_pos[BUTTON_INSERT_DTA]       = controls_pos[BUTTON_CONVERT_DL];
	controls_pos[BUTTON_INSERT_DTA].left  = controls_pos[BUTTON_CONVERT_DL].left + controls_pos[BUTTON_CONVERT_DL].right + 20;
}

BOOL DrawListofCommands(HWND hwnd, UINT uCtrlId, DRAWITEMSTRUCT *dis)
{	
	UNREFERENCED_PARAMETER(uCtrlId);
	UNREFERENCED_PARAMETER(hwnd);

	switch(dis->itemAction) {
		case ODA_SELECT:        
		case ODA_DRAWENTIRE: {
			if (dis->itemState & ODS_SELECTED) {
				SetTextColor(dis->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
				SetBkColor(dis->hDC, GetSysColor(COLOR_HIGHLIGHT));
			} else {
				SetTextColor(dis->hDC, GetSysColor(dis->itemID < global.instruction_index ? COLOR_GRAYTEXT : COLOR_WINDOWTEXT));
				SetBkColor(dis->hDC, GetSysColor(COLOR_WINDOW));
			}

			HFONT font;
			font = CreateFont(
				14, //cHeight
				0,  //cWidth
				0,  //cEscapement
				0,  //cOrientation
				dis->itemID == global.instruction_index ? FW_BOLD : FW_DONTCARE,  //cWeight
				0,  //bItalic
				dis->itemID == global.instruction_index,  //bUnderline
				global.commands[dis->itemID].disable,  //bStrikeOut
				0,  //iCharSet
				0,  //iOutPrecision
				0,  //iClipPrecision
				0,  //iQuality
				0,  //iPitchAndFamily
				L"Aptos" //pszFaceName
			);

			SelectObject(dis->hDC, font);
			ExtTextOut(dis->hDC, 0 , dis->rcItem.top, ETO_OPAQUE, &dis->rcItem, global.commands_lines[dis->itemID].c_str(), (UINT)global.commands_lines[dis->itemID].length(), 0);
			DeleteObject(font);
		} break;
	}

	return TRUE;
}