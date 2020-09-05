// -----------------------------------------------------------------
// FUNCTION INVENTORY
// -----------------------------------------------------------------

// Remove quotation marks from string
char* stripq(char *str) {
	if(str[0] == '"' && str[strlen(str)-1] == '"') {
		str++;
		str[strlen(str)-1] = '\0';
	}
	return str;
}








// "Tokenize" string, ignoring substrings (inside quotation marks)
// Ugly
char* strtok2(char* str) {
	char c;
	int i = 0, y = 0;
	bool x = false;

	// Replace spaces with null character
	do {
		c = str[i];

		if(c == '"')
			x=!x;

		if(c == ' ' && !x)
			str[i] = '\n';
		i++;
	} while(c != '\0');
	
	i = 0, y = 0;
	// Remove double-null characters
	do {
		str[y] = str[i];
		if(str[i] != '\0')
			if(str[i] != '\n' || str[i+1] != '\n')
				y++;
		i++;
	} while(str[y] != '\0');

	return str;
}







// Find matching command and return ID or -1 if not found
int matchCmd(char *cmd) {
	int cc = 0, i = 0;
	do {
		cc = cmdsList[i].id;
		if(!strcmp(cmd, (const char *) &cmdsList[i].cmd))
			return cc;
		i++;
	} while(cc != -1);
	return -1;
}







void QWrite(char* str, HANDLE file) {
	DWORD foo;
	unsigned int l = strlen(str);
	WriteFile(file, str, l, &foo, NULL);

	if(foo < l)
		DebugMessage("WARNING! Wanted to write %d bytes but only %d written", strlen(str), foo);
}








// Return "TRUE" or "FALSE"
inline char* getBool(short b) {
	if(b != 0)
		return "TRUE";
	else
		return "FALSE";
}








// Format special keys to string
char *formatKey(int c) {
	switch(c) {

//v1.13 updated key list
//http://msdn.microsoft.com/en-us/library/windows/desktop/dd375731%28v=vs.85%29.aspx
#define VK_XBUTTON1		0x05
#define VK_XBUTTON2		0x06
#define VK_KANA			0x15
#define VK_JUNJA		0x17
#define VK_FINAL		0x18
#define VK_KANJI		0x19
#define VK_ACCEPT		0x1E
#define VK_MODECHANGE	0x1F
#define VK_PRINT		0x2A
#define VK_SLEEP		0x5F
#define VK_SEPARATOR	0x6C

#define VK_BROWSER_BACK			0xA6
#define VK_BROWSER_FORWARD		0xA7
#define VK_BROWSER_REFRESH		0xA8
#define VK_BROWSER_STOP			0xA9
#define VK_BROWSER_SEARCH		0xAA
#define VK_BROWSER_FAVORITES	0xAB
#define VK_BROWSER_HOME			0xAC
#define VK_VOLUME_MUTE			0xAD
#define VK_VOLUME_DOWN			0xAE
#define VK_VOLUME_UP			0xAF
#define	VK_MEDIA_NEXT_TRACK		0xB0
#define VK_MEDIA_PREV_TRACK		0xB1
#define VK_MEDIA_STOP			0xB2
#define VK_MEDIA_PLAY_PAUSE		0xB3
#define VK_LAUNCH_MAIL			0xB4
#define VK_LAUNCH_MEDIA_SELECT	0xB5
#define VK_LAUNCH_APP1			0xB6
#define VK_LAUNCH_APP2			0xB7

#define VK_OEM_1		0xBA
#define VK_OEM_PLUS		0xBB
#define VK_OEM_COMMA	0xBC
#define VK_OEM_MINUS	0xBD
#define VK_OEM_PERIOD	0xBE
#define VK_OEM_2		0xBF
#define VK_OEM_3		0xC0
#define VK_ABNT_C1		0xC1
#define VK_ABNT_C2		0xC2
#define VK_OEM_4		0xDB
#define VK_OEM_5		0xDC
#define VK_OEM_6		0xDD
#define VK_OEM_7		0xDE
#define VK_OEM_8		0xDF
#define VK_OEM_102		0xE2
#define VK_PROCESSKEY	0xE5
#define VK_PACKET		0xE7


		case VK_LBUTTON: return "\"LBUTTON\"";
		case VK_RBUTTON: return "\"RBUTTON\"";
		case VK_MBUTTON: return "\"MBUTTON\"";
		case VK_CANCEL: return "\"CANCEL\"";
		case VK_XBUTTON1 : return "\"4BUTTON\"";	// 4th mouse button
		case VK_XBUTTON2 : return "\"5BUTTON\"";	// 5th mouse button

		case VK_BACK: return "\"BACKSPACE\"";
		case VK_TAB: return "\"TAB\"";
		case VK_CLEAR: return "\"CLEAR\"";			// numpad5 while numlock is off
		case VK_RETURN: return "\"ENTER\"";

		case VK_SHIFT: return "\"SHIFT\"";
		case VK_CONTROL: return "\"CTRL\"";
		case VK_MENU: return "\"ALT\"";
		case VK_PAUSE: return "\"PAUSE\"";			// pause break
		case VK_CAPITAL: return "\"CAPSLOCK\"";

		case VK_KANA: return "\"KANA\"";
		case VK_JUNJA: return "\"JUNJA\"";
		case VK_FINAL: return "\"FINAL\"";
		case VK_HANJA: return "\"HANJA\"";

		case VK_ESCAPE: return "\"ESC\"";
		case VK_CONVERT : return "\"CONVERT\"";
		case VK_NONCONVERT : return "\"NONCONVERT\"";
		case VK_ACCEPT : return "\"ACCEPT\"";
		case VK_MODECHANGE : return "\"MODECHANGE\"";

		case VK_SPACE: return "\"SPACE\"";
		case VK_PRIOR: return "\"PAGEUP\"";
		case VK_NEXT: return "\"PAGEDOWN\"";
		case VK_END: return "\"END\"";
		case VK_HOME: return "\"HOME\"";
		case VK_LEFT: return "\"LEFT\"";
		case VK_RIGHT: return "\"RIGHT\"";
		case VK_UP: return "\"UP\"";
		case VK_DOWN: return "\"DOWN\"";
		case VK_SELECT: return "\"SELECT\"";
		case VK_PRINT: return "\"PRINT\"";
		case VK_EXECUTE: return "\"EXECUTE\"";
		case VK_SNAPSHOT: return "\"PRINTSCREEN\"";
		case VK_INSERT: return "\"INSERT\"";
		case VK_DELETE: return "\"DELETE\"";
		case VK_HELP: return "\"HELP\"";

		case 0x30 : return "\"0\"";
		case 0x31 : return "\"1\"";
		case 0x32 : return "\"2\"";
		case 0x33 : return "\"3\"";
		case 0x34 : return "\"4\"";
		case 0x35 : return "\"5\"";
		case 0x36 : return "\"6\"";
		case 0x37 : return "\"7\"";
		case 0x38 : return "\"8\"";
		case 0x39 : return "\"9\"";
		case 0x41 : return "\"A\"";
		case 0x42 : return "\"B\"";
		case 0x43 : return "\"C\"";
		case 0x44 : return "\"D\"";
		case 0x45 : return "\"E\"";
		case 0x46 : return "\"F\"";
		case 0x47 : return "\"G\"";
		case 0x48 : return "\"H\"";
		case 0x49 : return "\"I\"";
		case 0x4A : return "\"J\"";
		case 0x4B : return "\"K\"";
		case 0x4C : return "\"L\"";
		case 0x4D : return "\"M\"";
		case 0x4E : return "\"N\"";
		case 0x4F : return "\"O\"";
		case 0x50 : return "\"P\"";
		case 0x51 : return "\"Q\"";
		case 0x52 : return "\"R\"";
		case 0x53 : return "\"S\"";
		case 0x54 : return "\"T\"";
		case 0x55 : return "\"U\"";
		case 0x56 : return "\"V\"";
		case 0x57 : return "\"W\"";
		case 0x58 : return "\"X\"";
		case 0x59 : return "\"Y\"";
		case 0x5A : return "\"Z\"";

		case VK_LWIN: return "\"LWIN\"";
		case VK_RWIN: return "\"RWIN\"";
		case VK_APPS: return "\"APPS\"";
		case VK_SLEEP: return "\"SLEEP\"";

		case VK_NUMPAD0: return "\"NUMPAD0\"";
		case VK_NUMPAD1: return "\"NUMPAD1\"";
		case VK_NUMPAD2: return "\"NUMPAD2\"";
		case VK_NUMPAD3: return "\"NUMPAD3\"";
		case VK_NUMPAD4: return "\"NUMPAD4\"";
		case VK_NUMPAD5: return "\"NUMPAD5\"";
		case VK_NUMPAD6: return "\"NUMPAD6\"";
		case VK_NUMPAD7: return "\"NUMPAD7\"";
		case VK_NUMPAD8: return "\"NUMPAD8\"";
		case VK_NUMPAD9: return "\"NUMPAD9\"";
		case VK_MULTIPLY: return "\"MULTIPLY\"";	// numpad asterisk	1.11 fixed
		case VK_ADD: return "\"ADD\"";				// numpad plus
		case VK_SEPARATOR: return "\"SEPARATOR\"";
		case VK_SUBTRACT: return "\"SUBTRACT\"";	// numpad minus
		case VK_DECIMAL: return "\"DECIMAL\"";		// numpad decimal mark
		case VK_DIVIDE: return "\"DIVIDE\"";		// numpad slash

		case VK_F1: return "\"F1\"";
		case VK_F2: return "\"F2\"";
		case VK_F3: return "\"F3\"";
		case VK_F4: return "\"F4\"";
		case VK_F5: return "\"F5\"";
		case VK_F6: return "\"F6\"";
		case VK_F7: return "\"F7\"";
		case VK_F8: return "\"F8\"";
		case VK_F9: return "\"F9\"";
		case VK_F10: return "\"F10\"";
		case VK_F11: return "\"F11\"";
		case VK_F12: return "\"F12\"";
		case VK_F13: return "\"F13\"";
		case VK_F14: return "\"F14\"";
		case VK_F15: return "\"F15\"";
		case VK_F16: return "\"F16\"";
		case VK_F17: return "\"F17\"";
		case VK_F18: return "\"F18\"";
		case VK_F19: return "\"F19\"";
		case VK_F20: return "\"F20\"";
		case VK_F21: return "\"F21\"";
		case VK_F22: return "\"F22\"";
		case VK_F23: return "\"F23\"";
		case VK_F24: return "\"F24\"";

		case VK_NUMLOCK: return "\"NUMLOCK\"";
		case VK_SCROLL: return "\"SCROLLOCK\"";
		case VK_LSHIFT: return "\"LSHIFT\"";
		case VK_RSHIFT: return "\"RSHIFT\"";
		case VK_LCONTROL: return "\"LCTRL\"";
		case VK_RCONTROL: return "\"RCTRL\"";
		case VK_LMENU: return "\"LALT\"";
		case VK_RMENU: return "\"RALT\"";

		case VK_BROWSER_BACK: return "\"WEBBACK\"";			// multimedia keyboard keys
		case VK_BROWSER_FORWARD: return "\"WEBFORWARD\"";
		case VK_BROWSER_REFRESH: return "\"WEBREFRESH\"";
		case VK_BROWSER_STOP: return "\"WEBSTOP\"";
		case VK_BROWSER_SEARCH: return "\"WEBSEARCH\"";
		case VK_BROWSER_FAVORITES: return "\"WEBFAVORITES\"";
		case VK_BROWSER_HOME: return "\"WEBHOME\"";
		case VK_VOLUME_MUTE: return "\"VOLUMEMUTE\"";
		case VK_VOLUME_DOWN: return "\"VOLUMEDOWN\"";
		case VK_VOLUME_UP: return "\"VOLUMEUP\"";
		case VK_MEDIA_NEXT_TRACK: return "\"MEDIANEXT\"";
		case VK_MEDIA_PREV_TRACK: return "\"MEDIAPREV\"";
		case VK_MEDIA_STOP: return "\"MEDIASTOP\"";
		case VK_MEDIA_PLAY_PAUSE: return "\"MEDIAPLAY\"";
		case VK_LAUNCH_MAIL: return "\"MAIL\"";
		case VK_LAUNCH_MEDIA_SELECT: return "\"MEDIASELECT\"";
		case VK_LAUNCH_APP1: return "\"MYCOMPUTER\"";
		case VK_LAUNCH_APP2: return "\"CALCULATOR\"";

		case VK_OEM_1: return "\";\"";				// semi-colon
		case VK_OEM_PLUS: return "\"=\"";			// equality
		case VK_OEM_COMMA: return "\",\"";			// comma
		case VK_OEM_MINUS: return "\"-\"";			// minus
		case VK_OEM_PERIOD: return "\".\"";			// dot
		case VK_OEM_2: return "\"/\"";				// slash
		case VK_OEM_3: return "\"`\"";				// grave accent
		case VK_OEM_4: return "\"[\"";				// open bracket
		case VK_OEM_5: return "\"\\\"";				// backslash
		case VK_OEM_6: return "\"]\"";				// close bracket
		case VK_OEM_7: return "\"'\"";				// apostrophe

		case VK_OEM_8: return "\"OEM8\"";
		case VK_OEM_102: return "\"OEM102\"";
		case VK_ABNT_C1 : return "\"ABNT_C1\"";
		case VK_ABNT_C2 : return "\"ABNT_C2\"";
		case VK_PROCESSKEY: return "\"PROCESS\"";
		case VK_PACKET: return "\"PACKET\"";
		case VK_ATTN: return "\"ATTN\"";
		case VK_CRSEL: return "\"CRSEL\"";
		case VK_EXSEL: return "\"EXSEL\"";
		case VK_EREOF: return "\"ERASEEOF\""; 
		case VK_PLAY: return "\"PLAY\""; 
		case VK_ZOOM: return "\"ZOOM\""; 
		case VK_NONAME: return "\"NONAME\""; 
		case VK_PA1: return "\"PA1\""; 
		case VK_OEM_CLEAR: return "\"OEMCLEAR\"";
	
		default:
			static char s[8];
			sprintf(s, "\"0x%x\"", c);		// format number to char   1.13 returns number instead of char
			return s;
	}
}







