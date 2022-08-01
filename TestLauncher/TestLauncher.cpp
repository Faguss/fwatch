#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
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
	char error_msg_last[512];
	bool error_log_enabled;
	bool error_log_started;
} global = {
	-1,
	"",
	"",
	"",
	false,
	false
};

enum GAME_TYPES {
	GAME_CLIENT,
	GAME_SERVER
};

enum COMMAND_ID {
	C_INFO_ERRORLOG    = 2670383007,
	C_RESTART_SERVER   = 1698669191,
	C_RESTART_CLIENT   = 1040407795,
	C_SPIG_EXE         = 3206713566,
	C_EXE_ADDONTEST    = 501781933,
	C_EXE_WGET         = 3861629986,
	C_EXE_UNPBO        = 1304888561,
	C_EXE_PREPROCESS   = 2891211499,
	C_EXE_ADDONINSTALL = 367297688,
	C_EXE_WEBSITE      = 226367578,
	C_EXE_MAKEPBO      = 3754554022
};

unsigned long GetIP(char *host);
void ReadUIConfig(char *filename, bool *no_ar, bool *is_custom, float *custom, int *customINT);
int ModfolderMissionsTransfer(char *mod, bool is_dedicated_server, char *player_name, int game_version);
void ModfolderMissionsReturn(bool is_dedicated_server);
void CustomFilesReturn();

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
		"fwatch/data/Format.sqf",
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



	// Temporary solution for a problem that I've created for myself
	if (GetFileAttributes("ColdWarAssault.exe") != 0xFFFFFFFF)
		rename("res\\bin\\resource.cpp","res\\bin\\resource_disabled.cpp");


	// Create mailslot
	SECURITY_DESCRIPTOR sd;
	InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
	SetSecurityDescriptorDacl(&sd, true, NULL, false);

	SECURITY_ATTRIBUTES sa;
	sa.lpSecurityDescriptor = &sd;
	sa.bInheritHandle       = false;

	HANDLE mailslot = CreateMailslot(TEXT("\\\\.\\mailslot\\fwatch_mailslot"), 0, MAILSLOT_WAIT_FOREVER, &sa);

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
	StringDynamic command_line;
	StringDynamic_init(command_line);

	String lpCmdLine_String    = {lpCmdLine, strlen(lpCmdLine)};
	String word                = {NULL, 0};
	size_t lpCmdLine_pos       = 0;
	bool launch_client         = true;
	bool listen_server         = false;
	bool add_nomap             = true;
	bool launch_steam          = false;
	bool first_item            = true;
	bool copy_param            = true;
	int custom_exe             = -1;
	ThreadArguments client_arg = {false, false, false, &mailslot};

	while ((word = String_tokenize(lpCmdLine_String, " \t\r\n", lpCmdLine_pos, OPTION_NONE)).length > 0) {
		if (strcmp(word.text,"-nomap") == 0)
			add_nomap = false;

		if (strcmp(word.text,"-nolaunch") == 0) {
			launch_client = false;
			copy_param    = false;
		}

		if (strncmp(word.text,"-gamespy=",9) == 0) {
			strncpy(global.master_server1, word.text+9, 64);
			client_arg.is_custom_master1 = true;
			copy_param                   = false;
		}

		if (strncmp(word.text,"-udpsoft=",9) == 0) {
			strncpy(global.master_server2, word.text+9, 18);
			client_arg.is_custom_master2 = true;
			copy_param                   = false;
		}

		if (strcmp(word.text,"-reporthost") == 0) {
			listen_server = true;
			copy_param    = false;
		}

		if (strcmp(word.text,"-removenomap") == 0) {
			add_nomap  = false;
			copy_param = false;
		}

		if (strcmp(word.text,"-steam") == 0) {
			launch_client = false;
			launch_steam  = true;
			copy_param    = false;
		}

		if (strncmp(word.text,"-run=",5) == 0) {
			for (int i=0; i<global_exe_num; i++) {
				if (strcmpi(global_exe_name[i], word.text+5) == 0) {
					custom_exe = i;
					break;
				}
			}

			copy_param = false;
		}

		if (fi)
			fprintf(fi, "%s\"%s\"", (first_item ? "" : ","), word.text);

		if (copy_param) {
			StringDynamic_append(command_line, " ");			
			StringDynamic_appends(command_line, word);
		} else
			copy_param = true;

		first_item = false;
	}

	if (fi) {
		fprintf(fi,"]]");
		fclose(fi);
	}



	// Read master server address from the file if an argument wasn't passed
	if (!client_arg.is_custom_master1) {
		StringDynamic ip_list;
		int result = StringDynamic_readfile(ip_list, "fwatch\\idb\\MasterServers.sqf");

		if (result == 0) {
			int open[]  = {'\"', '{'};
			int close[] = {'\"', '}'};

			for (int i=0; i<2; i++) {
				char *quote = strchr(ip_list.text, open[i]);

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

		StringDynamic_end(ip_list);
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

		StringDynamic command_line_new;
		StringDynamic_init(command_line_new);
		StringDynamic_appendf(command_line_new, " %s%s", (add_nomap ? "-nomap " : ""), command_line.text);

		for (int i=0; i<global_exe_num; i++) {
			if (custom_exe!=-1 && custom_exe!=i || strstr(global_exe_name[i],"_server.exe"))
				continue;

			if (CreateProcess(global_exe_name[i], command_line_new.text, NULL, NULL, false, 0, NULL,NULL,&si,&pi)) {
				launched = true;
				break;
			}
		}
		
		StringDynamic_end(command_line_new);

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
					StringDynamic command_line_new;
					StringDynamic_init(command_line_new);
					StringDynamic_appendf(command_line_new, "\"start \"\" \"%s\" -applaunch 65790 %s%s\"", SteamExe, (add_nomap ? "-nomap " : ""), command_line.text);
					system(command_line_new.text);
					StringDynamic_end(command_line_new);
				}

				RegCloseKey(hKey);
			}
		}

		// Just sit and wait here
		MessageBox( NULL, "Fwatch is now waiting for you to run the game and/or dedicated server.\nDon't forget to add -nomap\n\nWhen you're done playing press OK to close Fwatch.", "fwatch", MB_OK|MB_ICONINFORMATION );
	}




    RemoveHook();
	StringDynamic_end(command_line);
	ModfolderMissionsReturn(GAME_CLIENT);
	ModfolderMissionsReturn(GAME_SERVER);
	CustomFilesReturn();
	unlink("fwatch_info.sqf");
	unlink("fwatch\\data\\pid.db");
	CloseHandle(mailslot);

	WIN32_FIND_DATA FileInformation;
	HANDLE hFile = FindFirstFile("fwatch\\tmp\\_exelog\\*", &FileInformation);
	char del_path[128];
	
	if (hFile != INVALID_HANDLE_VALUE) {
		do {
			if (FileInformation.cFileName[0] != '.') {
				sprintf(del_path, "fwatch\\tmp\\_exelog\\%s", FileInformation.cFileName);
				DeleteFile(del_path);
			}
		}
		while (FindNextFile(hFile, &FileInformation) == TRUE);
		FindClose(hFile);
	}

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




