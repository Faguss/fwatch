// -----------------------------------------------------------------
// CLIPBOARD OPERATIONS
// -----------------------------------------------------------------

case C_CLIPBOARD_COPY:
{ // Copy text to clipboard

	// Define vars
	bool Append			= false;
	char *txt			= "";
	char *escape		= "";

	// Read arguments
	for (int i=2; i<numP; i++)
	{
		char *arg = stripq(par[i]);
		char *pch = strchr(arg, ':');

		if (pch == NULL) 
			continue;

		int pos = pch - arg;
		arg[pos] = '\0';
		char *val = Trim(arg+pos+1);

		if (strcmpi(arg,"append")==0) 
			Append = String2Bool(val);

		if (strcmpi(arg,"escape")==0)
			escape = val;

		if (strcmpi(arg,"text")==0)
			txt = val;
	};


	// Options
	char tmp[16]	 = "";
	bool numberStart = false;
	int quantity	 = -1;

	for (i=0; escape[i]!='\0'; i++)
	{
		if (!numberStart  &&  isdigit(escape[i]))
			numberStart = true;

		if (numberStart)
			if (!isdigit(escape[i]))
				numberStart = false,
				quantity = atoi(tmp),
				strcpy(tmp, "");
			else
				sprintf(tmp, "%s%c", tmp, escape[i]);
		
		if (escape[i] == 't')
			txt = EscSequences(txt,0,quantity),
			quantity = -1;

		if (escape[i] == 'n')
			txt = EscSequences(txt,1,quantity),
			quantity = -1;
	};


	if (CopyToClip(txt, Append, CommandID, out)) 
		FWerror(0,0,CommandID,"","",0,0,out);
}
break;









case C_CLIPBOARD_GET:		
{ // Return text from the system clipboard

	unsigned int startfrom	= 0;
	unsigned int howmany	= 0; 

	if (numP > 2) 
		startfrom = atoi(par[2]);

	if (numP > 3) 
		howmany = atoi(par[3]);


	// Open clipboard
	if (!OpenClipboard(NULL)) 
		break;


	// Return result only if text is in clibpoard
	if (::IsClipboardFormatAvailable(CF_TEXT))
	{
		// Get clipboard text
		HANDLE hClipboardData	= GetClipboardData(CF_TEXT);
		char *pchData			= (char*)GlobalLock(hClipboardData);
		unsigned int len		= strlen(pchData);

		// if specified size then cut string
		if (howmany>0  &&  startfrom+howmany<len)
			pchData[(startfrom+howmany)] = '\0';
		
		// if specified start position
		if (startfrom != 0)
			QWrite(pchData + startfrom, out);
		else 
			QWrite(pchData,out);

		GlobalUnlock(hClipboardData);
	};


	// Close
	CloseClipboard();
}
break;









