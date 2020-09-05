#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "..\testdll\testdll.h"		// sharing global vars, enums, structs with dll
#include <tlhelp32.h>				// for findProcess()
#include "winsock2.h"				// for reporting server
#include "..\common_functions.cpp"	// sharing functions with dll

struct ThreadArguments {
	bool is_dedicated_server;
	bool is_custom_master1;
	bool is_custom_master2;
	HANDLE *mailslot;
};

struct GLOBAL_VARIABLES_TESTLAUNCHER {	// variables shared between threads
	int listen_server_port;
	char master_server1[64];
	char master_server2[32];
	bool error_log_enabled;
	bool error_log_started;
} global = {
	-1,
	"",
	"",
	false
};

enum GAME_TYPES {
	GAME_CLIENT,
	GAME_SERVER
};

enum COMMAND_ID {
	C_INFO_ERRORLOG = 6,
	C_RESTART_SERVER = 105,
	C_RESTART_CLIENT,
	C_EXE_SPIG = 123,
	C_EXE_ADDONTEST,
	C_EXE_WGET,
	C_EXE_UNPBO,
	C_EXE_PREPROCESS,
	C_EXE_ADDONINSTALL,
	C_EXE_WEBSITE,
	C_EXE_MAKEPBO
};

unsigned long GetIP(char *host);
char* Tokenize(char *string, char *delimiter, int &i, int string_length, bool square_brackets, bool reverse);
void ReadUIConfig(char *filename, bool *no_ar, bool *is_custom, float *custom, int *customINT);
int ModfolderMissionsTransfer(char *mod, bool is_dedicated_server);
void ModfolderMissionsReturn(bool is_dedicated_server);

void FwatchPresence(ThreadArguments *arg);
void ListenServer(ThreadArguments *arg);
void WatchProgram(ThreadArguments *arg);













int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	if (findProcess("fwatch.exe")) {
		MessageBox( NULL, "Do not duplicate fwatch processes.", "fwatch", MB_OK|MB_ICONSTOP );
		return 1;
	}



	ModfolderMissionsReturn(GAME_CLIENT);
	ModfolderMissionsReturn(GAME_SERVER);



	// Check that all required files are in place
	char reqfiles[][256] = {
		"fwatch/data/wget.exe",
		"fwatch/data/true.sqf",
		"fwatch/data/fwatch_check.sqf",
		"fwatch/data/gameRestart.exe",
		"fwatch/data/ConvertKeyString.sqf",
		"fwatch/data/CurrentLanguage.sqf",
		"fwatch/data/DateDifference.sqf",
		"fwatch/data/DateDifferenceInDays.sqf",
		"fwatch/data/FormatDate.sqf",
		"fwatch/data/InitFLib.sqf",
		"fwatch/data/inKeys.sqf",
		"fwatch/data/InputMulti.sqs",
		"fwatch/data/MainMenu.sqs",
		"fwatch/data/MeasureTime.sqf",
		"fwatch/data/MemMulti.sqs",
		"fwatch/data/ModifyDate.sqf",
		"fwatch/data/onScreenTyping.sqs",
		"fwatch/data/getAspectRatio.sqf",
		"fwatch/data/DePbo.dll",
		"fwatch/data/ExtractPbo.exe",
		"fwatch/data/preproc.exe",
		"fwatch/data/Download.sqs",
		"fwatch/data/MainMenu_fnc.sqf",
		"fwatch/data/addonInstarrer.exe",
		"fwatch/data/7z.dll",
		"fwatch/data/7z.exe",
		"fwatch/data/MakePbo.exe",
		NULL,
	};

	int x = 0;
	char *f;
	while((f = reqfiles[x])[0] != NULL) {
		FILE *h = fopen(f, "rb");
		if(h)
			fclose(h);
		else {
			char foo[512];
			sprintf(foo, "File %s not found!\nEnsure that Fwatch is properly installed.", f);
			MessageBox(NULL, foo, "fwatch error", MB_OK|MB_ICONSTOP);
			exit(0);
		}		
		x++;
	}



	// Create mailslot
	HANDLE mailslot = CreateMailslot(TEXT("\\\\.\\mailslot\\fwatch_mailslot"), 0, MAILSLOT_WAIT_FOREVER, (LPSECURITY_ATTRIBUTES) NULL);

	if (mailslot == INVALID_HANDLE_VALUE) {
		DWORD error_code    = GetLastError();
		char error_msg[512] = "";

		sprintf(error_msg, "Failed to create mailslot - %d: ", error_code);
		int length = strlen(error_msg);

		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, error_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), &error_msg[length], 511-length, 0);
		MessageBox( NULL, error_msg, "fwatch", MB_OK|MB_ICONSTOP );
		return 2;
	}



	// Save info about this instance to a file
	FILE *fi = fopen("fwatch_info.sqf","w");
	if (fi)
		fprintf(fi, "[%d,[", GetCurrentProcessId());



	// Handle exe arguments
	// Remove the ones meant for Fwatch and pass the rest to the game
	int lpCmdLine_size         = strlen(lpCmdLine);
	bool launch_client         = true;
	bool listen_server         = false;
	bool add_nomap             = true;
	bool launch_steam          = false;
	bool first_item            = true;
	bool buffer_shift          = false;
	int custom_exe             = -1;
	ThreadArguments client_arg = {false, false, false, &mailslot};

	for (int i=0, begin=-1; i<=lpCmdLine_size; i++) {
		bool complete_word = i == lpCmdLine_size;

		if (isspace(lpCmdLine[i]))
			complete_word = true;

		if (!complete_word  &&  begin<0)
			begin = i;

		if (complete_word  &&  begin>=0) {
			char *word   = lpCmdLine + begin;
			char prev    = lpCmdLine[i];
			lpCmdLine[i] = '\0';

			if (strncmp(word,"-nomap",6) == 0)
				add_nomap = false;

			if (strncmp(word,"-nolaunch",9) == 0) {
				launch_client = false;
				buffer_shift  = true;
			}

			if (strncmp(word,"-gamespy=",9) == 0) {
				strncpy(global.master_server1, word+9, 64);
				client_arg.is_custom_master1 = true;
				buffer_shift = true;
			}

			if (strncmp(word,"-udpsoft=",9) == 0) {
				strncpy(global.master_server2, word+9, 18);
				client_arg.is_custom_master2 = true;
				buffer_shift = true;
			}

			if (strncmp(word,"-reporthost",11) == 0) {
				listen_server = true;
				buffer_shift  = true;
			}

			if (strncmp(word,"-removenomap",12) == 0) {
				add_nomap    = false;
				buffer_shift = true;
			}

			if (strncmp(word,"-steam",6) == 0) {
				launch_client = false;
				launch_steam  = true;
				buffer_shift  = true;
			}

			if (strncmp(word,"-run=",5) == 0) {
				for (int i=0; i<global_exe_num; i++) {
					if (strcmpi(global_exe_name[i], word+5) == 0) {
						custom_exe = i;
						break;
					}
				}

				buffer_shift = true;
			}

			if (fi)
				fprintf(fi, "%s\"%s\"", (first_item ? "" : ","), word);

			first_item   = false;
			lpCmdLine[i] = prev;

			if (buffer_shift) {
				memcpy(lpCmdLine+begin, lpCmdLine+i, lpCmdLine_size-i+1);
				lpCmdLine_size -= (i - begin);
				i               = begin - 1;
				buffer_shift    = false;
			}

			begin = -1;
		}
	}

	if (fi) {
		fprintf(fi,"]]");
		fclose(fi);
	}



	// Read master server address from the file if an argument wasn't passed
	if (!client_arg.is_custom_master1) {
		String ip_list;
		int result = String_readfile(ip_list, "fwatch\\idb\\MasterServers.sqf");

		if (result == 0) {
			int open[]  = {'\"', '{'};
			int close[] = {'\"', '}'};

			for (int i=0; i<2; i++) {
				char *quote = strchr(ip_list.pointer, open[i]);

				if (quote != NULL) {
					quote++;
					char *quote_end = strchr(quote, close[i]);

					if (quote_end != NULL) {
						client_arg.is_custom_master1 = true;
						strncpy(global.master_server1, quote, quote_end-quote);
						break;
					}
				}
			}
		} else
			if (result == 2) {
				fi = fopen("fwatch\\idb\\MasterServers.sqf","w");
				if (fi) {
					strcpy(global.master_server1, "master.ofpisnotdead.com");
					client_arg.is_custom_master1 = true;
					fprintf(fi,"[\"master.ofpisnotdead.com\"]");
					fclose(fi);
				}
			}

		String_end(ip_list);
	}



	// Install file hook and make sure it works
    InstallHook();
	int user_choice = 0;

	do {
		bool result = false;
		
		// Test it twice
		for (int i=0; i<2; i++) {
			if (i > 0)
				Sleep(500);

			HANDLE filu = CreateFile("scripts\\:info version", GENERIC_READ, 0, NULL, OPEN_EXISTING, 128, NULL);

			if (filu != INVALID_HANDLE_VALUE) {
				CloseHandle(filu);
				result = true;
				break;
			}
		}
		
		if (result)
			user_choice = IDOK;
		else {
			user_choice = MessageBox(NULL, "Self test failed, Fwatch hooks do not appear to be working.\n", "fwatch error", MB_ABORTRETRYIGNORE|MB_ICONSTOP);

			if (user_choice == IDABORT)
				exit(0);
		}
	} while (user_choice == IDRETRY);



	// Run threads and the game
	if (listen_server)
		_beginthread((void(*)(void*))ListenServer, 0, &client_arg);

	if (launch_client) {
		PROCESS_INFORMATION pi;
		STARTUPINFO si; 
		memset(&si, 0, sizeof(si));
		si.cb = sizeof(si);

		DWORD dwExitCode = STILL_ACTIVE;
		bool launched    = false;

		String command_line;
		String_init(command_line);
		String_append(command_line, " ");
		String_append(command_line, (add_nomap ? "-nomap " : ""));
		String_append(command_line, lpCmdLine);

		for (int i=0; i<global_exe_num; i++) {
			if (custom_exe!=-1 && custom_exe!=i || strstr(global_exe_name[i],"_server.exe"))
				continue;

			if (CreateProcess(global_exe_name[i], command_line.pointer, NULL, NULL, false, 0, NULL,NULL,&si,&pi)) {
				launched = true;
				break;
			}
		}
		
		String_end(command_line);

		if (launched) {
			_beginthread((void(*)(void*))FwatchPresence, 0, &client_arg);

			while(dwExitCode == STILL_ACTIVE) {
				GetExitCodeProcess(pi.hProcess, &dwExitCode);
				Sleep(1000);
			}
		} else
			MessageBox(NULL, "Could not run the game!\n\nEnsure fwatch.exe is located in your game directory,\nor run fwatch with -nolaunch parameter and then run the game manually.", "fwatch error", MB_OK|MB_ICONSTOP);		
	} else {
		ThreadArguments server_arg     = client_arg;
		server_arg.is_dedicated_server = true;
		
		_beginthread((void(*)(void*))FwatchPresence, 0, &client_arg);
		_beginthread((void(*)(void*))FwatchPresence, 0, &server_arg);

		// Launch the game through Steam
		if (launch_steam) {
			HKEY hKey			= 0;
			char SteamExe[512]	= "";
			DWORD dwType		= REG_SZ;
			DWORD SteamExeSize	= sizeof(SteamExe);

			if (RegOpenKey(HKEY_CURRENT_USER,"Software\\Valve\\Steam",&hKey) == ERROR_SUCCESS) {
				if (RegQueryValueEx(hKey, "SteamExe" , 0, &dwType, (BYTE*)SteamExe , &SteamExeSize) == ERROR_SUCCESS) {
					String command_line;
					String_init(command_line);
					String_append(command_line, "\"\"");
					String_append(command_line, SteamExe);
					String_append(command_line, "\" -applaunch 65790 ");
					String_append(command_line, (add_nomap ? "-nomap " : ""));
					String_append(command_line, lpCmdLine);
					String_append(command_line, "\"");
					system(command_line.pointer);
					String_end(command_line);
				}

				RegCloseKey(hKey);
			}
		}

		// Just sit and wait here
		MessageBox( NULL, "Fwatch is now waiting for you to run the game and/or dedicated server.\nDon't forget to add -nomap\n\nWhen you're done playing press OK to close Fwatch.", "fwatch", MB_OK|MB_ICONINFORMATION );
	}




    RemoveHook();
	ModfolderMissionsReturn(GAME_CLIENT);
	ModfolderMissionsReturn(GAME_SERVER);
	unlink("fwatch_info.sqf");
	unlink("fwatch\\data\\pid.db");
	CloseHandle(mailslot);
    return 0;
}













