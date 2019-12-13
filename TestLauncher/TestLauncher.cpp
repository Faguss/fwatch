#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "..\testdll\testdll.h"
#include <tlhelp32.h>							//v1.11 for process search
#include "winsock2.h"							//v1.14 for reporting server
#include "..\testdll\global_vars.h"	//1.16

bool runOfp(LPSTR lpCmdLine, bool addNoMap);
bool TestHook(void);
void FwatchPresence(void *loop);				//v1.1
int findProcess(char* name);					//v1.11
int countThreads(unsigned int pid);  			//v1.11
int findOFPwindow();							//v1.11
void ListenServer(void *loop);					//v1.14
void FixUIAspectRatio(void *loop);				//v1.15
int Read_Config_Fwatch_HUD(char *filename, bool *no_ar, bool *is_custom, float *custom, int *customINT);
void Transfer_Modfolder_Missions(char *mod, bool server);
void Return_Modfolder_Missions(bool server);


//v1.13 global variables for OFP and CWA
int Game_Version		= 0;
char GameWindowName[32]	= "";
char GameServerName[32]	= "";
bool ErrorLog_Enabled	= 0;					//v1.15 save error messages to text file
bool ErrorLog_Started	= 0;					//v1.15 is this the first entry in the log
int Game_Exe_Address    = 0;





char reqfiles[][256] = {
	"fwatch/data/wget.exe",
	"fwatch/data/true.sqf",
	"fwatch/data/fwatch_check.sqf",

	"fwatch/data/gameRestart.exe",			//v1.11

	"fwatch/data/ConvertKeyString.sqf",		//v1.13
	"fwatch/data/CurrentLanguage.sqf",		//v1.13
	"fwatch/data/DateDifference.sqf",		//v1.13
	"fwatch/data/DateDifferenceInDays.sqf",	//v1.13
	"fwatch/data/FormatDate.sqf",			//v1.13
	"fwatch/data/InitFLib.sqf",				//v1.13
	"fwatch/data/inKeys.sqf",				//v1.13
	"fwatch/data/InputMulti.sqs",			//v1.13
	"fwatch/data/MainMenu.sqs",				//v1.13
	"fwatch/data/MeasureTime.sqf",			//v1.13
	"fwatch/data/MemMulti.sqs",				//v1.13
	"fwatch/data/ModifyDate.sqf",			//v1.13
	"fwatch/data/onScreenTyping.sqs",		//v1.13

	"fwatch/data/getAspectRatio.sqf",		//v1.14
	"fwatch/data/DePbo.dll",				//v1.14
	"fwatch/data/ExtractPbo.exe",			//v1.14
	"fwatch/data/getExitCode.exe",			//v1.14
	"fwatch/data/preproc.exe",				//v1.14

	"fwatch/data/Download.sqs",				//v1.16
	"fwatch/data/MainMenu_fnc.sqf",			//v1.16
	"fwatch/data/addonInstaller.exe",		//v1.16
	"fwatch/data/7z.dll",					//v1.16
	"fwatch/data/7z.exe",					//v1.16
	"fwatch/data/MakePbo.exe",				//v1.16

	NULL,
};

char game_execs[][40] = {
	"armaresistance.exe",
	"coldwarassault.exe",
	"flashpointresistance.exe",
	"ofp.exe",
	"flashpointbeta.exe",
	"operationflashpoint.exe",
	"operationflashpointbeta.exe"
};

char server_execs[][40] = {
	"armaresistance_server.exe",
	"coldwarassault_server.exe",
	"ofpr_server.exe"
};

//v1.13 global chars for master servers names
char serv1[64] = "";
char serv2[19] = "";

//v1.14 global variables to share between threads
int ListenServerPort  = -1;
char master_serv1[64] = "";
char master_serv2[64] = ""; 

//v1.15 global variables for a pipe
HANDLE hPipe;
char pipe_buffer[1024];
DWORD dwRead;

//v1.16
char run_exe_name[256] = "";

enum CHAT_STATES {
	CHAT_MISSION,
	CHAT_BRIEFINGLOBBY,
	CHAT_CUSTOM
};