// Parse Fwatch modfolder configuration and fill arrays with new values
void ReadUIConfig(char *filename, bool *no_ar, bool *is_custom, float *custom, int *customINT)
{
	StringDynamic buf_config;
	if (StringDynamic_readfile(buf_config, filename) != 0)
		return;

	memset(no_ar    , 0, sizeof(no_ar));
	memset(is_custom, 0, sizeof(is_custom));
	memset(custom   , 0, sizeof(custom));
	memset(customINT, 0, sizeof(customINT));

	String config     = {buf_config.text, buf_config.length};
	String property   = {NULL, 0};
	size_t config_pos = 0;

	// For each config property
	while ((property = String_tokenize(config, ";", config_pos, OPTION_SKIP_SQUARE_BRACKETS)).length > 0) {
		char *equality = strchr(property.text, '=');
		if (equality == NULL)
			continue;

		String_trim_space(property);
		size_t equality_pos         = equality - property.text;
		property.text[equality_pos] = '\0';
		int property_index          = -1;

		for (int i=0;  i<sizeof(hud_names)/sizeof(hud_names[0]);  i++)
			if (strcmpi(property.text,hud_names[i]) == 0) {
				property_index = i;
				break;
			}
		
		if (property_index < 0)
			continue;

		String value     = {property.text+equality_pos+1, property.length-equality_pos-1};
		String subvalue  = {NULL, 0};
		size_t value_pos = 0;

		// For each property value
		while ((subvalue = String_tokenize(value, ",", value_pos, OPTION_SKIP_SQUARE_BRACKETS)).length > 0) {
			String_trim_space(subvalue);

			if (strcmpi(subvalue.text, "noar")==0)
				no_ar[property_index] = 1;
			else {
				is_custom[property_index] = 1;

				if (IsNumberInArray(i,hud_int_list,sizeof(hud_int_list)/sizeof(hud_int_list[0]))) {
					if (IsNumberInArray(i,hud_color_list,sizeof(hud_color_list)/sizeof(hud_color_list[0]))) {
						int index              = 0;
						unsigned char color[4] = {0,0,0,0};
						String number          = {NULL, 0};
						size_t subvalue_pos    = 0;

						while ((number = String_tokenize(subvalue, ",;", subvalue_pos, OPTION_TRIM_SQUARE_BRACKETS)).length>0  &&  index<4) {
							String_trim_space(number);
							color[index++] = (unsigned char)(atof(number.text) * 255);
						}

						customINT[property_index] = ((color[3] << 24) | (color[0] << 16) | (color[1] << 8) | color[2]);
					} else
						customINT[property_index] = atoi(subvalue.text);
				} else
					custom[property_index] = (float)atof(subvalue.text);
			}
		}
	}

	StringDynamic_end(buf_config);
}




