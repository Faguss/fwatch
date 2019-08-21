// -----------------------------------------------------------------
// CLASS OPERATIONS
// -----------------------------------------------------------------

#define MAX_CLASSESINASINGLELINE 128
#define CLASSPATH classpath[10][128]

case C_CLASS_LIST:
{  // Return list of classes in a file

	// Read arguments -------------------------------------------------
	char *file			= "";
	char *rawPath		= "";
	int StartPos		= 0;
	bool PassedStartPos	= false;


	// Parse arguments
	for (int i=2; i<numP; i++)
	{
		char *arg = stripq(par[i]);
		char *pch = strchr(arg, ':');

		if (pch == NULL) 
			continue;

		int pos		= pch-arg;
		arg[pos]	= '\0';
		char *val	= Trim(arg+pos+1);

		if (strcmpi(arg,"file") == 0)
			file = val;

		if (strcmpi(arg,"classpath") == 0) 
			rawPath = stripq(val);

		if (strcmpi(arg,"offset") == 0) 
			PassedStartPos	= true, 
			StartPos		= atoi(val);
	};
	

	// File not specified
	if (strcmpi(file,"") == 0)
	{
		FWerror(107,0,CommandID,"file","",0,0,out);
		QWrite("0,[],[],[]]", out);
		break;
	};


	// Allocate buffer for the file path
	int File_name_len	= strlen(file) + 1;
	char *File_name		= (char*) malloc (File_name_len);
	char *WantedFile	= File_name;

	if (File_name == NULL) 
	{
		FWerror(10,0,CommandID,"File_name","",File_name_len,0,out);
		QWrite("0,[],[],[]]", out);
		break;
	};
	strcpy(File_name, file);


	// Mission dir path
	if (isAllowedExternalPath(File_name,ALLOW_GAME_ROOT_DIR)) 
		WantedFile = File_name + 3;
	else
		if (getMissionDir(&File_name, File_name_len, CommandID, out)) 
			WantedFile = File_name;
		else 
		{
			QWrite("0,[],[],[]]", out);
			free(File_name);
			break;
		};


	// Class path
	int J			= 0;	// J is current index
	int K			= -1;	// K is max
	char CLASSPATH;
	char *pch2		= strtok(rawPath, "[,]");

	while (pch2!=NULL  &&  K<10)	//from ofp array to char array
	{
		K++;
		char *item = stripq(pch2);
		
		strncpy(classpath[K], Trim(item), 127);
		pch2 = strtok(NULL, "[,]");
	};
	// ----------------------------------------------------------------



	// Parse text -----------------------------------------------------
	// Open file
	FILE *f = fopen(WantedFile, "r");
	if (!f) 
	{
		FWerror(7,errno,CommandID,WantedFile,"",0,0,out);
		QWrite("0,[],[],[]]", out);
		free(File_name); 
		break;
	};

	
	// Create dynamic buffers for storing text
	char *line;				// current line from file
	char *lineNEW;			// used when reallocating "line" (to match its length)
	char *match;			// stores word that's being parsed
	char *matchNEW;			// used when reallocating "match"
	char *classes;			// list of classes, for output
	char *classesNEW;		// used when reallocating "classes"
	char *inherits;			// list of inherited class names, for output
	char *inheritsNEW;		// used when reallocating "inherits"
	char *offset;			// position of the classes in the file (in bytes)
	char *offsetNEW;		// used when reallocating "offset"
	bool error		= false;// stop iterating
	int lineLen		= 512;	// length of the "line"
	int matchLen	= 100;	// length of the "match"
	int classesLen	= 24;	// length of the "classes"
	int inheritsLen = 24;	// length of the "inherits"
	int offLen		= 24;	// length of the "offset"

	// Allocate
	line		= (char*) malloc (lineLen);
	match		= (char*) malloc (matchLen);
	classes		= (char*) malloc (classesLen);
	inherits	= (char*) malloc (inheritsLen);
	offset		= (char*) malloc (offLen);
	
	char failedBuf[36]	= "";
	int failedBufL		= 0;

	if (line==NULL  ||  match==NULL  ||  classes==NULL  ||  inherits==NULL  ||  offset==NULL)
	{
		if (line == NULL)		{strcat(failedBuf,"line "); failedBufL+=matchLen;};
		if (match == NULL)		{strcat(failedBuf,"match"); failedBufL+=offLen;};
		if (classes == NULL)	{strcat(failedBuf,"classes "); failedBufL+=classesLen;};
		if (inherits == NULL)	{strcat(failedBuf,"inherits "); failedBufL+=inheritsLen;};
		if (offset == NULL)		{strcat(failedBuf,"offset "); failedBufL+=lineLen;};
		
		FWerror(10,0,CommandID,failedBuf,"",failedBufL,0,out);
		QWrite("0,[],[],[]]", out);
		free(File_name); 
		free(line);
		free(match);
		free(classes);
		free(inherits);
		free(offset);
		fclose(f);
		break;
	}
	else
		strcpy(line, ""),
		strcpy(match,""),
		strcpy(classes,"["),
		strcpy(inherits,"["),
		strcpy(offset,"[");
		

	// Variables:
	char *ret			= "";		// return value when reading line from text
	int l				= 0;		// length of the current line
	int level			= 0;		// inside how many brackets we currently are
	int lastPos			= 0;		// remember position (in bytes) in the text file in case we need to revert
	int classOff		= 0;		// remember position (in bytes) of the current class
	bool commentBlock	= false;	// indicate that current text is commented so ignore it
	bool quit			= false;	// stop iterating
	bool reached		= false;	// found the class that user wanted
	bool macroBlock		= false;	// indicate that we're in a function-like macro so ignore it
	bool inQuote		= false;	// indicate that we're in a string so ignore control characters
	bool matchClass		= false;	// stumbled upon class keyword, collect characters to form class name
	bool inherit		= false;	// going through inherited class name
	bool incJ			= false;	// increment J if stumbled upon class that was defined in the class path
    

	// Start reading file from the given position
	if (PassedStartPos && K>=0) 
		fseek(f,StartPos,SEEK_SET), 
		lastPos = StartPos, 
		J		= K, 
		level	= J;


	// For each line in a text file -----------------------------------
	while(ret = fgets(line, lineLen ,f))
	{
		if (ferror(f)) 
			FWerror(7,errno,CommandID,WantedFile,"",0,0,out), 
			error = true;

		if (quit || error) 
			break;


		// If a long line then reallocate buffer and read again
		l = strlen(line);

		if (line[l-1]!='\n'  &&  !feof(f)  &&  ret!=NULL)
		{
			lineLen += 512;
			lineNEW = (char*) realloc(line, lineLen);

			if (lineNEW!=NULL) 
				line=lineNEW; 
			else 
			{	
				FWerror(11,0,CommandID,"line","",lineLen,0,out);
				error = 1; 
				break;
			};

			fseek (f, lastPos, SEEK_SET);
			continue;
		};	

		lastPos = ftell(f);


		// Search for control characters
		bool bracketL = strchr(line, '{') != NULL;
		bool bracketR = strchr(line, '}') != NULL;
		bool asterisk = strchr(line, '*') != NULL;
		bool isClass  = false;


		// Search for classes in a line; save pos of every occurence to array
		char *res;
		char *lineTMP	= line;						// pointer for traversing through text
		int I			= 0;						// position in the array
		int CurrentCOL	= 0;						// position in the line
		int classPOS[MAX_CLASSESINASINGLELINE];		// hold list of classes

		while (res = strstr(lineTMP, "class "))
		{
			if (I >= MAX_CLASSESINASINGLELINE) // reached limit
				break;

			isClass		= true;
			int pos		= (res - lineTMP);
			classPOS[I] = pos + CurrentCOL;
			I++;
			pos			+= 6;
			CurrentCOL	+= pos;
			lineTMP		= line + CurrentCOL;
		};


		// If the line doesn't have any of the occurences then skip it
		if (!bracketL  &&  !bracketR  &&  !isClass  &&  !asterisk) 
			continue;


		// Go one by one char
		bool firstChar = true;	// are we on first alphanum character?
		bool hitAlfNum = false;	// 

		for (int i=0; i<l; i++)
		{
			// Ignore special characters if we're in quotation
			if (line[i] == '"') 
				inQuote = !inQuote; 
			
			if (inQuote) 
				continue;


			// Check for comments
			if (line[i]=='/'  &&  line[i+1]=='/') // single line comment - skip to the end of the line
				i = l-1;

			if (line[i]=='/'  &&  line[i+1]=='*') // entering comment block
				commentBlock = true;

			if (i>0  &&  line[i-1]=='*'  &&  line[i]=='/') // leaving comment block
			{
				commentBlock = false; 
				continue;
			};

			if (commentBlock  &&  line[i]!='\n') // ignore text in the comment block
				continue;


			// Check for preprocessor
			if (line[i]=='#'  &&  firstChar) 
			{
				firstChar = 0; 

				if(line[l-2]=='\\')
					macroBlock=1;
				
				break;
			};

			if (!isspace(line[i])  &&  firstChar) 
				firstChar = false;

			if (macroBlock)	
			{
				if (line[l-2] != '\\') 
					macroBlock = 0;
				
				break;
			};


			// For brackets
			if (line[i] == '{')		// opening
			{
				if (incJ) 
				{
					J++;
					incJ = 0;
				};
				
				level++;
			};

			if (line[i] == '}')		// closing
				level--;

			if (J>K  &&  !reached  &&  level==K+1) // found the final wanted class
				reached = true;

			if (J>K  &&  reached  &&  level<K+1  ||  level<J)  // passed through the wanted class - stop
			{
				quit = true; 
				break;
			};


			// Check if we encountered a class name
			if (tolower(line[i])=='c'  &&  level<=K+1  &&  isClass)
			{
                // if array 'classPOS' contains the same index
				for (int j=0; j<=I; j++) 
					if (i == classPOS[j]) 
					{
						matchClass	= true; 
						classOff	= lastPos - l + i - 1; 
						i += 6; 
						break;
					};
			};
			

			// When storing characters
			if (matchClass)
			{
				// At end of class name
				if (!inherit  &&  (isspace(line[i]) || line[i]=='{' || line[i]==':' || line[i]=='\n')) 
				{
					// If that's the class that was passed in arguments
					if (K>=0  &&  J<=K  &&  level==J)
						if (strcmpi(classpath[J],match) == 0)
							incJ = 1;

					// Output class name
					if (reached)
					{
						inherit = true;

						// Reallocate buffer, append new class name
						classesLen += strlen(match) + 8;
						classesNEW = (char*) realloc(classes, classesLen);

						if (classesNEW != NULL)
						{
							classes = classesNEW;
							strcat(classes,"]+[\""); 
							strcat(classes, match); 
							strcat(classes,"\"");
						}
						else 
						{
							FWerror(11,0,CommandID,"classes","",classesLen,0,out);
							error = true; 
							break;
						};
						
						// Reallocate buffer, append position of the class (in bytes)
						char tmp[16] = "";
						sprintf(tmp,"]+[\"%d\"",classOff);
						offLen		= offLen + strlen(tmp);
						offsetNEW	= (char*) realloc(offset, offLen);

						if (offsetNEW != NULL) 
							offset = offsetNEW, 
							strcat(offset, tmp);
						else
						{
							FWerror(11,0,CommandID,"offset","",offLen,0,out);
							error = true; 
							break;
						};
					}
					else 
						matchClass = false, 
						hitAlfNum  = false;
					
					strcpy(match,"");
				};

				// At the end of inherit
				if (inherit  &&  ((isspace(line[i]) && hitAlfNum) || line[i]=='{' || line[i]=='\n'))
				{
					inherit		= false; 
					matchClass	= false; 
					hitAlfNum	= false;
					
					// Output inherit
					if (reached)
					{
						// Reallocate buffer, append new class inherit name
						inheritsLen += strlen(match) + 8;
						inheritsNEW = (char*) realloc(inherits, inheritsLen);

						if (inheritsNEW != NULL)
						{
							inherits = inheritsNEW;
							strcat(inherits,"]+[\""); 
							strcat(inherits, match); 
							strcat(inherits,"\"");
						}
						else
						{
							FWerror(11,0,CommandID,"inherits","",inheritsLen,0,out);
							error = true; 
							break;
						};
					};

					strcpy(match,"");
					
					// If there's no space between name and a bracket - this will fix vars
					if (line[i] == '{') 
					{
						level--; 
						i--; 
						continue;
					};
				};

				// Save character to 'match' - building a class/inherit name
				if (matchClass  &&  !isspace(line[i])  &&  line[i]!=':')
				{
					sprintf(match, "%s%c", match, line[i]);		// Append char

					if ((int)strlen(match) >= matchLen-2)		// Reallocate if necessary
					{
						matchLen +=100;
						matchNEW  = (char*) realloc (match, matchLen);

						if (matchNEW != NULL) 
							match = matchNEW; 
						else 
						{
							FWerror(11,0,CommandID,"match","",matchLen,0,out);
							error = true; 
							break;
						}
					};
				};

				if (matchClass  &&  isalnum(line[i])  &&  inherit) 
					hitAlfNum = true;
			};
		};
	};
	// ----------------------------------------------------------------
	fclose(f);

	if (!error)
	{
		// If couldn't find classes that were in the class path
		if (J <= K)
		{
			FWerror(250,0,CommandID,WantedFile,classpath[J],J,++K,out);

			char tmp[12] = ""; 
			sprintf(tmp, "%d", J); 
			QWrite(tmp, out);
			QWrite(",[],[],[]]", out);
		}
		// No issues - output data
		else
		{
			FWerror(0,0,CommandID,"","",0,0,out);
			
			char tmp[12] = "";
			sprintf(tmp,"%d,",J);
			QWrite(tmp, out);
			
			strcat(classes, "],"); 
			strcat(inherits, "],");
			strcat(offset, "]]");
			QWrite(classes, out);
			QWrite(inherits, out);
			QWrite(offset, out);
		};
	}
	// If error - error info was already passed; now empty arrays
	else
	{
		char tmp[12] = ""; 
		sprintf(tmp, "%d", J); 
		QWrite(tmp, out);
		QWrite(",[],[],[]]", out);
	};

	free(File_name); 
	free(line); 
	free(match); 
	free(classes); 
	free(inherits); 
	free(offset);
};
break;