// Return true if OFP is the active window (OFP client only, returns false on dedicated server)
bool checkActiveWindow(void) {
	char wn[128];
	GetWindowText(GetForegroundWindow(), wn, 128);
	if(!strcmp(wn, global_exe_window[global.exe_index]))
		return true;
	else
		return false;
}











//v1.13 Extract extension out of file name
void getAttributes(WIN32_FIND_DATA &fd, char *data, int systime)
{
	strcpy(data, "");
	
	// Name without extension and then extension alone
	int pos	  = 0;
	char *dot = strrchr(fd.cFileName,'.');
	char *ext = "";

	if (dot != NULL) 
		pos               = dot - fd.cFileName, 
		fd.cFileName[pos] = '\0', 
		ext               = fd.cFileName + pos + 1;

	sprintf(data, "[\"%s\",\"%s\",[", fd.cFileName, ext);


	// List of attributes
	if ((fd.dwFileAttributes & 128) == 0)
	{
		bool addComma	= false;
		int i			= 1;
		int j			= 0;

		char attribute[][40] = 
		{
			"readonly",
			"hidden",
			"system",
			"",
			"directory",
			"archive",
			"device",
			"",
			"temporary",
			"sparse",
			"reparse",
			"compressed",
			"offline",
			"notcontentindexed",
			"encrypted",
			"integritystream",
			"virtual",
			"noscrubdata"
		};

		while (i < 131072)
		{
			if (i!=8  &&  i!=128  &&  (fd.dwFileAttributes & i)!=0)
			{
				if (addComma) 
					strcat(data, ","); 
				else 
					addComma = true;

				strcat(data, "\""); 
				strcat(data, attribute[j]); 
				strcat(data, "\"");
			};

			i *= 2; 
			j++;
		};
	};
	strcat(data, "],");


	// Creation, last access and last write time
	FILETIME curr;
	bool addComma = false;

	for (int i=0; i<3; i++)
	{
		if (addComma) 
			strcat(data, ","); 
		else 
			addComma = true;

		switch(i)	// Select FILETIME struct from file attribute data
		{
			case 0: curr=fd.ftCreationTime; break;
			case 1: curr=fd.ftLastAccessTime; break;
			case 2: curr=fd.ftLastWriteTime; break;
		};

		FormatFileTime(curr, systime, data);
	};


	// Size
	double bytes	 = fd.nFileSizeLow;
	double kilobytes = 0;
	double megabytes = 0;

	if (bytes >= 1048576) 
		megabytes  = bytes / 1048576,
		megabytes -= fmod(megabytes, 1),
		bytes     -= megabytes * 1048576;

	if (bytes >= 1024)
		kilobytes  = bytes / 1024,
		kilobytes -= fmod(kilobytes, 1),
		bytes     -= kilobytes * 1024;

	sprintf(data, "%s,[%f,%f,%f]]", data, bytes, kilobytes, megabytes);
};













//v1.12 Replace all occurrences in a string
// http://someboringsite.com/2009/09/ansi-c-string-replacement-function.html
char* str_replace(const char *strbuf, const char *strold, const char *strnew, int matchWord, int caseSens) 
{
	char *strret;
	char *posnews;
	char *posold;
	char *p		 = NULL;
	size_t szold = strlen(strold);
	size_t sznew = strlen(strnew);
	size_t n	 = 1;

	if (!strbuf) 
		return NULL;

	if (!strold  ||  !strnew) 
		return strdup(strbuf);

	p = strstr2(strbuf, strold, matchWord, caseSens);

	if (!p) 
		return strdup(strbuf);

	while (n > 0)
	{
		if (!(p = strstr2(p+1, strold, matchWord, caseSens))) 
			break;

		n++;
	};

	strret = (char*) malloc (strlen(strbuf)-(n*szold)+(n*sznew)+1);

	if (strret == NULL) 
		return NULL;

	p = strstr2(strbuf, strold, matchWord, caseSens);

	strncpy(strret, strbuf, (p-strbuf));

	strret[p-strbuf] = 0;
	posold           = p + szold;
	posnews          = strret + (p-strbuf);

	strcpy(posnews, strnew);
	posnews += sznew;

	while (n > 0) 
	{
		if (!(p = strstr2(p+1, strold, matchWord, caseSens))) 
			break;

		strncpy(posnews, posold, p-posold);

		posnews[p-posold] = 0;
		posnews          += (p-posold);

		strcpy(posnews, strnew);

		posnews += sznew;
		posold   = p+szold;
	};

	strcpy(posnews, posold);
	return strret;
};











//v1.13 Search for string insensitive
//http://www.codeguru.com/cpp/cpp/string/article.php/c5641/Case-Insensitive-strstr.htm
char *strstr2(const char *arg1, const char *arg2, int matchWord, int caseSens)
{
	const char *a;
	const char *b;
	bool cond			= 0;
	unsigned int pos	= 0; 
	unsigned int len	= strlen(arg1);

	// For each letter in arg1
	for( ; *arg1; *arg1++, pos++)
	{
		a = arg1;
		b = arg2;

		if (caseSens) 
			cond = *a++ == *b++; 
		else 
			cond = (*a++ | 32) == (*b++ | 32);


		// Run a comparison with arg2
		while (cond)
		{
			// If got to the end
			if (!*b)
			{
				// If matching word - occurrence musn't be surrounded by alphanum chars
				bool LeftEmpty  = false;
				bool RightEmpty = false;

				if (matchWord)
				{
					if (pos == 0) 
						LeftEmpty = true; 
					else 
						if (!isalnum(arg1[-1])  &&  arg1[-1]!='_') 
							LeftEmpty = true;

					if (pos+strlen(arg2) >=  len) 
						RightEmpty = true; 
					else 
						if (!isalnum(arg1[strlen(arg2)])  &&  arg1[strlen(arg2)]!='_') 
							RightEmpty = true;
				};
			
				// Return pointer to the occurence
				if (!matchWord  ||  matchWord  &&  LeftEmpty  &&  RightEmpty) 
					return ((char *)arg1);
				
				// If failed to match word then move forward
				if (pos+strlen(arg2)  <  len) 
					arg1 += strlen(arg2), 
					pos  += strlen(arg2);
			};

			if (caseSens) 
				cond = *a++ == *b++; 
			else 
				cond = (*a++ | 32) == (*b++ | 32);
		};
	}

	return(NULL);
};






// v1.13 Move given file to the system recycle bin
bool trashFile(char* path, int CommandID, HANDLE out, int ErrorBehaviour)
{
	// Allocate buffer
	int varLen	= 256 + 1 + strlen(path) + 2;
	char *var	= (char*) malloc (varLen);

	if (var == NULL)
	{
		FWerror(10,0,CommandID,"trashFile:var","",varLen,0,out);
		return false;
	};
	

	// Build path
	GetCurrentDirectory(256, var);
	strcat(var, "\\");
	strcat(var, path);
	var[strlen(var)+1] = 0;


	// Trash file
	SHFILEOPSTRUCT shfos;
	shfos.hwnd   = NULL;
	shfos.wFunc  = FO_DELETE;
	shfos.pFrom  = var;
	shfos.pTo    = NULL;
	shfos.fFlags = FOF_SILENT | FOF_NOCONFIRMATION | FOF_ALLOWUNDO;
	int val	= SHFileOperation(&shfos);


		// Special mode - if file not found then return true
		if (ErrorBehaviour==1  &&  (val==1026 || val==2)) 
			val = 0;


	free(var);

	if (val == 0)	
		return true; 
	else 
	{
		FWerror(6,val,CommandID,"","",0,0,out);
		return false;
	};
};