int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	// Fwatch duplication check v1.11
	if (findProcess("fwatch.exe")) {
		MessageBox( NULL, "Do not duplicate fwatch processes.", "fwatch", MB_OK|MB_ICONSTOP );
		return 1;
	};

	Return_Modfolder_Missions(false);
	Return_Modfolder_Missions(true);
	
	//v1.16 save info about this instance to a file
	FILE *fi = fopen("fwatch_info.sqf","w");
	fprintf(fi,"[%d,[",GetCurrentProcessId());

	//v1.13 arguments to change the master server
	// Copy command line to a buffer
	int lpCmdLine_size  = strlen(lpCmdLine);
	bool nolaunch		= false;
	bool listenServer	= false;
	bool addNoMap		= true;
	bool Steam			= false;
	bool first_item     = true;
	bool buffer_shift   = false;
	char *command_line  = lpCmdLine;

	for (int i=0, begin=-1; i<=lpCmdLine_size; i++) {
		bool complete_word = i == lpCmdLine_size;

		if (isspace(lpCmdLine[i]))
			complete_word = true;

		if (!complete_word  &&  begin<0)
			begin = i;

		if (complete_word  &&  begin>=0) {
			char *word   = lpCmdLine+begin;
			char prev    = lpCmdLine[i];
			lpCmdLine[i] = '\0';

			if (strncmp(word,"-nolaunch",9) == 0) {
				nolaunch     = true;	//v1.13 -nolaunch moved here
				buffer_shift = true;
			}

			if (strncmp(word,"-gamespy=",9) == 0) {
				strncpy(serv1, word+9, 64);
				buffer_shift = true;
			}

			if (strncmp(word,"-udpsoft=",9) == 0) {
				strncpy(serv2, word+9, 18);
				buffer_shift = true;
			}

			if (strncmp(word,"-reporthost",11) == 0) {
				listenServer = true;
				buffer_shift = true;
			}

			if (strncmp(word,"-removenomap",12) == 0) {
				addNoMap     = false;
				buffer_shift = true;
			}

			if (strncmp(word,"-steam",6) == 0) {
				nolaunch     = true;
				Steam        = true;
				buffer_shift = true;
			}

			if (strncmp(word,"-run=",5) == 0) {
				strncpy(run_exe_name, word+5, 255);
				buffer_shift = true;
			}

			fprintf(fi,"%s\"%s\"",first_item ? "" : ",", word);
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
	};

	fprintf(fi,"]]");
	fclose(fi);

	
	// Check that all required files are in place
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






	// Install hook
    InstallHook();
	


	// Make sure the hooks work	
	int res;
	do {
		res = IDOK;
		if(!TestHook()) {
			Sleep(500); // Wait a while and automatically retry once
			if(!TestHook()) {
				res = MessageBox(NULL, "Self test failed, Fwatch hooks do not appear to be working.\n", "fwatch error", MB_ABORTRETRYIGNORE|MB_ICONSTOP);			
				if(res == IDABORT)
					exit(0);
			}
		}
	} while(res == IDRETRY);




	// Launch thread for listen server
	if (listenServer)
		_beginthread(ListenServer, 0, (void*) (0));

	// Launch thread for modifying UI
	_beginthread(FixUIAspectRatio, 0, (void*) (0));



	if(!nolaunch) {
		_beginthread(FwatchPresence, 0, (void*) (0));	//v1.1
		// Attempt to run OFP
		if(!runOfp(lpCmdLine, addNoMap))
			MessageBox(NULL, "Could not run the game!\n\nEnsure fwatch.exe is located in your game directory,\nor run fwatch with -nolaunch parameter and then run the game manually.", "fwatch error", MB_OK|MB_ICONSTOP);
	} else {
		// Just sit and wait here
		_beginthread(FwatchPresence, 0, (void*) (1));	//v1.1
		_beginthread(FwatchPresence, 0, (void*) (2));	//v1.1

			//1.15 launch steam
			if (Steam)
			{
				Sleep(5000);
				HKEY hKey			= 0;
				char SteamPath[255] = {0};
				char SteamExe[255]	= {0};
				DWORD dwType		= 0;
				DWORD SteamPathSize	= sizeof(SteamPath);
				DWORD SteamExeSize	= sizeof(SteamExe);

				if (RegOpenKey(HKEY_CURRENT_USER,"Software\\Valve\\Steam",&hKey) == ERROR_SUCCESS)
				{
					dwType = REG_SZ;

					bool key1 = RegQueryValueEx(hKey, "SteamPath", 0, &dwType, (BYTE*)SteamPath, &SteamPathSize) == ERROR_SUCCESS;
					bool key2 = RegQueryValueEx(hKey, "SteamExe", 0, &dwType, (BYTE*)SteamExe, &SteamExeSize)	 == ERROR_SUCCESS;

					if (key1 && key2)
					{
						/*PROCESS_INFORMATION pi2;
						STARTUPINFO si2; 
						memset(&si2, 0, sizeof(si2));
						memset(&pi2, 0, sizeof(pi2));
						si2.cb = sizeof(si2);
						si2.dwFlags = STARTF_USESHOWWINDOW;
						si2.wShowWindow = SW_SHOW;

						char *cmdLine = new char[strlen(SteamPath) + strlen(lpCmdLine) + 50];
						sprintf(cmdLine, " \"%s\" -applaunch 65790 %s ", SteamPath, lpCmdLine);

						if (addNoMap)
							strcat(cmdLine, "-nomap ");

						regFILE *fd=fopen("fwatch_steam.txt", "a");
						fprintf(fd, "SteamExe:%s\nSteamPath:%s\ncmdLine:%s\n\n", SteamExe, SteamPath, cmdLine);
						fclose(fd);

						CreateProcess(SteamExe, cmdLine, NULL, NULL, false, 0, NULL,NULL,&si2,&pi2);
						CloseHandle(pi2.hProcess);
						CloseHandle(pi2.hThread); 
						delete[] cmdLine;*/

						char *cmdLine = new char[strlen(SteamExe) + strlen(lpCmdLine) + 50];
						sprintf(cmdLine, " \"%s\" -applaunch 65790 %s%s ", SteamExe, addNoMap ? "-nomap " : "", lpCmdLine);

						system(cmdLine);
						delete[] cmdLine;
					};
				};

				RegCloseKey(hKey);

				/*
				PROCESS_INFORMATION pi2;
				STARTUPINFO si2; 
				memset(&si2, 0, sizeof(si2));
				memsetm(&pi2, 0, sizeof(pi2));
				si2.cb = sizeof(si2);
				si2.dwFlags = STARTF_USESHOWWINDOW;
				si2.wShowWindow = SW_SHOW;

				char *cmdLine = new char[strlen(lpCmdLine) + 50];
				sprintf(cmdLine, " c:\\ %s ", lpCmdLine);

				if (addNoMap)
					strcat(cmdLine, "-nomap ");

				CreateProcess("fwatch\\data\\launchsteam.exe", cmdLine, NULL, NULL, false, 0, NULL,NULL,&si2,&pi2);
				CloseHandle(pi2.hProcess);
				CloseHandle(pi2.hThread); 
				delete[] cmdLine;*/
			};

		MessageBox( NULL, "Fwatch is now waiting for you to run the game and/or dedicated server\nWhen you're done playing press OK to exit.", "fwatch", MB_OK|MB_ICONINFORMATION );
	}





	// Remove hook
    RemoveHook();
	Return_Modfolder_Missions(false);
	Return_Modfolder_Missions(true);
	unlink("fwatch_info.sqf");
    return 0;
}













// **** FUNCTIONS ***************************************************************************************


bool runOfp(LPSTR lpCmdLine, bool addNoMap) {
	PROCESS_INFORMATION pi;
    STARTUPINFO si; 
	DWORD dwExitCode = STILL_ACTIVE;

    memset(&si,0,sizeof(si));
    si.cb= sizeof(si);

	//1.13 check if -nomap is there
	int extraSize = 0;
	if (!strstr(lpCmdLine,"-nomap")) 
		extraSize = 7;

	// Duh, need an extra space in the beginning of the command line for it to work
	char *cmdLine = new char[strlen(lpCmdLine)+2+extraSize];
	cmdLine[0] = ' ';
	strcpy(cmdLine+1, lpCmdLine);

	//1.13 add -nomap
	if (extraSize>0 && addNoMap) 
		strcat(cmdLine, " -nomap");


	//1.11 loop launching exe
	int max_loops = sizeof(game_execs) / sizeof(game_execs[0]), 
		ok = 0;

	for (int i=0; i<max_loops; i++) {
		if (strcmp(run_exe_name,"")!=0  &&  strcmpi(game_execs[i],run_exe_name)!=0)
			continue;

		if (CreateProcess(game_execs[i], cmdLine, NULL, NULL, false, 0, NULL,NULL,&si,&pi)) {
			ok = 1;
			break;
		};
	}
	
	delete[] cmdLine;

	if (!ok) 
		return false;

	// Wait until the process has terminated
	while(dwExitCode == STILL_ACTIVE) {
		GetExitCodeProcess(pi.hProcess, &dwExitCode);
		Sleep(1000);
	}

	return true;
}