case C_CLASS_TOKENS:
{ // Return all properties from a class

	// Read arguments -------------------------------------------------
	char *file			= "";
	char *rawPath		= "";
	char *Target		= "";
	char *Wrap			= "";
	bool NoWrap			= false;
	bool NoDoubleWrap	= false;
	bool PassedStartPos = false;
	int  StartPos		= 0;

	// Parse arguments
	for (int i=2; i<numP; i++)
	{
		char *arg = stripq(par[i]);
		char *pch = strchr(arg, ':');

		if (pch == NULL) 
			continue;

		int pos		= pch-arg;
		arg[pos]	= '\0';
		char *val	= Trim(arg+pos+1);

		if (strcmpi(arg,"file") == 0) 
			file = val;

		if (strcmpi(arg,"classpath") == 0) 
			rawPath = stripq(val);

		if (strcmpi(arg,"offset") == 0) 
			PassedStartPos	= true, 
			StartPos		= atoi(val);

		if (strcmpi(arg,"token") == 0) 
			Target = val;

		if (strcmpi(arg,"wrap") == 0)
			Wrap = val;
	};


	// File not specified
	if (strcmpi(file,"") == 0)
	{
		FWerror(107,0,CommandID,"WantedFile","",0,0,out);
		QWrite("0,\"0\",[],[]]", out);
		break;
	};


	// Wrapping options
	if (strcmpi(Wrap,"no") == 0)
		NoWrap		 = true,
		NoDoubleWrap = false;

	if (strcmpi(Wrap,"nodouble") == 0)
		NoWrap		 = false,
		NoDoubleWrap = true;


	// Allocate buffer for the file path
	int File_name_len	= strlen(file) + 1;
	char *File_name		= (char*) malloc (File_name_len);
	char *WantedFile	= File_name;

	if (File_name == NULL) 
	{
		FWerror(10,0,CommandID,"File_name","",File_name_len,0,out);
		QWrite("0,\"0\",[],[]]", out);
		break;
	};
	strcpy(File_name, file);


	// Mission dir path
	if (isAllowedExternalPath(File_name,ALLOW_GAME_ROOT_DIR)) 
		WantedFile = File_name + 3;
	else
		if (getMissionDir(&File_name, File_name_len, CommandID, out)) 
			WantedFile = File_name;
		else 
		{
			QWrite("0,\"0\",[],[]]", out);
			free(File_name); 
			break;
		};


	// Class path
	int J			= 0;	// J is current index, 
	int K			= -1;	// K is max
	char CLASSPATH;
	char *pch		= strtok(rawPath, "[,]");

	while (pch!=NULL  &&  K<10)	//from ofp array to char array
	{
		K++;
		char *item = stripq(pch);
		strncpy(classpath[K], Trim(item), 127);
		pch = strtok(NULL, "[,]");
	};

	// If searching for specific token
	bool Search = strcmpi(Target,"")!=0;
	// ----------------------------------------------------------------



	// Parse text -----------------------------------------------------
	// Open file
	FILE *f = fopen(WantedFile, "r");
	if (!f) 
	{
		FWerror(7,errno,CommandID,WantedFile,"",0,0,out);
		QWrite("0,\"0\",[],[]]", out);
		free(File_name); 
		break;
	};


	// Create dynamic buffers for storing text
	char *line;					// current line from file
	char *lineNEW;				// used when reallocating "line" (to match its length)
	char *match;				// stores word that's being parsed
	char *matchNEW;				// used when reallocating "match"
	char *names;				// list with class properties names, for output
	char *namesNEW;				// used when reallocating "names"
	char *values;				// list with class properties values, for output
	char *valuesNEW;			// used when reallocating "values"
	bool error		= false;	// stop iterating
	int lineLen		= 512;		// length of the "line"
	int matchLen	= 100;		// length of the "match"
	int namesLen	= 24;		// length of the "names"
	int valuesLen	= 24;		// length of the "values"

	// Allocate
	line	= (char*) malloc (lineLen);
	match	= (char*) malloc (matchLen);
	names	= (char*) malloc (namesLen);
	values	= (char*) malloc (valuesLen);
	
	char failedBuf[40]	= "";
	int failedBufL		= 0;

	if (line==NULL  ||  match==NULL  ||  names==NULL  ||  values==NULL)
	{
		if (line == NULL)	{strcat(failedBuf,"line "); failedBufL+=lineLen;};
		if (match == NULL)	{strcat(failedBuf,"match"); failedBufL+=matchLen;};		
		if (names == NULL)	{strcat(failedBuf,"names "); failedBufL+=namesLen;};
		if (values == NULL) {strcat(failedBuf,"values "); failedBufL+=valuesLen;};

		FWerror(10,0,CommandID,failedBuf,"",failedBufL,0,out);
		QWrite("0,\"0\",[],[]]", out);
		free(File_name); 
		free(line);
		free(match);
		free(names);
		free(values);
		fclose(f);
		break;
	}
	else
		strcpy(line, ""),
		strcpy(match, ""),
		strcpy(names, "["),
		strcpy(values, "[");
				
		
	// Variables:
	char *ret			= "";		// return value when reading line from text
	int l				= 0;		// length of the current line
	int level			= 0;		// inside how many brackets we currently are
	int arrayLev		= 0;		// inside how many brackets in a property value array we are
	int valLen			= 0;		// for measuring lengths of items inside property value array
	int lastPos			= 0;		// remember position (in bytes) in the text file in case we need to revert
	int classOff		= 0;		// remember position (in bytes) of the current class
	bool commentBlock	= false;	// indicate that current text is commented so ignore it
	bool macroBlock		= false;	// indicate that we're in a function-like macro so ignore it
	bool quit			= false;	// stop iterating
	bool reached		= false;	// found the class that user wanted
	bool inQuote		= false;	// indicate that we're in a string so ignore control characters
	bool classAbove		= false;	// we're going trough a sub-class in a target class so skip it's contents
	bool inArray		= false;	// we're going trough property value that is an array
	bool value			= false;	// indicate that we're on the right side of the equality sign
	bool incJ			= false;	// increment J if stumbled upon class that was defined in the class path
	bool thatWasArray	= false;	// last matched property value was an array
	bool startsWithQuote= false;	// current scalar property value starts with quotation
	bool dontcopy		= false;	// if property is empty (left side is missing) then ignore it
	bool anticipateArray= false;	// don't end property on \n, wait instead for opening bracket

	if (K == -1) 
		reached = 1;

	// Start reading file from the given position
	if (PassedStartPos && K>=0) 
		fseek(f,StartPos,SEEK_SET), 
		lastPos = StartPos, 
		J		= K, 
		level	= J;


	// For each line in a text file -----------------------------------
	while (ret = fgets(line, lineLen ,f))
	{	
		if (ferror(f)) 
			FWerror(7,errno,CommandID,WantedFile,"",0,0,out), 
			error = true;

		if (quit || error) 
			break;


		// If a long line then reallocate buffer and read again
		l = strlen(line);

		if (line[l-1]!='\n'  &&  !feof(f)  &&  ret!=NULL)      // If long line
		{
			lineLen += 512;
			lineNEW = (char*) realloc(line, lineLen);

			if (lineNEW != NULL) 
				line = lineNEW; 
			else 
			{	
				FWerror(11,0,CommandID,"line","",lineLen,0,out);
				error = true;
				break;
			};

			fseek (f, lastPos, SEEK_SET);
			continue;
		};

		lastPos = ftell(f);


		// Search for crucial characters
		bool bracketL = strchr(line, '{') != NULL;
		bool bracketR = strchr(line, '}') != NULL;
		bool asterisk = strchr(line, '*') != NULL;
		bool equality = strchr(line, '=') != NULL;
		bool isClass  = false;


		// Search for classes in a line; save pos of every occurence to array
		char *res;
		char *lineTMP	= line;
		int I			= 0;
		int CurrentCOL	= 0;
		int classPOS[MAX_CLASSESINASINGLELINE];

		while (res = strstr(lineTMP, "class "))
		{
			if (I >= MAX_CLASSESINASINGLELINE) // reached limit
				break;

			isClass		= true;
			int pos		= (res - lineTMP);
			classPOS[I] = pos + CurrentCOL;
			I++;
			pos			+= 6;
			CurrentCOL	+= pos;
			lineTMP		= line + CurrentCOL;
		};


		// If in the line there isn't occurence we want then skip
		if (!bracketL  &&  !bracketR  &&  !isClass  &&  !asterisk  &&  !equality  &&  !reached) 
			continue;


		// Go one by one char
		bool between	= false;	// if there is more than one property in the line then indicate that we've finished current one
		bool firstChar	= true;		// indicate that we've hit first alfanum char after whitespace
		bool cont		= false;	// order to use 'continue' command

		for (int i=0; i<l; i++)
		{
			if (!inQuote)
			{
				// Check for comments
				if (!inQuote  &&  line[i]=='/'  &&  line[i+1]=='/') // single line comment - skip to the end of the line
					i = l - 1;

				if (!inQuote  &&  line[i]=='/'  &&  line[i+1]=='*')	// entering comment block
					commentBlock = true;

				if (i>0  &&  !inQuote  &&  line[i-1]=='*'  &&  line[i]=='/') // leaving comment block
				{
					commentBlock = false; 
					continue;
				};

				if (commentBlock  &&  line[i]!='\n')	// ignore text in the comment block
					continue;
			};
			

			// Check for preprocessor
			if (line[i]=='#'  &&  firstChar) 
			{
				firstChar = false;

				if (line[l-2] == '\\')
					macroBlock = true;
				
				break;
			};

			if (!isspace(line[i])  &&  firstChar) 
				firstChar = false;

			if (macroBlock) 
			{
				if (line[l-2] != '\\') 
					macroBlock = false;
				
				break;
			};


			// Ignore special characters if we're in quotation
			if (line[i] == '"') 
				inQuote = !inQuote;

			if (!inQuote)
			{
				// If a bracket
				if (line[i]=='{'  &&  value)	// opening array
					anticipateArray = false, 
					inArray			= true;

				if (line[i]=='{'  &&  !value)	// opening class
				{
					if (incJ) 
						J++,
						incJ = 0;

					level++;
				};

				if (line[i]=='}'  &&  !value)	// closing class
					level--;

				if (line[i]=='{'  &&  value)	// opening array
					arrayLev++;

				if (line[i]=='}'  &&  value)	// closing array
					arrayLev--;

				if (J>K && !reached  &&  level==K+1) // found the final wanted class
					reached = true;

				if (J>K  &&  reached  &&  level<K+1  ||  level<J) // passed through the wanted class - stop
				{
					quit = true; 
					break;
				};

				if (J>K  &&  reached  &&  level>K+1  &&  !classAbove) // if entered a sub-class inside desired class
				{
					classAbove = true; 
					continue;
				}; 

				if (J>K  &&  reached  &&  classAbove  &&  level==K+1  &&  !isalnum(line[i]))	// when leaving sub-class inside desired class
					continue; 
				else 
					classAbove = false; 

				if ((line[i]=='{' || line[i]=='}')  &&  !value) 
					continue;


				// Check if we encountered a class name
				if (tolower(line[i])=='c'  &&  isClass)
				{
					// if array 'classPOS' contains the same index
					for (int j=0; j<=I; j++)
					{
						char className[128] = "";

						if (i == classPOS[j])
						{
							// Get class name 
							for (int k=i+6,i3=0;  k<l && i3<128;  k++,i3++)
							{
								if (isspace(line[k])  ||  line[k]==':'  ||  line[k]=='{') 
									break;

								sprintf(className, "%s%c", className,line[k]);
							};

							// If reached class that was in passed arguments
							if (K>=0  &&  J<=K  &&  level==J)
								if (strcmpi(classpath[J],className)==0)
									classOff = lastPos - l + i - 1,
									incJ	 = true;

							// Jump to the EOL or bracket opening of that class
							for (int i2=i; i2<l; i2++) 
								if (line[i2] == '{')
								{
									i2--;
									break;
								};

							i	 = i + i2 - i - 1;
							cont = true;
						};
					};

					if (cont) 
					{
						cont = false; 
						continue;
					};
				};
			};


			// Save properties
			if ((equality || inArray)  &&  reached  &&  level==K+1)
			{
				// after finishing property don't start another one until hit alphanum character
				if (between  &&  !isalnum(line[i])) 
					continue; 
				else 
					between = false;

				// Purge 'match' if it's a word that ended without '='
				if (!inQuote  &&  line[i]==';'  &&  !inArray  &&  !value) 
					strcpy(match, "");

				// if hit property separator or line end
				if (!inQuote  &&  (line[i]==';' && arrayLev==0 || line[i]=='\n' && !anticipateArray && !inArray || line[i]=='}' && !inArray)  &&  value)
				{
					if (valLen==0  &&  !thatWasArray) 
						strcat(match,"\"");	// quote wasn't added on the start if it's empty value

					thatWasArray = false;

					if (!inArray) 
						if (!NoWrap  &&  !(NoDoubleWrap && startsWithQuote)  ||  valLen==0)
							strcat(match,"\"");

					value = false;

					if (Search  &&  !dontcopy) 
						quit = 1;

					if (line[i] == ';') 
						between = true;

					if (!dontcopy) 
					{
						// NoDoubleWrap for array items
						if (NoDoubleWrap  &&  inArray)
						{
							int max = strlen(match) - 3;

							for (int g=0;  g<max;  g++)
							{
								if (match[g]=='['  &&  match[g+1]=='\"'  &&  match[g+2]=='\"'  &&  match[g+3]=='\"'  || 
									match[g]=='\"'  &&  match[g+1]=='\"'  &&  match[g+2]=='\"'  &&  match[g+3]==']')
										match[g+1] = ' ',
										match[g+2] = ' ';
							};
						};
									
						// Reallocate buffer, append property value
						valuesLen = valuesLen + strlen(match) + 4;
						valuesNEW = (char*) realloc(values, valuesLen);

						if (valuesNEW != NULL)
						{
							values = valuesNEW;
							strcat(values, "]+[");
							strcat(values, match);
						}
						else 
						{
							FWerror(11,0,CommandID,"values","",valuesLen,0,out);
							error = true; 
							break;
						};
					}
					else 
						dontcopy = false;

					valLen	= 0;
					inArray = false;
					strcpy(match, "");
					continue;
				};

				// if hit property name/value separator (equality sign)
				if (!inQuote  &&  line[i]=='=')
				{
					value = true;

					if (match[strlen(match)-1]==']'  &&  match[strlen(match)-2]=='[') //if array bracket starts in the next line
						anticipateArray = true;

					if (strcmp(match,"")==0  ||  Search  &&  strcmpi(match,Target)!=0) 
						dontcopy = true; 
					else 
					{
						dontcopy = false;

						// Reallocate buffer, append new class name
						namesLen = namesLen + strlen(match) + 7;
						namesNEW = (char*) realloc(names, namesLen);

						if (namesNEW != NULL)
						{
							names = namesNEW;
							strcat(names, "]+[\""); 

							// double the amount of quotation marks in the property name
							if (strchr(match,'\"'))
							{
								char *rep = str_replace(match,"\"","\"\"",0,0);

								if (rep == NULL) 
								{
									FWerror(12,0,CommandID,"part (' with '')","",strlen(match),0,out);
									error = true; 
									break;
								};

								strcat(names, rep);
								free(rep);
							}
							else 
								strcat(names, match);

							strcat(names,"\"");
						}
						else 
						{
							FWerror(11,0,CommandID,"names","",namesLen,0,out);
							error = true; 
							break;
						};
					};

					strcpy(match,"");
					continue;
				}; 
				
				// Save character to 'match' - building property name/property value
				if (!(isspace(line[i]) && valLen==0)  &&  !(!value&&line[i]=='/')  &&  !(strlen(match)==0  &&  line[i]==';'))
				{
					if (strlen(match)==0  &&  value  &&  !inArray) 
					{
						startsWithQuote = line[i]=='\"';

						if (!NoWrap  &&  !(NoDoubleWrap && startsWithQuote))	// Open quotation for scalars
							strcat(match,"\"");
					};

					if (value) 
						valLen++;

					char StringValLen[50] = "";

					// If it's special character then replace it; default action - just append
					// replace brackets in the first dimension
					if (value  &&  !inQuote  &&  inArray  &&  arrayLev==1  &&  line[i]=='{') 
					{
						valLen--;
						strcat(StringValLen, "[");
						strcat(match, "[");

						if (!NoWrap) 
							strcat(match, "\"");
					}
					else 
						if (value  &&  !inQuote  &&  inArray  &&  arrayLev==0  &&  line[i]=='}') 
						{
							valLen--;
							sprintf(StringValLen,"%d]", valLen);

							if (!NoWrap) 
								strcat(match, "\"");

							// If empty array then remove quotes
							unsigned int ll = strlen(match);

							if (valLen==0  &&  match[ll-1]=='\"'  &&  match[ll-2]=='\"'  &&  match[ll-3]=='[')
								match[ll-1] = ' ',
								match[ll-2] = ' ';

							strcat(match, "]");
							thatWasArray = true;
						}
					else
						// replace commas with append to counter OFP allocation limit (only in the first dimension)
						if (value  &&  !inQuote  &&  inArray  &&  arrayLev==1  &&  (line[i]==',' || line[i]==';')) 
						{
							valLen--;
							sprintf(StringValLen, "%d]+[", valLen);

							if (!NoWrap) 
								strcat(match, "\"");

							strcat(match, "]+[");

							if (!NoWrap) 
								strcat(match, "\"");
						}
					else
						// if array spans across multiple lines then don't include whitespace
						if (value  &&  !inQuote  &&  inArray  &&  arrayLev==1  &&  isspace(line[i]))
							line[i] = ' ';
					else
						// double the amount of quotation marks
						if (value  &&  line[i]=='"'  &&  !NoWrap  &&  (!NoDoubleWrap || inArray)) 
							strcat(match, "\"\"");
					else
						// default - append char
						sprintf(match, "%s%c", match, line[i]);


					// Reallocate buffer if necessary
					if ((int)strlen(match) >= matchLen-10)
					{
						matchLen += 100;
						matchNEW = (char*) realloc (match, matchLen);

						if (matchNEW != NULL) 
							match = matchNEW; 
						else 
						{
							FWerror(11,0,CommandID,"match","",matchLen,0,out);
							error = true; 
							break;
						}
					};

				};// end save to match

			};// end save properties

		};// end one by one char

	};// end for each line
	// ----------------------------------------------------------------
	fclose(f);


	if (!error)
	{
		// If couldn't find classes that were in the class path
		if (J <= K)
		{
			FWerror(250,0,CommandID,WantedFile,classpath[J],J,++K,out);

			char tmp[32] = ""; 
			sprintf(tmp, "%d,\"%d\"", J,classOff);
			QWrite(tmp, out);
			QWrite(",[],[]]", out);
		}
		// No issues - output data
		else
		{
			FWerror(0,0,CommandID,"","",0,0,out);

			char tmp[32] = ""; 
			sprintf(tmp, "%d,\"%d\",", J,classOff);
			QWrite(tmp, out);

			strcat(names, "],");
			strcat(values, "]]");
			QWrite(names, out);
			QWrite(values, out);
		};
	}
	// If error - error info was already passed; give empty arrays to the game
	else
	{
		char tmp[32] = ""; 
		sprintf(tmp, "%d,\"%d\"", J,classOff);
		QWrite(tmp, out);
		QWrite(",[],[]]", out);
	};

	free(File_name); 
	free(names); 
	free(values); 
	free(line); 
	free(match); 
}
break;












