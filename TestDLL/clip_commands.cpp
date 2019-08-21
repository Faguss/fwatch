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
};
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
};
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
};
break;









case C_CLIPBOARD_TOFILE:
{ // Paste clipboard content to file

	// Arguments
	if (numP < 3) 
	{
		FWerror(100,0,CommandID,"","",numP,3,out);
		break;
	};

	bool noOverwrite	= true;
	bool append			= false;
	bool overwrite		= false;
	char mode[3]		= "wb";

	if (numP > 3)
	{
		if (strcmpi(par[3],"a") == 0) 
			noOverwrite = false, 
			append = true, 
			strcpy(mode, "ab");

		if (strcmpi(par[3],"w") == 0) 
			noOverwrite = false, 
			overwrite = true;
	};


	// Open clipboard
	if(!OpenClipboard(NULL)) 
	{	
		FWerror(20,GetLastError(),CommandID,"","",0,0,out);
		break;
	};
	

	// No text in clip
	if (!::IsClipboardFormatAvailable(CF_TEXT))
	{
		FWerror(21,0,CommandID,"","",0,0,out);
		CloseClipboard();
		break;
	};


	// Allocate buffer
	int File_name_len	= strlen(com);
	char *File_name		= (char*) malloc (File_name_len);
	char *WantedFile	= File_name;

	if (File_name == NULL) 
	{
		FWerror(10,0,CommandID,"File_name","",File_name_len,0,out);
		CloseClipboard();
		break;
	};

	par[2] = stripq(par[2]);
	strcpy(File_name, par[2]);

	
	// Validate path
	if (isLeavingDir(File_name,0,RESTRICT_TO_MISSION_DIR,CommandID,out)) 
	{
		CloseClipboard(); 
		free(File_name);
		break;
	};


	// Mission dir path
	if (isAllowedExternalPath(File_name,RESTRICT_TO_MISSION_DIR)) 
		WantedFile = File_name + 3;
	else
		if (getMissionDir(&File_name, File_name_len, CommandID, out)) 
			WantedFile = File_name;
		else 
		{
			free(File_name); 
			CloseClipboard(); 
			break;
		}


	// Check if file already exists
	FILE *f		= fopen(WantedFile, "r");
	bool exists = false;

	if (f) 
		exists = true, 
		fclose(f);


	// Overwriting not allowed
	if (exists  &&  noOverwrite)
	{
		FWerror(207,0,CommandID,WantedFile,"",0,0,out);
		CloseClipboard();
		free(File_name);
		break;
	}


	// Mode "overwrite" means trashing existing file
	if (exists  &&  overwrite)
	{
		// Delete if it's \fwatch\download\ location
		bool downloadDir = isLeavingDir(File_name,1,RESTRICT_TO_MISSION_DIR,CommandID,out);

		if (downloadDir) 
		{
			if (remove(WantedFile) != 0)
			{
				FWerror(7,errno,CommandID,WantedFile,"",0,0,out);
				CloseClipboard();
				free(File_name);
				break;
			};
		}
		// Trash for all other places
		else
		{
			if (!trashFile(WantedFile,CommandID,out,0)) 
			{
				CloseClipboard();
				free(File_name);
				break;
			};
		};
	};


	// Write file
	if (f = fopen(WantedFile, mode))
	{
		// Get existing text
		HANDLE hClipboardData	= GetClipboardData(CF_TEXT);
		char *pchData			= (char*)GlobalLock(hClipboardData);

		fwrite(pchData, 1, strlen(pchData), f);

		if (ferror(f)) 
			FWerror(7,errno,CommandID,WantedFile,"",0,0,out);
		else
			FWerror(0,0,CommandID,"","",0,0,out);

		GlobalUnlock(hClipboardData);
		fclose(f);
	}
	else 
		FWerror(7,errno,CommandID,WantedFile,"",0,0,out);


	CloseClipboard();
	free(File_name);
};
break;