case C_CLIPBOARD_GETLINE:		
{ // Return line(s) from the system clipboard

	// Open clipboard and check if contains text
	if (!OpenClipboard(NULL)) 
	{
		FWerror(20,GetLastError(),CommandID,"","",0,0,out);
		QWrite("[]]", out);
		break;
	};

	if (!::IsClipboardFormatAvailable(CF_TEXT))
	{
		FWerror(21,0,CommandID,"","",0,0,out);
		QWrite("[]]", out);
		CloseClipboard();
		break;
	};


	// Parse input arguments
	char *pch		= "";
	bool FullLines	= false;
	bool Cut		= false;
	int Start		= 0;
	int End			= 0;
	int Limit		= 122;
	int split		= 0;
	int CurrentROW	= 1;
	int size		= 0;
	int lastPos		= 0;
	int error		= 0;

	for (int i=1; i<numP; i++)
	{
		char *arg = stripq(par[i]);
		pch = strchr(arg, ':');

		if (pch==NULL) 
			continue;

		int pos = pch-arg;
		arg[pos] = '\0';
		char *val = arg+pos+1;

		if (strcmpi(arg,"start") == 0) 
			Start = atoi(val);

		if (strcmpi(arg,"end") == 0)
			End = atoi(val);

		if (strcmpi(arg,"cut") == 0)
			Cut = true,
			Limit = atoi(val); 				

		if (strcmpi(arg,"split") == 0)
			FullLines = String2Bool(val);
	};

	if (Limit < 0) 
		Limit = 0;


	// Get clipboard text
	HANDLE hClipboardData	= GetClipboardData(CF_TEXT);
	char *pchData			= (char*)GlobalLock(hClipboardData);
	size					= strlen(pchData);


	// Allocate buffer
	int linesLen	= 20;
	char *lines		= (char*) malloc (linesLen);
	char *linesNEW	= "";

	if (lines == NULL)
	{
		FWerror(10,0,CommandID,"lines","",linesLen,0,out);
		QWrite("[]]", out);
		CloseClipboard();
		break;
	}
	else
		strcpy(lines,"[");


	// Tokenize by lines
	int CurrentCOL	= 0;
	int pos			= 0;
	int end			= 0;
	char *line		= pchData;

	while (!end  &&  !error  &&  (CurrentROW<=End && End!=0 || End==0))
	{	
		// Find end of line OR go to end of the string
		pch = strchr(line, '\n');

		if (pch == NULL) 
			pch = line + strlen(line), 
			end = 1;


		// Put \0 to separate current line from the next
		int pos		= pch - line;
		line[pos]	= '\0';

		if (line[pos-1] == '\r') 
			line[pos-1] = '\0';

		// If current line number is in range
		if (CurrentROW >= Start  &&  Start>0  ||  Start==0)
		{
			char *part	= line;
			int len		= strlen(line);

			if (FullLines) 
				strcat(lines,"]+[["); 
			else 
				strcat(lines, "]+[");

			
			// If empty line
			if (len == 0) 
				strcat(lines, "\"\"");


			// Split line (if optional mode) and return it
			for (int i=0; (i<len  &&  FullLines  ||  !FullLines  &&  i==0  &&  len!=0); i+=Limit, part=line+i)
			{
				strcat(lines, "]+[\"");
					
				// Split
				char prev = part[Limit];

				if (Cut) 
					part[Limit] = '\0';

				// Double the amount of quotation marks
				bool replaced	= false;
				char *rep		= "";
				char *final		= part;

				if (strchr(part,'\"'))
				{
					rep = str_replace(part,"\"","\"\"",0,0);
					if (rep == NULL) 
					{
						FWerror(12,0,CommandID,"part (' with '')","",strlen(part),0,out);
						error = 1; 
						break;
					};

					replaced = true;
					final = rep;
				};


				// Reallocate buffer
				linesLen += strlen(final) + 20;
				linesNEW = (char*) realloc(lines, linesLen);

				if (linesNEW != NULL) 
					lines = linesNEW; 
				else 
				{	
					FWerror(11,0,CommandID,"lines","",linesLen,0,out);
					error = 1; 
					break;
				};
				

				// Add current line to the buffer
				strcat(lines, final);
				strcat(lines, "\"");
				part[Limit] = prev;

				if (replaced) 
					free(rep);
			};

			if (FullLines) 
				strcat(lines, "]");
		};

		if (end) 
			break;

		CurrentROW++;
		CurrentCOL += pos+1;
		line		= pchData + CurrentCOL;
	};


	// Return value
	if (!error) 
		FWerror(0,0,CommandID,"","",0,0,out),
		QWrite(lines, out),
		QWrite("]]", out);
	else
		QWrite("[]]", out);


	GlobalUnlock(hClipboardData);
	CloseClipboard();
	free(lines);
}
break;









case C_CLIPBOARD_SIZE:
{ // Return length of the text stored in the system clipboard

	if (!OpenClipboard(NULL)) 
	{
		FWerror(20,GetLastError(),CommandID,"","",0,0,out);
		QWrite("0,0]", out);
		break;
	};

	if (!::IsClipboardFormatAvailable(CF_TEXT))
	{
		FWerror(21,0,CommandID,"","",0,0,out);
		QWrite("0,0]", out);
		CloseClipboard();
		break;
	};

	HANDLE hClipboardData	= GetClipboardData(CF_TEXT);
	char *pchData			= (char*)GlobalLock(hClipboardData);
	char *pch				= "";
	char *txt				= pchData;
	char tmp[26]			= "";
	int size				= strlen(pchData);
	int lines				= 1;
	int	CurrentCOL			= 0;
	int pos					= 0;


	// Count how many lines
	while (pch = strstr(txt, "\n"))
	{
		pos = pch-txt;
		lines++;
		CurrentCOL += pos+1;
		txt = pchData + CurrentCOL;
	};


	FWerror(0,0,CommandID,"","",0,0,out);
	sprintf(tmp,"%d,%d]",size,lines);
	QWrite(tmp, out);

	GlobalUnlock(hClipboardData);
	CloseClipboard();
}
break;









