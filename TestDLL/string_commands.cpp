// -----------------------------------------------------------------
// STRING OPERATIONS
// -----------------------------------------------------------------

case C_STRING_FIRST:
{ // Return the index in str2 of the first occurrence of str1, or -1 if str1 is not found.
	if(argument_num < 4) {
		QWrite("ERROR: Not enough parameters");
		break;
	}

	char *str1 = stripq(argument[2]);
	char *str2 = stripq(argument[3]);
	char *res = strstr(str2, str1);
	
	if(!res) {
		// It wasn't found
		QWrite("-1");
	} else {
		// Found, return relative pointer
		QWritef("%d", res - str2);
	}
}
break;






case C_STRING_LAST:
{ // Return the index in str2 of the last occurrence of str1, or -1 if str1 is not found.
	if(argument_num < 4) {
		QWrite("ERROR: Not enough parameters");
		break;
	}

	char *str1 = stripq(argument[2]);
	char *str2 = stripq(argument[3]);
	char *str2t = stripq(argument[3]);

	char *res = 0, *res2 = 0;
	do {
		// Search substring until its not found anymore
		res = res2;
		res2 = strstr(str2t, str1);
		str2t = res2+1;
	} while (res2);
	
	if(!res) {
		// It wasn't found at all
		QWrite("-1");
	} else {
		// Found, return relative pointer
		QWritef("%d", res - str2);
	}
}
break;






case C_STRING_LENGTH:
{ // Return the number of characters in string.
	if(argument_num < 3) {
		QWrite("ERROR: Not enough parameters");
		break;
	}

	char *str = stripq(argument[2]);
	int res = strlen(str);
	QWritef("%d", res);
}
break;






case C_STRING_RANGE:
{ // Return the range of characters in str from i to j.
	if(argument_num < 5) {
		QWrite("ERROR: Not enough parameters");
		break;
	}

	char *str = stripq(argument[2]);
	unsigned int l = strlen(str);
	unsigned int i = atoi(argument[3]);
	unsigned int j = atoi(argument[4]);

	if(i > l) i = l;
	if(j > l) j = l;

	str[j] = '\0';
	QWritef("\"%s\"", str+i);
}
break;






case C_STRING_TOLOWER:
{ // Return string in lower case.
	if(argument_num < 3) {
		QWrite("ERROR: Not enough parameters");
		break;
	}

	char *str = stripq(argument[2]);
	unsigned int l = strlen(str);

	for(unsigned int x=0;x < l;x++)
		str[x] = tolower(str[x]);

	QWritef("\"%s\"", str);
}
break;






case C_STRING_TOUPPER:
{ // Return string in upper case.
	if(argument_num < 3) {
		QWrite("ERROR: Not enough parameters");
		break;
	}

	char *str = stripq(argument[2]);
	unsigned int l = strlen(str);

	for(unsigned int x=0;x < l;x++)
		str[x] = toupper(str[x]);

	QWritef("\"%s\"", str);
}
break;






case C_STRING_TOARRAY:
{ // Return all characters in string as array elements
	if(argument_num < 3) {
		QWrite("ERROR: Not enough parameters");
		break;
	}

	char *str = stripq(argument[2]);
	unsigned int l = strlen(str);

	QWrite("[");

	for (unsigned int x=0;x < l;x++) {
		if (str[x]=='"') 
			QWrite("\"\"\"\"");
		else 
			QWritef("\"%c\"", str[x]);//v1.11 fixed bug with quotes

		if (x < l-1)
			QWrite(",");
	}

	QWrite("];");
}
break;






case C_STRING_INDEX:
{ // Return the character at the specified index.
	if(argument_num < 4) {
		QWrite("ERROR: Not enough parameters");
		break;
	}

	unsigned int idx = atoi(argument[3]);
	argument[2] = stripq(argument[2]);

	if (idx > (strlen(argument[2])-1) || idx < 0) 
		QWrite("ERROR: Index out of bounds");
	else
		// Write char to file
		QWritef("\"%c\"", argument[2][idx]);
}
break;










case C_STRING_SIZE:
{ // Return the number of characters in a string
	
	QWritef("%d", argument_end - argument_length[0] - argument_length[1] - 2);
}
break;










case C_STRING_REPLACE:
{ // Replace text in a given string

	String arg_source       = {empty_string, 0};
	String arg_to_find      = {empty_string, 0};
	String arg_replace_with = {empty_string, 0};
	int arg_options         = OPTION_NONE;

	for (size_t i=2; i<argument_num; i+=2) {
		switch (argument_hash[i]) {
			case NAMED_ARG_TEXT : 
				arg_source.text   = argument[i+1];
				arg_source.length = argument_length[i+1];
				break;

			case NAMED_ARG_FIND : 
				arg_to_find.text   = argument[i+1];
				arg_to_find.length = argument_length[i+1];
				break;

			case NAMED_ARG_REPLACE : 
				arg_replace_with.text   = argument[i+1];
				arg_replace_with.length = argument_length[i+1];
				break;

			case NAMED_ARG_CASESENSITIVE : 
				String2Bool(argument[i+1]) 
					? arg_options |= OPTION_CASESENSITIVE 
					: arg_options &= ~OPTION_CASESENSITIVE;
				break;

			case NAMED_ARG_MATCHWORD : 
				String2Bool(argument[i+1]) 
					? arg_options |= OPTION_MATCHWORD 
					: arg_options &= ~OPTION_MATCHWORD;
				break;
		}
	}


	char *found     = NULL;
	char *found_end = arg_source.text;
	String temp     = arg_source;
	
	while ((found = strstr2(temp, arg_to_find, arg_options))) {
		QWritel(found_end, found-found_end);
		QWrite(arg_replace_with.text);

		found_end = found + arg_to_find.length;

		temp.text   = found_end;
		temp.length = arg_source.length-(found_end-arg_source.text);
	}
	
	if (found_end < arg_source.text+arg_source.length)
		QWritel(found_end, arg_source.text + arg_source.length - found_end);
}
break;










