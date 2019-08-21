// -----------------------------------------------------------------
// INFORMATION COMMANDS
// -----------------------------------------------------------------

case C_INFO_VERSION:
{ // Return version

	//v1.13 extended - return global vars
	char tmp[32]	= "";
	bool extended	= false;

	if (numP > 2) 
		if (strcmpi(par[2],"extended") == 0)
			extended = true;

	if (!extended)
		sprintf(tmp, "%.2f", SCRIPT_VERSION);
	else
	{
		int major = 0;
		int build = 0;
		char gameexe[256];

        GetModuleFileName( GetModuleHandle( NULL ), gameexe, sizeof(gameexe) );

		// Read game version from game exe
		//	http://stackoverflow.com/questions/940707/how-do-i-programatically-get-the-version-of-a-dll-or-exe-file
		DWORD  verHandle = NULL;
		UINT   size      = 0;
		LPBYTE lpBuffer  = NULL;
		DWORD  verSize   = GetFileVersionInfoSize( gameexe, &verHandle);

		if (verSize != NULL)
		{
			LPSTR verData = new char[verSize];

			if (GetFileVersionInfo( gameexe, verHandle, verSize, verData))
				if (VerQueryValue(verData,"\\",(VOID FAR* FAR*)&lpBuffer,&size))
					if (size)
					{
						VS_FIXEDFILEINFO *verInfo = (VS_FIXEDFILEINFO *)lpBuffer;

						if (verInfo->dwSignature == 0xfeef04bd)
							major = HIWORD(verInfo->dwFileVersionMS),
							build = verInfo->dwFileVersionLS;
					}

			delete[] verData;
		}

		QWrite("_fwatch_test_version=2;", out);
		sprintf(tmp, "[%.2f,%s,%s,false,%d.%s%d]", SCRIPT_VERSION, getBool(CWA), getBool(DedicatedServer),major,(build<10 ? "0" : ""),build);
	};

	QWrite(tmp, out);
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
	bool getCurrent = true;
	bool systime	= false;


	// Optional arguments
	if (numP > 2)
	{
		if (strcmpi(par[2],"systime") == 0) 
			systime = true;

		// If an array was passed
		int l = strlen(par[2]) - 1;

		if (par[2][0]=='['  &&  par[2][l]==']')
		{
			// Tokenize array
			int i		= 0;
			char *pch	= strtok(par[2], "[,]");

			while (pch != NULL)
			{
				// From ofp date array to systemtime structure
				switch (i)
				{
					case 0: st.wYear		= atoi(pch); break;
					case 1: st.wMonth		= atoi(pch); break;
					case 2: st.wDay			= atoi(pch); break;
					case 4: st.wHour		= atoi(pch); break;
					case 5: st.wMinute		= atoi(pch); break;
					case 6: st.wSecond		= atoi(pch); break;
					case 7: st.wMilliseconds= atoi(pch); break;

					case 9: 
					{
						if (strcmpi(pch,"true") == 0) 
							systime = true; 
						else 
							systime = false;
					}; 
					break;
				};
				
				i++;
				pch = strtok (NULL, "[,]");
			};

			FILETIME ft;
			SystemTimeToFileTime( &st, &ft );
			FileTimeToSystemTime( &ft, &st );
			getCurrent = false;
		};
	};


	// Current date
	if (getCurrent)
		if (!systime) 
			GetLocalTime(&st); 
		else 
			GetSystemTime(&st);


	// Return data
	char tmp[64] = "";
	FormatTime(st, systime, tmp);
	QWrite(tmp, out);
}
break;












case C_INFO_RESOLUTION:
{ // Get desktop and game resolution

	RECT desktop;
	RECT window;
	RECT game;
	HWND hwnd = NULL;


	// Default values
	window.left		= 0; 
	window.right	= 0; 
	window.top		= 0; 
	window.bottom	= 0;
	game.left		= 0; 
	game.right		= 0; 
	game.top		= 0; 
	game.bottom		= 0;


	// Get windows desktop size
	GetWindowRect(GetDesktopWindow(), &desktop);


	// Find game window
	if (hwnd = GetTopWindow(hwnd)) 
	{
		DWORD thisPID		= 0;
		char windowName[64] = "";
		
		do
		{
			// Compare process id
			GetWindowText(hwnd, windowName, 64);
			GetWindowThreadProcessId(hwnd, &thisPID);

			if (pid==thisPID  &&  strcmpi(windowName,GameWindowName)==0) 
			{
				// Get sizes		
				GetWindowRect(hwnd, &window);
				GetClientRect(hwnd, &game);
				break;
			};
		}
		while (hwnd = GetNextWindow(hwnd, GW_HWNDNEXT));
		
	};

	char tmp[64] = "";

	sprintf(tmp, "[%d,%d,[%d,%d,%d,%d],[%d,%d,%d,%d]]", 
		desktop.right, 
		desktop.bottom, 
		window.left,  
		window.top,  
		window.right - window.left,  
		window.bottom - window.top,
		game.left,  
		game.top,  
		game.right - game.left,  
		game.bottom - game.top);

	QWrite(tmp, out);
}
break;












case C_INFO_STARTTIME:
{ // Return process creation date

	FILETIME ct;
	FILETIME et;
	FILETIME kt;
	FILETIME ut;
	int systime = false;

	if (numP > 2)
		if (strcmpi(par[2],"systime") == 0) 
			systime = true;

	if (!GetProcessTimes(phandle, &ct, &et, &kt, &ut))
	{
		FWerror(5,GetLastError(),CommandID,"","",0,0,out);
		QWrite("[]]", out);
		break;
	};

	FWerror(0,0,CommandID,"","",0,0,out);

	char tmp[64] = "";
	FormatFileTime(ct, systime, tmp);
	QWrite(tmp, out);
	QWrite("]",out);
}
break;












case C_INFO_TICK:
{ // Get tick count

	char tmp[32] = "";
	sprintf(tmp, "%d", GetTickCount() - PathSqfTime);
	QWrite(tmp, out);
}
break;












case C_INFO_ERRORLOG:
{ // Switch error message logging

	// If argument passed then enable / disable
	if (numP > 2)
		ErrorLog_Enabled = String2Bool(par[2]);

	QWrite(getBool(ErrorLog_Enabled), out);
}
break;