case C_CLIPBOARD_TOFILE:
{ // Paste clipboard content to file

	// Read arguments
	char *arg_filename = "";
	char *arg_mode     = "nooverwrite";
	char *arg_escape   = "";
	char *arg_prefix   = "";
	char *arg_suffix   = "";
	
	for (int i=2; i<numP; i++) {
		char *arg = stripq(par[i]);
		char *col = strchr(arg, ':');

		if (col == NULL)
			continue;

		int pos   = col - arg;
		arg[pos]  = '\0';
		char *val = arg+pos+1;

		if (strcmpi(arg,"file") == 0) {
			arg_filename = val;
			continue;
		}

		if (strcmpi(arg,"mode") == 0) {
			arg_mode = val;
			continue;
		}

		if (strcmpi(arg,"prefix") == 0) {
			arg_prefix = val;
			continue;
		}

		if (strcmpi(arg,"suffix") == 0) {
			arg_suffix = val;
			continue;
		}

		if (strcmpi(arg,"escape") == 0) {
			arg_escape = val;
			continue;
		}
	}

	// File not specified
	if (strcmpi(arg_filename, "") == 0) {
		FWerror(107,0,CommandID,"file name","",0,0,out);
		break;
	};


	// Replace \t and \n
	char tmp[16]	 = "";
	bool numberStart = false;
	int quantity	 = -1;

	for (i=0; arg_escape[i]!='\0'; i++) {
		if (!numberStart  &&  isdigit(arg_escape[i]))
			numberStart = true;

		if (numberStart)
			if (!isdigit(arg_escape[i])) {
				numberStart = false;
				quantity    = atoi(tmp);
				strcpy(tmp, "");
			} else
				sprintf(tmp, "%s%c", tmp, arg_escape[i]);
		
		if (arg_escape[i] == 't') {
			arg_prefix = EscSequences(arg_prefix, 0, quantity);
			arg_suffix = EscSequences(arg_suffix, 0, quantity);
			quantity = -1;
		}

		if (arg_escape[i] == 'n') {
			arg_prefix = EscSequences(arg_prefix, 2, quantity);
			arg_suffix = EscSequences(arg_suffix, 2, quantity);
			quantity = -1;
		}
	};

	bool overwrite = false;
	char open_mode[] = "wb";

	if (strcmpi(arg_mode,"append")==0)
		strcpy(open_mode, "ab");


	// Open clipboard
	if(!OpenClipboard(NULL)) {	
		FWerror(20,GetLastError(),CommandID,"","",0,0,out);
		break;
	}

	// No text in clip
	if (!::IsClipboardFormatAvailable(CF_TEXT)) {
		FWerror(21,0,CommandID,"","",0,0,out);
		CloseClipboard();
		break;
	};

	// Verify and update path to the file
	String buf_filename;
	String_init(buf_filename);
	char *ptr_filename = arg_filename;
	int path_type      = VerifyPath(&ptr_filename, buf_filename, RESTRICT_TO_MISSION_DIR, CommandID, out);

	if (path_type == ILLEGAL_PATH) {
		CloseClipboard();
		break;
	}

	// Check if file already exists
	FILE *f     = fopen(ptr_filename, "r");
	bool exists = false;

	if (f) {
		exists = true;
		fclose(f);
	}

	// Overwriting not allowed
	if (exists  &&  strcmpi(arg_mode,"nooverwrite")==0) {
		FWerror(207,0,CommandID,ptr_filename,"",0,0,out);
		CloseClipboard();
		String_end(buf_filename);
		break;
	}

	// Mode "overwrite" means trashing existing file
	if (exists  &&  strcmpi(arg_mode,"overwrite")==0) {
		if (path_type == DOWNLOAD_DIR) {	// Delete if it's \fwatch\download\ location
			if (remove(ptr_filename) != 0) {
				FWerror(7,errno,CommandID,ptr_filename,"",0,0,out);
				CloseClipboard();
				String_end(buf_filename);
				break;
			}
		} else {
			if (!trashFile(ptr_filename,CommandID,out,0)) {	// Trash for all other places
				CloseClipboard();
				String_end(buf_filename);
				break;
			}
		}
	}


	// Write file
	if (f = fopen(ptr_filename, open_mode)) {
		if (strcmp(arg_prefix,"")!=0)
			fprintf(f, arg_prefix);

		HANDLE clipboard_data = GetClipboardData(CF_TEXT);
		char *clipboard_text  = (char*)GlobalLock(clipboard_data);

		fwrite(clipboard_text, 1, strlen(clipboard_text), f);

		if (ferror(f)) 
			FWerror(7,errno,CommandID,ptr_filename,"",0,0,out);
		else {
			FWerror(0,0,CommandID,"","",0,0,out);

			if (strcmp(arg_suffix,"")!=0)
				fprintf(f, arg_suffix);
		}

		GlobalUnlock(clipboard_data);
		fclose(f);
	} else 
		FWerror(7,errno,CommandID,ptr_filename,"",0,0,out);

	CloseClipboard();
	String_end(buf_filename);
}
break;