case C_STRING_COMPARE:
{ // Compare two strings with each other

	char *arg_text1         = empty_string;
	char *arg_text2         = empty_string;
	bool arg_case_sensitive = false;
	bool arg_natural_sort   = false;
	bool arg_reverse_case   = false;

	for (size_t i=2; i<argument_num; i+=2) {
		switch (argument_hash[i]) {
			case NAMED_ARG_TEXT1 : 
				arg_text1 = argument[i+1]; 
				break;

			case NAMED_ARG_TEXT2 : 
				arg_text2 = argument[i+1]; 
				break;

			case NAMED_ARG_CASESENSITIVE : 
				arg_case_sensitive = String2Bool(argument[i+1]); 
				break;

			case NAMED_ARG_NATURAL : 
				arg_natural_sort = String2Bool(argument[i+1]); 
				break;

			case NAMED_ARG_REVERSECASE : 
				arg_reverse_case = String2Bool(argument[i+1]); 
				break;
		}
	}


	if (arg_reverse_case) {
		for (int i=0; arg_text1[i]!='\0'; i++)
			arg_text1[i] = isupper(arg_text1[i]) ? tolower(arg_text1[i]) : toupper(arg_text1[i]);

		for (i=0; arg_text2[i]!='\0'; i++)
			arg_text2[i] = isupper(arg_text2[i]) ? tolower(arg_text2[i]) : toupper(arg_text2[i]);
	}

	QWritef("%d", 
		arg_natural_sort 
			? arg_case_sensitive 
				? strnatcasecmp(arg_text1, arg_text2) 
				: strnatcmp(arg_text1, arg_text2)
			: arg_case_sensitive 
				? strcmp(arg_text1, arg_text2) 
				: strcmpi(arg_text1, arg_text2)
	);
}
break;










case C_STRING_ISNUMBER:
{ // Check if string is a number

	char *arg_text        = empty_string;
	bool arg_skip_math_op = false;

	for (size_t i=2; i<argument_num; i+=2) {
		switch (argument_hash[i]) {
			case NAMED_ARG_TEXT : 
				arg_text = argument[i+1];
				break;

			case NAMED_ARG_SKIPMATH : 
				arg_skip_math_op = String2Bool(argument[i+1]);
				break;
		}
	}


	char result_name[][8]    = {"string","integer","float"};
	int index                = 0;
	int number_of_dots       = 0;
	bool skip_character      = false;
	bool found_number        = false;
	bool found_non_white     = false;
	bool found_space_between = false;

	for (i=0; arg_text[i]!='\0'; i++) {
		// Ignore whitespace
		if (isspace(arg_text[i])) {
			// Remember if there was space between characters
			if (found_non_white  &&  !arg_skip_math_op) 
				found_space_between = true;

			continue;
		}

		// If encountered a math operator
		switch (arg_text[i]) {
			case '+' : 
			case '-' : 
			case '*' : 
			case '/' : 
			case '%' : 
			case '^' : skip_character=true; break;
			default	 : found_non_white=true;
		}

		// Ignore operators if argument was passed
		if (skip_character) {
			if (arg_skip_math_op) {
				skip_character = false; 
				continue;
			}

			found_space_between = false;
		}

		// If there is a dot -  it could be a float
		if (arg_text[i] == '.') {
			number_of_dots++;

			if (number_of_dots>1  &&  !arg_skip_math_op) {
				index = 0; 
				break;
			} else 
				continue;
		}

		// If there is a minus - it could be a negative number
		if (arg_text[i] == '-') {
			if (found_number) {
				index = 0; 
				break;
			}

			continue;
		}


		// If current character is a digit
		if (isdigit(arg_text[i])  &&  !found_space_between) {
			found_number = true;

			// If previous character was dot - it could be a float
			if (i>0  &&  arg_text[i-1]=='.')
				index = 2;

			// If it wasn't already set to 'float' then assume it's integer
			if (index != 2) 
				index = 1;
		} else {
			index = 0;
			break;
		}
	}

	// If dot occured without a number then it's a string
	if (number_of_dots>0  &&  !found_number) 
		index = 0;

	QWrite(result_name[index]);
}
break;










