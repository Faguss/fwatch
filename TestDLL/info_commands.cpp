// -----------------------------------------------------------------
// INFORMATION COMMANDS
// -----------------------------------------------------------------

case C_INFO_VERSION:
{ // Return version

	if (argument_num <= 2  ||  strcmpi(argument[2].text,"extended")!=0)
		QWritef("%.2f", SCRIPT_VERSION);
	else {
		// Read version from the game executable
		// http://stackoverflow.com/questions/940707/how-do-i-programatically-get-the-version-of-a-dll-or-exe-file
		int major        = 0;
		int build        = 0;;
		DWORD  verHandle = NULL;
		UINT   size      = 0;
		LPBYTE lpBuffer  = NULL;
		DWORD  verSize   = GetFileVersionInfoSize((char*)global_exe_name[global.exe_index], &verHandle);

		if (verSize) {
			StringDynamic buffer;
			StringDynamic_init(buffer);
			StringDynamic_allocate(buffer, (size_t)verSize);

			LPSTR verData = buffer.text;

			if (GetFileVersionInfo((char*)global_exe_name[global.exe_index], verHandle, verSize, verData))
				if (VerQueryValue(verData, "\\", (VOID FAR* FAR*)&lpBuffer, &size))
					if (size) {
						VS_FIXEDFILEINFO *verInfo = (VS_FIXEDFILEINFO *)lpBuffer;

						if (verInfo->dwSignature == 0xfeef04bd) {
							major = HIWORD(verInfo->dwFileVersionMS);
							build = verInfo->dwFileVersionLS;
						}
					}

			StringDynamic_end(buffer);
		}

		QWritef("_fwatch_test_version=4;[%.2f,%s,%s,false,%d.%s%d]", 
			SCRIPT_VERSION, 
			getBool(global_exe_version[global.exe_index]!=VER_196 && global_exe_version[global.exe_index]!=VER_196_SERVER), 
			getBool(global.is_server), 
			major, 
			(build<10 ? "0" : ""), 
			build
		);
	}
}
break;






case C_INFO_DEBUGOUT:
{ // Output debug string
	if (argument_num < 3) {
		QWrite("ERROR: Not enough parameters");
		break;
	}

	DebugMessage(String_trim_quotes(argument[2]));
	QWrite("1");
}
break;





			
case C_INFO_DATE:	
{ // Return date and time information

	SYSTEMTIME st;
	bool get_current     = true;
	bool get_system_time = false;

	// Optional arguments
	if (argument_num > 2) {
		if (strcmpi(argument[2].text,"systime") == 0) 
			get_system_time = true;

		// If an array was passed - tokenize it
		if (argument[2].length>2  &&  argument[2].text[0]=='['  &&  argument[2].text[argument[2].length-1]==']') {
			int index  = 0;
			size_t pos = 0;
			String item;

			while ((item = String_tokenize(argument[2], ",", pos, OPTION_TRIM_SQUARE_BRACKETS)).length > 0) {
				// From ofp date array to systemtime structure
				switch (index) {
					case 0: st.wYear         = atoi(item.text); break;
					case 1: st.wMonth        = atoi(item.text); break;
					case 2: st.wDay          = atoi(item.text); break;
					case 4: st.wHour         = atoi(item.text); break;
					case 5: st.wMinute       = atoi(item.text); break;
					case 6: st.wSecond       = atoi(item.text); break;
					case 7: st.wMilliseconds = atoi(item.text); break;
					case 9: get_system_time  = strcmpi(item.text,"true") == 0; break;
				}
				
				index++;
			}

			FILETIME ft;
			SystemTimeToFileTime(&st, &ft);
			FileTimeToSystemTime(&ft, &st);
			get_current = false;
		}
	}


	if (get_current) {
		if (!get_system_time)
			GetLocalTime(&st); 
		else
			GetSystemTime(&st);
	}

	char temp[64] = "";
	SystemTimeToString(st, get_system_time, temp);
	QWrite(temp);
}
break;












case C_INFO_RESOLUTION:
{ // Get desktop and game resolution

	RECT desktop = {0,0,0,0};
	RECT window  = {0,0,0,0};
	RECT game    = {0,0,0,0};
	HWND hwnd    = NULL;

	GetWindowRect(GetDesktopWindow(), &desktop);

	// Find game window
	if ((hwnd = GetTopWindow(hwnd))) {
		DWORD pid            = 0;
		char window_name[32] = "";
		
		do {
			GetWindowText(hwnd, window_name, 32);
			GetWindowThreadProcessId(hwnd, &pid);

			if (global.pid==pid  &&  strcmpi(window_name,global_exe_window[global.exe_index])==0) {
				GetWindowRect(hwnd, &window);
				GetClientRect(hwnd, &game);
				break;
			}
		} while ((hwnd = GetNextWindow(hwnd, GW_HWNDNEXT)));
	}

	QWritef("[%d,%d,[%d,%d,%d,%d],[%d,%d,%d,%d]]", 
		desktop.right, 
		desktop.bottom, 
		window.left,  
		window.top,  
		window.right - window.left,  
		window.bottom - window.top,
		game.left,  
		game.top,  
		game.right - game.left,  
		game.bottom - game.top
	);
}
break;












case C_INFO_STARTTIME:
{ // Return game process creation date

	FILETIME ct;
	FILETIME et;
	FILETIME kt;
	FILETIME ut;

	if (!GetProcessTimes(phandle, &ct, &et, &kt, &ut)) {
		QWrite_err(FWERROR_WINAPI, 1, GetLastError());
		QWrite("[]]");
		break;
	}

	QWrite_err(FWERROR_NONE, 0);

	char temp[64] = "";
	FileTimeToString(ct, argument_num>2 && strcmpi(argument[2].text,"systime")==0, temp);
	QWritef("%s]", temp);
}
break;












case C_INFO_TICK:
{ // Get tick count

	QWritef("%u", GetTickCount() - global.mission_path_savetime);
}
break;












case C_INFO_ERRORLOG:
{ // Switch error message logging

	// If argument passed then enable / disable
	if (argument_num > 2) {
		if (strcmp(argument[2].text,"start") == 0) {
			FILE *f        = fopen(!global.is_server ? "fwatch\\idb\\_errorLog.txt" : "fwatch\\idb\\_errorLogDedi.txt", "a");
			HANDLE phandle = OpenProcess(PROCESS_ALL_ACCESS, 0, GetCurrentProcessId());

			if (f) {
				WriterHeaderInErrorLog(&f, &phandle, 0);
				fclose(f);
			}

			if (phandle)
				CloseHandle(phandle);
		} else {
			global.ErrorLog_Enabled = String_bool(argument[2]);
			NotifyFwatchAboutErrorLog();
		}
	}

	QWrite(getBool(global.ErrorLog_Enabled));
}
break;