case C_CLIPBOARD_FROMFILE:
{ // Paste file content to clipboard

	// Arguments
	if (numP < 3) 
	{
		FWerror(100,0,CommandID,"","",numP,3,out);
		break;
	};

	bool append = 0;

	if (numP > 3)
		if (strcmpi(par[3],"a") == 0) 
			append = true;


	// Allocate buffer
	int File_name_len	= strlen(com);
	char *File_name		= (char*) malloc (File_name_len);
	char *WantedFile	= File_name;

	if (File_name == NULL) 
	{
		FWerror(10,0,CommandID,"File_name","",File_name_len,0,out);
		CloseClipboard();
		break;
	};

	par[2] = stripq(par[2]);
	strcpy(File_name, par[2]);


	// Create string path
	if (isAllowedExternalPath(File_name,ALLOW_GAME_ROOT_DIR)) 
		WantedFile = File_name + 3;
	else
		if (getMissionDir(&File_name, File_name_len, CommandID, out)) 
			WantedFile = File_name;
		else 
			{free(File_name); break;}


	// Open file
	FILE *f = fopen(WantedFile, "rb");

	if (!f)
	{
		FWerror(7,errno,CommandID,WantedFile,"",0,0,out);
		free(File_name);
		break;
	};


	// Find file size, allocate buffer
	fseek(f, 0, SEEK_END);
	unsigned int fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	char *filetext = new char[fsize+1];

	if (!filetext) 
	{
		FWerror(10,0,CommandID,"filetext","",fsize+1,0,out);
		free(File_name);
		fclose(f); 
		break;
	};


	// Read contents and copy
	unsigned int result = fread(filetext,1,fsize,f);

	if (result == fsize) 
	{
		filetext[fsize] = '\0';

		if (CopyToClip(filetext, append, CommandID, out))
			FWerror(0,0,CommandID,"","",0,0,out);
	}
	else
		FWerror(7,errno,CommandID,WantedFile,"",0,0,out); 
	

	delete[] filetext;
	free(File_name);
	fclose(f);
};
break;