case C_STRING_ISVARIABLE:
{ // Check if string is a compliant OFP variable name

	enum STRING_ISVARIABLE_MODES {
		DEFAULT,
		ONLY_GLOBAL,
		ONLY_LOCAL,
		CLASS_NAME,
		CLASS_NAME_INHERIT,
		CLASS_TOKEN
	};

	char *arg_text         = empty_string;
	int arg_mode           = DEFAULT;
	size_t arg_text_length = 0;

	for (size_t i=2; i<argument_num; i+=2) {
		switch (argument_hash[i]) {
			case NAMED_ARG_TEXT : 
				arg_text        = argument[i+1]; 
				arg_text_length = argument_length[i+1]; 
				break;

			case NAMED_ARG_MODE : 
				char modes[][32] = {
					"default",
					"onlyGlobal",
					"onlyLocal",
					"className",
					"classNameInherit",
					"classToken"
				};

				for (int j=0, max=sizeof(modes)/sizeof(modes[0]);  j<max;  j++)
					if (strcmpi(argument[i+1],modes[j]) == 0) {
						arg_mode = j;
						break;
					}
				break;
		}
	}

	if (arg_text_length == 0) {
		QWrite("false"); 
		break;
	}


	int result         = true;  // return value
	bool allow_global  = true;	// allow global variables
	bool allow_local   = true;	// allow local variables
	bool allow_number  = false;	// allow the name to start with a number
	bool allow_bracket = false;	// allow square brackets at the end
	bool allow_inherit = false;	// allow second class name after colon
	char *class1       = arg_text;
	char *class2       = empty_string;

	if (arg_mode == ONLY_LOCAL)
		allow_global = false;

	if (arg_mode == ONLY_GLOBAL)
		allow_local = false;

	if (arg_mode==CLASS_NAME  ||  arg_mode==CLASS_TOKEN  ||  arg_mode==CLASS_NAME_INHERIT) {
		allow_global  = true;
		allow_local   = true;
		allow_number  = true;
		allow_bracket = arg_mode == CLASS_NAME;
		allow_inherit = arg_mode == CLASS_NAME_INHERIT;

		// Keyword "class" is reserved
		if (strncmpi(arg_text,"class ",6)==0  ||  strncmpi(arg_text,"class=",6)==0) {
			QWrite("false"); 
			break;
		}

		// If there are two class names then separate
		if (allow_inherit)
			if ((class2 = strchr(arg_text, ':'))) {
				int pos       = class2 - arg_text;
				arg_text[pos] = '\0';
				class1        = Trim(arg_text);
				class2        = Trim(class2 + 1);
			}
	}

	// Optional mode - remove square brackets at the end
	if (allow_bracket  &&  arg_text_length>2)
		if (arg_text[(arg_text_length-2)]=='['  &&  arg_text[(arg_text_length-1)]==']')
			arg_text[(arg_text_length-2)] = '\0';


	for (i=0;  result;  i++) {
		// End loop on string end OR check second string if it exists
		if (i == arg_text_length) {
			if (class2!=NULL  &&  arg_text!=class2) {
				i	     = 0;
				arg_text = class2;
			} else
				break;
		}

		// Fail conditions for the first character
		if (i == 0)	{						
			if (
				// DEFAULT - first char is (not alpha assuming digits aren't allowed OR not alpha and not a digit assuming allowed digits) and not underscore
				(allow_global  &&   allow_local  &&  ((!isalpha(arg_text[0]) && !allow_number) || (!isalpha(arg_text[0]) && !isdigit(arg_text[0]) && allow_number))  &&  arg_text[0]!='_')
				||	
				// GLOBALS ONLY - first char is (not alpha with digits not allowed OR not alpha and not digit with allowed digits)
				(allow_global  &&  !allow_local  &&  ((!isalpha(arg_text[0]) && !allow_number) || (!isalpha(arg_text[0]) && !isdigit(arg_text[0]) && allow_number)))
				||
				// LOCALS ONLY - first char is not underscore
				(!allow_global &&  allow_local  &&  arg_text[0]!='_')
			) 
				result = false;
		// If character isn't alphanumeric and isn't underscore then fail
		} else
			if (!isalnum(arg_text[i])  &&  arg_text[i]!='_')
				result = false;
	}

	QWrite(getBool(result));
}
break;










case C_STRING_ISEMPTY:
{ // Check if string consists solely of white-space

	QWrite(getBool(argument_num<=2));
}
break;