// v1.13 Convert radians to degrees
double rad2deg (double num) 
{
	num *= 180 / 3.14159265;

	// if error then return zero
	char tmp[32] = "";
	sprintf(tmp, "%f", num);

	if (strstr(tmp,"#IN")) 
		num = 0;

	return num;
};






// v1.13 Convert degrees to radians
double deg2rad (double num)
{
	num *= 3.14159265 / 180;

	// if error then return zero
	char tmp[32] = "";
	sprintf(tmp, "%f", num);

	if (strstr(tmp,"#IN")) 
		num = 0;

	return num;
};






// v1.13 Format time structure to a OFP array
void FormatTime(SYSTEMTIME &st, int systime, char *data)
{
	TIME_ZONE_INFORMATION TimeZoneInfo;
	int result = GetTimeZoneInformation (&TimeZoneInfo);
	int dst    = result==TIME_ZONE_ID_DAYLIGHT ? TimeZoneInfo.DaylightBias : 0;

	char tmp[64] = "";

	sprintf(tmp, "[%d,%d,%d,%d,%d,%d,%d,%d,%d,%s]",
		st.wYear, 
		st.wMonth, 
		st.wDay, 
		st.wDayOfWeek, 
		st.wHour, 
		st.wMinute, 
		st.wSecond, 
		st.wMilliseconds, 
		(TimeZoneInfo.Bias + dst) * -1, 
		getBool(systime));

	strcat(data, tmp);
};







// v1.13 Convert time structure and format it
void FormatFileTime(FILETIME &ft, int systime, char *data)
{
	SYSTEMTIME st;
	FILETIME ft2;


	// Convert FILETIME to computer local time
	if (!systime)	
		FileTimeToLocalFileTime(&ft,  &ft2),
		FileTimeToSystemTime(   &ft2, &st);
	else 
		FileTimeToSystemTime(&ft, &st);


	FormatTime(st, systime, data);
};






// v1.13 Copy data to clipboard
bool CopyToClip(char *txt, bool append, int CommandID, HANDLE out)
{
	// Open it
	if (!OpenClipboard(NULL)) 
	{
		FWerror(20,GetLastError(),CommandID,"","",0,0,out);
		return false;
	};


	HANDLE hClipboardData;
	char *pchData;
	int l1 = 0;
	int l2 = strlen(txt);


	// Get existing text
	if (append)
	{
		// Validate data type
		if (!::IsClipboardFormatAvailable(CF_TEXT))
		{
			FWerror(21,0,CommandID,"","",0,0,out);
			return false;
		};

		hClipboardData = GetClipboardData(CF_TEXT);
		pchData		   = (char*)GlobalLock(hClipboardData);
		l1			   = strlen(pchData);
		
	};
	

	// Allocate buffer for the old + new text
	char *newTxt = (char*) malloc (l1+l2+1);
	if (newTxt == NULL) 
	{
		FWerror(10,0,CommandID,"newTxt","",l1+l2+1,0,out);
		GlobalUnlock(hClipboardData);
		CloseClipboard(); 
		return false;
	};


	// Copy text to the buffer
	if (append) 
		strcpy(newTxt, pchData), 
		strcat(newTxt, txt);
	else 
		strcpy(newTxt, txt);

	GlobalUnlock(hClipboardData);


	// Allocate 2nd buffer for transfering to the clip
	HGLOBAL glob = GlobalAlloc(GMEM_FIXED, l1+l2+1);
	LPVOID GlobalLock(glob);
	memcpy(glob, newTxt, l1+l2+1);


	// Clear clip
	if(!EmptyClipboard())
	{
		FWerror(22,GetLastError(),CommandID,"","",0,0,out);
		GlobalUnlock(glob);
		CloseClipboard();
		free(newTxt);
		return false;
	};


	// Copy to clip
	HANDLE ok = ::SetClipboardData( CF_TEXT, glob );

	if (!ok) 
		FWerror(23,GetLastError(),CommandID,"","",0,0,out);


	GlobalUnlock(glob);
	free(newTxt); 
	CloseClipboard();

	if (ok) 
		return true; 
	else 
		return false;
};






// v1.13 Custom handling of escape sequences
char* EscSequences(char *txt, int mode, int quantity)
{
	bool shift = false;
	int count  = 0;

	for (int i=0;  txt[i]!='\0';  i++)
	{
		if (txt[i] == '\\')
		{
			// Replace char
			if (mode==0  &&  txt[i+1]=='t')
				count++,
				shift  = true, 
				txt[i] = '\t';

			if (mode==1  &&  txt[i+1]=='n') 
				count++,
				shift  = true, 
				txt[i] = '\n';

			if (mode==2  &&  txt[i+1]=='n') 
				count++,
				txt[i]   = '\r', 
				txt[i+1] = '\n';


			// Shift other chars to the left (thus making the string shorter)
			for (int j=i+1; shift; j++)
			{
				txt[j] = txt[j+1]; 

				if (txt[j] == '\0') 
					shift = false;
			};
		};

		if (count>=quantity  &&  quantity>=0)
			break;
	};

	return txt;
};