case C_CLIPBOARD_COPYFILE:
case C_CLIPBOARD_CUTFILE:
{ // Copy file names to clipboard

	// Not enough arguments
	if (numP < 3)
	{
		FWerror(100,0,CommandID,"","",numP,3,out);
		break;
	};


	/* 
	Clipboard needs a list of files:

		- with full paths (starting from drive)
		- separated by \0
		- that ends with \0\0

	so we need to prepare that first
	*/


	// Allocate buffers for a new list of files and for a temp buffer
	int FilesLen	 = 5;
	int	CurrFileLen	 = strlen(com);
	char *Files		 = (char*) malloc (CurrFileLen);
	char *FilesNEW	 = "";
	char *CurrFile	 = (char*) malloc (CurrFileLen);
	char *WantedFile = CurrFile;

	if (Files==NULL  ||  CurrFile==NULL) 
	{
		char failedBuf[10]	= "";
		int failedBufL		= 0;

		if (Files == NULL) 
			strcat(failedBuf, "Files "), 
			failedBufL += CurrFileLen;
		else
			free(Files);

		if (CurrFile == NULL) 
			strcat(failedBuf, "CurrFile "), 
			failedBufL += CurrFileLen;
		else
			free(CurrFile);

		FWerror(10,0,CommandID,failedBuf,"",failedBufL,0,out);
		break;
	};

	strcpy(Files, "");
	strcpy(CurrFile, "");


	// Get current working dir
	TCHAR pwd[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, pwd);


	// Parse arguments passed to this command
	char *argLine = com+14;

	if (strcmpi(par[1],"cutfile") == 0) 
		argLine = com+13;

	bool inQuote		= false;
	bool resetStart		= true;
	unsigned int start	= 0;
	unsigned int end	= 0;
	unsigned int error	= 0;
	unsigned int pos	= 0;
	unsigned int l		= strlen(argLine);
	unsigned int amount	= 0;

	for (unsigned i=0; i<=l; i++)
	{
		// find beggining of the arg
		if (!isspace(argLine[i])  &&  resetStart) 
			resetStart = false,
			start = i;
		

		// skip characters inside quotation marks
		if (argLine[i] == '\"') 
			inQuote = !inQuote;

		if (inQuote) 
			continue;
		

		// found end of the arg
		if (isspace(argLine[i])  ||  i==l) 
		{
			end = i;

			if (argLine[start] == '\"') 
				start++;

			if (argLine[end-1] == '\"') 
				end--;

			strncpy(CurrFile, argLine+start, end-start);
			CurrFile[end-start] = '\0';
			WantedFile = CurrFile;

			bool copyArg = true;

			// Verify path - can't cut files from outside
			bool allowOutside = CommandID==C_CLIPBOARD_COPYFILE  ||  isLeavingDir(CurrFile,1,RESTRICT_TO_MISSION_DIR,CommandID,out);

			if (!allowOutside)
				if (isLeavingDir(CurrFile,0,RESTRICT_TO_MISSION_DIR,CommandID,out)) 
					copyArg = false;


			// Get path to mission dir
			if (isAllowedExternalPath(CurrFile,allowOutside)) 
				WantedFile = CurrFile+3;
			else
				if (getMissionDir(&CurrFile, CurrFileLen, CommandID, out)) 
					WantedFile = CurrFile;
				else 
					copyArg = false;


			// Passsed restrictions
			if (copyArg)
			{
				// Extend size of the buffer so we can copy current file there
				FilesLen += strlen(WantedFile) + strlen(pwd) + 2;
				FilesNEW = (char*) realloc (Files, FilesLen);

				if (FilesNEW != NULL) 
					Files = FilesNEW; 
				else 
				{	
					FWerror(11,0,CommandID,"Files","",FilesLen,0,out);
					error = 1; 
					break;
				};


				// Copy full path to the buffer
				memcpy(Files+pos, pwd, strlen(pwd));
				pos += strlen(pwd);

				memcpy(Files+pos, "\\", 1);
				pos += 1;

				memcpy(Files+pos, WantedFile, strlen(WantedFile));
				pos += strlen(WantedFile);

				memcpy(Files+pos, "\0", 1);
				pos += 1;

				amount++;
			};

			resetStart = true;
			strcpy(CurrFile, "");
		};
	};

	memcpy(Files+pos, "\0", 1);


	// If nothing was copied
	if (amount==0  &&  !error)
		FWerror(105,0,CommandID,"","",CurrFileLen,0,out),
		error = 1;


	// In case of error - quit
	if (error) 
	{
		free(Files);
		free(CurrFile);
		break;
	};


	// Prepare header to copy to the clipboard
	// http://www.codeguru.com/cpp/w-p/clipboard/article.php/c2997/Copying-Files-into-Explorer.htm
	DROPFILES dobj	= { 20, { 0, 0 }, 0, 1 };
	int nGblLen		= sizeof(dobj) + FilesLen*2 + 5;//lots of nulls and multibyte_char
	HGLOBAL hGbl	= GlobalAlloc(GMEM_ZEROINIT|GMEM_MOVEABLE|GMEM_DDESHARE, nGblLen);
	char* sData		= (char*)GlobalLock(hGbl);		
	memcpy( sData, &dobj, 20 );
	char* sWStr		= sData+20;
		
	for (int j=0;  j<FilesLen*2;  j+=2)
		sWStr[j] = Files[j/2];


	// Finally copy
	if ( OpenClipboard(NULL) )
	{
		if(!EmptyClipboard())
			error = 1,
			FWerror(22,GetLastError(),CommandID,"","",CurrFileLen,0,out);

		if (!SetClipboardData( CF_HDROP, hGbl ))
			error = 1,
			FWerror(23,GetLastError(),CommandID,"","",CurrFileLen,0,out);


		// Register 'copy' or 'cut' effect
		UINT uFormat		= RegisterClipboardFormat(CFSTR_PREFERREDDROPEFFECT);
		HGLOBAL hGbl2		= GlobalAlloc(GMEM_FIXED, sizeof(DWORD));
		DWORD* pDropEffect	= (DWORD*)GlobalLock(hGbl2);

		if (CommandID == C_CLIPBOARD_COPYFILE)
			*pDropEffect = DROPEFFECT_COPY;
		else
			*pDropEffect = DROPEFFECT_MOVE;

		SetClipboardData(uFormat, hGbl2);
		GlobalUnlock(hGbl2);
		CloseClipboard();
	}
	else
		FWerror(20,GetLastError(),CommandID,"","",CurrFileLen,0,out);

	GlobalUnlock(hGbl);
	free(Files);
	free(CurrFile);

	if (!error)
		FWerror(0,0,CommandID,"","",0,0,out);
};
break;