case C_STRING_CUT:
{ // Return part of a string

	String arg_text             = {empty_string, 0};
	String arg_search_start     = {empty_string, 0};
	String arg_search_end       = {empty_string, 0};
	char *search_start          = empty_string;
	char *search_end            = empty_string;
	char *arg_start             = empty_string;
	char *arg_end               = empty_string;
	char *arg_len               = empty_string;
	size_t arg_offset_start     = 0;
	size_t arg_offset_end       = 0;
	bool arg_offset_start_right = 1;
	bool arg_offset_end_right   = 1;
	int arg_options             = OPTION_NONE;
	bool arg_crop_mode          = false;

	for (size_t i=2; i<argument_num; i+=2) {
		switch (argument_hash[i]) {
			case NAMED_ARG_TEXT : 
				arg_text.text   = argument[i+1]; 
				arg_text.length = argument_length[i+1]; 
				break;

			case NAMED_ARG_START : 
				arg_start = argument[i+1];
				break;

			case NAMED_ARG_END : 
				arg_end = argument[i+1]; 
				break;

			case NAMED_ARG_LENGTH : 
				arg_len = argument[i+1]; 
				break;

			case NAMED_ARG_EXCLUDE : 
				arg_crop_mode = String2Bool(argument[i+1]); 
				break;

			case NAMED_ARG_STARTFIND : 
				arg_search_start.text   = argument[i+1]; 
				arg_search_start.length = argument_length[i+1];
				break;

			case NAMED_ARG_ENDFIND : 
				arg_search_end.text   = argument[i+1]; 
				arg_search_end.length = argument_length[i+1];
				break;

			case NAMED_ARG_MATCHWORD : 
				String2Bool(argument[i+1]) 
					? arg_options |= OPTION_MATCHWORD 
					: arg_options &= ~OPTION_MATCHWORD;
				break;

			case NAMED_ARG_CASESENSITIVE : 
				String2Bool(argument[i+1]) 
					? arg_options |= OPTION_CASESENSITIVE 
					: arg_options &= ~OPTION_CASESENSITIVE;
				break;

			case NAMED_ARG_STARTOFFSET : 
				if (argument[i+1][0] == '-')
					arg_offset_start_right = 0;
				arg_offset_start = strtoul(argument[i+1], NULL, 0);
				break;

			case NAMED_ARG_ENDOFFSET : 
				if (argument[i+1][0] == '-')
					arg_offset_start_right = 0;
				arg_offset_end = strtoul(argument[i+1], NULL, 0);
				break;
		}
	}

	if (arg_text.length == 0)
		break;

	StringPos range = ConvertStringPos(arg_start, arg_end, arg_len, arg_text.length);


	// Search for the start and end position
	if (arg_search_start.length > 0) {
		char *start_ptr = strstr2(arg_text, arg_search_start, arg_options);
		if (!start_ptr)
			break;

		range.start = start_ptr - arg_text.text;

		if (arg_search_end.length > 0) {
			String search_start = {start_ptr, arg_text.length-range.start};
			char *end_ptr = strstr2(search_start, arg_search_end, arg_options);
			if (end_ptr) {
				range.end = end_ptr - arg_text.text;
			}
		}
	}

	range.start += arg_offset_start * (arg_offset_start_right ? 1 : -1);
	range.end   += arg_offset_end * (arg_offset_end_right ? 1 : -1);


	// Default mode - return range of characters
	if (!arg_crop_mode) {
		if (range.start <= range.end)
			QWritel(arg_text.text+range.start, range.end-range.start);
		else
			// Reverse range - iterate from the end
			for (--range.start;  range.start>=range.end;  range.start--)
				QWritef("%c", arg_text.text[range.start]);
	} else {
		// Crop mode - remove range of characters
		if (range.start <= range.end) {
			shift_buffer_chunk(arg_text.text, range.end, arg_text.length, range.end-range.start, OPTION_LEFT);
			QWritel(arg_text.text, arg_text.length-(range.end-range.start));
		} else {
			// Reverse range - iterate characters from the end
			for (size_t i=arg_text.length-1; i>0; i--)
				if (i<range.end  ||  i>=range.start)
					QWritef("%c", arg_text.text[i]);
		}
	}
}
break;










