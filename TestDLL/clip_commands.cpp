// -----------------------------------------------------------------
// CLIPBOARD OPERATIONS
// -----------------------------------------------------------------

case C_CLIP_COPY:
{ // Copy text to clipboard

	bool append     = false;
	char *text      = empty_string;
	char *escape    = empty_string;
	int text_length = 0;

	for (size_t i=2; i<argument_num; i+=2) {
		switch (argument_hash[i]) {
			case NAMED_ARG_APPEND :
				append = String2Bool(argument[i+1]);
				break;

			case NAMED_ARG_ESCAPE : 
				escape = argument[i+1];
				break;

			case NAMED_ARG_TEXT : 
				text        = argument[i+1];
				text_length = argument_length[i+1];
				break;
		}
	}


	// Process escape sequences
	char tmp[16]	  = "";
	bool number_start = false;
	int quantity	  = -1;

	for (i=0; escape[i]!='\0'; i++)	{
		if (!number_start  &&  isdigit(escape[i]))
			number_start = true;

		if (number_start) {
			if (!isdigit(escape[i])) {
				number_start = false;
				quantity     = atoi(tmp);
				strcpy(tmp, "");
			} else
				sprintf(tmp, "%s%c", tmp, escape[i]);
		}
		
		if (escape[i] == 't') {
			text     = EscSequences(text, OPTION_TAB, quantity);
			quantity = -1;
		}

		if (escape[i] == 'n') {
			text     = EscSequences(text, OPTION_LF, quantity);
			quantity = -1;
		}
	}

	if (CopyToClip(text, strlen(text), append)) {
		global.option_error_output = OPTION_ERROR_ARRAY_CLOSE;
		QWrite_err(FWERROR_NONE, 0);
	}
}
break;









case C_CLIP_GET:		
{ // Return text from the system clipboard

	int pos    = 0;
	int length = 0; 

	if (argument_num > 2) 
		pos = atoi(argument[2]);

	if (argument_num > 3) 
		length = atoi(argument[3]);


	if (!OpenClipboard(NULL)) 
		break;

	if (::IsClipboardFormatAvailable(CF_TEXT)) {
		HANDLE hClipboardData = GetClipboardData(CF_TEXT);
		char *clip_text       = (char*)GlobalLock(hClipboardData);
		int clip_length       = strlen(clip_text);

		// if specified size then cut string
		if (length>0  &&  pos+length<clip_length)
			clip_text[pos+length] = '\0';
		
		QWritel(clip_text+pos, length>0 ? length : clip_length-pos);
		GlobalUnlock(hClipboardData);
	}

	// Close
	CloseClipboard();
}
break;









