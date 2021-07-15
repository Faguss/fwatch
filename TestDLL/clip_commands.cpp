// -----------------------------------------------------------------
// CLIPBOARD OPERATIONS
// -----------------------------------------------------------------

case C_CLIP_COPY:
{ // Copy text to clipboard

	bool arg_append  = false;
	char *arg_escape = empty_char;
	size_t arg_text  = empty_char_index;

	for (size_t i=2; i<argument_num; i+=2) {
		switch (argument_hash[i]) {
			case NAMED_ARG_APPEND :
				arg_append = String_bool(argument[i+1]);
				break;

			case NAMED_ARG_ESCAPE : 
				arg_escape = argument[i+1].text;
				break;

			case NAMED_ARG_TEXT : 
				arg_text = i + 1;
				break;
		}
	}


	// Process escape sequences
	char number_string[16] = "";
	bool number_start      = false;
	int quantity	       = -1;

	for (i=0; arg_escape[i]!='\0'; i++)	{
		if (!number_start  &&  isdigit(arg_escape[i]))
			number_start = true;

		if (number_start) {
			if (!isdigit(arg_escape[i])) {
				number_start = false;
				quantity     = atoi(number_string);
				strcpy(number_string, "");
			} else
				strncat(number_string, arg_escape+i, 1);
		}
		
		if (arg_escape[i] == 't') {
			String_escape_sequences(argument[arg_text], OPTION_TAB, quantity);
			quantity = -1;
		}

		if (arg_escape[i] == 'n') {
			String_escape_sequences(argument[arg_text], OPTION_LF, quantity);
			quantity = -1;
		}
	}

	if (CopyToClip(argument[arg_text], arg_append)) {
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
		pos = atoi(argument[2].text);

	if (argument_num > 3) 
		length = atoi(argument[3].text);


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
				Start = atoi(argument[i+1].text);
				break;

			case NAMED_ARG_END : 
				End = atoi(argument[i+1].text);
				break;

			case NAMED_ARG_CUT : 
				Cut   = true;
				Limit = atoi(argument[i+1].text);
				if (Limit < 0) Limit=0;
				break;

			case NAMED_ARG_SPLIT : 
				FullLines = String_bool(argument[i+1]);
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

	size_t arg_file           = empty_char_index;
	size_t arg_prefix         = empty_char_index;
	size_t arg_suffix         = empty_char_index;
	char arg_mode_default[16] = "nooverwrite";
	char *arg_mode            = arg_mode_default;
	char *arg_escape          = empty_char;
	
	for (size_t i=2; i<argument_num; i+=2) {
		switch (argument_hash[i]) {
			case NAMED_ARG_FILE :
				arg_file = i + 1;
				break;

			case NAMED_ARG_MODE : 
				arg_mode = argument[i+1].text;
				break;

			case NAMED_ARG_PREFIX :
				arg_prefix = i + 1;
				break;

			case NAMED_ARG_SUFFIX : 
				arg_suffix = i + 1;
				break;

			case NAMED_ARG_ESCAPE : 
				arg_escape = argument[i+1].text;
				break;
		}
	}

	// File not specified
	if (argument[arg_file].length == 0) {
		QWrite_err(FWERROR_PARAM_EMPTY, 1, "arg_file");
		break;
	}


	// Replace \t and \n
	char number_string[16] = "";
	bool number_start      = false;
	int quantity           = -1;

	for (i=0; arg_escape[i]!='\0'; i++) {
		if (!number_start  &&  isdigit(arg_escape[i]))
			number_start = true;

		if (number_start) {
			if (!isdigit(arg_escape[i])) {
				number_start = false;
				quantity     = atoi(number_string);
				strcpy(number_string, "");
			} else
				strncat(number_string, arg_escape+i, 1);
		}
		
		if (arg_escape[i] == 't') {
			String_escape_sequences(argument[arg_prefix], OPTION_TAB, quantity);
			String_escape_sequences(argument[arg_suffix], OPTION_TAB, quantity);
			quantity   = -1;
		}

		if (arg_escape[i] == 'n') {
			String_escape_sequences(argument[arg_prefix], OPTION_CRLF, quantity);
			String_escape_sequences(argument[arg_suffix], OPTION_CRLF, quantity);
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

	int path_type = VerifyPath(argument[arg_file], buf_filename, OPTION_RESTRICT_TO_MISSION_DIR);
	if (path_type == PATH_ILLEGAL) {
		CloseClipboard();
		break;
	}

	// Check if file already exists
	FILE *f     = fopen(argument[arg_file].text, "r");
	bool exists = false;

	if (f) {
		exists = true;
		fclose(f);
	}

	// Overwriting not allowed
	if (exists  &&  strcmpi(arg_mode,"nooverwrite")==0) {
		QWrite_err(FWERROR_FILE_EXISTS, 1, argument[arg_file].text);
		CloseClipboard();
		StringDynamic_end(buf_filename);
		break;
	}

	// Mode "overwrite" means trashing existing file
	if (exists  &&  strcmpi(arg_mode,"overwrite")==0) {
		if (path_type == PATH_DOWNLOAD_DIR) {	// Delete if it's \fwatch\download\ location
			if (remove(argument[arg_file].text) != 0) {
				QWrite_err(FWERROR_ERRNO, 2, errno, argument[arg_file].text);
				CloseClipboard();
				StringDynamic_end(buf_filename);
				break;
			}
		} else {
			if (!trashFile(argument[arg_file], OPTION_NONE)) {	// Trash for all other places
				CloseClipboard();
				StringDynamic_end(buf_filename);
				break;
			}
		}
	}


	// Write file
	if ((f = fopen(argument[arg_file].text, open_mode))) {
		if (argument[arg_prefix].length > 0)
			fprintf(f, argument[arg_prefix].text);

		HANDLE clipboard_data = GetClipboardData(CF_TEXT);
		char *clipboard_text  = (char*)GlobalLock(clipboard_data);

		fwrite(clipboard_text, 1, strlen(clipboard_text), f);

		if (ferror(f)) 
			QWrite_err(FWERROR_ERRNO, 2, errno, argument[arg_file].text);
		else {
			QWrite_err(FWERROR_NONE, 0);

			if (argument[arg_suffix].length > 0)
				fprintf(f, argument[arg_suffix].text);
		}

		GlobalUnlock(clipboard_data);
		fclose(f);
	} else 
		QWrite_err(FWERROR_ERRNO, 2, errno, argument[arg_file].text);

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
	String_trim_quotes(argument[2]);

	StringDynamic buf_filename;
	StringDynamic_init(buf_filename);

	if (!VerifyPath(argument[2], buf_filename, OPTION_ALLOW_GAME_ROOT_DIR))
		break;


	FILE *f = fopen(argument[2].text, "rb");
	if (!f) {
		QWrite_err(FWERROR_ERRNO, 2, errno, argument[2].text);
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
		bool append     = argument_num>3 && strcmpi(argument[3].text,"a") == 0;

		String text_to_copy = {filetext, fsize};
		if (CopyToClip(text_to_copy, append))
			QWrite_err(FWERROR_NONE, 0);
	} else
		QWrite_err(FWERROR_ERRNO, 2, errno, argument[2].text);

	delete[] filetext;
	StringDynamic_end(buf_filename);
	fclose(f);
}
break;









case C_CLIP_COPYFILE:
case C_CLIP_CUTFILE:
{ // Copy file names to the Windows clipboard

	global.option_error_output = OPTION_ERROR_ARRAY_CLOSE;
	
	if (argument_num < 3) {
		QWrite_err(FWERROR_PARAM_FEW, 2, argument_num, 3);
		break;
	}


	// Calculate length of all the paths
	size_t all_files_length = 0;

	for (size_t i=2; i<argument_num; i++)
		all_files_length += global.game_dir_length + global.mission_path_length + argument[i].length + 2;


	// Create list of files separated by \0 with paths starting from drive
	// http://www.codeguru.com/cpp/w-p/clipboard/article.php/c2997/Copying-Files-into-Explorer.htm
	DROPFILES dobj = { 20, { 0, 0 }, 0, 1 };
	SIZE_T nGblLen = sizeof(dobj) + all_files_length*2 + 5;//lots of nulls and multibyte_char
	HGLOBAL hGbl   = GlobalAlloc(GMEM_ZEROINIT|GMEM_MOVEABLE|GMEM_DDESHARE, nGblLen);
	char *sData    = (char*)GlobalLock(hGbl);
	memcpy(sData, &dobj, 20);
	wchar_t *sWStr = (wchar_t*)(sData + 20);
	wchar_t *backup = sWStr;

	StringDynamic buf_filename;
	StringDynamic_init(buf_filename);
	size_t copied_files = 0;

	// Copy all the paths into a single buffer
	for (i=2; i<argument_num; i++) {
		buf_filename.length = 0;
		String_trim_quotes(argument[i]);

		if (VerifyPath(argument[i], buf_filename, OPTION_SUPPRESS_ERROR | (argument_hash[0]==C_CLIP_COPYFILE ? OPTION_ALLOW_GAME_ROOT_DIR : OPTION_RESTRICT_TO_MISSION_DIR))) {
			mbstowcs(sWStr, global.game_dir , global.game_dir_length);
			sWStr += global.game_dir_length;

			mbstowcs(sWStr, "\\", 1);
			sWStr += 1;

			mbstowcs(sWStr, argument[i].text, argument[i].length);
			sWStr += argument[i].length + 1;

			copied_files++;
		}
	}

	GlobalUnlock(hGbl);
	StringDynamic_end(buf_filename);


	// If nothing was copied
	if (copied_files == 0) {
		QWrite_err(FWERROR_PARAM_PATH_LEAVING, 1, "");
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

		// Register "copy" or "cut" effect
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

	if (!error)
		QWrite_err(FWERROR_NONE, 0);
}
break;









case C_CLIP_PASTEFILE:
{ // Copy files from locations stored in the Windows clipboard
  // http://bcbjournal.org/forums/viewtopic.php?f=10&t=1363

	size_t arg_destination = empty_char_index;
	bool arg_list_files    = false;

	if (argument_num > 2) {
		arg_destination = 2;
		String_trim_quotes(argument[arg_destination]);
		arg_list_files = strcmpi(argument[arg_destination].text,"?list") == 0;
	}


	// Verify and update destination path
	StringDynamic buf_destination;
	StringDynamic_init(buf_destination);
	int destination_type = 0;
	int options          = OPTION_RESTRICT_TO_MISSION_DIR;

	if (argument[arg_destination].length==0 || (argument[arg_destination].length>0 && argument[arg_destination].text[argument[arg_destination].length-1]!='\\' && argument[arg_destination].text[argument[arg_destination].length-1]!='/'))
		options |= OPTION_ASSUME_TRAILING_SLASH;

	if (!arg_list_files) {
		destination_type = VerifyPath(argument[arg_destination], buf_destination, options);

		// If path wasn't copied to the dynamic buffer then do it anyway because we need it later
		if (destination_type != PATH_ILLEGAL) {
			if (buf_destination.length == 0)
				StringDynamic_appends(buf_destination, argument[arg_destination]);

			if (options & OPTION_ASSUME_TRAILING_SLASH)
				StringDynamic_append(buf_destination, "\\");

			argument[arg_destination].text   = buf_destination.text;
			argument[arg_destination].length = buf_destination.length;
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


	// Find what's the registered effect
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

	const char effect_name[][16] = {
		"none",
		"copy",
		"move",
		"link",
		"scroll"
	};

	size_t effect_id = 0;
	for (size_t i=0; i<sizeof(effect)/sizeof(effect[0]); i++)
		if (dwEffect & effect[i]) {
			effect_id = i;
			break;
		}


	// Prohibit actions other than move or copy
	if (!arg_list_files  &&  ~dwEffect & DROPEFFECT_COPY  &&  ~dwEffect & DROPEFFECT_MOVE) {
		QWrite_err(FWERROR_CLIP_EFFECT, 0);
		QWritef("\"%s\",[]]", effect_name[effect_id]);
		CloseClipboard();
		StringDynamic_end(buf_destination);
		break;
	}


	// Get data
	HANDLE clipboard_handle = GetClipboardData(CF_HDROP);

	if (!clipboard_handle) {
		QWrite_err(FWERROR_CLIP_EMPTY, 0);
		QWrite("\"\",[]]");
		CloseClipboard();
		StringDynamic_end(buf_destination);
		break;
	}

	DROPFILES *df = (DROPFILES*) GlobalLock(clipboard_handle);

	if (!df->fWide)	{
		QWrite_err(FWERROR_CLIP_LOCK, 1, GetLastError());
		QWrite("\"\",[]]");
		GlobalUnlock(clipboard_handle);
		CloseClipboard();
		StringDynamic_end(buf_destination);
		break;
	}

	LPWSTR pFilenames = (LPWSTR) (((LPBYTE)df) + df->pFiles);


	// Create output array
	QWrite_err(FWERROR_NONE, 0);
	QWritef("\"%s\",[", effect_name[effect_id]);


	// Parse filepaths from the clipboard
	StringDynamic buf_source;
	StringDynamic_init(buf_source);
	size_t name_start          = 0;
	global.option_error_output = OPTION_ERROR_ARRAY_NESTED | OPTION_ERROR_ARRAY_CLOSE;

	for (i=0; pFilenames[name_start]!='\0'; i++) {
		if (pFilenames[i] == '\0') {
			// Convert wide string to multi-byte string
			wchar_t *wide_path      = pFilenames + name_start;
			size_t wide_path_length = wcslen(wide_path) + 1;
			
			if (StringDynamic_allocate(buf_source, wide_path_length*2) == 0)
				buf_source.length = wcstombs(buf_source.text, wide_path, wide_path_length);
			else {
				QWrite("]];");
				global.option_error_output = OPTION_NONE;
				QWrite_err(FWERROR_MALLOC, 2, "buf_source", wide_path_length);
				QWrite("\"\", [");
				StringDynamic_end(buf_source);
				break;
			}


			// Convert absolute path (starting from drive) to the OFP relative path - for logging and verification
			char *absolute_path  = buf_source.text;
			String relative_path = {buf_source.text, buf_source.length};
			bool is_game_dir     = false;

			// if game dir
			if (strncmpi(absolute_path, global.game_dir, global.game_dir_length) == 0) {
				absolute_path        += global.game_dir_length + 1;
				relative_path.text    = absolute_path;
				relative_path.length -= global.game_dir_length + 1;
				is_game_dir           = true;

				// if mission dir
				if (strncmpi(absolute_path, global.mission_path, global.mission_path_length) == 0) {
					relative_path.text   += global.mission_path_length;
					relative_path.length -= global.mission_path_length;
				} else {
					// if game root dir
					memcpy(absolute_path-3, "..\\", 3);
					relative_path.text    = absolute_path - 3;
					relative_path.length += 3;
				}
			}

			QWritef("]+[[\"%s\",", relative_path);

			int source_type = VerifyPath(relative_path, buf_source, OPTION_SUPPRESS_ERROR | OPTION_SUPPRESS_CONVERSION);
			bool path_valid = true;


			// Not allowed to move files to the fwatch\tmp directory
			if (is_game_dir  &&  dwEffect & DROPEFFECT_MOVE  &&  source_type!=PATH_DOWNLOAD_DIR  &&  destination_type==PATH_DOWNLOAD_DIR) {
				QWrite_err(FWERROR_FILE_MOVETOTMP, 2, relative_path.text, buf_destination);
				path_valid = false;
			}


			// Copy filename from the source path to the destination path
			char *last_slash = strrchr(relative_path.text, '\\');
			StringDynamic_append(buf_destination, last_slash!=NULL ? last_slash+1 : relative_path.text);


			// Execute move/copy
			if (path_valid) {
				if (!arg_list_files) {
					if (dwEffect & DROPEFFECT_MOVE) {
						if (MoveFileEx(absolute_path,buf_destination.text,0)) {
							QWrite_err(FWERROR_NONE, 0);
							EmptyClipboard();
						} else 
							QWrite_err(FWERROR_WINAPI, 1, GetLastError());
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


			// Reset before the next file
			buf_source.length      = 0;
			buf_destination.length = argument[arg_destination].length;
			name_start             = i + 1;
			QWrite("]");
		}
	}

	QWrite("]]");

	StringDynamic_end(buf_source);
	StringDynamic_end(buf_destination);

	GlobalUnlock(clipboard_handle);
	CloseClipboard();
}
break;

