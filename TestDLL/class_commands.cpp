// -----------------------------------------------------------------
// CLASS OPERATIONS
// -----------------------------------------------------------------

#define MAX_CLASSESINASINGLELINE 128
#define CLASSPATH classpath[10][128]

case C_CLASS_LIST:
{  // Return list of classes in a file

	char *arg_filename      = empty_string;
	char *raw_path          = empty_string;
	int arg_filename_length = 0;
	int start_pos           = 0;
	bool start_pos_set      = false;

	for (size_t i=2; i<argument_num; i+=2) {
		switch (argument_hash[i]) {
			case NAMED_ARG_FILE :
				arg_filename        = argument[i+1];
				arg_filename_length = argument_length[i+1];
				break;

			case NAMED_ARG_CLASSPATH :
				raw_path = stripq(argument[i+1]);
				break;

			case NAMED_ARG_OFFSET :
				start_pos_set = true;
				start_pos     = atoi(argument[i+1]);
				break;
		}
	}
	

	// File not specified
	if (arg_filename_length == 0) {
		QWrite_err(FWERROR_PARAM_EMPTY, 1, "arg_filename");
		QWrite("0,[],[],[]]");
		break;
	}


	// Verify and update path to the file
	String buf_filename;
	String_init(buf_filename);
	char *ptr_filename = arg_filename;

	if (!VerifyPath(&ptr_filename, buf_filename, OPTION_ALLOW_GAME_ROOT_DIR)) {
		QWrite("0,[],[],[]]");
		break;
	}


	// Class path
	int J			= 0;	// J is current index
	int K			= -1;	// K is max
	char CLASSPATH;
	char *pch2		= strtok(raw_path, "[,]");

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
	FILE *f = fopen(ptr_filename, "r");
	if (!f) 
	{
		QWrite_err(FWERROR_ERRNO, 2, errno, ptr_filename);
		QWrite("0,[],[],[]]");
		String_end(buf_filename);
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
	int lineLen		= 1024;	// length of the "line"
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
		if (!line)		{strcat(failedBuf,"line "); failedBufL+=matchLen;};
		if (!match)		{strcat(failedBuf,"match"); failedBufL+=offLen;};
		if (!classes)	{strcat(failedBuf,"classes "); failedBufL+=classesLen;};
		if (!inherits)	{strcat(failedBuf,"inherits "); failedBufL+=inheritsLen;};
		if (!offset)	{strcat(failedBuf,"offset "); failedBufL+=lineLen;};
		
		QWrite_err(FWERROR_MALLOC, 2, failedBuf, failedBufL);
		QWrite("0,[],[],[]]");
		String_end(buf_filename);
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
		strcpy(classes,""),
		strcpy(inherits,""),
		strcpy(offset,"");
		

	// Variables:
	char *ret			= NULL;		// return value when reading line from text
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
	if (start_pos_set && K>=0) {
		fseek(f, start_pos, SEEK_SET);
		lastPos = start_pos;
		J		= K;
		level	= J;
	}


	// For each line in a text file -----------------------------------
	while((ret = fgets(line, lineLen ,f)))
	{
		if (ferror(f)) {
			QWrite_err(FWERROR_ERRNO, 2, errno, ptr_filename);
			error = true;
		}

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
				QWrite_err(FWERROR_REALLOC, 2, "line", lineLen);
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

		while ((res = strstr(lineTMP, "class ")))
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

			if ((J>K  &&  reached  &&  level<K+1)  ||  level<J)  // passed through the wanted class - stop
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

						if (classesNEW)
						{
							classes = classesNEW;
							strcat(classes,"]+[\""); 
							strcat(classes, match); 
							strcat(classes,"\"");
						}
						else 
						{
							QWrite_err(FWERROR_REALLOC, 2, "classes", classesLen);
							error = true; 
							break;
						};
						
						// Reallocate buffer, append position of the class (in bytes)
						char tmp[16] = "";
						sprintf(tmp,"]+[\"%d\"",classOff);
						offLen		= offLen + strlen(tmp);
						offsetNEW	= (char*) realloc(offset, offLen);

						if (offsetNEW) 
							offset = offsetNEW, 
							strcat(offset, tmp);
						else
						{
							QWrite_err(FWERROR_REALLOC, 2, "offset", offLen);
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

						if (inheritsNEW)
						{
							inherits = inheritsNEW;
							strcat(inherits,"]+[\""); 
							strcat(inherits, match); 
							strcat(inherits,"\"");
						}
						else
						{
							QWrite_err(FWERROR_REALLOC, 2, "inherits", inheritsLen);
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

						if (matchNEW) 
							match = matchNEW; 
						else 
						{
							QWrite_err(FWERROR_REALLOC, 2, "match", matchLen);
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

	if (!error) {
		// If couldn't find classes that were in the class path
		if (J <= K)	{
			QWrite_err(FWERROR_CLASS_PARENT, 4, classpath[J], J, ++K, ptr_filename);
			QWritef("%d,[],[],[]]", J);
		} else {
			// No issues - output data
			QWrite_err(FWERROR_NONE, 0);
			QWritef("%d,[%s],[%s],[%s]]", J, classes, inherits, offset);
		}
	} else
		// If error - error info was already passed; now empty arrays
		QWritef("%d,[],[],[]]", J);

	String_end(buf_filename);
	free(line); 
	free(match); 
	free(classes); 
	free(inherits); 
	free(offset);
};
break;












case C_CLASS_TOKEN:
{ // Return all properties from a class

	// Read arguments -------------------------------------------------
	char *arg_filename	    = empty_string;
	char *raw_path          = empty_string;
	char *Target		    = empty_string;
	char *Wrap			    = empty_string;
	bool NoWrap			    = false;
	bool NoDoubleWrap	    = false;
	bool start_pos_set      = false;
	int  start_pos          = 0;
	int arg_filename_length = 0;

	// Parse arguments
	for (size_t i=2; i<argument_num; i+=2) {
		switch (argument_hash[i]) {
			case NAMED_ARG_FILE :
				arg_filename        = argument[i+1];
				arg_filename_length = argument_length[i+1];
				break;

			case NAMED_ARG_CLASSPATH :
				raw_path = stripq(argument[i+1]);
				break;

			case NAMED_ARG_OFFSET :
				start_pos_set = true;
				start_pos     = atoi(argument[i+1]);
				break;

			case NAMED_ARG_TOKEN :
				Target = argument[i+1];
				break;

			case NAMED_ARG_WRAP :
				Wrap = argument[i+1];
				break;
		}
	}


	// File not specified
	if (arg_filename_length == 0) {
		QWrite_err(FWERROR_PARAM_EMPTY, 1, "arg_filename");
		QWrite("0,\"0\",[],[]]");
		break;
	};


	// Wrapping options
	if (strcmpi(Wrap,"no") == 0)
		NoWrap		 = true,
		NoDoubleWrap = false;

	if (strcmpi(Wrap,"nodouble") == 0)
		NoWrap		 = false,
		NoDoubleWrap = true;


	// Verify and update path to the file
	String buf_filename;
	String_init(buf_filename);
	char *ptr_filename = arg_filename;

	if (!VerifyPath(&ptr_filename, buf_filename, OPTION_ALLOW_GAME_ROOT_DIR)) {
		QWrite("0,\"0\",[],[]]");
		break;
	}


	// Class path
	int J			= 0;	// J is current index, 
	int K			= -1;	// K is max
	char CLASSPATH;
	char *pch		= strtok(raw_path, "[,]");

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
	FILE *f = fopen(ptr_filename, "r");
	if (!f) 
	{
		QWrite_err(FWERROR_ERRNO, 2, errno, ptr_filename);
		QWrite("0,\"0\",[],[]]");
		String_end(buf_filename); 
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
	int lineLen		= 1024;		// length of the "line"
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
		if (!line)   {strcat(failedBuf,"line "); failedBufL+=lineLen;};
		if (!match)  {strcat(failedBuf,"match"); failedBufL+=matchLen;};		
		if (!names)  {strcat(failedBuf,"names "); failedBufL+=namesLen;};
		if (!values) {strcat(failedBuf,"values "); failedBufL+=valuesLen;};

		QWrite_err(FWERROR_MALLOC, 2, failedBuf, failedBufL);
		QWrite("0,\"0\",[],[]]");
		String_end(buf_filename); 
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
		strcpy(names, ""),
		strcpy(values, "");
				
		
	// Variables:
	char *ret			= NULL;		// return value when reading line from text
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
	if (start_pos_set && K>=0) {
		fseek(f, start_pos, SEEK_SET);
		lastPos = start_pos;
		J		= K;
		level	= J;
	}


	// For each line in a text file -----------------------------------
	while ((ret = fgets(line, lineLen ,f)))
	{	
		if (ferror(f)) {
			QWrite_err(FWERROR_ERRNO, 2, errno, ptr_filename);
			error = true;
		}

		if (quit || error) 
			break;


		// If a long line then reallocate buffer and read again
		l = strlen(line);

		if (line[l-1]!='\n'  &&  !feof(f)  &&  ret!=NULL)      // If long line
		{
			lineLen += 512;
			lineNEW = (char*) realloc(line, lineLen);

			if (lineNEW) 
				line = lineNEW; 
			else 
			{	
				QWrite_err(FWERROR_REALLOC, 2, "line", lineLen);
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

		while ((res = strstr(lineTMP, "class ")))
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

				if ((J>K  &&  reached  &&  level<K+1)  ||  level<J) // passed through the wanted class - stop
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
				if (!inQuote  &&  ((line[i]==';' && arrayLev==0) || (line[i]=='\n' && !anticipateArray && !inArray) || (line[i]=='}' && !inArray))  &&  value)
				{
					if (valLen==0  &&  !thatWasArray) 
						strcat(match,"\"");	// quote wasn't added on the start if it's empty value

					thatWasArray = false;

					if (!inArray) 
						if ((!NoWrap  &&  !(NoDoubleWrap && startsWithQuote))  ||  valLen==0)
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
								if ((match[g]=='['  &&  match[g+1]=='\"'  &&  match[g+2]=='\"'  &&  match[g+3]=='\"')  || 
									(match[g]=='\"'  &&  match[g+1]=='\"'  &&  match[g+2]=='\"'  &&  match[g+3]==']'))
										match[g+1] = ' ',
										match[g+2] = ' ';
							};
						};
									
						// Reallocate buffer, append property value
						valuesLen = valuesLen + strlen(match) + 4;
						valuesNEW = (char*) realloc(values, valuesLen);

						if (valuesNEW)
						{
							values = valuesNEW;
							strcat(values, "]+[");
							strcat(values, match);
						}
						else 
						{
							QWrite_err(FWERROR_REALLOC, 2, "values", valuesLen);
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

					if (strcmp(match,"")==0  ||  (Search  &&  strcmpi(match,Target)!=0)) 
						dontcopy = true; 
					else 
					{
						dontcopy = false;

						// Reallocate buffer, append new class name
						namesLen = namesLen + strlen(match) + 7;
						namesNEW = (char*) realloc(names, namesLen);

						if (namesNEW)
						{
							names = namesNEW;
							strcat(names, "]+[\""); 

							// double the amount of quotation marks in the property name
							if (strchr(match,'\"'))
							{
								char *rep = str_replace(match,"\"","\"\"",OPTION_NONE);

								if (!rep) 
								{
									QWrite_err(FWERROR_STR_REPLACE, 2, "part (' with '')", strlen(match));
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
							QWrite_err(FWERROR_REALLOC, 2, "names", namesLen);
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

						if (matchNEW) 
							match = matchNEW; 
						else 
						{
							QWrite_err(FWERROR_REALLOC, 2, "match", matchLen);
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


	if (!error) {
		// If couldn't find classes that were in the class path
		if (J <= K) {
			QWrite_err(FWERROR_CLASS_PARENT, 4, classpath[J], J, ++K, ptr_filename);
			QWritef("%d,\"%d\",[],[]]", J, classOff);
		} else {
			// No issues - output data
			QWrite_err(FWERROR_NONE, 0);
			QWritef("%d,\"%d\",[%s],[%s]]", J, classOff, names, values);
		}
	} else
		// If error - error info was already passed; give empty arrays to the game
		QWritef("%d,\"%d\",[],[]]", J, classOff);

	String_end(buf_filename);
	free(names); 
	free(values); 
	free(line); 
	free(match); 
}
break;












case C_CLASS_MODIFY:
{ // Modify class name in a file

	// Read arguments -------------------------------------------------
	int action              = 0;
	int arg_filename_length = 0;
	char *arg_filename      = empty_string;
	char *raw_path          = empty_string;
	char *Target            = empty_string;
	char *RenameDst         = empty_string;


	// Parse arguments
	for (size_t i=2; i<argument_num; i+=2) {
		switch (argument_hash[i]) {
			case NAMED_ARG_FILE :
				arg_filename        = argument[i+1];
				arg_filename_length = argument_length[i+1];
				break;

			case NAMED_ARG_CLASSPATH :
				raw_path = stripq(argument[i+1]);
				break;

			case NAMED_ARG_ADD :
				action = 1;
				Target = argument[i+1];
				break;

			case NAMED_ARG_RENAME :
				action = 2;
				Target = argument[i+1];
				break;

			case NAMED_ARG_TO :
				RenameDst = argument[i+1];
				break;

			case NAMED_ARG_DELETE :
				action = 3;
				Target = argument[i+1];
				break;
		}
	}


	// File not specified
	if (arg_filename_length == 0) {
		QWrite_err(FWERROR_PARAM_EMPTY, 1, "arg_filename");
		QWrite("0]");
		break;
	};


	// If action was not determined
	if (action == 0)
	{
		QWrite_err(FWERROR_PARAM_ACTION, 0);
		QWrite("0]");
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

		QWrite_err(FWERROR_PARAM_EMPTY, 1, tmp);
		QWrite("0]");
		break;
	};


	// Verify and update path to the file
	String buf_filename;
	String_init(buf_filename);
	char *ptr_filename = arg_filename;
	
	if (!VerifyPath(&ptr_filename, buf_filename, OPTION_RESTRICT_TO_MISSION_DIR)) {
		QWrite("0]");
		break;
	}


	// Class path
	int J			= 0;	// J is current index
	int K			= -1;	// K is max
	char CLASSPATH; 
	char *pch		= strtok(raw_path, "[,]");

	while (pch!=NULL  &&  K<10)	//from ofp array to char array
	{
		K++;
		char *item = stripq(pch);
		strncpy(classpath[K], Trim(item), 127);
		pch = strtok(NULL, "[,]");
	};


	// Split argument into classname and inherit
	char *WantedClass	= Target; 
	char *inherit		= NULL;

	if ((pch = strchr(WantedClass, ':')))
	{
		int pos			 = pch - WantedClass;
		inherit			 = WantedClass + pos + 1;
		WantedClass[pos] = '\0';
	};

	WantedClass = Trim(WantedClass);
	inherit		= Trim(inherit);
	// ----------------------------------------------------------------


	// Open file ------------------------------------------------------
	FILE *f = fopen(ptr_filename, "r");
	if (!f) 
	{
		QWrite_err(FWERROR_ERRNO, 2, errno, ptr_filename);
		QWrite("0]");
		String_end(buf_filename); 
		break;
	};


	// Find file size, allocate buffer
	fseek(f, 0, SEEK_END);

	char *buf	 = 0;
	int bufsize  = 0;
	int fsize	 = ftell(f);
	size_t buf_size = fsize + 70;
	for (size_t z=0;z<argument_num;z++)buf_size+=argument_length[z];
	buf			 = new char[buf_size];

	if (!buf)
	{
		QWrite_err(FWERROR_MALLOC, 2, "buf", buf_size);
		QWrite("0]");
		String_end(buf_filename); 
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
	int lineLen		= 1024;	// length of the "line"
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

		QWrite_err(FWERROR_MALLOC, 2, failedBuf, failedBufL);
		QWrite("0]");
		String_end(buf_filename);
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
	char *ret			= NULL;		// return value when reading line from text
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
	while((ret = fgets(line, lineLen ,f)))
	{
		if (ferror(f)) {
			QWrite_err(FWERROR_ERRNO, 2, errno, ptr_filename);
			error = 1;
		}

		if (error) 
			break;


		// If a long line then reallocate buffer and read again
		l = strlen(line);

		if (line[l-1]!='\n'  &&  !feof(f)  &&  ret!=NULL)
		{
			lineLen  += 512;
			lineNEW  = (char*) realloc(line, lineLen);
			line2NEW = (char*) realloc(line2, lineLen);

			if (lineNEW) 
				line = lineNEW; 
			else 
			{
				QWrite_err(FWERROR_REALLOC, 2, "line", lineLen);
				error = 1; 
				break;
			};

			if (line2NEW) 
				line2 = line2NEW; 
			else
			{
				QWrite_err(FWERROR_REALLOC, 2, "line2", lineLen); 
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

		while ((res = strstr(lineTMP, "class ")))
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

						if (matchNEW) 
							match = matchNEW; 
						else 
						{
							QWrite_err(FWERROR_REALLOC, 2, "match", lineLen);
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
			QWrite_err(FWERROR_CLASS_PARENT, 4, classpath[J], J, ++K, ptr_filename);
		else 
			// Couldn't remove/rename global class
			if (action>1  &&  !jobDone) 
				QWrite_err(FWERROR_CLASS_NOCLASS, 2, WantedClass, ptr_filename);
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
			f = fopen(ptr_filename, "w");
			if (f) 
			{
				fwrite(buf, 1, strlen(buf), f);

				if (ferror(f)) 
					QWrite_err(FWERROR_ERRNO, 2, errno, ptr_filename);
				else
					QWrite_err(FWERROR_NONE, 0);

				fclose(f);
			}
			else 
				QWrite_err(FWERROR_ERRNO, errno, ptr_filename);
		};
	};
	

	// Class already exists
	if (error == 2) 
		QWrite_err(FWERROR_CLASS_EXISTS, 2, WantedClass, ptr_filename);


	// Couldn't find class
	if (error == 3) 
		QWrite_err(FWERROR_CLASS_NOCLASS, 2, WantedClass, ptr_filename);


	// Output position in the class path
	QWritef("%d]", J);


	delete[] buf; 
	String_end(buf_filename); 
	free(line); 
	free(line2); 
	free(match);
}
break;












case C_CLASS_MODTOK:
{ // Modify property within a class

	// Read arguments -------------------------------------------------
	int action              = 0; 
	int ARRtargetID         = -1;
	int ArrAppend           = 0;
	int ArrDelete           = 0;
	int arg_filename_length = 0;
	char *arg_filename      = empty_string;
	char *raw_path          = empty_string;
	char *WantedToken       = empty_string;
	char *WantedValue       = empty_string;
	char *RenameDst         = empty_string;


	// Parse arguments
	for (size_t i=2; i<argument_num; i+=2) {
		switch (argument_hash[i]) {
			case NAMED_ARG_FILE :
				arg_filename        = argument[i+1];
				arg_filename_length = argument_length[i+1];
				break;

			case NAMED_ARG_CLASSPATH :
				raw_path = stripq(argument[i+1]);
				break;

			case NAMED_ARG_ADD : 		// add/overwrite
				action      = 1; 
				WantedToken = argument[i+1];
				break;

			case NAMED_ARG_APPEND : 	// append
				action      = 4; 
				WantedToken = argument[i+1];
				break;

			case NAMED_ARG_RENAME :		// rename source
				action      = 2; 
				WantedToken = argument[i+1];
				break;

			case NAMED_ARG_TO :			// rename destination
				RenameDst = argument[i+1];
				break;

			case NAMED_ARG_DELETE : 	// remove property 
				action      = 3;
				WantedToken = argument[i+1];
				break;

			case NAMED_ARG_INDEX :		// modify item inside array
				if (action != 2) { 
					if (action == 3) 
						ArrDelete = 1;

					if (action == 4) 
						ArrAppend = 1;

					action      = 5; 
					ARRtargetID = atoi(argument[i+1]);
				} break;
		}
	}


	// File not specified
	if (arg_filename_length == 0) {
		QWrite_err(FWERROR_PARAM_EMPTY, 1, "WantedFile");
		QWrite("0]");
		break;
	};


	// If action was not determined
	if (action==0  ||  (action==5  &&  strcmp(WantedToken,"")==0))
	{
		QWrite_err(FWERROR_PARAM_ACTION, 0);
		QWrite("0]");
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

		QWrite_err(FWERROR_PARAM_EMPTY, 1, tmp);
		QWrite("0]");
		break;
	};


	// Incorrect array index
	if (action==5  &&  ARRtargetID<0)
	{
		QWrite_err(FWERROR_PARAM_LTZERO, 2, "ArrtargetID", ARRtargetID);
		QWrite("0]");
		break;
	};


	// Verify and update path to the file
	String buf_filename;
	String_init(buf_filename);
	char *ptr_filename = arg_filename;
	
	if (!VerifyPath(&ptr_filename, buf_filename, OPTION_RESTRICT_TO_MISSION_DIR)) {
		QWrite("0]");
		break;
	}
		

	// Class path
	int J			= 0;	// J is current index
	int K			= -1;	// K is max
	char CLASSPATH;
	char *pch		= strtok(raw_path, "[,]");
	
	while (pch!=NULL  &&  K<10)	//from ofp array to char array
	{
		K++;
		char *item = stripq(pch);
		strncpy(classpath[K], Trim(item), 127);
		pch = strtok(NULL, "[,]");
	};


	// Separate property name and value
	if ((pch = strchr(WantedToken,'='))) 
	{
		int pos				= pch - WantedToken;
		WantedToken[pos]	= '\0';
		WantedValue			= WantedToken + pos + 1;

		if (ArrDelete) 
			WantedValue = NULL;
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
	FILE *f = fopen(ptr_filename, "r");
	if (!f) 
	{
		QWrite_err(FWERROR_ERRNO, 2, errno, ptr_filename);
		QWrite("0]");
		String_end(buf_filename); 
		break;
	};


	// Find file size, allocate buffer
	fseek(f, 0, SEEK_END);

	char *buf	 = 0;
	int bufsize  = 0;
	int fsize	 = ftell(f);
	size_t buf_size = fsize + 70;
	for (size_t z=0;z<argument_num;z++)buf_size+=argument_length[z];
	buf = new char[buf_size];

	if (!buf)
	{
		QWrite_err(FWERROR_MALLOC, 2, "buf", buf_size);
		QWrite("0]");
		String_end(buf_filename); 
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
	int lineLen		= 1024;	// length of the "line"
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

		QWrite_err(FWERROR_MALLOC, 2, failedBuf, failedBufL);	
		QWrite("0]");
		String_end(buf_filename);
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
	char *ret			= NULL;		// return value when reading line from text
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
	while ((ret = fgets(line, lineLen ,f)))
	{   
		if (ferror(f)) {
			QWrite_err(FWERROR_ERRNO, 2, errno, ptr_filename);
			error = 1;
		}

		if (error==1  ||  error>=3) 
			break;


		// If a long line then reallocate buffer and read again
		l = strlen(line);

		if (line[l-1]!='\n'  &&  !feof(f)  &&  ret!=NULL)
		{
			lineLen  += 512;
			lineNEW  = (char*) realloc(line, lineLen);
			line2NEW = (char*) realloc(line2, lineLen);

			if (lineNEW) 
				line = lineNEW; 
			else 
			{
				QWrite_err(FWERROR_REALLOC, 2, "line", lineLen);
				error = 1; 
				break;
			};

			if (line2NEW) 
				line2 = line2NEW; 
			else
			{
				QWrite_err(FWERROR_REALLOC, 2, "line2", lineLen);  
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
		
		while ((res = strstr(lineTMP, "class ")))
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
					if ((action!=1  &&  action!=4)  ||  foundTokenPos>=0) 
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
				if (!inQuote  &&  ((line[i]==';' && arrayLev==0) || (line[i]=='\n' && !anticipateArray && !inArray) || (line[i]=='}' && !inArray))  &&  value)
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

						if (matchNEW) 
							match = matchNEW; 
						else 
						{
							char tmp[] = "match";
							char *tmp2 = tmp;
							QWrite_err(FWERROR_REALLOC, 2, "match", lineLen);
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
									
							if (!inQuote2  &&  (line[z]==',' || line[z]=='}' || (line[z]==';' && arrayLev>0))  &&  ((appendingToSubArray && tmpArrayLev==arrayLev)  ||  !appendingToSubArray)) 
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
			QWrite_err(FWERROR_CLASS_PARENT, 4, classpath[J], J, ++K, ptr_filename);
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
					QWrite_err(FWERROR_CLASS_NOITEM, 3, ARRtargetID, WantedToken, ptr_filename);
				else
					 // not an array
					if (action==5 && foundTokenPos>=0 && ARRcurrent<0)     
						QWrite_err(FWERROR_CLASS_NOTARRAY, 2, WantedToken, ptr_filename);
					else
						// ...a property
						QWrite_err(FWERROR_CLASS_NOVAR, 2, WantedToken, ptr_filename);
			};

		// Rewrite the file
		if (!error)
		{	
			f = fopen(ptr_filename, "w");
			if (f) 
			{
				fwrite(buf, 1, strlen(buf), f);

				if (ferror(f)) 
					QWrite_err(FWERROR_ERRNO, 2, errno, ptr_filename);
				else
					QWrite_err(FWERROR_NONE, 0);

				fclose(f);
			}
			else
				QWrite_err(FWERROR_ERRNO, 2, errno, ptr_filename);
		};
	}

	QWritef("%d]", J);


	delete[] buf; 
	String_end(buf_filename); 
	free(line); 
	free(line2); 
	free(match);
}
break;












case C_CLASS_READ:
{
	const int predefined_capacity = 8;

	// Read arguments------------------------------------------------------------
	char *arg_filename      = empty_string;
	char *arg_classpath     = empty_string;
	char *arg_wrap          = empty_string;
	char *arg_findproperty  = empty_string;
	int arg_offset          = 0;
	int arg_classpath_pos   = -1;
	int arg_level           = predefined_capacity;
	int art_filename_length = 0;

	for (size_t i=2; i<argument_num; i+=2) {
		switch (argument_hash[i]) {
			case NAMED_ARG_FILE : 
				arg_filename        = argument[i+1];
				art_filename_length = argument_length[i+1];
				break;

			case NAMED_ARG_PATH :
				arg_classpath = argument[i+1];
				break;
			
			case NAMED_ARG_OFFSET : 
				arg_offset = atoi(argument[i+1]);
				break;
			
			case NAMED_ARG_FIND : {
				arg_findproperty          = argument[i+1];
				int arg_findproperty_last = argument_length[i+1] - 1;

				if (arg_findproperty[0]=='[' && arg_findproperty[arg_findproperty_last]==']') {
					arg_findproperty++;
					arg_findproperty[arg_findproperty_last] = '\0';
				}
			} break;

			case NAMED_ARG_WRAP : 
				arg_wrap = argument[i+1];
				break;
			
			case NAMED_ARG_PATHPOS : 
				arg_classpath_pos = atoi(argument[i+1]);
				break;
			
			case NAMED_ARG_MAXLEVEL : 
				arg_level = atoi(argument[i+1]);
				break;
		}
	}
	
	// File not specified
	if (art_filename_length == 0) {
		QWrite_err(FWERROR_PARAM_EMPTY, 1, "arg_filename");
		QWrite("[],[],0]");
		break;
	}

	// Verify and update path to the file
	String buf_filename;
	String_init(buf_filename);
	char *ptr_filename = arg_filename;

	if (!VerifyPath(&ptr_filename, buf_filename, OPTION_ALLOW_GAME_ROOT_DIR)) {
		QWrite("[],[],0]");
		break;
	}


	// Class path
	int classpath_capacity = arg_level!=predefined_capacity ? arg_level : predefined_capacity;
	int classpath_current  = 0;
	int classpath_size     = 0;
	char *classpath[predefined_capacity];
	char *class_name = strtok(arg_classpath, "[,]");

	//from ofp array to char array
	while (class_name!=NULL  &&  classpath_size<classpath_capacity) {
		classpath[classpath_size++] = stripq(class_name);
		class_name                  = strtok(NULL, "[,]");
	}

	if (arg_offset > 0) {
		classpath_current = classpath_size;
		
		if (arg_classpath_pos >= 0)
			classpath_current = arg_classpath_pos+1;
	} else
		arg_offset = 0;


	// Wrapping
	enum WRAP {
		NO_WRAP,
		YES_WRAP,
		NODOUBLE_WRAP
	};
	
	int wrap = NODOUBLE_WRAP;
	
	if (strcmpi(arg_wrap,"no") == 0)
		wrap = NO_WRAP;
 
	if (strcmpi(arg_wrap,"yes") == 0)
		wrap = YES_WRAP;
		
		
	// Finding properties
	const int properties_to_find_capacity = 64;
	char *properties_to_find[properties_to_find_capacity];
	int properties_to_find_size = 0;

	if (arg_findproperty[0] == '-') {
		arg_findproperty[0]     = '\0';
		properties_to_find_size = -1;
	} else {
		char *property = strtok(arg_findproperty, ",");
		while (property!=NULL  &&  properties_to_find_size<properties_to_find_capacity) {
			properties_to_find[properties_to_find_size++] = stripq(property);
			property                                      = strtok(NULL, ",");
		}
	}
	//---------------------------------------------------------------------------
	


	// Open wanted file -----------------------------------------------------------------	
	FILE *file = fopen(ptr_filename, "rb");
	if (!file) {
		QWrite_err(FWERROR_ERRNO, 2, errno, ptr_filename);
		QWrite("[],[],0]");
		String_end(buf_filename);
		break;
	}

	// Find file size
	if (fseek(file, 0, SEEK_END) != 0) {
		QWrite_err(FWERROR_ERRNO, 2, errno, ptr_filename);
		QWrite("[],[],0]");
		String_end(buf_filename);
		fclose(file);
		break;
	};

	size_t file_size = ftell(file);
	if (file_size == 0xFFFFFFFF) {
		QWrite_err(FWERROR_ERRNO, 2, errno, ptr_filename);
		QWrite("[],[],0]");
		String_end(buf_filename);
		fclose(file);
		break;
	} else
		file_size -= arg_offset;

	// Allocate buffer
	String file_contents;
	String_init(file_contents);
	
	int result = String_allocate(file_contents, file_size+1);
	if (result != 0) {
		QWrite_err(FWERROR_MALLOC, 2, "file_contents", file_size+1);
		QWrite("[],[],0]");
		String_end(buf_filename);
		break;
	}

	// Copy text to buffer
	fseek(file, arg_offset, SEEK_SET);
	size_t bytes_read = fread(file_contents.text, 1, file_size, file);
	file_contents.text[file_size] = '\0';

	if (bytes_read != file_size) {		
		if (ferror(file))
			QWrite_err(FWERROR_ERRNO, 2, errno, ptr_filename);
		else
			QWrite_err(FWERROR_FILE_WRITE, 4, errno, bytes_read, file_size, ptr_filename);
		
		String_end(buf_filename);
		String_end(file_contents);
		QWrite("[],[],0]");
		fclose(file);
		break;
	}

	fclose(file);
	



	enum EXPECT {
		PROPERTY,
		EQUALITY,
		VALUE,
		SEMICOLON,
		CLASS_NAME,
		CLASS_INHERIT,
		CLASS_COLON,
		CLASS_BRACKET,
		ENUM_BRACKET,
		ENUM_CONTENT,
		EXEC_BRACKET,
		EXEC_CONTENT,
		MACRO_CONTENT
	};
	
	enum COMMENT {
		NONE,
		LINE,
		BLOCK
	};
	
	String output_class[predefined_capacity];
	String output_inherit[predefined_capacity];
	String output_bytes[predefined_capacity];
	String output_property[predefined_capacity];
	String output_value[predefined_capacity];
	
	String *all_output_strings[] = {
		output_class,
		output_inherit,
		output_bytes,
		output_property,
		output_value
	};
	const int all_output_strings_num = sizeof(all_output_strings) / sizeof(all_output_strings[0]);
	
	for (int z=0; z<classpath_capacity; z++)
		for (int j=0; j<all_output_strings_num; j++) {
			String_init(all_output_strings[j][z]);
			String_append(all_output_strings[j][z], "[");
		}
		
	String output_classpath_bytes;
	String_init(output_classpath_bytes);
	String_append(output_classpath_bytes, "[");
  
	int comment           = NONE;
	int expect            = PROPERTY;
	int word_start        = -1;
	int class_level       = classpath_current;
	int array_level       = 0;
	int output_level      = 0;
	int parenthesis_level = 0;
	int property_start    = 0;
	int property_end      = 0;
	bool first_char       = true;
	bool is_array         = false;
	bool in_quote         = false;
	bool macro            = false;
	bool is_inherit       = false;
	bool classpath_done   = false;
	bool classpath_match  = false;
	bool property_found   = false;
	bool purge_comment    = false;
	char separator        = ' ';
	char *text            = file_contents.text;

	for (i=0; i<file_size; i++) {
		char c = text[i];

		// Parse preprocessor comment
		switch (comment) {
			case NONE  : {
				if (c == '/' && !in_quote) {
					char c2 = text[i+1];
					
					if (c2 == '/')
						comment = LINE;
					else 
						if (c2 == '*')
							comment = BLOCK;
				}
				
				if (comment == NONE)
					break;
				else {
					if (word_start>=0)
						purge_comment = true;
					
					continue;
				}
			}
			
			case LINE  : {
				if (c=='\r' || c=='\n')
					comment = NONE;

				continue;
			}
			
			case BLOCK : {
				if (i>0 && text[i-1]=='*' && c=='/')
					comment = NONE;

				continue;
			}
		}

		// Parse preprocessor directives
		if (!first_char && (c=='\r' || c=='\n')) {
			first_char = true;
			
			if (macro && text[i-1] != '\\')
				macro = false;
		}
		
		if (!isspace(text[i])  &&  first_char) {
			first_char = false;
			
			if (c == '#')
				macro = true;
		}
		
		if (macro)
			continue;


		// Parse classes
		switch (expect) {
			case SEMICOLON : {
				if (c == ';') {
					expect = PROPERTY;
					continue;
				} else 
					if (!isspace(c))
						expect = PROPERTY;
			}
			
			case PROPERTY : {
				if (c == '}') {
					// End parsing when left the target class
					if (classpath_current==classpath_size && class_level==classpath_current && !classpath_done) {
						classpath_done = true;
						i = file_size;
						continue;
					}
					
					// When left subclass within target class
					if (classpath_current==classpath_size && class_level>=classpath_current && !classpath_done) {
						int level = class_level - classpath_current;

						for (int j=level==0 ? 1 : level; j<classpath_capacity; j++)
							for (int k=0; k<all_output_strings_num; k++)
								String_append(all_output_strings[k][j], "]");
					}
						
					expect = SEMICOLON;
					class_level--;

					// End parsing when couldn't find wanted classes
					if (class_level < classpath_current)
						i = file_size;

					continue;
				}
				
				if (isalnum(c) || c=='_' || c=='[' || c==']') {
					if (word_start == -1)
						word_start = i;
				} else
					if (word_start >= 0) {
						text[i] = '\0';
	
						if (strcmp(text+word_start,"class")==0) {
							expect = CLASS_NAME;
						} else 
							if (strcmp(text+word_start,"enum")==0) {
								expect    = ENUM_BRACKET;
								separator = '{';
							} else 
								if (strcmp(text+word_start,"__EXEC")==0) {
									expect    = EXEC_BRACKET;
									separator = '(';
								} else {
									expect         = EQUALITY;
									separator      = '=';
									property_start = word_start;
									property_end   = i;							
									is_array       = text[i-2]=='[' && text[i-1]==']';
								}

						word_start = -1;
					}
				
				if (separator == ' ')
					break;
			}
			
			case EQUALITY     : 
			case ENUM_BRACKET : 
			case EXEC_BRACKET : {
				if (c == separator) {
					expect++;
					separator = ' ';
				} else 
					if (expect==EQUALITY && c=='(') {
						expect            = MACRO_CONTENT;
						separator         = ' ';
						parenthesis_level = 1;
					} else 
						if (!isspace(c)) {	//ignore syntax error
							i--;
							separator = ' ';
							expect    = SEMICOLON;
						}
				
				break;
			}
			
			case VALUE : {
				if (c == '"')
					in_quote = !in_quote;

				if (!in_quote && (c=='{' || c=='['))
					array_level++;

				if (!in_quote && (c=='}' || c==']')) {
					array_level--;

					// Remove trailing commas
					for (int z=i-1; z>0 && (isspace(text[z]) || text[z]==',' || text[z]=='}' || text[z]==']'); z--)
						if (text[z]==',')
							text[z] = ' ';
				}

				if (!in_quote && c==';' && array_level>0)
					text[i] = ',';

				if (word_start == -1) {
					if (!isspace(c))
						word_start = i;
				} else {
					if (!in_quote && array_level==0 && (c==';' || c=='\r' || c=='\n')) {
						text[i] = '\0';

						char *property = text + property_start;
						char *value    = text + word_start;

						if (purge_comment) {
							purge_comment = false;
							PurgeComments(text, property_start, property_end);
							PurgeComments(text, word_start    , i);
						}

						if (classpath_current==classpath_size && class_level>=classpath_current && !classpath_done) {
							bool found = properties_to_find_size == 0;

							for (int j=0; j<properties_to_find_size && !found; j++)
								if (strcmpi(property,properties_to_find[j]) == 0)
									found = true;

							if (found) {
								property_found = true;
								int level      = class_level - classpath_current;

								if (level < classpath_capacity) {
									// Add property name
									String_append_format(output_property[level], "]+[\"%s\"", property);

									// Add property value
									String_append(output_value[level], "]+[");

									if (wrap==YES_WRAP || (wrap==NODOUBLE_WRAP && value[0]!='\"'))
										String_append(output_value[level], "\"");

									// Convert arrays (square brackets) and strings (double quotes) so that "call" command in OFP can be used
									if (is_array || (wrap==YES_WRAP && value[0]=='\"')) {
										for (size_t j=word_start; j<i; j++) {
											if (text[j] == '"')
												in_quote = !in_quote;

											if (text[j]=='{' && !in_quote && wrap==NO_WRAP)
												String_append(output_value[level], "[");
											else 
												if (text[j]=='}' && !in_quote && wrap==NO_WRAP)
													String_append(output_value[level], "]");
												else 
													if (text[j]=='"' && (wrap==YES_WRAP || wrap==NODOUBLE_WRAP))
														String_append(output_value[level], "\"\"");
													else 
														String_append_len(output_value[level], text+j, 1);
										}
									} else
										String_append(output_value[level], value);
									
									if (wrap==YES_WRAP || (wrap==NODOUBLE_WRAP && value[0]!='\"'))
										String_append(output_value[level], "\"");
								}
							}
						}
						
						word_start = -1;
						expect     = PROPERTY;
					}
				}
				
				break;
			}
			
			case CLASS_NAME    :
			case CLASS_INHERIT : {
				if (isalnum(c) || c=='_') {
					if (word_start == -1)
						word_start = i;
				} else
					if (word_start >= 0) {
						text[i] = '\0';
						
						if (purge_comment) {
							purge_comment = false;
							PurgeComments(text, word_start, i);
						}
						
						if (expect==CLASS_NAME && classpath_size>0 && classpath_current<classpath_size && class_level==classpath_current && strcmpi(text+word_start,classpath[classpath_current])==0) {
							classpath_current++;
							classpath_match = true;
						}

						if (classpath_current==classpath_size && class_level>=classpath_current && !classpath_done) {
							String *output_array = expect==CLASS_NAME ? output_class : output_inherit;
							int level            = class_level - classpath_current;

							if (level < classpath_capacity && !classpath_done)
								String_append_format(output_array[level], "]+[\"%s\"", text+word_start);
						}
						
						is_inherit = expect == CLASS_INHERIT;
						word_start = -1;
						expect     = expect==CLASS_NAME ? CLASS_COLON : CLASS_BRACKET;
					}
				
				if (expect!=CLASS_COLON && expect!=CLASS_BRACKET)
					break;
			}
			
			case CLASS_COLON   :
			case CLASS_BRACKET : {
				if (expect==CLASS_COLON && c==':')
					expect = CLASS_INHERIT;
				else 
					if (c == '{') {
						if (!is_inherit) {
							if (classpath_current==classpath_size && class_level>=classpath_current && !classpath_done) {
								int level = class_level - classpath_current;
								
								if (level < classpath_capacity && !classpath_done)
									String_append(output_inherit[level], "]+[\"\"");
							}
						}
						
						if (classpath_match) {
							String_append_format(output_classpath_bytes, "]+[\"%d\"", i+1);
							classpath_match = false;
						}
								
						if (classpath_current==classpath_size && class_level>=classpath_current && !classpath_done) {
							int level = class_level - classpath_current;

							if (level < classpath_capacity)
								String_append_format(output_bytes[level], "]+[\"%d\"", i+1);
						}
						
						class_level++;
						expect = PROPERTY;
							
						if (classpath_current==classpath_size && class_level>=classpath_current && !classpath_done) {
							int level = class_level - classpath_current;
							
							if (level > output_level)
								output_level = level;

							for (int j=level==0 ? 1 : level; j<classpath_capacity; j++)
								for (int k=0; k<all_output_strings_num; k++)
									String_append(all_output_strings[k][j], "]+[[");
						}
					} else
						if (!isspace(c)) {	//ignore syntax error
							i--;
							expect = SEMICOLON;
						}
				
				break;
			}
			
			case ENUM_CONTENT : 
			case EXEC_CONTENT : {
				if ((expect==EXEC_CONTENT && c==')') || (expect==ENUM_CONTENT && c=='}'))
					expect = SEMICOLON;

				break;
			}
			
			case MACRO_CONTENT : {
				if (c == '"')
					in_quote = !in_quote;
					
				if (!in_quote) {
					if (c == '(')
						parenthesis_level++;
						
					if (c == ')')
						parenthesis_level--;
						
					if (parenthesis_level == 0)
						expect = SEMICOLON;
				}
					
				break;
			}
		}
	}

	//---------------------------------------------------------------------------
	char output_strings_name[all_output_strings_num][16] = {
		"class",
		"inherit",
		"bytes",
		"property",
		"value"
	};

	for (z=0; z<classpath_capacity && z<=output_level; z++) {
		for (int j=0; j<all_output_strings_num; j++) {
			QWritef("_output_%s%d=%s];", output_strings_name[j], z, all_output_strings[j][z].text);
			String_end(all_output_strings[j][z]);
		}
	}
	
	if (classpath_current < classpath_size)
		QWrite_err(FWERROR_CLASS_PARENT, 4, classpath[classpath_current], classpath_current, ++classpath_size, ptr_filename);
	else
		if (properties_to_find_size>0 && !property_found)
			QWrite_err(FWERROR_CLASS_NOVAR, 2, arg_findproperty, ptr_filename);
		else
			QWrite_err(FWERROR_NONE, 0);

	QWrite("[");
	
	for (z=0; z<classpath_capacity && z<=output_level; z++) {
		if (z == 0)
			QWrite("[");
		else
			QWrite(",[");

		for (int j=0; j<all_output_strings_num; j++)
			QWritef("%c_output_%s%d", (j==0 ? ' ' : ','), output_strings_name[j], z);

		QWrite("]");
	}

	QWritef("],%s],%d]", output_classpath_bytes.text, classpath_current);

	String_end(output_classpath_bytes);
	String_end(file_contents);
	String_end(buf_filename);
}
break;












case C_CLASS_READ2:
{
	const int predefined_capacity = 8;

	// Read arguments------------------------------------------------------------
	char *arg_filename      = empty_string;
	char *arg_classpath     = empty_string;
	char *arg_wrap          = empty_string;
	char *arg_findproperty  = empty_string;
	int arg_offset          = 0;
	int arg_classpath_pos   = -1;
	int arg_level           = predefined_capacity;
	int arg_filename_length = 0;
	bool arg_verify         = false;
	
	for (size_t i=2; i<argument_num; i+=2) {
		switch (argument_hash[i]) {
			case NAMED_ARG_FILE : 
				arg_filename        = argument[i+1];
				arg_filename_length = argument_length[i+1];
				break;

			case NAMED_ARG_PATH : 
				arg_classpath = argument[i+1];
				break;
			
			case NAMED_ARG_OFFSET : 
				arg_offset = atoi(argument[i+1]);
				break;
			
			case NAMED_ARG_FIND : {
				arg_findproperty          = argument[i+1];
				int arg_findproperty_last = argument_length[i+1] - 1;
				
				if (arg_findproperty[0]=='[' && arg_findproperty[arg_findproperty_last]==']') {
					arg_findproperty++;
					arg_findproperty[arg_findproperty_last] = '\0';
				}
			} break;

			case NAMED_ARG_WRAP : 
				arg_wrap = argument[i+1];
				break;
			
			case NAMED_ARG_PATHPOS : 
				arg_classpath_pos = atoi(argument[i+1]);
				break;
			
			case NAMED_ARG_MAXLEVEL : 
				arg_level = atoi(argument[i+1]);
				break;

			case NAMED_ARG_VERIFY : 
				arg_verify = String2Bool(argument[i+1]);
				break;
		}
	}

	// File not specified
	if (arg_filename_length == 0) {
		QWrite_err(FWERROR_PARAM_EMPTY, 1, "arg_filename");
		QWrite("[],[],0]");
		break;
	}

	// Verify and update path to the file
	String buf_filename;
	String_init(buf_filename);
	char *ptr_filename = arg_filename;

	if (!VerifyPath(&ptr_filename, buf_filename, OPTION_ALLOW_GAME_ROOT_DIR)) {
		QWrite("[],[],0]");
		break;
	}


	// Class path
	int classpath_capacity = arg_level!=predefined_capacity ? arg_level : predefined_capacity;
	int classpath_current  = 0;
	int classpath_size     = 0;
	char *classpath[predefined_capacity];
	char *class_name = strtok(arg_classpath, "[,]");

	//from ofp array to char array
	if (!arg_verify) {
		while (class_name!=NULL  &&  classpath_size<classpath_capacity) {
			classpath[classpath_size++] = stripq(class_name);
			class_name                  = strtok(NULL, "[,]");
		}

		if (arg_offset > 0) {
			classpath_current = classpath_size;
			
			if (arg_classpath_pos >= 0)
				classpath_current = arg_classpath_pos+1;
		} else
			arg_offset = 0;
	}


	// Wrapping
	enum WRAP {
		NO_WRAP,
		YES_WRAP,
		NODOUBLE_WRAP
	};
	
	int wrap = NODOUBLE_WRAP;
	
	if (strcmpi(arg_wrap,"no") == 0)
		wrap = NO_WRAP;
 
	if (strcmpi(arg_wrap,"yes") == 0)
		wrap = YES_WRAP;
		
		
	// Finding properties
	const int properties_to_find_capacity = 64;
	char *properties_to_find[properties_to_find_capacity];
	int properties_to_find_size = 0;
	
	if (arg_findproperty[0] == '-') {
		arg_findproperty[0]     = '\0';
		properties_to_find_size = -1;
	} else {
		char *property = strtok(arg_findproperty, ",");
		while (property!=NULL  &&  properties_to_find_size<properties_to_find_capacity) {
			properties_to_find[properties_to_find_size++] = stripq(property);
			property                                      = strtok(NULL, ",");
		}
	}
	//---------------------------------------------------------------------------
	


	// Open wanted file -----------------------------------------------------------------	
	FILE *file = fopen(ptr_filename, "rb");
	if (!file) {
		QWrite_err(FWERROR_ERRNO, 2, errno, ptr_filename);
		QWrite("[],[],0]");
		String_end(buf_filename);
		break;
	}

	// Find file size
	if (fseek(file, 0, SEEK_END) != 0) {
		QWrite_err(FWERROR_ERRNO, 2, errno, ptr_filename);
		QWrite("[],[],0]");
		String_end(buf_filename);
		fclose(file);
		break;
	};

	size_t file_size = ftell(file);
	if (file_size == 0xFFFFFFFF) {
		QWrite_err(FWERROR_ERRNO, 2, errno, ptr_filename);
		QWrite("[],[],0]");
		String_end(buf_filename);
		fclose(file);
		break;
	} else
		file_size -= arg_offset;

	// Allocate buffer
	String file_contents;
	String_init(file_contents);
	
	int result = String_allocate(file_contents, file_size+1);
	if (result != 0) {
		QWrite_err(FWERROR_MALLOC, 2, "file_contents", file_size+1);
		QWrite("[],[],0]");
		String_end(buf_filename);
		break;
	}

	// Copy text to buffer
	fseek(file, arg_offset, SEEK_SET);
	size_t bytes_read = fread(file_contents.text, 1, file_size, file);
	file_contents.text[file_size] = '\0';

	if (bytes_read != file_size) {		
		if (ferror(file))
			QWrite_err(FWERROR_ERRNO, 2, errno, ptr_filename);
		else
			QWrite_err(FWERROR_FILE_WRITE, 3, bytes_read, file_size, ptr_filename);
		
		String_end(buf_filename);
		String_end(file_contents);
		QWrite("[],[],0]");
		fclose(file);
		break;
	}

	fclose(file);
	



	enum EXPECT {
		PROPERTY,
		EQUALITY,
		VALUE,
		SEMICOLON,
		CLASS_NAME,
		CLASS_INHERIT,
		CLASS_COLON,
		CLASS_BRACKET,
		ENUM_BRACKET,
		ENUM_CONTENT,
		EXEC_BRACKET,
		EXEC_CONTENT,
		MACRO_CONTENT
	};
	
	enum COMMENT {
		NONE,
		LINE,
		BLOCK
	};
	
	String output_class[predefined_capacity];
	String output_inherit[predefined_capacity];
	String output_bytes[predefined_capacity];
	String output_property[predefined_capacity];
	String output_value[predefined_capacity];
	
	String *all_output_strings[] = {
		output_class,
		output_inherit,
		output_bytes,
		output_property,
		output_value
	};
	const int all_output_strings_num = sizeof(all_output_strings) / sizeof(all_output_strings[0]);

	char output_strings_name[all_output_strings_num][16] = {
		"class",
		"inherit",
		"bytes",
		"property",
		"value"
	};

	char temp[512] = "";
	for (int z=0; z<classpath_capacity; z++)
		for (int j=0; j<all_output_strings_num; j++) {
			String_init(all_output_strings[j][z]);
			String_append_format(all_output_strings[j][z], "_output_%s%d=[];", output_strings_name[j], z);
		}
		
	String output_classpath_bytes;
	String_init(output_classpath_bytes);
	String_append(output_classpath_bytes, "[");
	
	int subclass_count[predefined_capacity] = {0,0,0,0,0,0,0,0};
	
	int comment           = NONE;
	int expect            = PROPERTY;
	int word_start        = -1;
	int class_level       = classpath_current;
	int array_level       = 0;
	int output_level      = 0;
	int parenthesis_level = 0;
	int property_start    = 0;
	int property_end      = 0;
	int line_num          = 1;
	int column_num        = 0;
	bool first_char       = true;
	bool is_array         = false;
	bool in_quote         = false;
	bool macro            = false;
	bool is_inherit       = false;
	bool classpath_done   = false;
	bool classpath_match  = false;
	bool property_found   = false;
	bool purge_comment    = false;
	bool syntax_error     = false;
	char separator        = ' ';
	char *text            = file_contents.text;
	
	for (i=0; i<file_size; i++) {
		char c = text[i];
		
		if (c == '\n') {
			if (i<file_size-1)
				line_num++;
				
			column_num = 1;
		} else
			column_num++;

		// Parse preprocessor comment
		switch (comment) {
			case NONE  : {
				if (c == '/' && !in_quote) {
					char c2 = text[i+1];
					
					if (c2 == '/')
						comment = LINE;
					else 
						if (c2 == '*')
							comment = BLOCK;
				}
				
				if (comment == NONE)
					break;
				else {
					if (word_start>=0)
						purge_comment = true;
					
					continue;
				}
			}
			
			case LINE  : {
				if (c=='\r' || c=='\n')
					comment = NONE;

				continue;
			}
			
			case BLOCK : {
				if (i>0 && text[i-1]=='*' && c=='/')
					comment = NONE;

				continue;
			}
		}

		// Parse preprocessor directives
		if (!first_char && (c=='\r' || c=='\n')) {
			first_char = true;
			
			if (macro && text[i-1] != '\\')
				macro = false;
		}
		
		if (!isspace(text[i])  &&  first_char) {
			first_char = false;
			
			if (c == '#')
				macro = true;
		}
		
		if (macro)
			continue;


		// Parse classes
		switch (expect) {
			case SEMICOLON : {
				if (c == ';') {
					expect = PROPERTY;
					continue;
				} else 
					if (!isspace(c))
						expect = PROPERTY;
			}
			
			case PROPERTY : {
				if (c == '}') {
					// End parsing when left the target class
					if (!arg_verify && classpath_current==classpath_size && class_level==classpath_current && !classpath_done) {
						classpath_done = true;
						i              = file_size;
						continue;
					}
					
					// When left subclass within target class
					if (!arg_verify && classpath_current==classpath_size && class_level>=classpath_current && !classpath_done) {
						int level = class_level - classpath_current;

						for (int z=level; z<predefined_capacity; z++)
							subclass_count[z]=0;
							
						if (level > 0)
							subclass_count[level-1]++;
					}
					
					expect = SEMICOLON;
					class_level--;
					
					// End parsing when couldn't find wanted classes
					if (!arg_verify && class_level < classpath_current)
						i = file_size;

					// Excess closing brackets
					if (arg_verify  &&  class_level < 0) {
						column_num--;
						separator = ' ';
						syntax_error = true;
						goto class_read2_end_parsing;
					}

					continue;
				}
				
				if (isalnum(c) || c=='_' || c=='[' || c==']') {
					if (word_start == -1)
						word_start = i;
				} else
					if (word_start >= 0) {
						text[i] = '\0';
	
						if (strcmp(text+word_start,"class")==0) {
							expect = CLASS_NAME;
						} else 
							if (strcmp(text+word_start,"enum")==0) {
								expect    = ENUM_BRACKET;
								separator = '{';
							} else 
								if (strcmp(text+word_start,"__EXEC")==0) {
									expect    = EXEC_BRACKET;
									separator = '(';
								} else {
									expect         = EQUALITY;
									separator      = '=';
									property_start = word_start;
									property_end   = i;							
									is_array       = text[i-2]=='[' && text[i-1]==']';
								}

						word_start = -1;
					}
				
				if (separator == ' ')
					break;
			}
			
			case EQUALITY     : 
			case ENUM_BRACKET : 
			case EXEC_BRACKET : {
				if (c == separator) {
					expect++;
					separator = ' ';
				} else 
					if (expect==EQUALITY && c=='(') {
						expect            = MACRO_CONTENT;
						separator         = ' ';
						parenthesis_level = 1;
					} else 
						if (!isspace(c)) {
							if (!arg_verify) {	//syntax error
								i--;
								separator = ' ';
								expect    = SEMICOLON;
							} else {
								syntax_error = true;
								goto class_read2_end_parsing;
							}
						}
				
				break;
			}
			
			case VALUE : {
				if (c == '"')
					in_quote = !in_quote;

				if (!in_quote && (c=='{' || c=='['))
					array_level++;

				if (!in_quote && (c=='}' || c==']')) {
					array_level--;

					// Remove trailing commas
					for (int z=i-1; z>0 && (isspace(text[z]) || text[z]==',' || text[z]=='}' || text[z]==']'); z--)
						if (text[z]==',')
							text[z] = ' ';
				}

				if (word_start == -1) {
					if (!isspace(c))
						word_start = i;
				} else {
					if (!in_quote && array_level==0 && (c==';' || c=='\r' || c=='\n')) {
						text[i] = '\0';

						char *property = text + property_start;
						char *value    = text + word_start;

						if (purge_comment) {
							purge_comment = false;
							PurgeComments(text, property_start, property_end);
							PurgeComments(text, word_start    , i);
						}

						if (!arg_verify && classpath_current==classpath_size && class_level>=classpath_current && !classpath_done) {
							bool found = properties_to_find_size == 0;

							for (int j=0; j<properties_to_find_size && !found; j++)
								if (strcmpi(property,properties_to_find[j]) == 0)
									found = true;

							if (found) {
								property_found = true;
								int level      = class_level - classpath_current;

								if (!arg_verify && level < classpath_capacity) {
									// Add property name
									String_append_format(output_property[level], "%s\"%s\"];", Output_Nested_Array(temp, level, "property", level, subclass_count), property);

									// Add property value
									String_append(output_value[level], Output_Nested_Array(temp, level, "value", level, subclass_count));

									if (wrap==YES_WRAP || (wrap==NODOUBLE_WRAP && value[0]!='\"'))
										String_append(output_value[level], "\"");

									// Convert arrays (square brackets) and strings (double quotes) so that "call" command in OFP can be used
									if (is_array || (wrap==YES_WRAP && value[0]=='\"')) {
										for (size_t j=word_start; j<i; j++) {											
											if (text[j] == '"')
												in_quote = !in_quote;

											if (text[j]=='{' && !in_quote && wrap==NO_WRAP)
												String_append(output_value[level], "[");
											else 
												if (text[j]=='}' && !in_quote && wrap==NO_WRAP)
													String_append(output_value[level], "]");
												else 
													if (text[j]=='"' && (wrap==YES_WRAP || wrap==NODOUBLE_WRAP))
														String_append(output_value[level], "\"\"");
													else 
														String_append_len(output_value[level], text+j, 1);
										}
									} else
										String_append(output_value[level], value);
									
									if (wrap==YES_WRAP || (wrap==NODOUBLE_WRAP && value[0]!='\"'))
										String_append(output_value[level], "\"");
										
									String_append(output_value[level], "];");
								}
							}
						}
						
						word_start = -1;
						expect     = PROPERTY;
					}
				}
				
				break;
			}
			
			case CLASS_NAME    :
			case CLASS_INHERIT : {
				if (isalnum(c) || c=='_') {
					if (word_start == -1)
						word_start = i;
				} else
					if (word_start >= 0) {
						text[i] = '\0';
						
						if (purge_comment) {
							purge_comment = false;
							PurgeComments(text, word_start, i);
						}
						
						if (!arg_verify && expect==CLASS_NAME && classpath_size>0 && classpath_current<classpath_size && class_level==classpath_current && strcmpi(text+word_start,classpath[classpath_current])==0) {
							classpath_current++;
							classpath_match = true;
						}

						if (!arg_verify && classpath_current==classpath_size && class_level>=classpath_current && !classpath_done) {
							String *output_array = expect==CLASS_NAME ? output_class : output_inherit;
							int level            = class_level - classpath_current;

							if (!arg_verify && level < classpath_capacity && !classpath_done) {
								char tempname[8] = "class";
								if (expect != CLASS_NAME)
									strcpy(tempname, "inherit");

								String_append_format(output_array[level], "%s\"%s\"];", Output_Nested_Array(temp, level, tempname, level, subclass_count), text+word_start);
							}
						}
						
						is_inherit = expect == CLASS_INHERIT;
						word_start = -1;
						expect     = expect==CLASS_NAME ? CLASS_COLON : CLASS_BRACKET;
					}
				
				if (expect!=CLASS_COLON && expect!=CLASS_BRACKET)
					break;
			}
			
			case CLASS_COLON   :
			case CLASS_BRACKET : {
				if (expect==CLASS_COLON && c==':')
					expect = CLASS_INHERIT;
				else 
					if (c == '{') {
						if (!arg_verify && !is_inherit) {
							if (classpath_current==classpath_size && class_level>=classpath_current && !classpath_done) {
								int level = class_level - classpath_current;
								
								if (!arg_verify && level < classpath_capacity && !classpath_done)
									String_append_format(output_inherit[level], "%s\"\"];", Output_Nested_Array(temp, level, "inherit", level, subclass_count));
								
							}
						}

						if (!arg_verify && classpath_match) {
							String_append_format(output_classpath_bytes, "]+[\"%d\"", i+1);
							classpath_match = false;
						}

						if (!arg_verify && classpath_current==classpath_size && class_level>=classpath_current && !classpath_done) {
							int level = class_level - classpath_current;

							if (!arg_verify && level < classpath_capacity)
								String_append_format(output_bytes[level], "%s\"%d\"];", Output_Nested_Array(temp, level, "bytes", level, subclass_count), i+1);
						}

						class_level++;
						expect = PROPERTY;
						
						if (!arg_verify && classpath_current==classpath_size && class_level>=classpath_current && !classpath_done) {
							int level = class_level - classpath_current;
							
							if (level > output_level)
								output_level = level;
							
							if (!arg_verify && level > 0)
							for (int j=level==0 ? 1 : level; j<classpath_capacity; j++)
								for (int k=0; k<all_output_strings_num; k++)
									String_append_format(all_output_strings[k][j], "%s[]];", Output_Nested_Array(temp, level-1, output_strings_name[k], j, subclass_count));
						}
					} else
						if (!isspace(c)) {
							if (!arg_verify) {	//syntax error
								i--;
								expect = SEMICOLON;
							} else {
								separator = '{';
								syntax_error = true;
								goto class_read2_end_parsing;
							}
						}
				
				break;
			}
			
			case ENUM_CONTENT : 
			case EXEC_CONTENT : {
				if ((expect==EXEC_CONTENT && c==')') || (expect==ENUM_CONTENT && c=='}'))
					expect = SEMICOLON;

				break;
			}
			
			case MACRO_CONTENT : {
				if (c == '"')
					in_quote = !in_quote;
					
				if (!in_quote) {
					if (c == '(')
						parenthesis_level++;
						
					if (c == ')')
						parenthesis_level--;
						
					if (parenthesis_level == 0)
						expect = SEMICOLON;
				}
					
				break;
			}
		}
	}

	//---------------------------------------------------------------------------
	class_read2_end_parsing:

	if (!arg_verify) {
		for (z=0; z<classpath_capacity && z<=output_level; i++) {
			for (int j=0; j<all_output_strings_num; j++) {
				QWritel(all_output_strings[j][z].text, all_output_strings[j][z].length);
				String_end(all_output_strings[j][z]);
			}
		}

		if (classpath_current < classpath_size)
			QWrite_err(FWERROR_CLASS_PARENT, 4, classpath[classpath_current], classpath_current, ++classpath_size, ptr_filename);
		else
			if (properties_to_find_size>0 && !property_found)
				QWrite_err(FWERROR_CLASS_NOVAR, 2, arg_findproperty, ptr_filename);
			else
				QWrite_err(FWERROR_NONE, 0);

		QWrite("[");
		
		for (z=0; z<classpath_capacity && z<=output_level; z++) {
			if (z == 0)
				QWrite("[");
			else
				QWrite(",[");

			for (int j=0; j<all_output_strings_num; j++)
				QWritef("%c_output_%s%d", (j==0 ? ' ' : ','), output_strings_name[j], z);

			QWrite("]");
		}

		QWritef("],%s],%d]", output_classpath_bytes.text, classpath_current);
	} else {
		if (!syntax_error && class_level>0) {
			syntax_error = true;
			separator    = '}';
		}

		if (syntax_error) {
			char error_msg[32]   = "";
			char encountered[16] = "end of file";
			char insteadof[16]   = "nothing";

			if (i < file_size)
				sprintf(encountered, "%c", text[i]);

			if (separator != ' ')
				sprintf(insteadof, "%c", separator);

			sprintf(error_msg, "%s instead of %s", encountered, insteadof);
			QWrite_err(FWERROR_CLASS_SYNTAX, 4, error_msg, line_num, column_num, ptr_filename);
		} else
			QWrite_err(FWERROR_NONE, 0);

		QWrite("[]]");
	}

	String_end(output_classpath_bytes);
	String_end(file_contents);
	String_end(buf_filename);
}
break;












case C_CLASS_READSQM:
{ // Convert SQM file to SQF format

	global.option_error_output = OPTION_ERROR_ARRAY_CLOSE;

	char *arg_filename      = empty_string;
	int arg_filename_length = 0;
	
	for (size_t i=2; i<argument_num; i+=2) {
		switch (argument_hash[i]) {
			case NAMED_ARG_FILE : 
				arg_filename        = argument[i+1];
				arg_filename_length = argument_length[i+1];
				break;
		}
	}

	// File not specified
	if (arg_filename_length == 0) {
		QWrite_err(FWERROR_PARAM_EMPTY, 1, "arg_filename");
		break;
	}

	// Verify and update path to the file
	String buf_filename;
	String_init(buf_filename);
	char *ptr_filename = arg_filename;

	if (!VerifyPath(&ptr_filename, buf_filename, OPTION_ALLOW_GAME_ROOT_DIR))
		break;
	//---------------------------------------------------------------------------
	


	// Open wanted file -----------------------------------------------------------------	
	FILE *file = fopen(ptr_filename, "rb");
	if (!file) {
		QWrite_err(FWERROR_ERRNO, 2, errno, ptr_filename);
		String_end(buf_filename);
		break;
	}

	// Find file size
	if (fseek(file, 0, SEEK_END) != 0) {
		QWrite_err(FWERROR_ERRNO, 2, errno, ptr_filename);
		String_end(buf_filename);
		fclose(file);
		break;
	};

	size_t file_size = ftell(file);
	if (file_size == 0xFFFFFFFF) {
		QWrite_err(FWERROR_ERRNO, 2, errno, ptr_filename);
		String_end(buf_filename);
		fclose(file);
		break;
	}

	// Allocate buffer
	String file_content;
	String_init(file_content);
	
	int result = String_allocate(file_content, file_size+1);
	if (result != 0) {
		QWrite_err(FWERROR_MALLOC, 2, "file_content", file_size);
		String_end(buf_filename);
		break;
	}

	// Copy text to buffer
	fseek(file, 0, SEEK_SET);
	size_t bytes_read = fread(file_content.text, 1, file_size, file);

	if (bytes_read != file_size) {		
		if (ferror(file))
			QWrite_err(FWERROR_ERRNO, 2, errno, ptr_filename);
		else
			QWrite_err(FWERROR_FILE_WRITE, 3, bytes_read, file_size, ptr_filename);
		
		String_end(buf_filename);
		String_end(file_content);
		fclose(file);
		break;
	}

	file_content.length = bytes_read;
	fclose(file);

	int inside            = 0;
	const int capacity    = 8;
	
	int opened_classes[capacity] = {0};

	enum MISSION_SQM_CLASSES {
		CLASS_MISSION   = 0x1,
		CLASS_INTEL     = 0x2,
		CLASS_GROUPS    = 0x4,
		CLASS_ITEM      = 0x8,
		CLASS_VEHICLES  = 0x10,
		CLASS_WAYPOINTS = 0x20,
		CLASS_EFFECTS   = 0x40,
		CLASS_MARKERS   = 0x80,
		CLASS_SENSORS   = 0x100
	};
	
	const char class_names[][16] = {
		"Mission",
		"Intel",
		"Groups",
		"Item",
		"Vehicles",
		"Waypoints",
		"Effects",
		"Markers",
		"Sensors"
	};
	
	const int class_ids[] = {
		CLASS_MISSION,
		CLASS_INTEL,
		CLASS_GROUPS,
		CLASS_ITEM,
		CLASS_VEHICLES,
		CLASS_WAYPOINTS,
		CLASS_EFFECTS,
		CLASS_MARKERS,
		CLASS_SENSORS
	};
	
	int class_max = sizeof(class_ids) / sizeof(class_ids[0]);
	
	int units_in_this_group = 0;
	SQM_ParseState state;
	SQM_Init(state);
	
	while (state.i < file_content.length) {
		switch (SQM_Parse(file_content.text, file_content.length, state, SQM_ACTION_GET_NEXT_ITEM, NULL, 0)) {
			case SQM_OUTPUT_PROPERTY : {								
				if (inside & CLASS_MISSION) {
					// Convert array format by replacing brackets
					if (state.property[state.property_length-1]==']' && state.property[state.property_length-2]=='[') {
						state.property[state.property_length-1] = ' ';
						state.property[state.property_length-2] = ' ';
						
						bool in_quote = false;
	
						for (size_t i=0; i<state.value_length; i++) {
							if (!in_quote  &&  state.value[i]=='{')
								state.value[i] = '[';
	
							if (!in_quote  &&  state.value[i]=='}')
								state.value[i] = ']';
	
							if (state.value[i] == '"')
								in_quote = !in_quote;
						}
					}
					
					// Remove "side=" from a group
					if (inside & CLASS_GROUPS && inside & CLASS_ITEM && ~inside & CLASS_VEHICLES) {
						if (strncmpi("side",state.property,state.property_length)==0) {
							if (state.value[state.value_length-1] == ';')
								state.value_length--;
							
							QWritel(state.value, state.value_length);
							QWrite(",");
						}
					}
					
					// Convert properties to local vars by adding underscore
					if (
						(inside & CLASS_GROUPS && inside & CLASS_VEHICLES && inside & CLASS_ITEM && state.class_level==5) || 
						(inside & CLASS_GROUPS && inside & CLASS_WAYPOINTS && inside & CLASS_ITEM && state.class_level==5) ||
						(~inside & CLASS_GROUPS && (inside & CLASS_VEHICLES || inside & CLASS_MARKERS || inside & CLASS_SENSORS) && inside & CLASS_ITEM) ||
						(inside & CLASS_EFFECTS) || 
						inside & CLASS_INTEL
					) {
						QWrite("_");
						QWritel(state.property, state.value_end - state.property_start);
					}
				}
			} break;
			
			case SQM_OUTPUT_CLASS : {							
				for (int z=0; z<class_max; z++) {
					if (strncmpi(state.class_name,class_names[z],strlen(class_names[z])) == 0) {
						int level             = state.class_level - 1;
						int opened            = class_ids[z];
						opened_classes[level] = opened;
						inside               |= opened;
										
						if (inside & CLASS_MISSION)
							switch (opened) {
								case CLASS_INTEL  : QWrite("_Intel={"); break;
								case CLASS_GROUPS : QWrite("_Groups=["); break;
								
								case CLASS_ITEM : {
									// Soldier group
									if (inside & CLASS_GROUPS && level==2)
										QWrite("[");
										
									// Soldier
									if ((inside & CLASS_GROUPS && (inside & CLASS_VEHICLES || inside & CLASS_WAYPOINTS))) {
										if (units_in_this_group > 0)
											QWrite(",{");
										else
											QWrite("{");

										units_in_this_group++;
									}
									
									// Vehicle
									if (~inside & CLASS_GROUPS && (inside & (CLASS_VEHICLES|CLASS_MARKERS|CLASS_SENSORS)))
										QWrite("{");
								} break;
								
								case CLASS_WAYPOINTS : {
									units_in_this_group=0; 
									QWrite(",["); 
								} break;
								
								case CLASS_VEHICLES : {														
									if (~inside & CLASS_GROUPS)
										QWrite("_Vehicles=[");
									else
										QWrite("[");
								} break;
								
								case CLASS_MARKERS : QWrite("_Markers=["); break;
								case CLASS_SENSORS : QWrite("_Sensors=["); break;
								case CLASS_EFFECTS : QWrite("_Effects={"); break;
							}
						
						break;
					}
				}
			} break;
			
			case SQM_OUTPUT_END_OF_SCOPE : {
				int level             = state.class_level;
				int closed            = opened_classes[level];
				opened_classes[level] = 0;
				inside                = 0;
				
				for (int z=0; z<level; z++)
					inside |= opened_classes[z];
				
				if (closed == CLASS_MISSION) {
					QWrite(""); 
					goto class_readsqm_endparsing;
				}
				
				if (inside & CLASS_MISSION)
					switch (closed) {
						case CLASS_INTEL    : QWrite("};"); break;
						case CLASS_GROUPS   : QWrite("];"); break;
						case CLASS_ITEM : {
							// Soldier group
							if (inside & CLASS_GROUPS && level==2) {
								QWrite("]]+[");
								units_in_this_group = 0;
							}
							
							// Soldier/Waypoint
							if (inside & CLASS_GROUPS && (inside & CLASS_VEHICLES || inside & CLASS_WAYPOINTS))
								QWrite("}");
							
							// Vehicle
							if (~inside & CLASS_GROUPS && inside & (CLASS_VEHICLES|CLASS_MARKERS|CLASS_SENSORS))
								QWrite("}]+[");
						} break;
						
						case CLASS_WAYPOINTS : QWrite("]"); break;
						
						case CLASS_VEHICLES : {
							if (~inside & CLASS_GROUPS)
								QWrite("];");
							else
								QWrite("]");
						} break;
						
						case CLASS_MARKERS : QWrite("];"); break;
						case CLASS_SENSORS : QWrite("];"); break;
						case CLASS_EFFECTS : QWrite("};"); break;
					}
			} break;
		}
	}
	
	class_readsqm_endparsing:

	//---------------------------------------------------------------------------
	QWrite_err(FWERROR_NONE, 0);

	String_end(file_content);
	String_end(buf_filename);
}
break;













case C_CLASS_WRITE : 
{ // Modify class and properties in a file

	char *arg_filename               = empty_string;
	char *arg_merge                  = empty_string;
	char *arg_classpath              = empty_string;
	char *arg_deleteclass            = empty_string;
	char *arg_deleteproperty         = empty_string;
	char *arg_renameclass            = empty_string;
	char *arg_renameproperty         = empty_string;
	char *arg_renameto               = empty_string;
    char *arg_offset                 = empty_string;
	size_t arg_filename_length       = 0;
	size_t arg_merge_length          = 0;
	size_t arg_deleteclass_length    = 0;
	size_t arg_deleteproperty_length = 0;
	size_t arg_renameclass_length    = 0;
	size_t arg_renameproperty_length = 0;
	size_t arg_renameto_length       = 0;
	size_t arg_offset_length         = 0;
	
	for (size_t i=2; i<argument_num; i+=2) {
		switch(argument_hash[i]) {
			case NAMED_ARG_FILE :
				arg_filename        = argument[i+1];
				arg_filename_length = argument_length[i+1];
				break;
				
			case NAMED_ARG_MERGE : 
				arg_merge        = argument[i+1];
				arg_merge_length = argument_length[i+1];
				break;
				
			case NAMED_ARG_PATH :
				arg_classpath        = argument[i+1];
				break;
				
			case NAMED_ARG_DELETECLASS : 
				arg_deleteclass        = argument[i+1];
				arg_deleteclass_length = argument_length[i+1];
				break;
				
			case NAMED_ARG_DELETEPROPERTY : 
				arg_deleteproperty        = argument[i+1];
				arg_deleteproperty_length = argument_length[i+1];
				break;
				
			case NAMED_ARG_RENAMECLASS : 
				arg_renameclass        = argument[i+1];
				arg_renameclass_length = argument_length[i+1];
				break;
				
			case NAMED_ARG_RENAMEPROPERTY : 
				arg_renameproperty        = argument[i+1];
				arg_renameproperty_length = argument_length[i+1];
				break;
				
			case NAMED_ARG_TO : 
				arg_renameto        = argument[i+1];
				arg_renameto_length = argument_length[i+1];
				break;
				
			case NAMED_ARG_OFFSET :
				arg_offset        = argument[i+1];
				arg_offset_length = argument_length[i+1];
				break;
		}
	}
	
	// File not specified
	if (arg_filename_length == 0) {
		QWrite_err(FWERROR_PARAM_EMPTY, 1, "arg_filename");
		QWrite("0,\"0\",[],[]]");
		break;
	};
	
	// Verify and update path to the file
	String buf_filename;
	String_init(buf_filename);
	char *ptr_filename = arg_filename;

	if (!VerifyPath(&ptr_filename, buf_filename, OPTION_ALLOW_GAME_ROOT_DIR)) {
		QWrite("[],[],0]");
		break;
	}
		


	// Open wanted file -----------------------------------------------------------------	
	FILE *file = fopen(ptr_filename, "rb");
	if (!file) {
		QWrite_err(FWERROR_ERRNO, 2, errno, ptr_filename);
		QWrite("[],[],0]");
		String_end(buf_filename);
		break;
	}

	// Find file size
	if (fseek(file, 0, SEEK_END) != 0) {
		QWrite_err(FWERROR_ERRNO, 2, errno, ptr_filename);
		QWrite("[],[],0]");
		String_end(buf_filename);
		fclose(file);
		break;
	};

	size_t file_size = ftell(file);
	if (file_size == 0xFFFFFFFF) {
		QWrite_err(FWERROR_ERRNO, 2, errno, ptr_filename);
		QWrite("[],[],0]");
		String_end(buf_filename);
		fclose(file);
		break;
	};

	// Allocate buffer
	String file_contents;
	String_init(file_contents);
	int buffer_max = file_size + 1 + arg_merge_length;
	int result     = String_allocate(file_contents, buffer_max);
	if (result != 0) {
		QWrite_err(FWERROR_MALLOC, 2, "file_contents", buffer_max);
		QWrite("[],[],0]");
		String_end(buf_filename);
		break;
	}

	// Copy text to buffer
	fseek(file, 0, SEEK_SET);
	size_t bytes_read    = fread(file_contents.text, 1, file_size, file);
	file_contents.length = bytes_read;

	if (bytes_read != file_size) {		
		if (ferror(file))
			QWrite_err(FWERROR_ERRNO, 2, errno, ptr_filename);
		else
			QWrite_err(FWERROR_FILE_WRITE, 4, errno, bytes_read, file_size, ptr_filename);
		
		String_end(buf_filename);
		String_end(file_contents);
		QWrite("[],[],0]");
		fclose(file);
		break;
	}

	fclose(file);
	
	SQM_ParseState source_state;
	SQM_Init(source_state);
	
	int classpath_current = 0;
	int classpath_size    = 0;
	char *classpath[SQM_CLASSPATH_CAPACITY];
	char *classpath_item = strtok(arg_classpath, "[,]");  
	
	while (classpath_item!=NULL  &&  classpath_size<SQM_CLASSPATH_CAPACITY) {
		classpath[classpath_size++] = classpath_item;
		classpath_item = strtok(NULL, "[,]");
	}
	
	if (arg_offset_length != 0) {
		source_state.i    = strtoul(arg_offset, NULL, 0);
		classpath_current = classpath_size;
	} else {
		for (int i=0; i<classpath_size; i++) {
			if (SQM_Parse(file_contents.text, file_contents.length, source_state, SQM_ACTION_FIND_CLASS, classpath[i], strlen(classpath[i]))) {
				classpath_current++;
			} else
				break;
				
		}
	}
	
	// If classes given in the path were found
	if (classpath_current == classpath_size) {
		result            = 0;
		bool save_changes = false;
		
		// Merge classes	
		if (arg_merge_length > 0) {
			SQM_ParseState merge_state;
			SQM_Init(merge_state);

			if (SQM_Merge(arg_merge, arg_merge_length, merge_state, file_contents, source_state))
				save_changes = true;
		}
		
		// Delete property
		if (arg_deleteproperty_length > 0) {
			SQM_ParseState source_state_copy = source_state;
	
			if ((result = SQM_Parse(file_contents.text, file_contents.length, source_state_copy, SQM_ACTION_FIND_PROPERTY, arg_deleteproperty, arg_deleteproperty_length))) {
				size_t removed_length = source_state_copy.value_end - source_state_copy.property_start;
				
				shift_buffer_chunk(file_contents.text, source_state_copy.value_end, file_contents.length, removed_length, OPTION_LEFT);
				
				file_contents.length -= removed_length;
				save_changes          = true;
			} else {
				QWrite_err(FWERROR_CLASS_NOVAR, 2, arg_renameproperty, ptr_filename);
				goto class_write_end;
			}
		}
		
		// Delete class
		if (arg_deleteclass_length > 0) {				
			SQM_ParseState source_state_copy = source_state;
	
			if ((result = SQM_Parse(file_contents.text, file_contents.length, source_state_copy, SQM_ACTION_FIND_CLASS, arg_deleteclass, arg_deleteclass_length))) {
				SQM_Parse(file_contents.text, file_contents.length, source_state_copy, SQM_ACTION_FIND_CLASS_END, NULL, 0);
				
				size_t removed_length = source_state_copy.i - source_state_copy.class_start;
				
				shift_buffer_chunk(file_contents.text, source_state_copy.i, file_contents.length, removed_length, OPTION_LEFT);
				
				file_contents.length -= removed_length;
				save_changes          = true;
			} else {
				QWrite_err(FWERROR_CLASS_NOCLASS, 2, arg_renameclass, ptr_filename);
				goto class_write_end;
			}
		}
		
		// Rename property
		if (arg_renameproperty_length>0  &&  arg_renameto_length>0) {
			SQM_ParseState source_state_copy = source_state;
	
			if ((result = SQM_Parse(file_contents.text, file_contents.length, source_state_copy, SQM_ACTION_FIND_PROPERTY, arg_renameproperty, arg_renameproperty_length))) {
				size_t shift_amount  = arg_renameto_length > source_state_copy.property_length ? arg_renameto_length-source_state_copy.property_length : source_state_copy.property_length-arg_renameto_length;
				bool shift_direction = arg_renameto_length > source_state_copy.property_length;
				save_changes         = true;
				
				shift_buffer_chunk(file_contents.text, source_state_copy.property_end, file_contents.length, shift_amount, shift_direction);
				memcpy(source_state_copy.property, arg_renameto, arg_renameto_length);
				
				if (shift_direction)
					file_contents.length += shift_amount;
				else
					file_contents.length -= shift_amount;
			} else {
				QWrite_err(FWERROR_CLASS_NOVAR, 2, arg_renameproperty, ptr_filename);
				goto class_write_end;
			}
		} else
			// Rename class
			if (arg_renameclass_length > 0  &&  arg_renameto_length > 0) {
				SQM_ParseState source_state_copy = source_state;
				
				if ((result = SQM_Parse(file_contents.text, file_contents.length, source_state_copy, SQM_ACTION_FIND_CLASS, arg_renameclass, arg_renameclass_length))) {
					size_t current_name_length = source_state_copy.class_name_full_end - source_state_copy.class_name_start;
					size_t shift_amount        = arg_renameto_length > current_name_length ? arg_renameto_length-current_name_length : current_name_length-arg_renameto_length;
					bool shift_direction       = arg_renameto_length > current_name_length;
					save_changes               = true;
					
					shift_buffer_chunk(file_contents.text, source_state_copy.class_name_full_end, file_contents.length, shift_amount, shift_direction);
					memcpy(source_state_copy.class_name, arg_renameto, arg_renameto_length);
					
					if (shift_direction)
						file_contents.length += shift_amount;
					else
						file_contents.length -= shift_amount;
				} else {
					QWrite_err(FWERROR_CLASS_NOCLASS, 2, arg_renameclass, ptr_filename);
					goto class_write_end;
				}
			}
		
		// Rewrite file
		if (save_changes) {
			if ((file = fopen(ptr_filename, "wb"))) {
				fwrite(file_contents.text, 1, file_contents.length, file);
	
				if (ferror(file)) 
					QWrite_err(FWERROR_ERRNO, 2, errno, ptr_filename);
				else
					QWrite_err(FWERROR_NONE, 0);
	
				fclose(file);
			} else 
				QWrite_err(FWERROR_ERRNO, errno, ptr_filename);
		} else 
			if (!result && arg_merge_length==0)
				QWrite_err(FWERROR_PARAM_ACTION, 0);
			else
				QWrite_err(FWERROR_NONE, 0);
	} else
		if (classpath_current < classpath_size)
			QWrite_err(FWERROR_CLASS_PARENT, 4, classpath[classpath_current], classpath_current, classpath_size, ptr_filename);
		else
			QWrite_err(FWERROR_NONE, 0);

	class_write_end:
	QWritef("%d]", classpath_current);
	
	String_end(file_contents);
	String_end(buf_filename);
} break;