case C_CLIP_GETLINE:		
{ // Return line(s) from the system clipboard

	if (!OpenClipboard(NULL)) {
		QWrite_err(FWERROR_CLIP_OPEN, 1, GetLastError());
		QWrite("[]]");
		break;
	}

	if (!::IsClipboardFormatAvailable(CF_TEXT))	{
		QWrite_err(FWERROR_CLIP_FORMAT, 0);
		QWrite("[]]");
		CloseClipboard();
		break;
	}


	char *pch		= NULL;
	bool FullLines	= false;
	bool Cut		= false;
	int Start		= 0;
	int End			= 0;
	int Limit		= 122;
	int CurrentROW	= 1;
	int size		= 0;
	int error		= 0;

	for (size_t i=2; i<argument_num; i+=2) {
		switch (argument_hash[i]) {
			case NAMED_ARG_START : 
				Start = atoi(argument[i+1]);
				break;

			case NAMED_ARG_END : 
				End = atoi(argument[i+1]);
				break;

			case NAMED_ARG_CUT : 
				Cut   = true;
				Limit = atoi(argument[i+1]);
				if (Limit < 0) Limit=0;
				break;

			case NAMED_ARG_SPLIT : 
				FullLines = String2Bool(argument[i+1]);
				break;
		}
	}




	// Get clipboard text
	HANDLE hClipboardData = GetClipboardData(CF_TEXT);
	char *pchData         = (char*)GlobalLock(hClipboardData);
	size                  = strlen(pchData);


	// Allocate buffer
	int linesLen   = 20;
	char *lines    = (char*) malloc (linesLen);
	char *linesNEW = NULL;

	if (!lines) {
		QWrite_err(FWERROR_MALLOC, 2, "lines", linesLen);
		QWrite("[]]");
		CloseClipboard();
		break;
	} else
		strcpy(lines,"[");


	// Tokenize by lines
	int CurrentCOL = 0;
	int end        = 0;
	char *line     = pchData;

	while (!end  &&  !error  &&  ((CurrentROW<=End && End!=0) || End==0)) {	
		// Find end of line OR go to end of the string
		pch = strchr(line, '\n');

		if (!pch) {
			pch = line + strlen(line);
			end = 1;
		}


		// Put \0 to separate current line from the next
		int pos   = pch - line;
		line[pos] = '\0';

		if (line[pos-1] == '\r') 
			line[pos-1] = '\0';

		// If current line number is in range
		if ((CurrentROW >= Start  &&  Start>0)  ||  Start==0) {
			char *part = line;
			int len    = strlen(line);

			if (FullLines) 
				strcat(lines,"]+[["); 
			else 
				strcat(lines, "]+[");

			
			// If empty line
			if (len == 0) 
				strcat(lines, "\"\"");


			// Split line (if optional mode) and return it
			for (int i=0; ((i<len  &&  FullLines)  ||  (!FullLines  &&  i==0  &&  len!=0)); i+=Limit, part=line+i) {
				strcat(lines, "]+[\"");
					
				// Split
				char prev = part[Limit];

				if (Cut) 
					part[Limit] = '\0';

				// Double the amount of quotation marks
				bool replaced = false;
				char *rep     = NULL;
				char *final   = part;

				if (strchr(part,'\"')) {
					rep = str_replace(part,"\"","\"\"",OPTION_NONE);
					if (!rep) {
						QWrite_err(FWERROR_STR_REPLACE, 2, "part (' with '')", strlen(part));
						error = 1; 
						break;
					}

					replaced = true;
					final    = rep;
				}


				// Reallocate buffer
				linesLen += strlen(final) + 20;
				linesNEW = (char*) realloc(lines, linesLen);

				if (linesNEW) 
					lines = linesNEW; 
				else {	
					QWrite_err(FWERROR_REALLOC, 2, "lines", linesLen);
					error = 1; 
					break;
				}
				

				// Add current line to the buffer
				strcat(lines, final);
				strcat(lines, "\"");
				part[Limit] = prev;

				if (replaced) 
					free(rep);
			}

			if (FullLines) 
				strcat(lines, "]");
		}

		if (end) 
			break;

		CurrentROW++;
		CurrentCOL += pos+1;
		line		= pchData + CurrentCOL;
	}


	// Return value
	if (!error) {
		QWrite_err(FWERROR_NONE, 0);
		QWritef("%s]]", lines);
	} else
		QWrite("[]]");


	GlobalUnlock(hClipboardData);
	CloseClipboard();
	free(lines);
}
break;









case C_CLIP_SIZE:
{ // Return length of the text stored in the system clipboard

	if (!OpenClipboard(NULL)) {
		QWrite_err(FWERROR_CLIP_OPEN, 1, GetLastError());
		QWrite("0,0]");
		break;
	}

	if (!::IsClipboardFormatAvailable(CF_TEXT)) {
		QWrite_err(FWERROR_CLIP_FORMAT, 0);
		QWrite("0,0]");
		CloseClipboard();
		break;
	}

	HANDLE hClipboardData = GetClipboardData(CF_TEXT);
	char *pchData         = (char*)GlobalLock(hClipboardData);
	char *pch             = NULL;
	char *txt             = pchData;
	int size              = strlen(pchData);
	int lines             = 1;
	int	CurrentCOL        = 0;
	int pos               = 0;


	// Count how many lines
	while ((pch = strstr(txt, "\n"))) {
		pos = pch-txt;
		lines++;
		CurrentCOL += pos+1;
		txt         = pchData + CurrentCOL;
	}


	QWrite_err(FWERROR_NONE, 0);
	QWritef("%d,%d]", size, lines);

	GlobalUnlock(hClipboardData);
	CloseClipboard();
}
break;