case C_STRING_TOKENIZE:
{ // Split string on given occurences

	String arg_text      = {empty_string, 0};
	String arg_delimiter = {empty_string, 0};
	String arg_add_empty = {empty_string, 0};
	bool arg_by_string   = false;
	bool arg_skip_quotes = false;
	int arg_options      = 0;

	for (size_t i=2; i<argument_num; i+=2) {
		switch (argument_hash[i]) {
			case NAMED_ARG_TEXT : 
				arg_text.text   = argument[i+1]; 
				arg_text.length = argument_length[i+1]; 
				break;

			case NAMED_ARG_DELIMITER : 
				arg_delimiter.text   = argument[i+1]; 
				arg_delimiter.length = argument_length[i+1]; 
				break;

			case NAMED_ARG_WORDDELIMITER : 
				arg_by_string = String2Bool(argument[i+1]); 
				break;

			case NAMED_ARG_QUOTESKIP : 
				arg_skip_quotes = String2Bool(argument[i+1]); 
				break;

			case NAMED_ARG_MATCHWORD : 
				String2Bool(argument[i+1]) 
					? arg_options |= OPTION_MATCHWORD 
					: arg_options &= ~OPTION_MATCHWORD;
				break;

			case NAMED_ARG_CASESENSITIVE : 
				String2Bool(argument[i+1]) 
					? arg_options |= OPTION_CASESENSITIVE 
					: arg_options &= ~OPTION_CASESENSITIVE;
				break;

			case NAMED_ARG_ADDEMPTY : 
				arg_add_empty.text   = argument[i+1];
				arg_add_empty.length = argument_length[i+1];
				break;
		}
	}


	// If we're skipping occurences in quotes then first we need to save position of quot marks
	size_t Qmax    = 0;
	size_t Qindex  = -2;
	size_t *Qarray;

	if (arg_skip_quotes) {
		// Count how many quotes there are
		for (size_t i=0;  i<arg_text.length;  i++)
			if (arg_text.text[i] == '"')
				Qmax++;

		// Even number
		if (Qmax % 2 != 0)
			Qmax++;

		// Allocate integer array
		if (Qmax > 0) {
			Qarray = (size_t*) calloc (Qmax, sizeof(int));

			if (!Qarray) {
				QWrite("[]");
				break;
			}
		}

		// Store quotes position in an array
		bool in_quote = false;

		for (i=0;  i<arg_text.length && Qindex<Qmax;  i++)
			if (arg_text.text[i] == '"') {
				in_quote = !in_quote;

				if (in_quote) {
					Qindex          += 2;
					Qarray[Qindex]   = i;
					Qarray[Qindex+1] = arg_text.length;
				} else 
					Qarray[Qindex+1] = i;
			}
	}


	// Prepare text before tokenizing by words
	String search_source = arg_text;
	size_t skip          = 0;
	char *occurence;

	if (arg_by_string) {
		// Search for the word
		while ((occurence = strstr2(search_source, arg_delimiter, arg_options))) {
			size_t pos    = occurence - search_source.text;
			bool in_quote = false;

			// Check if the word is inside quotation
			if (arg_skip_quotes  &&  Qmax>0) {
				for (size_t i=0;  i<Qmax && !in_quote;  i+=2)
					if (Qarray[i]<=(skip+pos)  &&  (skip+pos)<=Qarray[i+1]) 
						in_quote = true;

				// If so then skip it
				if (in_quote) {
					skip                += pos + arg_delimiter.length; 
					search_source.text   = arg_text.text + skip;
					search_source.length = arg_text.length - skip;
					continue;
				}
			}
			
			// Replace the word with unused characters
			for (size_t i=pos; i<(pos+arg_delimiter.length); i++) 
				search_source.text[i] = '\a';
		}

	// Prepare text before tokenizing by single characters
	} else {
		// For each char in delim
		for (size_t i=0; i<arg_delimiter.length; i++) {
			search_source  = arg_text;
			skip           = 0;
			String to_find = {arg_delimiter.text + i, 1};

			while ((occurence = strstr2(search_source, to_find, arg_options))) {
				size_t pos    = occurence - search_source.text;
				bool in_quote = false;

				// Check if the character is inside quotation
				if (arg_skip_quotes) {
					for (size_t i=0;  i<Qmax && !in_quote;  i+=2)
						if (Qarray[i]<=(skip+pos)  &&  (skip+pos)<=Qarray[i+1]) 
							in_quote = true;

					// If so then skip it
					if (in_quote) {
						skip                += pos + 1; 
						search_source.text   = arg_text.text + skip;
						search_source.length = arg_text.length - skip;
						continue;
					}
				}

				// Replace char with an unused char
				search_source.text[pos] = '\a';
			}
		}
	}

	if (Qmax > 0)
		free(Qarray);


	// Set variables for adding empty
	bool add_empty_start = false;
	bool add_empty_end   = false;
	size_t pch_len       = 0;
	size_t pos           = 0;
	size_t count         = 0;

	if (strncmpi(arg_add_empty.text,"both",4)==0) {
		add_empty_start = true;
		add_empty_end   = true;
	} else
		if (strncmpi(arg_add_empty.text,"start",5)==0)
			add_empty_start = true;
		else
			if (strncmpi(arg_add_empty.text,"end",3)==0)
				add_empty_end = true;


	// Start tokenization
	QWrite("[");
	occurence            = strtok(arg_text.text, "\a");
	size_t occurence_len = 0;

	while (occurence) {
		// Keep track of numbers
		if (add_empty_start  ||  add_empty_end) {
			pos           = occurence - arg_text.text;
			occurence_len = strlen(occurence);
		}

		// if string begins with token 
		if (add_empty_start) {
			add_empty_start = false;

			// then add empty string to the output array
			if (pos != 0)
				QWrite("\"\"");
		}
		
		QWrite("]+[\"");
		QWriteDoubleQ(occurence);
		QWrite("\"");

		count++;
		occurence = strtok(NULL, "\a");
	}


	// if there were no parts at all
	if (add_empty_start)
		QWrite("\"\"");

	// if last part doesn't end the string
	if (add_empty_end  &&  (pos+occurence_len<arg_text.length || count==0))
		QWrite("]+[\"\"");

	QWrite("]");
}
break;










case C_STRING_TRIM:
{ // Trim white-space from string

	char *arg_text         = empty_string;
	size_t arg_text_length = 0;
	bool arg_left          = false;
	bool arg_middle        = false;
	bool arg_right         = false;
	bool arg_middle_single = false;

	for (size_t i=2; i<argument_num; i+=2) {
		switch (argument_hash[i]) {
			case NAMED_ARG_LEFT : 
				arg_left = String2Bool(argument[i+1]); 
				break;

			case NAMED_ARG_MIDDLE : 
				arg_middle        = String2Bool(argument[i+1]); 
				arg_middle_single = !arg_middle; 
				break;

			case NAMED_ARG_MIDDLESINGLE : 
				arg_middle_single = String2Bool(argument[i+1]); 
				arg_middle        = !arg_middle_single; 
				break;

			case NAMED_ARG_RIGHT : 
				arg_right = String2Bool(argument[i+1]); 
				break;

			case NAMED_ARG_TEXT : 
				arg_text        = argument[i+1]; 
				arg_text_length = argument_length[i+1];
				break;
		}
	}


	// Remove leading and trailing whitespaces
	while (arg_left && isspace(arg_text[0])) {
		arg_text++;
		arg_text_length++;
	}

	for (i=arg_text_length-1;  arg_right && i>=0 && isspace(arg_text[i]);  i--) {
		arg_text[i] = '\0';
		arg_text_length--;
	}


	// Remove whitespace in the middle of the string
	if (arg_middle || arg_middle_single) {
		// get starting position (ignore leading space)
		for (size_t i=0; isspace(arg_text[i]);  i++);

		// get ending position (ignore trailling space)
		for (size_t end=arg_text_length-1;  isspace(arg_text[end]);  end--);

		int count_space = 0;

		// shift chars to the left when encountered whitespace
		for (; i<end-1; i++)
			if (isspace(arg_text[i])) {
				count_space++;

				if (!arg_middle_single  ||  (arg_middle_single && count_space>1)) {
					shift_buffer_chunk(arg_text, i, i+1, 1, OPTION_LEFT);
					end--;
					arg_text_length--;
				}
			} else
				count_space = 0;
	}

	QWritel(arg_text, arg_text_length);
}
break;










