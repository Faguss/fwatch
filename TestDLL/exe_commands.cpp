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

	if (strstr(argument[2].text,"..")  ||  strstr(argument[3].text,"..")) {
		QWrite("\"Illegal argument\""); 
		break;
	}

	// Path to the program
	char program[256] = "Set-Pos-In-Game\\Exe\\";
	strcat(program, String_trim_quotes(argument[2]));

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
	StringDynamic_appendf(buf_arguments, "\"%s\" \"fwatch\\mdb\\%s\" -silent", global.mission_path, String_trim_quotes(argument[3]));

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
	bool check = argument_num>2 && strcmpi(argument[2].text,"check") == 0;
	bool close = argument_num>2 && strcmpi(argument[2].text,"close") == 0;

	if (check || close) {
		if (argument_num > 3) 
			db_id = atoi(argument[3].text);
		else {
			QWrite_err(FWERROR_PARAM_FEW, 2, argument_num, 4);
			QWrite("0,0]");
			break;
		}

		// Convert db_id to pid here
		WatchProgramInfo info = db_pid_load(db_id);

		if (info.db_id > 0) {
			// If create process failed
			if (info.launch_error != 0) {
				QWrite_err(FWERROR_WINAPI, 1, info.launch_error);
				QWritef("%d,%d]", db_id, info.exit_code);
				break;
			}

			// If thread in fwatch.exe hasn't prepared data yet then quit
			if (info.pid == 0) {
				if (check) {
					QWrite_err(FWERROR_NONE, 0);
					QWritef("%d,%d]", db_id, info.exit_code);
					break;
				} else
					if (close) {
						QWrite_err(FWERROR_NO_PROCESS, 1, exe_name);
						QWritef("%d,%d]", db_id, info.exit_code);
						break;
					}
			}

			// If program is done then return exit code
			if (info.exit_code != STILL_ACTIVE) {
				if (check) {
					QWrite_err(FWERROR_NO_PROCESS, 1, exe_name);
					QWritef("%d,%d]", db_id, info.exit_code);
					break;
				} else
					if (close) {
						QWrite_err(FWERROR_NONE, 0);
						QWritef("%d,%d]", db_id, info.exit_code);
						break;
					}
			}

			// If program is running
			if (check) {
				QWrite_err(FWERROR_NONE, 0);
				QWritef("%d,%d]", db_id, info.exit_code);
				break;
			}
		} else {
			QWrite_err(FWERROR_NO_PROCESS, 1, exe_name);
			QWritef("%d,%d]", db_id, info.exit_code);
			break;
		}

		
		// Search for the process
		PROCESSENTRY32 processInfo;
		processInfo.dwSize       = sizeof(processInfo);
		HANDLE processesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
		bool found               = false;

		if (processesSnapshot != INVALID_HANDLE_VALUE) {
			Process32First(processesSnapshot, &processInfo);

			do {
				if (processInfo.th32ProcessID != info.pid) 
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
			HANDLE AThandle = OpenProcess(PROCESS_ALL_ACCESS, 0, info.pid);

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


	// Assign id number for this instance so we can find it later
	if (global.external_program_id == 0) {
		srand(time(NULL));
		global.external_program_id = rand() % 10000 + 1;
	}

	db_id = ++global.external_program_id;


	// Create string that will be passed to the program
	// Starting with directory path (deal with spaces)
	StringDynamic param;
	StringDynamic_init(param);
	StringDynamic_appendf(param, "%d|%d|\"%s", argument_hash[0], db_id, global.game_dir);

	if (argument_hash[0] == C_EXE_WGET)
		StringDynamic_append(param, "\\fwatch\\data");

	StringDynamic_append(param, "\" ");

	bool error = false;

	// Additional options for the extractpbo.exe
	switch(argument_hash[0]) {
		case C_EXE_UNPBO : 
		case C_EXE_MAKEPBO : {
			bool extra_option = argument_num>=5 && ((argument_hash[0]==C_EXE_UNPBO && strcmpi(argument[2].text,"-F")==0) || (argument_hash[0]==C_EXE_MAKEPBO && strcmpi(argument[2].text,"-Z")==0));

			StringDynamic buf_filename;
			StringDynamic_init(buf_filename);
			size_t arg_filename = extra_option ? 4 : 2;

			String_trim_quotes(argument[arg_filename]);
			VerifyPath(argument[arg_filename], buf_filename, OPTION_ALLOW_MOST_LOCATIONS | OPTION_SUPPRESS_ERROR);
			StringDynamic_append(param, argument_hash[0]==C_EXE_UNPBO ? " -YP " : " -NRK ");

			if (extra_option)
				StringDynamic_appendf(param, "%s %s \"%s\"", argument[2].text, argument[3].text, argument[arg_filename].text);
			else
				StringDynamic_appendf(param, "\"%s\"", argument[arg_filename].text);

			// Destination folder must start with drive
			if (argument_hash[0] == C_EXE_UNPBO) {
				StringDynamic_appendf(param, " \"%s", global.game_dir);

				if ((extra_option && argument_num>=6) || (!extra_option && argument_num>=4)) {
					buf_filename.length = 0;
					arg_filename        = extra_option ? 5 : 3;

					String_trim_quotes(argument[arg_filename]);

					if (VerifyPath(argument[arg_filename], buf_filename, OPTION_LIMIT_WRITE_LOCATIONS | OPTION_SUPPRESS_ERROR | OPTION_SUPPRESS_CONVERSION))
						StringDynamic_appendf(param, "\\fwatch\\tmp\\%s\" ", argument[arg_filename].text);
					else
						StringDynamic_append(param, "\\fwatch\\tmp\" ");
				} else
					StringDynamic_append(param, "\\fwatch\\tmp\" ");
			} else {
				char fwatch_tmp[] = "fwatch\\tmp";
				String to_find    = {fwatch_tmp, 10};

				if (argument_num>=6  ||  String_find(argument[arg_filename], to_find, OPTION_NONE)==0) {
					StringDynamic_append(param, " \"fwatch\\tmp");

					if (argument_num >= 6)
						StringDynamic_appendf(param, "\\%s", String_trim_quotes(argument[5]));

					StringDynamic_append(param, "\"");
				}
			}

			StringDynamic_end(buf_filename);
		}
		break;
		
		case C_EXE_WGET : {
			StringDynamic_appendf(param, "--directory-prefix=fwatch\\tmp --no-check-certificate --user-agent=\"Mozilla/5.0 (Windows NT 6.1; Win64; x64; rv:103.0) Gecko/20100101 Firefox/103.0\" ");

			for (size_t i=2; i<argument_num; i++) {
				StringDynamic_appendl(param, " ", 1);
				StringDynamic_appends(param, argument[i]);
			}
		}
		break;
		
		case C_EXE_PREPROCESS : {
			String input_file  = empty_string;
			String output_file = empty_string;

			StringDynamic_append(param, " -fwatch ");

			// Find which arguments are files and convert paths
			for (size_t i=2;  i<argument_num;  i++) {
				if (strncmp(argument[i].text,"-addondir=",10) == 0) {
					StringDynamic buf_filename;
					StringDynamic_init(buf_filename);
					String addondir = {argument[i].text+10, argument[i].length-10};
					VerifyPath(addondir, buf_filename, OPTION_ALLOW_MOST_LOCATIONS | OPTION_SUPPRESS_ERROR);
					StringDynamic_appendf(param, "\"-addondir=%s\" ", addondir);
					StringDynamic_end(buf_filename);
					continue;
				}

				if (strncmp(argument[i].text,"-gamedir=",9)==0  ||  strcmp(argument[i].text,"-fwatch")==0) {
					continue;
				}

				if (strcmpi(argument[i].text,"-merge") == 0) {
					StringDynamic_append(param, "-merge ");
					continue;
				}

				if (strncmp(argument[i].text,"-out=",5) == 0) {
					if (output_file.length == 0) {
						output_file.text   = argument[i].text+5;
						output_file.length = argument[i].length-5;
						StringDynamic buf_filename;
						StringDynamic_init(buf_filename);

						if (VerifyPath(output_file, buf_filename, OPTION_LIMIT_WRITE_LOCATIONS))
							StringDynamic_appendf(param, "\"-out=%s\" ", output_file.text);
						else {
							QWrite("0,0]"); 
							error = true;
						}
							
						StringDynamic_end(buf_filename);
					}
					continue;
				}

				if (input_file.length == 0) {
					StringDynamic buf_filename;
					StringDynamic_init(buf_filename);
					input_file = argument[i];
					VerifyPath(input_file, buf_filename, OPTION_ALLOW_MOST_LOCATIONS | OPTION_SUPPRESS_ERROR);
					StringDynamic_appendf(param, "\"%s\" ", input_file.text);
					StringDynamic_end(buf_filename);
					continue;
				}
			}
		}
		break;
		
		default : {
			// Add text that was passed to this command
			for (size_t i=2; i<argument_num; i++) {
				StringDynamic_appendl(param, " ", 1);
				StringDynamic_appends(param, argument[i]);
			}

			// Add information about this game instance
			char game_exe_buffer[1024];
			GetModuleFileName(GetModuleHandle(NULL), game_exe_buffer, sizeof(game_exe_buffer));
			char *game_exe_ptr = strrchr(game_exe_buffer, '\\');

			if (game_exe_ptr)
				game_exe_ptr += 1;
			else
				game_exe_ptr = game_exe_buffer;

			StringDynamic_appendf(param, " -pid=%d \"-run=%s\"", global.pid, game_exe_ptr);
		}
		break;
	}
	
	if (!error) {
		// Send a message to the fwatch.exe
		HANDLE mailslot = CreateFile(TEXT("\\\\.\\mailslot\\fwatch_mailslot"), GENERIC_WRITE, FILE_SHARE_READ, (LPSECURITY_ATTRIBUTES)NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);
		
		if (mailslot != INVALID_HANDLE_VALUE) {
			WatchProgramInfo info = {db_id, 0, 0, 0};
			db_pid_save(info);

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
{ // Open url in a browser

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
		strncmpi(argument[2].text,"https://",8) != 0  &&
		strncmpi(argument[2].text,"http://",7)  != 0  &&
		strncmpi(argument[2].text,"ftp://",6)   != 0  &&
		strncmpi(argument[2].text,"www.",4)     != 0
	) break;

	ShellExecute(NULL, "open", argument[2].text, NULL, NULL, SW_SHOWNORMAL);
}
break;












case C_RESTART_SCHEDULE:		
{ // Add task to the Windows Task Scheduler

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

	enum RESTART_SCHEDULE_MODE {
		MODE_CREATE_TASK,
		MODE_DELETE_TASK,
		MODE_LIST_TASKS
	};

	const char modes[][8] = {
		"create",
		"delete",
		"list"
	};

	size_t arg_eventid    = empty_char_index;
	size_t arg_parameters = empty_char_index;
	size_t arg_comment    = empty_char_index;
	size_t arg_date       = empty_char_index;
	int arg_recurrence    = TASK_TIME_TRIGGER_ONCE;
	int arg_mode          = MODE_CREATE_TASK;

	for (size_t i=2; i<argument_num; i+=2) {
		switch (argument_hash[i]) {
			case NAMED_ARG_EVENTID:
				arg_eventid = i + 1;
				break;

			case NAMED_ARG_PARAMETERS:
				arg_parameters = i + 1;
				break;

			case NAMED_ARG_COMMENT:
				arg_comment = i + 1;
				break;

			case NAMED_ARG_DATE:
				arg_date = i + 1;
				break;

			case NAMED_ARG_RECURRENCE:
				arg_recurrence = atoi(argument[i+1].text);
				break;

			case NAMED_ARG_MODE:
				for (int j=0; j<(sizeof(modes) / sizeof(modes[0])); j++)
					if (strcmpi(argument[i+1].text,modes[j]) == 0)
						arg_mode = j;
				break;
		}
	}

	switch(arg_mode) {
		case MODE_CREATE_TASK : {
			if (argument[arg_eventid].length == 0) {
				QWrite_err(FWERROR_PARAM_EMPTY, 1, "arg_eventid");
				QWrite("[]]");
				break;
			}

			if (argument[arg_date].length == 0) {
				QWrite_err(FWERROR_PARAM_EMPTY, 1, "arg_date");
				QWrite("[]]");
				break;
			}

			// Task name
			const int taskname_size         = 32;
			wchar_t taskname[taskname_size] = L"OFP_GS_";
			MultiByteToWideChar(CP_UTF8, 0, argument[arg_eventid].text, argument[arg_eventid].length, taskname+wcslen(taskname), taskname_size-wcslen(taskname));

			// Path to exe
			wchar_t appname[MAX_PATH] = L"";
			GetCurrentDirectoryW(MAX_PATH, appname);
			wchar_t gameRestart[] = L"\\fwatch\\data\\gameRestart.exe";

			if (wcslen(appname) + wcslen(gameRestart) + 1 < MAX_PATH)
				wcscat(appname, gameRestart);

			// Exe arguments
			size_t params_size = (argument[arg_parameters].length + 128) * 2;
			wchar_t *params = (wchar_t*) malloc (params_size);
			if (!params) {
				QWrite_err(FWERROR_MALLOC, 2, "params", params_size);
				QWrite("[]]");
				break;
			}
			memset(params, 0, params_size);

			int remaining_size = (int)params_size;
			wcscat(params, L"-run=");
			wchar_t *p = params + wcslen(params);
			remaining_size -= wcslen(params)*2;
			MultiByteToWideChar(CP_UTF8, 0, global_exe_name[global.exe_index], strlen(global_exe_name[global.exe_index]), p, remaining_size);

			wcscat(params, L" -eventtask=");
			wcscat(params, taskname);
			wcscat(params, L" ");
			p = params + wcslen(params);
			remaining_size -= wcslen(params)*2;
			MultiByteToWideChar(CP_UTF8, 0, argument[arg_parameters].text, argument[arg_parameters].length, p, remaining_size);

			// Working directory
			wchar_t appdir[MAX_PATH] = L"";
			MultiByteToWideChar(CP_UTF8, 0, global.game_dir, global.game_dir_length, appdir, MAX_PATH);

			// Task comment
			const int comment_size        = 128;
			wchar_t comment[comment_size] = L"";
			MultiByteToWideChar(CP_UTF8, 0, argument[arg_comment].text, argument[arg_comment].length, comment, comment_size);

			// Task user
			DWORD username_length = 128;
			wchar_t username[128] = L"";
			GetUserNameW(username, &username_length);

			// Start time
			TASK_TRIGGER trigger_options;
			ZeroMemory(&trigger_options, sizeof(TASK_TRIGGER));
			trigger_options.cbTriggerSize = sizeof(TASK_TRIGGER);
			String item;
			size_t pos = 0;
			int index = 0;

			while ((item = String_tokenize(argument[arg_date], ",", pos, OPTION_TRIM_SQUARE_BRACKETS)).length > 0) {
				switch(index) {
					case 0 : trigger_options.wBeginYear=(WORD)strtoul(item.text,NULL,0); break;
					case 1 : trigger_options.wBeginMonth=(WORD)strtoul(item.text,NULL,0); break;
					case 2 : trigger_options.wBeginDay=(WORD)strtoul(item.text,NULL,0); break;
					case 3 : trigger_options.Type.Weekly.rgfDaysOfTheWeek=1 << ((WORD)strtoul(item.text,NULL,0)); break;
					case 4 : trigger_options.wStartHour=(WORD)strtoul(item.text,NULL,0); break;
					case 5 : trigger_options.wStartMinute=(WORD)strtoul(item.text,NULL,0); break;
				}
				index++;
			}

			trigger_options.TriggerType = (TASK_TRIGGER_TYPE)arg_recurrence;
			switch(trigger_options.TriggerType) {
				case TASK_TIME_TRIGGER_WEEKLY : trigger_options.Type.Weekly.WeeksInterval=1; break;
				case TASK_TIME_TRIGGER_DAILY  : trigger_options.Type.Daily.DaysInterval=1; break;
			}

			// Task flags
			#define TASK_FLAG_RUN_ONLY_IF_LOGGED_ON 0x2000
			DWORD flags = 
				TASK_FLAG_RUN_IF_CONNECTED_TO_INTERNET | 
				TASK_FLAG_RUN_ONLY_IF_LOGGED_ON | 
				TASK_FLAG_SYSTEM_REQUIRED | 
				TASK_FLAG_INTERACTIVE;
			
			ITaskScheduler *scheduler = NULL;
			IUnknown       *unknown   = NULL;
			ITask          *task      = NULL;
			ITaskTrigger *trigger     = NULL;
			IPersistFile *job_file    = NULL;
			WORD trigger_number       = 0;
			char error_text[32]       = "";

			HRESULT result = CoInitialize(NULL);
			if (SUCCEEDED(result)) {
				result = CoCreateInstance(CLSID_CTaskScheduler, NULL, CLSCTX_INPROC_SERVER, IID_ITaskScheduler, (void **)&scheduler);
				if (FAILED(result)) {strcpy(error_text, "CoCreateInstance"); goto scheduler_end;}
			} else {
				strcpy(error_text, "CoInitialize"); goto scheduler_end;
			}

			result = scheduler->Activate((LPCWSTR)taskname, IID_ITask, &unknown);
			if (SUCCEEDED(result)) {
				unknown->Release();
				unknown = NULL;
				result  = scheduler->Delete((LPCWSTR)taskname);
				if (FAILED(result)) {strcpy(error_text, "Delete"); goto scheduler_end;}
			}
			
			result = scheduler->NewWorkItem((LPCWSTR)taskname, CLSID_CTask, IID_ITask, &unknown);
			if (FAILED(result)) {strcpy(error_text, "NewWorkItem"); goto scheduler_end;}

			result = unknown->QueryInterface(IID_ITask, (void **)&task);
			if (FAILED(result)) {strcpy(error_text, "QueryInterface"); goto scheduler_end;}

			result = task->SetAccountInformation((LPCWSTR)username, NULL);
			if (FAILED(result)) {strcpy(error_text, "SetAccountInformation"); goto scheduler_end;}

			result = task->SetApplicationName((LPCWSTR)appname);
			if (FAILED(result)) {strcpy(error_text, "SetApplicationName"); goto scheduler_end;}

			result = task->SetParameters((LPCWSTR)params);
			if (FAILED(result)) {strcpy(error_text, "SetParameters"); goto scheduler_end;}

			result = task->SetWorkingDirectory((LPCWSTR)appdir);
			if (FAILED(result)) {strcpy(error_text, "SetDirectory"); goto scheduler_end;}

			result = task->SetComment((LPCWSTR)comment);
			if (FAILED(result)) {strcpy(error_text, "SetComment"); goto scheduler_end;}

			result = task->SetFlags(flags);
			if (FAILED(result)) {strcpy(error_text, "SetFlags"); goto scheduler_end;}

			result = task->CreateTrigger(&trigger_number, &trigger);
			if (FAILED(result)) {strcpy(error_text, "CreateTrigger"); goto scheduler_end;}

			result = trigger->SetTrigger(&trigger_options);
			if (FAILED(result)) {strcpy(error_text, "SetTrigger"); goto scheduler_end;}

			result = task->QueryInterface(IID_IPersistFile, (void **)&job_file);
			if (FAILED(result)) {strcpy(error_text, "QueryInterface job"); goto scheduler_end;}

			result = job_file->Save(NULL, FALSE);
			if (FAILED(result)) {strcpy(error_text, "Save"); goto scheduler_end;}

			scheduler_end:
			free(params);
			if (job_file)  job_file->Release();
			if (trigger)   trigger->Release();
			if (task)      task->Release();
			if (unknown)   unknown->Release();
			if (scheduler) scheduler->Release();
			CoUninitialize();

			if (SUCCEEDED(result))
				QWrite_err(FWERROR_NONE, 0);
			else {
				QWrite_err(FWERROR_HRESULT, 2, error_text, result);
			}
			QWrite("[]]");
		} break;

		case MODE_DELETE_TASK : {
			if (argument[arg_eventid].length == 0) {
				QWrite_err(FWERROR_PARAM_EMPTY, 1, "arg_eventid");
				QWrite("[]]");
				break;
			}

			// Task name
			const int taskname_size         = 32;
			wchar_t taskname[taskname_size] = L"OFP_GS_";
			MultiByteToWideChar(CP_UTF8, 0, argument[arg_eventid].text, argument[arg_eventid].length, taskname+wcslen(taskname), taskname_size-wcslen(taskname));

			ITaskScheduler *scheduler = NULL;
			IUnknown       *unknown   = NULL;
			char error_text[32]       = "";

			HRESULT result = CoInitialize(NULL);
			if (SUCCEEDED(result)) {
				result = CoCreateInstance(CLSID_CTaskScheduler, NULL, CLSCTX_INPROC_SERVER, IID_ITaskScheduler, (void **)&scheduler);
				if (FAILED(result)) {strcpy(error_text, "CoCreateInstance"); goto scheduler_end2;}
			} else {
				strcpy(error_text, "CoInitialize"); goto scheduler_end2;
			}

			result = scheduler->Activate((LPCWSTR)taskname, IID_ITask, &unknown);
			if (SUCCEEDED(result)) {
				unknown->Release();
				unknown = NULL;
				result  = scheduler->Delete((LPCWSTR)taskname);
				if (FAILED(result)) {strcpy(error_text, "Delete"); goto scheduler_end2;}
			}

			scheduler_end2:
			if (scheduler) scheduler->Release();
			if (unknown)   unknown->Release();
			CoUninitialize();

			if (SUCCEEDED(result))
				QWrite_err(FWERROR_NONE, 0);
			else {
				QWrite_err(FWERROR_HRESULT, 2, error_text, result);
			}
			QWrite("[]]");
		} break;

		case MODE_LIST_TASKS : {
			ITaskScheduler *scheduler   = NULL;
			IEnumWorkItems *enumeration = NULL;
			char error_text[32]         = "";
  
			LPWSTR *task_names  = NULL;
			LPWSTR task_name    = NULL;
			DWORD task_count    = 0;
			ULONG retrieve_size = 5;

			HRESULT result = CoInitialize(NULL);
			if (SUCCEEDED(result)) {
				result = CoCreateInstance(CLSID_CTaskScheduler, NULL, CLSCTX_INPROC_SERVER, IID_ITaskScheduler, (void **)&scheduler);
				if (FAILED(result)) {strcpy(error_text, "CoCreateInstance"); goto scheduler_end3;}
			} else {
				strcpy(error_text, "CoInitialize"); goto scheduler_end3;
			}
			
			result = scheduler->Enum(&enumeration);
			if (FAILED(result)) {strcpy(error_text, "Enum"); goto scheduler_end3;};

			QWrite_err(FWERROR_NONE, 0);
			QWrite("[");

			while (SUCCEEDED(enumeration->Next(retrieve_size, &task_names, &task_count)) && (task_count != 0)) {
				while (task_count) {
					task_name = task_names[--task_count];

					if (_wcsnicmp(task_name,L"OFP_GS_",7) == 0) {
						LPWSTR event_idW = task_name + 7;
						size_t length    = wcslen(event_idW);
						wchar_t *dot     = wcschr(event_idW, L'.');

						if (dot)
							length = dot - event_idW;

						const unsigned int max_length = 16;

						if (length > max_length)
							length = max_length;

						char event_idA[max_length] = "";
						int output_size = WideCharToMultiByte(CP_UTF8, 0, event_idW, (int)length, NULL, 0, NULL, NULL);
						WideCharToMultiByte(CP_UTF8, 0, event_idW, length, event_idA, output_size, NULL, NULL);

						QWritef("]+[\"%s\"", event_idA);
					}

					CoTaskMemFree(task_name);
				}
				CoTaskMemFree(task_names);
			}

			QWrite("]]");
  
			scheduler_end3:
			if (enumeration) enumeration->Release();
			if (scheduler) scheduler->Release();
			CoUninitialize();
			
			if (FAILED(result)) {
				QWrite_err(FWERROR_HRESULT, 2, error_text, result);
				QWrite("[]]");
			}
		} break;
	}
}
break;