// v1.13 Output error code and message
void FWerror(int code, int secondaryCode, int CommandID, char* str1, char* str2, int num1, int num2, HANDLE out)
{
	if (out==NULL  ||  CommandID==-1) 
		return;

	LPVOID lpMsgBuf = NULL;
	bool shfileop   = code == 6;
	char msg[512]   = "";
	char desc[512]  = ""; 
	char *descPTR   = desc;


	// Find command name
	char *cmd = "";
	for (int i=0;  cmdsList[i].id!=-1;  i++)
		if (cmdsList[i].id == CommandID) {
			cmd = cmdsList[i].cmd; 
			break;
		}


	// SHFileOp error description
	if (shfileop) {
		switch (secondaryCode) {
			case 0x71 : strcpy(desc,"The source and destination files are the same file."); break;
			case 0x72 : strcpy(desc,"Multiple file paths were specified in the source buffer, but only one destination file path."); break;
			case 0x73 : strcpy(desc,"Rename operation was specified but the destination path is a different directory. Use the move operation instead."); break;
			case 0x74 : strcpy(desc,"The source is a root directory, which cannot be moved or renamed."); break;
			case 0x75 : strcpy(desc,"The operation was canceled by the user, or silently canceled."); break;
			case 0x76 : strcpy(desc,"The destination is a subtree of the source."); break;
			case 0x78 :	strcpy(desc,"Security settings denied access to the source."); break;
			case 0x79 : strcpy(desc,"The source or destination path exceeded or would exceed MAX_PATH."); break;
			case 0x7A : strcpy(desc,"The operation involved multiple destination paths, which can fail in the case of a move operation."); break;
			case 0x7C : strcpy(desc,"The path in the source or destination or both was invalid."); break;
			case 0x7D : strcpy(desc,"The source and destination have the same parent folder."); break;
			case 0x7E : strcpy(desc,"The destination path is an existing file."); break;
			case 0x80 : strcpy(desc,"The destination path is an existing folder."); break;
			case 0x81 : strcpy(desc,"The name of the file exceeds MAX_PATH."); break;
			case 0x82 : strcpy(desc,"The destination is a read-only CD-ROM, possibly unformatted."); break;
			case 0x83 : strcpy(desc,"The destination is a read-only DVD, possibly unformatted."); break;
			case 0x84 : strcpy(desc,"The destination is a writable CD-ROM, possibly unformatted."); break;
			case 0x85 : strcpy(desc,"The file involved in the operation is too large for the destination media or file system."); break;
			case 0x86 : strcpy(desc,"The source is a read-only CD-ROM, possibly unformatted."); break;
			case 0x87 : strcpy(desc,"The source is a read-only DVD, possibly unformatted."); break;
			case 0x88 : strcpy(desc,"The source is a writable CD-ROM, possibly unformatted."); break;
			case 0xB7 : strcpy(desc,"MAX_PATH was exceeded during the operation."); break;
			case 0x402 : strcpy(desc,"An unknown error occurred. This is typically due to an invalid path in the source or destination."); break;
			case 0x10000 : strcpy(desc,"An unspecified error occurred on the destination."); break;
			case 0x10074 : strcpy(desc,"Destination is a root directory and cannot be renamed."); break;

			default : shfileop=false;
		}
	}


	// Fwatch error description
	if (code!=5  &&  code!=6  &&  code!=7  &&  code!=20 && code!=22 && code!=23) {
		switch(code) {
			// General 0-99
			case 0: sprintf(desc, ""); break;
			case 1: sprintf(desc, "Unknown command"); break;
			case 2: sprintf(desc, "Couldn't find %s process", str1); break;
			case 3: sprintf(desc, "Game Window not in front"); break;
			case 4: sprintf(desc, "Command not allowed on the dedicated server"); break;
			case 5: sprintf(desc, "Winapi"); break;
			case 6: sprintf(desc, "SHFileOperation"); break;
			case 7: sprintf(desc, "Errno"); break;

			// Memory
			case 10: sprintf(desc, "Failed to allocate memory block %s %d bytes", str1, num1); break;
			case 11: sprintf(desc, "Failed to reallocate memory block %s %d bytes", str1, num1); break;
			case 12: sprintf(desc, "Failed to perform str_replace %s %d bytes", str1, num1); break;

			// Clipboard
			case 20: sprintf(desc, "Couldn't open clipboard"); break;
			case 21: sprintf(desc, "Clipboard data incompatible format"); break;
			case 22: sprintf(desc, "Couldn't clear clipboard"); break;
			case 23: sprintf(desc, "Couldn't copy text to clipboard"); break;
			case 24: sprintf(desc, "Clipboard is empty"); break;
			case 25: sprintf(desc, "Invalid preferred effect value"); break;

			// Parameters 100-199
			case 100: sprintf(desc, "Not enough parameters %d/%d", num1,num2); break;
			case 101: sprintf(desc, "Parameter is less than zero %s=%d", str1,num1); break;
			case 102: sprintf(desc, "Parameter is zero or less %s=%d", str1,num1); break;
			case 103: sprintf(desc, "Parameter is one or less %s=%d", str1,num1); break;
			case 104: sprintf(desc, "Range start is larger than range end %d/%d",num1,num2); break;
			case 105: sprintf(desc, "Path leads outside source directory"); break;
			case 106: sprintf(desc, "Action was not specified"); break;
			case 107: sprintf(desc, "Parameter(s) are empty: %s", str1); break;
			case 108: sprintf(desc, "Restricted location"); break;

			// File 200-299
			case 200: sprintf(desc, "File is empty"); break;
			case 201: sprintf(desc, "File is not a directory"); break;
			case 202: sprintf(desc, "Moving file from outside to the tmp directory"); break;
			case 203: sprintf(desc, "Couldn't find variable %s", str2); break;
			case 204: sprintf(desc, "Couldn't find line %d", num1); break;
			case 205: sprintf(desc, "Couldn't append to %s too long line (%d chars)", str2,num1); break;
			case 206: sprintf(desc, "Couldn't replace line %d with %d (end of file)",num1,num2); break;
			case 207: sprintf(desc, "File already exists"); break;
			case 208: sprintf(desc, "Directory already exists"); break;
			case 209: sprintf(desc, "Read only %d bytes from a %d file",num1, num2); break;
			case 210: sprintf(desc, "Written only %d bytes from a %d buffer",num1, num2); break;

			// Classes
			case 250: sprintf(desc, "Couldn't find parent class %s %d/%d",str2,num1,num2); break;
			case 251: sprintf(desc, "Class %s already exists", str2); break;
			case 252: sprintf(desc, "Couldn't find class %s", str2); break;
			case 253: sprintf(desc, "Couldn't find property %s", str2); break;
			case 254: sprintf(desc, "Couldn't find item %d in array %s",num1,str2); break;
			case 255: sprintf(desc, "Property %s is not an array",str2); break;

			default: sprintf(desc, "Unknown error %d",code); break;
		}
	}
	else
		// Winapi error description
		if (code==5  ||  code==6  &&  !shfileop  ||  code==20  ||  code==22  ||  code==23) {
			FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			secondaryCode, 
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR) &lpMsgBuf,
			0,
			NULL);
			descPTR   = (char*)lpMsgBuf;
		} else
			// Errno error description
			if (code == 7) {
				switch(secondaryCode) {
					case EPERM: sprintf(desc, "Operation not permitted"); break;
					case ENOENT: sprintf(desc, "No such file or directory"); break;
					case ESRCH: sprintf(desc, "No such process"); break;
					case EINTR: sprintf(desc, "Interrupted function call"); break;
					case EIO: sprintf(desc, "Input/output error"); break;
					case ENXIO: sprintf(desc, "No such device or address"); break;
					case E2BIG: sprintf(desc, "Argument list too long"); break;
					case ENOEXEC: sprintf(desc, "Exec format error"); break;
					case EBADF: sprintf(desc, "Bad file descriptor"); break;
					case ECHILD: sprintf(desc, "No child processes"); break;
					case EAGAIN: sprintf(desc, "Resource temporarily unavailable"); break;
					case ENOMEM: sprintf(desc, "Not enough memory available for the attempted operator"); break;
					case EACCES: sprintf(desc, "Permission denied"); break;
					case EFAULT: sprintf(desc, "Bad address"); break;
					case EBUSY: sprintf(desc, "Device or resource busy"); break;
					case EEXIST: sprintf(desc, "File exists"); break;
					case EXDEV: sprintf(desc, "Cross-device link. An attempt was made to move a file to a different device"); break;
					case ENODEV: sprintf(desc, "No such device"); break;
					case ENOTDIR: sprintf(desc, "Not a directory"); break;
					case EISDIR: sprintf(desc, "Is a directory"); break;
					case EINVAL: sprintf(desc, "Invalid argument"); break;
					case ENFILE: sprintf(desc, "Too many open files in system"); break;
					case EMFILE: sprintf(desc, "Too many open files"); break;
					case ENOTTY: sprintf(desc, "Inappropriate I/O control operation"); break;
					case EFBIG: sprintf(desc, "File too large"); break;
					case ENOSPC: sprintf(desc, "No space left on device"); break;
					case ESPIPE: sprintf(desc, "Invalid seek"); break;
					case EROFS: sprintf(desc, "Read-only file system"); break;
					case EMLINK: sprintf(desc, "Too many links"); break;
					case EPIPE: sprintf(desc, "Broken pipe"); break;
					case EDOM: sprintf(desc, "Invalid math argument"); break;
					case ERANGE: sprintf(desc, "Result too large"); break;
					case EDEADLK: sprintf(desc, "Command would cause a deadlock"); break;
					case ENAMETOOLONG: sprintf(desc, "Filename too long"); break;
					case ENOLCK: sprintf(desc, "Too many segment locks open, lock table is full"); break;
					case ENOSYS: sprintf(desc, "Function not supported"); break;
					case ENOTEMPTY: sprintf(desc, "Directory not empty"); break;
					case EILSEQ: sprintf(desc, "Illegal sequence of bytes"); break;

					default: sprintf(desc, "Unknown error %d", secondaryCode); break;
				}
			}



	// Special formatting for a command that features multiple error arrays
	if (ErrorWithinError) {
		sprintf(msg, "[%s,%d,%d,\"", getBool(!code), code, secondaryCode);

		if (code != 0)
			sprintf(msg, "%s%s - ", msg, cmd);

		QWrite(msg, out);
		PrintDoubleQ(descPTR, out);

		// If file error then add filename
		if (code==7  ||  code==105  ||  code>=200)
			QWrite(" - ", out),
			PrintDoubleQ(str1, out);

		QWrite("\"]", out);

		if (lpMsgBuf != NULL)
			LocalFree(lpMsgBuf);

		return;
	}


	// If this function was used before then semi-colon is required
	if (firstErrorMSG)
		firstErrorMSG = 0;
	else
		QWrite(";", out);


	// Set separate _fwatch_error variable for these commands:
	if (CommandID == C_FILE_READ ||
		CommandID == C_FILE_READ2 ||
		CommandID == C_FILE_WRITE ||
		CommandID == C_FILE_AWRITE
		)
	{
		if (!code)
			QWrite("_fwatch_error=[true,0,0,\"\"];", out);
		else {
			sprintf(msg, "_fwatch_error=[%s,%d,%d,\"", getBool(!code), code, secondaryCode);
			QWrite(msg, out);

			if (code != 0)
				sprintf(msg, "%s - ", cmd),
				QWrite(msg, out);

			PrintDoubleQ(descPTR, out);

			// If file error then add filename
			if (code==7  ||  code==105  ||  code==108  ||  code>=200)
				QWrite(" - ", out),
				PrintDoubleQ(str1, out);
		
			QWrite("\"];", out);
		}
	}

	// Normal error array
	else {
		if (!SuppressNextError) {
			// first items are error codes
			sprintf(msg, "[%s,%d,%d,\"", getBool(!code), code, secondaryCode);
			QWrite(msg, out);

			// format description
			if (code > 0) {
				QWrite(cmd, out);
				QWrite(" - ", out);
				PrintDoubleQ(descPTR, out);

				// If file error then add filename
				if 
				(
					code == 7    &&  (CommandID!=C_IGSE_RENAME || CommandID==C_IGSE_RENAME && secondaryCode!=17) ||
					code == 105  ||  
					code == 108  ||  
					code >= 200  ||
					(CommandID == C_IGSE_COPY  ||  C_IGSE_NEWFILE)  &&  secondaryCode == 2  ||  
					CommandID == C_IGSE_LIST						&&  (secondaryCode == 2 || secondaryCode == 3)
				)
					QWrite(" - ", out),
					PrintDoubleQ(str1, out);

				// Destination file already exists
				if 
				(
					CommandID == C_IGSE_COPY    &&  secondaryCode==80  ||
					CommandID == C_IGSE_RENAME  &&  secondaryCode==17
				)
					QWrite(" - ", out),
					PrintDoubleQ(str2, out);
			}

			// If it's a command that doesn't return additional data then close array
			if (CommandID == C_CLIPBOARD_COPY || 
				CommandID == C_CLIPBOARD_TOFILE ||
				CommandID == C_CLIPBOARD_FROMFILE ||
				CommandID == C_CLIPBOARD_COPYFILE ||
				CommandID == C_CLIPBOARD_CUTFILE ||
				CommandID == C_FILE_RENAMEMISSIONS ||
				CommandID == C_IGSE_WRITE ||
				CommandID == C_IGSE_NEWFILE ||
				CommandID == C_IGSE_RENAME ||
				CommandID == C_IGSE_COPY ||
				CommandID == C_MEM_ERROR ||
				CommandID == C_CLASS_READSQM
				) 
				QWrite("\"]",out); 
			else 
				QWrite("\",",out);
		} else
			SuppressNextError = false;
	}


	// Log errors
	if (global.ErrorLog_Enabled  &&  code>0) {
		FILE *f        = fopen(!global.DedicatedServer ? "fwatch\\idb\\_errorLog.txt" : "fwatch\\idb\\_errorLogDedi.txt", "a");

		if (f) {
			HANDLE phandle = OpenProcess(PROCESS_ALL_ACCESS, 0, GetCurrentProcessId());
			SIZE_T stBytes = 0;

			WriterHeaderInErrorLog(&f, &phandle);

			// output command input
			fprintf(f, "\t%s\n", global.com_ptr);

			// Get mission time
			if (phandle != 0) {
				int missionTime = 0;
				int base		= !global.DedicatedServer ? (!global.CWA ? 0x7DD028 : 0x7CBFE8) : (!global.CWA ? 0x75A2E0 : 0x75A370);
				ReadProcessMemory(phandle, (LPVOID)base, &missionTime, 4, &stBytes);

				int seconds = missionTime / 1000;
				int minutes = seconds / 60;

				if (minutes < 10)
					fprintf(f, "0");

				fprintf(f, "%d:", minutes);

				if (seconds < 10)
					fprintf(f, "0");

				fprintf(f, "%d  ", seconds);
				CloseHandle(phandle);
			}

			// Trim error description
			for (unsigned int a=strlen(descPTR)-1;   descPTR[a]=='\r' || descPTR[a]=='\n';   a--)
				descPTR[a] = '\0';

			fprintf(f, "%d %d - %s - %s", code, secondaryCode, cmd, descPTR);

			// If file error then add filename
			if (code==7  ||  code==105  ||  code==108  ||  code>=200)
				fprintf(f, " - %s", str1);

			fprintf(f, "\n");
			fclose(f);
		}
	}

	if (lpMsgBuf != NULL)
		LocalFree(lpMsgBuf);
};





// v1.14 Like QWrite but doubles the quotes
void PrintDoubleQ(char *txt, HANDLE out)
{
	char *quote = strchr(txt, '\"');

	if (quote == NULL) 
		QWrite(txt, out); 
	else {
		char tmp[2] = "";

		for (unsigned int i=0; i<strlen(txt); i++) {
			if (txt[i] == '\"') 
				QWrite("\"", out);

			sprintf(tmp, "%c", txt[i]);
			QWrite(tmp, out);
		}
	}
};




// v1.14 Like QWrite but splits strings into parts of given length and doubles the quotes
void SplitStringIntoParts(char *txt, int cut, bool addComma, HANDLE out)
{
	int len = strlen(txt);
	
	if (len <= cut) {
		if (addComma) 
			QWrite(",",out); 
		else 
			addComma = true;

		QWrite("\""     , out),
		PrintDoubleQ(txt, out),
		QWrite("\""     , out);
	} else {
		char *txt2 = txt;

		for (int i=0;  i<len;  i+=cut, txt2=txt+i) {
			int len2 = strlen(txt2);

			if (cut > len2) 
				cut = len2;

			char prev = txt2[cut];
			txt2[cut] = '\0';

			if (addComma) 
				QWrite(",",out); 
			else 
				addComma = true;

			QWrite("\""      , out);
			PrintDoubleQ(txt2, out);
			QWrite("\""      , out);

			txt2[cut] = prev;
		}
	}
};







