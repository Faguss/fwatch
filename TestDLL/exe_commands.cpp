// -----------------------------------------------------------------
// EXTERNAL EXECUTABLES
// -----------------------------------------------------------------

case C_EXE_SPIG:
{ // Execute program from the SPIG directory

	if (DedicatedServer) 
		break;

	if (numP < 3) 
	{
		QWrite("\"Not enough parameters\"", out); 
		break;
	};

	par[2] = stripq(par[2]);

	if (isLeavingDir(par[2],0,RESTRICT_TO_MISSION_DIR,CommandID,NULL)) 
	{
		QWrite("\"Illegal program path\"",out); 
		break;
	}


	// Path to the program
	char program[1024] = "Set-Pos-In-Game\\Exe\\";
	strcat(program, par[2]);

	// Check if a program exists
	FILE *f = fopen(program, "r");
	if (!f) 
	{
		QWrite("\"Missing ",out);
		QWrite(program, out);
		QWrite("\"", out); 
		break;
	};
	fclose(f);


	// Get mission directory
	int pathLen = 128;
	char *path	= (char*) malloc (pathLen);

	if (path==NULL) 
	{
		QWrite("\"Couldn't allocate mem for the mission path\"",out); 
		free(program); 
		break;
	};

	strcpy(path,"");

	if (!getMissionDir(&path,pathLen,CommandID,NULL)) 
	{
		QWrite("\"", out); 
		QWrite(path, out); 
		QWrite("\"", out); 
		free(path); 
		break;
	};


	// Format argument which will be passed to the program
	int argumentsLen = pathLen + 30;
	char *arguments	 = (char*) malloc(argumentsLen);

	if (arguments == NULL) 
	{
		QWrite("Couldn't reallocate mem for the 2nd argument",out); 
		free(path); 
		break;
	};

	strcpy(arguments, "");
	par[3] = stripq(par[3]);
	sprintf(arguments, "\"%s\" \"fwatch\\mdb\\%s\" -silent", path,par[3]);


	// Run program
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;
	ZeroMemory(&pi, sizeof(pi));

	int ok = CreateProcess(program, arguments, NULL, NULL, TRUE, HIGH_PRIORITY_CLASS, NULL, NULL, &si, &pi);
	if (!ok) 
	{
		QWrite("\"Failed to execute ",out);
		QWrite(program, out);
		QWrite("\"", out);
		break;
	};


	// Wait for the program to end
	DWORD st;
	do
	{					
		GetExitCodeProcess(pi.hProcess, &st);
		Sleep(100);
	} 
	while (st == STILL_ACTIVE);

	CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread); 


	// Pass number returned by a program
	char tmp[8] = "";
	sprintf(tmp, "[%d]", st);
	QWrite(tmp, out);


	free(arguments);
	free(path);
}
break;












case C_RESTART_SERVER:		
{ // signal fwatch.exe thread to restart server

	if (!DedicatedServer) 
		break;
	
	FILE *fp = fopen("fwatch\\data\\fwatch_server_restart.db", "w");

	if (fp) 
		fprintf(fp, " %s", com+15), 
		fclose(fp);
}
break;












/*case C_RESTART_CLIENT:		
{ // execute 3rd party program to restart game

	if (DedicatedServer) 
		break;


	// Allocate buffer
	int paramLen	= MAX_PATH + strlen(com) + 30;
	char *param		= (char*) malloc (paramLen);

	if (param==NULL) 
	{
		FWerror(10,0,CommandID,"param","",paramLen,0,out);
		break;
	};

	strcpy(param,"");


	// Working dir
	TCHAR pwd[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, pwd);


	// Format argument which will be passed to the program
	sprintf(param, "\"%s\" -pid=%d -cwa=%d ", pwd, pid, CWA);

	if (strlen(com) > 15) 
		strcat(param, com+15);


	// Run program
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;
	ZeroMemory(&pi, sizeof(pi));

	if (CreateProcess("fwatch\\data\\gameRestart.exe", param, NULL, NULL, TRUE, HIGH_PRIORITY_CLASS, NULL, NULL, &si, &pi))
		FWerror(0,0,CommandID,"","",0,0,out);
	else
		FWerror(5,GetLastError(),CommandID,"","",0,0,out);

	CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
	free(param);
}
break;*/