// **** FUNCTIONS ***************************************************************************************

// Convert domain name to IP address
unsigned long GetIP(char *host) 
{
    struct hostent *hp;
    unsigned long host_ip = inet_addr(host);
    
    if (host_ip == htonl(INADDR_NONE)) {
        hp = gethostbyname(host);

        if (hp) 
            host_ip = *(unsigned long *)(hp->h_addr);
        else 
            return INADDR_NONE;
    }
    
    return host_ip;
}




// Split string by given characters; each call returns the next part; must supply current pos and length
char* Tokenize(char *string, char *delimiter, int &i, int string_length, bool square_brackets, bool reverse) 
{
	int delimiter_len = strlen(delimiter);
	bool in_brackets  = false;

	for (int begin=-1;  !reverse && i<=string_length || reverse && i>=-1;  reverse ? i-- : i++) {
		if (square_brackets && string[i] == '[')
			in_brackets = true;

		if (square_brackets && string[i] == ']')
			in_brackets = false;

		bool is_delim = false;

		// Check if current character is a delimiter
		for (int j=0; j<delimiter_len; j++)
			if (string[i] == delimiter[j] && !in_brackets  ||  delimiter[j]==' ' && isspace(string[i])) {
				is_delim = true;
				break;
			}

		// Mark beginning of the word
		if (begin<0  &&  (!is_delim || (!reverse && i==string_length || reverse && i==-1)))
			begin = i + (!reverse ? 0 : 1);

		// End the word
		if (begin>=0  &&  (is_delim ||  (!reverse && i==string_length || reverse && i==-1))) {
			string[(!reverse ? i : begin)] = '\0';
			
			if (!reverse && i<string_length)
				i++;
				
			return (!reverse ? string+begin : string+i+1);
		}
	}

	return "";
}




// Parse Fwatch modfolder configuration and fill arrays with new values
void ReadUIConfig(char *filename, bool *no_ar, bool *is_custom, float *custom, int *customINT)
{
	String config;
	if (String_readfile(config, filename) != 0)
		return;

	memset(no_ar    , 0, sizeof(no_ar));
	memset(is_custom, 0, sizeof(is_custom));
	memset(custom   , 0, sizeof(custom));
	memset(customINT, 0, sizeof(customINT));

	char *token		= "";
	int config_pos  = 0;
	int value_index = -1;
	
	while (config_pos < config.current_length) {
		char *setting = Tokenize(config.pointer, ";", config_pos, config.current_length, true, false);

		char *equality = strchr(setting, '=');
		if (equality == NULL)
			continue;

		setting      = Trim(setting);
		int pos      = equality - setting;
		setting[pos] = '\0';
		value_index  = -1;
		int	max_loops = sizeof(hud_names) / sizeof(hud_names[0]);

		for (int i=0; i<max_loops; i++) {
			if (strcmpi(setting,hud_names[i])==0) {
				value_index = i;
				break;
			}
		}
		
		if (value_index < 0)
			continue;

		setting        = setting + pos + 1;
		int values_len = strlen(setting);
		int values_pos = 0;

		while (values_pos < values_len) {
			char *value = Tokenize(setting, ",", values_pos, values_len, true, false);
			value = Trim(value);

			if (strcmpi(value, "noar")==0)
				no_ar[value_index] = 1;
			else {
				is_custom[value_index] = 1;

				if (IsNumberInArray(i,hud_int_list,sizeof(hud_int_list)/sizeof(hud_int_list[0]))) {
					if (IsNumberInArray(i,hud_color_list,sizeof(hud_color_list)/sizeof(hud_color_list[0]))) {
						int index              = 0;
						char *number           = strtok(value, "[,];");
						unsigned char color[4] = {0,0,0,0};

						while (number != NULL  &&  index<4) {
							color[index++] = (unsigned char)(atof(number) * 255);
							number         = strtok(NULL, "[,];");
						}

						customINT[value_index] = ((color[3] << 24) | (color[0] << 16) | (color[1] << 8) | color[2]);
					} else
						customINT[value_index] = atoi(value);
				} else
					custom[value_index] = (float)atof(value);
			}
		}
	}

	String_end(config);
}