void ReadJoystick(char *data, int customJoyID)
{
	// If passed -2 then quit this function
	if (customJoyID == -2) {
		strcat(data, "[]");
		return;
	}
	
	
	JOYINFOEX joy;							// input
	JOYCAPS joyCaps;						// device
	joy.dwFlags     = JOY_RETURNBUTTONS;	// enable dwButtons
	int device      = 0;
	int input       = 0; 
	int joyID       = 0; 
	int maxButtons  = 32;
	char POVtype[7] = "NOPOV";

	if (customJoyID>=0  &&  customJoyID<=15)	// Optional argument - joy ID
		joyID = customJoyID;


	// Read input and device info starting from 0 to 15
	do {
		input = joyGetPosEx(joyID, &joy);

		// read device after successfully read input
		if (input == JOYERR_NOERROR) 
			device = joyGetDevCapsA(joyID, &joyCaps, sizeof(joyCaps));
		
		// end here if user passed arg OR successfully read both dev and inp OR reached ID limit
		if (customJoyID>=0  ||  device==JOYERR_NOERROR  &&  input==JOYERR_NOERROR  ||  joyID==15) 
			break;

		joyID++;
	}
	while (input != 0);


	// Return error value
	if (device!=JOYERR_NOERROR  ||  input!=JOYERR_NOERROR) {
		//strcat(data, "[\"couldn't find\"]");
		char info[100] = "";

		if (input == MMSYSERR_NODRIVER) 
			strcpy(info, "The joystick driver is not present.");

		if (input == MMSYSERR_INVALPARAM) 
			strcpy(info, "An invalid parameter was passed.");

		if (input == MMSYSERR_BADDEVICEID) 
			strcpy(info, "The specified joystick identifier is invalid.");

		if (input == JOYERR_UNPLUGGED) 
			strcpy(info, "The specified joystick is not connected to the system.");

		if (input == JOYERR_PARMS) 
			strcpy(info,"The specified joystick identifier is invalid.");

		sprintf(data, "%s[\"%s\",%d]", data, info, joyID);
		return;
	}


	//* Device information *//
	// Check type of POV and set a flag according to it
	if (joyCaps.wCaps & JOYCAPS_POV4DIR) {					// discrete values (digital POV)
		joy.dwFlags = JOY_RETURNBUTTONS | JOY_RETURNPOV;
		strcpy(POVtype, "DIGITAL");
	}

	if (joyCaps.wCaps & JOYCAPS_POVCTS) {					// continuous values (analog POV)
		joy.dwFlags = JOY_RETURNBUTTONS | JOY_RETURNPOVCTS;
		strcpy(POVtype, "ANALOG");
	}




	//* Read input *//
	// ARRAY WITH AXES NAMES ==========================================================
	int Axes[6];
	int count_axis = 2;

	Axes[0] = joy.dwXpos;	// always read basic X,Y
	Axes[1] = joy.dwYpos;

	strcat(data, "[[\"X\",\"Y\"");

	// detect which axes joy has
	if (joyCaps.wCaps & JOYCAPS_HASZ) {
		strcat(data, ",\"Z\"");
		Axes[count_axis++] = joy.dwZpos;
	}

	if (joyCaps.wCaps & JOYCAPS_HASR) {
		strcat(data, ",\"R\"");
		Axes[count_axis++] = joy.dwRpos;
	}

	if (joyCaps.wCaps & JOYCAPS_HASU) {
		strcat(data, ",\"U\"");
		Axes[count_axis++] = joy.dwUpos;
	}

	if (joyCaps.wCaps & JOYCAPS_HASV) {
		strcat(data, ",\"V\"");
		Axes[count_axis++] = joy.dwVpos;
	}
	// ================================================================================


	// ARRAY WITH AXES ================================================================
	bool ADDcomma = false;

	strcat(data, "],[");

	for (int i=0; i<count_axis; i++) {		// Loop each axis
		// format array
		if (ADDcomma) 
			strcat(data, ","); 
		else 
			ADDcomma = true;

		sprintf(data, "%s%d", data, Axes[i]);
	}
	// ================================================================================


	// ARRAY WITH BUTTONS =============================================================
	int Buttons[32];

	maxButtons = joyCaps.wNumButtons;
	sprintf(data, "%s],%d,[", data, maxButtons);

	ADDcomma = false;
	int buts = joy.dwButtons;

	for (i=0; i<maxButtons; i++) {	// Loop each button
		Buttons[i] = buts & (1 << i);

		if (Buttons[i] > 0) {
			if (ADDcomma) 
				strcat(data, ","); 
			else 
				ADDcomma = true;

			sprintf(data, "%s\"JOY%d\"", data, i+1);
		}
	}
	// ================================================================================


	// POV ============================================================================
	if (strcmp(POVtype,"NOPOV") != 0) {
		if (joy.dwPOV!=65535  &&  ADDcomma) 
			strcat(data, ",");

		switch (joy.dwPOV) {
			case 0     : strcat(data, "\"JOYPOVUP\""); break;
			case 4500  : strcat(data, "\"JOYPOVUPLEFT\""); break;
			case 9000  : strcat(data, "\"JOYPOVLEFT\""); break;
			case 13500 : strcat(data, "\"JOYPOVDOWNLEFT\""); break;
			case 18000 : strcat(data, "\"JOYPOVDOWN\""); break;
			case 22500 : strcat(data, "\"JOYPOVDOWNRIGHT\""); break;
			case 27000 : strcat(data, "\"JOYPOVRIGHT\""); break;
			case 31500 : strcat(data, "\"JOYPOVUPRIGHT\""); break;
		}
	}

	sprintf(data, "%s],[\"%s\",%d]", data, POVtype, joy.dwPOV);
	// ================================================================================


	// ARRAY WITH JOY INFORMATION =====================================================
	sprintf(data, "%s,[%d,%d,%d]]", data, joyID, joyCaps.wMid, joyCaps.wPid);
	// ================================================================================
}







// Write mission path information
void createPathSqf(LPCSTR lpFileName, int len, int offset)
{
	int ticks = GetTickCount();

	// Don't write multiple times at once
	if (ticks < global.mission_path_savetime+1000)
		return;

	global.mission_path_savetime = ticks;	
	
	FILE *f      = fopen(!global.DedicatedServer ? "fwatch\\data\\path.sqf" : "fwatch\\data\\pathDedi.sqf", "w");
	int pathType = -1;


	// Find mission type
	// Campaign
	if (!strncmp("campaigns", lpFileName, 9)) 
		pathType = 7;
	
	
	// Mission Editor
	if (!strncmp("Users\\", lpFileName, 6)) {
		char *nextBackSlash = strchr(lpFileName+6, '\\');

		if (nextBackSlash != NULL) {
			if (!strncmp("missions", nextBackSlash+1, 8)) 
				pathType = 4;

			if (!strncmp("missions\\.", nextBackSlash+1, 10)) 
				pathType = 3;

			if (!strncmp("mpmissions", nextBackSlash+1, 10)) 
				pathType = 0;
		}
	}


	// SP
	if (!strncmp("missions", lpFileName, 8)) 
		pathType = 6;

	if (!strncmp("missions\\__cur_sp.", lpFileName, 18)) 
		pathType = 5;


	// MP
	if (!strncmp("mpmissions", lpFileName, 10)) 
		pathType = 2;

	if (!strncmp("mpmissions\\__cur_mp.", lpFileName, 20)) 
		pathType = 1;


	// mission is PBO and briefing in wanted language
	if (!strncmp(".pbo", lpFileName+len-4, 4)) {
		if (!strncmp("missions", lpFileName, 8)) 
			pathType = 5;

		if (!strncmp("mpmissions", lpFileName, 10)) 
			pathType = 1;
	}
		

	// Mission path length
	int len2 = offset<0 ? len+offset : offset;

	fprintf(f,"[%d,%d,[\"", pathType, len2);

	// output mission path
	strcpy(global.mission_path, "");

	for (int i=0; i<len2; i++)
		if (lpFileName[i]=='\\'  ||  lpFileName[i]=='/') {
			if (i != len2-1)
				fprintf(f, "\",\"");

			strcat(global.mission_path, "\\");
		} else {
			fprintf(f, "%c", lpFileName[i]);
			sprintf(global.mission_path, "%s%c", global.mission_path, lpFileName[i]);
		}

	fprintf(f, "\"]]");
	fclose(f);

	
	// Is this is a new mission?
	if (strcmp(global.mission_path_previous, global.mission_path) !=0)
		strcpy(global.mission_path_previous, global.mission_path);
	else {
		// Is this is a mission restart?
		// Check mission time
		HANDLE phandle  = OpenProcess(PROCESS_ALL_ACCESS, 0, GetCurrentProcessId());

		if (phandle != 0) {
			int time_address = !global.DedicatedServer ? (!global.CWA ? 0x7DD028 : 0x7CBFE8) : (!global.CWA ? 0x75A2E0 : 0x75A370);
			int time_value   = 0;
			SIZE_T stBytes   = 0;

			ReadProcessMemory(phandle, (LPVOID)time_address, &time_value, 4, &stBytes);
			CloseHandle(phandle);

			if (time_value > 500)
				return;
		}
	}

	// Reset error logging
	global.ErrorLog_Enabled = false;
	global.ErrorLog_Started = false;
	NotifyFwatchAboutErrorLog();

	// Restore what was modified by Fwatch commands
	RestoreMemValues(pathType==3 || pathType==4);
}







// Check if string is empty
bool IsWhiteSpace(char *txt)
{
	while (*txt != '\0') {
		if (!isspace(*txt)) 
			return false;

		txt++;
	}

	return true;
}







// Interpret bool value from a text
bool String2Bool(char *txt)
{
	txt = Trim(txt);

	if (strcmpi(txt,"false") == 0  ||  strcmpi(txt,"") == 0)
		return false;

	if (strcmpi(txt,"true") == 0)
		return true;

	if (atoi(txt))
		return true;
	else
		return false;
}






// Return first two words from a text line
void GetFirstTwoWords(char* text, char *buffer, int maxSize)
{
	int words     = 0;
	int wordStart = -1;
	int size      = 0;
	int length    = strlen(text);

	for (int i=0;  i<=length && words<2;  i++)
		if (isspace(text[i])  ||  i==length) {			
			if (wordStart >= 0) {
				if (size + i-wordStart + 1 >= maxSize)
					break;

				strncat(buffer, text+wordStart, i-wordStart);

				if (words == 0)
					strcat(buffer, " ");

				size += i-wordStart + 1;
				wordStart = -1;
				words++;
			}
		} else
			if (wordStart < 0)
				wordStart = i;

	for (int j=0;  j<maxSize; j++)
		buffer[j] = tolower(buffer[j]);
}