case C_STRING_FIND:
{ // Search string

	String arg_text      = {empty_string, 0};
	String arg_find      = {empty_string, 0};
	char *arg_start      = empty_string;
	char *arg_end        = empty_string;
	char *arg_length     = empty_string;
	size_t arg_limit     = -1;
	int arg_options      = OPTION_NONE;
	bool arg_skip_quotes = false;

	for (size_t i=2; i<argument_num; i+=2) {
		switch (argument_hash[i]) {
			case NAMED_ARG_TEXT : 
				arg_text.text   = argument[i+1]; 
				arg_text.length = argument_length[i+1]; 
				break;

			case NAMED_ARG_FIND : 
				arg_find.text   = argument[i+1]; 
				arg_find.length = argument_length[i+1]; 
				break;

			case NAMED_ARG_MATCHWORD : 
				String2Bool(argument[i+1]) 
					? arg_options |= OPTION_MATCHWORD 
					: arg_options &= ~OPTION_MATCHWORD;
				break;

			case NAMED_ARG_CASESENSITIVE : 
				String2Bool(argument[i+1]) 
					? arg_options |= OPTION_CASESENSITIVE 
					: arg_options &= ~OPTION_CASESENSITIVE;
				break;

			case NAMED_ARG_START : 
				arg_start = argument[i+1]; 
				break;

			case NAMED_ARG_END : 
				arg_end = argument[i+1]; 
				break;

			case NAMED_ARG_LENGTH : 
				arg_length = argument[i+1]; 
				break;

			case NAMED_ARG_LIMIT : 
				arg_limit = strtoul(argument[i+1], NULL, 0);
				break;

			case NAMED_ARG_QUOTESKIP : 
				arg_skip_quotes = String2Bool(argument[i+1]); 
				break;
		}
	}



	// If we're skipping occurences in quotes then first we need to save position of quotation marks
	size_t Qmax    = 0;
	size_t Qindex  = -1;
	size_t *Qarray = NULL;
	bool in_quote  = false;

	if (arg_skip_quotes) {
		// Count how many quotes there are
		for (size_t i=0;  i<arg_text.length;  i++)
			if (arg_text.text[i] == '"')
				Qmax++;

		// Even number
		if (Qmax % 2 != 0)
			Qmax++;

		// Allocate integer array
		if (Qmax > 0) {
			Qarray = (size_t*) calloc (Qmax, sizeof(size_t));

			if (!Qarray) {
				QWrite("[]");
				break;
			}
		}

		// Store quotes position in an array
		in_quote = false;

		for (i=0;  i<arg_text.length && Qindex<Qmax;  i++)
			if (arg_text.text[i] == '"') {
				in_quote = !in_quote;

				if (in_quote) {
					Qindex          += 2;
					Qarray[Qindex]   = i;
					Qarray[Qindex+1] = in_quote;
				} else 
					Qarray[Qindex+1] = i;
			}
	}


	// Search
	QWrite("[");
	char *occurence;
	size_t count         = 0;
	size_t pos_absolute  = 0;
	size_t pos_relative  = 0;
	String search_source = arg_text;
	StringPos range      = ConvertStringPos(arg_start, arg_end, arg_length, arg_text.length);

	while ((occurence=strstr2(search_source, arg_find, arg_options))) {
		pos_relative = occurence - search_source.text;
		in_quote     = false;

		// If in range
		if (pos_absolute+pos_relative >= range.start) {
			// If passed the range
			if (pos_absolute+pos_relative > range.end)
				break;

			// If within result limit
			if (arg_limit>=0  &&  ++count>arg_limit)
				break;

			// Check if the word is inside quotation
			if (arg_skip_quotes  &&  Qmax>0)
				for (size_t i=0;  i<Qmax && !in_quote;  i+=2)
					if (Qarray[i]<=(pos_absolute+pos_relative)  &&  (pos_absolute+pos_relative)<=Qarray[i+1]) 
						in_quote = true;

			if (!in_quote)
				QWritef("]+[%d", pos_absolute + pos_relative);
		}

		// Move past the current occurence
		pos_absolute        += pos_relative + 1;
		search_source.text   = arg_text.text + pos_absolute;
		search_source.length = arg_text.length - pos_absolute;
	}

	if (Qarray)
		free(Qarray);

	QWrite("]");
}
break;










case C_STRING_SPLIT:
{ // Return string as an array

	char *arg_text            = empty_string;
	char *arg_start           = empty_string;
	char *arg_end             = empty_string;
	char *arg_length          = empty_string;
	size_t arg_segment_length = 1;
	size_t arg_limit          = -1;
	size_t arg_text_length    = 0;

	for (size_t i=2; i<argument_num; i+=2) {
		switch (argument_hash[i]) {
			case NAMED_ARG_TEXT : 
				arg_text        = argument[i+1]; 
				arg_text_length = argument_length[i+1]; 
				break;

			case NAMED_ARG_SIZE : 
				arg_segment_length = strtoul(argument[i+1], NULL, 0);
				break;

			case NAMED_ARG_LIMIT : 
				arg_limit = strtoul(argument[i+1], NULL, 0); 
				break;

			case NAMED_ARG_START : 
				arg_start = argument[i+1]; 
				break;

			case NAMED_ARG_END : 
				arg_end = argument[i+1]; 
				break;

			case NAMED_ARG_LENGTH : 
				arg_length = argument[i+1];
				break;
		}
	}


	// Split
	QWrite("[");
	size_t count    = 0;
	size_t parts    = 0;
	StringPos range = ConvertStringPos(arg_start, arg_end, arg_length, arg_text_length);

	for (i=range.start;  i<range.end && arg_limit!=0;  i++) {
		// Start segment
		if (count == 0)
			QWrite("\"");

		// Output character
		if (arg_text[i] == '"') 
			QWrite("\"\""); 
		else
			QWritef("%c", arg_text[i]);

		// Limit segment length
		if (++count >= arg_segment_length  ||  i==range.end-1) {
			QWrite("\"");

			count = 0;
			QWrite("]+[");

			parts++;

			if (arg_limit>=0  &&  parts>=arg_limit)
				break;
		}
	}

	QWrite("]");
}
break;