// Move mission files from the selected modfolder to the game missions folder
int ModfolderMissionsTransfer(char *mod, bool is_dedicated_server)
{
	int file_count    = 0;
	char filename[64] = "fwatch\\data\\sortMissions";
	strcat(filename, is_dedicated_server ? "_server.txt" : ".txt");

	FILE *f = fopen(filename, "a");
	if (f) {
		char source[2048]           = "";
		char destination[2048]      = "";
		wchar_t source_w[1024]      = L"";
		wchar_t destination_w[1024] = L"";

		sprintf(source     , "%s\\Missions", mod);
		sprintf(destination, "Missions\\%s", mod);

		for (int i=10; destination[i]!='\0'; i++)
			if (destination[i]=='.')
				destination[i]='_';

		if (MoveFileEx(source, destination, 0)) {
			fprintf(f, "%s?%s\n", source, destination);
			file_count++;
		}

		WIN32_FIND_DATAW fd;
		HANDLE hFind   = INVALID_HANDLE_VALUE;
		size_t mod_len = strlen(mod);

		char folder[][16] = {
			"MPMissions",
			"Templates",
			"SPTemplates"
		};
		int folder_num       = sizeof(folder) / sizeof(folder[0]);
		wchar_t folder_w[32] = L"";

		for (i=0; i<folder_num; i++) {
			size_t folder_len = strlen(folder[i]);
			mbstowcs(folder_w, folder[i], folder_len+1);
			
			// Source path is: mod\MPMissions\*
			mbstowcs(source_w, mod, mod_len+1);
			wcscat(source_w  , L"\\");
			wcscat(source_w  , folder_w);
			wcscat(source_w  , L"\\*");

			// Destination path is: MPMissions\ 
			wcscpy(destination_w, folder_w);
			wcscat(destination_w, L"\\");

			hFind = FindFirstFileW(source_w, &fd);

			if (hFind != INVALID_HANDLE_VALUE) {
				do {
					if (wcscmp(fd.cFileName,L".")==0 || wcscmp(fd.cFileName,L"..")==0)
						continue;
					
					size_t filename_length = wcslen(fd.cFileName);
					unsigned long dir      = fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;

					if (filename_length<3 || filename_length>(1023-mod_len-13))
						continue;

					int dots     = 0;
					wchar_t *dot = fd.cFileName;

					while((dot=wcschr(dot,L'.')) != NULL && dots<2) {
						dots++;
						dot++;
					}

					if (dots>=1 && dir || !dir && dots>=2 && wcscmp(fd.cFileName+filename_length-4,L".pbo")==0) {
						wcscpy(source_w + mod_len + folder_len + 2, fd.cFileName);
						wcscpy(destination_w + folder_len + 1     , fd.cFileName);

						if (MoveFileExW(source_w, destination_w, 0)) {
							wcstombs(source, source_w, wcslen(source_w)+1);
							fprintf(f, "%s\n", source);
							file_count++;
						}
					}
				} 
				while (FindNextFileW(hFind, &fd) != 0);
				FindClose(hFind);
			}
		}

		fclose(f);
	}

	return file_count;
}




// Move mission files from the game missions folder back to the modfolders
void ModfolderMissionsReturn(bool is_dedicated_server) 
{
	char filename[64] = "fwatch\\data\\sortMissions";
	strcat(filename, is_dedicated_server ? "_server.txt" : ".txt");

	String mission_list;
	if (String_readfile(mission_list, filename) != 0)
		return;

	int i                       = 0;
	int word_start              = 0;
	wchar_t source_w[2048]      = L"";
	wchar_t destination_w[2048] = L"";
				
	while (i < mission_list.current_length) {
		char *destination = Tokenize(mission_list.pointer, "\r\n", i, mission_list.current_length, false, false);
		char *source      = strchr(destination,'\\');
		bool remove_line  = false;
		
		if (source != NULL) {
			source++;

			if (strncmpi(source,"missions?",9) == 0) {
				char *separator            = strchr(destination,'?');
				int separator_pos          = separator - destination;
				source                     = destination + separator_pos + 1;
				destination[separator_pos] = '\0';
				
				if (MoveFileEx(source, destination, 0))
					remove_line = true;

				destination[separator_pos] = '?';
			} else {
				size_t source_len      = strlen(source);
				size_t destination_len = strlen(destination);
				mbstowcs(source_w, source, source_len+1);
				mbstowcs(destination_w, destination, destination_len+1);

				if (MoveFileExW(source_w, destination_w, 0))
					remove_line = true;
			}
		} else
			remove_line = true;
		
		if (i > 0)
			mission_list.pointer[i-1] = '\r';
		
		// Cut text in the middle of the buffer and close the gap
		if (remove_line) {
			if (mission_list.pointer[i] == '\n') 
				i++;
				
			memcpy(mission_list.pointer+word_start, mission_list.pointer+i, mission_list.current_length-i);
			
			int len                      = i - word_start;
			i                           -= len;
			mission_list.current_length -= len;
		}

		word_start = i;
	}

	mission_list.pointer[mission_list.current_length] = '\0';
	
	FILE *f = fopen(filename, "wb");
	if (f) {
		fwrite(mission_list.pointer, 1, mission_list.current_length, f);
		fclose(f);
	}

	String_end(mission_list);
}
// ******************************************************************************************************



















// **** THREADS *****************************************************************************************