case C_CLIPBOARD_FROMFILE:
{ // Paste file content to clipboard

	// Read arguments
	if (numP < 3) {
		FWerror(100,0,CommandID,"","",numP,3,out);
		break;
	}

	
	// Verify and update path to the file
	String buf_filename;
	String_init(buf_filename);
	char *ptr_filename = stripq(par[2]);

	if (!VerifyPath(&ptr_filename, buf_filename, ALLOW_GAME_ROOT_DIR, CommandID, out))
		break;


	// Open file
	FILE *f = fopen(ptr_filename, "rb");
	if (!f) {
		FWerror(7,errno,CommandID,ptr_filename,"",0,0,out);
		String_end(buf_filename);
		break;
	}


	// Find file size, allocate buffer
	fseek(f, 0, SEEK_END);
	unsigned int fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	char *filetext = new char[fsize+1];

	if (!filetext) {
		FWerror(10,0,CommandID,"filetext","",fsize+1,0,out);
		String_end(buf_filename);
		fclose(f); 
		break;
	}


	// Read contents and copy
	unsigned int result = fread(filetext,1,fsize,f);

	if (result == fsize) {
		filetext[fsize] = '\0';
		bool append     = numP>3 && strcmpi(par[3],"a") == 0;

		if (CopyToClip(filetext, append, CommandID, out))
			FWerror(0,0,CommandID,"","",0,0,out);
	} else
		FWerror(7,errno,CommandID,ptr_filename,"",0,0,out); 

	delete[] filetext;
	String_end(buf_filename);
	fclose(f);
}
break;









case C_CLIPBOARD_COPYFILE:
case C_CLIPBOARD_CUTFILE:
{ // Copy file names to clipboard

	// Not enough arguments
	if (numP < 3) {
		FWerror(100,0,CommandID,"","",numP,3,out);
		break;
	}

	TCHAR pwd[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, pwd);

	int pwd_len          = strlen(pwd);
	int all_files_length = strlen(com) + (pwd_len+strlen(MissionPath)+2) * (numP-2);
	int copied_files     = 0;
	int check_mode       = SUPPRESS_ERROR | (CommandID==C_CLIPBOARD_COPYFILE ? ALLOW_GAME_ROOT_DIR : RESTRICT_TO_MISSION_DIR);

	String buf_filename;
	String_init(buf_filename);


	// Create list of files separated by \0 with paths starting from drive
	// http://www.codeguru.com/cpp/w-p/clipboard/article.php/c2997/Copying-Files-into-Explorer.htm
	DROPFILES dobj = { 20, { 0, 0 }, 0, 1 };
	int nGblLen    = sizeof(dobj) + all_files_length*2 + 5;//lots of nulls and multibyte_char
	HGLOBAL hGbl   = GlobalAlloc(GMEM_ZEROINIT|GMEM_MOVEABLE|GMEM_DDESHARE, nGblLen);
	char* sData    = (char*)GlobalLock(hGbl);
	memcpy(sData, &dobj, 20);
	wchar_t* sWStr = (wchar_t*)(sData + 20);

	// Parse arguments passed to this command
	for (int i=2; i<numP; i++) {
		char *ptr_filename = stripq(par[i]);

		if (VerifyPath(&ptr_filename, buf_filename, check_mode, CommandID, out)) {
			int ptr_filename_len = buf_filename.current_length>0 ? buf_filename.current_length : strlen(ptr_filename);

			mbstowcs(sWStr          , pwd         , pwd_len);
			mbstowcs(sWStr+pwd_len  , "\\"        , 1);
			mbstowcs(sWStr+pwd_len+1, ptr_filename, ptr_filename_len);

			sWStr += pwd_len + 1 + ptr_filename_len + 1;
			buf_filename.current_length = 0;
			copied_files++;
		}
	}

	String_end(buf_filename);

	// If nothing was copied
	if (copied_files == 0) {
		FWerror(105,0,CommandID,"","",0,0,out),
		GlobalUnlock(hGbl);
		break;
	}


	// Copy to the clipboard
	bool error = false;

	if (OpenClipboard(NULL)) {
		if (!EmptyClipboard()) {
			error = true;
			FWerror(22,GetLastError(),CommandID,"","",0,0,out);
		}

		if (!SetClipboardData(CF_HDROP, hGbl)) {
			error = true;
			FWerror(23,GetLastError(),CommandID,"","",0,0,out);
		}

		// Register 'copy' or 'cut' effect
		UINT uFormat       = RegisterClipboardFormat(CFSTR_PREFERREDDROPEFFECT);
		HGLOBAL hGbl2      = GlobalAlloc(GMEM_FIXED, sizeof(DWORD));
		DWORD* pDropEffect = (DWORD*)GlobalLock(hGbl2);
		*pDropEffect       = CommandID==C_CLIPBOARD_COPYFILE ? DROPEFFECT_COPY : DROPEFFECT_MOVE;

		SetClipboardData(uFormat, hGbl2);
		GlobalUnlock(hGbl2);
		CloseClipboard();
	} else {
		error = true;
		FWerror(20,GetLastError(),CommandID,"","",0,0,out);
	}

	GlobalUnlock(hGbl);

	if (!error)
		FWerror(0,0,CommandID,"","",0,0,out);
}
break;