case C_CLIP_TOFILE:
{ // Paste clipboard content to file

	global.option_error_output = OPTION_ERROR_ARRAY_CLOSE;

	char *arg_filename        = empty_string;
	char arg_mode_default[16] = "nooverwrite";
	char *arg_mode            = arg_mode_default;
	char *arg_escape          = empty_string;
	char *arg_prefix          = empty_string;
	char *arg_suffix          = empty_string;
	int arg_filename_length   = 0;
	
	for (size_t i=2; i<argument_num; i+=2) {
		switch (argument_hash[i]) {
			case NAMED_ARG_FILE :
				arg_filename        = argument[i+1];
				arg_filename_length = argument_length[i+1];
				break;

			case NAMED_ARG_MODE : 
				arg_mode = argument[i+1];
				break;

			case NAMED_ARG_PREFIX :
				arg_prefix = argument[i+1];
				break;

			case NAMED_ARG_SUFFIX : 
				arg_suffix = argument[i+1];
				break;

			case NAMED_ARG_ESCAPE : 
				arg_escape = argument[i+1];
				break;
		}
	}

	// File not specified
	if (arg_filename_length == 0) {
		QWrite_err(FWERROR_PARAM_EMPTY, 1, "arg_filename");
		break;
	}


	// Replace \t and \n
	char tmp[16]     = "";
	bool numberStart = false;
	int quantity     = -1;

	for (i=0; arg_escape[i]!='\0'; i++) {
		if (!numberStart  &&  isdigit(arg_escape[i]))
			numberStart = true;

		if (numberStart) {
			if (!isdigit(arg_escape[i])) {
				numberStart = false;
				quantity    = atoi(tmp);
				strcpy(tmp, "");
			} else
				sprintf(tmp, "%s%c", tmp, arg_escape[i]);
		}
		
		if (arg_escape[i] == 't') {
			arg_prefix = EscSequences(arg_prefix, OPTION_TAB, quantity);
			arg_suffix = EscSequences(arg_suffix, OPTION_TAB, quantity);
			quantity   = -1;
		}

		if (arg_escape[i] == 'n') {
			arg_prefix = EscSequences(arg_prefix, OPTION_CRLF, quantity);
			arg_suffix = EscSequences(arg_suffix, OPTION_CRLF, quantity);
			quantity   = -1;
		}
	}

	char open_mode[] = "wb";

	if (strcmpi(arg_mode,"append")==0)
		strcpy(open_mode, "ab");


	// Open clipboard
	if(!OpenClipboard(NULL)) {	
		QWrite_err(FWERROR_CLIP_OPEN, 1, GetLastError());
		break;
	}

	// No text in clip
	if (!::IsClipboardFormatAvailable(CF_TEXT)) {
		QWrite_err(FWERROR_CLIP_FORMAT, 0);
		CloseClipboard();
		break;
	}

	// Verify and update path to the file
	StringDynamic buf_filename;
	StringDynamic_init(buf_filename);
	char *ptr_filename = arg_filename;
	int path_type      = VerifyPath(&ptr_filename, buf_filename, OPTION_RESTRICT_TO_MISSION_DIR);

	if (path_type == PATH_ILLEGAL) {
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
		QWrite_err(FWERROR_FILE_EXISTS, 1, ptr_filename);
		CloseClipboard();
		StringDynamic_end(buf_filename);
		break;
	}

	// Mode "overwrite" means trashing existing file
	if (exists  &&  strcmpi(arg_mode,"overwrite")==0) {
		if (path_type == PATH_DOWNLOAD_DIR) {	// Delete if it's \fwatch\download\ location
			if (remove(ptr_filename) != 0) {
				QWrite_err(FWERROR_ERRNO, 2, errno, ptr_filename);
				CloseClipboard();
				StringDynamic_end(buf_filename);
				break;
			}
		} else {
			if (!trashFile(ptr_filename,strlen(ptr_filename),0)) {	// Trash for all other places
				CloseClipboard();
				StringDynamic_end(buf_filename);
				break;
			}
		}
	}


	// Write file
	if ((f = fopen(ptr_filename, open_mode))) {
		if (strcmp(arg_prefix,"")!=0)
			fprintf(f, arg_prefix);

		HANDLE clipboard_data = GetClipboardData(CF_TEXT);
		char *clipboard_text  = (char*)GlobalLock(clipboard_data);

		fwrite(clipboard_text, 1, strlen(clipboard_text), f);

		if (ferror(f)) 
			QWrite_err(FWERROR_ERRNO, 2, errno, ptr_filename);
		else {
			QWrite_err(FWERROR_NONE, 0);

			if (strcmp(arg_suffix,"")!=0)
				fprintf(f, arg_suffix);
		}

		GlobalUnlock(clipboard_data);
		fclose(f);
	} else 
		QWrite_err(FWERROR_ERRNO, 2, errno, ptr_filename);

	CloseClipboard();
	StringDynamic_end(buf_filename);
}
break;