// Similar to strtok2 but for the named argument system
char* strtok3(char* str, int CommandID) 
{
	char c;
	int i							= 0;
	int y							= 0;
	int QuotePos					= -1;
	int wordStart					= -1;
	bool inQuote					= false;
	bool newWord					= true;
	bool waitForSpecialSeparator	= false;
	bool prevCharWasSpace			= false;


	// For each character
	do {
		c = str[i];

		// Entered/left quote
		if (c=='"'  &&  (!inQuote && str[i-1]=='\a' || inQuote && (str[i+1]==' ' || str[i+1]=='\0'))  &&  !waitForSpecialSeparator) {
			inQuote = !inQuote;

			if (QuotePos < 0)
				QuotePos = i;
			else
				str[QuotePos] = '\a',
				str[i]		  = '\a',
				QuotePos	  = -1;
		}


		// Special separator
		if (c == 0x1F) {
			c = ' ';
			waitForSpecialSeparator = false;
		}


		// Replace spaces with \a
		if (c==' '  &&  !inQuote) {
			if (!waitForSpecialSeparator)
				str[i]			 = '\a',
				prevCharWasSpace = true;
		} else
			if (prevCharWasSpace  &&  c!='\"') {
				prevCharWasSpace = false;

				char c2;
				int j	= i-1;

				do
					c2 = str[++j];
				while 
					(c2!='\0'  &&  c2!=' '  &&  c2!=':');

				if 
				(
					strncmpi(str+i,"text:"     ,j-i+1) == 0 ||
					strncmpi(str+i,"text1:"    ,j-i+1) == 0 || 
					strncmpi(str+i,"text2:"    ,j-i+1) == 0 || 
					strncmpi(str+i,"prefix:"   ,j-i+1) == 0 || 
					strncmpi(str+i,"suffix:"   ,j-i+1) == 0 || 
					strncmpi(str+i,"find:"     ,j-i+1) == 0  && CommandID != C_CLASS_READ && CommandID != C_CLASS_READ2 ||
					strncmpi(str+i,"replace:"  ,j-i+1) == 0 ||
					strncmpi(str+i,"delimiter:",j-i+1) == 0 ||
					strncmpi(str+i,"merge:"    ,j-i+1) == 0 ||
					(CommandID == C_STRING_RANGE2 &&  strncmpi(str+i,"startfind",j-i+1) == 0) ||
					(CommandID == C_STRING_RANGE2 &&  strncmpi(str+i,"endfind"  ,j-i+1) == 0) ||
					(CommandID == C_CLASS_MODTOK  &&  strncmpi(str+i,"add:"	    ,j-i+1) == 0) ||
					(CommandID == C_CLASS_MODTOK  &&  strncmpi(str+i,"append:"  ,j-i+1) == 0) ||
					(CommandID == C_IGSE_DB       &&  strncmpi(str+i,"key:"	    ,j-i+1) == 0) ||
					(CommandID == C_IGSE_DB       &&  strncmpi(str+i,"read:"    ,j-i+1) == 0) ||
					(CommandID == C_IGSE_DB       &&  strncmpi(str+i,"write:"   ,j-i+1) == 0) ||
					(CommandID == C_IGSE_DB       &&  strncmpi(str+i,"append:"  ,j-i+1) == 0) ||
					(CommandID == C_IGSE_DB       &&  strncmpi(str+i,"rename:"  ,j-i+1) == 0) ||
					(CommandID == C_IGSE_DB       &&  strncmpi(str+i,"remove:"  ,j-i+1) == 0)
				)
					waitForSpecialSeparator=true;
			}

		i++;
	} while (c != '\0');

	i = 0; 
	y = 0;

	// Remove double-null characters
	do {
		str[y] = str[i];

		if (str[i] != '\0')
			if(str[i]!='\a'  ||  str[i+1]!='\a')
				y++;

		i++;
	} while (str[y] != '\0');

	return str;
}








// If numbers out of bounds then fix them
void CorrectStringPos(int *start, int *end, int length, bool endSet, bool lengthSet, int textSize)
{
	// If end argument was not given then assume end of the string
	if (!endSet)
		*end = textSize;

	// Convert relative value to absolute
	if (lengthSet)
		*end = *start + length;

	// If range coordinates are out of bounds
	if (*start < 0)
		*start += textSize;

	if (*start < 0)
		*start = 0;

	if (*start > textSize)
		*start = textSize;

	if (*end < 0)
		*end += textSize;

	if (*end < 0)
		*end = 0;

	if (*end > textSize)
		*end = textSize;
}







// Return character number type
int GetCharType(char c)
{
	if (c=='\r'  ||  c=='\n')
		return 0;
	else
		if (c<0x20  ||  c==' ')
			return 1;
		else
			if (c>=0x80  ||  isalnum(c)  ||  c=='_')
				return 2;
			else
				return 3;
}







// Restore memory values that were changed in the game
void RestoreMemValues(bool isMissionEditor)
{
	int max_loops  = (sizeof(global.restore_memory) / sizeof(global.restore_memory[0]));
	HANDLE phandle = NULL;
	SIZE_T stBytes = 0;
	phandle        = OpenProcess(PROCESS_ALL_ACCESS, 0, GetCurrentProcessId());

	if (phandle == NULL)
		return;

	for (int i=0; i<max_loops; i++) {
		if (!global.restore_memory[i])
			continue;

		if (i == RESTORE_BRIGHTNESS) {	
			int base    = !global.CWA ? 0x789D88 : 0x778E80;
			int pointer = 0;

			ReadProcessMemory(phandle, (LPVOID)base, &pointer, 4, &stBytes);
			WriteProcessMemory(phandle,(LPVOID)(pointer+0x2C),  &global.restore_float[FLOAT_BRIGHTNESS], 4, &stBytes);
		}

		if (i>=RESTORE_OBJECT_SHADOWS  &&  i<=RESTORE_CLOUDLETS) {
			int base    = !global.CWA ? 0x79F8D0 : 0x78E9C8;
			int	pointer = 0;
			int offset  = 0;

			if (i == RESTORE_VEHICLE_SHADOWS) 
				offset = 1;

			if (i == RESTORE_CLOUDLETS) 
				offset = 2;

			ReadProcessMemory(phandle, (LPVOID)base, &pointer, 4, &stBytes);
			WriteProcessMemory(phandle, (LPVOID)(pointer+0x5B0+offset), &global.restore_byte[BYTE_OBJECT_SHADOWS+offset], 1, &stBytes);
		}

		if (i>=RESTORE_BULLETS  &&  i<RESTORE_CADET) {
			int offset[] = {
				!global.DedicatedServer ? (!global.CWA ? 0x71D518 : 0x710520) : (!global.CWA ? 0x6ABDE8 : 0x6ABDA8), //gravity acceleration	9.8065996170043945
				!global.DedicatedServer ? (!global.CWA ? 0x5F1570 : 0x5F818C) : (!global.CWA ? 0x5374B8 : 0x537671), //bullet lifetime		3
				!global.DedicatedServer ? (!global.CWA ? 0x5F16D2 : 0x5F7ACB) : (!global.CWA ? 0x534F9C : 0x53517D), //shell  lifetime		20
				!global.DedicatedServer ? (!global.CWA ? 0x5F1527 : 0x4857B3) : (!global.CWA ? 0x533A91 : 0x533C4A), //rocket lifetime		10
				!global.DedicatedServer ? (!global.CWA ? 0x5F147B : 0x487867) : (!global.CWA ? 0x5357C7 : 0x5359A8), //bomb lifetime		120
				!global.DedicatedServer ? (!global.CWA ? 0x5F178F : 0x487986) : (!global.CWA ? 0x5371A1 : 0x53735A), //smoke lifetime		60
				!global.DedicatedServer ? (!global.CWA ? 0x5F12AD : 0x5F7D5D) : (!global.CWA ? 0x536D09 : 0x536EEA), //flare lifetime		17
				!global.DedicatedServer ? (!global.CWA ? 0x7137A0 : 0x7067A0) : (!global.CWA ? 0x6A66C0 : 0x6A66C0), //flare duration		15
				!global.DedicatedServer ? (!global.CWA ? 0x5F14BF : 0x485820) : (!global.CWA ? 0x5345F9 : 0x5347DA), //pipebomb lifetime	3.402823466E38 (7F7FFFFF)
				!global.DedicatedServer ? (!global.CWA ? 0x5F1818 : 0x5F789B) : (!global.CWA ? 0x5347F7 : 0x5349D8)  //timebomb lifetime	20
			};

			WriteProcessMemory(phandle, (LPVOID)offset[i-4], &global.restore_float[i-3], 4, &stBytes);	
		}

		if (i>=RESTORE_CADET  &&  i<RESTORE_RADAR) {
			int	offsets[][4] = {
				{0x7DD0C8, 0x7DD0D4, 0x75A380, 0x75A38C},	//ofp
				{0x7CC088, 0x7CC094, 0x75A410, 0x75A41C}	//cwa
			};

			int j    = !global.CWA ? 0 : 1;	// which game
			int k    = 0;				// which difficulty
			int base = RESTORE_CADET;

			if (i >= RESTORE_VETERAN) {
				k++;
				base = RESTORE_VETERAN;
			}

			if (global.DedicatedServer) 
				k += 2;

			int offset = offsets[j][k];

			WriteProcessMemory(phandle, (LPVOID)(offset+i-base), &global.restore_byte[i-11],  1, &stBytes);
		}

		if (i == RESTORE_RADAR)
			WriteProcessMemory(phandle,(LPVOID)(!global.CWA ? 0x7DD074 : 0x7CC034), &global.restore_float[FLOAT_RADAR], 4, &stBytes);

		if (i == RESTORE_MAX_OBJECTS)
			WriteProcessMemory(phandle,(LPVOID)(!global.CWA ? 0x7DD07C : 0x7CC03C), &global.restore_int[INT_MAX_OBJECTS], 4, &stBytes);

		if (i == RESTORE_TRACK1)
			WriteProcessMemory(phandle,(LPVOID)(!global.CWA ? 0x7DD080 : 0x7CC040), &global.restore_float[FLOAT_TRACK1], 4, &stBytes);

		if (i == RESTORE_TRACK2)
			WriteProcessMemory(phandle,(LPVOID)(!global.CWA ? 0x7DD084 : 0x7CC044), &global.restore_float[FLOAT_TRACK2], 4, &stBytes);

		if (i == RESTORE_MAX_LIGHTS)
			WriteProcessMemory(phandle,(LPVOID)(!global.CWA ? 0x7DD08C : 0x7CC04C), &global.restore_int[INT_MAX_LIGHTS], 4, &stBytes);

		if (i == RESTORE_TIDE) {
			int base = !global.DedicatedServer ? (!global.CWA ? 0x72F8E4 : 0x72295C) : (!global.CWA ? 0x6BE184 : 0x6BE144);
			WriteProcessMemory(phandle,(LPVOID)base, &global.restore_float[FLOAT_TIDE], 4, &stBytes);
		}

		if (i == RESTORE_WAVE) {
			int base = !global.DedicatedServer ? (!global.CWA ? 0x72F8E4 : 0x72295C) : (!global.CWA ? 0x6BE184 : 0x6BE144);
			WriteProcessMemory(phandle,(LPVOID)(base+4), &global.restore_float[FLOAT_WAVE], 4, &stBytes);
		}

		if (i == RESTORE_EXTCAMPOS) {
			int pointer[]	= {0x7894A0,0,0};
			int	modif[]		= {0x5C, 0x69C};
			int	max_loops	= (sizeof(pointer) / sizeof(pointer[0])) - 1;

			if (global.CWA) 
				pointer[0] = 0x778590,
				modif[1]   = 0x6A0;

			for (int i=0; i<max_loops; i++) {
				ReadProcessMemory(phandle, (LPVOID)pointer[i], &pointer[i+1], 4, &stBytes);
				pointer[i+1] = pointer[i+1] + modif[i];
			}
			
			WriteProcessMemory(phandle, (LPVOID)(global.extCamOffset+0), &global.restore_float[FLOAT_EXTCAMX], 4, &stBytes);
			WriteProcessMemory(phandle, (LPVOID)(global.extCamOffset+4), &global.restore_float[FLOAT_EXTCAMZ], 4, &stBytes);
			WriteProcessMemory(phandle, (LPVOID)(global.extCamOffset+8), &global.restore_float[FLOAT_EXTCAMY], 4, &stBytes);
		}

		if (i == RESTORE_WAVE_SPEED) {
			int base	= !global.DedicatedServer ? (!global.CWA ? 0x7B3ACC : 0x7A2C0C) : (!global.CWA ? 0x73392C : 0x7339C4);
			int pointer = 0;

			ReadProcessMemory (phandle, (LPVOID)base, &pointer, 4, &stBytes);
			WriteProcessMemory(phandle, (LPVOID)(pointer+0x2059C), &global.restore_float[FLOAT_WAVE_SPEED], 4, &stBytes);
		}

		if (!isMissionEditor) {
			if (i == RESTORE_FOVLEFT) {
				int base    = !global.CWA ? 0x789D88 : 0x778E80;
				int pointer = 0;

				ReadProcessMemory(phandle, (LPVOID)base, &pointer, 4, &stBytes);
				pointer += 0x40;

				WriteProcessMemory(phandle,(LPVOID)pointer, &global.restore_float[FLOAT_FOVLEFT], 4, &stBytes);
			}
		
			if (i == RESTORE_FOVTOP) {
				int base    = !global.CWA ? 0x789D88 : 0x778E80;
				int pointer = 0;

				ReadProcessMemory(phandle, (LPVOID)base, &pointer, 4, &stBytes);
				pointer += 0x40 + 4;

				WriteProcessMemory(phandle,(LPVOID)pointer, &global.restore_float[FLOAT_FOVTOP], 4, &stBytes);
			}
		
			if (i == RESTORE_UITOPLEFTX) {
				int base    = !global.CWA ? 0x789D88 : 0x778E80;
				int pointer = 0;

				ReadProcessMemory(phandle, (LPVOID)base, &pointer, 4, &stBytes);
				pointer += 0x40 + 8;

				WriteProcessMemory(phandle,(LPVOID)pointer, &global.restore_float[FLOAT_UITOPLEFTX], 4, &stBytes);
			}
		
			if (i == RESTORE_UITOPLEFTY)
			{
				int base    = !global.CWA ? 0x789D88 : 0x778E80;
				int pointer = 0;

				ReadProcessMemory(phandle, (LPVOID)base, &pointer, 4, &stBytes);
				pointer += 0x40 + 12;

				WriteProcessMemory(phandle,(LPVOID)pointer, &global.restore_float[FLOAT_UITOPLEFTY], 4, &stBytes);
			}
		
			if (i == RESTORE_UIBOTTOMRIGHTX) {
				int base    = !global.CWA ? 0x789D88 : 0x778E80;
				int pointer = 0;

				ReadProcessMemory(phandle, (LPVOID)base, &pointer, 4, &stBytes);
				pointer += 0x40 + 16;

				WriteProcessMemory(phandle,(LPVOID)pointer, &global.restore_float[FLOAT_UIBOTTOMRIGHTX], 4, &stBytes);
			}
		
			if (i == RESTORE_UIBOTTOMRIGHTY) {
				int base    = !global.CWA ? 0x789D88 : 0x778E80;
				int pointer = 0;

				ReadProcessMemory(phandle, (LPVOID)base, &pointer, 4, &stBytes);
				pointer += 0x40 + 20;

				WriteProcessMemory(phandle,(LPVOID)pointer, &global.restore_float[FLOAT_UIBOTTOMRIGHTY], 4, &stBytes);
			}
		}

		if (i >= RESTORE_HUD) {
			int pointer = 0;
			
			if ((i-RESTORE_HUD) >= CHAT_X) {
				switch(global_exe_version[global.exe_index]) {
					case VER_196 : pointer=0x7831B0; break;
					case VER_199 : pointer=0x7722A0; break;
					case VER_201 : pointer=global.exe_address+0x6FFCC0; break;
				}
			} else {
				switch(global_exe_version[global.exe_index]) {
					case VER_196 : pointer=0x79F8D0; break;
					case VER_199 : pointer=0x78E9C8; break;
					case VER_201 : pointer=global.exe_address+0x6D8240; break;
				}

				ReadProcessMemory(phandle, (LPVOID)(pointer+0x0), &pointer, 4, &stBytes);
				ReadProcessMemory(phandle, (LPVOID)(pointer+0x8), &pointer, 4, &stBytes);
			}

			if (pointer != 0) {
				if (IsNumberInArray((i-RESTORE_HUD),hud_int_list,sizeof(hud_int_list)/sizeof(hud_int_list[0])))
					WriteProcessMemory(phandle,(LPVOID)(pointer+hud_offset[i-RESTORE_HUD]), &global.restore_hud_int[i-RESTORE_HUD], 4, &stBytes);
				else
					WriteProcessMemory(phandle,(LPVOID)(pointer+hud_offset[i-RESTORE_HUD]), &global.restore_hud_float[i-RESTORE_HUD], 4, &stBytes);
			}
		}

		if (!isMissionEditor  ||  isMissionEditor && (i<RESTORE_FOVLEFT || i>RESTORE_UIBOTTOMRIGHTY))
			global.restore_memory[i] = 0;
	}

	CloseHandle(phandle);
}