bool TestHook() {
	HANDLE filu = CreateFile("scripts\\:info version", GENERIC_READ, 0, NULL, OPEN_EXISTING, 128, NULL);
	if(filu == INVALID_HANDLE_VALUE)
		return false;
	
	CloseHandle(filu);
	return true;
}






// v1.11 find process by exename by traversing the list; returns PID number
int findProcess(char* name)
{
	PROCESSENTRY32 processInfo;
	processInfo.dwSize = sizeof(processInfo);
	int pid = 0,
		currPID = GetCurrentProcessId();

	HANDLE processesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if ( processesSnapshot != INVALID_HANDLE_VALUE )
	{
		Process32First(processesSnapshot, &processInfo);
		do
		{
			int tempPID = processInfo.th32ProcessID;
			if (strcmpi(processInfo.szExeFile,name)==0 && tempPID != currPID) {
				pid = tempPID;
				break;
			};
		}
		while (Process32Next(processesSnapshot, &processInfo));

		CloseHandle(processesSnapshot);
	};

	return pid;
};






// v1.11 return number of threads in a process
// http://msdn.microsoft.com/en-us/library/ms686852%28VS.85%29.aspx
int countThreads(unsigned int pid)
{
	int threads = 0;
	THREADENTRY32 te32;
	HANDLE hThreadSnap = INVALID_HANDLE_VALUE;
 
	hThreadSnap = CreateToolhelp32Snapshot( TH32CS_SNAPTHREAD, 0 ); 
	if( hThreadSnap == INVALID_HANDLE_VALUE ) return 0;
	te32.dwSize = sizeof(THREADENTRY32 ); 
 
	if( !Thread32First( hThreadSnap, &te32 ) ) 
	{
		CloseHandle( hThreadSnap );
		return 0;
	};

	do 
		if( te32.th32OwnerProcessID == pid ) 
			threads++;
	while( Thread32Next(hThreadSnap, &te32 ) );

    CloseHandle( hThreadSnap );    
    return threads; 
};







// v1.11 find OFP window by traversing the list; returns PID number
// checks if exe name is OFP
int findOFPwindow()
{
	HWND hwnd = NULL;
	hwnd = GetTopWindow(hwnd);
	if (!hwnd) 
		return 0;

	char windowName[1024];
	DWORD pid=0;

	while(hwnd)
	{
		GetWindowText(hwnd, windowName, 1024);
		
		// If window has game name
		if (strcmpi(windowName, "Operation Flashpoint")==0  ||  strcmpi(windowName, "Cold War Assault")==0  ||  strcmpi(windowName, "ArmA Resistance")==0)
		{
			GetWindowThreadProcessId(hwnd, &pid);


			// Get first module
			HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
			MODULEENTRY32 xModule;
 
			if (hSnap == INVALID_HANDLE_VALUE) return 0;
			xModule.dwSize = sizeof(MODULEENTRY32);
			if (Module32First(hSnap, &xModule) == 0) {
				CloseHandle(hSnap);
				return 0;
			};

			Game_Exe_Address = (int)xModule.modBaseAddr;


			// Check if window exe name is OFP exe
			int max_loops = sizeof(game_execs) / sizeof(game_execs[0]);

			for (int i=0; i<max_loops; i++)
				if (lstrcmpi(xModule.szModule, (LPCTSTR)game_execs[i]) == 0) 
				{
					CloseHandle(hSnap);
					if (strcmpi(windowName, "Operation Flashpoint")==0)
						Game_Version = VER_196;
					else 
						if (strcmpi(windowName, "Cold War Assault")==0)
							Game_Version = VER_199;
						else
							if (strcmpi(windowName, "ArmA Resistance")==0)
								Game_Version = VER_201;
							else
								Game_Version = 0;

					return pid;
				};
		};
		hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
	};

	return 0;
};







// v1.13 searches string with custom length (for skipping middle \0)
char *strstr2(const char *arg1, const char *arg2, int matchWord, int caseSens, unsigned int len)
{
	const char *a, *b;
	bool cond=0;
	unsigned int pos=0;

	// For each letter in arg1
	for(unsigned int i=0 ; i<len; i++, *arg1++, pos++)
	{
		a = arg1;
		b = arg2;
		if (caseSens) cond=*a++ == *b++; else cond=(*a++ | 32) == (*b++ | 32);

		// Run a comparison with arg2
		while (cond)
		{
			// If got to the end
			if (!*b)
			{
				// If matching word - occurrence musn't be surrounded by alphanum chars
				bool LeftEmpty=false, RightEmpty=false;
				if (matchWord)
				{
					if (pos==0) LeftEmpty=true; else if (!isalnum(arg1[-1]) && arg1[-1]!='_') LeftEmpty=true;
					if (pos+strlen(arg2) >= len) RightEmpty=true; else if (!isalnum(arg1[strlen(arg2)]) && arg1[strlen(arg2)]!='_') RightEmpty=true;
				};
			
				// Return pointer to the occurence
				if (!matchWord || matchWord && LeftEmpty && RightEmpty) return ((char *)arg1);
				
				// If failed to match word then move forward
				if (pos+strlen(arg2) < len) arg1+=strlen(arg2), pos+=strlen(arg2);
			};

			if (caseSens) cond=*a++ == *b++; else cond=(*a++ | 32) == (*b++ | 32);
		};
	}

	return(NULL);
};







