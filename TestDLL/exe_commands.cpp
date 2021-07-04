// -----------------------------------------------------------------
// EXTERNAL EXECUTABLES
// -----------------------------------------------------------------

case C_SPIG_EXE:	//TODO: remove this command on release because it's obsolete
{ // Execute program from the SPIG directory

	if (global.is_server) 
		break;

	if (argument_num < 3) {
		QWrite("\"Not enough parameters\""); 
		break;
	}

	if (strstr(argument[2],"..")  ||  strstr(argument[3],"..")) {
		QWrite("\"Illegal argument\""); 
		break;
	}

	// Path to the program
	char program[256] = "Set-Pos-In-Game\\Exe\\";
	strcat(program, stripq(argument[2]));

	// Check if a program exists
	FILE *f = fopen(program, "r");
	if (!f) {
		QWritef("\"Missing %s\"", program);
		break;
	}
	fclose(f);


	// Format argument which will be passed to the program
	StringDynamic buf_arguments;
	StringDynamic_init(buf_arguments);
	StringDynamic_append_format(buf_arguments, "\"%s\" \"fwatch\\mdb\\%s\" -silent", global.mission_path, stripq(argument[3]));

	// Run program
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;
	ZeroMemory(&pi, sizeof(pi));

	if (!CreateProcess(program, buf_arguments.text, NULL, NULL, TRUE, HIGH_PRIORITY_CLASS, NULL, NULL, &si, &pi)) {
		QWritef("\"Failed to execute %s\"", program);
		StringDynamic_end(buf_arguments);
		break;
	}

	StringDynamic_end(buf_arguments);


	// Wait for the program to end
	DWORD st;
	do {					
		GetExitCodeProcess(pi.hProcess, &st);
		Sleep(100);
	} 
	while(st == STILL_ACTIVE);

	CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread); 


	// Pass number returned by a program
	QWritef("[%d]", st);
}
break;












