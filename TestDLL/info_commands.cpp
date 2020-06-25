// -----------------------------------------------------------------
// INFORMATION COMMANDS
// -----------------------------------------------------------------

case C_INFO_VERSION:
{ // Return version

	char output[64] = "";

	if (numP <= 2  ||  strcmpi(par[2],"extended") != 0)
		sprintf(output, "%.2f", SCRIPT_VERSION);
	else {
		// Read version from the game executable
		// http://stackoverflow.com/questions/940707/how-do-i-programatically-get-the-version-of-a-dll-or-exe-file
		int major        = 0;
		int build        = 0;;
		DWORD  verHandle = NULL;
		UINT   size      = 0;
		LPBYTE lpBuffer  = NULL;
		DWORD  verSize   = GetFileVersionInfoSize((char*)global_exe_name[global.exe_index], &verHandle);

		if (verSize != NULL) {
			String buffer;
			String_init(buffer);
			String_allocate(buffer, verSize);

			LPSTR verData = buffer.pointer;

			if (GetFileVersionInfo((char*)global_exe_name[global.exe_index], verHandle, verSize, verData))
				if (VerQueryValue(verData, "\\", (VOID FAR* FAR*)&lpBuffer, &size))
					if (size) {
						VS_FIXEDFILEINFO *verInfo = (VS_FIXEDFILEINFO *)lpBuffer;

						if (verInfo->dwSignature == 0xfeef04bd) {
							major = HIWORD(verInfo->dwFileVersionMS);
							build = verInfo->dwFileVersionLS;
						}
					}

			String_end(buffer);
		}

		QWrite("_fwatch_test_version=2;", out);
		sprintf(output, "[%.2f,%s,%s,false,%d.%s%d]", SCRIPT_VERSION, getBool(global.CWA), getBool(global.DedicatedServer), major, (build<10 ? "0" : ""), build);
	}

	QWrite(output, out);
}
break;






case C_INFO_DEBUGOUT:
{ // Output debug string
	if(numP < 3) {
		QWrite("ERROR: Not enough parameters", out);
		break;
	}

	char *str1 = stripq(par[2]);
	DebugMessage(str1);
	QWrite("1", out);
}
break;





			
case C_INFO_DATE:	
{ // Return date and time information

	SYSTEMTIME st;
	bool get_current     = true;
	bool get_system_time = false;


	// Optional arguments
	if (numP > 2) {
		if (strcmpi(par[2],"systime") == 0) 
			get_system_time = true;

		// If an array was passed - tokenize it
		int l = strlen(par[2]) - 1;

		if (par[2][0]=='['  &&  par[2][l]==']') {
			int index  = 0;
			char *item = strtok(par[2], "[,]");

			while (pch != NULL) {
				// From ofp date array to systemtime structure
				switch (index) {
					case 0: st.wYear         = atoi(item); break;
					case 1: st.wMonth        = atoi(item); break;
					case 2: st.wDay          = atoi(item); break;
					case 4: st.wHour         = atoi(item); break;
					case 5: st.wMinute       = atoi(item); break;
					case 6: st.wSecond       = atoi(item); break;
					case 7: st.wMilliseconds = atoi(item); break;
					case 9: get_system_time  = strcmpi(pch,"true") == 0; break;
				}
				
				index++;
				item = strtok (NULL, "[,]");
			}

			FILETIME ft;
			SystemTimeToFileTime(&st, &ft);
			FileTimeToSystemTime(&ft, &st);
			get_current = false;
		}
	}


	if (get_current)
		if (!get_system_time) 
			GetLocalTime(&st); 
		else 
			GetSystemTime(&st);

	char output[64] = "";
	FormatTime(st, get_system_time, output);
	QWrite(output, out);
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
	if (hwnd = GetTopWindow(hwnd)) {
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
		} while (hwnd = GetNextWindow(hwnd, GW_HWNDNEXT));
	}

	char output[64] = "";

	sprintf(output, "[%d,%d,[%d,%d,%d,%d],[%d,%d,%d,%d]]", 
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

	QWrite(output, out);
}
break;












case C_INFO_STARTTIME:
{ // Return game process creation date

	FILETIME ct;
	FILETIME et;
	FILETIME kt;
	FILETIME ut;
	int get_system_time = numP>2  &&  strcmpi(par[2],"systime")==0;

	if (!GetProcessTimes(phandle, &ct, &et, &kt, &ut)) {
		FWerror(5,GetLastError(),CommandID,"","",0,0,out);
		QWrite("[]]", out);
		break;
	}

	FWerror(0,0,CommandID,"","",0,0,out);

	char output[64] = "";
	FormatFileTime(ct, get_system_time, output);
	QWrite(output, out);
	QWrite("]",out);
}
break;












case C_INFO_TICK:
{ // Get tick count

	char output[32] = "";
	sprintf(output, "%d", GetTickCount() - global.mission_path_savetime);
	QWrite(output, out);
}
break;












case C_INFO_ERRORLOG:
{ // Switch error message logging

	// If argument passed then enable / disable
	if (numP > 2)
		global.ErrorLog_Enabled = String2Bool(par[2]);

	QWrite(getBool(global.ErrorLog_Enabled), out);
}
break;