/* -*- mode: c; c-file-style: "k&r" -*-
  strnatcmp.c -- Perform 'natural order' comparisons of strings in C.
  Copyright (C) 2000, 2004 by Martin Pool <mbp sourcefrog net>
  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.
  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:
  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/


/* partial change history:
 *
 * 2004-10-10 mbp: Lift out character type dependencies into macros.
 *
 * Eric Sosman pointed out that ctype functions take a parameter whose
 * value must be that of an unsigned int, even on platforms that have
 * negative chars in their default char type.
 */


/* These are defined as macros to make it easier to adapt this code to
 * different characters types or comparison functions. */
static inline int
nat_isdigit(nat_char a)
{
     return isdigit((unsigned char) a);
}


static inline int
nat_isspace(nat_char a)
{
     return isspace((unsigned char) a);
}


static inline nat_char
nat_toupper(nat_char a)
{
     return toupper((unsigned char) a);
}


static int
compare_right(nat_char const *a, nat_char const *b)
{
     int bias = 0;
     
     /* The longest run of digits wins.  That aside, the greatest
	value wins, but we can't know that it will until we've scanned
	both numbers to know that they have the same magnitude, so we
	remember it in BIAS. */
     for (;; a++, b++) {
	  if (!nat_isdigit(*a)  &&  !nat_isdigit(*b))
	       return bias;
	  if (!nat_isdigit(*a))
	       return -1;
	  if (!nat_isdigit(*b))
	       return +1;
	  if (*a < *b) {
	       if (!bias)
		    bias = -1;
	  } else if (*a > *b) {
	       if (!bias)
		    bias = +1;
	  } else if (!*a  &&  !*b)
	       return bias;
     }

     return 0;
}


static int
compare_left(nat_char const *a, nat_char const *b)
{
     /* Compare two left-aligned numbers: the first to have a
        different value wins. */
     for (;; a++, b++) {
	  if (!nat_isdigit(*a)  &&  !nat_isdigit(*b))
	       return 0;
	  if (!nat_isdigit(*a))
	       return -1;
	  if (!nat_isdigit(*b))
	       return +1;
	  if (*a < *b)
	       return -1;
	  if (*a > *b)
	       return +1;
     }

     return 0;
}


static int
strnatcmp0(nat_char const *a, nat_char const *b, int fold_case)
{
     int ai, bi;
     nat_char ca, cb;
     int fractional, result;

     ai = bi = 0;
     while (1) {
	  ca = a[ai]; cb = b[bi];

	  /* skip over leading spaces or zeros */
	  while (nat_isspace(ca))
	       ca = a[++ai];

	  while (nat_isspace(cb))
	       cb = b[++bi];

	  /* process run of digits */
	  if (nat_isdigit(ca)  &&  nat_isdigit(cb)) {
	       fractional = (ca == '0' || cb == '0');

	       if (fractional) {
		    if ((result = compare_left(a+ai, b+bi)) != 0)
			 return result;
	       } else {
		    if ((result = compare_right(a+ai, b+bi)) != 0)
			 return result;
	       }
	  }

	  if (!ca && !cb) {
	       /* The strings compare the same.  Perhaps the caller
                  will want to call strcmp to break the tie. */
	       return 0;
	  }

	  if (fold_case) {
	       ca = nat_toupper(ca);
	       cb = nat_toupper(cb);
	  }

	  if (ca < cb)
	       return -1;

	  if (ca > cb)
	       return +1;

	  ++ai; ++bi;
     }
}


int
strnatcmp(nat_char const *a, nat_char const *b) {
     return strnatcmp0(a, b, 0);
}


/* Compare, recognizing numeric string and ignoring case. */
int
strnatcasecmp(nat_char const *a, nat_char const *b) {
     return strnatcmp0(a, b, 1);
}


