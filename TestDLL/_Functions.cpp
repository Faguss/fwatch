// -----------------------------------------------------------------
// FUNCTION INVENTORY
// -----------------------------------------------------------------

// Remove quotation marks from string
char* String_trim_quotes(String &input) {
	if (input.text[0]=='"'  &&  input.text[input.length-1]=='"') {
		input.text++;
		input.length -= 2;
		input.text[input.length] = '\0';
	}

	return input.text;
}







void QWrite(const char *str) {
	QWritel(str, strlen(str));
}








// Return "TRUE" or "FALSE"
const char* getBool(short b) {
	if(b != 0)
		return "TRUE";
	else
		return "FALSE";
}








// Format special keys to string
void QWrite_format_key(int c) {
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

	switch(c) {
		case VK_LBUTTON: QWrite("\"LBUTTON\""); break; 
		case VK_RBUTTON: QWrite("\"RBUTTON\""); break;
		case VK_MBUTTON: QWrite("\"MBUTTON\""); break;
		case VK_CANCEL: QWrite("\"CANCEL\""); break;
		case VK_XBUTTON1 : QWrite("\"4BUTTON\""); break;  // 4th mouse button
		case VK_XBUTTON2 : QWrite("\"5BUTTON\""); break;  // 5th mouse button

		case VK_BACK: QWrite("\"BACKSPACE\""); break;
		case VK_TAB: QWrite("\"TAB\""); break;
		case VK_CLEAR: QWrite("\"CLEAR\""); break;        // numpad5 while numlock is off
		case VK_RETURN: QWrite("\"ENTER\""); break;

		case VK_SHIFT: QWrite("\"SHIFT\""); break;
		case VK_CONTROL: QWrite("\"CTRL\""); break;
		case VK_MENU: QWrite("\"ALT\""); break;
		case VK_PAUSE: QWrite("\"PAUSE\""); break;        // pause break
		case VK_CAPITAL: QWrite("\"CAPSLOCK\""); break;

		case VK_KANA: QWrite("\"KANA\""); break;
		case VK_JUNJA: QWrite("\"JUNJA\""); break;
		case VK_FINAL: QWrite("\"FINAL\""); break;
		case VK_HANJA: QWrite("\"HANJA\""); break;

		case VK_ESCAPE: QWrite("\"ESC\""); break;
		case VK_CONVERT : QWrite("\"CONVERT\""); break;
		case VK_NONCONVERT : QWrite("\"NONCONVERT\""); break;
		case VK_ACCEPT : QWrite("\"ACCEPT\""); break;
		case VK_MODECHANGE : QWrite("\"MODECHANGE\""); break;

		case VK_SPACE: QWrite("\"SPACE\""); break;
		case VK_PRIOR: QWrite("\"PAGEUP\""); break;
		case VK_NEXT: QWrite("\"PAGEDOWN\""); break;
		case VK_END: QWrite("\"END\""); break;
		case VK_HOME: QWrite("\"HOME\""); break;
		case VK_LEFT: QWrite("\"LEFT\""); break;
		case VK_RIGHT: QWrite("\"RIGHT\""); break;
		case VK_UP: QWrite("\"UP\""); break;
		case VK_DOWN: QWrite("\"DOWN\""); break;
		case VK_SELECT: QWrite("\"SELECT\""); break;
		case VK_PRINT: QWrite("\"PRINT\""); break;
		case VK_EXECUTE: QWrite("\"EXECUTE\""); break;
		case VK_SNAPSHOT: QWrite("\"PRINTSCREEN\""); break;
		case VK_INSERT: QWrite("\"INSERT\""); break;
		case VK_DELETE: QWrite("\"DELETE\""); break;
		case VK_HELP: QWrite("\"HELP\""); break;

		case 0x30 : QWrite("\"0\""); break;
		case 0x31 : QWrite("\"1\""); break;
		case 0x32 : QWrite("\"2\""); break;
		case 0x33 : QWrite("\"3\""); break;
		case 0x34 : QWrite("\"4\""); break;
		case 0x35 : QWrite("\"5\""); break;
		case 0x36 : QWrite("\"6\""); break;
		case 0x37 : QWrite("\"7\""); break;
		case 0x38 : QWrite("\"8\""); break;
		case 0x39 : QWrite("\"9\""); break;
		case 0x41 : QWrite("\"A\""); break;
		case 0x42 : QWrite("\"B\""); break;
		case 0x43 : QWrite("\"C\""); break;
		case 0x44 : QWrite("\"D\""); break;
		case 0x45 : QWrite("\"E\""); break;
		case 0x46 : QWrite("\"F\""); break;
		case 0x47 : QWrite("\"G\""); break;
		case 0x48 : QWrite("\"H\""); break;
		case 0x49 : QWrite("\"I\""); break;
		case 0x4A : QWrite("\"J\""); break;
		case 0x4B : QWrite("\"K\""); break;
		case 0x4C : QWrite("\"L\""); break;
		case 0x4D : QWrite("\"M\""); break;
		case 0x4E : QWrite("\"N\""); break;
		case 0x4F : QWrite("\"O\""); break;
		case 0x50 : QWrite("\"P\""); break;
		case 0x51 : QWrite("\"Q\""); break;
		case 0x52 : QWrite("\"R\""); break;
		case 0x53 : QWrite("\"S\""); break;
		case 0x54 : QWrite("\"T\""); break;
		case 0x55 : QWrite("\"U\""); break;
		case 0x56 : QWrite("\"V\""); break;
		case 0x57 : QWrite("\"W\""); break;
		case 0x58 : QWrite("\"X\""); break;
		case 0x59 : QWrite("\"Y\""); break;
		case 0x5A : QWrite("\"Z\""); break;

		case VK_LWIN: QWrite("\"LWIN\""); break;
		case VK_RWIN: QWrite("\"RWIN\""); break;
		case VK_APPS: QWrite("\"APPS\""); break;
		case VK_SLEEP: QWrite("\"SLEEP\""); break;

		case VK_NUMPAD0: QWrite("\"NUMPAD0\""); break;
		case VK_NUMPAD1: QWrite("\"NUMPAD1\""); break;
		case VK_NUMPAD2: QWrite("\"NUMPAD2\""); break;
		case VK_NUMPAD3: QWrite("\"NUMPAD3\""); break;
		case VK_NUMPAD4: QWrite("\"NUMPAD4\""); break;
		case VK_NUMPAD5: QWrite("\"NUMPAD5\""); break;
		case VK_NUMPAD6: QWrite("\"NUMPAD6\""); break;
		case VK_NUMPAD7: QWrite("\"NUMPAD7\""); break;
		case VK_NUMPAD8: QWrite("\"NUMPAD8\""); break;
		case VK_NUMPAD9: QWrite("\"NUMPAD9\""); break;
		case VK_MULTIPLY: QWrite("\"MULTIPLY\""); break;   // numpad asterisk	1.11 fixed
		case VK_ADD: QWrite("\"ADD\""); break;             // numpad plus
		case VK_SEPARATOR: QWrite("\"SEPARATOR\""); break;
		case VK_SUBTRACT: QWrite("\"SUBTRACT\""); break;   // numpad minus
		case VK_DECIMAL: QWrite("\"DECIMAL\""); break;     // numpad decimal mark
		case VK_DIVIDE: QWrite("\"DIVIDE\""); break;       // numpad slash

		case VK_F1: QWrite("\"F1\""); break;
		case VK_F2: QWrite("\"F2\""); break;
		case VK_F3: QWrite("\"F3\""); break;
		case VK_F4: QWrite("\"F4\""); break;
		case VK_F5: QWrite("\"F5\""); break;
		case VK_F6: QWrite("\"F6\""); break;
		case VK_F7: QWrite("\"F7\""); break;
		case VK_F8: QWrite("\"F8\""); break;
		case VK_F9: QWrite("\"F9\""); break;
		case VK_F10: QWrite("\"F10\""); break;
		case VK_F11: QWrite("\"F11\""); break;
		case VK_F12: QWrite("\"F12\""); break;
		case VK_F13: QWrite("\"F13\""); break;
		case VK_F14: QWrite("\"F14\""); break;
		case VK_F15: QWrite("\"F15\""); break;
		case VK_F16: QWrite("\"F16\""); break;
		case VK_F17: QWrite("\"F17\""); break;
		case VK_F18: QWrite("\"F18\""); break;
		case VK_F19: QWrite("\"F19\""); break;
		case VK_F20: QWrite("\"F20\""); break;
		case VK_F21: QWrite("\"F21\""); break;
		case VK_F22: QWrite("\"F22\""); break;
		case VK_F23: QWrite("\"F23\""); break;
		case VK_F24: QWrite("\"F24\""); break;

		case VK_NUMLOCK: QWrite("\"NUMLOCK\""); break;
		case VK_SCROLL: QWrite("\"SCROLLOCK\""); break;
		case VK_LSHIFT: QWrite("\"LSHIFT\""); break;
		case VK_RSHIFT: QWrite("\"RSHIFT\""); break;
		case VK_LCONTROL: QWrite("\"LCTRL\""); break;
		case VK_RCONTROL: QWrite("\"RCTRL\""); break;
		case VK_LMENU: QWrite("\"LALT\""); break;
		case VK_RMENU: QWrite("\"RALT\""); break;

		case VK_BROWSER_BACK: QWrite("\"WEBBACK\""); break;   // multimedia keyboard keys
		case VK_BROWSER_FORWARD: QWrite("\"WEBFORWARD\""); break;
		case VK_BROWSER_REFRESH: QWrite("\"WEBREFRESH\""); break;
		case VK_BROWSER_STOP: QWrite("\"WEBSTOP\""); break;
		case VK_BROWSER_SEARCH: QWrite("\"WEBSEARCH\""); break;
		case VK_BROWSER_FAVORITES: QWrite("\"WEBFAVORITES\""); break;
		case VK_BROWSER_HOME: QWrite("\"WEBHOME\""); break;
		case VK_VOLUME_MUTE: QWrite("\"VOLUMEMUTE\""); break;
		case VK_VOLUME_DOWN: QWrite("\"VOLUMEDOWN\""); break;
		case VK_VOLUME_UP: QWrite("\"VOLUMEUP\""); break;
		case VK_MEDIA_NEXT_TRACK: QWrite("\"MEDIANEXT\""); break;
		case VK_MEDIA_PREV_TRACK: QWrite("\"MEDIAPREV\""); break;
		case VK_MEDIA_STOP: QWrite("\"MEDIASTOP\""); break;
		case VK_MEDIA_PLAY_PAUSE: QWrite("\"MEDIAPLAY\""); break;
		case VK_LAUNCH_MAIL: QWrite("\"MAIL\""); break;
		case VK_LAUNCH_MEDIA_SELECT: QWrite("\"MEDIASELECT\""); break;
		case VK_LAUNCH_APP1: QWrite("\"MYCOMPUTER\""); break;
		case VK_LAUNCH_APP2: QWrite("\"CALCULATOR\""); break;

		case VK_OEM_1: QWrite("\";\""); break;      // semi-colon
		case VK_OEM_PLUS: QWrite("\"=\""); break;   // equality
		case VK_OEM_COMMA: QWrite("\",\""); break;  // comma
		case VK_OEM_MINUS: QWrite("\"-\""); break;  // minus
		case VK_OEM_PERIOD: QWrite("\".\""); break; // dot
		case VK_OEM_2: QWrite("\"/\""); break;      // slash
		case VK_OEM_3: QWrite("\"`\""); break;      // grave accent
		case VK_OEM_4: QWrite("\"[\""); break;      // open bracket
		case VK_OEM_5: QWrite("\"\\\""); break;     // backslash
		case VK_OEM_6: QWrite("\"]\""); break;      // close bracket
		case VK_OEM_7: QWrite("\"'\""); break;      // apostrophe

		case VK_OEM_8: QWrite("\"OEM8\""); break;
		case VK_OEM_102: QWrite("\"OEM102\""); break;
		case VK_ABNT_C1 : QWrite("\"ABNT_C1\""); break;
		case VK_ABNT_C2 : QWrite("\"ABNT_C2\""); break;
		case VK_PROCESSKEY: QWrite("\"PROCESS\""); break;
		case VK_PACKET: QWrite("\"PACKET\""); break;
		case VK_ATTN: QWrite("\"ATTN\""); break;
		case VK_CRSEL: QWrite("\"CRSEL\""); break;
		case VK_EXSEL: QWrite("\"EXSEL\""); break;
		case VK_EREOF: QWrite("\"ERASEEOF\""); break; 
		case VK_PLAY: QWrite("\"PLAY\""); break; 
		case VK_ZOOM: QWrite("\"ZOOM\""); break; 
		case VK_NONAME: QWrite("\"NONAME\""); break; 
		case VK_PA1: QWrite("\"PA1\""); break; 
		case VK_OEM_CLEAR: QWrite("\"OEMCLEAR\""); break;
	
		default: QWritef("\"0x%x\"", c);
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







//v1.12 Replace all occurrences in a string
// http://someboringsite.com/2009/09/ansi-c-string-replacement-function.html
char* str_replace(const char *strbuf, const char *strold, const char *strnew, int options) {	//TODO: remove this function on release because it's obsolete
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

	p = strstr2_old(strbuf, strlen(strbuf), strold, strlen(strold), options);

	if (!p) 
		return strdup(strbuf);

	while (n > 0) {
		if (!(p = strstr2_old(p+1, strlen(strbuf), strold, strlen(strold), options))) 
			break;

		n++;
	}

	strret = (char*) malloc(strlen(strbuf)-(n*szold)+(n*sznew)+1);

	if (!strret) 
		return NULL;

	p = strstr2_old(strbuf, strlen(strbuf), strold, strlen(strold), options);

	strncpy(strret, strbuf, (p-strbuf));

	strret[p-strbuf] = 0;
	posold           = p + szold;
	posnews          = strret + (p-strbuf);

	strcpy(posnews, strnew);
	posnews += sznew;

	while (n > 0) {
		if (!(p = strstr2_old(p+1, strlen(p+1), strold, strlen(strold), options))) 
			break;

		strncpy(posnews, posold, p-posold);

		posnews[p-posold] = 0;
		posnews          += (p-posold);

		strcpy(posnews, strnew);

		posnews += sznew;
		posold   = p+szold;
	}

	strcpy(posnews, posold);
	return strret;
}







//v1.13 Search for string insensitive
//http://www.codeguru.com/cpp/cpp/string/article.php/c5641/Case-Insensitive-strstr.htm
char* String_find(String &source, String &to_find, int options) {
	if (source.length==0 || to_find.length==0)
		return NULL;
	
	for (
		size_t pos=options & OPTION_REVERSE ? source.length-to_find.length : 0;
		options & OPTION_REVERSE ? true : pos<source.length;
		options & OPTION_REVERSE ? pos-- : pos++
	) {
		size_t pos_arg1 = pos;
		size_t pos_arg2 = 0;

		// Compare arg1+pos with arg2
		while (
			options & OPTION_CASESENSITIVE 
				? source.text[pos_arg1++] == to_find.text[pos_arg2++] 
				: (source.text[pos_arg1++] | 32) == (to_find.text[pos_arg2++] | 32)
		) {
			if (pos_arg2 == to_find.length) {
				if (~options & OPTION_MATCHWORD  
					||  
					// If matching the whole word then occurrence musn't be surrounded by graphic characters
					(options & OPTION_MATCHWORD && 
						// If left side of the match is empty
						(pos==0  ||  (pos>0  &&  !isalnum(source.text[pos-1])  &&  source.text[pos-1]!='_')) 
						&& 
						// If right side of the match is empty
						(pos+to_find.length>=source.length  ||  (pos+to_find.length<source.length  &&  !isalnum(source.text[pos+to_find.length])  &&  source.text[pos+to_find.length]!='_'))
					)
				) 
					return (char*)(source.text+pos);
				
				// If failed to match the whole word then move forward
				pos += to_find.length;
				break;
			}
		}

		if (pos==0 && options & OPTION_REVERSE)
			break;
	}

	return NULL;
}







// v1.13 Move given file to the system recycle bin
bool trashFile(String file, int error_behaviour) {
	StringDynamic buffer;
	StringDynamic_init(buffer);

	StringDynamic_appendl(buffer, global.game_dir, global.game_dir_length);
	StringDynamic_append(buffer, "\\");
	StringDynamic_appends(buffer, file);
	StringDynamic_appendl(buffer, "\0", 1);

	SHFILEOPSTRUCT shfos;
	shfos.hwnd   = NULL;
	shfos.wFunc  = FO_DELETE;
	shfos.pFrom  = buffer.text;
	shfos.pTo    = NULL;
	shfos.fFlags = FOF_SILENT | FOF_NOCONFIRMATION | FOF_ALLOWUNDO;
	int result   = SHFileOperation(&shfos);

	// Special mode - if file not found then return true
	if (error_behaviour==1  &&  (result==1026 || result==ERROR_FILE_NOT_FOUND)) 
		result = 0;

	StringDynamic_end(buffer);

	if (result == 0)	
		return true; 
	else {
		if (error_behaviour != -1)
			QWrite_err(FWERROR_SHFILEOP, 1, result);

		return false;
	}
}







// v1.13 Convert radians to degrees
double rad2deg(double num) {
	num *= 180 / 3.14159265;

	// if error then return zero
	if (_isnan(num))
		num = 0;

	return num;
}







// v1.13 Convert degrees to radians
double deg2rad(double num) {
	num *= 3.14159265 / 180;

	// if error then return zero
	if (_isnan(num))
		num = 0;

	return num;
}







// v1.13 Format time structure to a OFP array
void SystemTimeToString(SYSTEMTIME &st, bool systime, char *str) {
	TIME_ZONE_INFORMATION TimeZoneInfo;
	int result = GetTimeZoneInformation (&TimeZoneInfo);
	int dst    = result==TIME_ZONE_ID_DAYLIGHT ? TimeZoneInfo.DaylightBias : 0;

	sprintf(str, "[%d,%d,%d,%d,%d,%d,%d,%d,%ld,%s]",
		st.wYear, 
		st.wMonth, 
		st.wDay, 
		st.wDayOfWeek, 
		st.wHour, 
		st.wMinute, 
		st.wSecond, 
		st.wMilliseconds, 
		(TimeZoneInfo.Bias + dst) * -1, 
		getBool(systime)
	);
}







// v1.13 Convert time structure and format it
void FileTimeToString(FILETIME &ft, bool systime, char *str) {
	SYSTEMTIME st;

	// Convert FILETIME to computer local time
	if (!systime) {
		FILETIME ft2;
		FileTimeToLocalFileTime(&ft, &ft2);
		FileTimeToSystemTime(&ft2, &st);
	} else 
		FileTimeToSystemTime(&ft, &st);

	SystemTimeToString(st, systime, str);
}







// v1.13 Copy data to clipboard
bool CopyToClip(String &input, bool append) {
	if (input.length==0 && append)
		return true;

	if (!OpenClipboard(NULL)) {
		QWrite_err(FWERROR_CLIP_OPEN, 1, GetLastError());
		return false;
	}

	HANDLE clip_handle;
	char *clip      = NULL;
	int clip_length = 0;

	// Get existing text
	if (append) {
		// Validate data type
		if (!::IsClipboardFormatAvailable(CF_TEXT)) {
			QWrite_err(FWERROR_CLIP_FORMAT, 0);
			return false;
		}

		clip_handle = GetClipboardData(CF_TEXT);
		clip        = (char*)GlobalLock(clip_handle);
		clip_length = strlen(clip);
	}


	// Allocate new buffer for transfering text to the clipboard
	HGLOBAL clip_new_buffer = GlobalAlloc(GMEM_ZEROINIT|GMEM_FIXED, clip_length + input.length + 1);
	char *clip_new          = (char*)GlobalLock(clip_new_buffer);

	if (append) {
		memcpy(clip_new            , clip      , clip_length);
		memcpy(clip_new+clip_length, input.text, input.length);
	} else
		memcpy(clip_new, input.text, input.length);

	GlobalUnlock(clip_handle);

	// Clear clip
	if(!EmptyClipboard()) {
		QWrite_err(FWERROR_CLIP_CLEAR, 1, GetLastError());
		GlobalUnlock(clip_new_buffer);
		CloseClipboard();
		return false;
	}

	// Copy to clip
	HANDLE ok = ::SetClipboardData(CF_TEXT, clip_new_buffer);

	if (!ok) 
		QWrite_err(FWERROR_CLIP_COPY, 1, GetLastError());

	GlobalUnlock(clip_new_buffer);
	CloseClipboard();
	return ok ? true : false;
}







// v1.13 Custom handling of escape sequences
void String_escape_sequences(String input, int mode, int quantity) {
	bool shift = false;
	int count  = 0;

	for (size_t i=0;  i<input.length;  i++) {
		if (input.text[i] == '\\') {
			// Replace char
			if (mode==OPTION_TAB  &&  input.text[i+1]=='t') {
				count++;
				input.text[i] = '\t';
				shift_buffer_chunk(input.text, i+2, input.length+1, 1, OPTION_LEFT);
			}

			if (mode==OPTION_LF  &&  input.text[i+1]=='n') {
				count++;
				input.text[i] = '\n';
				shift_buffer_chunk(input.text, i+2, input.length+1, 1, OPTION_LEFT);
			}

			if (mode==OPTION_CRLF  &&  input.text[i+1]=='n') {
				count++;
				input.text[i]   = '\r';
				input.text[i+1] = '\n';
			}
		}

		if (count>=quantity  &&  quantity>=0)
			break;
	}
}







void QWrite_joystick(int customJoyID) {
	// If passed -2 then quit this function
	if (customJoyID == -2) {
		QWrite("[]");
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
		if (customJoyID>=0  ||  (device==JOYERR_NOERROR  &&  input==JOYERR_NOERROR)  ||  joyID==15) 
			break;

		joyID++;
	}
	while (input != 0);


	// Return error value
	if (device!=JOYERR_NOERROR  ||  input!=JOYERR_NOERROR) {
		//strcat(data, "[\"couldn't find\"]");
		QWrite("[\"");

		switch (input) {
			case MMSYSERR_NODRIVER    : QWrite("The joystick driver is not present."); break;
			case MMSYSERR_INVALPARAM  : QWrite("An invalid parameter was passed."); break;
			case MMSYSERR_BADDEVICEID : QWrite("The specified joystick identifier is invalid."); break;
			case JOYERR_UNPLUGGED     : QWrite("The specified joystick is not connected to the system."); break;
			case JOYERR_PARMS         : QWrite("The specified joystick identifier is invalid."); break;
		};

		QWritef("\",%d]", joyID);
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
	// Axes names array
	DWORD Axes[6]  = {joy.dwXpos, joy.dwYpos};
	int count_axis = 2;

	QWrite("[[\"X\",\"Y\"");

	if (joyCaps.wCaps & JOYCAPS_HASZ) {
		QWrite(",\"Z\"");
		Axes[count_axis++] = joy.dwZpos;
	}

	if (joyCaps.wCaps & JOYCAPS_HASR) {
		QWrite(",\"R\"");
		Axes[count_axis++] = joy.dwRpos;
	}

	if (joyCaps.wCaps & JOYCAPS_HASU) {
		QWrite(",\"U\"");
		Axes[count_axis++] = joy.dwUpos;
	}

	if (joyCaps.wCaps & JOYCAPS_HASV) {
		QWrite(",\"V\"");
		Axes[count_axis++] = joy.dwVpos;
	}


	// Axes values array
	bool add_comma = false;

	QWrite("],[");

	for (int i=0; i<count_axis; i++) {
		if (add_comma) 
			QWrite(","); 
		else 
			add_comma = true;

		QWritef("%u", Axes[i]);
	}


	// Buttons array
	int Buttons[32] = {0};

	maxButtons = joyCaps.wNumButtons;
	QWritef("],%d,[", maxButtons);

	add_comma = false;
	int buts  = joy.dwButtons;

	for (i=0; i<maxButtons; i++) {	// Loop each button
		Buttons[i] = buts & (1 << i);

		if (Buttons[i] > 0) {
			if (add_comma) 
				QWrite(","); 
			else 
				add_comma = true;

			QWritef("\"JOY%d\"", i+1);
		}
	}


	// POV array
	if (strcmp(POVtype,"NOPOV") != 0) {
		if (joy.dwPOV!=65535  &&  add_comma) 
			QWrite(",");

		switch (joy.dwPOV) {
			case 0     : QWrite("\"JOYPOVUP\""); break;
			case 4500  : QWrite("\"JOYPOVUPLEFT\""); break;
			case 9000  : QWrite("\"JOYPOVLEFT\""); break;
			case 13500 : QWrite("\"JOYPOVDOWNLEFT\""); break;
			case 18000 : QWrite("\"JOYPOVDOWN\""); break;
			case 22500 : QWrite("\"JOYPOVDOWNRIGHT\""); break;
			case 27000 : QWrite("\"JOYPOVRIGHT\""); break;
			case 31500 : QWrite("\"JOYPOVUPRIGHT\""); break;
		}
	}

	QWritef("],[\"%s\",%d],[%d,%d,%d]]", POVtype, joy.dwPOV, joyID, joyCaps.wMid, joyCaps.wPid);
}







// Write mission path information
void createPathSqf(LPCSTR lpFileName, size_t len, int offset) {
	DWORD ticks = GetTickCount();

	// Don't write multiple times at once
	if (ticks < global.mission_path_savetime+1000)
		return;

	global.mission_path_savetime = ticks;	
	
	FILE *f      = fopen(!global.is_server ? "fwatch\\data\\path.sqf" : "fwatch\\data\\pathDedi.sqf", "w");
	int pathType = -1;


	// Find mission type
	// Campaign
	if (!strncmp("campaigns", lpFileName, 9)) 
		pathType = 7;
	
	
	// Mission Editor
	if (!strncmp("Users\\", lpFileName, 6)) {
		char *nextBackSlash = strchr(lpFileName+6, '\\');

		if (nextBackSlash) {
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

	fprintf(f, "[%d,%d,[\"", pathType, len2);

	// output mission path
	global.mission_path_length = 0;

	for (int i=0; i<len2; i++)
		if (lpFileName[i]=='\\'  ||  lpFileName[i]=='/') {
			if (i != len2-1)
				fprintf(f, "\",\"");

			global.mission_path[global.mission_path_length++] = '\\';
		} else {
			fprintf(f, "%c", lpFileName[i]);
			global.mission_path[global.mission_path_length++] = lpFileName[i];
		}

	global.mission_path[global.mission_path_length] = '\0';
	fprintf(f, "\"]]");
	fclose(f);

	
	// Is this is a new mission?
	if (global.mission_path_length!=global.mission_path_previous_length  ||  strcmp(global.mission_path_previous, global.mission_path) !=0) {
		memcpy(global.mission_path_previous, global.mission_path, global.mission_path_length+1);
		global.mission_path_previous_length = global.mission_path_length;
	} else {
		int base = 0;

		switch(global_exe_version[global.exe_index]) {
			case VER_196        : base=0x7DD028; break;
			case VER_199        : base=0x7CBFE8; break;
			case VER_196_SERVER : base=0x75A2E0; break;
			case VER_199_SERVER : base=0x75A370; break;
		}

		// Is this is a mission restart?
		// Check mission time
		if (base) {
			HANDLE phandle = OpenProcess(PROCESS_ALL_ACCESS, 0, global.pid);

			if (phandle) {
				int time_value = 0;
				SIZE_T stBytes = 0;

				ReadProcessMemory(phandle, (LPVOID)base, &time_value, 4, &stBytes);
				CloseHandle(phandle);

				if (time_value > 500)
					return;
			}
		}
	}

	// Reset error logging
	global.ErrorLog_Enabled = false;
	global.ErrorLog_Started = false;
	NotifyFwatchAboutErrorLog();

	// Restore what was modified by Fwatch commands
	RestoreMemValues(pathType==3 || pathType==4);
}







// Interpret bool value from a text
bool String_bool(String &input) {
	String_trim_space(input);

	if (input.length==0  ||  strcmpi(input.text,"0")==0  ||  strcmpi(input.text,"false")==0)
		return false;

	if (strcmpi(input.text,"1")==0  ||  strcmpi(input.text,"true")==0)
		return true;

	return atoi(input.text) ? true : false;
}







// Convert input numbers meant to indicate position in a string
StringPos ConvertStringPos(char *range_start, char *range_end, char *range_length, size_t text_length) {
	StringPos range = {0, text_length};

	if (range_start[0]) {
		size_t i      = 0;
		bool negative = false;

		while (range_start[i] == '-') {
			i++;
			negative = !negative;
		}

		// Negative number means counting from the end of the string
		if (negative) {
			range.start = text_length - strtoul(range_start+i, NULL, 0);

			// If too much negative then it's beginning of the string
			if (range.start > text_length)
				range.start = 0;
		} else
			range.start = strtoul(range_start, NULL, 0);
	}

	if (range_end[0]) {
		size_t i      = 0;
		bool negative = false;

		while (range_end[i] == '-') {
			i++;
			negative = !negative;
		}

		if (negative) {
			range.end = text_length - strtoul(range_end+i, NULL, 0);

			if (range.end > text_length)
				range.end = 0;
		} else
			range.end = strtoul(range_end, NULL, 0);
	}

	if (range_length[0])
		range.end = range.start + strtoul(range_length, NULL, 0);

	if (range.start > text_length)
		range.start = text_length;

	if (range.end > text_length)
		range.end = text_length;

	return range;
}







// Return character number type
int GetCharType(char c) {
	if (c=='\r'  ||  c=='\n')
		return CHAR_TYPE_NEW_LINE;
	else
		if (c<0x20  ||  c==' ')
			return CHAR_TYPE_SPACE;
		else
			if (c>=0x80  ||  isalnum(c)  ||  c=='_')
				return CHAR_TYPE_LETTER;
			else
				return CHAR_TYPE_MISC;
}







// Restore memory values that were changed in the game
void RestoreMemValues(bool isMissionEditor) {
	int max_loops  = (sizeof(global.restore_memory) / sizeof(global.restore_memory[0]));
	HANDLE phandle = NULL;
	SIZE_T stBytes = 0;
	phandle        = OpenProcess(PROCESS_ALL_ACCESS, 0, global.pid);

	if (!phandle)
		return;

	for (int i=0; i<max_loops; i++) {
		if (!global.restore_memory[i])
			continue;

		if (i == RESTORE_BRIGHTNESS) {	
			int base    = 0;
			int pointer = 0;

			switch(global_exe_version[global.exe_index]) {
				case VER_196 : base=0x789D88; break;
				case VER_199 : base=0x778E80; break;
				case VER_201 : base=global.exe_address+0x6D6A10; break;
			}

			if (base) {
				ReadProcessMemory(phandle, (LPVOID)base, &pointer, 4, &stBytes);
				WriteProcessMemory(phandle,(LPVOID)(pointer+0x2C),  &global.restore_float[FLOAT_BRIGHTNESS], 4, &stBytes);
			}
		}

		if (i>=RESTORE_OBJECT_SHADOWS  &&  i<=RESTORE_CLOUDLETS) {
			int base    = 0;
			int	pointer = 0;
			int offset  = 0;

			switch(global_exe_version[global.exe_index]) {
				case VER_196 : base=0x79F8D0; break;
				case VER_199 : base=0x78E9C8; break;
				case VER_201 : base=global.exe_address+0x6D8240; break;
			}

			if (i == RESTORE_VEHICLE_SHADOWS) 
				offset = 1;

			if (i == RESTORE_CLOUDLETS) 
				offset = 2;

			if (base) {
				ReadProcessMemory(phandle, (LPVOID)base, &pointer, 4, &stBytes);
				WriteProcessMemory(phandle, (LPVOID)(pointer+0x5B0+offset), &global.restore_byte[BYTE_OBJECT_SHADOWS+offset], 1, &stBytes);
			}
		}

		if (i>=RESTORE_BULLETS  &&  i<RESTORE_CADET) {
			int offset[10] = {0};

			switch(global_exe_version[global.exe_index]) {
				case VER_196 : 
					offset[0] = 0x71D518;
					offset[1] = 0x5F1570;
					offset[2] = 0x5F16D2;
					offset[3] = 0x5F1527;
					offset[4] = 0x5F147B;
					offset[5] = 0x5F178F;
					offset[6] = 0x5F12AD;
					offset[7] = 0x7137A0;
					offset[8] = 0x5F14BF;
					offset[9] = 0x5F1818;
					break;

				case VER_199 : 
					offset[0] = 0x710520;
					offset[1] = 0x5F818C;
					offset[2] = 0x5F7ACB;
					offset[3] = 0x4857B3;
					offset[4] = 0x487867;
					offset[5] = 0x487986;
					offset[6] = 0x5F7D5D;
					offset[7] = 0x7067A0;
					offset[8] = 0x485820;
					offset[9] = 0x5F789B;
					break;

				case VER_201 : 
					offset[0] = global.exe_address+0x57249C;
					offset[1] = global.exe_address+0x35473B;
					offset[3] = global.exe_address+0x34EBA6;
					offset[4] = global.exe_address+0x351A2C;
					offset[5] = global.exe_address+0x354206;
					offset[6] = global.exe_address+0x353A0E;
					offset[7] = global.exe_address+0x5724D0;
					offset[8] = global.exe_address+0x34FB2A;
					offset[9] = global.exe_address+0x3550D3;
					break;

				case VER_196_SERVER : 
					offset[0] = 0x6ABDE8;
					offset[1] = 0x5374B8;
					offset[2] = 0x534F9C;
					offset[3] = 0x533A91;
					offset[4] = 0x5357C7;
					offset[5] = 0x5371A1;
					offset[6] = 0x536D09;
					offset[7] = 0x6A66C0;
					offset[8] = 0x5345F9;
					offset[9] = 0x5347F7;
					break;

				case VER_199_SERVER : 
					offset[0] = 0x6ABDA8;
					offset[1] = 0x537671;
					offset[2] = 0x53517D;
					offset[3] = 0x533C4A;
					offset[4] = 0x5359A8;
					offset[5] = 0x53735A;
					offset[6] = 0x536EEA;
					offset[7] = 0x6A66C0;
					offset[8] = 0x5347DA;
					offset[9] = 0x5349D8;
					break;

				case VER_201_SERVER : 
					offset[0] = global.exe_address+0x492A88;
					offset[1] = global.exe_address+0x2E15FB;
					offset[3] = global.exe_address+0x2DBC88;
					offset[4] = global.exe_address+0x2DE81C;
					offset[5] = global.exe_address+0x2E0F96;
					offset[6] = global.exe_address+0x2E080E;
					offset[7] = global.exe_address+0x492AC0;
					offset[8] = global.exe_address+0x2DCA5A;
					offset[9] = global.exe_address+0x2E1F63;
					break;
			}

			if (offset[i-4])
				WriteProcessMemory(phandle, (LPVOID)offset[i-4], &global.restore_float[i-3], 4, &stBytes);	
		}

		if (i>=RESTORE_CADET  &&  i<RESTORE_RADAR) {
			int	offsets[][4] = {
				{0x7DD0C8, 0x7DD0D4, 0x75A380, 0x75A38C},	//ofp
				{0x7CC088, 0x7CC094, 0x75A410, 0x75A41C},	//cwa
				{global.exe_address + 0x714B50, global.exe_address + 0x714B5D, global.exe_address + 0x60AFA0, global.exe_address + 0x60AFAD}	//2.01
			};

			int j    = 0;	// which game
			if (global_exe_version[global.exe_index] == VER_199) j=1;
			if (global_exe_version[global.exe_index] == VER_201) j=2;
			int k    = 0;				// which difficulty
			int base = RESTORE_CADET;

			if (i >= RESTORE_VETERAN) {
				k++;
				base = RESTORE_VETERAN;
			}

			if (global.is_server) 
				k += 2;

			int offset = offsets[j][k];

			WriteProcessMemory(phandle, (LPVOID)(offset+i-base), &global.restore_byte[i-11],  1, &stBytes);
		}

		if (i == RESTORE_RADAR) {
			int base = 0;
			switch(global_exe_version[global.exe_index]) {
				case VER_196 : base=0x7DD074; break;
				case VER_199 : base=0x7CC034; break;
			}
			if (base)
				WriteProcessMemory(phandle,(LPVOID)base, &global.restore_float[FLOAT_RADAR], 4, &stBytes);
		}

		if (i == RESTORE_MAX_OBJECTS) {
			int base = 0;
			switch(global_exe_version[global.exe_index]) {
				case VER_196 : base=0x7DD07C; break;
				case VER_199 : base=0x7CC03C; break;
				case VER_201 : base=global.exe_address+0x714AFC; break;
				default      : base=0;
			}
			if (base)
				WriteProcessMemory(phandle,(LPVOID)base, &global.restore_int[INT_MAX_OBJECTS], 4, &stBytes);
		}

		if (i == RESTORE_TRACK1) {
			int base = 0;
			switch(global_exe_version[global.exe_index]) {
				case VER_196 : base=0x7DD080; break;
				case VER_199 : base=0x7CC040; break;
				case VER_201 : base=global.exe_address+0x714B08; break;
			}
			if (base)
				WriteProcessMemory(phandle,(LPVOID)base, &global.restore_float[FLOAT_TRACK1], 4, &stBytes);
		}

		if (i == RESTORE_TRACK2) {
			int base = 0;
			switch(global_exe_version[global.exe_index]) {
				case VER_196 : base=0x7DD084; break;
				case VER_199 : base=0x7CC044; break;
				case VER_201 : base=global.exe_address+0x714B0C; break;
				default      : base=0;
			}
			if (base)
				WriteProcessMemory(phandle,(LPVOID)base, &global.restore_float[FLOAT_TRACK2], 4, &stBytes);
		}

		if (i == RESTORE_MAX_LIGHTS) {
			int base = 0;
			switch(global_exe_version[global.exe_index]) {
				case VER_196 : base=0x7DD08C; break;
				case VER_199 : base=0x7CC04C; break;
				case VER_201 : base=global.exe_address+0x714B14; break;
			}
			if (base)
				WriteProcessMemory(phandle,(LPVOID)base, &global.restore_int[INT_MAX_LIGHTS], 4, &stBytes);
		}

		if (i == RESTORE_TIDE) {
			int base = 0;
			switch(global_exe_version[global.exe_index]) {
				case VER_196        : base=0x72F8E4; break;
				case VER_199        : base=0x72295C; break;
				case VER_201        : base=0x10718CA0; break;
				case VER_196_SERVER : base=0x6BE184; break;
				case VER_199_SERVER : base=0x6BE144; break;
			}
			if (base)
				WriteProcessMemory(phandle,(LPVOID)base, &global.restore_float[FLOAT_TIDE], 4, &stBytes);
		}

		if (i == RESTORE_WAVE) {
			int base = 0;
			switch(global_exe_version[global.exe_index]) {
				case VER_196        : base=0x72F8E4; break;
				case VER_199        : base=0x72295C; break;
				case VER_201        : base=0x10718CA0; break;
				case VER_196_SERVER : base=0x6BE184; break;
				case VER_199_SERVER : base=0x6BE144; break;
			}
			if (base)
				WriteProcessMemory(phandle,(LPVOID)(base+4), &global.restore_float[FLOAT_WAVE], 4, &stBytes);
		}

		if (i == RESTORE_EXTCAMPOS) {
			int pointer[3] = {0};
			int	modif[]    = {0x5C, 0x69C};
			int	max_loops  = (sizeof(pointer) / sizeof(pointer[0])) - 1;

			switch(global_exe_version[global.exe_index]) {
				case VER_196 : pointer[0]=0x7894A0; break;
				case VER_199 : pointer[0]=0x778590; modif[1]=0x6A0; break;
			}

			if (pointer[0]) {
				for (int i=0; i<max_loops; i++) {
					ReadProcessMemory(phandle, (LPVOID)pointer[i], &pointer[i+1], 4, &stBytes);
					pointer[i+1] = pointer[i+1] + modif[i];
				}
				
				WriteProcessMemory(phandle, (LPVOID)(global.extCamOffset+0), &global.restore_float[FLOAT_EXTCAMX], 4, &stBytes);
				WriteProcessMemory(phandle, (LPVOID)(global.extCamOffset+4), &global.restore_float[FLOAT_EXTCAMZ], 4, &stBytes);
				WriteProcessMemory(phandle, (LPVOID)(global.extCamOffset+8), &global.restore_float[FLOAT_EXTCAMY], 4, &stBytes);
			}
		}

		if (i == RESTORE_WAVE_SPEED) {
			int base	= 0;
			int pointer = 0;

			switch(global_exe_version[global.exe_index]) {
				case VER_196        : base=0x7B3ACC; break;
				case VER_199        : base=0x7A2C0C; break;
				case VER_201        : base=global.exe_address+0x6D6B34; break;
				case VER_196_SERVER : base=0x73392C; break;
				case VER_199_SERVER : base=0x7339C4; break;
				case VER_201_SERVER : base=global.exe_address+0x5CCFB8; break;
			}

			if (base) {
				ReadProcessMemory (phandle, (LPVOID)base, &pointer, 4, &stBytes);
				WriteProcessMemory(phandle, (LPVOID)(pointer+0x2059C), &global.restore_float[FLOAT_WAVE_SPEED], 4, &stBytes);
			}
		}

		if (!isMissionEditor) {
			if (i == RESTORE_FOVLEFT) {
				int base    = 0;
				int pointer = 0;

				switch(global_exe_version[global.exe_index]) {
					case VER_196 : base=0x789D88; break;
					case VER_199 : base=0x778E80; break;
					case VER_201 : base=global.exe_address+0x6D6A10; break;
				}

				if (base) {
					ReadProcessMemory(phandle, (LPVOID)base, &pointer, 4, &stBytes);
					pointer += 0x40;

					WriteProcessMemory(phandle,(LPVOID)pointer, &global.restore_float[FLOAT_FOVLEFT], 4, &stBytes);
				}
			}
		
			if (i == RESTORE_FOVTOP) {
				int base    = 0;
				int pointer = 0;

				switch(global_exe_version[global.exe_index]) {
					case VER_196 : base=0x789D88; break;
					case VER_199 : base=0x778E80; break;
					case VER_201 : base=global.exe_address+0x6D6A10; break;
				}

				if (base) {
					ReadProcessMemory(phandle, (LPVOID)base, &pointer, 4, &stBytes);
					pointer += 0x40 + 4;

					WriteProcessMemory(phandle,(LPVOID)pointer, &global.restore_float[FLOAT_FOVTOP], 4, &stBytes);
				}
			}
		
			if (i == RESTORE_UITOPLEFTX) {
				int base    = 0;
				int pointer = 0;

				switch(global_exe_version[global.exe_index]) {
					case VER_196 : base=0x789D88; break;
					case VER_199 : base=0x778E80; break;
					case VER_201 : base=global.exe_address+0x6D6A10; break;
				}

				if (base) {
					ReadProcessMemory(phandle, (LPVOID)base, &pointer, 4, &stBytes);
					pointer += 0x40 + 8;

					WriteProcessMemory(phandle,(LPVOID)pointer, &global.restore_float[FLOAT_UITOPLEFTX], 4, &stBytes);
				}
			}
		
			if (i == RESTORE_UITOPLEFTY) {
				int base    = 0;
				int pointer = 0;

				switch(global_exe_version[global.exe_index]) {
					case VER_196 : base=0x789D88; break;
					case VER_199 : base=0x778E80; break;
					case VER_201 : base=global.exe_address+0x6D6A10; break;
				}

				if (base) {
					ReadProcessMemory(phandle, (LPVOID)base, &pointer, 4, &stBytes);
					pointer += 0x40 + 12;

					WriteProcessMemory(phandle,(LPVOID)pointer, &global.restore_float[FLOAT_UITOPLEFTY], 4, &stBytes);
				}
			}
		
			if (i == RESTORE_UIBOTTOMRIGHTX) {
				int base    = 0;
				int pointer = 0;

				switch(global_exe_version[global.exe_index]) {
					case VER_196 : base=0x789D88; break;
					case VER_199 : base=0x778E80; break;
					case VER_201 : base=global.exe_address+0x6D6A10; break;
				}

				if (base) {
					ReadProcessMemory(phandle, (LPVOID)base, &pointer, 4, &stBytes);
					pointer += 0x40 + 16;

					WriteProcessMemory(phandle,(LPVOID)pointer, &global.restore_float[FLOAT_UIBOTTOMRIGHTX], 4, &stBytes);
				}
			}
		
			if (i == RESTORE_UIBOTTOMRIGHTY) {
				int base    = 0;
				int pointer = 0;

				switch(global_exe_version[global.exe_index]) {
					case VER_196 : base=0x789D88; break;
					case VER_199 : base=0x778E80; break;
					case VER_201 : base=global.exe_address+0x6D6A10; break;
				}

				if (base) {
					ReadProcessMemory(phandle, (LPVOID)base, &pointer, 4, &stBytes);
					pointer += 0x40 + 20;

					WriteProcessMemory(phandle,(LPVOID)pointer, &global.restore_float[FLOAT_UIBOTTOMRIGHTY], 4, &stBytes);
				}
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

		if (!isMissionEditor  ||  (isMissionEditor && (i<RESTORE_FOVLEFT || i>RESTORE_UIBOTTOMRIGHTY)))
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
//--------------------------------------------------------------------------






// Verify and convert user input path from relative to absolute
int VerifyPath(String &path, StringDynamic &buffer, int options) {
	size_t item_start = 0;
	size_t item_index = 0;
	int level         = 0;
	bool dir_root     = false;
	bool dir_fwatch   = false;
	bool dir_illegal  = false;
	bool dir_download = false;
	bool dir_allowed  = false;
	char *item        = path.text;
	
	const char allowed_dirs[][32] = {
		"in-game-script-editor",
		"flashpointcutscenemaker",
		"missioneditor3d",
		"@addontest",
		"set-pos-in-game"
	};


	for (size_t i=0; i<=path.length; i++) {
		if (path.text[i]=='\\' || path.text[i]=='/' || path.text[i]=='\0') {
			char backup  = path.text[i];
			path.text[i] = '\0';
			item         = path.text + item_start;

			if (strcmp(item,"..") == 0) {
				if (item_index == 0)
					dir_root = true;
				else {
					level--;

					if (level < 0) {
						path.text[i] = backup;
						break;
					}
					
					if (dir_root) {
						if (level == 0) {
							dir_fwatch  = false;
							dir_allowed = false;
						}
							
						if (level < 2)
							dir_download = false;
					}
				}
			} else
				if (backup != '\0')
					level++;
				
			if (dir_root && level==1) {
				if (strcmpi(item,"fwatch") == 0)
					dir_fwatch = true;
				else
					for (int j=0; j<sizeof(allowed_dirs)/sizeof(allowed_dirs[0]); j++)
						if (strcmpi(item,allowed_dirs[j]) == 0)
							dir_allowed = true;
			}

			if (dir_fwatch && level==2) {
				if (strcmpi(item,"data")==0 || strcmpi(item,"mdb")==0) {
					dir_illegal  = true;
					path.text[i] = backup;
					break;
				}

				if (strcmpi(item,"tmp") == 0)
					dir_download = true;
			}
			
			path.text[i] = backup;
			
			if (path.text[i] == '\0') {
				if (dir_fwatch && level==1) {
					dir_illegal = true;

					if (options & OPTION_ASSUME_TRAILING_SLASH) {
						if (strcmpi(item,"idb") == 0)
							dir_illegal = false;
						else
							if (strcmpi(item,"tmp") == 0) {
								dir_illegal  = false;
								dir_download = true;
							}
					}
				}

				break;
			}
				
			item_start = i + 1;
			item_index++;
		}
	}


	if (dir_illegal) {
		if (~options & OPTION_SUPPRESS_ERROR)
			QWrite_err(FWERROR_PARAM_PATH_RESTRICTED, 1, path);
	} else
		if (dir_root  &&  !dir_fwatch  &&  !dir_allowed  &&  options & OPTION_RESTRICT_TO_MISSION_DIR) {
			if (~options & OPTION_SUPPRESS_ERROR)
				QWrite_err(FWERROR_PARAM_PATH_LEAVING, 1, path);
		} else
			if (level >= 0) {
				if (~options & OPTION_SUPPRESS_CONVERSION) {
					if (dir_root) {
						path.text   += 3;
						path.length -= 3;
					} else {
						StringDynamic_appendf(buffer, "%s%s", global.mission_path, path.text);
						path.text   = buffer.text;
						path.length = buffer.length;
					}
				}

				return dir_download ? PATH_DOWNLOAD_DIR : PATH_LEGAL;
			}

	return PATH_ILLEGAL;
}







//https://stackoverflow.com/questions/11413860/best-string-hashing-function-for-short-filenames
unsigned int fnv1a_hash(unsigned int hash, char *text, size_t text_length, bool lowercase) {
    for (size_t i=0; i<text_length; i++)
		hash = (hash ^ (lowercase ? tolower(text[i]) : text[i])) * FNV_PRIME;

    return hash;
}







void PurgeComments(char *text, int string_start, int string_end) {
	enum COMMENT_TYPE {
		COMMENT_NONE,
		COMMENT_LINE,
		COMMENT_BLOCK
	};
	
	int comment_start = -1;
	int comment_type  = COMMENT_NONE;
	int new_end       = string_end;
	
	for (int j=string_start; text[j]!='\0'; j++) {
		if (comment_start == -1) {
			if (text[j]=='/' && text[j+1]=='*') {
				comment_start = j;
				comment_type  = COMMENT_BLOCK;
			} else
				if (text[j]=='/' && text[j+1]=='/') {
					comment_start = j;
					comment_type  = COMMENT_LINE;
				}
		} else {
			if ((comment_type==2 && text[j]=='*' && text[j+1]=='/') || (comment_type==COMMENT_LINE && (text[j]=='\r' || text[j]=='\n'))) {
				if (comment_type == COMMENT_BLOCK)
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







	// Delete file or directory with its contents  http://stackoverflow.com/a/10836193
int DeleteWrapper(char *refcstrRootDirectory) {
	if (~GetFileAttributes(refcstrRootDirectory) & FILE_ATTRIBUTE_DIRECTORY) {
		if (DeleteFile(refcstrRootDirectory))
			return 0;
		else
			return GetLastError();
	}
	
	int             return_value = 0;
	bool            bDeleteSubdirectories = true;
	bool            bSubdirectory = false;       // Flag, indicating whether subdirectories have been found
	HANDLE          hFile;                       // Handle to directory
	StringDynamic   strFilePath;                 // Filepath
	StringDynamic   strPattern;                  // Pattern
	WIN32_FIND_DATA FileInformation;             // File information

	StringDynamic_init(strFilePath);
	StringDynamic_init(strPattern);
	StringDynamic_appendf(strFilePath, "%s\\", refcstrRootDirectory);
	StringDynamic_appendf(strPattern, "%s\\*.*", refcstrRootDirectory);
	
	int saved_length = strFilePath.length;
	hFile            = FindFirstFile(strPattern.text, &FileInformation);
	
	if (hFile != INVALID_HANDLE_VALUE) {
		do {
			if (FileInformation.cFileName[0] != '.') {
				strFilePath.length = saved_length;
				StringDynamic_append(strFilePath, FileInformation.cFileName);

				if (FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
					if (bDeleteSubdirectories) {
						// Delete subdirectory
						int result = DeleteWrapper(strFilePath.text);
						if (result) {
							return_value = result;
							goto DeleteDirectory_return;
						}
					} else
						bSubdirectory = true;
				} else {
					// Set file attributes
					if (SetFileAttributes(strFilePath.text, FILE_ATTRIBUTE_NORMAL) == FALSE) {
						return_value = GetLastError();
						goto DeleteDirectory_return;
					}

					// Delete file
					if (DeleteFile(strFilePath.text) == FALSE) {
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
	StringDynamic_end(strFilePath);
	StringDynamic_end(strPattern);
	return return_value;
}







// Send message to fwatch.exe to enable/disable error logging there
void NotifyFwatchAboutErrorLog() {
	HANDLE mailslot = CreateFile(TEXT("\\\\.\\mailslot\\fwatch_mailslot"), GENERIC_WRITE, FILE_SHARE_READ, (LPSECURITY_ATTRIBUTES)NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);
	
	if (mailslot != INVALID_HANDLE_VALUE) {
		char temp[32] = "";
		sprintf(temp, "%d|%d|%d", C_INFO_ERRORLOG, global.ErrorLog_Enabled, global.ErrorLog_Started);

		DWORD bytes_written = 0;
		WriteFile(mailslot, temp, strlen(temp)+1, &bytes_written, (LPOVERLAPPED)NULL);
		CloseHandle(mailslot);
	}
}







void WriterHeaderInErrorLog(void *ptr_logfile, void *ptr_phandle, bool notify) {
	FILE **logfile  = (FILE **)ptr_logfile;
	HANDLE *phandle = (HANDLE *)ptr_phandle;
	
	global.ErrorLog_Started = true;
	if (notify)
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
		int base		 = 0;
		int pointer		 = 0;
		DWORD stBytes    = 0;

		switch(global_exe_version[global.exe_index]) {
			case VER_196        : base=0x7DD0E0; break;
			case VER_199        : base=0x7CC0A0; break;
			case VER_196_SERVER : base=0x75A398; break;
			case VER_199_SERVER : base=0x75A428; break;
		}

		if (base)
			ReadProcessMemory(*phandle, (LPVOID)base, &buffer, 80, &stBytes);


		// if packed pbo
		if (strcmp(buffer,"__cur_sp")==0  ||  strcmp(buffer,"__cur_mp")==0) {
			switch(global_exe_version[global.exe_index]) {
				case VER_196        : base=0x7DD180; break;
				case VER_199        : base=0x7CC140; break;
				case VER_196_SERVER : base=0x75A438; break;
				case VER_199_SERVER : base=0x75A4C8; break;
				default             : base=0;
			}

			if (base) {
				ReadProcessMemory(*phandle, (LPVOID)base, &pointer, 4, &stBytes);

				if (pointer != 0)
					ReadProcessMemory(*phandle, (LPVOID)(pointer+0x8), &buffer, 255, &stBytes);
			}
		}
	
		fprintf(*logfile, "%s", buffer);
		buffer[0] = '\0';


		// island name
		switch(global_exe_version[global.exe_index]) {
			case VER_196        : base=0x7DD130; break;
			case VER_199        : base=0x7CC0F0; break;
			case VER_196_SERVER : base=0x75A3E8; break;
			case VER_199_SERVER : base=0x75A478; break;
			default             : base=0;
		}

		if (base)
			ReadProcessMemory(*phandle, (LPVOID)base, &buffer, 80, &stBytes);

		fprintf(*logfile, ".%s\t", buffer);
		buffer[0] = '\0';


		// briefing title
		switch(global_exe_version[global.exe_index]) {
			case VER_196        : base=0x78324C; break;
			case VER_199        : base=0x77233C; break;
			case VER_196_SERVER : base=0x7030AC; break;
			case VER_199_SERVER : base=0x7030FC; break;
			default             : base=0;
		}

		if (base) {
			ReadProcessMemory(*phandle, (LPVOID)base, &pointer, 4,	 &stBytes);

			if (pointer) 
				ReadProcessMemory(*phandle, (LPVOID)(pointer+0x114), &buffer, 255, &stBytes);
			else {
				switch(global_exe_version[global.exe_index]) {
					case VER_196        : base=0x786880; break;
					case VER_199        : base=0x775968; break;
					case VER_196_SERVER : base=0x7066D8; break;
					case VER_199_SERVER : base=0x706728; break;
					default             : base=0;
				}

				if (base) {
					ReadProcessMemory(*phandle, (LPVOID)base, &pointer, 4,	 &stBytes);

					if (pointer)
						ReadProcessMemory(*phandle, (LPVOID)(pointer+0x8), &buffer, 255, &stBytes);
				}
			}
		}

		fprintf(*logfile, "%s\n", buffer);
	}
}







// Output formatted text to the game
void QWritef(const char *format, ...) {
	if (!global.outf) {
		int out_id  = _open_osfhandle((long)global.out, _O_BINARY|_O_APPEND);
		global.outf = _fdopen(out_id, "a");
		setvbuf(global.outf, NULL, _IONBF, 0);
	}

	va_list args;
	va_start(args, format);
	vfprintf(global.outf, format, args);
	va_end(args);
}






// Output text to the game with specified length
void QWritel(const char *input, size_t input_length) {
	if (input_length > 0) {
		DWORD bytes_written;
		WriteFile(global.out, input, input_length, &bytes_written, NULL);

		if (bytes_written < input_length)
			DebugMessage("WARNING! Wanted to write %d bytes but only %d written", input_length, bytes_written);
	}
}






// Search for an int in an int array
int binary_search(unsigned int item_to_find, unsigned int *array, int low, int high) {
	if (high >= low) {
		int mid                 = low + (high - low) / 2;
		unsigned int *mid_value = array + mid;
		
		if (*mid_value == item_to_find)
			return mid;
		else
			return binary_search(
				item_to_find,
				array,
				*mid_value > item_to_find ? low   : mid+1,
				*mid_value > item_to_find ? mid-1 : high
			);
	}

	return -1;
}






// Search for an unsigned int in a string buffer
BinarySearchResult binary_search_str(char *buffer, size_t array_size, unsigned int value_to_find, size_t low, size_t high) {
	if (array_size  &&  high>=low) {
		size_t mid        = low + (high - low) / 2;
		size_t *mid_value = (size_t*)(buffer + mid*sizeof(size_t));

		if (*mid_value == value_to_find) {
			BinarySearchResult out = {mid,1};
			return out;
		} else
			if (*mid_value > value_to_find) {
				if (mid > 0)
					return binary_search_str(buffer, array_size, value_to_find, low, mid-1);
				else {
					// target key should be at the start of the array
					BinarySearchResult out = {low, 0};
					return out;
				}
			} else
				if (mid < array_size-1)
					return binary_search_str(buffer, array_size, value_to_find, mid+1, high);
				else {
					// target key should be at the end of the array
					BinarySearchResult out = {mid+1, 0};
					return out;
				}
	}
	
	BinarySearchResult out = {low,0};
	return out;
}







void QWrite_err(int code_primary, int arg_num, ...) {
	if (!global.out)
		return;

	char format[128]      = "";
	DWORD code_secondary  = ERROR_SUCCESS;
	LPVOID winapi_msg_ptr = NULL;

	va_list arg_list;
	va_start(arg_list, arg_num);

	switch (code_primary) {
		case FWERROR_NONE : 
		case FWERROR_UNKNOWN_COMMAND : 
		case FWERROR_GAME_WINDOW_NOT_IN_FRONT : 
		case FWERROR_COMMAND_ILLEGAL_ON_SERVER : break;

		case FWERROR_ERRNO : {
			if (arg_num > 0)
				code_secondary = va_arg(arg_list, DWORD);
		} break;

		case FWERROR_WINAPI : 
		case FWERROR_SHFILEOP : 
		case FWERROR_CLIP_OPEN :
		case FWERROR_CLIP_CLEAR :
		case FWERROR_CLIP_COPY :
		case FWERROR_CLIP_LOCK : {
			if (arg_num > 0)
				code_secondary = va_arg(arg_list, DWORD);
			
			FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			code_secondary, 
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR) &winapi_msg_ptr,
			0,
			NULL);
		} break;
		
		case FWERROR_NO_PROCESS  : strcpy(format,"Couldn't find %s process"); break;
		case FWERROR_HRESULT     : strcpy(format,"%s failed. Error code: 0x%x"); break;

		case FWERROR_MALLOC      : strcpy(format,"Failed to allocate memory block %s %u bytes"); break;
		case FWERROR_REALLOC     : strcpy(format,"Failed to reallocate memory block %s %u bytes"); break;
		case FWERROR_STR_REPLACE : strcpy(format,"Failed to perform str_replace %s %u bytes"); break;

		case FWERROR_CLIP_FORMAT : strcpy(format,"Clipboard data incompatible format"); break;
		case FWERROR_CLIP_EMPTY  : strcpy(format,"Clipboard is empty"); break;
		case FWERROR_CLIP_EFFECT : strcpy(format,"Invalid preferred effect value"); break;

		case FWERROR_PARAM_FEW             : strcpy(format,"Not enough parameters %u/%u"); break;
		case FWERROR_PARAM_LTZERO          : strcpy(format,"Parameter is less than zero %s=%d"); break;
		case FWERROR_PARAM_ZERO            : strcpy(format,"Parameter is zero or less %s=%d"); break;
		case FWERROR_PARAM_ONE             : strcpy(format,"Parameter is one or less %s=%d"); break;
		case FWERROR_PARAM_RANGE           : strcpy(format,"Range start %s is larger than range end %s %d>%d"); break;
		case FWERROR_PARAM_PATH_LEAVING    : strcpy(format,"Path leads outside source directory %s"); break;
		case FWERROR_PARAM_ACTION          : strcpy(format,"Action was not specified"); break;
		case FWERROR_PARAM_EMPTY           : strcpy(format,"Parameter is empty: %s"); break;
		case FWERROR_PARAM_PATH_RESTRICTED : strcpy(format,"Restricted location %s"); break;

		case FWERROR_FILE_EMPTY       : strcpy(format,"File is empty"); break;
		case FWERROR_FILE_NOTDIR      : strcpy(format,"File is not a directory"); break;
		case FWERROR_FILE_MOVETOTMP   : strcpy(format,"Moving file from outside to the tmp directory %s %s"); break;
		case FWERROR_FILE_NOVAR       : strcpy(format,"Couldn't find variable %s in file %s"); break;
		case FWERROR_FILE_NOLINE      : strcpy(format,"Couldn't find line %d in file %s"); break;
		case FWERROR_FILE_APPEND      : strcpy(format,"Couldn't append to %s too long line (%u chars) in file %s"); break;
		case FWERROR_FILE_LINEREPLACE : strcpy(format,"Couldn't replace line %d with %d (end of file)"); break;
		case FWERROR_FILE_EXISTS      : strcpy(format,"File %s already exists"); break;
		case FWERROR_FILE_DIREXISTS   : strcpy(format,"Directory %s already exists"); break;
		case FWERROR_FILE_READ        : strcpy(format,"Read only %u/%u bytes from file %s"); break;
		case FWERROR_FILE_WRITE       : strcpy(format,"Written only %u/%u bytes to file %s"); break;

		case FWERROR_CLASS_PARENT   : strcpy(format,"Couldn't find parent class %s %d/%d in file %s"); break; 
		case FWERROR_CLASS_EXISTS   : strcpy(format,"Class %s already exists in file %s"); break;
		case FWERROR_CLASS_NOCLASS  : strcpy(format,"Couldn't find class %s in file %s"); break;
		case FWERROR_CLASS_NOVAR    : strcpy(format,"Couldn't find property %s in file %s"); break;
		case FWERROR_CLASS_NOITEM   : strcpy(format,"Couldn't find item %d in array %s in file %s"); break;
		case FWERROR_CLASS_NOTARRAY : strcpy(format,"Property %s is not an array in file %s"); break;
		case FWERROR_CLASS_SYNTAX   : strcpy(format,"Syntax error: %s at line %d column %d in file %s"); break;

		case FWERROR_DB_SIGNATURE   : strcpy(format,"Incorrect database signature 0x%I64x (should be 0x%I64x) in file %s"); break;
		case FWERROR_DB_VERSION     : strcpy(format,"Database version not supported %d (should be <=%d) in file %s"); break;
		case FWERROR_DB_SMALL       : strcpy(format,"Database is too small %u bytes (should be at least %u bytes) in file %s"); break;
		case FWERROR_DB_COLLISION   : strcpy(format,"Collision between key hashes %s (%u) and %s (%u) in file %s"); break;
		case FWERROR_DB_HASHORDER   : strcpy(format,"Incorrect hash order for keys %u and %u in file %s"); break;
		case FWERROR_DB_PTRORDER    : strcpy(format,"Database pointer #%u (%u) is smaller than or equal to pointer #%u (%u) in file %s"); break;
		case FWERROR_DB_PTRSMALL    : strcpy(format,"Database pointer %u/%u value %u is too small (minimal is %u) in file %s"); break;
		case FWERROR_DB_PTRBIG      : strcpy(format,"Database pointer %u/%u value %u is too big (maximal is %u) in file %s"); break;
		case FWERROR_DB_PTRFIRST    : strcpy(format,"Database first pointer value %u is incorrect (should be %u) in file %s"); break;
		case FWERROR_DB_CONSISTENCY : strcpy(format,"Database consistency error (item %u/%u at %u) in file %s"); break;
		case FWERROR_DB_KEYEXISTS   : strcpy(format,"Key %s already exists in file %s"); break;
		
		default : sprintf(format, "Unknown error %d", code_primary);
	}

	//TODO: Log errors to file
	if (~global.option_error_output & OPTION_ERROR_ARRAY_SUPPRESS) {
		// If this function is being called more than once then separate arrays with a semi-colon
		if (~global.option_error_output & OPTION_ERROR_ARRAY_NESTED) {
			if (global.option_error_output & OPTION_ERROR_ARRAY_STARTED)
				QWrite(";");
			else
				global.option_error_output |= OPTION_ERROR_ARRAY_STARTED;
		}

		// Start output
		if (global.option_error_output & OPTION_ERROR_ARRAY_LOCAL)
			QWrite("_fwatch_error=");

		QWritef("[%s,%d,%d,\"", getBool(code_primary==FWERROR_NONE), code_primary, code_secondary);

		if (winapi_msg_ptr) {
			switch (code_primary) {
				case FWERROR_CLIP_OPEN  : QWrite("Couldn't open clipboard -"); break;
				case FWERROR_CLIP_CLEAR : QWrite("Couldn't clear clipboard"); break;
				case FWERROR_CLIP_COPY  : QWrite("Couldn't copy text to clipboard"); break;
				case FWERROR_CLIP_LOCK  : QWrite("GlobalLock failed"); break;
			}

			String winapi_msg = {(char*)winapi_msg_ptr, strlen((char*)winapi_msg_ptr)};
			String_trim_space(winapi_msg);
			QWritesq(winapi_msg);
			LocalFree(winapi_msg_ptr);

			for (int i=1; i<arg_num; i++) {
				QWrite(" - ");
				QWriteq(va_arg(arg_list, char*));
			}
		} else
			if (code_primary == FWERROR_ERRNO) {
				QWriteq(strerror(code_secondary));

				for (int i=1; i<arg_num; i++) {
					QWrite(" - ");
					QWriteq(va_arg(arg_list, char*));
				}
			} else
				vfprintf(global.outf, format, arg_list);

		va_end(arg_list);

		if (global.option_error_output & OPTION_ERROR_ARRAY_CLOSE)
			QWrite("\"]");
		else
			if (global.option_error_output & OPTION_ERROR_ARRAY_LOCAL)
				QWrite("\"];");
			else
				QWrite("\",");
	}
}






// Get higher units out of number of bytes
FileSize DivideBytes(double bytes) {
	FileSize out;
	out.bytes     = bytes;
	out.kilobytes = 0;
	out.megabytes = 0;

	if (out.bytes >= 1048576) {
		out.megabytes  = out.bytes / 1048576;
		out.megabytes -= fmod(out.megabytes, 1);
		out.bytes     -= out.megabytes * 1048576;
	}

	if (out.bytes >= 1024) {
		out.kilobytes  = out.bytes / 1024;
		out.kilobytes -= fmod(out.kilobytes, 1);
		out.bytes     -= out.kilobytes * 1024;
	}

	return out;
}






// Default values for ParseState struct
void SQM_Init(SQM_ParseState &input) {
	input.i                 = 0;
	input.word_start        = -1;
	input.comment           = SQM_NONE;
	input.expect            = SQM_PROPERTY;
	input.class_level       = 0;
	input.array_level       = 0;
	input.parenthesis_level = 0;
    input.word_started      = false;
	input.first_char        = true;
	input.is_array          = false;
	input.in_quote          = false;
	input.macro             = false;
	input.is_inherit        = false;
	input.purge_comment     = false;
	input.separator         = ' ';
	strcpy(input.empty_char, "");

	// Output
	input.property.text       = input.empty_char;
	input.property.length     = 0;
	input.property_start      = 0;
	input.property_end        = 0;
	input.value.text          = input.empty_char;
	input.value.length        = 0;
	input.value_start         = 0;
	input.value_end           = 0;
	input.class_name.text     = input.empty_char;
	input.class_name.length   = 0;
	input.class_start         = 0;
	input.class_end           = 0;
	input.class_length        = 0;
	input.class_name_full_end = 0;
	input.inherit.text        = input.empty_char;
	input.inherit.length      = 0;
	input.scope_end           = 0;
}

// Read OFP file with classes
int SQM_Parse(String &input, SQM_ParseState &state, int action_type, String &to_find) {
	int initial_level = state.class_level;
	
	for (; state.i<input.length; state.i++) {
		char c = input.text[state.i];

		// Parse preprocessor comment
		switch (state.comment) {
			case SQM_NONE  : {
				if (c == '/' && !state.in_quote) {
					char c2 = input.text[state.i+1];
					
					if (c2 == '/')
						state.comment = SQM_LINE;
					else 
						if (c2 == '*')
							state.comment = SQM_BLOCK;
				}
				
				if (state.comment == SQM_NONE)
					break;
				else {
					if (state.word_started)
						state.purge_comment = true;
					
					continue;
				}
			}
			
			case SQM_LINE  : {
				if (c=='\r' || c=='\n')
					state.comment = SQM_NONE;

				continue;
			}
			
			case SQM_BLOCK : {
				if (state.i>0 && input.text[state.i-1]=='*' && c=='/')
					state.comment = SQM_NONE;

				continue;
			}
		}

		// Parse preprocessor directives
		if (!state.first_char && (c=='\r' || c=='\n')) {
			state.first_char = true;
			
			if (state.macro && input.text[state.i-1] != '\\')
				state.macro = false;
		}
		
		if (!isspace(input.text[state.i])  &&  state.first_char) {
			state.first_char = false;
			
			if (c == '#')
				state.macro = true;
		}
		
		if (state.macro)
			continue;


		// Parse classes
		switch (state.expect) {
			case SQM_SEMICOLON : {
				if (c == ';') {
					state.expect = SQM_PROPERTY;
					continue;
				} else 
					if (!isspace(c))
						state.expect = SQM_PROPERTY;
			}
			
			case SQM_PROPERTY : {
				if (c == '}') {
					state.scope_end = state.i;
					state.expect    = SQM_SEMICOLON;
					state.class_level--;
						
					// If wanted to move to the end of the current scope
					if ((action_type == SQM_ACTION_FIND_CLASS_END || action_type==SQM_ACTION_FIND_CLASS_END_CONVERT) && state.class_level+1==initial_level) {

						// Include separator in the class length
						for (size_t z=state.i; z<=input.length; z++) {
							if (z == input.length || input.text[z]==';' || input.text[z]=='\n') {
								state.i = z;
								break;
							}
						}
						
						state.i++;
						state.class_end    = state.i;
						state.class_length = state.i - state.class_start;
						return SQM_OUTPUT_END_OF_SCOPE;
					}
									
					// End parsing when leaving starting scope
					if (state.class_level < initial_level  ||  action_type == SQM_ACTION_GET_NEXT_ITEM) {
						state.i++;
						return SQM_OUTPUT_END_OF_SCOPE;
						
					}

					continue;
				}
				
				if (isalnum(c) || c=='_' || c=='[' || c==']') {
					if (!state.word_started) {
						state.word_start   = state.i;
						state.word_started = true;
					}
				} else
					if (state.word_started) {
						if (strncmp(input.text+state.word_start,"class",5)==0) {
							state.expect = SQM_CLASS_NAME;
							
							if (action_type != SQM_ACTION_FIND_CLASS_END && action_type != SQM_ACTION_FIND_CLASS_END_CONVERT)
								state.class_start = state.word_start;
						} else 
							if (strncmp(input.text+state.word_start,"enum",4)==0) {
								state.expect    = SQM_ENUM_BRACKET;
								state.separator = '{';
							} else 
								if (strncmp(input.text+state.word_start,"__EXEC",6)==0) {
									state.expect    = SQM_EXEC_BRACKET;
									state.separator = '(';
								} else {
									state.expect          = SQM_EQUALITY;
									state.separator       = '=';
									state.property.text   = input.text + state.word_start;
									state.property_start  = state.word_start;
									state.property_end    = state.i;
									state.property.length = state.property_end - state.property_start;
									state.is_array        = input.text[state.i-2]=='[' && input.text[state.i-1]==']';
								}

						state.word_started = false;
					}
				
				if (state.separator == ' ') {
					break;
				}
			}
			
			case SQM_EQUALITY     : 
			case SQM_ENUM_BRACKET : 
			case SQM_EXEC_BRACKET : {
				if (c == state.separator) {
					state.expect++;
					state.separator = ' ';
				} else 
					if (state.expect==SQM_EQUALITY && c=='(') {
						state.expect            = SQM_MACRO_CONTENT;
						state.separator         = ' ';
						state.parenthesis_level = 1;
					} else 
						if (state.expect == SQM_ENUM_BRACKET) { // ignore what's between "enum" keyword and bracket
							if (c != '{')
								break;
						} else
							if (!isspace(c)) {	//ignore syntax error
								state.i--;
								state.separator = ' ';
								state.expect    = SQM_SEMICOLON;
							}
				
				break;
			}
			
			case SQM_VALUE : {
				if (c == '"')
					state.in_quote = !state.in_quote;

				if (!state.in_quote && (c=='{' || c=='[')) {
					state.array_level++;
					
					if (SQM_ACTION_FIND_CLASS_END_CONVERT)
						input.text[state.i] = '{';
				}

				if (!state.in_quote && (c=='}' || c==']')) {
					state.array_level--;
					
					if (SQM_ACTION_FIND_CLASS_END_CONVERT)
						input.text[state.i] = '}';

					// Remove trailing commas
					/*for (int z=state.i-1; z>0 && (isspace(text[z]) || text[z]==',' || text[z]=='}' || text[z]==']'); z--)
						if (text[z]==',')
							text[z] = ' ';*/
				}

				// Convert semi-colons to commas
				/*if (!state.in_quote && c==';' && state.is_array && state.array_level>0)
					text[state.i] = ',';*/

				if (!state.word_started) {
					if (!isspace(c)) {
						state.word_start   = state.i;
						state.word_started = true;
					}
				} else {
					if (!state.in_quote && state.array_level==0 && (c==';' || c=='\r' || c=='\n')) {
						state.value.text   = input.text + state.word_start;
						state.value_start  = state.word_start;
						state.value_end    = state.i;
						
						// Include separator in the length
						for (size_t z=state.i; z<=input.length; z++) {
							if (z == input.length) {
								state.value_end = z;
								break;
							}
							
							if (input.text[z]==';' || input.text[z]=='\n') {
								state.value_end = z + 1;
								break;
							}
						}
						
						state.value.length = state.value_end - state.word_start;
						state.word_started = false;
						state.expect       = SQM_PROPERTY;
						
						if (
							action_type == SQM_ACTION_GET_NEXT_ITEM || 
							(
								action_type           == SQM_ACTION_FIND_PROPERTY && 
								state.class_level     == initial_level && 
								state.property.length == to_find.length &&
								strncmpi(state.property.text, to_find.text, to_find.length) == 0
							)
						) {
							state.i++;
							return SQM_OUTPUT_PROPERTY;
						}
					}
				}
				
				break;
			}
			
			case SQM_CLASS_NAME    :
			case SQM_CLASS_INHERIT : {
				if (isalnum(c) || c=='_') {
					if (!state.word_started) {
						state.word_start   = state.i;
						state.word_started = true;
					}
				} else
					if (state.word_started) {
						if (state.expect == SQM_CLASS_NAME) {
							state.class_name.text   = input.text + state.word_start;
							state.class_name.length = state.i - state.word_start;
							state.class_name_start  = state.word_start;
							state.class_name_end    = state.i;
							state.inherit.text      = state.empty_char;
							state.inherit.length    = 0;
						} else {
							state.inherit.text   = input.text + state.word_start;
							state.inherit.length = state.i - state.word_start;
						}
						
						state.is_inherit          = state.expect == SQM_CLASS_INHERIT;
						state.word_started        = false;
						state.expect              = state.expect==SQM_CLASS_NAME ? SQM_CLASS_COLON : SQM_CLASS_BRACKET;
						state.class_name_full_end = state.i;
					}
				
				if (state.expect!=SQM_CLASS_COLON && state.expect!=SQM_CLASS_BRACKET)
					break;
			}
			
			case SQM_CLASS_COLON   :
			case SQM_CLASS_BRACKET : {
				if (state.expect==SQM_CLASS_COLON && c==':')
					state.expect = SQM_CLASS_INHERIT;
				else 
					if (c == '{') {
						state.class_level++;
						state.expect = SQM_PROPERTY;
						
						// Return starting position of this class
						if (
							action_type == SQM_ACTION_GET_NEXT_ITEM || 
							(
								action_type             == SQM_ACTION_FIND_CLASS && 
								state.class_level-1     == initial_level && 
								state.class_name.length == to_find.length && 
								strncmpi(state.class_name.text, to_find.text, to_find.length) == 0
							)
						) {
							state.i++;
							return SQM_OUTPUT_CLASS;
						}
					} else
						if (!isspace(c)) {	//ignore syntax error
							state.i--;
							state.expect = SQM_SEMICOLON;
						}
				
				break;
			}
			
			case SQM_ENUM_CONTENT : 
			case SQM_EXEC_CONTENT : {
				if ((state.expect==SQM_EXEC_CONTENT && c==')') || (state.expect==SQM_ENUM_CONTENT && c=='}'))
					state.expect = SQM_SEMICOLON;

				break;
			}
			
			case SQM_MACRO_CONTENT : {
				if (c == '"')
					state.in_quote = !state.in_quote;
					
				if (!state.in_quote) {
					if (c == '(')
						state.parenthesis_level++;
						
					if (c == ')')
						state.parenthesis_level--;
						
					if (state.parenthesis_level == 0)
						state.expect = SQM_SEMICOLON;
				}
					
				break;
			}
		}
	}
	
	state.scope_end = input.length;
	return SQM_OUTPUT_END_OF_SCOPE;
}





// Copy classes and properties from "merge" to "source"
int SQM_Merge(String &merge, SQM_ParseState &merge_state, StringDynamic &source_dynamic, SQM_ParseState &source_state, char *arg_setpos_line) {
	int merge_parse_result             = 0;
	SQM_ParseState source_state_backup = source_state;
	bool buffer_modified               = false;
	char empty_char[]                  = "";
	String empty_string                = {empty_char, 0};
	
	while ((merge_parse_result = SQM_Parse(merge, merge_state, SQM_ACTION_GET_NEXT_ITEM, empty_string))) {
		switch (merge_parse_result) {
			case SQM_OUTPUT_PROPERTY : {
				// Convert array format from SQS to SQM
				if (merge_state.property.text[merge_state.property.length-1]==']' && merge_state.property.text[merge_state.property.length-2]=='[') {
					bool in_quote = false;

					for (size_t i=0; i<merge_state.value.length; i++) {
						if (!in_quote  &&  merge_state.value.text[i]=='[')
							merge_state.value.text[i] = '{';

						if (!in_quote  &&  merge_state.value.text[i]==']')
							merge_state.value.text[i] = '}';

						if (merge_state.value.text[i] == '"')
							in_quote = !in_quote;
					}
				}

				source_state      = source_state_backup;
				String source     = {source_dynamic.text, source_dynamic.length};
				int source_result = SQM_Parse(source, source_state, SQM_ACTION_FIND_PROPERTY, merge_state.property);

				// Replace property
				switch (source_result) {
					case SQM_OUTPUT_PROPERTY : {
						// Special "init" property modification
						if (arg_setpos_line) {
							char tofind[]     = "this setpos";
							String tofind_str = {tofind, strlen(tofind)};
							char *found       = String_find(source_state.value, tofind_str, 0);
							size_t found_pos  = found - source_state.value.text;
							size_t begin_pos  = 0;
							size_t end_pos    = 0;
															
							// If "this setpos" already exists
							if (found) {
								int bracket_level = 0;
								bool in_quote     = false;
								
								// Parse this comamnd line to find closing bracket
								for (size_t i=found_pos; i<source_state.value.length; i++) {
									if (source_state.value.text[i] == '\"')
										in_quote = !in_quote;
									
									if (!in_quote) {
										if (source_state.value.text[i] == '[') {
											if (bracket_level == 0)
												begin_pos = i;
											bracket_level++;
										}
										
										if (source_state.value.text[i] == ']') {
											bracket_level--;
											if (bracket_level == 0) {
												end_pos = i;
												break;
											}
										}
									}
								}
							}
							
							// Replace this command with an updated one
							if (begin_pos && end_pos) {									
								merge_state.value.text   += 1;
								merge_state.value.length -= 6;	// trim to the closing bracket
								
								size_t thissetpos_len = end_pos - found_pos;

								size_t shift_amount  = merge_state.value.length >= thissetpos_len ? merge_state.value.length-thissetpos_len : thissetpos_len-merge_state.value.length;
								bool shift_direction = merge_state.value.length >= thissetpos_len;
							
								char *end = source_state.value.text + end_pos;
								shift_buffer_chunk(source_dynamic.text, end-source_dynamic.text, source_dynamic.length, shift_amount, shift_direction);								
								memcpy(found, merge_state.value.text, merge_state.value.length);
								
								source_dynamic.length                     += shift_amount * (shift_direction ? 1 : -1);
								source_dynamic.text[source_dynamic.length] = '\0';
								buffer_modified                            = true;
							} else {
								// If there's no "this setpos" then prepend it
								merge_state.value.text   += 1;
								merge_state.value.length -= 3;	// Get rid of quotation marks

								shift_buffer_chunk(source_dynamic.text, source_state.value_start+1, source_dynamic.length, merge_state.value.length, OPTION_RIGHT);
								memcpy(source_state.value.text+1, merge_state.value.text, merge_state.value.length);

								source_dynamic.length                     += merge_state.value.length;
								source_dynamic.text[source_dynamic.length] = '\0';
								buffer_modified                            = true;
							}
						} else {
							// Normal property value replacement
							size_t shift_amount  = merge_state.value.length >= source_state.value.length ? merge_state.value.length-source_state.value.length : source_state.value.length-merge_state.value.length;
							bool shift_direction = merge_state.value.length >= source_state.value.length;

							shift_buffer_chunk(source_dynamic.text, source_state.value_end, source_dynamic.length, shift_amount, shift_direction);
							memcpy(source_state.value.text, merge_state.value.text, merge_state.value.length);

							source_dynamic.length                     += shift_amount * (shift_direction ? 1 : -1);
							source_dynamic.text[source_dynamic.length] = '\0';
							buffer_modified                            = true;
						}
					} break;

					case SQM_OUTPUT_END_OF_SCOPE : {
						// Add a new property
						
						// Check what's the last character in the source
						bool add_semicolon = false;
						for (size_t z=source_state.scope_end-1;  source_state.scope_end>0 && source_dynamic.length>0;  z--) {
							if (source_dynamic.text[z]=='\n' || source_dynamic.text[z]==';')
								break;
							else
								if (!isspace(source_dynamic.text[z])) {
									add_semicolon = true;
									break;
								}

							if (z == 0)
								break;
						}
						
						const size_t added_length = (merge_state.value_end - merge_state.property_start) + add_semicolon;

						shift_buffer_chunk(source_dynamic.text, source_state.scope_end, source_dynamic.length, added_length, OPTION_RIGHT);
						
						if (add_semicolon)
							memset(source_dynamic.text+source_state.scope_end, ';', 1);
													
						memcpy(source_dynamic.text+source_state.scope_end+add_semicolon, merge_state.property.text, merge_state.value_end-merge_state.property_start);

						source_dynamic.length                     += added_length;
						source_dynamic.text[source_dynamic.length] = '\0';
						buffer_modified                            = true;
					} break;
				}
			} break;
			
			case SQM_OUTPUT_CLASS : {
				source_state      = source_state_backup;
				String source     = {source_dynamic.text, source_dynamic.length};
				int source_result = SQM_Parse(source, source_state, SQM_ACTION_FIND_CLASS, merge_state.class_name);
				
				switch (source_result) {
					case SQM_OUTPUT_CLASS : {
						// If the class already exists in the source then scan it recursively
						if (SQM_Merge(merge, merge_state, source_dynamic, source_state, arg_setpos_line))
							buffer_modified = true;
					} break;

					case SQM_OUTPUT_END_OF_SCOPE : {
						// If the class doesn't exist in the source then copy the entire thing
						SQM_Parse(merge, merge_state, SQM_ACTION_FIND_CLASS_END_CONVERT, empty_string);
						
						shift_buffer_chunk(source_dynamic.text, source_state.scope_end, source_dynamic.length, merge_state.class_length, OPTION_RIGHT);
						memcpy(source_dynamic.text+source_state.scope_end, merge.text+merge_state.class_start, merge_state.class_length);

						source_dynamic.length                     += merge_state.class_length;
						source_dynamic.text[source_dynamic.length] = '\0';
						buffer_modified                            = true;
					} break;
				}
			} break;
		}
	}

	return buffer_modified;
}

char* strstr2_old(const char *arg1, size_t arg1_len, const char *arg2, size_t arg2_len, int options) {	//TODO: remove this function
	if (arg1_len==0 || arg2_len==0)
		return NULL;
	
	for (size_t pos=0;  pos<arg1_len;  pos++) {
		size_t pos_arg1 = pos;
		size_t pos_arg2 = 0;

		// Compare arg1+pos with arg2
		while (
			options & OPTION_CASESENSITIVE 
				? arg1[pos_arg1++] == arg2[pos_arg2++] 
				: (arg1[pos_arg1++] | 32) == (arg2[pos_arg2++] | 32)
		) {
			if (pos_arg2 == arg2_len) {
				if (~options & OPTION_MATCHWORD  
					||  
					// If matching the whole word then occurrence musn't be surrounded by graphic characters
					(options & OPTION_MATCHWORD && 
						// If left side of the match is empty
						(pos==0  ||  (pos>0  &&  !isalnum(arg1[pos-1])  &&  arg1[pos-1]!='_')) 
						&& 
						// If right side of the match is empty
						(pos+arg2_len>=arg1_len  ||  (pos+arg2_len<arg1_len  &&  !isalnum(arg1[pos+arg2_len])  &&  arg1[pos+arg2_len]!='_'))
					)
				) 
					return (char*)(arg1+pos);
				
				// If failed to match the whole word then move forward
				pos += arg2_len;
				break;
			}
		}
	}

	return NULL;
}




// Output text to the game with specified length
void QWrites(String &input) {
	QWritel(input.text, input.length);
}

// Output text to the game doubling the amount of quotation marks
void QWriteq(const char *str) {
	size_t pos  = 0;
	char *quote = NULL;
	
	while ((quote = strchr(str, '"'))) {
		pos = quote - str;
		QWritel(str, pos+1);
		QWrite("\"");
		str = quote + 1;
	}
	
	QWrite(str);
}

void QWritesq(String input) {
	size_t pos  = 0;
	char *quote = NULL;
	
	while ((quote = strchr(input.text, '"'))) {
		pos = quote - input.text;
		QWritel(input.text, pos+1);
		QWrite("\"");
		input.text    = quote + 1;
		input.length -= pos + 1;
	}
	
	QWrites(input);
}