// While the game is running access its memory, move mission files, handle signals from dll
void FwatchPresence(ThreadArguments *arg)
{
	HANDLE phandle;
	SIZE_T stBytes      = 0;
	DWORD game_pid      = 0;
	DWORD game_pid_last = 0;

	int game_exe_address            = 0;
	int game_param_address          = 0;
	int game_exe_index              = 0;
	int game_mods_num               = 0;
	const int game_mods_max         = 32;
	const int game_param_max        = 512;
	char game_param[game_param_max] = "";
	char *game_mods[game_mods_max]  = {0};

	int master_server1_address = 0;
	int master_server2_address = 0;
	int transfered_missions    = 0;
	int signal_action          = -1;
	char *signal_buffer        = "";
	char error_msg_last[512]   = "";

	bool ui_change_enabled     = false;
	bool ui_portrait_mode      = false;
	bool ui_apply_change       = true;
	bool ui_portrait_calculate = false;
	bool ui_move_tank_down     = false;
	int ui_address             = 0;
	int ui_address_chat        = 0;
	int ui_chat_state          = 0;
	int ui_chat_state_last     = -1;
	float ui_shift_x           = 0;
	float ui_shift_y           = 0;
	float ui_move_tank_down_y  = 0;
	float ui_current[ARRAY_SIZE];
	float ui_written[ARRAY_SIZE];
	float ui_custom[ARRAY_SIZE];
	bool ui_no_ar[ARRAY_SIZE];
	bool ui_is_custom[ARRAY_SIZE];
	int ui_currentINT[ARRAY_SIZE];
	int ui_writtenINT[ARRAY_SIZE];
	int ui_customINT[ARRAY_SIZE];

	enum UI_CHAT_STATES {
		UI_CHAT_MISSION,
		UI_CHAT_BRIEFINGLOBBY,
		UI_CHAT_CUSTOM
	};



	// Prepare necessary data for fixing UI on game client - 
	// Generate settings file, preprocess it and then parse it
	if (!arg->is_dedicated_server) {
		FILE *f = fopen("Aspect_Ratio.sqf","w");
		if (f) {
			fprintf(f, "#include \"Aspect_Ratio.hpp\"\n#ifdef AR_CENTERHUD\nar_center=1;\n#else\nar_center=0;\n#endif\nar_modifx=AR_modifX;ar_modify=AR_modifY;ar_modifx_2ndmon=AR_modifX_2NDMON;true");
			fclose(f);

			TCHAR pwd[MAX_PATH];
			GetCurrentDirectory(MAX_PATH, pwd);

			PROCESS_INFORMATION pi;
			STARTUPINFO si; 
			memset(&si, 0, sizeof(si));
			memset(&pi, 0, sizeof(pi));
			si.cb          = sizeof(si);
			si.dwFlags     = STARTF_USESHOWWINDOW;
			si.wShowWindow = SW_HIDE;

			char cmdLine[MAX_PATH+64] = "";
			sprintf(cmdLine, "\"%s\" -nolog Aspect_Ratio.sqf Aspect_Ratio.sqf ", pwd);

			if (CreateProcess("fwatch\\data\\preproc.exe", cmdLine, NULL, NULL, false, 0, NULL, NULL, &si, &pi)) {
				DWORD st;

				do {					
					GetExitCodeProcess(pi.hProcess, &st);
					Sleep(5);
				} while (st == STILL_ACTIVE);

				CloseHandle(pi.hProcess);
				CloseHandle(pi.hThread); 


				String config;
				if (String_readfile(config, "Aspect_Ratio.sqf") == 0) {
					char *token = strtok(config.pointer, ";\n\t ");

					while (token != NULL) {
						char *eq = strchr(token, '=');

						if (eq != NULL) {
							int pos   = eq - token;
							char *val = token + pos + 1;

							if (strncmp(token,"ar_modifx",pos) == 0) {
								ui_shift_x       = (float)atof(val);
								ui_portrait_mode = ui_shift_x < 0;
							}

							if (strncmp(token,"ar_modify",pos) == 0) 
								ui_shift_y = (float)atof(val);

							if (strncmp(token,"ar_center",pos) == 0)
								ui_change_enabled = atoi(val) == 0;
						}

						token = strtok (NULL, ";\n\t ");
					}
				}

				String_end(config);
			}
		}
	}




	while (true) {
		Sleep(250);

		// Search for the game window
		HWND hwnd = NULL;
		hwnd      = GetTopWindow(hwnd);
		game_pid  = 0;

		while (hwnd && game_pid==0) {
			char current_window_name[32] = "";
			GetWindowText(hwnd, current_window_name, 32);

			for (int i=0; i<global_window_num && game_pid==0; i++) {
				if (strncmp(current_window_name, global_window_name[i], strlen(global_window_name[i])) == 0) {
					DWORD pid = 0;
					GetWindowThreadProcessId(hwnd, &pid);
					HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
 
					if (hSnap != INVALID_HANDLE_VALUE) {
						MODULEENTRY32 xModule;
						memset (&xModule, 0, sizeof(xModule));
						xModule.dwSize = sizeof(xModule);

						// First process module must match game executable name
						if (Module32First(hSnap, &xModule)) {							
							for (int i=0; i<global_exe_num && game_pid==0; i++)
								if ((!arg->is_dedicated_server && !strstr(global_exe_name[i],"_server.exe") || arg->is_dedicated_server && strstr(global_exe_name[i],"_server.exe")) && lstrcmpi(xModule.szModule, (LPCTSTR)global_exe_name[i]) == 0) {
									game_pid = pid;

									// If this a new game instance then also get address for exe parameters
									if (game_pid != game_pid_last) {
										game_exe_address     = (int)xModule.modBaseAddr;
										game_exe_index       = i;
										game_param_address   = 0;
										char module_name[16] = "ifc22.dll";

										if (arg->is_dedicated_server)
											strcpy(module_name, "ijl15.dll");

										do {
											if (lstrcmpi(xModule.szModule, (LPCTSTR)module_name) == 0) {	
												game_param_address = (int)xModule.modBaseAddr + (!arg->is_dedicated_server ? 0x2C154 : 0x4FF20);
												break;
											}
										} while(Module32Next(hSnap, &xModule));
									}
								}
						}

						CloseHandle(hSnap);
					}
				}
			}

			if (game_pid == 0)
				hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
		}

		// If there's no game
		if (game_pid==0 || game_param_address==0) {
			if (transfered_missions > 0) {
				transfered_missions = 0;
				ModfolderMissionsReturn(arg->is_dedicated_server);
			}

			continue;
		}

		phandle = OpenProcess(PROCESS_ALL_ACCESS, 0, game_pid);

		if (phandle == 0)
			continue;

		// If this is a new game instance
		if (game_pid != game_pid_last) {
			if (!arg->is_dedicated_server) {
				// Reset UI state
				memset(ui_current    , 0, sizeof(ui_current));
				memset(ui_currentINT , 0, sizeof(ui_currentINT));
				memset(ui_written    , 0, sizeof(ui_written));
				memset(ui_writtenINT , 0, sizeof(ui_writtenINT));
				memset(ui_custom     , 0, sizeof(ui_custom));
				memset(ui_customINT  , 0, sizeof(ui_customINT));
				memset(ui_no_ar      , 0, sizeof(ui_no_ar));
				memset(ui_is_custom  , 0, sizeof(ui_is_custom));

				ReadUIConfig((global_exe_version[game_exe_index]==VER_196 ? "Res\\bin\\config_fwatch_hud.cfg" : "bin\\config_fwatch_hud.cfg"), ui_no_ar, ui_is_custom, ui_custom, ui_customINT);

				// Find where UI coordinates are stored
				ui_address      = 0;
				ui_address_chat = 0;

				switch(global_exe_version[game_exe_index]) {
					case VER_196 : ui_address=0x79F8D0; break;
					case VER_199 : ui_address=0x78E9C8; break;
					case VER_201 : ui_address=game_exe_address+0x6D8240; break;
				}

				ReadProcessMemory(phandle, (LPVOID)(ui_address+0x0), &ui_address, 4, &stBytes);
				ReadProcessMemory(phandle, (LPVOID)(ui_address+0x8), &ui_address, 4, &stBytes);

				if (ui_address != 0) {
					// Chat has a different address than the rest of UI
					switch(global_exe_version[game_exe_index]) {
						case VER_196 : ui_address_chat=0x7831B0; break;
						case VER_199 : ui_address_chat=0x7722A0; break;
						case VER_201 : ui_address_chat=game_exe_address+0x6FFCC0; break;
					}
				} else {
					// Wait until game fills that memory
					CloseHandle(phandle);
					continue;
				}

				if (ui_portrait_mode)
					ui_portrait_calculate = true;
			}


			// Get list of arguments passed to the game executable
			ReadProcessMemory(phandle, (LPVOID)game_param_address, &game_param_address, 4, &stBytes);
			ReadProcessMemory(phandle, (LPVOID)game_param_address, &game_param_address, 4, &stBytes);
			ReadProcessMemory(phandle, (LPVOID)game_param_address, &game_param        , game_param_max, &stBytes);

			int word_start = -1;
			game_mods_num  = 0;

			// Copy mod names into array
			for (int i=0; i<game_param_max; i++) {
				if (game_param[i] == '\0') {
					if (strncmp(game_param+word_start,"-mod=",5) == 0) {
						char *value   = game_param + word_start + 5;
						int value_len = strlen(value);
						int value_pos = 0;

						while (value_pos < value_len  &&  game_mods_num < game_mods_max)
							game_mods[game_mods_num++] = Tokenize(value, ";", value_pos, value_len, false, false);
					}

					if (game_param[i+1] == '\0')
						break;

					word_start = -1;
				} else
					if (word_start == -1)
						word_start = i;
			}

			// Read fwatch configuration from each mod
			if (!arg->is_dedicated_server)
				for (i=0; i<game_mods_num; i++)
					if (strlen(game_mods[i]) < 485) {
						char filename[512] = "";
						sprintf(filename, "%s\\bin\\config_fwatch_hud.cfg", game_mods[i]);
						ReadUIConfig(filename, ui_no_ar, ui_is_custom, ui_custom, ui_customINT);
					}


			// Change master servers
			switch(global_exe_version[game_exe_index]) {
				case VER_196 : master_server1_address=0x76EBC0; master_server2_address=0x775F58; break;
				case VER_199 : master_server1_address=0x756530; master_server2_address=0x75D7F0; break;
				case VER_201 : master_server1_address=game_exe_address+0x6C9298; master_server2_address=game_exe_address+0x6C9560; break;
			}

			if (!arg->is_dedicated_server) {
				if (master_server1_address!=0  &&  arg->is_custom_master1)
					WriteProcessMemory(phandle, (LPVOID)master_server1_address, global.master_server1, 64, &stBytes);

				if (master_server2_address!=0  &&  arg->is_custom_master2)
					WriteProcessMemory(phandle, (LPVOID)master_server2_address, global.master_server2, 19, &stBytes);
			}


			// Find listen server port
			int listen_server_port_address = 0;

			switch(global_exe_version[game_exe_index]) {
				case VER_196 : listen_server_port_address=0x778794; break;
				case VER_199 : listen_server_port_address=0x75F960; break;
				case VER_201 : listen_server_port_address=game_exe_address+0x6C9610; break;
			}

			if (listen_server_port_address != 0)
				ReadProcessMemory(phandle, (LPVOID)listen_server_port_address, &global.listen_server_port, 4, &stBytes);


			// Find 'Cheat 1' string and replace it with 'FWATCH'
			bool found     = false;
			int pointer[4] = {0,0,0,0};
			int modif[3]   = {0x434, 0x9C, 0x8};
			int max_loops  = sizeof(pointer) / sizeof(pointer[0]) - 1;
			char buffer[8] = "";

			for (int j=0; j<14 && game_pid_last==0; j++) {
				if (!arg->is_dedicated_server) {
					switch(global_exe_version[game_exe_index]) {
						case VER_196 : pointer[0]=0x7D5280; break;
						case VER_199 : pointer[0]=0x7C4248; break;
						case VER_201 : pointer[0]=game_exe_address+0x6C9A60; break;
					}
				} else {
					switch(global_exe_version[game_exe_index]) {
						case VER_196 : pointer[0]=0x754D78; break;
						case VER_199 : pointer[0]=0x754E08; break;
						case VER_201 : pointer[0]=game_exe_address+0x5C08A0; break;
					}
				}
						
				// prefedefined memory locations
				switch (j) {
					case 0 : modif[0]=0x434; modif[1]=0x9C; break;
					case 1 : modif[0]=0x434; modif[1]=0x94; break;
					case 2 : modif[0]=0x3B4; modif[1]=0x44; break;
					case 3 : modif[0]=0x434; modif[1]=0xAC; break;
					case 4 : modif[0]=0x3B4; modif[1]=0x3C; break;
					case 5 : modif[0]=0x1C4; modif[1]=0x70; modif[2]=0x68; break;
					case 6 : modif[0]=0x374; modif[1]=0x46C; modif[2]=0x8; break;
					case 7 : modif[0]=0x7F4; modif[1]=0x46C; modif[2]=0x8; break;
					case 8 : modif[0]=0x6F4; modif[1]=0x364; modif[2]=0x8; break;
					case 9 : modif[0]=0x1C4; modif[1]=0x68; modif[2]=0x68; break;
					case 10: modif[0]=0x3B4; modif[1]=0x2C; modif[2]=0x8; break; //armaresistance.exe
					case 11: modif[0]=0x3B4; modif[1]=0x34; modif[2]=0x8; break; //armaresistance.exe
					case 12: pointer[0]=0x7B4A54; modif[0]=0x74; modif[1]=0x1C4; modif[2]=0x8; break;
					case 13: pointer[0]=0x7D52A8; modif[1]=0x7C4; modif[2]=0x8; max_loops--; break;
				}

				for (int i=0; i<max_loops; i++) {
					ReadProcessMemory(phandle, (LPVOID)pointer[i], &pointer[i+1], 4, &stBytes);
					pointer[i+1] = pointer[i+1] +  modif[i];
				}

				ReadProcessMemory(phandle, (LPVOID)pointer[max_loops], &buffer, 8, &stBytes);

				if (strcmpi(buffer,"Cheat 1") == 0) {
					WriteProcessMemory(phandle, (LPVOID)pointer[max_loops], "FWATCH", 7, &stBytes);
					found = true;
					break;
				}
			}

			// If not found then do a broad search
			if (!found) {
				for (int current=!arg->is_dedicated_server ? 0x10000008 : 0x00D00008; current<0x7FFF0000; current+=0x10) {
					ReadProcessMemory(phandle, (LPVOID)current, &buffer, 7, &stBytes);

					if (strcmp(buffer,"Cheat 1")==0) {
						WriteProcessMemory(phandle, (LPVOID)current, "FWATCH", 7, &stBytes);
						break;
					}
				}
			}

			ui_apply_change = true;
			game_pid_last   = game_pid;
		} else {
			// If it's a game instance that we have accessed before
			// Transfer mission files from each mod
			if (transfered_missions==0 && game_mods_num>0)			
				for (int i=game_mods_num-1; i>=0; i--)
					transfered_missions += ModfolderMissionsTransfer(game_mods[i], arg->is_dedicated_server);

			// Refresh master servers (user can change them in the main menu)
			ReadProcessMemory(phandle, (LPVOID)master_server1_address, &global.master_server1, 64, &stBytes);
			ReadProcessMemory(phandle, (LPVOID)master_server2_address, &global.master_server2, 19, &stBytes);

			// Check for game scripting error
			if (global.error_log_enabled) {
				int pointers[] = {0, 0, 0, 0};
				int modif[]	   = {0x68, 0x1C, 0};
				int last       = sizeof(pointers) / sizeof(pointers[0]) - 1;

				switch(global_exe_version[game_exe_index]) {
					case VER_196 : pointers[0]=0x789D88; break;
					case VER_199 : pointers[0]=0x778E80; break;
					case VER_201 : pointers[0]=game_exe_address+0x6D6A10; break;
				}

				for (int i=0; i<last; i++) {
					ReadProcessMemory(phandle, (LPVOID)pointers[i], &pointers[i+1], 4, &stBytes);
					pointers[i+1] = pointers[i+1] +  modif[i];
				}

				// Save message if it's different from the last one
				if (pointers[last] != 0) {
					char error_msg[512] = "";
					ReadProcessMemory(phandle, (LPVOID)pointers[last], &error_msg, 512, &stBytes);

					if (strcmp(error_msg,error_msg_last) != 0) {						
						strcpy(error_msg_last, error_msg);

						if (!global.error_log_started) {
							global.error_log_started = true;
							HANDLE message = CreateFile("scripts\\:info errorlog start", GENERIC_READ, 0, NULL, OPEN_EXISTING, 128, NULL);

							if (message != INVALID_HANDLE_VALUE)
								CloseHandle(message);
						}

						FILE *f = fopen("fwatch\\idb\\_errorLog.txt", "a");

						if (f) {
							fprintf(f, "%s\n", error_msg);
							fclose(f);
						}
					}
				}
			}

			if (ui_change_enabled) {
				// Read position of UI elements
				for (int i=0; i<hud_offset_num; i++) {			
					int address = i<CHAT_X ? ui_address : ui_address_chat;

					if (IsNumberInArray(i,hud_int_list,sizeof(hud_int_list)/sizeof(hud_int_list[0])))
						ReadProcessMemory(phandle, (LPVOID)(address+hud_offset[i]), &ui_currentINT[i], 4, &stBytes);
					else
						ReadProcessMemory(phandle, (LPVOID)(address+hud_offset[i]), &ui_current[i], 4, &stBytes);
				}

				if (ui_currentINT[CHAT_ROWS]==4  &&  ui_current[CHAT_X]==0.05f  &&  (ui_current[CHAT_Y]==0.02f || ui_current[CHAT_Y]==0.90f)  &&  ui_current[CHAT_W]==0.9f  &&  ui_current[CHAT_H]==0.02f)
					ui_chat_state = UI_CHAT_BRIEFINGLOBBY;
				else 
					if (ui_currentINT[CHAT_ROWS]==6  &&  ui_current[CHAT_X]==0.05f  &&  ui_current[CHAT_Y]==0.68f  &&  ui_current[CHAT_W]==0.7f  &&  ui_current[CHAT_H]==0.02f)
						ui_chat_state = UI_CHAT_MISSION;
					else
						ui_chat_state = UI_CHAT_CUSTOM;


				// Run calculation for the portrait mode
				if (ui_portrait_calculate) {
					ui_portrait_calculate = false;

					float tank_x    = ui_current[TANK_X]    + (ui_current[TANK_X]<0.5 ? -1 : 1)    * ui_shift_x;
					float tank_y    = ui_current[TANK_Y]    + (ui_current[TANK_Y]<0.5 ? -1 : 1)    * ui_shift_y;
					float radar_y	= ui_current[RADAR_Y]   + (ui_current[RADAR_Y]<0.5 ? -1 : 1)   * ui_shift_y;
					float compass_y = ui_current[COMPASS_Y] + (ui_current[COMPASS_Y]<0.5 ? -1 : 1) * ui_shift_y;	

					if (ui_portrait_mode && 
						tank_x + ui_current[TANK_W] > ui_current[RADAR_X]  &&
						tank_y + ui_current[TANK_H] >= radar_y) 
					{
						ui_move_tank_down   = true;
						ui_move_tank_down_y = (radar_y + ui_shift_y) + ui_current[RADAR_H];
					} else
						if (ui_portrait_mode &&
							tank_x + ui_current[TANK_W] > ui_current[COMPASS_X]  &&
							tank_y + ui_current[TANK_H] >= compass_y) 
						{
							ui_move_tank_down   = true;
							ui_move_tank_down_y = (compass_y + ui_shift_y) + ui_current[COMPASS_H];
						}
				}


				// Modify HUD position
				for (i=0; i<hud_offset_num; i++) {
					int is_int = IsNumberInArray(i,hud_int_list,sizeof(hud_int_list)/sizeof(hud_int_list[0]));

					// Write new position once per game instance
					// Except for the chat because its position keeps changing throughout the game
					//if (first_check  ||  (!is_int && current[i]!=written[i])  ||  (is_int  && currentINT[i]!=writtenINT[i]))
					if (ui_apply_change  ||  i>=CHAT_X && ui_chat_state!=ui_chat_state_last) {
						if (ui_is_custom[i] && (i<CHAT_X || i>=CHAT_X && ui_chat_state==UI_CHAT_MISSION))
							if (is_int)
								ui_currentINT[i] = ui_customINT[i];
							else
								ui_current[i] = ui_custom[i];
						
						if (ui_no_ar[i]) {
							if (is_int)
								ui_writtenINT[i] = ui_currentINT[i];
							else
								ui_written[i] = ui_current[i];
						} else
							switch (i) {
								// move to the left
								case ACTION_X	 :
								case TANK_X		 :	
								case GROUPDIR_X  : ui_written[i] = ui_current[i] + (ui_current[i]<0.5 ? -1 : 1) * ui_shift_x; break;

								// move up (but compensate for portrait)
								case RADIOMENU_Y :
								case TANK_Y		 : ui_written[i] = ui_current[i] + (ui_current[i]<0.5 ? -1 : 1) * ui_shift_y + ui_move_tank_down_y; break;

								// move up/down
								case ACTION_Y	 :
								case LEADER_Y	 :
								case GROUPDIR_Y  :
								case RADAR_Y	 :
								case HINT_Y		 : 
								case COMPASS_Y	 : ui_written[i] = ui_current[i] + (ui_current[i]<0.5 ? -1 : 1) * ui_shift_y; break;

								// move left on portrait
								case RADIOMENU_X : {
									if (ui_move_tank_down)
										ui_written[i] = ui_current[i] + (ui_current[i]<0.5 ? -1 : 1) * ui_shift_x;
									else
										ui_written[i] = ui_current[i];
								}
								break;

								// extend on widescreen
								case RADIOMENU_W : ui_written[i] = ui_current[i] + (!ui_move_tank_down ? ui_shift_x : 0); break; 

								// shrink on portrait
								case LEADER_X : ui_written[i] = ui_current[i] - ui_shift_x; break;
								case LEADER_W : ui_written[i] = ui_current[i] + ui_shift_x*2; break;
								/*{
									if (portrait)
									{
										if (i == LEADER_X)
											written[i] = current[i] + (current[i]<0.5 ? -1 : 1) * AR_modifX;
										else
											written[i] = current[i] + AR_modifX*2;
									}
									else
										written[i] = current[i];
								}
								break;*/

								case CHAT_X : ui_written[i] = ui_current[i] - ui_shift_x; break;
								case CHAT_Y : ui_written[i] = ui_current[i] + (ui_current[i]<0.5 ? -ui_shift_y : ui_shift_y); break;
								case CHAT_W : {
									if (ui_portrait_mode) {
										if (ui_current[CHAT_X] + ui_current[i] > 1 + ui_shift_x*2)
											ui_written[i] = ui_current[i] + ui_shift_x*2;
									} else
										ui_written[CHAT_W] = ui_current[CHAT_W] + ui_shift_x*2;
								}
								break;

								default: {
									if (is_int)
										ui_writtenINT[i] = ui_currentINT[i];
									else
										ui_written[i] = ui_current[i];								
								}
								break;
									
							}

						int address = i<CHAT_X ? ui_address : ui_address_chat;
						
						if (is_int) {
							//fprintf(fd,"%s: 0x%x %d %d\n",hud_names[i],pointer+hud_offset[i],currentINT[i],writtenINT[i]);
							WriteProcessMemory(phandle, (LPVOID)(address+hud_offset[i]), &ui_writtenINT[i], 4, &stBytes);
						} else {
							//fprintf(fd,"%s: 0x%x %f %f\n",hud_names[i],pointer+hud_offset[i],current[i],written[i]);
							WriteProcessMemory(phandle, (LPVOID)(address+hud_offset[i]), &ui_written[i], 4, &stBytes);
						}
					}
				}

				ui_apply_change    = false;
				ui_chat_state_last = UI_CHAT_CUSTOM;
			}

			// Check if there's a signal to restart the client/server
			char file_name[64] = "fwatch\\data\\fwatch_client_restart.db";

			if (arg->is_dedicated_server)
				strcpy(file_name, "fwatch\\data\\fwatch_server_restart.db");

			if (signal_action == -1) {
				FILE *fp = fopen(file_name, "r");					 
				if (fp) {
					fseek(fp, 0, SEEK_END);
					int buffer_size = ftell(fp);
					signal_buffer   = (char*) malloc (buffer_size+1);

					if (signal_buffer) {
						fseek(fp, 0, SEEK_SET);
						fread(signal_buffer, 1, buffer_size, fp);
						signal_buffer[buffer_size] = '\0';
						signal_action = 1;
					}
				
					fclose(fp);
				}
			} 

			if (signal_action == 1) {
				bool file_removed = false;

				if (remove(file_name) == 0) {
					file_removed = true;
				} else {
					if (errno == 2) {
						file_removed = true;
					}
				}

				if (file_removed) {
					char exe_name[64] = "fwatch\\data\\gameRestart.exe";

					if (arg->is_dedicated_server) {
						strcpy(exe_name, global_exe_name[game_exe_index]);
						TerminateProcess(phandle, 0);
						CloseHandle(phandle);
					}

					// Run program
					STARTUPINFO si;
					PROCESS_INFORMATION pi;
					ZeroMemory(&si, sizeof(si));
					si.cb = sizeof(si);
					si.dwFlags = STARTF_USESHOWWINDOW;
					si.wShowWindow = SW_HIDE;
					ZeroMemory(&pi, sizeof(pi));

					if (CreateProcess(exe_name, signal_buffer, NULL, NULL, TRUE, HIGH_PRIORITY_CLASS, NULL, NULL, &si, &pi)) {
						CloseHandle(pi.hProcess);
						CloseHandle(pi.hThread); 
					}

					signal_action = -1;
					free(signal_buffer);
				}
			}


			// Check messages from fwatch.dll
			if (*arg->mailslot != INVALID_HANDLE_VALUE) {
				FILE *fd=fopen("fwatch_debug.txt","a");
				DWORD message_size   = 0;
				DWORD message_number = 0;

				if (GetMailslotInfo(*arg->mailslot, (LPDWORD)NULL, &message_size, &message_number, (LPDWORD)NULL)) {
					if (message_size != MAILSLOT_NO_MESSAGE) {
						_beginthread((void(*)(void*))WatchProgram, 0, arg);
					} else 
						fprintf(fd,"fwatch.exe - no messages\n");
				} else {
					fprintf(fd,"fwatch.exe - failed to get mailslot info %d\n",GetLastError());
				}
				fclose(fd);
			} else {
				FILE *fd=fopen("fwatch_debug.txt","a");
				fprintf(fd,"fwatch.exe - invalid handle value\n");
				fclose(fd);
			}
		}

		CloseHandle(phandle);
	}

	_endthread();
}