case C_CLIPBOARD_PASTEFILE:
{ // Paste files from clipboard
	//http://bcbjournal.org/forums/viewtopic.php?f=10&t=1363

	// Check optional argument
	bool list_files = numP>2  &&  strcmpi(par[2],"?list")==0;


	// Verify and update path to the file
	String buf_destination;
	String_init(buf_destination);
	char *ptr_destination = numP>2 && !list_files ? stripq(par[2]) : "";
	int destination_type  = 0;
	int destination_len   = 0;

	if (!list_files) {
		destination_type = VerifyPath(&ptr_destination, buf_destination, RESTRICT_TO_MISSION_DIR, CommandID, out);

		if (destination_type != ILLEGAL_PATH) {
			if (buf_destination.current_length == 0)
				String_append(buf_destination, ptr_destination);

			if (buf_destination.pointer[buf_destination.current_length-1] != '\\')
				String_append(buf_destination, "\\");

			destination_len = buf_destination.current_length;
		} else {
			QWrite("\"\",[]]", out);
			String_end(buf_destination);
			break;
		}
	}


	// Open clipboard
	if (!OpenClipboard(NULL)) {
		FWerror(20,GetLastError(),CommandID,"","",0,0,out);
		QWrite("\"\",[]]", out);
		String_end(buf_destination);
		break;
	}


	// Check format
	if (!IsClipboardFormatAvailable(CF_HDROP)) {
		FWerror(21,0,CommandID,"","",0,0,out);
		QWrite("\"\",[]]", out);
		CloseClipboard();
		String_end(buf_destination);
		break;
	}


	// Find registered effect
	DWORD dwEffect = 0;
	DWORD dwFormat = RegisterClipboardFormat(CFSTR_PREFERREDDROPEFFECT);
	HANDLE hGlobal = GetClipboardData(dwFormat);

	if (hGlobal) {
		LPDWORD pdwEffect = (LPDWORD) GlobalLock(hGlobal);
		dwEffect          = *pdwEffect;
		GlobalUnlock(hGlobal);
	}

	int effect[] = {
		DROPEFFECT_NONE,
		DROPEFFECT_COPY,
		DROPEFFECT_MOVE,
		DROPEFFECT_LINK,
		DROPEFFECT_SCROLL
	};

	char effect_name[][16] = {
		"none",
		"copy",
		"move",
		"link",
		"scroll"
	};

	int effect_id   = 0;
	int effect_size = sizeof(effect) / sizeof(effect[0]);

	for (int i=0; i<effect_size; i++)
		if (dwEffect & effect[i]) {
			effect_id = i;
			break;
		}


	// Prohibit actions other than move or copy
	if (!list_files  &&  ~dwEffect & DROPEFFECT_COPY  &&  ~dwEffect & DROPEFFECT_MOVE) {
		FWerror(25,0,CommandID,"","",0,0,out);
		QWrite("\"", out); 
		QWrite(effect_name[effect_id], out); 
		QWrite("\",[]]", out);
		CloseClipboard();
		String_end(buf_destination);
		break;
	}


	// Get data
	HANDLE Data = GetClipboardData(CF_HDROP);

	if (Data == NULL) {
		FWerror(24,0,CommandID,"","",0,0,out);
		QWrite("\"\",[]]", out);
		CloseClipboard();
		String_end(buf_destination);
		break;
	}

	DROPFILES *df = (DROPFILES*) GlobalLock(Data);

	if (!df->fWide)	{
		FWerror(24,0,CommandID,"","",0,0,out);
		QWrite("\"\",[]]", out);
		GlobalUnlock(Data);
		CloseClipboard();
		String_end(buf_destination);
		break;
	}

	LPWSTR pFilenames = (LPWSTR) (((LPBYTE)df) + df->pFiles);


	// Create output array
	FWerror(0,0,CommandID,"","",0,0,out);
	QWrite("\"", out); 
	QWrite(effect_name[effect_id], out); 
	QWrite("\",[", out);


	// Get current working dir
	TCHAR pwd[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, pwd);


	// Parse filenames
	String buf_source;
	String_init(buf_source);
	char *ptr_source = "";

	int buf_dest_len = buf_destination.current_length;
	int pwd_len      = strlen(pwd);
	int mission_len  = strlen(MissionPath);
	int name_start   = 0;
	ErrorWithinError = true;

	for (i=0; pFilenames[name_start]!='\0'; i++) {
		if (pFilenames[i] == '\0') {
			// Convert wide string to multi-byte string
			wchar_t *wide_path      = pFilenames + name_start;
			size_t wide_path_length = wcslen(wide_path) + 1;
			
			if (String_allocate(buf_source, wide_path_length) == 0) 
				wcstombs(buf_source.pointer, wide_path, wide_path_length);
			else {
				QWrite("]]]", out);
				FWerror(10,0,CommandID,"buf_source","",wide_path_length,0,out);
				String_end(buf_source);
				break;
			}


			// Convert path (starting from drive) to OFP format for logging and for verification
			char *absolute_path = buf_source.pointer;
			char *relative_path = absolute_path;
			bool is_game_dir    = false;

			if (strncmpi(absolute_path, pwd, pwd_len) == 0) {
				absolute_path += pwd_len + 1;
				relative_path  = absolute_path;
				is_game_dir    = true;

				if (strncmpi(absolute_path, MissionPath, mission_len) == 0)
					relative_path += mission_len;
				else {
					memcpy(absolute_path-3, "..\\", 3);
					relative_path = absolute_path - 3;
				}
			}


			// Log
			QWrite("]+[[\"", out);
			QWrite(relative_path, out);
			QWrite("\",", out);


			// Verify path
			int source_type = VerifyPath(&relative_path, buf_source, SUPPRESS_ERROR | SUPPRESS_CONVERSION, CommandID, out);
			bool allow_copy = true;


			// Not allowed to move files to the download directory
			if (is_game_dir  &&  dwEffect & DROPEFFECT_MOVE  &&  source_type!=DOWNLOAD_DIR  &&  destination_type==DOWNLOAD_DIR) {
				FWerror(202,0,CommandID,relative_path,"",0,0,out);
				allow_copy = false;
			}


			// Copy filename from the source path to the destination path
			char *last_slash = strrchr(relative_path, '\\');
			String_append(buf_destination, last_slash!=NULL ? last_slash+1 : relative_path);


			// Perform file operation
			if (allow_copy)
				if (!list_files) {
					if (dwEffect & DROPEFFECT_MOVE)
						if (MoveFileEx(absolute_path,buf_destination.pointer,0) == 0) {
							FWerror(0,0,CommandID,"","",0,0,out);
							EmptyClipboard();
						} else 
							FWerror(7,errno,CommandID,absolute_path,"",0,0,out);

					if (dwEffect & DROPEFFECT_COPY)
						if (CopyFile((LPCTSTR)absolute_path, (LPCTSTR)buf_destination.pointer, true)) 
							FWerror(0,0,CommandID,"","",0,0,out);
						else 
							FWerror(5,GetLastError(),CommandID,"","",0,0,out);
				} else
					FWerror(0,0,CommandID,"","",0,0,out);


			// Reset
			buf_source.current_length      = 0;
			buf_destination.current_length = destination_len;
			name_start = i + 1;
			QWrite("]", out);
		}
	}

	ErrorWithinError = false;
	QWrite("]]", out);

	String_end(buf_source);
	String_end(buf_destination);

	GlobalUnlock(Data);
	CloseClipboard();
}
break;