case C_STRING_DOMAIN:
{ // Tokenize URL
	
	if (argument_num < 3)
		break;

	// Search for protocol and world wide web
	size_t http_pos = 0;
	size_t www_pos  = 0;
	char *http      = strstr(argument[2], "://");
	char *www       = strstr(argument[2], "www.");

	if (http) 
		http_pos = http - argument[2];

	if (www) {
		// Confirm www validity
		www_pos = www - argument[2];

		for (size_t i=www_pos-4;  i>0 && i>http_pos;  i++)
			if (isalnum(argument[2][i])) {
				www = NULL;
				break;
			}

		if (www_pos >= 0) 
			argument[2][www_pos+3] = '/';
	}


	// Find domain start position
	int domain_pos = 0;

	if (www) 
		domain_pos = www_pos + 4; 
	else 
		if (http_pos >= 0) 
			domain_pos = http_pos + 3;

	char *domain = argument[2] + domain_pos;


	// Find query string start position
	size_t query_pos   = -1;
	size_t directories = 0;
	char *qmark        = strchr(domain, '?');

	if (qmark) {
		query_pos = qmark - domain;

		for (size_t i=0; i<query_pos; i++) 
			if (domain[i]=='/'  &&  domain[i+1]!='?') 
				directories++;
	}


	// Tokenize whole address
	int  cnt   = 0;
	bool count = true;
	char *pch  = strtok(argument[2], ":/?&");

	QWrite("[[");
	
	while (pch) {
		if (strncmp(pch,domain,strlen(pch)) == 0) {
			cnt++;
			count = 0;
		}

		if (count) 
			cnt++;
		
		QWritef("]+[\"%s\"", pch); 
		pch = strtok(NULL, ":/?&");
	}


	// Return index where query string vars start
	if (qmark) 
		QWritef("],%u,[", cnt + directories);
	else
		QWrite("],-1,[");
	

	// Tokenize domain
	pch       = strtok(domain, ".");
	int parts = -1;

	while (pch) {
		parts++;
		QWritef("]+[\"%s\"", pch); 
		pch = strtok(NULL, ".");
	}


	// Reassemble domain and then return it
	QWrite("],\"");

	for (i=0; parts>0; i++)
		if (domain[i] == '\0') {
			domain[i] = '.';
			parts--;
		}

	QWritef("%s\"]", domain);
}
break;










case C_STRING_CASE:
{ // Convert to upper or lower case

	char *arg_text         = empty_string;
	char *arg_start        = empty_string;
	char *arg_end          = empty_string;
	char *arg_length       = empty_string;
	bool arg_sentences     = false;
	bool arg_words         = false;
	bool arg_upper         = false;
	bool arg_lower         = false;
	size_t arg_text_length = 0;

	for (size_t i=2; i<argument_num; i+=2) {
		switch (argument_hash[i]) {
			case NAMED_ARG_SENTENCES : 
				arg_sentences = String2Bool(argument[i+1]); 
				break;

			case NAMED_ARG_WORDS : 
				arg_words = String2Bool(argument[i+1]); 
				break;

			case NAMED_ARG_UPPER : 
				arg_upper = String2Bool(argument[i+1]); 
				break;

			case NAMED_ARG_LOWER : 
				arg_lower = String2Bool(argument[i+1]); 
				break;

			case NAMED_ARG_TEXT : 
				arg_text        = argument[i+1]; 
				arg_text_length = argument_length[i+1]; 
				break;

			case NAMED_ARG_START : 
				arg_start = argument[i+1];
				break;

			case NAMED_ARG_END : 
				arg_end = argument[i+1];
				break;

			case NAMED_ARG_LENGTH : 
				arg_length = argument[i+1];
				break;
		}
	}

	StringPos range = ConvertStringPos(arg_start, arg_end, arg_length, arg_text_length);
	bool capitalize = true;

	for (i=range.start; i<range.end; i++) {
		// Default mode - change all characters
		if (!arg_sentences && !arg_words) {
			if (arg_upper && !arg_lower) 
				arg_text[i] = toupper(arg_text[i]);

			if (!arg_upper && arg_lower) 
				arg_text[i] = tolower(arg_text[i]);

		// Special mode - change only the first character in sentences or words
		} else {
			if (capitalize  &&  (arg_words || arg_sentences && !isspace(arg_text[i]))) {
				capitalize = false;

				if (arg_upper) 
					arg_text[i] = toupper(arg_text[i]);
			} else
				if (arg_lower) 
					arg_text[i] = tolower(arg_text[i]);

			// separator between words/sentences
			if ((arg_words  &&  isspace(arg_text[i]))  ||  (arg_sentences  &&  (arg_text[i]=='.' || arg_text[i]=='?' || arg_text[i]=='!'))) 
				capitalize = true;
		}
	}

	QWritel(arg_text, arg_text_length);
}
break;