// Report listen server to the master server
// Adapted from code by Pulverizer http://pulverizer.pp.fi/ewe/c/OFPReportListenServer.c
void ListenServer(ThreadArguments *arg)
{	
	WSADATA wsaData;
	SOCKET reportSocket;
	SOCKADDR_IN localAddr;
	SOCKADDR_IN remoteAddr;
	SOCKADDR_IN tempAddr;

	bool bQuit = false,
		 bAskConfirm = true,
		 bSettingsChanged = false;

	int i = 0,
		remotePort = -1,
		localPort = 2303,
		returnBytes = 0,
		actualVersion = 0,
		requiredVersion = 195,
		maxplayers = 0,
		localAddrSize = sizeof(localAddr),
		remoteAddrSize = sizeof(remoteAddr),
		tempAddrSize = sizeof(tempAddr);

    short int password = 0,
			  equalMod = 0;

	unsigned int queryid = 1;
	unsigned long int uli,
					  ticks,
					  lastQuery = 0,
					  lastHeartbeat = 0;

	char c, 
		 SendBuf[4096], 
		 ListenBuf[4096], 
		 TempBuf[4096],
         TempBuf2[4096], 
		 SettingsBuf[4096],
		 set_hostname[256] = "Listen Server\0",
		 set_missionname[256] = "\0",
		 set_mod[256] = "RES\0",
		 set_players = 0,
		 set_gamestate = (char) 255, // select mis 2, wait plr 6, brief 13, play 14
		 LANQueryPacket[32] = { (char) 32, 0, (char) 1, (char) 8, // 0x0020 length, 01, 08
                                (char) 27, (char) 201, (char) 194, (char) 233, (char) 1, 0, 0, 0, // tickcount? (needed)
                                (char) 1, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0,
                                (char) 174, (char) 145, (char) 225, (char) 238,
                                0, 0, 0, 0 },
		 cmpWeirdPing[11] = {(char) 254, (char) 253,0,0,0,0,0,(char) 255,0,0,0};
	

	// Wait for the other thread to read port from the memory
	while (global.listen_server_port < 0)
		Sleep(1000);

	initsocket:
	localPort = global.listen_server_port + 1;

	// INIT SOCKET
	if (WSAStartup(MAKEWORD(2,2),&wsaData) != 0)
		_endthread();

	reportSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	uli = 1; 
	ioctlsocket(reportSocket, FIONBIO, &uli); // enable non-blocking

	localAddr.sin_family = AF_INET;
	localAddr.sin_port = htons(localPort);
	localAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(reportSocket, (SOCKADDR *) &localAddr, sizeof(localAddr)) != 0)
	{
		closesocket(reportSocket);
		WSACleanup();
		_endthread();
	};



	// **** MAIN LOOP ********************************
	while (true)
	{	
		// check if port changed
		if (localPort != global.listen_server_port + 1) {
			closesocket(reportSocket);
			WSACleanup();
			goto initsocket;
		}
		
		ticks = GetTickCount();

		// LANQUERY gstate etc from the game
		if ((ticks - lastQuery) > 10000)
		{
			for (c=0; c==0; )
			{
				remoteAddr.sin_family = AF_INET;
				remoteAddr.sin_port = htons((localPort-1));
				remoteAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

				if (sendto(reportSocket, LANQueryPacket, 32, 0, (SOCKADDR *) &remoteAddr, sizeof(remoteAddr))) 
					Sleep(100);
				else
				{
					closesocket(reportSocket);
					WSACleanup();
					_endthread();
				};

				returnBytes = recvfrom(reportSocket, ListenBuf, 434, 0, (SOCKADDR *)&tempAddr, &tempAddrSize);
				if (returnBytes == SOCKET_ERROR)
				{
					i = WSAGetLastError();
					if (i==10054) // reset by peer
						Sleep(1000);
					else if (i==10035) // would block
						Sleep(1000);
					else
					{
						closesocket(reportSocket);
						WSACleanup();
						_endthread();
					};
				}
				else
				{
					ListenBuf[returnBytes] = '\0';

					// Skip bytes until MAGIC_ENUM_RESPONSE					
					int I = 0,
						j = 0, 
						magic[4] = {0xFFFFFFAC, 0xFFFFFFE8, 0xFFFFFFF1, 0xFFFFFFFF};

					for (; I<returnBytes && j<4; I++)
						if (ListenBuf[I] == magic[j])
							j++;
						else
							j=0;

					if (j>=4 && I+406<=returnBytes)
					{
						// Server name
						strncpy(set_hostname, ListenBuf+I, 256); 
						I += 256;

						// Actual version
						for (actualVersion=0, j=0; j<4; I++, j++)
							actualVersion += ListenBuf[I] & 0xFF;

						// Required version
						for (requiredVersion=0, j=0; j<4; I++, j++)
							requiredVersion += ListenBuf[I] & 0xFF;

						// Mission name
						strncpy(set_missionname, ListenBuf+I, 40); 
						I += 40;

						// Game state
						for (set_gamestate=0, j=0; j<4; I++, j++)
							set_gamestate += ListenBuf[I] & 0xFF;

						// Max players
						for (maxplayers=0, j=0; j<4; I++, j++)
							maxplayers += ListenBuf[I] & 0xFF;

						// Password
						for (password=0, j=0; j<2; I++, j++)
							password += ListenBuf[I] & 0xFF;

						// Port - skip
						I += 2;

						// Players
						for (set_players=1, j=0; j<4; I++, j++)
							set_players += ListenBuf[I] & 0xFF;

						// Serial number of enumeration request - skip
						I += 4;

						// Mods
						strncpy(set_mod, ListenBuf+I, 80);
						I += 80;

						// Equal mod required
						for (equalMod=0, j=0; j<2; I++, j++)
							equalMod += ListenBuf[I] & 0xFF;       

						if (!bAskConfirm) 
							bSettingsChanged = true;
					};

					c++;
				};
			}; // c==0

			sprintf(SettingsBuf, "\\hostname\\%s" // set_hostname
								 "\\hostport\\%i" // localPort
								 "\\mapname\\"
								 "\\gametype\\%s" // set_missionname
								 "\\numplayers\\%i" // set_players
								 "\\maxplayers\\%i"
								 "\\gamemode\\openplaying"
								 "\\timeleft\\0"
								 "\\param1\\0"
								 "\\param2\\0"
								 "\\actver\\%i"
								 "\\reqver\\%i"
								 "\\mod\\%s"  // set_mod
								 "\\equalModRequired\\%i"
								 "\\password\\%i"
								 "\\gstate\\%i" // set_gamestate
								 "\\impl\\sockets"
								 "\\platform\\win",
								 set_hostname,
								 (localPort-1),
								 set_missionname,
								 set_players,
								 maxplayers,
								 actualVersion,
								 requiredVersion,
								 set_mod,
								 equalMod,
								 password,
								 set_gamestate);

			ticks = GetTickCount();
			lastQuery = ticks;
		}; //lanquery end

		
		if (bAskConfirm)
			ticks = 1000000,
			bAskConfirm = false;


        // REPORTING aka Heartbeat
        if( (bSettingsChanged || (ticks - lastHeartbeat)>300000))
        {		
			// By default don't allow to report to the defunct GameSpy unless set by user
			if (arg->is_custom_master1 || strcmpi(global.master_server1,"master.gamespy.com")!=0)
			{	
				remoteAddr.sin_family = AF_INET; // gamespy
				remoteAddr.sin_port = htons(27900);
				remoteAddr.sin_addr.s_addr = GetIP(global.master_server1);

				if(bSettingsChanged)
					sprintf(TempBuf, "\\heartbeat\\%d\\gamename\\opflashr\\statechanged\\1\0", localPort);
				else
					sprintf(TempBuf, "\\heartbeat\\%d\\gamename\\opflashr\0", localPort);

				strcpy(SendBuf, TempBuf);
				if (!sendto(reportSocket, SendBuf, strlen(SendBuf), 0, (SOCKADDR *) &remoteAddr, sizeof(remoteAddr)))
				{
					closesocket(reportSocket);
					WSACleanup();
					_endthread();
				}
			};

			// By default don't allow to report to the defunct All-Seeing Eye unless set by user
			if (arg->is_custom_master2 || strcmpi(global.master_server2,"master.udpsoft.com")!=0)
			{
				remoteAddr.sin_family = AF_INET; // all seeing eye
				remoteAddr.sin_port = htons(27900);
				remoteAddr.sin_addr.s_addr = GetIP(global.master_server2);

				if (bSettingsChanged)
					sprintf(TempBuf, "\\heartbeat\\%d\\gamename\\opflashr\\statechanged\\1\0", localPort);
				else
					sprintf(TempBuf, "\\heartbeat\\%d\\gamename\\opflashr\\\0", localPort);

				if (!sendto(reportSocket, SendBuf, strlen(SendBuf), 0, (SOCKADDR *) &remoteAddr, sizeof(remoteAddr)))
				{
					closesocket(reportSocket);
					WSACleanup();
					_endthread();
				};
			};

            bSettingsChanged = false;
            ticks = GetTickCount();
            lastHeartbeat = ticks;
        };



		// LISTEN AND ANSWER status queries etc
		returnBytes = recvfrom(reportSocket, ListenBuf, sizeof(ListenBuf), 0, (SOCKADDR *)&remoteAddr, &remoteAddrSize);
		if (returnBytes == SOCKET_ERROR)
		{
			i = WSAGetLastError();
			if (i==10054) // reset by peer
			{
				Sleep(1000); 
				ListenBuf[0]='\0';
			}
			else if (i==10035) // would block
			{
				Sleep(5); 
				ListenBuf[0]='\0';
			}
            else
			{
				closesocket(reportSocket);
				WSACleanup();
				_endthread();
			};
		}
		else
		{
			ListenBuf[returnBytes] = '\0';

			// choose answer
			SendBuf[0] = '\0';

            char *info = strstr(ListenBuf, "info"),
                 *rules = strstr(ListenBuf, "rules"),
                 *players = strstr(ListenBuf, "players"),
				 *status = strstr(ListenBuf, "status");
			bool INFO = info != NULL,
				 RULES = rules != NULL,
				 PLAYERS = players != NULL,
				 STATUS = status != NULL;

			if (STATUS)
			{
				queryid++;
				TempBuf2[0]='\0';

				for (i=0; i<set_players; i++) 
				{
					sprintf(TempBuf, "\\player_%i\\Player %i\\team_%i\\"
									 "\\score_%i\\0\\deaths_%i\\0\0", i,(i+1),i,i,i);
					strcat(TempBuf2, TempBuf);
				};

				sprintf(TempBuf, "\\gamename\\opflashr\\gamever\\1.96\\groupid\\261"
                                 "%s%s\\final\\\\queryid\\%i.1\0)", SettingsBuf, TempBuf2, queryid);
				strcpy(SendBuf, TempBuf);
			}
			else if (INFO && RULES && PLAYERS)
			{
				queryid++;
				
				TempBuf2[0] = '\0';
				for (i=0; i<set_players; i++)
					sprintf(TempBuf, "\\player_%i\\Player %i\\team_%i\\"
									 "\\score_%i\\0\\deaths_%i\\0\0", i,(i+1),i,i,i),
					strcat(TempBuf2, TempBuf);
					
				sprintf(TempBuf, "\\groupid\\261"
								 "%s%s\\final\\\\queryid\\%i.1\0)", SettingsBuf, TempBuf2, queryid);
				strcpy(SendBuf, TempBuf);
			}
			else if (INFO && RULES)  // in-game browser
			{
				queryid++;
				sprintf(TempBuf, "\\groupid\\261"
								 "%s\\final\\\\queryid\\%i.1\0)", SettingsBuf, queryid);
				strcpy(SendBuf, TempBuf);
			}
			else if (strncmp(ListenBuf, "\\echo\\", 6) == 0 || INFO || RULES)  // ping
			{
				queryid++;
				strcpy(SendBuf, ListenBuf);
				sprintf(TempBuf, "\\final\\\\queryid\\%i.1\0", queryid);
				strcat(SendBuf, TempBuf);
			}
			else if (memcmp(ListenBuf, cmpWeirdPing, 11) == 0)  // ping
			{
				queryid++;
				sprintf(TempBuf, "\\final\\\\queryid\\%i.1\0", queryid);
				strcpy(SendBuf, TempBuf);
			};

			// send answer
			if (strlen(SendBuf) >= 4)
                sendto(reportSocket, SendBuf, strlen(SendBuf), 0, (SOCKADDR *) &remoteAddr, sizeof(remoteAddr));
        };
    }; // main loop end


	closesocket(reportSocket);
	WSACleanup();
	_endthread();
}