// v1.14 Recognize if game is displaying error message box
// OFP process has two windows ("operation flashpoint" and "diemwin")
// if the latter is missing then probably error is present
bool IsOfpError(DWORD pid)
{
	int loop=0;
	while (true)
	{
		HWND hwnd = NULL;
		hwnd = GetTopWindow(hwnd);
		if (!hwnd) return true;

		char windowName[24] = "";
		DWORD CurrentPID = 0;

		// find windows with given process id number
		while (hwnd)
		{
			GetWindowText(hwnd, windowName, 24);
			GetWindowThreadProcessId(hwnd, &CurrentPID);	

			if (CurrentPID==pid  &&  strcmpi(windowName,"diemwin")==0)
				return false;
  
			hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
		};

		// when the game starts it may take a while for the "diemwin" to appear
		// loop to make sure
		if (loop<3) 
		{
			loop++;
			Sleep(500);
			continue;
		};

		return true;
	};
};







// v1.14 domain to IP address
unsigned long resolv(char *host) 
{
    struct hostent *hp;
    unsigned long host_ip = inet_addr(host);
    
    if (host_ip == htonl(INADDR_NONE)) 
    {
        hp = gethostbyname(host);
        if (hp) 
            host_ip = *(unsigned long *)(hp->h_addr);
        else 
            return INADDR_NONE;
    };
    
    return host_ip;
};




char* Tokenize(char *string, char *delimiter, int &i, int string_length, bool square_brackets, bool reverse) {
	int delimiter_len = strlen(delimiter);
	bool in_brackets  = false;

	for (int begin=-1;  !reverse && i<=string_length || reverse && i>=-1;  reverse ? i-- : i++) {
		if (square_brackets && string[i] == '[')
			in_brackets = true;

		if (square_brackets && string[i] == ']')
			in_brackets = false;

		bool is_delim = false;
		for (int j=0; j<delimiter_len; j++)
			if (string[i] == delimiter[j] && !in_brackets  ||  delimiter[j]==' ' && isspace(string[i])) {
				is_delim = true;
				break;
			}

		if (begin<0  &&  (!is_delim || (!reverse && i==string_length || reverse && i==-1))) {
			begin = i + (!reverse ? 0 : 1);
		}

		if (begin>=0  &&  (is_delim ||  (!reverse && i==string_length || reverse && i==-1))) {
			string[(!reverse ? i : begin)] = '\0';
			
			if (!reverse && i<string_length)
				i++;
				
			return (!reverse ? string+begin : string+i+1);
		}
	}

	return "";
};




char* Trim(char *txt)
{
	while (isspace(txt[0])) 
		txt++;

	for (int i=strlen(txt)-1;  i>=0 && isspace(txt[i]);  i--) 
		txt[i] = '\0';

	return txt;
};




// Find integer in an integer array
bool IsNumberInArray(int number, int* array, int max_loops)
{
	for (int i=0;  i<max_loops;  i++)
		if (number == array[i])
			return true;

	return false;
};