case C_CLASS_MODIFY:
{ // Modify class name in a file

	// Read arguments -------------------------------------------------
	int action		= 0;
	char *file		= "";
	char *rawPath	= "";
	char *Target	= "";
	char *RenameDst = "";


	// Parse arguments
	for (int i=2; i<numP; i++)
	{
		char *arg = stripq(par[i]);
		char *pch = strchr(arg, ':');

		if (pch == NULL) 
			continue;

		int pos		= pch - arg;
		arg[pos]	= '\0';
		char *val	= Trim(arg+pos+1);

		if (strcmpi(arg,"file") == 0) 
			file = val;

		if (strcmpi(arg,"classpath") == 0) 
			rawPath = stripq(val);

		if (strcmpi(arg,"add") == 0)	// add new class
			action = 1, 
			Target = val;		

		if (strcmpi(arg,"rename") == 0)	// rename source class
			action = 2, 
			Target = val;

		if (strcmpi(arg,"to") == 0) 	// rename destination class
			RenameDst = val;

		if (strcmpi(arg,"delete") == 0)	// remove class
			action = 3, 
			Target = val;
	};


	// File not specified
	if (strcmpi(file,"") == 0)
	{
		FWerror(107,0,CommandID,"WantedFile","",0,0,out);
		QWrite("0]", out);
		break;
	};


	// If action was not determined
	if (action == 0)
	{
		FWerror(106,0,CommandID,"","",0,0,out);
		QWrite("0]", out);
		break;
	};


	// Nothing to rename to
	if (action==2  &&  (strcmp(Target,"")==0 || strcmp(RenameDst,"")==0))
	{
		char tmp[20] = "";
		if (strcmp(Target,"") == 0) 
			strcat(tmp, "OldName");

		if (strcmp(RenameDst,"") == 0)
		{
			if (strlen(tmp) > 0) 
				strcat(tmp,", ");

			strcat(tmp,"NewName");
		};

		FWerror(107,0,CommandID,tmp,"",0,0,out);
		QWrite("0]", out);
		break;
	};


	// Allocate buffer for the file path
	int File_name_len	= strlen(file) + 1;
	char *File_name		= (char*) malloc (File_name_len);
	char *WantedFile	= File_name;

	if (File_name == NULL) 
	{
		FWerror(10,0,CommandID,"File_name","",File_name_len,0,out);
		QWrite("0,[],[],[]]", out);
		break;
	};
	strcpy(File_name, file);


	// Mission dir path
	if (isLeavingDir(File_name,0,RESTRICT_TO_MISSION_DIR,CommandID,out)) 
	{
		free(File_name); 
		break;
	};

	if (isAllowedExternalPath(File_name,RESTRICT_TO_MISSION_DIR)) 
		WantedFile = File_name + 3;
	else
		if (getMissionDir(&File_name, File_name_len, CommandID, out)) 
			WantedFile = File_name;
		else 
		{ 
			QWrite("0,[],[],[]]", out);
			free(File_name);
			break;
		};


	// Class path
	int J			= 0;	// J is current index
	int K			= -1;	// K is max
	char CLASSPATH; 
	char *pch		= strtok(rawPath, "[,]");

	while (pch!=NULL  &&  K<10)	//from ofp array to char array
	{
		K++;
		char *item = stripq(pch);
		strncpy(classpath[K], Trim(item), 127);
		pch = strtok(NULL, "[,]");
	};


	// Split argument into classname and inherit
	char *WantedClass	= Target; 
	char *inherit		= "";

	if (pch = strchr(WantedClass, ':'))
	{
		int pos			 = pch - WantedClass;
		inherit			 = WantedClass + pos + 1;
		WantedClass[pos] = '\0';
	};

	WantedClass = Trim(WantedClass);
	inherit		= Trim(inherit);
	// ----------------------------------------------------------------


	// Open file ------------------------------------------------------
	FILE *f = fopen(WantedFile, "r");
	if (!f) 
	{
		FWerror(7,errno,CommandID,WantedFile,"",0,0,out);
		QWrite("0]", out);
		free(File_name); 
		break;
	};


	// Find file size, allocate buffer
	fseek(f, 0, SEEK_END);

	char* buf	= 0;
	int bufsize = 0;
	int fsize	= ftell(f);
	buf			= new char[fsize + 70 + (strlen(com))];

	if (!buf)
	{
		FWerror(10,0,CommandID,"buf","",fsize+70+(strlen(com)),0,out);
		QWrite("0]", out);
		free(File_name); 
		fclose(f);
		break;
	};

	fseek(f, 0, SEEK_SET);
	// ----------------------------------------------------------------


	// Create dynamic buffers for storing text
	char *line;				// current line from file
	char *lineNEW;			// used when reallocating "line" (to match its length)
	char *line2;			// modified text line for the writing buffer
	char *line2NEW;			// used when reallocating "line2"
	char *match;			// stores word that's being parsed
	char *matchNEW;			// used when reallocating "match"
	int error		= 0;	// stop iterating
	int lineLen		= 512;	// length of the "line"
	int matchLen	= 100;	// length of the "match"

	// Allocate
	line	= (char*) malloc (lineLen);
	line2	= (char*) malloc (lineLen);
	match	= (char*) malloc (matchLen);

	char failedBuf[20]	= "";
	int failedBufL		= 0;

	if (line==NULL  ||  line2==NULL  ||  match==NULL)
	{
		if (line==NULL)  {error=1; strcat(failedBuf,"line "); failedBufL=+lineLen;};
		if (line2==NULL) {error=1; strcat(failedBuf,"line2 "); failedBufL+=lineLen;};
		if (match==NULL) {error=1; strcat(failedBuf,"match"); failedBufL+=matchLen;};

		FWerror(10,0,CommandID,failedBuf,"",failedBufL,0,out);
		QWrite("0]", out);
		free(File_name);
		free(line);
		free(line2);
		free(match);
		fclose(f);
		break;
	}
	else
		strcpy(line, ""),
		strcpy(line2, ""),
		strcpy(match, "");
		

	// Variables:
	char *ret			= "";		// return value when reading line from text
	int l				= 0;		// length of the current text line
	int level			= 0;		// inside how many brackets we currently are
	int remLevel		= 0;		// remember level of the class we want to remove
	int lastPos			= 0;		// remember position (in bytes) in the text file in case we need to revert
	int Cpos			= 0;		// remember position of the current "class" keyword
	bool commentBlock	= false;	// indicate that current text is commented so ignore it
	int macroBlock		= false;	// indicate that we're in a function-like macro so ignore it
	int reached			= false;	// found the class that user wanted
	int remove			= false;	// in the process of removing a class
	int reachedRemLev	= false;	// we're inside the class that was ordered for removal
	int jobDone			= false;	// just copy rest of the file
	int matchClass		= false;	// in the process of comparing current class to the one we seek
	int incJ			= false;	// increment J if stumbled upon class that was defined in the class path


	// For each line in a text file -----------------------------------
	while(ret = fgets(line, lineLen ,f))
	{
		if (ferror(f)) 
			FWerror(7,errno,CommandID,WantedFile,"",0,0,out), 
			error = 1;

		if (error) 
			break;


		// If a long line then reallocate buffer and read again
		l = strlen(line);

		if (line[l-1]!='\n'  &&  !feof(f)  &&  ret!=NULL)
		{
			lineLen  += 512;
			lineNEW  = (char*) realloc(line, lineLen);
			line2NEW = (char*) realloc(line2, lineLen);

			if (lineNEW != NULL) 
				line = lineNEW; 
			else 
			{
				FWerror(11,0,CommandID,"line","",lineLen,0,out);
				error = 1; 
				break;
			};

			if (line2NEW != NULL) 
				line2 = line2NEW; 
			else
			{
				FWerror(11,0,CommandID,"line2","",lineLen,0,out); 
				error = 1; 
				break;
			};

			fseek (f, lastPos, SEEK_SET);
			continue;
		};	

		lastPos = ftell(f);


		// If we don't have to search/modify anymore then just copy rest of the file
		if (jobDone) 
		{
			memcpy(buf+bufsize, line, l+1); 
			bufsize += l; 
			continue;
		};


		// Search for crucial characters
		bool bracketL = strchr(line, '{') != NULL;
		bool bracketR = strchr(line, '}') != NULL;
		bool asterisk = strchr(line, '*') != NULL;
		bool isClass  = false;


		// Search for classes in a line; save pos of every occurence to array
		char *res;
		char *lineTMP	= line;
		char I			= 0;
		char CurrentCOL = 0;
		int classPOS[MAX_CLASSESINASINGLELINE];

		while (res = strstr(lineTMP, "class "))
		{
			if (I >= MAX_CLASSESINASINGLELINE) // reached limit
				break;

			isClass		= true;
			int pos		= (res - lineTMP);
			classPOS[I] = pos + CurrentCOL;
			I++;
			pos			+= 6;
			CurrentCOL	+= pos;
			lineTMP		= line + CurrentCOL;
		};


		// Go one by one char
		bool firstChar	= true;	// are we on first alphanum character?
		bool inQuote	= false;

		for (int i=0;  i<l && (bracketL || bracketR || isClass || asterisk);  i++)
		{
			// Ignore special characters if we're in quotation
			if (line[i] == '"') 
				inQuote = !inQuote; 
			
			if (inQuote) 
				continue;


			// Check for comments
			if (line[i]=='/'  &&  line[i+1]=='/') // single line comment - skip to the end of the line
				i = l - 1;

			if (line[i]=='/'  &&  line[i+1]=='*') // entering comment block
				commentBlock = true;

			if (i>0  &&  line[i-1]=='*'  &&  line[i]=='/') // leaving comment block
			{
				commentBlock = false; 
				continue;
			};

			if (commentBlock  &&  line[i]!='\n') // ignore text in the comment block
				continue;


			// Check for preprocessor
			if (line[i]=='#'  &&  firstChar) 
			{
				firstChar = 0; 
				
				if (line[l-2] == '\\')
					macroBlock = 1;
				
				break;
			};


			if (!isspace(line[i]) && firstChar) 
				firstChar = false;

			if (macroBlock) 
			{
				if (line[l-2]!='\\') 
					macroBlock = 0;
				
				break;
			};


			// For brackets
			if (line[i] == '{')		// opening
			{
				if (incJ)
				{
					J++;
					incJ = 0;
				};
				
				level++;
			};

			if (line[i] == '}')		// closing
				level--;

			if (J>K  &&  !reached  &&  level==K+1) // found the final wanted class
				reached = true;


			// Add new class
			if (J>K  &&  reached  &&  level<K+1  &&  action==1)
			{
				// Copy anything that's before the closing bracket;
				strcpy(line2, line);

				if (line2[i] == '}') 
					line2[i] = '\0';

				if (line2[i+1] == ';') 
					i++,
					line2[i+1] = '\0';

				int l2 = strlen(line2);
				memcpy(buf+bufsize, line2, l2+1);
				bufsize += l2;

				// Add new class
				char tabs[10] = "";

				for (int z=0;  z<level && z<10;  z++) 
					strcat(tabs, "\t");

				sprintf(line2, "\n%s\tclass %s ", tabs, WantedClass);

				if (strcmp(inherit,"") != 0) 
					sprintf(line2, "%s: %s ", line2,inherit);

				sprintf(line2, "%s\n%s\t{\n%s\t};\n%s};\n",line2,tabs,tabs,tabs);
				l2 = strlen(line2);
				memcpy(buf+bufsize, line2, l2+1);
				bufsize += l2;

				// Add what was after the closing bracket;
				strcpy(line2, "");

				for (i++; i<l; i++) 
					sprintf(line2,"%s%c",line2,line[i]);

				strcpy(line, line2);
				jobDone = true; 
				break;
			};


			if (remove  &&  level==remLevel) // Inside class that we want to remove
				reachedRemLev = true; 


			if (remove && reachedRemLev && level<remLevel)    // Finish removing class
			{
				// Copy everything after the class has ended
				strcpy(line2,"");

				if (line[i+1] == ';') 
					i++;

				for (int z=i+1;  z<l;  z++) 
					sprintf(line2,"%s%c",line2,line[z]);

				strcpy(line, line2);
				remove  = false; 
				jobDone = true; 
				break;
			};


			// Couldn't find class that was in classpath
			if (level<J  &&  !jobDone) 
			{
				error = 3; 
				break;
			};


			// Check if it's a class
			if (tolower(line[i])=='c'  &&  (reached && level<=K+1 || !reached && level==J)  &&  isClass)
			{
				// if array 'classes' contains the same index
				for (int j=0; j<=I; j++) 
					if (i == classPOS[j]) 
					{
						Cpos	   = i;
						matchClass = true; 
						i		  += 6; 
						break;
					};
			};


			// When storing characters
			if (matchClass)
			{
				// At end of class name
				if ((isspace(line[i])  ||  line[i]=='{'  ||  line[i]==':'  ||  line[i]=='\n')) 
				{
					// If reached class that was in passed arguments
					if (K>=0  &&  J<=K  &&  level==J)
						if (strcmpi(classpath[J],match)==0) 
							incJ = true;

					// If you want to add global class that already exist
					int level2 = level; 

					if (line[i] == '{') 
						level2--;

					if (K==-1  &&  action==1  &&  level2==0  &&  strcmpi(match,WantedClass)==0) 
					{
						error	= 2 ; 
						jobDone = true;
					};

					// Output class name
					if (reached  &&  strcmpi(match,WantedClass)==0)
					{
						// If it's the same class that you wanted to add
						if (action == 1) 
						{
							error	= 2; 
							jobDone = true;
						};

						// Renaming class
						if (action == 2)
						{
							// Copy anything that's before the class name
							strcpy(line2, "");

							for (int z=0;  z<(Cpos+6);  z++) 
								sprintf(line2, "%s%c", line2,line[z]);

							int l2 = strlen(line2);
							memcpy(buf+bufsize, line2, l2+1);
							bufsize += l2;

							// Add new class name
							sprintf(line2, "%s", RenameDst);
							l2 = strlen(line2);
							memcpy(buf+bufsize, line2, l2+1);
							bufsize += l2;

							// Copy anything that's after the class name
							strcpy(line2, "");

							// Search for inherit first
							bool hitAlfNum	= false; 
							bool isInherit	= false;
							bool comBlock	= false;

							for (z=i; z<l; z++)
							{
								// Check for comments
								if (line[i]=='/'  &&  line[i+1]=='/') 
									break;

								if (line[i]=='/'  &&  line[i+1]=='*') 
									comBlock = true;

								if (i>0 && line[i-1]=='*' && line[i]=='/') 
								{
									comBlock = false; 
									continue;
								};

								if (comBlock) continue;

								if (line[z]=='{'  ||  line[z]==0x0A  ||  line[z]==0x0D)  // hit end
									break;

								if (line[z] == ':')		 // hit separator
								{
									isInherit = true; 
									continue;
								};

								if (hitAlfNum  &&  (isspace(line[z]) || line[z]=='{' || line[z]==0x0A || line[z]==0x0D))  // ended inherit
									break;

								if (isInherit  &&  !isspace(line[z]))  // found inherit
									hitAlfNum = true;
							};

							if (hitAlfNum) 
								i = z;

							for (; i<l; i++) 
								sprintf(line2,"%s%c",line2,line[i]);

							strcpy(line, line2);
							jobDone = true;
						};


						// Removing class
						if (action == 3)
						{
							remove	 = true; 
							remLevel = level2+1;

							if (line[i] == '{') 
								reachedRemLev = true;

							// Copy anything that's before the class name
							strcpy(line2, "");

							if (i>0)
							{
								for (int z=0; z<Cpos; z++) 
									sprintf(line2, "%s%c", line2,line[z]);

								int l2 = strlen(line2);
								memcpy(buf+bufsize, line2, l2+1);
								bufsize += l2;
							};
						};
					};

					matchClass = false; 
					strcpy(match, "");

					// If there's no space between name and a bracket - this will fix vars
					if (line[i] == '{') 
					{
						level--; 
						i--; 
						continue;
					};
				};


				// Save character to 'match' - building a class name
				if (matchClass  &&  !isspace(line[i])  &&  line[i]!=':')
				{
					sprintf(match, "%s%c", match, line[i]);		// Append char

					// Reallocate if necessary
					if ((int)strlen(match) >= matchLen-2)
					{
						matchLen += 100;
						matchNEW = (char*) realloc (match, matchLen);

						if (matchNEW != NULL) 
							match = matchNEW; 
						else 
						{
							FWerror(11,0,CommandID,"match","",lineLen,0,out);
							error = 1; 
							break;
						}
					};
				};
			};
		};


		// Copy line to the buffer
		if (remove) 
			continue;

		l = strlen(line);
		memcpy(buf+bufsize, line, l+1);
		bufsize += l;
	};
	// ----------------------------------------------------------------
	fclose(f);


	if (!error)
	{
		// If couldn't find classes that were in the class path
		if (J <= K)
			FWerror(250,0,CommandID,WantedFile,classpath[J],J,++K,out);
		else 
			// Couldn't remove/rename global class
			if (action>1  &&  !jobDone) 
				FWerror(252,0,CommandID,WantedFile,WantedClass,0,0,out);
		else
		{
			// Add new global class
			if (K==-1  &&  action==1)
			{
				sprintf(line, "\nclass %s ", WantedClass);

				if (strcmp(inherit,"") != 0) 
					sprintf(line, "%s: %s ", line,inherit);

				sprintf(line, "%s\n{\n};\n", line);

				int l = strlen(line);
				memcpy(buf+bufsize, line, l+1);
				bufsize += l;
			};

			// Rewrite the file
			f = fopen(WantedFile, "w");
			if (f) 
			{
				fwrite(buf, 1, strlen(buf), f);

				if (ferror(f)) 
					FWerror(7,errno,CommandID,WantedFile,"",0,0,out);
				else
					FWerror(0,0,CommandID,"","",0,0,out);

				fclose(f);
			}
			else 
				FWerror(7,errno,CommandID,WantedFile,"",0,0,out);
		};
	};
	

	// Class already exists
	if (error == 2) 
		FWerror(251,0,CommandID,WantedFile,WantedClass,0,0,out);


	// Couldn't find class
	if (error == 3) 
		FWerror(252,0,CommandID,WantedFile,WantedClass,0,0,out);


	// Output position in the class path
	char tmp[14] = ""; 
	sprintf(tmp, "%d]", J);
	QWrite(tmp, out);


	delete[] buf; 
	free(File_name); 
	free(line); 
	free(line2); 
	free(match);
}
break;