case C_STRING_REPLACECHAR:
{ // Replace single characters

	char *arg_text            = empty_string;
	char *arg_find            = empty_string;
	char *arg_replace         = empty_string;
	size_t arg_text_length    = 0;
	size_t arg_find_length    = 0;
	size_t arg_replace_length = 0;
	bool arg_correspond       = false;
	int arg_options           = OPTION_NONE;

	for (size_t i=2; i<argument_num; i+=2) {
		switch (argument_hash[i]) {
			case NAMED_ARG_TEXT : 
				arg_text        = argument[i+1]; 
				arg_text_length = argument_length[i+1]; 
				break;

			case NAMED_ARG_FIND : 
				arg_find        = argument[i+1]; 
				arg_find_length = argument_length[i+1]; 
				break;

			case NAMED_ARG_REPLACE : 
				arg_replace        = argument[i+1]; 
				arg_replace_length = argument_length[i+1]; 
				break;

			case NAMED_ARG_CORRESPOND : 
				arg_correspond = String2Bool(argument[i+1]); 
				break;

			case NAMED_ARG_CASESENSITIVE : 
				String2Bool(argument[i+1]) 
					? arg_options |= OPTION_CASESENSITIVE 
					: arg_options &= ~OPTION_CASESENSITIVE;
				break;

			case NAMED_ARG_MATCHWORD : 
				String2Bool(argument[i+1]) 
					? arg_options |= OPTION_MATCHWORD 
					: arg_options &= ~OPTION_MATCHWORD;
				break;
		}
	}


	for (i=0; i<arg_text_length; i++) {	
		char current = arg_text[i];

		// Optional: occurrence musn't be surrounded by alphanumeric characters
		if (arg_options & OPTION_MATCHWORD) {
			bool left_cleared  = i==0  ||  (i>0  &&  !isalnum(arg_text[i-1])  &&  arg_text[i-1]!='_');
			bool right_cleared = i>=arg_text_length  ||  (i<arg_text_length  &&  !isalnum(arg_text[i+1])  &&  arg_text[i+1]!='_');

			if (!left_cleared  ||  !right_cleared) {
				QWritef("%c", current);
				continue;
			}
		}

		// Search for replacement
		for (size_t j=0; j<arg_find_length; j++)
			if ((arg_options & OPTION_CASESENSITIVE  &&  arg_text[i]==arg_find[j])  ||  (~arg_options & OPTION_CASESENSITIVE  &&  tolower(arg_text[i])==tolower(arg_find[j])))
				current = !arg_correspond
					? (arg_replace_length > 0 ? arg_replace[0] : '\0')	// Default mode: replace if given third argument; otherwise remove
					: (j < arg_replace_length ? arg_replace[j] : '\0');	// Parallel mode: find corresponding character from the third argument; otherwise remove

		if (current != '\0')
			QWritef("%c", current);
	}
}
break;










case C_STRING_JOIN:
{ // Add characters in the midddle of the string

	char *arg_text          = empty_string;
	char *arg_merge         = empty_string;
	char *arg_position      = empty_string;
	size_t arg_text_length  = 0;
	size_t arg_merge_length = 0;

	for (size_t i=2; i<argument_num; i+=2) {
		switch (argument_hash[i]) {
			case NAMED_ARG_TEXT : 
				arg_text        = argument[i+1]; 
				arg_text_length = argument_length[i+1]; 
				break;

			case NAMED_ARG_MERGE : 
				arg_merge        = argument[i+1]; 
				arg_merge_length = argument_length[i+1]; 
				break;

			case NAMED_ARG_POSITION : 
				arg_position = argument[i+1];
				break;
		}
	}

	if (arg_text_length == 0) {
		QWritel(arg_merge, arg_merge_length);
		break;
	}

	if (arg_merge_length == 0) {
		QWritel(arg_text, arg_text_length);
		break;
	}

	StringPos range = ConvertStringPos(arg_position, empty_string, empty_string, arg_text_length);

	QWritel(arg_text, range.start);
	QWritel(arg_merge, arg_merge_length);
	QWritel(arg_text+range.start, arg_text_length-range.start);
}
break;










case C_STRING_WORDPOS:
{ // Find where words start

	char *arg_text         = empty_string;
	size_t arg_text_length = 0;

	for (size_t i=2; i<argument_num; i+=2) {
		switch (argument_hash[i]) {
			case NAMED_ARG_TEXT : 
				arg_text        = argument[i+1]; 
				arg_text_length = argument_length[i+1]; 
				break;
		}
	}

	QWrite("[");

	for (i=0; i<arg_text_length; i++) {
		// Current word
		int initial_type = GetCharType(arg_text[i]);

		// Output word position
		if (initial_type != CHAR_TYPE_SPACE)
			QWritef("]+[%u", i);

		// Iterate until current word ends
		while (i<arg_text_length  &&  GetCharType(arg_text[i])==initial_type)
			i++;

		// Skip spaces
		while (i<arg_text_length  &&  GetCharType(arg_text[i])==CHAR_TYPE_SPACE)
			i++;
	}

	QWrite("]");
}
break;