int Read_Config_Fwatch_HUD(char *filename, bool *no_ar, bool *is_custom, float *custom, int *customINT)
{
	FILE *f = fopen(filename,"r");
	if (f) {
		memset(no_ar    , 0, ARRAY_SIZE*1);
		memset(is_custom, 0, ARRAY_SIZE*1);
		memset(custom   , 0, ARRAY_SIZE*4);
		memset(customINT, 0, ARRAY_SIZE*4);

		fseek(f, 0, SEEK_END);
		int fsize = ftell(f);
		fseek(f, 0, SEEK_SET);

		char *token		 = "";
		char *settings	 = new char[fsize+1];
		int result		 = fread(settings, 1, fsize, f);
		settings[result] = '\0';
		int settings_pos = 0;
		int value_index  = -1;
		
		while (settings_pos < result) {
			char *setting = Tokenize(settings, ";", settings_pos, result, true, false);

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
					int is_int = IsNumberInArray(i,hud_int_list,sizeof(hud_int_list)/sizeof(hud_int_list[0]));
					if (is_int) {
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

		delete[] settings;
		fclose(f);
	}

	return 0;
};

void Transfer_Modfolder_Missions(char *mod, bool server) 
{
	char filename[64] = "fwatch\\data\\sortMissions";
	strcat(filename, server ? "_server.txt" : ".txt");

	FILE *f = fopen(filename,"a");
	if (f) {
		char source[2048]      = "";
		char destination[2048] = "";
		sprintf(source, "%s\\Missions", mod);
		sprintf(destination, "Missions\\%s", mod);

		if (MoveFileEx(source, destination, 0))
			fprintf(f, "%s\n", source);

		WIN32_FIND_DATA fd;
		HANDLE hFind = INVALID_HANDLE_VALUE;
		sprintf(source, "%s\\MPMissions\\*.pbo", mod);
		hFind = FindFirstFile(source, &fd);

		if (hFind != INVALID_HANDLE_VALUE) {
			do {
				sprintf(source, "%s\\MPMissions\\%s", mod, fd.cFileName);
				sprintf(destination, "MPMissions\\%s", fd.cFileName);

				if (MoveFileEx(source, destination, 0))
					fprintf(f, "%s\n", source);
			} 
			while (FindNextFile(hFind, &fd) != 0);
			FindClose(hFind);
		}

		fclose(f);
	}
}

void Return_Modfolder_Missions(bool server) 
{
	char filename[64]      = "fwatch\\data\\sortMissions";
	strcat(filename, server ? "_server.txt" : ".txt");

	FILE *f = fopen(filename,"rb");
	if (f) {
		fseek(f, 0, SEEK_END);
		int file_size = ftell(f);
		fseek(f, 0, SEEK_SET);

		char *text_buffer = (char *) malloc(file_size + 1);
		if (text_buffer == NULL) {
			fclose(f);
			return;
		}

		memset(text_buffer, 0, file_size);

		int result = fread(text_buffer, 1, file_size, f);
		fclose(f);
		if (result != file_size) {
			free(text_buffer);
			return;
		}

		text_buffer[file_size] = '\0';


		int i          = 0;
		int word_start = 0;
		
		while (i < file_size) {
			char *destination = Tokenize(text_buffer, "\r\n", i, file_size, false, false);
			char *mission     = strrchr(destination,'\\');
			bool remove_line  = false;
			
			if (mission != NULL) {
				char source[2048] = "";

				if (strcmpi(mission,"\\missions") == 0) {
					strcpy(source, "missions\\");
					memcpy(source+9, destination, mission - destination);
				} else
					sprintf(source, "mpmissions%s", mission);

				if (MoveFileEx(source, destination, 0))
					remove_line = true;
			} else
				remove_line = true;
			
			if (i>0)
				text_buffer[i-1] = '\r';
				
			if (remove_line) {
				if (text_buffer[i] == '\n') 
					i++;
					
				memcpy(text_buffer+word_start, text_buffer+i, file_size-i);
				
				int len    = i - word_start;
				i         -= len;
				file_size -= len;
			}

			word_start = i;
		}

		text_buffer[file_size] = '\0';
		
		f = fopen(filename, "wb");
		if (f) {
			result = fwrite(text_buffer, 1, file_size, f);
			fclose(f);
		}

		free(text_buffer);
	}
}
// ******************************************************************************************************



















// **** THREADS *****************************************************************************************

//v1.1
// Poke STR_USRACT_CHEAT_1 value to indicate that Fwatch is enabled
void FwatchPresence(void *loop)
{
	bool dedicated_server	 = (int)loop==2;
	int sleep_rate           = 250;
	bool transfered_missions = false;
	int signal_action        = -1;
	char *signal_buffer      = "";
	SIZE_T stBytes           = 0;
	DWORD pid		         = 0;
	DWORD pid_last	         = 0;
	HANDLE phandle;


	while (true) {
		Sleep(sleep_rate);

		// Search for the OFP window
		if (!dedicated_server)
			pid = findOFPwindow();
		else {
			int max_loops = sizeof(server_execs) / sizeof(server_execs[0]);

			for (int i=0; i<max_loops; i++) {
				pid = findProcess(server_execs[i]);	

				if (pid != 0) {
					strcpy(GameServerName, server_execs[i]);
					break;
				}
			}
		}

		// If found
		if (pid != 0) {
			// If a new game window
			if (pid != pid_last) {
				HANDLE phandle = OpenProcess(PROCESS_ALL_ACCESS, 0, pid);

				if (phandle == 0) 
					continue;


				// Change master servers
				int master_base1 = 0;
				int master_base2 = 0;

				switch(Game_Version) {
					case VER_196 : master_base1=0x76EBC0; master_base2=0x775F58; break;
					case VER_199 : master_base1=0x756530; master_base2=0x75D7F0; break;
					case VER_201 : master_base1=Game_Exe_Address+0x6C9298; master_base2=Game_Exe_Address+0x6C9560; break;
				}

				if (!dedicated_server) {
					if (master_base1!=0  &&  strcmp(serv1,"")!=0)
						WriteProcessMemory(phandle, (LPVOID)master_base1, serv1, 64, &stBytes);

					if (master_base1!=0  &&  strcmp(serv2,"")!=0)
						WriteProcessMemory(phandle, (LPVOID)master_base2, serv2, 19, &stBytes);
				};

				ReadProcessMemory(phandle, (LPVOID)master_base1, &master_serv1, 64, &stBytes);
				ReadProcessMemory(phandle, (LPVOID)master_base2, &master_serv2, 19, &stBytes);


				// Find listen server port
				int listen_server_port_address = 0;

				switch(Game_Version) {
					case VER_196 : listen_server_port_address=0x778794; break;
					case VER_199 : listen_server_port_address=0x75F960; break;
					case VER_201 : listen_server_port_address=Game_Exe_Address+0x6C9610; break;
				}

				if (listen_server_port_address != 0)
					ReadProcessMemory(phandle, (LPVOID)listen_server_port_address, &ListenServerPort, 4, &stBytes);


				// Find 'Cheat 1' string and replace it with 'FWATCH'
				bool found     = false;
				int pointer[4] = {0,0,0,0};
				int modif[3]   = {0x434, 0x9C, 0x8};
				int max_loops  = sizeof(pointer) / sizeof(pointer[0]) - 1;
				char buffer[8] = "";

				for (int j=0; j<14 && pid_last==0; j++) {
					switch(Game_Version) {
						case VER_196 : pointer[0]=0x7D5280; break;
						case VER_199 : pointer[0]=0x7C4248; break;
						case VER_201 : pointer[0]=Game_Exe_Address+0x6C9A60; break;
					}

					if (dedicated_server)
						switch(Game_Version) {
							case VER_196 : pointer[0]=0x754D78; break;
							case VER_199 : pointer[0]=0x754E08; break;
							case VER_201 : pointer[0]=Game_Exe_Address+0x5C08A0; break;
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
					};

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

				// If not found then do a general search
				if (!found) {
					for (int current=!dedicated_server ? 0x10000008 : 0x00D00008; current<0x7FFF0000; current+=0x10) {
						ReadProcessMemory(phandle, (LPVOID)current, &buffer, 7, &stBytes);

						if (strcmp(buffer,"Cheat 1")==0) {
							WriteProcessMemory(phandle, (LPVOID)current, "FWATCH", 7, &stBytes);
							break;
						}
					}
				}

				CloseHandle(phandle);

				sleep_rate = 500;
				pid_last   = pid;
			} else {
				// If it's a game that we accessed before
				// Get active modfolders
				if (!transfered_missions) {
					phandle = OpenProcess(PROCESS_ALL_ACCESS, 0, pid);

					if (phandle) {
						HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
							
 						if (hSnap != INVALID_HANDLE_VALUE) {
							MODULEENTRY32 xModule;
							memset (&xModule, 0, sizeof(xModule));
							xModule.dwSize = sizeof(xModule);

							if (Module32First(hSnap, &xModule)) {
								xModule.dwSize = sizeof(MODULEENTRY32);
								int baseOffset = 0;

								char module_name[16] = "ifc22.dll";
								if (dedicated_server)
									strcpy(module_name, "ijl15.dll");

								do {
									if (lstrcmpi(xModule.szModule, (LPCTSTR)module_name) == 0) {
										baseOffset = (int)xModule.modBaseAddr + (!dedicated_server ? 0x2C154 : 0x4FF20);
										break;
									}
								} while(Module32Next(hSnap, &xModule));

								transfered_missions = true;
								char parameters[512] = "";
								ReadProcessMemory(phandle, (LPVOID)baseOffset, &baseOffset, 4, &stBytes);
								ReadProcessMemory(phandle, (LPVOID)baseOffset, &baseOffset, 4, &stBytes);
								ReadProcessMemory(phandle, (LPVOID)baseOffset, &parameters, 512, &stBytes);

								int emptyChars = 0;
								for (int i=0; i<512; i++)
									if (parameters[i] == '\0') {
										emptyChars++;

										if (emptyChars == 2) 
											break;

										parameters[i] = ' ';
									} else 
										emptyChars = 0;

								int parameters_len = strlen(parameters);
								int parameters_pos = parameters_len - 1;

								while (parameters_pos > 0) {
									char *parameter = Tokenize(parameters, " ", parameters_pos, parameters_len, false, true);

									if (strncmp(parameter,"-mod=",5) == 0) {
										parameter += 5;
										int parameter_len = strlen(parameter);
										int parameter_pos = parameter_len - 1;

										while (parameter_pos > 0) {
											char *mod = Tokenize(parameter, ";", parameter_pos, parameter_len, false, true);
											Transfer_Modfolder_Missions(mod, dedicated_server);
										}
									}
								}
							}
						
							CloseHandle(hSnap);
						}

						CloseHandle(phandle);
					}
				}


				// Check if there's a signal to restart the client/server
				char file_name[64] = "fwatch\\data\\fwatch_client_restart.db";

				if (dedicated_server)
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

						if (dedicated_server) {
							strcpy(exe_name, GameServerName);
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
			}
		} else {
			// If game window wasn't found
			if (transfered_missions) {
				Return_Modfolder_Missions(dedicated_server);
				transfered_missions = false;
			}

			sleep_rate = 250;
		}
	}

	_endthread();
}








//v1.14 report listen server to the master server
//Original code by Pulverizer http://pulverizer.pp.fi/ewe/c/OFPReportListenServer.c
void ListenServer(void *loop)
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

	switch(Game_Version) {
		case VER_196 : actualVersion=196; break;
		case VER_199 : actualVersion=199; break;
		case VER_201 : actualVersion=201; break;
	}

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
	while (ListenServerPort < 0)
		Sleep(1000);
	localPort = ListenServerPort + 1;

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
		localPort = ListenServerPort + 1; //update port var
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
			// By default don't allow to report to defunct gamespy unless set by user
			if (strcmpi(master_serv1,"master.gamespy.com")!=0 || strcmp(serv1,"")!=0)
			{	
				remoteAddr.sin_family = AF_INET; // gamespy
				remoteAddr.sin_port = htons(27900);
				remoteAddr.sin_addr.s_addr = resolv(master_serv1);

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

			// By default don't allow to report to defunct all-seeing eye unless set by user
			if (strcmpi(master_serv2,"master.udpsoft.com")!=0 || strcmp(serv2,"")!=0)
			{
				remoteAddr.sin_family = AF_INET; // all seeing eye
				remoteAddr.sin_port = htons(27900);
				remoteAddr.sin_addr.s_addr = resolv(master_serv2);

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
};








//v1.15 modify hardcoded ui elements
void FixUIAspectRatio(void *loop)
{	
	Sleep(100);

	// Write settings file
	FILE *f = fopen("Aspect_Ratio.sqf","w");
	if (!f)
		_endthread();

	fprintf(f, "#include \"Aspect_Ratio.hpp\"\n#ifdef AR_CENTERHUD\nar_center=1;\n#else\nar_center=0;\n#endif\nar_modifx=AR_modifX;ar_modify=AR_modifY;ar_modifx_2ndmon=AR_modifX_2NDMON;true");
	fclose(f);


	// Set up vars to execute program
	TCHAR pwd[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, pwd);

	PROCESS_INFORMATION pi;
	STARTUPINFO si; 
	memset(&si, 0, sizeof(si));
	memset(&pi, 0, sizeof(pi));
	si.cb			= sizeof(si);
	si.dwFlags		= STARTF_USESHOWWINDOW;
	si.wShowWindow	= SW_HIDE;
	char *cmdLine	= new char[strlen(pwd)+64];

	strcpy(cmdLine, "");
	sprintf(cmdLine, "\"%s\" -nolog Aspect_Ratio.sqf Aspect_Ratio.sqf ", pwd);

	// Preprocess settings file
	if (!CreateProcess("fwatch\\data\\preproc.exe", cmdLine, NULL, NULL, false, 0, NULL, NULL, &si, &pi))
		_endthread();



	// Wait for the program to end
	DWORD st;
	do 
	{					
		GetExitCodeProcess(pi.hProcess, &st);
		Sleep(5);
	} 
	while (st == STILL_ACTIVE);

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread); 
	delete[] cmdLine;
	
	
	// Parse settings file
	Sleep(100);
	f = fopen("Aspect_Ratio.sqf","r");
	if (!f)
		_endthread();

	fseek(f, 0, SEEK_END);
	int fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	char *token			= "";
	char *settings		= new char[fsize+1];
	int result			= fread(settings, 1, fsize, f);
	int AR_CENTERHUD	= 0;
	float AR_modifX		= 0;
	float AR_modifY		= 0;
	bool portrait		= 0;

	settings[result] = '\0';
	token			 = strtok(settings, ";\n\t ");

	while (token != NULL)
	{	
		char *eq = strchr(token, '=');

		if (eq != NULL) 
		{
			int pos   = eq - token;
			char *val = token + pos + 1;

			if (strncmp(token,"ar_modifx",pos) == 0) 
				AR_modifX = (float)atof(val),
				portrait  = AR_modifX < 0;

			if (strncmp(token,"ar_modify",pos) == 0) 
				AR_modifY = (float)atof(val);

			if (strncmp(token,"ar_center",pos) == 0) 
				AR_CENTERHUD = atoi(val);
		};

		token = strtok (NULL, ";\n\t ");
	};

	fclose(f);
	Sleep(100);
	delete[] settings;

	// Quit if there are no changes to be made
	if (AR_CENTERHUD)
		_endthread();


	// Variables for process handling
	HANDLE phandle;
	SIZE_T stBytes			= 0;
	DWORD pid				= 0;
	DWORD lastpid			= -1;
	char lastErrorMSG[512]	= "";

	int	max_loops = sizeof(hud_offset) / sizeof(hud_offset[0]);

	float current[ARRAY_SIZE];
	float written[ARRAY_SIZE];
	float custom[ARRAY_SIZE];
	bool no_ar[ARRAY_SIZE];
	bool is_custom[ARRAY_SIZE];

	int currentINT[ARRAY_SIZE];
	int writtenINT[ARRAY_SIZE];
	int customINT[ARRAY_SIZE];


	bool doOnce			= portrait;
	bool moveTankDown	= false;
	float moveTankDownY = 0;
	bool read_mods      = true;
	bool read_base      = true;
	bool first_check    = true;
	int pointer		    = 0;
	int ui_base         = 0;
	int chat_base       = 0;
	int last_chat_state = -1;
	int current_chat_state = 0;
	
	while (true)
	{
		Sleep(500);
		
		// Search for the OFP window ------------------------------
		pid = findOFPwindow();

		if (pid == 0)
			continue;

		if (pid != lastpid) {
			read_base   = true;
			read_mods   = true;
			first_check = true;
			lastpid     = pid;
		}


		// Open process --------------------------------------------
		phandle = OpenProcess(PROCESS_ALL_ACCESS, 0, pid);

		if (phandle == 0) 
			continue;

		// Check error
		if (ErrorLog_Enabled)
		{
			// Base address and offsets
			int pointers[]	 = {0, 0, 0, 0};
			int modif[]		 = {0x68, 0x1C, 0};
			int pointer_size = sizeof(pointers) / sizeof(pointers[0]) - 1;

			switch(Game_Version) {
				case VER_196 : pointers[0]=0x789D88; break;
				case VER_199 : pointers[0]=0x778E80; break;
				case VER_201 : pointers[0]=Game_Exe_Address+0x6D6A10; break;
			}

			// Read values
			for (int i=0; i<pointer_size; i++) {
				ReadProcessMemory(phandle, (LPVOID)pointers[i], &pointers[i+1], 4, &stBytes);
				pointers[i+1] = pointers[i+1] +  modif[i];
			};

			// if went nowhere
			if (pointers[pointer_size]==0) 
				break;
	
			char errorMSG[512] = "";
			ReadProcessMemory(phandle, (LPVOID)pointers[pointer_size], &errorMSG, 512, &stBytes);

			if (strcmp(errorMSG,lastErrorMSG) != 0) {
				strcpy(lastErrorMSG, errorMSG);
				FILE *f1 = fopen("fwatch\\idb\\_errorLog.txt", "a");
				fprintf(f1, "%s\n", errorMSG);
				fclose(f1);
			};
		};

		// Read mods ui settings
		if (read_mods) {
			memset(current    , 0, ARRAY_SIZE*4);
			memset(currentINT , 0, ARRAY_SIZE*4);
			memset(written    , 0, ARRAY_SIZE*4);
			memset(writtenINT , 0, ARRAY_SIZE*4);
			memset(custom     , 0, ARRAY_SIZE*4);
			memset(customINT  , 0, ARRAY_SIZE*4);
			memset(no_ar      , 0, ARRAY_SIZE*1);
			memset(is_custom  , 0, ARRAY_SIZE*1);

			Read_Config_Fwatch_HUD((Game_Version==VER_196 ? "Res\\bin\\config_fwatch_hud.cfg" : "bin\\config_fwatch_hud.cfg"), no_ar, is_custom, custom, customINT);

			HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);

 			if (hSnap != INVALID_HANDLE_VALUE) {
				MODULEENTRY32 xModule;
				memset (&xModule, 0, sizeof(xModule));
				xModule.dwSize = sizeof(xModule);

				if (Module32First(hSnap, &xModule)) {
					xModule.dwSize = sizeof(MODULEENTRY32);
					int baseOffset = 0;

					do {
						if (lstrcmpi(xModule.szModule, (LPCTSTR)"ifc22.dll") == 0) {
							baseOffset = (int)xModule.modBaseAddr + 0x2C154;
							break;
						}
					} while(Module32Next(hSnap, &xModule));

					char parameters[512] = "";
					ReadProcessMemory(phandle, (LPVOID)baseOffset, &baseOffset, 4, &stBytes);
					ReadProcessMemory(phandle, (LPVOID)baseOffset, &baseOffset, 4, &stBytes);
					ReadProcessMemory(phandle, (LPVOID)baseOffset, &parameters, 512, &stBytes);

					int emptyChars = 0;
					for (int i=0; i<512; i++)
						if (parameters[i] == '\0') {
							emptyChars++;

							if (emptyChars == 2) 
								break;

							parameters[i] = ' ';
						} else 
							emptyChars = 0;

					int parameters_len = strlen(parameters);
					int parameters_pos = 0;

					while (parameters_pos < parameters_len) {
						char *parameter = Tokenize(parameters, " ", parameters_pos, parameters_len, false, false);

						if (strncmp(parameter,"-mod=",5) == 0) {
							parameter += 5;
							int parameter_len = strlen(parameter);
							int parameter_pos = 0;

							while (parameter_pos < parameter_len) {
								char *mod = Tokenize(parameter, ";", parameter_pos, parameter_len, false, false);
								char filename[512] = "";
								sprintf(filename, "%s\\bin\\config_fwatch_hud.cfg", mod);
								Read_Config_Fwatch_HUD(filename, no_ar, is_custom, custom, customINT);
							}
						}
					}
				}
	
				CloseHandle(hSnap);
			}

			read_mods = false;
		}


		// Find where UI coordinates are stored
		if (read_base) {
			if (first_check)
				Sleep(1000);
			
			ui_base   = 0;
			chat_base = 0;

			switch(Game_Version) {
				case VER_196 : ui_base=0x79F8D0; break;
				case VER_199 : ui_base=0x78E9C8; break;
				case VER_201 : ui_base=Game_Exe_Address+0x6D8240; break;
			};

			ReadProcessMemory(phandle, (LPVOID)(ui_base+0x0), &ui_base, 4, &stBytes);
			ReadProcessMemory(phandle, (LPVOID)(ui_base+0x8), &ui_base, 4, &stBytes);

			if (ui_base == 0)
				continue;
			else
				read_base = false;

			// Chat has a different address
			switch(Game_Version) {
				case VER_196 : chat_base=0x7831B0; break;
				case VER_199 : chat_base=0x7722A0; break;
				case VER_201 : chat_base=Game_Exe_Address+0x6FFCC0; break;
			};
		}

		// Read position of UI elements
		for (int i=0; i<max_loops; i++) {			
			if (i >= CHAT_X)
				pointer = chat_base;
			else
				pointer = ui_base;

			int is_int = IsNumberInArray(i,hud_int_list,sizeof(hud_int_list)/sizeof(hud_int_list[0]));
			if (is_int)
				ReadProcessMemory(phandle, (LPVOID)(pointer+hud_offset[i]), &currentINT[i], 4, &stBytes);
			else
				ReadProcessMemory(phandle, (LPVOID)(pointer+hud_offset[i]), &current[i], 4, &stBytes);
		}

		if (currentINT[CHAT_ROWS]==4  &&  current[CHAT_X]==0.05f  &&  (current[CHAT_Y]==0.02f || current[CHAT_Y]==0.90f)  &&  current[CHAT_W]==0.9f  &&  current[CHAT_H]==0.02f)
			current_chat_state = CHAT_BRIEFINGLOBBY;
		else 
			if (currentINT[CHAT_ROWS]==6  &&  current[CHAT_X]==0.05f  &&  current[CHAT_Y]==0.68f  &&  current[CHAT_W]==0.7f  &&  current[CHAT_H]==0.02f)
				current_chat_state = CHAT_MISSION;
			else
				if (first_check && currentINT[CHAT_ROWS]==0  &&  current[CHAT_X]==0  &&  current[CHAT_Y]==0  &&  current[CHAT_W]==0  &&  current[CHAT_H]==0)
					continue;
				else
					current_chat_state = CHAT_CUSTOM;




		// Run calculation for portrait mode
		if (doOnce)
		{
			doOnce			= false;
			float tank_x    = current[TANK_X]    + (current[TANK_X]<0.5 ? -1 : 1)    * AR_modifX;
			float tank_y    = current[TANK_Y]    + (current[TANK_Y]<0.5 ? -1 : 1)    * AR_modifY;
			float radar_y	= current[RADAR_Y]   + (current[RADAR_Y]<0.5 ? -1 : 1)   * AR_modifY;
			float compass_y = current[COMPASS_Y] + (current[COMPASS_Y]<0.5 ? -1 : 1) * AR_modifY;	

			if (portrait && 
				tank_x + current[TANK_W] > current[RADAR_X]  &&
				tank_y + current[TANK_H] >= radar_y
				)
				moveTankDown = true,
				moveTankDownY = (radar_y + AR_modifY) + current[RADAR_H];
			else
				if (portrait &&
					tank_x + current[TANK_W] > current[COMPASS_X]  &&
					tank_y + current[TANK_H] >= compass_y
					)
					moveTankDown = true,
					moveTankDownY = (compass_y + AR_modifY) + current[COMPASS_H];
		};



		for (i=0; i<max_loops; i++) {
			int is_int = IsNumberInArray(i,hud_int_list,sizeof(hud_int_list)/sizeof(hud_int_list[0]));

			// Change if it's different from the last one
			//if (first_check  ||  (!is_int && current[i]!=written[i])  ||  (is_int  && currentINT[i]!=writtenINT[i]))
			if (first_check  ||  i>=CHAT_X && current_chat_state!=last_chat_state)
			{			
				if (is_custom[i] && (i<CHAT_X || i>=CHAT_X && current_chat_state==CHAT_MISSION))
					if (is_int)
						currentINT[i] = customINT[i];
					else
						current[i] = custom[i];
				
				if (no_ar[i]) {
					if (is_int)
						writtenINT[i] = currentINT[i];
					else
						written[i] = current[i];
				} else
					switch (i) {
						// move to the left
						case ACTION_X	 :
						case TANK_X		 :	
						case GROUPDIR_X  : written[i] = current[i] + (current[i]<0.5 ? -1 : 1) * AR_modifX; break;

						// move up (but compensate for portrait)
						case RADIOMENU_Y :
						case TANK_Y		 : written[i] = current[i] + (current[i]<0.5 ? -1 : 1) * AR_modifY + moveTankDownY; break;

						// move up/down
						case ACTION_Y	 :
						case LEADER_Y	 :
						case GROUPDIR_Y  :
						case RADAR_Y	 :
						case HINT_Y		 : 
						case COMPASS_Y	 : written[i] = current[i] + (current[i]<0.5 ? -1 : 1) * AR_modifY; break;

						// move left on portrait
						case RADIOMENU_X : 
						{
							if (moveTankDown)
								written[i] = current[i] + (current[i]<0.5 ? -1 : 1) * AR_modifX;
							else
								written[i] = current[i];
						}
						break;

						// extend on widescreen
						case RADIOMENU_W : written[i] = current[i] + (!moveTankDown ? AR_modifX : 0); break; 

						// shrink on portrait
						case LEADER_X : written[i] = current[i] - AR_modifX; break;
						case LEADER_W : written[i] = current[i] + AR_modifX*2; break;
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

						case CHAT_X : {written[i] = current[i] - AR_modifX;} break;
						case CHAT_Y : written[i] = current[i] + (current[i]<0.5 ? -AR_modifY : AR_modifY); break;
						case CHAT_W : 
						{
							if (portrait) {
								if (current[CHAT_X] + current[i] > 1 + AR_modifX*2)
									written[i] = current[i] + AR_modifX*2;
							} else
								written[CHAT_W] = current[CHAT_W] + AR_modifX*2;
						}
						break;

						default: {
							if (is_int)
								writtenINT[i] = currentINT[i];
							else
								written[i] = current[i];								
						}
						break;
							
					};

				if (i >= CHAT_X)
					pointer = chat_base;
				else
					pointer = ui_base;
				
				if (is_int) {
					//fprintf(fd,"%s: 0x%x %d %d\n",hud_names[i],pointer+hud_offset[i],currentINT[i],writtenINT[i]);
					WriteProcessMemory(phandle, (LPVOID)(pointer+hud_offset[i]), &writtenINT[i], 4, &stBytes);
				} else {
					//fprintf(fd,"%s: 0x%x %f %f\n",hud_names[i],pointer+hud_offset[i],current[i],written[i]);
					WriteProcessMemory(phandle, (LPVOID)(pointer+hud_offset[i]), &written[i], 4, &stBytes);
				}
			}
		}

		if (first_check)
			first_check = false;

		if (last_chat_state != current_chat_state)
			last_chat_state = CHAT_CUSTOM;

		CloseHandle(phandle);
	}

	_endthread();
};