int VerifyPath(char **ptr_filename, String &str, int options, int CommandID, HANDLE out) 
{
	int i             = 0;
	int item_start    = 0;
	int item_index    = 0;
	int level         = 0;
	bool root_dir     = false;
	bool fwatch_dir   = false;
	bool illegal_dir  = false;
	bool download_dir = false;
	bool allowed_dir  = false;
	char *path        = *ptr_filename;
	
	static char allowed_dirs[][32] = {
		"in-game-script-editor",
		"flashpointcutscenemaker",
		"missioneditor3d",
		"@addontest",
		"set-pos-in-game"
	};
	int allowed_dirs_num = sizeof(allowed_dirs) / sizeof(allowed_dirs[0]);

	while(true) {
		if (path[i]=='\\' || path[i]=='/' || path[i]=='\0') {
			char backup = path[i];
			path[i]     = '\0';
			char *item  = path + item_start;

			if (strcmp(item,"..") == 0) {
				if (item_index == 0)
					root_dir = true;
				else {
					level--;

					if (level < 0) {
						path[i] = backup;
						break;
					}
					
					if (root_dir) {
						if (level == 0) {
							fwatch_dir  = false;
							allowed_dir = false;
						}
							
						if (level < 2)
							download_dir = false;
					}
				}
			} else
				if (backup != '\0')
					level++;
				
			if (root_dir && level==1) {
				if (strcmpi(item,"fwatch") == 0)
					fwatch_dir = true;
				else
					for(int j=0; j<allowed_dirs_num; j++)
						if (strcmpi(item,allowed_dirs[j]) == 0)
							allowed_dir = true;
			}

			if (fwatch_dir && level==2) {
				if (strcmpi(item,"data")==0 || strcmpi(item,"mdb")==0) {
					illegal_dir = true;
					path[i]     = backup;
					break;
				}

				if (strcmpi(item,"tmp") == 0)
					download_dir = true;
			}
			
			path[i] = backup;
			
			if (path[i] == '\0') {
				if (fwatch_dir && level==1)
					illegal_dir = true;

				break;
			}
				
			item_start = i + 1;
			item_index++;
		}
		
		i++;
	}
	
	

	if (illegal_dir) {
		if (~options & SUPPRESS_ERROR)
			FWerror(108,0,CommandID,path,"",0,0,out);
	} else
		if (root_dir && !fwatch_dir && !allowed_dir && options & RESTRICT_TO_MISSION_DIR) {
			if (~options & SUPPRESS_ERROR)
				FWerror(105,0,CommandID,path,"",0,0,out);
		} else
			if (level >= 0) {
				if (~options & SUPPRESS_CONVERSION)
					if (root_dir)
						*ptr_filename += 3;
					else {
						String_append(str, global.mission_path);
						String_append(str, *ptr_filename);
						*ptr_filename = str.pointer;
					}

				return download_dir ? DOWNLOAD_DIR : LEGAL_PATH;
			}

	return ILLEGAL_PATH;
}

//https://stackoverflow.com/questions/11413860/best-string-hashing-function-for-short-filenames
unsigned int fnv_hash(unsigned int hash, char* text, int text_length)
{
    for (int i=0; i<text_length; i++)
        hash = (hash*16777619) ^ text[i];

    return hash;
}

void PurgeComments(char *text, int string_start, int string_end) {
	int comment_start = -1;
	int comment_type  = 0;
	int new_end       = string_end;
	
	for (int j=string_start; text[j]!='\0'; j++) {
		if (comment_start == -1) {
			if (text[j]=='/' && text[j+1]=='*') {
				comment_start = j;
				comment_type  = 2;
			} else
				if (text[j]=='/' && text[j+1]=='/') {
					comment_start = j;
					comment_type  = 1;
				}
		} else {
			if (comment_type==2 && text[j]=='*' && text[j+1]=='/' || comment_type==1 && (text[j]=='\r' || text[j]=='\n')) {
				if (comment_type == 2)
					j += 2;
					
				memcpy(text+comment_start, text+j, new_end-j);
				
				new_end      -= j - comment_start;
				j            -= j - comment_start;
				text[new_end] = '\0';
				comment_type  = 0;
				comment_start = -1;
			}
		}
	}
}

char* Output_Nested_Array(char *temp, int level, char *output_strings_name, int j, int *subclass_count) {
	strcpy(temp,"");

	for (int z=0; z<level; z++)
		strcat(temp, "(");

	sprintf(temp, "%s_output_%s%d", temp, output_strings_name, j);

	for (z=0; z<level; z++)
		sprintf(temp, "%s select %d)", temp, subclass_count[z]);

	strcat(temp, " set[count ");

	for (z=0; z<level; z++)
		strcat(temp, "(");

	sprintf(temp, "%s_output_%s%d", temp, output_strings_name, j);

	for (z=0; z<level; z++)
		sprintf(temp, "%s select %d)", temp, subclass_count[z]);

	strcat(temp,",");

	return temp;
}

	// Delete file or directory with its contents  http://stackoverflow.com/a/10836193
int DeleteWrapper(char *refcstrRootDirectory)
{
	if (~GetFileAttributes(refcstrRootDirectory) & FILE_ATTRIBUTE_DIRECTORY)
		if (DeleteFile(refcstrRootDirectory))
			return 0;
		else
			return GetLastError();
	
	int             return_value = 0;
	bool            bDeleteSubdirectories = true;
	bool            bSubdirectory = false;       // Flag, indicating whether subdirectories have been found
	HANDLE          hFile;                       // Handle to directory
	String     	    strFilePath;                 // Filepath
	String          strPattern;                  // Pattern
	WIN32_FIND_DATA FileInformation;             // File information

	String_init(strFilePath);
	String_init(strPattern);
	String_append(strFilePath, refcstrRootDirectory);
	String_append(strFilePath, "\\");
	String_append(strPattern, refcstrRootDirectory);
	String_append(strPattern, "\\*.*");
	
	int saved_length = strFilePath.current_length;
	hFile = FindFirstFile(strPattern.pointer, &FileInformation);
	
	if (hFile != INVALID_HANDLE_VALUE) {
		do {
			if (FileInformation.cFileName[0] != '.') {
				strFilePath.current_length = saved_length;
				String_append(strFilePath, FileInformation.cFileName);

				if (FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
					if (bDeleteSubdirectories) {
						// Delete subdirectory
						int result = DeleteWrapper(strFilePath.pointer);
						if (result) {
							return_value = result;
							goto DeleteDirectory_return;
						}
					} else
						bSubdirectory = true;
				} else {
					// Set file attributes
					if (SetFileAttributes(strFilePath.pointer, FILE_ATTRIBUTE_NORMAL) == FALSE) {
						return_value = GetLastError();
						goto DeleteDirectory_return;
					}

					// Delete file
					if (DeleteFile(strFilePath.pointer) == FALSE) {
						return_value = GetLastError();
						goto DeleteDirectory_return;
					}
				}
			}
		}
		while (FindNextFile(hFile, &FileInformation) == TRUE);

		// Close handle
		FindClose(hFile);

		DWORD dwError = GetLastError();
		
		if (dwError != ERROR_NO_MORE_FILES) {
      		return_value = dwError;
			goto DeleteDirectory_return;
		} else {
			if (!bSubdirectory) {
				// Set directory attributes
				if (SetFileAttributes(refcstrRootDirectory, FILE_ATTRIBUTE_NORMAL) == FALSE) {
					return_value = GetLastError();
					goto DeleteDirectory_return;
				}

				// Delete directory
				if (RemoveDirectory(refcstrRootDirectory) == FALSE) {
					return_value = GetLastError();
					goto DeleteDirectory_return;
				}
			}
		}
	}

DeleteDirectory_return:
	String_end(strFilePath);
	String_end(strPattern);
	return return_value;
}


// Send message to fwatch.exe to enable/disable error logging there
void NotifyFwatchAboutErrorLog()
{
	HANDLE mailslot = CreateFile(TEXT("\\\\.\\mailslot\\fwatch_mailslot"), GENERIC_WRITE, FILE_SHARE_READ, (LPSECURITY_ATTRIBUTES)NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);
	
	if (mailslot != INVALID_HANDLE_VALUE) {
		char temp[32] = "";
		sprintf(temp, "6|%d|%d", global.ErrorLog_Enabled, global.ErrorLog_Started);

		DWORD bytes_written = 0;
		WriteFile(mailslot, temp, strlen(temp+1), &bytes_written, (LPOVERLAPPED)NULL);
		CloseHandle(mailslot);
	}
};


void WriterHeaderInErrorLog(void *ptr_logfile, void *ptr_phandle)
{
	FILE **logfile  = (FILE **)ptr_logfile;
	HANDLE *phandle = (HANDLE *)ptr_phandle;
	
	global.ErrorLog_Started = true;
	NotifyFwatchAboutErrorLog();

	SYSTEMTIME st;
	GetLocalTime(&st);
	
	fprintf(*logfile, "\n\n\n===========================================================================\nLog started on %d.", st.wYear);

	if (st.wMonth < 10)
		fprintf(*logfile, "0");

	fprintf(*logfile, "%d.", st.wMonth);

	if (st.wDay < 10)
		fprintf(*logfile, "0");

	fprintf(*logfile, "%d  ", st.wDay);
	
	if (st.wHour < 10)
		fprintf(*logfile, "0");

	fprintf(*logfile, "%d:", st.wHour);

	if (st.wMinute < 10)
		fprintf(*logfile, "0");

	fprintf(*logfile, "%d:", st.wMinute);

	if (st.wSecond < 10)
		fprintf(*logfile, "0");

	fprintf(*logfile, "%d\n", st.wSecond);


	if (*phandle != 0) {
		// mission name
		char buffer[256] = "";
		int base		 = !global.DedicatedServer ? (!global.CWA ? 0x7DD0E0 : 0x7CC0A0) : (!global.CWA ? 0x75A398 : 0x75A428);
		int pointer		 = 0;
		DWORD stBytes    = 0;
		ReadProcessMemory(*phandle, (LPVOID)base, &buffer, 80, &stBytes);

		// if packed pbo
		if (strcmp(buffer,"__cur_sp")==0  ||  strcmp(buffer,"__cur_mp")==0) {
			base = !global.DedicatedServer ? (!global.CWA ? 0x7DD180 : 0x7CC140) : (!global.CWA ? 0x75A438 : 0x75A4C8);

			ReadProcessMemory(*phandle, (LPVOID)base, &pointer, 4, &stBytes);

			if (pointer != 0)
				ReadProcessMemory(*phandle, (LPVOID)(pointer+0x8),			 &buffer,  255, &stBytes);
		}
	
		fprintf(*logfile, "%s", buffer);

		// island name
		base = !global.DedicatedServer ? (!global.CWA ? 0x7DD130 : 0x7CC0F0) : (!global.CWA ? 0x75A3E8 : 0x75A478);
		ReadProcessMemory(*phandle, (LPVOID)base, &buffer, 80, &stBytes);
		fprintf(*logfile, ".%s\t", buffer);

		// briefing title
		strcpy(buffer, "");
		base = !global.DedicatedServer ? (!global.CWA ? 0x78324C : 0x77233C) : (!global.CWA ? 0x7030AC : 0x7030FC);
		ReadProcessMemory(*phandle, (LPVOID)base, &pointer, 4,	 &stBytes);

		if (pointer != 0) 
			ReadProcessMemory(*phandle, (LPVOID)(pointer+0x114), &buffer, 255, &stBytes);
		else {
			base = !global.DedicatedServer ? (!global.CWA ? 0x786880 : 0x775968) : (!global.CWA ? 0x7066D8 : 0x706728);
			ReadProcessMemory(*phandle, (LPVOID)base, &pointer, 4,	 &stBytes);

			if (pointer != 0)
				ReadProcessMemory(*phandle, (LPVOID)(pointer+0x8), &buffer, 255, &stBytes);
		}

		fprintf(*logfile, "%s\n", buffer);
	}
}