// Move mission files from the selected modfolder to the game missions folder
int ModfolderMissionsTransfer(char *mod, bool is_dedicated_server, char *player_name, int game_version)
{
	int file_count    = 0;
	char filename[64] = "fwatch\\data\\sortMissions";
	strcat(filename, is_dedicated_server ? "_server.txt" : ".txt");

	FILE *f = fopen(filename, "a");
	if (f) {
		char source[1024]           = "";
		char destination[1024]      = "";
		wchar_t source_w[1024]      = L"";
		wchar_t destination_w[1024] = L"";
		wchar_t extension[5]        = L"";

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

		char folder_src[][64] = {
			"MPMissions",
			"Templates",
			"SPTemplates",
			"IslandCutscenes",
			"IslandCutscenes\\_RES",
			"MissionsUsers",
			"MPMissionsUsers"
		};
		char folder_dst[][64] = {
			"MPMissions",
			"Templates",
			"SPTemplates",
			"Addons",
			"Res\\Addons",
			"Users\\",
			"Users\\"
		};

		if (game_version != VER_196)
			strcpy(folder_dst[4], "Addons");

		strcat(folder_dst[5], player_name);
		strcat(folder_dst[5], "\\Missions");
		strcat(folder_dst[6], player_name);
		strcat(folder_dst[6], "\\MPMissions");

		int folder_num           = sizeof(folder_src) / sizeof(folder_src[0]);
		wchar_t folder_src_w[64] = L"";
		wchar_t folder_dst_w[64] = L"";

		if (strcmp(player_name,"") == 0)
			folder_num -= 2;


		for (i=0; i<folder_num; i++) {
			size_t folder_src_len = strlen(folder_src[i]);
			size_t folder_dst_len = strlen(folder_dst[i]);

			MultiByteToWideChar(CP_UTF8, 0, folder_src[i], folder_src_len+1, folder_src_w, 1023);
			MultiByteToWideChar(CP_UTF8, 0, folder_dst[i], folder_dst_len+1, folder_dst_w, 1023);
			
			// Source path is: mod\MPMissions\*
			MultiByteToWideChar(CP_UTF8, 0, mod, mod_len+1, source_w, 1023);
			wcscat(source_w  , L"\\");
			wcscat(source_w  , folder_src_w);
			wcscat(source_w  , L"\\*");

			// Destination path is: MPMissions\ 
			wcscpy(destination_w, folder_dst_w);
			wcscat(destination_w, L"\\");

			hFind = FindFirstFileW(source_w, &fd);
			WideCharToMultiByte(CP_UTF8,0,source_w,-1,source,1023,NULL,NULL);

			bool only_folders = i >= 3;

			if (hFind != INVALID_HANDLE_VALUE) {
				do {
					if (wcscmp(fd.cFileName,L".")==0 || wcscmp(fd.cFileName,L"..")==0 || (_wcsicmp(fd.cFileName,L"_RES")==0 && i==3))
						continue;
					
					size_t filename_length = wcslen(fd.cFileName);
					DWORD dir              = fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;

					if (filename_length<3 || filename_length>(1023-mod_len-13))
						continue;

					int dots     = 0;
					wchar_t *dot = fd.cFileName;

					while((dot=wcschr(dot,L'.')) != NULL && dots<2) {
						dots++;
						dot++;
					}

					// Lowercase extension
					if (filename_length > 3) {
						wcsncpy(extension, fd.cFileName+filename_length-4, 4);
						_wcslwr(extension);
					} else
						wcscpy(extension, L"");

					if (dots>=1 && dir || (!dir && dots>=2 && wcscmp(extension,L".pbo")==0) || (only_folders && dir)) {
						wcscpy(source_w + mod_len + folder_src_len + 2, fd.cFileName);
						wcscpy(destination_w + folder_dst_len + 1, fd.cFileName);

						WideCharToMultiByte(CP_UTF8,0,source_w,-1,source,1023,NULL,NULL);
						WideCharToMultiByte(CP_UTF8,0,destination_w,-1,destination,1023,NULL,NULL);

						if (MoveFileExW(source_w, destination_w, 0)) {
							file_count++;

							if (i >= 5  || (game_version!=VER_196 && i>=4))
								fprintf(f, "%s?%s\n", source, destination);
							else
								fprintf(f, "%s\n", source);
						}
					}
				} 
				while (FindNextFileW(hFind, &fd));
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

	StringDynamic buf_mission_list;
	if (StringDynamic_readfile(buf_mission_list, filename) != 0)
		return;

	size_t word_start           = 0;
	char backup_buffer[1024]    = "";
	wchar_t source_w[1024]      = L"";
	wchar_t destination_w[1024] = L"";
	String mission_list         = {buf_mission_list.text, buf_mission_list.length};
	String destination          = {NULL, 0};
	size_t mission_list_pos     = 0;

	while ((destination = String_tokenize(mission_list, "\r\n", mission_list_pos, OPTION_NONE)).length > 0) {
		char *source     = strchr(destination.text, '\\');
		bool remove_line = false;
		
		// Legitimate line contains a backslash
		if (source) {
			source++;
			char *separator = strchr(destination.text, '?');
			
			// Check if line is: source?destination
			// For example: @mod\Missions?Missions\@mod
			if (separator != NULL) {
				int separator_pos               = separator - destination.text;
				source                          = destination.text + separator_pos + 1;
				destination.text[separator_pos] = '\0';
				
				if (MoveFileEx(source, destination.text, 0))
					remove_line = true;
				else
					destination.text[separator_pos] = '?';
			} else {
				// Otherwise it's only a source location but destination can be derived from it
				// For example: @mod\MPMissions\name.island.pbo
				size_t source_length = (destination.text + destination.length) - source;
				
				// Check if it's island cutscene
				if (strncmpi(source,"IslandCutscenes\\_RES\\",21) == 0) {
					strcpy(backup_buffer, "Res\\Addons\\");
					strcat(backup_buffer, source+21);
					
					source_length += 11 - 21;
					source         = backup_buffer;
				} else
					if (strncmpi(source,"IslandCutscenes\\",16) == 0) {
						strcpy(backup_buffer, "Addons\\");
						strcat(backup_buffer, source+16);
						
						source_length += 7 - 16;
						source         = backup_buffer;
					}

				MultiByteToWideChar(CP_UTF8, 0, source          , source_length+1     , source_w     , 1023);
				MultiByteToWideChar(CP_UTF8, 0, destination.text, destination.length+1, destination_w, 1023);

				if (MoveFileExW(source_w, destination_w, 0)) {
					remove_line = true;
				} else {
					// Check if the file still  exists
					WIN32_FILE_ATTRIBUTE_DATA fad;

					if (GetFileAttributesExW(source_w, GetFileExInfoStandard, &fad) == -1)
						remove_line = true;
				}
			}
		} else
			remove_line = true;
		
		
		// Restore new line character
		if (mission_list_pos < mission_list.length) {
			if (mission_list.text[mission_list_pos] == '\n') {
				if (mission_list_pos > 0)
					mission_list.text[mission_list_pos-1] = '\r';
					
				mission_list_pos++;
			} else
				mission_list.text[mission_list_pos-1] = '\n';
		}
		
		
		// Remove current line from the buffer
		if (remove_line) {					
			size_t shift_amount = mission_list_pos - word_start;

			shift_buffer_chunk(mission_list.text, mission_list_pos, mission_list.length, shift_amount, OPTION_LEFT);
			
			mission_list_pos    -= shift_amount;
			mission_list.length -= shift_amount;
		}

		word_start = mission_list_pos;
	}

	mission_list.text[mission_list.length] = '\0';
	
	FILE *f = fopen(filename, "wb");
	if (f) {
		fwrite(mission_list.text, 1, mission_list.length, f);
		fclose(f);
	}

	StringDynamic_end(buf_mission_list);
}




// Move sounds back to the sounds folder and revert to custom face
void CustomFilesReturn() 
{
	char filename[64] = "fwatch\\data\\user_rename.txt";

	StringDynamic buf_rename_list;
	if (StringDynamic_readfile(buf_rename_list, filename) != 0)
		return;

	size_t word_start    = 0;
	String file_list     = {buf_rename_list.text, buf_rename_list.length};
	String destination   = {NULL, 0};
	size_t file_list_pos = 0;

	while ((destination = String_tokenize(file_list, "\r\n", file_list_pos, OPTION_NONE)).length > 0) {
		char *separator  = strchr(destination.text, '?');
		bool remove_line = false;

		// Bring back face back to custom
		if (strstr(destination.text, "UserInfo.cfg")) {
			StringDynamic buf_userinfo;
			if (StringDynamic_readfile(buf_userinfo, destination.text) == 0) {
				char *face_ptr = strstr(buf_userinfo.text, "face=\"Face52\"");
				if (face_ptr) {
					memcpy(face_ptr, "face=\"Custom\"", 13);
					FILE *f = fopen(destination.text, "wb");
					if (f) {
						fwrite(buf_userinfo.text, 1, buf_userinfo.length, f);
						remove_line = true;
					}
				}
			}
			StringDynamic_end(buf_userinfo);
		} else 
		if (separator) {
			// Move sound back to the sound folder
			int separator_pos               = separator - destination.text;
			char *source                    = destination.text + separator_pos + 1;
			destination.text[separator_pos] = '\0';
			
			if (MoveFileEx(source, destination.text, 0))
				remove_line = true;
			else
				destination.text[separator_pos] = '?';
		} else
			remove_line = true;
		
		// Restore new line character
		if (file_list_pos < file_list.length) {
			if (file_list.text[file_list_pos] == '\n') {
				if (file_list_pos > 0)
					file_list.text[file_list_pos-1] = '\r';
					
				file_list_pos++;
			} else
				file_list.text[file_list_pos-1] = '\n';
		}
		
		
		// Remove current line from the buffer
		if (remove_line) {
			size_t shift_amount = file_list_pos - word_start;

			shift_buffer_chunk(file_list.text, file_list_pos, file_list.length, shift_amount, OPTION_LEFT);
			
			file_list_pos    -= shift_amount;
			file_list.length -= shift_amount;
		}

		word_start = file_list_pos;
	}

	file_list.text[file_list.length] = '\0';
	
	FILE *f = fopen(filename, "wb");
	if (f) {
		fwrite(file_list.text, 1, file_list.length, f);
		fclose(f);
	}

	StringDynamic_end(buf_rename_list);
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
	bool transfer_missions     = true;
	int signal_action          = -1;
	char *signal_buffer        = "";

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
			sprintf(cmdLine, "\"%s\" Aspect_Ratio.sqf -out=Aspect_Ratio.sqf ", pwd);

			if (CreateProcess("fwatch\\data\\preproc.exe", cmdLine, NULL, NULL, false, 0, NULL, NULL, &si, &pi)) {
				DWORD st;

				do {					
					GetExitCodeProcess(pi.hProcess, &st);
					Sleep(5);
				} while (st == STILL_ACTIVE);

				CloseHandle(pi.hProcess);
				CloseHandle(pi.hThread); 


				StringDynamic config;
				if (StringDynamic_readfile(config, "Aspect_Ratio.sqf") == 0) {
					char *token = strtok(config.text, ";\n\t ");

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

				StringDynamic_end(config);
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
				if (strcmp(current_window_name, global_window_name[i]) == 0) {
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
			if (!transfer_missions) {
				transfered_missions = 0;
				transfer_missions   = true;
				ModfolderMissionsReturn(arg->is_dedicated_server);
			}

			continue;
		}

		phandle = OpenProcess(PROCESS_ALL_ACCESS, 0, game_pid);

		if (phandle == INVALID_HANDLE_VALUE)
			continue;

		// If this is a new game instance
		if (game_pid != game_pid_last) {
			if (!arg->is_dedicated_server) {
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

				if (ui_portrait_mode)
					ui_portrait_calculate = true;
			}


			// Get list of arguments passed to the game executable
			ReadProcessMemory(phandle, (LPVOID)game_param_address, &game_param_address, 4, &stBytes);
			ReadProcessMemory(phandle, (LPVOID)game_param_address, &game_param_address, 4, &stBytes);
			ReadProcessMemory(phandle, (LPVOID)game_param_address, &game_param        , game_param_max, &stBytes);

			size_t word_start = 0;
			bool word_started = false;
			game_mods_num     = 0;

			// Copy mod names into array
			for (size_t i=0; i<game_param_max; i++) {
				if (game_param[i] == '\0') {
					if (strncmp(game_param+word_start,"-mod=",5) == 0) {
						String value     = {game_param+word_start+5, strlen(game_param+word_start+5)};
						String item;
						size_t value_pos = 0;

						while ((item = String_tokenize(value, ";", value_pos, OPTION_NONE)).length>0  && game_mods_num<game_mods_max)
							game_mods[game_mods_num++] = item.text;
					}

					if (game_param[i+1] == '\0')
						break;

					word_started = false;
				} else
					if (!word_started) {
						word_start   = i;
						word_started = true;
					}
			}

			// Read fwatch configuration from each mod
			if (!arg->is_dedicated_server)
				for (int i=0; i<game_mods_num; i++)
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
			// Read player's name
			char player_name[25] = "";
			int player_name_ptr  = 0;
			int player_name_addr = 0;

			if (!arg->is_dedicated_server) {
				switch(global_exe_version[game_exe_index]) {
					case VER_196 : player_name_ptr=0x7DD184; break;
					case VER_199 : player_name_ptr=0x7CC144; break;
					case VER_201 : player_name_ptr=game_exe_address+0x714C10; break;
				}

				ReadProcessMemory(phandle, (LPVOID)player_name_ptr       , &player_name_addr, 4 , &stBytes);
				ReadProcessMemory(phandle, (LPVOID)(player_name_addr+0x8), &player_name     , 25, &stBytes);
			}
			
			// If it's a game instance that we have accessed before
			// Transfer mission files from each mod
			if (transfer_missions && game_mods_num>0) {
				transfer_missions = false;

				for (int i=game_mods_num-1; i>=0; i--)
					transfered_missions += ModfolderMissionsTransfer(game_mods[i], arg->is_dedicated_server, player_name, global_exe_version[game_exe_index]);
			}

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

					if (strcmp(error_msg,global.error_msg_last) != 0) {
						strcpy(global.error_msg_last, error_msg);

						/*if (!global.error_log_started) {
							fd=fopen("fwatch_debug.txt","a");fprintf(fd,"EXE message dll that log is starting\n");fclose(fd);
							global.error_log_started = true;
							HANDLE message = CreateFile("scripts\\:info errorlog start", GENERIC_READ, 0, NULL, OPEN_EXISTING, 128, NULL);

							if (message != INVALID_HANDLE_VALUE)
								CloseHandle(message);
						}*/

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


			// Check messages from fwatch.dll
			if (*arg->mailslot != INVALID_HANDLE_VALUE) {
				DWORD message_size   = 0;
				DWORD message_number = 0;

				if (GetMailslotInfo(*arg->mailslot, (LPDWORD)NULL, &message_size, &message_number, (LPDWORD)NULL)) {
					if (message_size != MAILSLOT_NO_MESSAGE)
						_beginthread((void(*)(void*))WatchProgram, 0, arg);
				}
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
				HGLOBAL hGbl = GlobalAlloc(GMEM_ZEROINIT|GMEM_MOVEABLE|GMEM_DDESHARE, message_size);
				
				if (hGbl) {
					char* message_buffer = (char*)GlobalLock(hGbl);
					DWORD bytes_read     = 0;

					if (ReadFile(*arg->mailslot, message_buffer, message_size, &bytes_read, 0)) {
						String message      = {message_buffer, message_size};
						String token;
						unsigned int hash   = 0;
						int db_id           = 0;
						char empty_string[] = "";
						char *params        = empty_string;
						size_t message_pos  = 0;
						int value_index     = 0;

						while ((token = String_tokenize(message, "|", message_pos, OPTION_NONE)).length > 0) {

							switch(value_index++) {
								case 0 : hash   = strtoul(token.text, NULL, 0); break;
								case 1 : db_id  = atoi(token.text); break;
								case 2 : params = token.text; break;
							}
						}

						// Set variables depending on the program
						char exe_name[64] = "";
						char exe_path[64] = "fwatch\\data\\";

						switch(hash) {
							case C_EXE_ADDONINSTALL : strcpy(exe_name, "addonInstarrer.exe"); break;
							case C_EXE_ADDONTEST    : strcpy(exe_name, "addontest.exe"); strcpy(exe_path, "@addontest\\ModData\\"); break;
							case C_EXE_UNPBO        : strcpy(exe_name, "extractpbo.exe"); break;
							case C_EXE_MAKEPBO      : strcpy(exe_name, "makepbo.exe"); break;
							case C_EXE_WGET         : strcpy(exe_name, "wget.exe"); break;
							case C_EXE_PREPROCESS   : strcpy(exe_name, "preproc.exe"); break;
							case C_RESTART_SERVER   :
							case C_RESTART_CLIENT   : strcpy(exe_name, "gameRestart.exe"); break;

							case C_INFO_ERRORLOG    : {
								bool enabled_new = db_id ? 1 : 0;
								bool started_new = atoi(params) ? 1 : 0;

								if (global.error_log_enabled != enabled_new || global.error_log_started != started_new)
									strcpy(global.error_msg_last,"");
								
								global.error_log_enabled = db_id ? 1 : 0;
								global.error_log_started = atoi(params) ? 1 : 0;
							} break;
						}

						if (strcmp(exe_name,"") != 0) {
							strcat(exe_path, exe_name);

							// Create log file
							// TODO: need to have separate files
							SECURITY_ATTRIBUTES sa;
							sa.nLength              = sizeof(sa);
							sa.lpSecurityDescriptor = NULL;
							sa.bInheritHandle       = TRUE;

							CreateDirectory("fwatch\\tmp\\_exelog", NULL);
							char logfilename[128];
							sprintf(logfilename, "fwatch\\tmp\\_exelog\\%d", db_id);

							HANDLE logfile_stdout = CreateFile((LPTSTR)logfilename,
								FILE_WRITE_DATA,
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
							si.hStdOutput  = logfile_stdout;
							si.hStdError   = logfile_stdout;
							ZeroMemory(&pi, sizeof(pi));

							if (CreateProcess(exe_path, params, NULL, NULL, TRUE, HIGH_PRIORITY_CLASS, NULL, NULL, &si, &pi)) {
								WatchProgramInfo info = {db_id, pi.dwProcessId, STILL_ACTIVE, 0};
								db_pid_save(info);

								// Run another program monitoring the first program
								if (hash != C_RESTART_CLIENT) {
									do {
										Sleep(5);
										GetExitCodeProcess(pi.hProcess, &info.exit_code);
									} while (info.exit_code == STILL_ACTIVE);

									if (hash==C_EXE_MAKEPBO  &&  info.exit_code==0) {
										//Extract path from arguments
										StringDynamic path_dir;
										StringDynamic path_pbo;
										StringDynamic path_dir_file;
										StringDynamic_init(path_dir);
										StringDynamic_init(path_pbo);
										StringDynamic_init(path_dir_file);

										for (int i=strlen(params),quot=0; i>=0; i--) {
											if (params[i] == '"') {
												quot++;

												if (quot == 1)
													params[i] = '\0';

												if (quot == 2)
													StringDynamic_append(path_dir, params+i+1);
											}
										}
										
										StringDynamic_appendf(path_pbo, "%s.pbo", path_dir.text);
										StringDynamic_appendf(path_dir_file, "%s\\", path_dir.text);
										
										StringDynamic buffer_pbo;
										int result = StringDynamic_readfile(buffer_pbo, path_pbo.text);

										if (result == 0) {
											const int name_max  = 512;
											char name[name_max] = "";
											int name_len        = 0;
											int file_count      = 0;
											size_t file_pos     = 0;
											bool save_file      = false;
											 
											while (file_pos < buffer_pbo.length) {
												memset(name, 0, name_max);
												name_len = 0;

												for (int i=0; i<name_max-1; i++) {
													char c = buffer_pbo.text[file_pos++];

													if (c != '\0')
														name[name_len++] = c;
													else
														break;
												}

												unsigned long pbo_mime_type  = *((unsigned long*)&buffer_pbo.text[file_pos]);
												unsigned long pbo_time_stamp = *((unsigned long*)&buffer_pbo.text[file_pos+12]);
												unsigned long pbo_data_size  = *((unsigned long*)&buffer_pbo.text[file_pos+16]);

												file_pos += 20;

												if (name_len == 0) {
													if (file_count==0 && pbo_mime_type==0x56657273 && pbo_time_stamp==0 && pbo_data_size==0) {
														int value_len = 0;
														bool is_name  = true;
														
														while (file_pos < buffer_pbo.length) {
															if (buffer_pbo.text[file_pos++] != '\0')
																value_len++;
															else {
																if (is_name && value_len==0)
																	break;
																else {
																	is_name   = !is_name;
																	value_len = 0;
																}
															}
														}
													} else
														break;
												} else {
													path_dir_file.length = path_dir.length+ 1;
													StringDynamic_append(path_dir_file, name);
													
													WIN32_FILE_ATTRIBUTE_DATA fd;
													GetFileAttributesEx(path_dir_file.text, GetFileExInfoStandard, &fd);
													
													ULARGE_INTEGER ull;
													ull.LowPart              = fd.ftLastWriteTime.dwLowDateTime;
													ull.HighPart             = fd.ftLastWriteTime.dwHighDateTime;
													ULONGLONG n1             = (ULONGLONG)10000000;
													ULONGLONG n2             = UInt32x32To64(116444736, 100);
													unsigned long file_stamp = (unsigned long)(ull.QuadPart / n1 - n2);

													if (file_stamp != pbo_time_stamp) {
														memcpy(buffer_pbo.text+file_pos-8, &file_stamp, 4);
														save_file = true;
													}
												}
													
												file_count++;
											}
											
											if (save_file) {
												FILE *f = fopen(path_pbo.text, "wb");
												if (f) {
													fwrite(buffer_pbo.text, 1, buffer_pbo.length, f);
													fclose(f);
												}
											}
										}

										StringDynamic_end(path_dir);
										StringDynamic_end(path_pbo);
										StringDynamic_end(path_dir_file);
										StringDynamic_end(buffer_pbo);
									}

									db_pid_save(info);
								}

								CloseHandle(pi.hProcess);
								CloseHandle(pi.hThread);
							} else {
								WatchProgramInfo info = {db_id, pi.dwProcessId, STILL_ACTIVE, GetLastError()};
								db_pid_save(info);
							}

							CloseHandle(logfile_stdout);
						}
					}

					GlobalFree(hGbl);
				}
			}
		}
	}

	_endthread();
}