case C_EXE_MAKEPBO:
case C_EXE_ADDONTEST:
case C_EXE_ADDONINSTALL:
case C_EXE_UNPBO:
case C_EXE_WGET:
case C_EXE_PREPROCESS:
case C_RESTART_CLIENT:
{ // Execute a program from the fwatch\data\

	// Not enough arguments
	if (numP < 3  &&  CommandID!=C_RESTART_CLIENT) {
		FWerror(100,0,CommandID,"","",numP,3,out);
		QWrite("0]", out);
		break;
	};

	// Not allowed on the server
	if (DedicatedServer  &&  CommandID==C_RESTART_CLIENT  &&  CommandID==C_EXE_ADDONTEST) 
		break;
	

	// Set variables depending on the program
	char exe_name[64] = "";
	char exe_path[64] = "fwatch\\data\\";

	if (CommandID == C_EXE_ADDONINSTALL)
		strcpy(exe_name, "addonInstaller.exe");

	if (CommandID == C_EXE_ADDONTEST) 
		strcpy(exe_name, "addontest.exe"), 
		strcpy(exe_path, "@addontest\\ModData\\");

	if (CommandID == C_EXE_UNPBO) 
		strcpy(exe_name, "extractpbo.exe");

	if (CommandID == C_EXE_MAKEPBO)
		strcpy(exe_name, "makepbo.exe");

	if (CommandID == C_EXE_WGET) 
		strcpy(exe_name, "wget.exe");

	if (CommandID == C_EXE_PREPROCESS) 
		strcpy(exe_name, "preproc.exe");

	if (CommandID == C_RESTART_CLIENT)
		strcpy(exe_name, "gameRestart.exe");

	strcat(exe_path, exe_name);
	
	
	// Optional argument - check if the program is running or terminate it
	bool check = numP>2 && strcmpi(par[2],"check") == 0;
	bool close = numP>2 && strcmpi(par[2],"close") == 0;

	if (check || close) {
		DWORD pid	= 0;
		bool  found	= false;

		if (numP > 3) 
			pid = atoi(par[3]);
		else {
			FWerror(100,0,CommandID,"","",numP,4,out);
			QWrite("0]", out);
			break;
		};
		//int pid = findProcess(exe_name);
		
		// Search for the process
		PROCESSENTRY32 processInfo;
		processInfo.dwSize       = sizeof(processInfo);
		HANDLE processesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

		if (processesSnapshot != INVALID_HANDLE_VALUE) {
			Process32First(processesSnapshot, &processInfo);
			do {
				if (processInfo.th32ProcessID != pid) 
					continue;

				if (strcmpi(processInfo.szExeFile,exe_name) == 0) {
					found = true; 
					break;
				};
			} while(Process32Next(processesSnapshot, &processInfo));
			CloseHandle(processesSnapshot);
		} else {
			FWerror(5,GetLastError(),CommandID,"","",0,0,out),
			QWrite("0]", out);
			break;
		};

		// Terminate process
		int primary_error   = 0;
		int secondary_error = 0;
		char *string_error  = "";
		
		if (close)
			if (found) {
				HANDLE AThandle = OpenProcess(PROCESS_ALL_ACCESS,0,pid);

				if (!TerminateProcess(AThandle,0)) {
					primary_error   = 5;
					secondary_error = GetLastError();
				}

				CloseHandle(AThandle);
			}

		// Return search result
		if (check && !found) {
			primary_error = 2;
			string_error  = exe_name;
		}

		FWerror(primary_error,secondary_error,CommandID,string_error,"",0,0,out);
		QWrite("0]", out);
		break;
	}
	// ---------

	
	// Get current dir
	TCHAR pwd[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, pwd);
	String param;
	String_init(param);

	// Create string that will be passed to the program
	// Starting with directory path (deal with spaces)
	String_append(param, "\"");
	String_append(param, pwd);

	if (CommandID == C_EXE_WGET)
		String_append(param, "\\fwatch\\data");

	String_append(param, "\" ");
	bool error = false;

	// Additional options for the extractpbo.exe
	switch(CommandID) {
		case C_EXE_UNPBO : 
		case C_EXE_MAKEPBO : {
			bool extra_option = numP>=5 && (CommandID==C_EXE_UNPBO && strcmpi(par[2],"-F")==0 || CommandID==C_EXE_MAKEPBO && strcmpi(par[2],"-Z")==0);

			String buf_filename;
			String_init(buf_filename);
			char *ptr_filename = stripq(par[extra_option ? 4 : 2]);

			if (isAllowedExternalPath(ptr_filename,ALLOW_GAME_ROOT_DIR)) {
				ptr_filename = ptr_filename + 3;
			} else {
				String_append(buf_filename, MissionPath);
				String_append(buf_filename, ptr_filename);
				ptr_filename = buf_filename.pointer;
			}
			
			String_append(param, CommandID==C_EXE_UNPBO ? " -YP " : " -NRK ");

			if (extra_option) {
				String_append(param, par[2]);
				String_append(param, " ");
				String_append(param, par[3]);
				String_append(param, " \"");
				String_append(param, ptr_filename);
				String_append(param, "\"");
			} else {
				String_append(param, "\"");
				String_append(param, ptr_filename);
				String_append(param, "\"");
			}

			// Destination folder must start with drive
			if (CommandID == C_EXE_UNPBO) {
				String_append(param, " \"");
				String_append(param, pwd);
				String_append(param, "\\fwatch\\tmp\" ");
			} else {
				if (numP>=6 || !strstr2(ptr_filename,"fwatch\\tmp",0,0)) {
					String_append(param, " \"fwatch\\tmp");

					if (numP >= 6) {
						String_append(param, "\\");
						String_append(param, stripq(par[5]));
					};

					String_append(param, "\"");
				}
			}

			String_end(buf_filename);
		}; 
		break;
		
		case C_EXE_WGET : {
			String_append(param, "--directory-prefix=fwatch\\tmp ");
			String_append(param, com+9);
		};
		break;
		
		case C_EXE_PREPROCESS : {
			String_append(param, "-silent ");

			String buf_filename1;
			String buf_filename2;
			String_init(buf_filename1);
			String_init(buf_filename2);
			char *ptr_filename1 = 0;
			char *ptr_filename2 = 0;

			// Check which parameters are files
			for (int i=2;  i<numP;  i++) {
				if (strcmpi(par[i],"-merge") == 0) {
					String_append(param, "-merge "); 
					continue;
				}

				if (strcmpi(par[i],"-silent") == 0) 
					continue;

				if (!ptr_filename1) {
					ptr_filename1 = par[i];
					continue;
				}

				if (!ptr_filename2) {
					ptr_filename2 = par[i];
					continue;
				}
			}

			// Check path
			if (ptr_filename1)
				if (isAllowedExternalPath(ptr_filename1,ALLOW_GAME_ROOT_DIR))
					ptr_filename1 = ptr_filename1 + 3;
				else {
					String_append(buf_filename1, MissionPath);
					String_append(buf_filename1, ptr_filename1);
					ptr_filename1 = buf_filename1.pointer;
				}

			if (ptr_filename2) {
				if (isLeavingDir(ptr_filename2,0,RESTRICT_TO_MISSION_DIR,CommandID,out)) {
					QWrite("0]", out); 
					String_end(buf_filename1);
					String_end(buf_filename2);
					String_end(param);
					error = true;
					break;
				}

				if (isAllowedExternalPath(ptr_filename2,ALLOW_GAME_ROOT_DIR))
					ptr_filename2 = ptr_filename2 + 3;
				else {
					String_append(buf_filename2, MissionPath);
					String_append(buf_filename2, ptr_filename2);
					ptr_filename2 = buf_filename2.pointer;
				}
			}

			// Add file to parameter list
			String_append(param, " \"");
			String_append(param, ptr_filename1);
			String_append(param, "\"");

			if (ptr_filename2) {
				String_append(param, " \"");
				String_append(param, ptr_filename2);
				String_append(param, "\"");
			};

			String_end(buf_filename1);
			String_end(buf_filename2);
		};
		break;
		
		default : {
			// Add text that was passed to this command
			if (numP > 2)
				String_append(param, com + (strlen(par[0]) + strlen(par[1]) + 1));

			// Add information about this game instance
			char game_exe_buffer[1024];
			GetModuleFileName( GetModuleHandle( NULL ), game_exe_buffer, sizeof(game_exe_buffer) );
			char *game_exe_ptr = strrchr(game_exe_buffer,'\\');

			if (game_exe_ptr != NULL)
				game_exe_ptr += 1;
			else
				game_exe_ptr = game_exe_buffer;

			String_append(param, " -pid=");
			char tmp[64] = "";
			sprintf(tmp, "%d \"-run=", pid);
			String_append(param, tmp);
			String_append(param, game_exe_ptr);
			String_append(param, "\"");
		};
		break;
	};
	
	if (error)
		break;

	if (CommandID == C_RESTART_CLIENT) {
		FILE *f = fopen("fwatch\\data\\fwatch_client_restart.db", "a");
		//fprintf(f, " %s ", exe_path);
		fprintf(f, param.pointer);
		fclose(f);
		String_end(param);
		FWerror(0,0,CommandID,"","",0,0,out);
		QWrite("0]", out);
		break;
	}



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
	
	if (CreateProcess(exe_path, param.pointer, NULL, NULL, TRUE, HIGH_PRIORITY_CLASS, NULL, NULL, &si, &pi)) {
		FWerror(0,0,CommandID,"","",0,0,out);

		DWORD dwPid	 = pi.dwProcessId;
		char tmp[13] = "";

		sprintf(tmp, "%d]", dwPid);
		QWrite(tmp, out);

		// Run another program monitoring the first program
		if (CommandID==C_EXE_UNPBO  ||  CommandID==C_EXE_WGET) {
			// Remove temporary file
			char filename[128] = "";
			sprintf(filename, "fwatch\\tmp\\%d.pid", pid);
			unlink(filename);

			// Add process id number of the first program
			param.current_length = 0;
			String_append(param, pwd);
			String_append(param, " -pid=");
			char tmp[32] = "";
			sprintf(tmp, "%d ", pi.dwProcessId);
			String_append(param, tmp);

			// Launch
			STARTUPINFO si2;
			PROCESS_INFORMATION pi2;
			ZeroMemory(&si2, sizeof(si2));
			si2.cb          = sizeof(si2);
			si2.dwFlags     = STARTF_USESHOWWINDOW;
			si2.wShowWindow = SW_HIDE;
			ZeroMemory(&pi2, sizeof(pi2));
			CreateProcess("fwatch\\data\\getExitCode.exe", param.pointer, NULL, NULL, TRUE, HIGH_PRIORITY_CLASS, NULL, NULL, &si2, &pi2);
			CloseHandle(pi2.hProcess);
			CloseHandle(pi2.hThread);
		}
	} else {
		FWerror(5,GetLastError(),CommandID,"","",0,0,out);
		QWrite("0]", out);
	}

	CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
	CloseHandle(logFile);
	String_end(param);
};
break;












case C_EXE_WEBSITE:		
{ // open url in a browser

	if (numP < 2)
		break;

	if (DedicatedServer)
		break;


	// Is it main menu
	int	inMenuOff = 0;
	int inMenu    = 0;

	switch(Game_Version) {
		case VER_196 : inMenuOff=0x75E254; break;
		case VER_199 : inMenuOff=0x74B58C; break;
	}

	if (inMenuOff != 0) {
		ReadProcessMemory(phandle, (LPVOID)inMenuOff, &inMenu, 4, &stBytes);
		if (!inMenu) 
			break;
	}

	char *url = com + 12;

	if 
	(
		strncmpi(url,"https://",8) != 0  && 	
		strncmpi(url,"http://",7)  != 0  &&   
		strncmpi(url,"ftp://",6)   != 0  &&  
		strncmpi(url,"www.",4)     != 0
	)
	{
		break;
	};

	ShellExecute(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);
}
break;