case C_CLASS_MODTOK:
{ // Modify property within a class

	// Read arguments -------------------------------------------------
	int action			= 0; 
	int ARRtargetID		= -1;
	int ArrAppend		= 0;
	int ArrDelete		= 0;
	char *file			= "";
	char *rawPath		= "";
	char *WantedToken	= "";
	char *WantedValue	= "";
	char *RenameDst		= "";


	// Parse arguments
	for (int i=2; i<numP; i++)
	{
		char *arg = stripq(par[i]);
		char *pch = strchr(arg, ':');

		if (pch == NULL) 
			continue;

		int pos		= pch-arg;
		arg[pos]	= '\0';
		char *val	= Trim(arg+pos+1);

		if (strcmpi(arg,"file") == 0) 
			file = val;

		if (strcmpi(arg,"classpath") == 0) 
			rawPath = stripq(val);

		if (strcmpi(arg,"add") == 0) 		// add/overwrite
			action = 1, 
			WantedToken = val;

		if (strcmpi(arg,"append") == 0) 	// append
			action = 4, 
			WantedToken = val;

		if (strcmpi(arg,"rename") == 0)		// rename source
			action = 2, 
			WantedToken = val;

		if (strcmpi(arg,"to") == 0)			// rename destination
			RenameDst = val;

		if (strcmpi(arg,"delete") == 0) 	// remove property
			action = 3, 
			WantedToken = val;

		if (strcmpi(arg,"index")==0  &&  action!=2) // modify item inside array
		{
			if (action == 3) 
				ArrDelete = 1;

			if (action == 4) 
				ArrAppend = 1;

			action		= 5; 
			ARRtargetID = atoi(val);
		};
	};


	// File not specified
	if (strcmpi(file,"") == 0)
	{
		FWerror(107,0,CommandID,"WantedFile","",0,0,out);
		QWrite("0]", out);
		break;
	};


	// If action was not determined
	if (action==0  ||  action==5  &&  strcmp(WantedToken,"")==0)
	{
		FWerror(106,0,CommandID,"","",0,0,out);
		QWrite("0]", out);
		break;
	};


	// Nothing to rename to
	if (action==2  &&  (strcmp(WantedToken,"")==0 || strcmp(RenameDst,"")==0))
	{
		char tmp[20] = "";

		if (strcmp(WantedToken,"") == 0) 
			strcat(tmp, "OldName");

		if (strcmp(RenameDst,"") == 0)
		{
			if (strlen(tmp) > 0) 
				strcat(tmp, ", ");

			strcat(tmp, "NewName");
		};

		FWerror(107,0,CommandID,tmp,"",0,0,out);
		QWrite("0]", out);
		break;
	};


	// Incorrect array index
	if (action==5  &&  ARRtargetID<0)
	{
		FWerror(101,0,CommandID,"ARRtargetID","",ARRtargetID,0,out);
		QWrite("0]", out);
		break;
	};


	// Allocate buffer for the file path
	int File_name_len	= strlen(file) + 1;
	char *File_name		= (char*) malloc (File_name_len);
	char *WantedFile	= File_name;

	if (File_name == NULL) 
	{
		FWerror(10,0,CommandID,"File_name","",File_name_len,0,out);
		QWrite("0,[],[],[]]", out);
		break;
	};
	strcpy(File_name, file);


	// Mission dir path
	if (isLeavingDir(File_name,0,RESTRICT_TO_MISSION_DIR,CommandID,out)) 
	{
		free(File_name); 
		break;
	};

	if (isAllowedExternalPath(File_name,RESTRICT_TO_MISSION_DIR)) 
		WantedFile = File_name + 3;
	else
		if (getMissionDir(&File_name, File_name_len, CommandID, out)) 
			WantedFile = File_name;
		else 
		{
			QWrite("0,[],[],[]]", out);
			free(File_name);
			break;
		};
		

	// Class path
	int J			= 0;	// J is current index
	int K			= -1;	// K is max
	char CLASSPATH;
	char *pch		= strtok(rawPath, "[,]");
	
	while (pch!=NULL  &&  K<10)	//from ofp array to char array
	{
		K++;
		char *item = stripq(pch);
		strncpy(classpath[K], Trim(item), 127);
		pch = strtok(NULL, "[,]");
	};


	// Separate property name and value
	if (pch = strchr(WantedToken,'=')) 
	{
		int pos				= pch - WantedToken;
		WantedToken[pos]	= '\0';
		WantedValue			= WantedToken + pos + 1;

		if (ArrDelete) 
			WantedValue = "";
	};

	WantedToken = Trim(WantedToken);


	// Replace array brackets
	int lval = strlen(WantedValue);

	if (WantedValue[0]=='['  &&  WantedValue[lval-1]==']') 
		WantedValue[0]		= '{', 
		WantedValue[lval-1] = '}';
	// ----------------------------------------------------------------



	// Parse text -----------------------------------------------------
	// Open file
	FILE *f = fopen(WantedFile, "r");
	if (!f) 
	{
		FWerror(7,errno,CommandID,WantedFile,"",0,0,out);
		QWrite("0]", out);
		free(File_name); 
		break;
	};


	// Find file size, allocate buffer
	fseek(f, 0, SEEK_END);

	char* buf	= 0;
	int bufsize = 0;
	int fsize	= ftell(f);
	buf			= new char[fsize + 70 + (strlen(com))];

	if (!buf)
	{
		FWerror(10,0,CommandID,"buf","",fsize+70+(strlen(com)),0,out);
		QWrite("0]", out);
		free(File_name); 
		fclose(f);
		break;
	};

	fseek(f, 0, SEEK_SET);


	// Create dynamic buffers for storing text
	char *line;				// current line from file
	char *lineNEW;			// used when reallocating "line"
	char *line2;			// modified text line for the writing buffer
	char *line2NEW;			// used when reallocating "line2"
	char *match;			// stores word that's being parsed
	char *matchNEW;			// used when reallocating "match"
	int error		= 2;	// stop iterating (error=2 is "property not found")
	int lineLen		= 512;	// length of the "line"
	int matchLen	= 100;	// length of the "match"

	// Allocate
	line	= (char*) malloc (lineLen);
	line2	= (char*) malloc (lineLen);
	match	= (char*) malloc (matchLen);

	char failedBuf[20]	= "";
	int failedBufL		= 0;

	if (line==NULL  ||  line2==NULL  ||  match==NULL)
	{
		if (line==NULL)	 {error=1; strcat(failedBuf,"line "); failedBufL=+lineLen;};
		if (line2==NULL) {error=1; strcat(failedBuf,"line2 "); failedBufL+=lineLen;};
		if (match==NULL) {error=1; strcat(failedBuf,"match"); failedBufL+=matchLen;};

		FWerror(10,0,CommandID,failedBuf,"",failedBufL,0,out);	
		QWrite("0]", out);
		free(File_name);
 		free(line);
		free(line2);
		free(match);
		fclose(f);
		break;
	}
	else
		strcpy(line, ""),
		strcpy(line2, ""),
		strcpy(match, "");
		

	// Variables:
	char *ret			= "";		// return value when reading line from text
	int l				= 0;		// length of the current text line
	int lastPos			= 0;		// remember position (in bytes) in the text file in case we need to revert
	int level			= 0;		// inside how many brackets we currently are
	int foundTokenPos	= -1;		// remember position of the wanted property
	int ARRcurrent		= -1;		// index of the item in the current array
	int arrayLev		= 0;		// inside how many brackets we are in an array
	bool foundToken		= false;	// fount the property that user wanted
	bool commentBlock	= false;	// indicate that current text is commented so ignore it
	bool macroBlock		= false;	// indicate that we're in a function-like macro so ignore it
	bool jobDone		= false;	// just copy rest of the file
	bool reached		= false;	// found the class that user wanted
	bool inQuote		= false;	// indicate that we're in a string so ignore control characters
	bool classAbove		= false;	// we're going through a sub-class in a target class so skip it's contents
	bool inArray		= false;	// we're going through property value that is an array
	bool value			= false;	// indicate that we're on the right side of the equality sign
	bool remove			= false;	// we're skipping property that user wanted to remove
	bool overwriteArray = false;	// if array spans trough multiple lines then skip those lines
	bool ARRreplaced	= false;	// if we did replace item in an array
	bool incJ			= false;	// increment J if stumbled upon class that was defined in the class path
	bool anticipateArray= false;	// don't end property on \n, wait instead for opening bracket

	// For properties outside of any class
	if (K == -1) 
		reached = 1;


	// For each line in a text file -----------------------------------
	while (ret = fgets(line, lineLen ,f))
	{   
		if (ferror(f)) 
			FWerror(7,errno,CommandID,WantedFile,"",0,0,out), 
			error = 1;

		if (error==1  ||  error>=3) 
			break;


		// If a long line then reallocate buffer and read again
		l = strlen(line);

		if (line[l-1]!='\n'  &&  !feof(f)  &&  ret!=NULL)
		{
			lineLen  += 512;
			lineNEW  = (char*) realloc(line, lineLen);
			line2NEW = (char*) realloc(line2, lineLen);

			if (lineNEW != NULL) 
				line = lineNEW; 
			else 
			{
				FWerror(11,0,CommandID,"line","",lineLen,0,out);
				error = 1; 
				break;
			};

			if (line2NEW != NULL) 
				line2 = line2NEW; 
			else
			{
				FWerror(11,0,CommandID,"line2","",lineLen,0,out);  
				error = 1; 
				break;
			};

			fseek (f, lastPos, SEEK_SET);
			continue;
		};
		lastPos = ftell(f);


		// If we don't have to search/modify anymore then just copy rest of the file
		if (jobDone) 
		{
			memcpy(buf+bufsize, line, l+1); 
			bufsize += l; 
			continue;
		};


		// Search for crucial characters
		bool bracketL = strchr(line, '{') != NULL;
		bool bracketR = strchr(line, '}') != NULL;
		bool asterisk = strchr(line, '*') != NULL;
		bool equality = strchr(line, '=') != NULL;
		bool isClass  = false;


		// Search for classes in a line; save pos of every occurence to array
		char *res;
		char *lineTMP	= line;
		int I			= 0;
		int CurrentCOL	= 0;
		int classPOS[MAX_CLASSESINASINGLELINE];
		
		while (res = strstr(lineTMP, "class "))
		{
			if (I >= MAX_CLASSESINASINGLELINE)	// reached limit
				break;

			isClass		= true;
			int pos		= (res - lineTMP);
			classPOS[I] = pos + CurrentCOL;
			I++;
			pos			+= 6;
			CurrentCOL	+= pos;
			lineTMP		= line + CurrentCOL;
		};


		// If in the line there isn't occurence we want then skip
		if (!bracketL && !bracketR && !isClass && !asterisk && !equality && !reached) 
		{
			memcpy(buf+bufsize, line, l+1); 
			bufsize += l; 
			continue;
		};


		// Go one by one char
		bool between	= false;	// if there is more than one property in the line then indicate that we've finished current one
		bool firstChar	= true;		// indicate that we've hit first alfanum char after whitespace
		bool cont		= false;	// order to use 'continue' command

		for (int i=0; i<l; i++)
		{
			// Check for comments
			if (!inQuote)
			{
				if (!inQuote  &&  line[i]=='/'  &&  line[i+1]=='/') // single line comment - skip to the end of the line
					i = l - 1;

				if (!inQuote  &&  line[i]=='/'  &&  line[i+1]=='*') // entering comment block
					commentBlock = true;

				if (i>0  &&  !inQuote  &&  line[i-1]=='*'  &&  line[i]=='/') // leaving comment block
				{
					commentBlock = false; 
					continue;
				};

				if (commentBlock  &&  line[i]!='\n')	// ignore text in the comment block
					continue;
			};


			// Check for preprocessor
			if (line[i]=='#'  &&  firstChar) 
			{
				firstChar = 0; 	

				if(line[l-2] == '\\')
					macroBlock = 1;
				
				break;
			
			};

			if (!isspace(line[i])  &&  firstChar) 
				firstChar = false;

			if (macroBlock) 
			{
				if (line[l-2] != '\\') 
					macroBlock = 0;
				
				break;
			};


			// Ignore special characters if we're in quotation
			if (line[i] == '"') 
				inQuote =! inQuote;

			if (!inQuote)
			{
				// If a bracket
				if (line[i]=='{'  &&  value)	// opening array
					anticipateArray = false, 
					inArray			= true;

				if (line[i]=='{'  &&  !value)	// opening class
				{
					if (incJ)
						J++,
						incJ = 0;
					
					level++;
				};

				if (line[i]=='}'  &&  !value)	// closing class
					level--;

				if (line[i]=='{'  &&  value)	// opening array
					arrayLev++;

				if (line[i]=='}'  &&  value)	// closing array
					arrayLev--;

				if (J>K  &&  !reached  &&  level==K+1)	// if reached desired class
					reached = true;     

				if (level<J  &&  !reached) // couldn't find wanted class
				{
					error = 3; 
					break;
				};

				if (J>K  &&  reached  &&  level<K+1) // if left desired class
				{
					jobDone = true;

					// If not add/overwrite OR if found the value
					if (action!=1  &&  action!=4  ||  foundTokenPos>=0) 
						break;  

					// Copy anything that's before the closing bracket;
					strcpy(line2, line);
					bool notEmpty = 0;

					for (int y=0; y<i; y++) 
						if (!isspace(line[y]))
						{
							notEmpty = 1;
							break;
						};

					if (line2[i] == '}') 
						line2[i] = '\0';

					if (line2[i+1] == ';') 
						i++,
						line2[i+1] = '\0';

					int l2 = strlen(line2);
					memcpy(buf+bufsize, line2, l2+1);
					bufsize += l2;

					// If given property value is empty then replace it with semi-colon
					char empty[] = ";";

					if (strcmp(WantedValue,"") == 0)  
						WantedValue = empty;

					// Add new property
					char tabs[10] = "";

					for (int z=0; z<level; z++) 
						strcat(tabs, "\t");

					if (between  ||  notEmpty)	// add a new line
						strcpy(line2, "\n"); 
					else 
						strcpy(line2, "");

					// Add token name
					sprintf(line2, "%s%s\t%s=", line2, tabs,WantedToken);

					l2 = strlen(line2);
					memcpy(buf+bufsize, line2, l2+1);
					bufsize += l2;
									
					// If an array then check if brackets are missing
					strcpy(line2, "");

					bool addEndingBracket   = false;
					bool addEndingSemiColon = false;
					
					if (WantedToken[strlen(WantedToken)-1] == ']'  &&  WantedToken[strlen(WantedToken)-2] == '[')
					{
						addEndingBracket   = true;
						addEndingSemiColon = true;
						
						if (WantedValue[0] != '{')
							strcat(line2, "{");
							
						for (int z=lval-1; z>0; z--)
						{
							if (WantedValue[z] == ';')
							{
								addEndingSemiColon = false;
								continue;
							};
								
							if (!isspace(WantedValue[z]))
							{
								if (WantedValue[z] == '}')
									addEndingBracket = false;
								
								break;
							};
						}
					} else {
						if (WantedValue[lval-1]!=';')
							addEndingSemiColon = true;
					}
					
					// Add token value
					strcat(line2, WantedValue);
					
					if (addEndingBracket)
						strcat(line2, "}");
						
					if (addEndingSemiColon)
						strcat(line2, ";");
					
					sprintf(line2, "%s\n%s};\n", line2, tabs);

					l2 = strlen(line2);
					memcpy(buf+bufsize, line2, l2+1);
					bufsize += l2;

					// Add what was after the closing bracket;
					strcpy(line2, "");

					for (i++; i<l; i++) 
						sprintf(line2, "%s%c", line2,line[i]);

					strcpy(line, line2);
					error = 0; 
					break;
				};


				// if entered a sub-class inside desired class
				if (J>K  &&  reached  &&  level>K+1  &&  !classAbove)	
				{
					classAbove = true; 
					continue;
				}; 


				// when leaving sub-class inside desired class
				if (J>K  &&  reached  &&  classAbove  &&  level==K+1  &&  !isalnum(line[i])) 
					continue; 
				else 
					classAbove = false; 

				if ((line[i]=='{' || line[i]=='}')  &&  !value) 
					continue;


				// Check if we encountered a class name
				if (tolower(line[i])=='c'  &&  isClass)
				{
					// if array 'classPOS' contains the same index then it's a class
					for (int j=0; j<=I; j++)
					{
						char className[128] = "";

						if (i == classPOS[j])
						{
							// Get class name 
							for (int k=i+6,i3=0;  k<l && i3<128;  k++,i3++)
							{
								if (isspace(line[k])  ||  line[k]==':'  ||  line[k]=='{') 
									break;

								sprintf(className, "%s%c", className,line[k]);
							};

							// If reached class that was in passed arguments (classpath)
							if (K>=0  &&  J<=K  &&  level==J)
								if (strcmpi(classpath[J],className) == 0) 
									incJ = true;

							// Jump to the EOL or bracket opening of that class
							for (int i2=i;  i2<l;  i2++) 
								if (line[i2] == '{')
								{
									i2--;
									break;
								};

							i	 = i + i2 -i;
							cont = true;						 
						};
					};

					if (cont) 
					{
						cont = 0; 
						continue;
					};
				};
			};


			// Going trough class properties
			if ((equality || inArray)  &&  reached  &&  level==K+1)
			{
				// after finishing property don't start another one until hit alphanum character
				if (between  &&  !isalnum(line[i])) 
					continue; 
				else 
					between = false;

				// Purge 'match' if it's a word that ended without '='
				if (!inQuote  &&  line[i]==';'  &&  !inArray  &&  !value) 
					strcpy(match, "");

				// At end of value
				if (!inQuote  &&  (line[i]==';' && arrayLev==0 || line[i]=='\n' && !anticipateArray && !inArray || line[i]=='}' && !inArray)  &&  value)
				{
					inArray = false;
					value	= false;

					if (line[i] == ';') 
						between = true;

					// Replace value
					if (foundToken  &&  !overwriteArray  &&  action!=4  &&  action!=5)
					{
						// Save anything prior to the value
						strcpy(line2, "");

						for (int z=0; z<=foundTokenPos; z++) 
							sprintf(line2, "%s%c", line2,line[z]);

						if (line2[strlen(line2)-1]=='='  &&  line2[strlen(line2)-2]==' ') 
							strcat(line2, " ");

						int l2 = strlen(line2);
						memcpy(buf+bufsize, line2, l2+1);
						bufsize += l2;

						// Add property value
						sprintf(line2, "%s", WantedValue);
						l2 = strlen(line2);
						memcpy(buf+bufsize, line2, l2+1);
						bufsize += l2;

						// Copy anything that's after the property
						strcpy(line2, "");

						for (; i<l; i++) 
							sprintf(line2,"%s%c",line2,line[i]);

						strcpy(line, line2);

						foundToken  = 0; 
						error		= 0; 
						jobDone		= true; 
						break;
					};


					// Append value
					if (foundToken  &&  !overwriteArray  &&  action==4  &&  action!=5)
					{
						foundToken	= 0; 
						error		= 0; 
						jobDone		= true;
						bool array  = false;

						// Pull back behind separator
						for (; i>0; i--)
						{
							if (!isspace(line[i]))
							{
								if (line[i] == ';')
									continue;
								
								if (line[i] == '}')
									array = true;
									
								break;
							};
						};

						// Save anything prior to the separator
						strcpy(line2, "");

						for (int z=0; z<i; z++) 
							sprintf(line2, "%s%c", line2,line[z]);

						int l2 = strlen(line2);
						memcpy(buf+bufsize, line2, l2+1);
						bufsize += l2;

						// Add property value
						strcpy(line2, "");

						// Add comma (unless array is empty)
						if (array  &&  strcmp(WantedValue,"{}")!=0  &&  strcmp(WantedValue,"")!=0) 
							strcpy(line2, ",");

						strcat(line2, WantedValue);
						l2 = strlen(line2);
						memcpy(buf+bufsize, line2, l2+1);
						bufsize += l2;

						// Copy anything that's after the property
						strcpy(line2, "");

						for (; i<l; i++) 
							sprintf(line2,"%s%c",line2,line[i]);

						strcpy(line, line2);
						break;
					};

					// Finish removing / overwriting
					if (remove  ||  overwriteArray)
					{
						// Copy anything that's after the property
						strcpy(line2, "");

						if (remove  &&  between) // skip ; when removing
							i++;   

						for (; i<l; i++) 
							sprintf(line2, "%s%c", line2,line[i]);

						strcpy(line, line2);

						foundToken		= false; 
						overwriteArray	= false; 
						remove			= false; 
						error			= 0; 
						jobDone			= true; 
						break;
					};

					if (foundToken) 
						foundToken = 0;

					strcpy(match, "");
					continue;
				};


				// At end of the property name
				if (!inQuote  &&  line[i]=='=')
				{
					//if array bracket starts in the next line
					if (match[strlen(match)-1]==']'  &&  match[strlen(match)-2]=='[') 
						anticipateArray = true; 
                    

                    // If that's the one we're looking for
					if(strcmpi(match,WantedToken) == 0)
					{
						// if add/overwrite/replacesingleitem then rememember pos
						if (action==1  ||  action==4  ||  action==5) 
							foundToken		= 1, 
							foundTokenPos	= i; 

						// if rename
						if (action == 2)
						{
							// Save anything prior to the name
							strcpy(line2, "");
							int max = i - strlen(match);

							for (int z=0; z<max; z++) 
								sprintf(line2, "%s%c", line2,line[z]);

							int l2 = strlen(line2);
							memcpy(buf+bufsize, line2, l2+1);
							bufsize += l2;

							// Add new name
							l2 = strlen(RenameDst);
							memcpy(buf+bufsize, RenameDst, l2+1);
							bufsize += l2;

							// Copy anything that's after the class name
							strcpy(line2, "");

							for (; i<l; i++) 
								sprintf(line2, "%s%c", line2,line[i]);

							strcpy(line, line2);
							jobDone = true; 
							error	= 0; 
							break;
						};

						// if remove
						if (action == 3)
						{
							// Save anything prior to the name
							strcpy(line2, "");

							for (int z=0; z<foundTokenPos; z++) 
								sprintf(line2, "%s%c", line2,line[z]);

							int l2 = strlen(line2);
							memcpy(buf+bufsize, line2, l2+1);
							bufsize += l2; 

							remove = true;
						};
					};

					value = true;
					strcpy(match, "");
					continue;
				};


				// Save character to 'match' - building a property name
				if (!(isspace(line[i]) && !inQuote)  &&  !(value&&line[i]=='/')  &&  !(strlen(match)==0  &&  line[i]==';'))
				{
					// Append char
					sprintf(match, "%s%c", match, line[i]);

					// Reallocate if necessary
					if ((int)strlen(match) >= matchLen-2)
					{
						matchLen += 100;
						matchNEW = (char*) realloc (match, matchLen);

						if (matchNEW != NULL) 
							match = matchNEW; 
						else 
						{
							char tmp[] = "match";
							char *tmp2 = tmp;
							FWerror(11,0,CommandID,"match","",lineLen,0,out);
							error = 1; 
							break;
						}
					};
				};


				// Replace item in an array
				if (foundToken  &&  action==5  &&  inArray  &&  !isspace(line[i])  &&  !ARRreplaced  &&  ARRcurrent==ARRtargetID)
				{
					// Save anything prior to the separator
					strcpy(line2, "");

					for (int z=0; z<i; z++) 
						sprintf(line2, "%s%c", line2,line[z]);
					
					int lastIsQuote 		 = -1;							// when appending to string use this value to remove ending quote
					bool appendingToSubArray = ArrAppend  &&  line[i]=='{';	// are we appending to sub-array?

					// If we're appending then also save the current value
					if (ArrAppend)
					{
						bool inQuote2	= false;	// are we going through string item
						int tmpArrayLev	= arrayLev;	// count sub-arrays

						// Go through the value
						// Determine if it's a string (we need to remove last quotation mark)
						// or a sub-array (we need to remove closing bracket)
						for (int z=i; z<l; z++) 
						{
							if (!inQuote2  &&  line[z]=='{')
								tmpArrayLev++;

							if (!inQuote2  &&  line[z]=='}')
								tmpArrayLev--;

							if (line[z]=='"'  &&  tmpArrayLev==1) 
								lastIsQuote = z,
								inQuote2	=! inQuote2;
							else
								if (!isspace(line[z])  &&  (!inQuote2 && line[z]!=',')  &&  (line[z]!='}' && tmpArrayLev==1))
									lastIsQuote = -1;
									
							if (!inQuote2  &&  (line[z]==',' || line[z]=='}' || line[z]==';' && arrayLev>0)  &&  (appendingToSubArray && tmpArrayLev==arrayLev  ||  !appendingToSubArray)) 
							{
								if (lastIsQuote >= 0)
									line2[lastIsQuote] = '\0';

								break;
							};

							sprintf(line2, "%s%c", line2,line[z]);
						};
					};

					if (appendingToSubArray)
						strcat(line2, ",");

					int l2 = strlen(line2);
					memcpy(buf+bufsize, line2, l2+1);
					bufsize += l2;

					// Add property value
					strcpy(line2, WantedValue);

					if (lastIsQuote >= 0)
						strcat(line2, "\"");

					if (appendingToSubArray)
						strcat(line2, "}");

					l2 = strlen(line2);
					memcpy(buf+bufsize, line2, l2+1);
					bufsize += l2;

					ARRreplaced = true;
				};


				// After element has been replaced
				if (foundToken  &&  action==5  &&  ARRreplaced && (((line[i]==',' || line[i]==';') && arrayLev==1) && ARRcurrent>=ARRtargetID || line[i]=='}' && arrayLev==0 && ARRcurrent==ARRtargetID))
				{
					strcpy(line2, "");

					// if removing then skip one comma
					if (line[i]==','  &&  strcmp(WantedValue,"")==0) 
						i++;

					for (; i<l; i++) 
						sprintf(line2, "%s%c", line2,line[i]);

					strcpy(line, line2);

					foundToken	= 0; 
					error		= 0; 
					jobDone		= true; 
					break;
				};


				// If hit item in the array
				if (foundToken  &&  !inQuote  &&  inArray  &&  line[i]=='{'  &&  foundToken  &&  arrayLev==1) 
					ARRcurrent = 0;

				if (foundToken  &&  !inQuote  &&  inArray  &&  ((line[i]==',' || line[i]==';') && arrayLev==1)  &&  foundToken) 
					ARRcurrent++;
			};
		};


		// Finished going through line
		// If found property but there wasn't end of value
		// Means we're overwriting array
		if (foundToken  &&  value  &&  !overwriteArray  &&  action==1) 
		{
			// Save anything prior to the value
			strcpy(line2, "");

			for (int z=0; z<=foundTokenPos; z++) 
				sprintf(line2, "%s%c", line2,line[z]);

			int l2 = strlen(line2);
			memcpy(buf+bufsize, line2, l2+1);
			bufsize += l2;

			// Add property value
			sprintf(line2, "%s", WantedValue);

			l2 = strlen(line2);
			memcpy(buf+bufsize, line2, l2+1);
			bufsize += l2;

			overwriteArray = true;
		};


		// Don't save to buffer things that we want to remove
		if (remove  ||  overwriteArray) 
			continue;


		// If we replaced item in array but it didn't end with comma or bracket      
		if (ARRreplaced  &&  !jobDone) 
			strcpy(line, "\n");


		l = strlen(line);
		memcpy(buf+bufsize, line, l+1);
		bufsize += l;
	};
	// ----------------------------------------------------------------
	fclose(f);


	if (error != 1)
	{
		// If couldn't find classes that were in the class path
		if (J <= K)
			FWerror(250,0,CommandID,WantedFile,classpath[J],J,++K,out);
		else
			// Add new global property
			if (K==-1  &&  foundTokenPos<0  &&  (action==1 || action==4))
			{
				// If given property value is empty then replace it with semi-colon
				char empty[] = ";";

				if (strcmp(WantedValue,"") == 0)  
					WantedValue = empty;

				sprintf(line2, "\n%s=%s\n", WantedToken,WantedValue);

				int l = strlen(line2);
				memcpy(buf+bufsize, line2, l+1);
				bufsize += l;

				error = 0;
			}
		else
			// Couldn't find...
			if (error == 2) 
			{
				// ...an item inside array
				if (action==5  &&  ARRcurrent>0  &&  ARRcurrent<ARRtargetID)	
					FWerror(254,0,CommandID,WantedFile,WantedToken,ARRtargetID,0,out);
				else
					 // not an array
					if (action==5 && foundTokenPos>=0 && ARRcurrent<0)     
						FWerror(255,0,CommandID,WantedFile,WantedToken,0,0,out);
					else
						// ...a property
						FWerror(253,0,CommandID,WantedFile,WantedToken,0,0,out);	
			};

		// Rewrite the file
		if (!error)
		{	
			f = fopen(WantedFile, "w");
			if (f) 
			{
				fwrite(buf, 1, strlen(buf), f);

				if (ferror(f)) 
					FWerror(7,errno,CommandID,WantedFile,"",0,0,out);
				else
					FWerror(0,0,CommandID,"","",0,0,out);

				fclose(f);
			}
			else
				FWerror(7,errno,CommandID,WantedFile,"",0,0,out);
		};
	};


	char tmp[14] = ""; 
	sprintf(tmp, "%d]",J);
	QWrite(tmp, out);


	delete[] buf; 
	free(File_name); 
	free(line); 
	free(line2); 
	free(match);
}
break;