case C_EXE_MAKEPBO:
case C_EXE_ADDONTEST:
case C_EXE_ADDONINSTALL:
case C_EXE_UNPBO:
case C_EXE_WGET:
case C_EXE_PREPROCESS:
case C_RESTART_CLIENT:
case C_RESTART_SERVER:
{ // Execute program from fwatch\data

	// Not enough arguments
	if (argument_num < 3  &&  argument_hash[0]!=C_RESTART_CLIENT) {
		QWrite_err(FWERROR_PARAM_FEW, 2, argument_num, 3);
		QWrite("0,0]");
		break;
	}

	// Abort if command not allowed on the server or client
	if ((global.is_server && (argument_hash[0]==C_RESTART_CLIENT || argument_hash[0]==C_EXE_ADDONTEST)) || (!global.is_server && argument_hash[0]==C_RESTART_SERVER))
		break;



	// Set variables depending on the program
	int db_id         = 0;
	char exe_name[64] = "";

	switch(argument_hash[0]) {
		case C_EXE_ADDONINSTALL : strcpy(exe_name, "addonInstarrer.exe"); break;
		case C_EXE_ADDONTEST    : strcpy(exe_name, "addontest.exe"); break;
		case C_EXE_UNPBO        : strcpy(exe_name, "extractpbo.exe"); break;
		case C_EXE_MAKEPBO      : strcpy(exe_name, "makepbo.exe"); break;
		case C_EXE_WGET         : strcpy(exe_name, "wget.exe"); break;
		case C_EXE_PREPROCESS   : strcpy(exe_name, "preproc.exe"); break;
		case C_RESTART_CLIENT   : strcpy(exe_name, "gameRestart.exe"); break;
	}


	// Optional argument - check if the program is running or terminate it
	bool check = argument_num>2 && strcmpi(argument[2],"check") == 0;
	bool close = argument_num>2 && strcmpi(argument[2],"close") == 0;

	if (check || close) {
		DWORD pid	= 0;
		bool  found	= false;

		if (argument_num > 3) 
			db_id = atoi(argument[3]);
		else {
			QWrite_err(FWERROR_PARAM_FEW, 2, argument_num, 4);
			QWrite("0,0]");
			break;
		}

		// Convert db_id to pid here
		WatchProgramInfo info = db_pid_load(db_id);
		pid = info.pid;

		// If create process failed
		if (info.launch_error != 0) {
			QWrite_err(FWERROR_WINAPI, 1, info.launch_error);
			QWritef("%d,%d]", db_id, info.exit_code);
			break;
		}

		// If thread in fwatch.exe hasn't prepared data yet then quit
		if (pid == 0  ||  info.exit_code == STILL_ACTIVE) {
			QWrite_err(FWERROR_NONE, 0);
			QWritef("%d,%d]", db_id, info.exit_code);
			break;
		}

		// If program is done then return exit code
		if (pid != 0  &&  info.exit_code != STILL_ACTIVE) {
			QWrite_err(FWERROR_NO_PROCESS, 1, exe_name);
			QWritef("%d,%d]", db_id, info.exit_code);
			break;
		}
		
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
				}
			} while(Process32Next(processesSnapshot, &processInfo));

			CloseHandle(processesSnapshot);
		} else {
			QWrite_err(FWERROR_WINAPI, 1, GetLastError());
			QWritef("%d,%d]", db_id, info.exit_code);
			break;
		}
	
		if (close && found) {
			HANDLE AThandle = OpenProcess(PROCESS_ALL_ACCESS, 0, pid);

			if (!TerminateProcess(AThandle,0))
				QWrite_err(FWERROR_WINAPI, 1, GetLastError());

			CloseHandle(AThandle);
		}

		// Return search result
		if (check && !found)
			QWrite_err(FWERROR_NO_PROCESS, 1, exe_name);

		if (~global.option_error_output & OPTION_ERROR_ARRAY_STARTED)
			QWrite_err(FWERROR_NONE, 0);

		QWritef("%d,%d]", db_id, info.exit_code);
		break;
	}
	// ---------


	// Assign id number for this instance so we can find it later
	if (global.external_program_id == 0) {
		srand(time(NULL));
		global.external_program_id = rand() % 10000 + 1;
	}

	db_id = ++global.external_program_id;


	// Get current dir
	TCHAR pwd[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, pwd);

	// Create string that will be passed to the program
	// Starting with directory path (deal with spaces)
	StringDynamic param;
	StringDynamic_init(param);
	StringDynamic_append_format(param, "%d|%d|\"%s", argument_hash[0], db_id, pwd);

	if (argument_hash[0] == C_EXE_WGET)
		StringDynamic_append(param, "\\fwatch\\data");

	if (argument_hash[0] != C_EXE_PREPROCESS)
		StringDynamic_append(param, "\" ");

	bool error = false;

	// Additional options for the extractpbo.exe
	switch(argument_hash[0]) {
		case C_EXE_UNPBO : 
		case C_EXE_MAKEPBO : {
			bool extra_option = argument_num>=5 && ((argument_hash[0]==C_EXE_UNPBO && strcmpi(argument[2],"-F")==0) || (argument_hash[0]==C_EXE_MAKEPBO && strcmpi(argument[2],"-Z")==0));

			StringDynamic buf_filename;
			StringDynamic_init(buf_filename);
			char *ptr_filename = stripq(argument[extra_option ? 4 : 2]);
			VerifyPath(&ptr_filename, buf_filename, OPTION_ALLOW_GAME_ROOT_DIR | OPTION_SUPPRESS_ERROR);

			StringDynamic_append(param, argument_hash[0]==C_EXE_UNPBO ? " -YP " : " -NRK ");

			if (extra_option)
				StringDynamic_append_format(param, "%s %s \"%s\"", argument[2], argument[3], ptr_filename);
			else
				StringDynamic_append_format(param, "\"%s\"", ptr_filename);

			// Destination folder must start with drive
			if (argument_hash[0] == C_EXE_UNPBO) {
				StringDynamic_append_format(param, " \"%s", pwd);

				if ((extra_option && argument_num>=6) || (!extra_option && argument_num>=4)) {
					buf_filename.length = 0;
					ptr_filename        = stripq(argument[extra_option ? 5 : 3]);

					if (VerifyPath(&ptr_filename, buf_filename, OPTION_RESTRICT_TO_MISSION_DIR | OPTION_SUPPRESS_ERROR | OPTION_SUPPRESS_CONVERSION))
						StringDynamic_append_format(param, "\\fwatch\\tmp\\%s\" ", ptr_filename);
					else
						StringDynamic_append(param, "\\fwatch\\tmp\" ");
				} else
					StringDynamic_append(param, "\\fwatch\\tmp\" ");
			} else {
				char fwatch_tmp[]    = "fwatch\\tmp";
				String search_source = {ptr_filename, strlen(ptr_filename)};
				String to_find = {fwatch_tmp, strlen(fwatch_tmp)};

				if (argument_num>=6 || !strstr2(search_source, to_find, OPTION_NONE)) {
					StringDynamic_append(param, " \"fwatch\\tmp");

					if (argument_num >= 6)
						StringDynamic_append_format(param, "\\%s", stripq(argument[5]));

					StringDynamic_append(param, "\"");
				}
			}

			StringDynamic_end(buf_filename);
		}
		break;
		
		case C_EXE_WGET : {
			StringDynamic_append_format(param, "--directory-prefix=fwatch\\tmp --no-check-certificate ");

			for (size_t i=2; i<argument_num; i++) {
				StringDynamic_append_len(param, " ", 1);
				StringDynamic_append_len(param, argument[i], argument_length[i]);
			}
		}
		break;
		
		case C_EXE_PREPROCESS : {
			StringDynamic buf_filename1;
			StringDynamic buf_filename2;
			StringDynamic_init(buf_filename1);
			StringDynamic_init(buf_filename2);

			char *ptr_filename1 = 0;
			char *ptr_filename2 = 0;
			bool merge = false;

			// Check which parameters are files
			for (size_t i=2;  i<argument_num;  i++) {
				if (strcmpi(argument[i],"-merge") == 0) {
					merge = true;
					continue;
				}

				if (strcmpi(argument[i],"-silent") == 0) 
					continue;

				if (!ptr_filename1) {
					ptr_filename1 = argument[i];
					continue;
				}

				if (!ptr_filename2) {
					ptr_filename2 = argument[i];
					continue;
				}
			}

			// Check path
			if (ptr_filename1)
				VerifyPath(&ptr_filename1, buf_filename1, OPTION_ALLOW_GAME_ROOT_DIR | OPTION_SUPPRESS_ERROR);

			if (ptr_filename2) {
				if (!VerifyPath(&ptr_filename2, buf_filename2, OPTION_RESTRICT_TO_MISSION_DIR)) {
					QWrite("0,0]"); 
					StringDynamic_end(buf_filename1);
					StringDynamic_end(buf_filename2);
					error = true;
				}
			}

			// Add starting path
			StringDynamic_append_format(param, "\\%s", ptr_filename1);

			char *lastSlash = strrchr(ptr_filename1, '\\');
			int length      = lastSlash - ptr_filename1 + 1;
			param.length   -= length;

			StringDynamic_append(param, "\" -silent ");

			if (merge)
				StringDynamic_append(param, "-merge "); 

			// Add file to parameter list
			StringDynamic_append_format(param, " \"%s\"", ptr_filename1);

			if (ptr_filename2)
				StringDynamic_append_format(param, " \"%s\"", ptr_filename2);

			StringDynamic_end(buf_filename1);
			StringDynamic_end(buf_filename2);
		}
		break;
		
		default : {
			// Add text that was passed to this command
			for (size_t i=2; i<argument_num; i++) {
				StringDynamic_append_len(param, " ", 1);
				StringDynamic_append_len(param, argument[i], argument_length[i]);
			}

			// Add information about this game instance
			char game_exe_buffer[1024];
			GetModuleFileName(GetModuleHandle(NULL), game_exe_buffer, sizeof(game_exe_buffer));
			char *game_exe_ptr = strrchr(game_exe_buffer, '\\');

			if (game_exe_ptr)
				game_exe_ptr += 1;
			else
				game_exe_ptr = game_exe_buffer;

			StringDynamic_append_format(param, " -pid=%d \"-run=%s\"", global.pid, game_exe_ptr);
		}
		break;
	}
	
	if (!error) {
		// Send a message to the fwatch.exe
		HANDLE mailslot = CreateFile(TEXT("\\\\.\\mailslot\\fwatch_mailslot"), GENERIC_WRITE, FILE_SHARE_READ, (LPSECURITY_ATTRIBUTES)NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);
		
		if (mailslot != INVALID_HANDLE_VALUE) {
			DWORD bytes_written = 0;

			if (WriteFile(mailslot, param.text, param.length+1, &bytes_written, (LPOVERLAPPED)NULL)) {
				QWrite_err(FWERROR_NONE, 0);
				QWritef("%d,%d]", db_id, STILL_ACTIVE);
			} else {
				QWrite_err(FWERROR_WINAPI, 1, GetLastError());
				QWrite("0,0]");
			}

			CloseHandle(mailslot);
		} else {
			QWrite_err(FWERROR_WINAPI, 1, GetLastError());
			QWrite("0,0]");
		}
	}

	StringDynamic_end(param);
} 
break;












case C_EXE_WEBSITE:		
{ // open url in a browser

	if (argument_num < 2 || global.is_server)
		break;

	// Is it main menu
	int	is_main_menu_address = 0;
	int is_main_menu         = 0;

	switch(global_exe_version[global.exe_index]) {
		case VER_196 : is_main_menu_address=0x75E254; break;
		case VER_199 : is_main_menu_address=0x74B58C; break;
	}

	if (is_main_menu_address) {
		ReadProcessMemory(phandle, (LPVOID)is_main_menu_address, &is_main_menu, 4, &stBytes);

		if (!is_main_menu) 
			break;
	}

	if (
		strncmpi(argument[2],"https://",8) != 0  &&
		strncmpi(argument[2],"http://",7)  != 0  &&
		strncmpi(argument[2],"ftp://",6)   != 0  &&
		strncmpi(argument[2],"www.",4)     != 0
	) break;

	ShellExecute(NULL, "open", argument[2], NULL, NULL, SW_SHOWNORMAL);
}
break;