case C_CLIP_FROMFILE:
{ // Paste file content to clipboard

	global.option_error_output = OPTION_ERROR_ARRAY_CLOSE;

	if (argument_num < 3) {
		QWrite_err(FWERROR_PARAM_FEW, 2, argument_num, 3);
		break;
	}
	
	// Verify and update path to the file
	StringDynamic buf_filename;
	StringDynamic_init(buf_filename);
	char *ptr_filename = stripq(argument[2]);

	if (!VerifyPath(&ptr_filename, buf_filename, OPTION_ALLOW_GAME_ROOT_DIR))
		break;


	FILE *f = fopen(ptr_filename, "rb");
	if (!f) {
		QWrite_err(FWERROR_ERRNO, 2, errno, ptr_filename);
		StringDynamic_end(buf_filename);
		break;
	}


	// Find file size, allocate buffer
	fseek(f, 0, SEEK_END);
	int fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	char *filetext = new char[fsize+1];
	if (!filetext) {
		QWrite_err(FWERROR_MALLOC, 2, "filetext", fsize+1);
		StringDynamic_end(buf_filename);
		fclose(f); 
		break;
	}


	// Read contents and copy
	int result = fread(filetext, 1, fsize, f);

	if (result == fsize) {
		filetext[fsize] = '\0';
		bool append     = argument_num>3 && strcmpi(argument[3],"a") == 0;

		if (CopyToClip(filetext, fsize, append))
			QWrite_err(FWERROR_NONE, 0);
	} else
		QWrite_err(FWERROR_ERRNO, 2, errno, ptr_filename);

	delete[] filetext;
	StringDynamic_end(buf_filename);
	fclose(f);
}
break;









case C_CLIP_COPYFILE:
case C_CLIP_CUTFILE:
{ // Copy file names to clipboard

	global.option_error_output = OPTION_ERROR_ARRAY_CLOSE;
	
	if (argument_num < 3) {
		QWrite_err(FWERROR_PARAM_FEW, 2, argument_num, 3);
		break;
	}

	TCHAR pwd[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, pwd);

	int pwd_len          = strlen(pwd);
	int all_files_length = argument_end + (pwd_len+strlen(global.mission_path)+2) * (argument_num-2);
	int copied_files     = 0;
	int check_mode       = OPTION_SUPPRESS_ERROR | (argument_hash[0]==C_CLIP_COPYFILE ? OPTION_ALLOW_GAME_ROOT_DIR : OPTION_RESTRICT_TO_MISSION_DIR);

	StringDynamic buf_filename;
	StringDynamic_init(buf_filename);


	// Create list of files separated by \0 with paths starting from drive
	// http://www.codeguru.com/cpp/w-p/clipboard/article.php/c2997/Copying-Files-into-Explorer.htm
	DROPFILES dobj = { 20, { 0, 0 }, 0, 1 };
	int nGblLen    = sizeof(dobj) + all_files_length*2 + 5;//lots of nulls and multibyte_char
	HGLOBAL hGbl   = GlobalAlloc(GMEM_ZEROINIT|GMEM_MOVEABLE|GMEM_DDESHARE, nGblLen);
	char *sData    = (char*)GlobalLock(hGbl);
	memcpy(sData, &dobj, 20);
	wchar_t *sWStr = (wchar_t*)(sData + 20);

	// Parse arguments passed to this command
	for (size_t i=2; i<argument_num; i++) {
		char *ptr_filename = stripq(argument[i]);

		if (VerifyPath(&ptr_filename, buf_filename, check_mode)) {
			int ptr_filename_len = buf_filename.length>0 ? buf_filename.length : strlen(ptr_filename);

			mbstowcs(sWStr          , pwd         , pwd_len);
			mbstowcs(sWStr+pwd_len  , "\\"        , 1);
			mbstowcs(sWStr+pwd_len+1, ptr_filename, ptr_filename_len);

			sWStr += pwd_len + 1 + ptr_filename_len + 1;
			buf_filename.length = 0;
			copied_files++;
		}
	}

	StringDynamic_end(buf_filename);

	// If nothing was copied
	if (copied_files == 0) {
		QWrite_err(FWERROR_PARAM_PATH_LEAVING, 1, "");
		GlobalUnlock(hGbl);
		break;
	}


	// Copy to the clipboard
	bool error = false;

	if (OpenClipboard(NULL)) {
		if (!EmptyClipboard()) {
			error = true;
			QWrite_err(FWERROR_CLIP_CLEAR, 1, GetLastError());
		}

		if (!SetClipboardData(CF_HDROP, hGbl)) {
			error = true;
			QWrite_err(FWERROR_CLIP_COPY, 1, GetLastError());
		}

		// Register 'copy' or 'cut' effect
		UINT uFormat       = RegisterClipboardFormat(CFSTR_PREFERREDDROPEFFECT);
		HGLOBAL hGbl2      = GlobalAlloc(GMEM_FIXED, sizeof(DWORD));
		DWORD *pDropEffect = (DWORD*)GlobalLock(hGbl2);
		*pDropEffect       = argument_hash[0]==C_CLIP_COPYFILE ? DROPEFFECT_COPY : DROPEFFECT_MOVE;

		SetClipboardData(uFormat, hGbl2);
		GlobalUnlock(hGbl2);
		CloseClipboard();
	} else {
		error = true;
		QWrite_err(FWERROR_CLIP_OPEN, 1, GetLastError());
	}

	GlobalUnlock(hGbl);

	if (!error)
		QWrite_err(FWERROR_NONE, 0);
}
break;