// Save program's exit code
void WatchProgram(ThreadArguments *arg)
{
	if (*arg->mailslot != INVALID_HANDLE_VALUE) {
		DWORD message_size   = 0;
		DWORD message_number = 0;

		if (GetMailslotInfo(*arg->mailslot, (LPDWORD)NULL, &message_size, &message_number, (LPDWORD)NULL)) {
			if (message_size != MAILSLOT_NO_MESSAGE) {

				//void *message_buffer = GlobalAlloc(GMEM_FIXED, message_size);
				HGLOBAL hGbl = GlobalAlloc(GMEM_ZEROINIT|GMEM_MOVEABLE|GMEM_DDESHARE, message_size);
				
				if (hGbl) {
					char* message_buffer = (char*)GlobalLock(hGbl);
					DWORD bytes_read     = 0;

					if (ReadFile(*arg->mailslot, message_buffer, message_size, &bytes_read, 0)) {
						int CommandID = 0;
						int db_id     = 0;
						char *params  = "";

						int value_pos   = 0;
						int value_index = 0;

						while (value_pos < (int)message_size) {
							char *token = Tokenize(message_buffer, "|", value_pos, message_size, false, false);

							switch(value_index++) {
								case 0 : CommandID=atoi(token); break;
								case 1 : db_id=atoi(token); break;
								case 2 : params=token; break;
							}
						}

						// Set variables depending on the program
						char exe_name[64] = "";
						char exe_path[64] = "fwatch\\data\\";

						switch(CommandID) {
							case C_EXE_ADDONINSTALL : strcpy(exe_name, "addonInstarrer.exe"); break;
							case C_EXE_ADDONTEST    : strcpy(exe_name, "addontest.exe"); strcpy(exe_path, "@addontest\\ModData\\"); break;
							case C_EXE_UNPBO        : strcpy(exe_name, "extractpbo.exe"); break;
							case C_EXE_MAKEPBO      : strcpy(exe_name, "makepbo.exe"); break;
							case C_EXE_WGET         : strcpy(exe_name, "wget.exe"); break;
							case C_EXE_PREPROCESS   : strcpy(exe_name, "preproc.exe"); break;
							case C_RESTART_CLIENT   : strcpy(exe_name, "gameRestart.exe"); break;

							case C_INFO_ERRORLOG    : {
								global.error_log_enabled = db_id ? 1 : 0;
								global.error_log_started = atoi(params) ? 1 : 0;
							} break;
						}

						if (strcmp(exe_name,"") != 0) {
							strcat(exe_path, exe_name);

							// Create log file
							SECURITY_ATTRIBUTES sa;
							sa.nLength              = sizeof(sa);
							sa.lpSecurityDescriptor = NULL;
							sa.bInheritHandle       = TRUE;       

							HANDLE logFile = CreateFile(TEXT("fwatch\\tmp\\exelog.txt"),
								FILE_APPEND_DATA,
								FILE_SHARE_WRITE | FILE_SHARE_READ,
								&sa,
								CREATE_ALWAYS,
								FILE_ATTRIBUTE_NORMAL,
								NULL );

							// Run program
							STARTUPINFO si;
							PROCESS_INFORMATION pi;
							ZeroMemory(&si, sizeof(si));
							si.cb          = sizeof(si);
							si.dwFlags     = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
							si.wShowWindow = SW_HIDE;
							si.hStdOutput  = logFile;
							si.hStdError   = logFile;
							ZeroMemory(&pi, sizeof(pi));
							
							if (CreateProcess(exe_path, params, NULL, NULL, TRUE, HIGH_PRIORITY_CLASS, NULL, NULL, &si, &pi)) {						
								WatchProgramInfo info = {db_id, pi.dwProcessId, STILL_ACTIVE, 0};
								db_pid_save(info);

								// Run another program monitoring the first program
								if (CommandID != C_RESTART_CLIENT) {
									do {
										Sleep(5);
										GetExitCodeProcess(pi.hProcess, &info.exit_code);
									} while (info.exit_code == STILL_ACTIVE);

									db_pid_save(info);
								}

								CloseHandle(pi.hProcess);
								CloseHandle(pi.hThread);
								CloseHandle(logFile);
							} else {
								WatchProgramInfo info = {db_id, pi.dwProcessId, STILL_ACTIVE, GetLastError()};
								db_pid_save(info);
							}
						}
					}

					GlobalFree(hGbl);
				}
			}
		}
	}

	_endthread();
}