case C_CLIPBOARD_PASTEFILE:
{ // Paste files from clipboard
	//http://bcbjournal.org/forums/viewtopic.php?f=10&t=1363

	// Allocate buffers
	int valLen			= 128 + strlen(com);
	int missionDirLen	= 128;
	char *val			= (char*) malloc (valLen);
	char *destination	= val;
	char *missionDir	= (char*) malloc (missionDirLen);

	if (val==NULL  ||  missionDir==NULL)
	{
		char failedBuf[21] = "";
		int failedBufL = 0;

		if (val == NULL) 
			strcat(failedBuf,"val "), 
			failedBufL += valLen;
		else
			free(val);

		if (missionDir == NULL) 
			strcat(failedBuf,"missionDir "), 
			failedBufL += missionDirLen;
		else
			free(missionDir);

		FWerror(10,0,CommandID,failedBuf,"",failedBufL,0,out);
		QWrite("\"\",[]]", out);
		free(missionDir);
		free(val); 
		break;

	};

	strcpy(val, "");
	strcpy(missionDir, "");


	// Check for optional argument
	if (numP > 2) 
		strcpy(val, com+15);

	bool ListFiles = strcmpi(val, "?list") == 0;


	// Get current working dir
	TCHAR pwd[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, pwd);


	// Get current mission dir
	getMissionDir(&missionDir, missionDirLen, CommandID, out);


	// Verify destination path
	bool IsDestinationDownload = 0;

	if (!ListFiles)
	{
		if (isLeavingDir(val,0,RESTRICT_TO_MISSION_DIR,CommandID,out)) 
		{
			QWrite("\"\",[]]", out); 
			free(val); 
			free(missionDir); 
			break;
		};

		IsDestinationDownload = isLeavingDir(val,1,RESTRICT_TO_MISSION_DIR,CommandID,out);

		if (isAllowedExternalPath(val,RESTRICT_TO_MISSION_DIR)) 
			destination = val + 3;
		else
			if (getMissionDir(&val, valLen, CommandID, out)) 
				destination = val;
			else 
			{
				QWrite("\"\",[]]", out); 
				free(missionDir); 
				free(val); 
				break;
			}
	};

	int destLen = strlen(destination);


	// Open clipboard
	if (!OpenClipboard(NULL))
	{
		FWerror(20,GetLastError(),CommandID,"","",0,0,out);
		QWrite("\"\",[]]", out);
		free(missionDir);
		free(val);
		break;
	};


	// Check format
	if (!IsClipboardFormatAvailable(CF_HDROP))
	{
		FWerror(21,0,CommandID,"","",0,0,out);
		QWrite("\"\",[]]", out);
		CloseClipboard();
		free(missionDir);
		free(val); 
		break;
	};


	// Get data
	HANDLE Data = GetClipboardData(CF_HDROP);

	if (Data == NULL)
	{
		FWerror(24,0,CommandID,"","",0,0,out);
		QWrite("\"\",[]]", out);
		CloseClipboard();
		free(missionDir);
		free(val); 
		break;
	}

	DROPFILES *df = (DROPFILES*) GlobalLock(Data);

	if (!df->fWide)
	{
		FWerror(24,0,CommandID,"","",0,0,out);
		QWrite("\"\",[]]", out);
		GlobalUnlock(Data);
		CloseClipboard();
		free(missionDir);
		free(val); 
		break;
	}

	LPWSTR pFilenames = (LPWSTR) (((LPBYTE)df) + df->pFiles);

	DWORD dwEffect = 0;
	DWORD dwFormat = RegisterClipboardFormat(CFSTR_PREFERREDDROPEFFECT);
	HANDLE hGlobal = GetClipboardData(dwFormat);

	if (hGlobal)
	{
		LPDWORD pdwEffect	= (LPDWORD) GlobalLock(hGlobal);
		dwEffect			= *pdwEffect;
		GlobalUnlock(hGlobal);
	}


	// Registered effect
	char effect[7] = "";

	if ((dwEffect & DROPEFFECT_COPY) == DROPEFFECT_COPY)
		strcpy(effect,"copy");
	else 
		if ((dwEffect & DROPEFFECT_MOVE) == DROPEFFECT_MOVE)
			strcpy(effect, "move");
		else 
			if ((dwEffect & DROPEFFECT_LINK) == DROPEFFECT_LINK)
				strcpy(effect, "link");
			else 
				if ((dwEffect & DROPEFFECT_SCROLL) == DROPEFFECT_SCROLL)
					strcpy(effect, "scroll");
				else 
					if ((dwEffect & DROPEFFECT_NONE) == DROPEFFECT_NONE)
						strcpy(effect, "none");


	// Prohibit other actions
	if (!ListFiles  &&  strcmp(effect,"copy")!=0  &&  strcmp(effect,"move")!=0)
	{
		FWerror(25,0,CommandID,"","",0,0,out);
		QWrite("\"", out); 
		QWrite(effect, out); 
		QWrite("\",[]]", out);
		GlobalUnlock(Data);
		CloseClipboard();
		free(missionDir);
		free(val); 
		break;
	}


	// Find clipboard size
	int varLen		 = 0;
	int nameLen      = 0;
	int largestName  = 1;
	bool lastWasNull = false;

	while (true)
	{
		if (pFilenames[varLen] == '\0')
		{
			if (nameLen > largestName)
				largestName = nameLen;

			nameLen = 0;

			if (lastWasNull) 
				break;
			else
				lastWasNull=true;
		}
		else
			lastWasNull = false;

		varLen++;
		nameLen++;
	};


	// Allocate buffer for the filenames from the clip
	char *var	 = (char*) malloc (varLen);
	char *source = var;

	if (var == NULL) 
	{
		FWerror(10,0,CommandID,"var ","",varLen,0,out);
		QWrite("\"", out); 
		QWrite(effect, out); 
		QWrite("\",[]]", out);
		free(missionDir);
		free(val); 
		break;
	};

	strcpy(var, "");


	// Reallocate destination buffer so that the filenames will fit
	if (destLen + largestName > valLen)
	{
		int distance = destination - val;
		valLen      += largestName;
		char *valNEW = (char*) realloc(val, valLen);

		if (valNEW != NULL) 
		{
			val         = valNEW;
			destination	= val + distance;
		}
		else 
		{	
			FWerror(10,0,CommandID,"val ","",valLen,0,out);
			QWrite("\"", out); 
			QWrite(effect, out); 
			QWrite("\",[]]", out);
			free(missionDir);
			free(val); 
			break;
		};
	}


	// Create array with feedback (will be sent to the game)
	FWerror(0,0,CommandID,"","",0,0,out);
	QWrite("\"", out); 
	QWrite(effect, out); 
	QWrite("\",[", out);


	// Parse filenames
	lastWasNull		 = false;
	char tmp[2]		 = "";
	ErrorWithinError = true;

	for (int i=0; true; i++)
	{
		// If stumbled on a separator between files
		if (pFilenames[i] == '\0')
		{
			// Quit if double-null (parsed all files)
			if (lastWasNull) 
				break;
			else
				lastWasNull = true;


			char *PTR_to_ofpdir = var;
			char *PTR_to_misdir = var;

			// Assign pointer to OFP root dir
			if (strncmpi(var,pwd,strlen(pwd)) == 0) 
				PTR_to_ofpdir = var + strlen(pwd) + 1,
				PTR_to_misdir = PTR_to_ofpdir;

			// Assign pointer to mission dir
			if (strncmpi(PTR_to_ofpdir, missionDir, strlen(missionDir)) == 0)
				PTR_to_misdir = PTR_to_ofpdir + strlen(missionDir);
			else
				// If it's not a path to mission dir then it needs to have ..\ at the beginning
				var[strlen(pwd)-1] = '.',
				var[strlen(pwd)-2] = '.',
				PTR_to_misdir	   = var + strlen(pwd) - 2;


			// Log
			QWrite("]+[[\"", out);

			//if (!ListFiles)
			//	QWrite(PTR_to_misdir, out);
			//else
			{
				// Separate file name from path
				char prev;
				for (int i=strlen(PTR_to_misdir);  i>=0 && PTR_to_misdir[i]!='\\' && PTR_to_misdir[i]!='/';  i--);

				if (i >= 0)
					prev			 = PTR_to_misdir[i],
					PTR_to_misdir[i] = '\0',
					QWrite(PTR_to_misdir, out),
					PTR_to_misdir[i] = prev;

				QWrite("\",\"",	out);
				QWrite(PTR_to_misdir+i+1, out);
			};

			QWrite("\",", out);


			// Verify path
			bool AllowCopy = true;

			if (strcmpi(effect,"move")==0  &&  isLeavingDir(PTR_to_misdir,0,RESTRICT_TO_MISSION_DIR,CommandID,out)) 
				AllowCopy = false;


			// Not allowed to move files to the download directory
			bool IsSourceDownload = isLeavingDir(PTR_to_misdir,1,RESTRICT_TO_MISSION_DIR,CommandID,out);

			if (strcmpi(effect,"move")==0  &&  !IsSourceDownload && IsDestinationDownload)
				FWerror(202,0,CommandID,source,"",0,0,out),
				AllowCopy = false;

			if (isAllowedExternalPath(PTR_to_misdir,ALLOW_GAME_ROOT_DIR)) 
				source = PTR_to_misdir+3;
			else
				source = PTR_to_ofpdir;


			// Copy filename from the source path to the destination path
			char *lastSlash = strrchr(source,'\\');

			if (destination[strlen(destination)-1] != '\\')
				strcat(destination,"\\");

			if (lastSlash != NULL)
				strcat(destination, lastSlash+1);
			else
				strcat(destination, source);


			// Perform file operation
			if (AllowCopy) 
			{
				if (!ListFiles)
				{
					if (strcmpi(effect,"move") == 0)
						if (rename(source,destination) == 0) 
							FWerror(0,0,CommandID,"","",0,0,out),
							EmptyClipboard();
						else 
							FWerror(7,errno,CommandID,source,"",0,0,out);

					if (strcmpi(effect, "copy") == 0)
						if (CopyFile((LPCTSTR)source, (LPCTSTR)destination, true)) 
							FWerror(0,0,CommandID,"","",0,0,out);
						else 
							FWerror(5,GetLastError(),CommandID,"","",0,0,out);
				}
				else
					FWerror(0,0,CommandID,"","",0,0,out);
			};


			// Clear
			destination[destLen] = 0;
			strcpy(var, "");
			QWrite("]", out);
		}
		else
			lastWasNull = false;


		// Fill buffer char by char
		sprintf(tmp, "%c", pFilenames[i]);
		strcat(var, tmp);
	};


	ErrorWithinError = false;
	QWrite("]]", out);

	free(missionDir);
	free(var); 
	free(val); 

	GlobalUnlock(Data);
	CloseClipboard();
}
break;