case C_CLIP_PASTEFILE:
{ // Paste files from clipboard
	//http://bcbjournal.org/forums/viewtopic.php?f=10&t=1363

	// Check optional argument
	bool list_files = argument_num>2  &&  strcmpi(argument[2],"?list")==0;


	// Verify and update path to the file
	StringDynamic buf_destination;
	StringDynamic_init(buf_destination);
	char *ptr_destination = argument_num>2 && !list_files ? stripq(argument[2]) : NULL;
	int destination_type  = 0;
	int destination_len   = 0;

	if (!list_files) {
		destination_type = VerifyPath(&ptr_destination, buf_destination, OPTION_RESTRICT_TO_MISSION_DIR);

		if (destination_type != PATH_ILLEGAL) {
			if (buf_destination.length == 0)
				StringDynamic_append(buf_destination, ptr_destination);

			if (buf_destination.text[buf_destination.length-1] != '\\')
				StringDynamic_append(buf_destination, "\\");

			destination_len = buf_destination.length;
		} else {
			QWrite("\"\",[]]");
			StringDynamic_end(buf_destination);
			break;
		}
	}


	// Open clipboard
	if (!OpenClipboard(NULL)) {
		QWrite_err(FWERROR_CLIP_OPEN, 1, GetLastError());
		QWrite("\"\",[]]");
		StringDynamic_end(buf_destination);
		break;
	}


	// Check format
	if (!IsClipboardFormatAvailable(CF_HDROP)) {
		QWrite_err(FWERROR_CLIP_FORMAT, 0);
		QWrite("\"\",[]]");
		CloseClipboard();
		StringDynamic_end(buf_destination);
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

	DWORD effect[] = {
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
		QWrite_err(FWERROR_CLIP_EFFECT, 0);
		QWritef("\"%s\",[]]", effect_name[effect_id]);
		CloseClipboard();
		StringDynamic_end(buf_destination);
		break;
	}


	// Get data
	HANDLE Data = GetClipboardData(CF_HDROP);

	if (!Data) {
		QWrite_err(FWERROR_CLIP_EMPTY, 0);
		QWrite("\"\",[]]");
		CloseClipboard();
		StringDynamic_end(buf_destination);
		break;
	}

	DROPFILES *df = (DROPFILES*) GlobalLock(Data);

	if (!df->fWide)	{
		QWrite_err(FWERROR_CLIP_LOCK, 1, GetLastError());
		QWrite("\"\",[]]");
		GlobalUnlock(Data);
		CloseClipboard();
		StringDynamic_end(buf_destination);
		break;
	}

	LPWSTR pFilenames = (LPWSTR) (((LPBYTE)df) + df->pFiles);


	// Create output array
	QWrite_err(FWERROR_NONE, 0);
	QWritef("\"%s\",[", effect_name[effect_id]);


	// Get current working dir
	TCHAR pwd[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, pwd);


	// Parse filenames
	StringDynamic buf_source;
	StringDynamic_init(buf_source);
	int pwd_len      = strlen(pwd);
	int mission_len  = strlen(global.mission_path);
	int name_start   = 0;

	//global.error_within_error = true;
	global.option_error_output = OPTION_ERROR_ARRAY_CLOSE;

	for (i=0; pFilenames[name_start]!='\0'; i++) {
		if (pFilenames[i] == '\0') {
			// Convert wide string to multi-byte string
			wchar_t *wide_path      = pFilenames + name_start;
			size_t wide_path_length = wcslen(wide_path) + 1;
			
			if (StringDynamic_allocate(buf_source, wide_path_length) == 0) 
				wcstombs(buf_source.text, wide_path, wide_path_length);
			else {
				QWrite("]]]");
				global.option_error_output = OPTION_ERROR_ARRAY_CLOSE;
				QWrite_err(FWERROR_MALLOC, 2, "buf_source", wide_path_length);
				StringDynamic_end(buf_source);
				break;
			}


			// Convert path (starting from drive) to OFP format for logging and for verification
			char *absolute_path = buf_source.text;
			char *relative_path = absolute_path;
			bool is_game_dir    = false;

			if (strncmpi(absolute_path, pwd, pwd_len) == 0) {
				absolute_path += pwd_len + 1;
				relative_path  = absolute_path;
				is_game_dir    = true;

				if (strncmpi(absolute_path, global.mission_path, mission_len) == 0)
					relative_path += mission_len;
				else {
					memcpy(absolute_path-3, "..\\", 3);
					relative_path = absolute_path - 3;
				}
			}


			// Log
			QWritef("]+[[\"%s\",", relative_path);


			// Verify path
			int source_type = VerifyPath(&relative_path, buf_source, OPTION_SUPPRESS_ERROR | OPTION_SUPPRESS_CONVERSION);
			bool allow_copy = true;


			// Not allowed to move files to the download directory
			if (is_game_dir  &&  dwEffect & DROPEFFECT_MOVE  &&  source_type!=PATH_DOWNLOAD_DIR  &&  destination_type==PATH_DOWNLOAD_DIR) {
				QWrite_err(FWERROR_FILE_MOVETOTMP, 2, relative_path, buf_destination);
				allow_copy = false;
			}


			// Copy filename from the source path to the destination path
			char *last_slash = strrchr(relative_path, '\\');
			StringDynamic_append(buf_destination, last_slash!=NULL ? last_slash+1 : relative_path);


			// Perform file operation
			if (allow_copy) {
				if (!list_files) {
					if (dwEffect & DROPEFFECT_MOVE) {
						if (MoveFileEx(absolute_path,buf_destination.text,0) == 0) {
							QWrite_err(FWERROR_NONE, 0);
							EmptyClipboard();
						} else 
							QWrite_err(FWERROR_ERRNO, 2, errno, absolute_path);
					}

					if (dwEffect & DROPEFFECT_COPY) {
						if (CopyFile((LPCTSTR)absolute_path, (LPCTSTR)buf_destination.text, true)) 
							QWrite_err(FWERROR_NONE, 0);
						else 
							QWrite_err(FWERROR_WINAPI, 1, GetLastError());
					}
				} else
					QWrite_err(FWERROR_NONE, 0);
			}


			// Reset
			buf_source.length      = 0;
			buf_destination.length = destination_len;
			name_start             = i + 1;
			QWrite("]");
		}
	}

	//global.error_within_error = false;
	QWrite("]]");

	StringDynamic_end(buf_source);
	StringDynamic_end(buf_destination);

	GlobalUnlock(Data);
	CloseClipboard();
}
break;

