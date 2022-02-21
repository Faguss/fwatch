// -----------------------------------------------------------------
// LEGACY STRING COMMANDS
// -----------------------------------------------------------------

case C_STRING_FIRST:
{ // Return the index in str2 of the first occurrence of str1, or -1 if str1 is not found.
	if(argument_num < 4) {
		QWrite("ERROR: Not enough parameters");
		break;
	}

	String_trim_quotes(argument[2]);
	String_trim_quotes(argument[3]);
	char *res = strstr(argument[3].text, argument[2].text);
	
	if(!res) {
		// It wasn't found
		QWrite("-1");
	} else {
		// Found, return relative pointer
		QWritef("%d", res - argument[3].text);
	}
}
break;






case C_STRING_LAST:
{ // Return the index in str2 of the last occurrence of str1, or -1 if str1 is not found.
	if(argument_num < 4) {
		QWrite("ERROR: Not enough parameters");
		break;
	}

	String_trim_quotes(argument[2]);
	String_trim_quotes(argument[3]);
	char *str2t = argument[3].text;

	char *res = 0, *res2 = 0;
	do {
		// Search substring until its not found anymore
		res = res2;
		res2 = strstr(str2t, argument[2].text);
		str2t = res2+1;
	} while (res2);
	
	if(!res) {
		// It wasn't found at all
		QWrite("-1");
	} else {
		// Found, return relative pointer
		QWritef("%d", res - argument[3].text);
	}
}
break;






case C_STRING_LENGTH:
{ // Return the number of characters in string.
	if(argument_num < 3) {
		QWrite("ERROR: Not enough parameters");
		break;
	}

	String_trim_quotes(argument[2]);
	QWritef("%d", argument[2].length);
}
break;






case C_STRING_RANGE:
{ // Return the range of characters in str from i to j.
	if(argument_num < 5) {
		QWrite("ERROR: Not enough parameters");
		break;
	}

	String_trim_quotes(argument[2]);
	unsigned int i = atoi(argument[3].text);
	unsigned int j = atoi(argument[4].text);

	if(i > argument[2].length) i = argument[2].length;
	if(j > argument[2].length) j = argument[2].length;

	argument[2].text[j] = '\0';
	QWritef("\"%s\"", argument[2].text+i);
}
break;






case C_STRING_TOLOWER:
{ // Return string in lower case.
	if(argument_num < 3) {
		QWrite("ERROR: Not enough parameters");
		break;
	}

	String_trim_quotes(argument[2]);

	for(unsigned int x=0;x < argument[2].length;x++)
		argument[2].text[x] = tolower(argument[2].text[x]);

	QWritef("\"%s\"", argument[2].text);
}
break;






case C_STRING_TOUPPER:
{ // Return string in upper case.
	if(argument_num < 3) {
		QWrite("ERROR: Not enough parameters");
		break;
	}

	String_trim_quotes(argument[2]);

	for(unsigned int x=0;x < argument[2].length;x++)
		argument[2].text[x] = toupper(argument[2].text[x]);

	QWritef("\"%s\"", argument[2].text);
}
break;






case C_STRING_TOARRAY:
{ // Return all characters in string as array elements
	if(argument_num < 3) {
		QWrite("ERROR: Not enough parameters");
		break;
	}

	String_trim_quotes(argument[2]);
	QWrite("[");

	for (unsigned int x=0;x < argument[2].length;x++) {
		if (argument[2].text[x]=='"') 
			QWrite("\"\"\"\"");
		else 
			QWritef("\"%c\"", argument[2].text[x]);//v1.11 fixed bug with quotes

		if (x < argument[2].length-1)
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

	unsigned int idx = atoi(argument[3].text);
	String_trim_quotes(argument[2]);

	if (idx > (argument[2].length-1) || idx < 0) 
		QWrite("ERROR: Index out of bounds");
	else
		// Write char to file
		QWritef("\"%c\"", argument[2].text[idx]);
}
break;









// -----------------------------------------------------------------
// NEW STRING COMMANDS
// -----------------------------------------------------------------

case C_STRING_SIZE:
{ // Return length of the command line (minus command name)
	
	size_t arg_text = empty_char_index;

	for (size_t i=2; i<argument_num; i+=2) {
		switch (argument_hash[i]) {
			case NAMED_ARG_TEXT : 
				arg_text = i + 1;
				break;
		}
	}

	QWritef("%u", argument[arg_text].length);
}
break;










case C_STRING_REPLACE:
{ // Replace text in a given string

	size_t arg_text    = empty_char_index;
	size_t arg_find    = empty_char_index;
	size_t arg_replace = empty_char_index;
	int arg_options    = OPTION_NONE;

	for (size_t i=2; i<argument_num; i+=2) {
		switch (argument_hash[i]) {
			case NAMED_ARG_TEXT : 
				arg_text = i + 1;
				break;

			case NAMED_ARG_FIND : 
				arg_find = i + 1;
				break;

			case NAMED_ARG_REPLACE : 
				arg_replace = i + 1;
				break;

			case NAMED_ARG_CASESENSITIVE : 
				String_bool(argument[i+1]) 
					? arg_options |= OPTION_CASESENSITIVE 
					: arg_options &= ~OPTION_CASESENSITIVE;
				break;

			case NAMED_ARG_MATCHWORD : 
				String_bool(argument[i+1]) 
					? arg_options |= OPTION_MATCHWORD 
					: arg_options &= ~OPTION_MATCHWORD;
				break;
		}
	}


	char *found          = NULL;
	char *found_end      = argument[arg_text].text;
	String search_source = argument[arg_text];
	
	while ((found = String_find(search_source, argument[arg_find], arg_options))) {
		QWritel(found_end, found-found_end);
		QWrites(argument[arg_replace]);

		found_end = found + argument[arg_find].length;

		search_source.text   = found_end;
		search_source.length = argument[arg_text].length - (found_end-argument[arg_text].text);
	}
	
	if (found_end < argument[arg_text].text+argument[arg_text].length)
		QWritel(found_end, argument[arg_text].text + argument[arg_text].length - found_end);
}
break;










case C_STRING_COMPARE:
{ // Compare two strings with each other

	size_t arg_text1        = empty_char_index;
	size_t arg_text2        = empty_char_index;
	bool arg_case_sensitive = false;
	bool arg_natural_sort   = false;
	bool arg_reverse_case   = false;

	for (size_t i=2; i<argument_num; i+=2) {
		switch (argument_hash[i]) {
			case NAMED_ARG_TEXT1 : 
				arg_text1 = i + 1; 
				break;

			case NAMED_ARG_TEXT2 : 
				arg_text2 = i + 1;
				break;

			case NAMED_ARG_CASESENSITIVE : 
				arg_case_sensitive = String_bool(argument[i+1]); 
				break;

			case NAMED_ARG_NATURAL : 
				arg_natural_sort = String_bool(argument[i+1]); 
				break;

			case NAMED_ARG_REVERSECASE : 
				arg_reverse_case = String_bool(argument[i+1]); 
				break;
		}
	}


	if (arg_reverse_case) {
		for (size_t i=0;  i<argument[arg_text1].length;  i++)
			argument[arg_text1].text[i] = isupper(argument[arg_text1].text[i]) 
				? tolower(argument[arg_text1].text[i]) 
				: toupper(argument[arg_text1].text[i]);

		for (i=0;  i<argument[arg_text2].length;  i++)
			argument[arg_text2].text[i] = isupper(argument[arg_text2].text[i]) 
				? tolower(argument[arg_text2].text[i]) 
				: toupper(argument[arg_text2].text[i]);
	}

	QWritef("%d", 
		arg_natural_sort 
			? arg_case_sensitive 
				? strnatcmp(argument[arg_text1].text, argument[arg_text2].text) 
				: strnatcasecmp(argument[arg_text1].text, argument[arg_text2].text)
			: arg_case_sensitive 
				? strcmp(argument[arg_text1].text, argument[arg_text2].text) 
				: strcmpi(argument[arg_text1].text, argument[arg_text2].text)
	);
}
break;










case C_STRING_ISNUMBER:
{ // Check if string is a number

	size_t arg_text       = empty_char_index;
	bool arg_skip_math_op = false;
	bool arg_number       = false;
	bool arg_object       = false;

	for (size_t i=2; i<argument_num; i+=2) {
		switch (argument_hash[i]) {
			case NAMED_ARG_TEXT : 
				arg_text = i + 1;
				break;

			case NAMED_ARG_SKIPMATH : 
				arg_skip_math_op = String_bool(argument[i+1]);
				break;

			case NAMED_ARG_NUMBER : 
				arg_number = String_bool(argument[i+1]);
				break;

			case NAMED_ARG_OBJECT : 
				arg_object = String_bool(argument[i+1]);
				break;
		}
	}

	enum STRING_ISNUMBER_INDEX {
		STRING,
		INTEGER,
		FLOAT,
		NUMBER,
		OBJECT
	};

	char result_name[][8]    = {"string","integer","float","number","object"};
	int index                = STRING;
	int number_of_dots       = 0;
	bool skip_character      = false;
	bool found_number        = false;
	bool found_non_white     = false;
	bool found_space_between = false;

	for (i=0; i<argument[arg_text].length; i++) {
		// Ignore whitespace
		if (isspace(argument[arg_text].text[i])) {
			// Remember if there was space between characters
			if (found_non_white  &&  !arg_skip_math_op) 
				found_space_between = true;

			continue;
		}

		// If encountered a math operator
		switch (argument[arg_text].text[i]) {
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
		if (argument[arg_text].text[i] == '.') {
			number_of_dots++;

			if (number_of_dots>1  &&  !arg_skip_math_op) {
				index = STRING; 
				break;
			} else 
				continue;
		}

		// If there is a minus - it could be a negative number
		if (argument[arg_text].text[i] == '-') {
			if (found_number) {
				index = STRING; 
				break;
			}

			continue;
		}


		// If current character is a digit
		if (isdigit(argument[arg_text].text[i])  &&  !found_space_between) {
			found_number = true;

			// If previous character was dot - it could be a float
			if (i>0  &&  argument[arg_text].text[i-1]=='.')
				index = !arg_number ? FLOAT : NUMBER;

			// If it wasn't already set to 'float' then assume it's integer
			if (index != 2) 
				index = !arg_number ? INTEGER : NUMBER;
		} else {
			index = STRING;
			break;
		}
	}

	// If dot occured without a number then it's a string
	if (number_of_dots>0  &&  !found_number) 
		index = STRING;

	// If string con
	for (i=0; index==STRING && arg_object && i<argument[arg_text].length; i++)
		if (argument[arg_text].text[i]==':' || argument[arg_text].text[i]=='#')
			index = OBJECT;

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

	size_t arg_text = empty_char_index;
	int arg_mode    = DEFAULT;

	for (size_t i=2; i<argument_num; i+=2) {
		switch (argument_hash[i]) {
			case NAMED_ARG_TEXT : 
				arg_text = i + 1; 
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
					if (strcmpi(argument[i+1].text,modes[j]) == 0) {
						arg_mode = j;
						break;
					}
				break;
		}
	}

	if (argument[arg_text].length == 0) {
		QWrite("false"); 
		break;
	}


	int result         = true;  // return value
	bool allow_global  = true;	// allow global variables
	bool allow_local   = true;	// allow local variables
	bool allow_number  = false;	// allow the name to start with a number
	bool allow_bracket = false;	// allow square brackets at the end
	bool allow_inherit = false;	// allow second class name after colon
	String inherit     = empty_string;

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
		if (strncmpi(argument[arg_text].text,"class ",6)==0  ||  strncmpi(argument[arg_text].text,"class=",6)==0) {
			QWrite("false"); 
			break;
		}

		// If there are two class names then separate
		if (allow_inherit) {
			char *colon;

			if ((colon = strchr(argument[arg_text].text, ':'))) {
				size_t pos                   = colon - argument[arg_text].text;
				inherit.text                 = colon + 1;
				inherit.length               = argument[arg_text].text+argument[arg_text].length - (colon+1);
				argument[arg_text].text[pos] = '\0';
				argument[arg_text].length    = pos;

				String_trim_space(argument[arg_text]);
				String_trim_space(inherit);
			}
		}
	}

	// Optional mode - remove square brackets at the end
	if (allow_bracket  &&  argument[arg_text].length>2)
		if (argument[arg_text].text[argument[arg_text].length-2]=='['  &&  argument[arg_text].text[argument[arg_text].length-1]==']')
			argument[arg_text].text[argument[arg_text].length-2] = '\0';


	for (i=0;  result;  i++) {
		// End loop on string end OR check second string if it exists
		if (i == argument[arg_text].length) {
			if (inherit.length>0  &&  argument[arg_text].text!=inherit.text) {
				i	               = 0;
				argument[arg_text] = inherit;
			} else
				break;
		}

		// Fail conditions for the first character
		if (i == 0)	{						
			if (
				// DEFAULT - first char is (not alpha assuming digits aren't allowed OR not alpha and not a digit assuming allowed digits) and not underscore
				(allow_global  &&   allow_local  &&  ((!isalpha(argument[arg_text].text[0]) && !allow_number) || (!isalpha(argument[arg_text].text[0]) && !isdigit(argument[arg_text].text[0]) && allow_number))  &&  argument[arg_text].text[0]!='_')
				||	
				// GLOBALS ONLY - first char is (not alpha with digits not allowed OR not alpha and not digit with allowed digits)
				(allow_global  &&  !allow_local  &&  ((!isalpha(argument[arg_text].text[0]) && !allow_number) || (!isalpha(argument[arg_text].text[0]) && !isdigit(argument[arg_text].text[0]) && allow_number)))
				||
				// LOCALS ONLY - first char is not underscore
				(!allow_global &&  allow_local  &&  argument[arg_text].text[0]!='_')
			) 
				result = false;
		// If character isn't alphanumeric and isn't underscore then fail
		} else
			if (!isalnum(argument[arg_text].text[i])  &&  argument[arg_text].text[i]!='_')
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

	size_t arg_text             = empty_char_index;
	size_t arg_search_start     = empty_char_index;
	size_t arg_search_end       = empty_char_index;
	char *search_start          = empty_char;
	char *search_end            = empty_char;
	char *arg_start             = empty_char;
	char *arg_end               = empty_char;
	char *arg_len               = empty_char;
	size_t arg_offset_start     = 0;
	size_t arg_offset_end       = 0;
	bool arg_offset_start_right = 1;
	bool arg_offset_end_right   = 1;
	int arg_options             = OPTION_NONE;
	bool arg_crop_mode          = false;

	for (size_t i=2; i<argument_num; i+=2) {
		switch (argument_hash[i]) {
			case NAMED_ARG_TEXT : 
				arg_text = i + 1; 
				break;

			case NAMED_ARG_START : 
				arg_start = argument[i+1].text;
				break;

			case NAMED_ARG_END : 
				arg_end = argument[i+1].text; 
				break;

			case NAMED_ARG_LENGTH : 
				arg_len = argument[i+1].text; 
				break;

			case NAMED_ARG_EXCLUDE : 
				arg_crop_mode = String_bool(argument[i+1]); 
				break;

			case NAMED_ARG_STARTFIND : 
				arg_search_start = i + 1; 
				break;

			case NAMED_ARG_ENDFIND : 
				arg_search_end = i + 1; 
				break;

			case NAMED_ARG_MATCHWORD : 
				String_bool(argument[i+1]) 
					? arg_options |= OPTION_MATCHWORD 
					: arg_options &= ~OPTION_MATCHWORD;
				break;

			case NAMED_ARG_CASESENSITIVE : 
				String_bool(argument[i+1]) 
					? arg_options |= OPTION_CASESENSITIVE 
					: arg_options &= ~OPTION_CASESENSITIVE;
				break;

			case NAMED_ARG_STARTOFFSET : 
				if (argument[i+1].text[0] == '-')
					arg_offset_start_right = 0;
				arg_offset_start = strtoul(argument[i+1].text, NULL, 0);
				break;

			case NAMED_ARG_ENDOFFSET : 
				if (argument[i+1].text[0] == '-')
					arg_offset_start_right = 0;
				arg_offset_end = strtoul(argument[i+1].text, NULL, 0);
				break;
		}
	}

	if (argument[arg_text].length == 0)
		break;

	StringPos range = ConvertStringPos(arg_start, arg_end, arg_len, argument[arg_text].length);


	// Search for the start and end position
	if (argument[arg_search_start].length > 0) {
		char *start_ptr = String_find(argument[arg_text], argument[arg_search_start], arg_options);
		if (!start_ptr)
			break;

		range.start = start_ptr - argument[arg_text].text;

		if (argument[arg_search_end].length > 0) {
			String search_start = {start_ptr, argument[arg_text].length-range.start};
			char *end_ptr = String_find(search_start, argument[arg_search_end], arg_options);
			if (end_ptr) {
				range.end = end_ptr - argument[arg_text].text;
			}
		}
	}

	range.start += arg_offset_start * (arg_offset_start_right ? 1 : -1);
	range.end   += arg_offset_end * (arg_offset_end_right ? 1 : -1);


	// Default mode - return range of characters
	if (!arg_crop_mode) {
		if (range.start <= range.end)
			QWritel(argument[arg_text].text+range.start, range.end-range.start);
		else
			// Reverse range - iterate from the end
			for (--range.start;  range.start>=range.end;  range.start--)
				QWritef("%c", argument[arg_text].text[range.start]);
	} else {
		// Crop mode - remove range of characters
		if (range.start <= range.end) {
			shift_buffer_chunk(argument[arg_text].text, range.end, argument[arg_text].length, range.end-range.start, OPTION_LEFT);
			QWritel(argument[arg_text].text, argument[arg_text].length-(range.end-range.start));
		} else {
			// Reverse range - iterate characters from the end
			for (size_t i=argument[arg_text].length-1; i>0; i--)
				if (i<range.end  ||  i>=range.start)
					QWritef("%c", argument[arg_text].text[i]);
		}
	}
}
break;










case C_STRING_TOKENIZE:
{ // Split string on given occurences

	size_t arg_text      = empty_char_index;
	size_t arg_delimiter = empty_char_index;
	size_t arg_add_empty = empty_char_index;
	bool arg_by_string   = false;
	bool arg_skip_quotes = false;
	int arg_options      = 0;

	for (size_t i=2; i<argument_num; i+=2) {
		switch (argument_hash[i]) {
			case NAMED_ARG_TEXT : 
				arg_text = i + 1; 
				break;

			case NAMED_ARG_DELIMITER : 
				arg_delimiter = i + 1; 
				break;

			case NAMED_ARG_WORDDELIMITER : 
				arg_by_string = String_bool(argument[i+1]); 
				break;

			case NAMED_ARG_QUOTESKIP : 
				arg_skip_quotes = String_bool(argument[i+1]); 
				break;

			case NAMED_ARG_MATCHWORD : 
				String_bool(argument[i+1]) 
					? arg_options |= OPTION_MATCHWORD 
					: arg_options &= ~OPTION_MATCHWORD;
				break;

			case NAMED_ARG_CASESENSITIVE : 
				String_bool(argument[i+1]) 
					? arg_options |= OPTION_CASESENSITIVE 
					: arg_options &= ~OPTION_CASESENSITIVE;
				break;

			case NAMED_ARG_ADDEMPTY : 
				arg_add_empty = i + 1;
				break;
		}
	}


	// If we're skipping occurences in quotes then first we need to save position of quot marks
	size_t Qmax    = 0;
	size_t Qindex  = -2;
	size_t *Qarray;

	if (arg_skip_quotes) {
		// Count how many quotes there are
		for (size_t i=0;  i<argument[arg_text].length;  i++)
			if (argument[arg_text].text[i] == '"')
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

		for (i=0;  i<argument[arg_text].length && Qindex<Qmax;  i++)
			if (argument[arg_text].text[i] == '"') {
				in_quote = !in_quote;

				if (in_quote) {
					Qindex          += 2;
					Qarray[Qindex]   = i;
					Qarray[Qindex+1] = argument[arg_text].length;
				} else 
					Qarray[Qindex+1] = i;
			}
	}


	// Prepare text before tokenizing by words
	String search_source = argument[arg_text];
	size_t skip          = 0;
	char *occurence;

	if (arg_by_string) {
		// Search for the word
		while ((occurence = String_find(search_source, argument[arg_delimiter], arg_options))) {
			size_t pos    = occurence - search_source.text;
			bool in_quote = false;

			// Check if the word is inside quotation
			if (arg_skip_quotes  &&  Qmax>0) {
				for (size_t i=0;  i<Qmax && !in_quote;  i+=2)
					if (Qarray[i]<=(skip+pos)  &&  (skip+pos)<=Qarray[i+1]) 
						in_quote = true;

				// If so then skip it
				if (in_quote) {
					skip                += pos + argument[arg_delimiter].length; 
					search_source.text   = argument[arg_text].text + skip;
					search_source.length = argument[arg_text].length - skip;
					continue;
				}
			}
			
			// Replace the word with unused characters
			for (size_t i=pos; i<(pos+argument[arg_delimiter].length); i++) 
				search_source.text[i] = '\a';
		}

	// Prepare text before tokenizing by single characters
	} else {
		// For each char in delim
		for (size_t i=0; i<argument[arg_delimiter].length; i++) {
			search_source  = argument[arg_text];
			skip           = 0;
			String to_find = {argument[arg_delimiter].text + i, 1};

			while ((occurence = String_find(search_source, to_find, arg_options))) {
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
						search_source.text   = argument[arg_text].text + skip;
						search_source.length = argument[arg_text].length - skip;
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

	if (strncmpi(argument[arg_add_empty].text,"both",4)==0) {
		add_empty_start = true;
		add_empty_end   = true;
	} else
		if (strncmpi(argument[arg_add_empty].text,"start",5)==0)
			add_empty_start = true;
		else
			if (strncmpi(argument[arg_add_empty].text,"end",3)==0)
				add_empty_end = true;


	// Start tokenization
	QWrite("[");
	String item;
	pos = 0;
	size_t tokenize_pos = 0;

	while ((item = String_tokenize(argument[arg_text], "\a", tokenize_pos, OPTION_NONE)).length > 0) {
		// Keep track of numbers
		if (add_empty_start  ||  add_empty_end) {
			pos = item.text - argument[arg_text].text;
		}

		// if string begins with token 
		if (add_empty_start) {
			add_empty_start = false;

			// then add empty string to the output array
			if (pos != 0)
				QWrite("{}");
		}
		
		QWrite("]+[\"");
		QWritesq(item);
		QWrite("\"");
		count++;
	}


	// if there were no parts at all
	if (add_empty_start)
		QWrite("{}");

	// if last part doesn't end the string
	if (add_empty_end  &&  (pos+item.length<argument[arg_text].length || count==0))
		QWrite("]+[{}");

	QWrite("]");
}
break;










case C_STRING_TRIM:
{ // Trim white-space from string

	size_t arg_text        = empty_char_index;
	bool arg_left          = false;
	bool arg_middle        = false;
	bool arg_right         = false;
	bool arg_middle_single = false;

	for (size_t i=2; i<argument_num; i+=2) {
		switch (argument_hash[i]) {
			case NAMED_ARG_LEFT : 
				arg_left = String_bool(argument[i+1]); 
				break;

			case NAMED_ARG_MIDDLE : 
				arg_middle        = String_bool(argument[i+1]); 
				arg_middle_single = !arg_middle; 
				break;

			case NAMED_ARG_MIDDLESINGLE : 
				arg_middle_single = String_bool(argument[i+1]); 
				arg_middle        = !arg_middle_single; 
				break;

			case NAMED_ARG_RIGHT : 
				arg_right = String_bool(argument[i+1]); 
				break;

			case NAMED_ARG_TEXT : 
				arg_text = i + 1; 
				break;
		}
	}


	// Remove leading and trailing whitespaces
	while (arg_left && isspace(argument[arg_text].text[0])) {
		argument[arg_text].text++;
		argument[arg_text].length++;
	}

	for (i=argument[arg_text].length-1;  arg_right && i>=0 && isspace(argument[arg_text].text[i]);  i--) {
		argument[arg_text].text[i] = '\0';
		argument[arg_text].length--;
	}


	// Remove whitespace in the middle of the string
	if (arg_middle || arg_middle_single) {
		// get starting position (ignore leading space)
		for (size_t i=0; isspace(argument[arg_text].text[i]);  i++);

		// get ending position (ignore trailling space)
		for (size_t end=argument[arg_text].length-1;  isspace(argument[arg_text].text[end]);  end--);

		int count_space = 0;

		// shift chars to the left when encountered whitespace
		for (; i<end-1; i++)
			if (isspace(argument[arg_text].text[i])) {
				count_space++;

				if (!arg_middle_single  ||  (arg_middle_single && count_space>1)) {
					shift_buffer_chunk(argument[arg_text].text, i, i+1, 1, OPTION_LEFT);
					end--;
					argument[arg_text].length--;
				}
			} else
				count_space = 0;
	}

	QWrites(argument[arg_text]);
}
break;










case C_STRING_FIND:
{ // Search string

	size_t arg_text      = empty_char_index;
	size_t arg_find      = empty_char_index;
	size_t arg_findchar  = empty_char_index;
	char *arg_start      = empty_char;
	char *arg_end        = empty_char;
	char *arg_length     = empty_char;
	size_t arg_limit     = -1;
	int arg_options      = OPTION_NONE;
	bool arg_skip_quotes = false;

	for (size_t i=2; i<argument_num; i+=2) {
		switch (argument_hash[i]) {
			case NAMED_ARG_TEXT : 
				arg_text = i + 1; 
				break;

			case NAMED_ARG_FIND : 
				arg_find = i + 1;
				break;

			case NAMED_ARG_MATCHWORD : 
				String_bool(argument[i+1]) 
					? arg_options |= OPTION_MATCHWORD 
					: arg_options &= ~OPTION_MATCHWORD;
				break;

			case NAMED_ARG_CASESENSITIVE : 
				String_bool(argument[i+1]) 
					? arg_options |= OPTION_CASESENSITIVE 
					: arg_options &= ~OPTION_CASESENSITIVE;
				break;

			case NAMED_ARG_START : 
				arg_start = argument[i+1].text; 
				break;

			case NAMED_ARG_END : 
				arg_end = argument[i+1].text; 
				break;

			case NAMED_ARG_LENGTH : 
				arg_length = argument[i+1].text; 
				break;

			case NAMED_ARG_LIMIT : 
				arg_limit = strtoul(argument[i+1].text, NULL, 0);
				break;

			case NAMED_ARG_QUOTESKIP : 
				arg_skip_quotes = String_bool(argument[i+1]); 
				break;

			case NAMED_ARG_FINDCHAR :
				arg_findchar = i + 1;
				break;
		}
	}


	if ((argument[arg_findchar].length==0 && argument[arg_find].length==0)  ||  argument[arg_text].length==0) {
		QWrite("[]");
		break;
	}

	// If we're skipping occurences in quotes then first we need to save position of quotation marks
	size_t Qmax    = 0;
	size_t Qindex  = -1;
	size_t *Qarray = NULL;
	bool in_quote  = false;

	if (arg_skip_quotes) {
		// Count how many quotes there are
		for (size_t i=0;  i<argument[arg_text].length;  i++)
			if (argument[arg_text].text[i] == '"')
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

		for (i=0;  i<argument[arg_text].length && Qindex<Qmax;  i++)
			if (argument[arg_text].text[i] == '"') {
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
	bool default_mode = argument[arg_findchar].length == 0;

	for (size_t pos=0;  pos<(default_mode ? 1 : argument[arg_findchar].length);  pos++) {
		char *occurence;
		size_t count         = 0;
		size_t pos_absolute  = 0;
		size_t pos_relative  = 0;
		String search_source = argument[arg_text];
		String to_find       = argument[default_mode ? arg_find : arg_findchar];
		StringPos range      = ConvertStringPos(arg_start, arg_end, arg_length, argument[arg_text].length);

		if (!default_mode) {
			to_find.text   = argument[arg_findchar].text + pos;
			to_find.length = 1;
		}

		while ((occurence = String_find(search_source, to_find, arg_options))) {
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
			search_source.text   = argument[arg_text].text   + pos_absolute;
			search_source.length = argument[arg_text].length - pos_absolute;
		}
	}

	if (Qarray)
		free(Qarray);

	QWrite("]");
}
break;










case C_STRING_SPLIT:
{ // Return string as an array

	size_t arg_text           = empty_char_index;
	char *arg_start           = empty_char;
	char *arg_end             = empty_char;
	char *arg_length          = empty_char;
	size_t arg_segment_length = 1;
	size_t arg_limit          = -1;

	for (size_t i=2; i<argument_num; i+=2) {
		switch (argument_hash[i]) {
			case NAMED_ARG_TEXT : 
				arg_text = i + 1; 
				break;

			case NAMED_ARG_SIZE : 
				arg_segment_length = strtoul(argument[i+1].text, NULL, 0);
				break;

			case NAMED_ARG_LIMIT : 
				arg_limit = strtoul(argument[i+1].text, NULL, 0); 
				break;

			case NAMED_ARG_START : 
				arg_start = argument[i+1].text; 
				break;

			case NAMED_ARG_END : 
				arg_end = argument[i+1].text; 
				break;

			case NAMED_ARG_LENGTH : 
				arg_length = argument[i+1].text;
				break;
		}
	}


	// Split
	QWrite("[");
	size_t count    = 0;
	size_t parts    = 0;
	StringPos range = ConvertStringPos(arg_start, arg_end, arg_length, argument[arg_text].length);

	for (i=range.start;  i<range.end && arg_limit!=0;  i++) {
		// Start segment
		if (count == 0)
			QWrite("\"");

		// Output character
		if (argument[arg_text].text[i] == '"') 
			QWrite("\"\""); 
		else
			QWritef("%c", argument[arg_text].text[i]);

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

	enum URL_PARSE_OPTIONS {
		NONE,
		QUERY_FOUND        = 0x1,
		FRAGMENT_FOUND     = 0x2,
		AUTHORITY_FOUND    = 0x4,
		AUTHORITY_ENDED    = 0x8,
		IN_SQUARE_BRACKETS = 0x10,
		KEEP_WWW           = 0x20,
		PASSED_SLASH       = 0x40,
		ASSUME_HOST        = 0x100,
		ASSUME_SCHEME      = 0x400
	};
	
	enum URL_COMPONENTS {
		SCHEME,
		USER,
		PASS,
		HOST,
		PORT,
		PATH,
		QUERY,
		FRAGMENT,
		MAX_COMPONENTS
	};

	size_t arg_url   = empty_char_index;
	bool arg_keepwww = false;
	int status       = ASSUME_SCHEME;

	for (size_t i=2; i<argument_num; i+=2) {
		switch (argument_hash[i]) {
			case NAMED_ARG_URL : 
				arg_url = i + 1; 
				break;

			case NAMED_ARG_SHORT : 
				status = String_bool(argument[i+1]) ? ASSUME_HOST : ASSUME_SCHEME;
				break;

			case NAMED_ARG_KEEPWWW : 
				arg_keepwww = String_bool(argument[i+1]);
				break;
		}
	}

	
	String url[MAX_COMPONENTS];
	size_t word_start      = 0;
	int expect             = SCHEME;
	const char separator[] = ":@?#/";

	for (i=0; i<MAX_COMPONENTS; i++)
		url[i] = empty_string;
	
	for (i=0; i<=argument[arg_url].length; i++) {		
		char c = argument[arg_url].text[i];
		
		// Square brackets escape IPv6
		if (expect==HOST  &&  c=='[')
			status |= IN_SQUARE_BRACKETS;
		
		if (c == ']')
			status &= ~IN_SQUARE_BRACKETS;
		
		// Check if it's word end
		bool is_separator = false;
		for (size_t j=0; j<=sizeof(separator)/sizeof(separator[0])-1 && !is_separator; j++)
			if (c == separator[j])
				is_separator = true;
				
		if (is_separator  &&  status & IN_SQUARE_BRACKETS)
			continue;
				
		if (is_separator) {
			if (c == '/') {
				// If authority
				if (argument[arg_url].text[i+1]=='/'  &&  ~status & AUTHORITY_FOUND) {					
					expect     = HOST;
					status    |= AUTHORITY_FOUND;
					status    &= ~PASSED_SLASH;
					word_start = i + 2;
					i++;
					continue;
				}
				
				// Accept only the first slash and ignore all others
				if (~status & PASSED_SLASH) {
					if (~status & ASSUME_SCHEME || (status & ASSUME_SCHEME && status & AUTHORITY_FOUND))
						status |= PASSED_SLASH;
					else
						continue;
				} else
					continue;
			}
			
			// Question marks in a query do not do anything
			if ((expect==QUERY  &&  c=='?')  ||  (expect==FRAGMENT  &&  c=='#'))
				continue;
				
			// At symbol ignored when not in authority
			if (status & ASSUME_SCHEME  &&  c=='@'  &&  (~status & AUTHORITY_FOUND  ||  status & AUTHORITY_ENDED))
				continue;
			
			// Save string
			if (expect < MAX_COMPONENTS) {
				url[expect].text   = argument[arg_url].text + word_start;
				url[expect].length = i - word_start;
				word_start         = i + 1;
				
				// if scheme
				if (expect == SCHEME) {
					if (c == ':') {
						expect = PATH;
					} else {
						// If scheme ends without colon then end parsing
						if (status & ASSUME_SCHEME) {
							url[SCHEME] = empty_string;
							i           = argument[arg_url].length;
							continue;
						}
						
						// If there's no scheme then assume host is the first item
						if (status & ASSUME_HOST) {
							url[HOST]   = url[SCHEME];
							url[SCHEME] = empty_string;
							expect      = PATH;
							
							for (size_t j=0; j<url[HOST].length; j++)
								if (url[HOST].text[j] == '/') {
									url[HOST].length = j;
									url[PATH].text   = argument[arg_url].text + j + 1;
									url[PATH].length = i - (j+1);
									break;
								}
						}
					}
				} else {
					if (expect == PATH) {
						// if path consists solely of digits then assume it's port instead
						if (url[PATH].length>0  &&  status & ASSUME_HOST  &&  ~status & AUTHORITY_FOUND  &&   url[SCHEME].text!=empty_string.text  &&  c!='@') {
							bool all_digits = true;
							
							for (size_t j=0; j<url[PATH].length && all_digits; j++)
								if (!isdigit(url[PATH].text[j]))
									all_digits = false;
							
							// If it's a port then shift strings
							if (all_digits) {
								url[HOST]   = url[SCHEME];
								url[PORT]   = url[PATH];
								url[SCHEME] = empty_string;
								url[PATH]   = empty_string;
								expect      = PATH;
							} else {
								// Otherwise continue path
								word_start = url[PATH].text - argument[arg_url].text;
								expect     = PATH;
							}
						}
					}
					
					expect = MAX_COMPONENTS;
					
					// Next will be port
					if (c==':'  &&  status & AUTHORITY_FOUND  &&  ~status & AUTHORITY_ENDED)
						expect = PORT;
					
					// Next will be path
					if (c == '/') {
						expect = PATH;
						
						if (status & AUTHORITY_FOUND)
							status |= AUTHORITY_ENDED;
					}
					
					if (c == '@') {
						// If userinfo then swap strings
						if (status & AUTHORITY_FOUND  &&  ~status & AUTHORITY_ENDED) {
							url[USER] = url[HOST];
							url[PASS] = url[PORT];
							url[HOST] = empty_string;
							url[PORT] = empty_string;
						} else 
							if (status & ASSUME_HOST) {
								// If userinfo without scheme
								url[USER]   = url[SCHEME];
								url[PASS]   = url[PATH];
								url[SCHEME] = empty_string;
								url[PATH]   = empty_string;
							}
						
						expect = HOST;
					}
				}
			}
			
			// Question mark begins query
			if (c=='?'  &&  ~status & QUERY_FOUND) {
				expect      = QUERY;
				word_start  = i + 1;
				status     |= AUTHORITY_ENDED | QUERY_FOUND;
			}
			
			// Hash begins fragment
			if (c=='#'  &&  ~status & FRAGMENT_FOUND) {
				expect     = FRAGMENT;
				word_start = i + 1;
				status    |= AUTHORITY_ENDED | FRAGMENT_FOUND;
			}
			
			// Slash begins path
			if (status & AUTHORITY_FOUND  &&  ~status & AUTHORITY_ENDED  &&  c=='/') {
				expect     = PATH;
				word_start = i + 1;
			}
		}
	}
	

	// If IPv6
	if (url[HOST].length>=1  &&  url[HOST].text[0]=='['  &&  url[HOST].text[url[HOST].length-1]==']') {
		url[HOST].text   += 1;
		url[HOST].length -= 2;
	}
	
	// Strip www
	if (url[HOST].length>=4  &&  ~status & KEEP_WWW  &&  strncmpi("www.",url[HOST].text,4)==0) {
		url[HOST].text   += 4;
		url[HOST].length -= 4;
	}
	

	// Output
	QWrite("[");
	for (i=0; i<MAX_COMPONENTS; i++) {
		if (i > 0)
			QWrite(",{");
		else
			QWrite("{");

		QWrites(url[i]);
		QWrite("}");
	}
	

	// Tokenize domain
	QWrite(",[");
	String item;
	size_t pos = 0;

	while ((item = String_tokenize(url[HOST], ".", pos, OPTION_NONE)).length > 0)
		QWritef("]+[{%s}", item.text);

	QWrite("]");
	

	// Tokenize query
	StringDynamic attributes;
	StringDynamic values;
	StringDynamic_init(attributes);
	StringDynamic_init(values);
	pos = 0;

	while ((item = String_tokenize(url[QUERY], "&", pos, OPTION_NONE)).length > 0) {
		char *equality = strchr(item.text, '=');

		if (equality) {
			size_t equality_pos     = equality - item.text;
			item.text[equality_pos] = '\0';
			
			StringDynamic_appendf(attributes, "]+[{%s}", item.text);		
			StringDynamic_appendf(values, "]+[{%s}", item.text+equality_pos+1);
		} else {
			StringDynamic_append(attributes, "]+[{");
			StringDynamic_appends(attributes, item);
			StringDynamic_append(attributes, "}");
			StringDynamic_append(values, "]+[{}");
		}
	}

	QWritef(",[%s],[%s]]", attributes.text, values.text);
	StringDynamic_end(attributes);
	StringDynamic_end(values);	
}
break;










case C_STRING_CASE:
{ // Convert to upper or lower case

	size_t arg_text    = empty_char_index;
	char *arg_start    = empty_char;
	char *arg_end      = empty_char;
	char *arg_length   = empty_char;
	bool arg_sentences = false;
	bool arg_words     = false;
	bool arg_upper     = false;
	bool arg_lower     = false;

	for (size_t i=2; i<argument_num; i+=2) {
		switch (argument_hash[i]) {
			case NAMED_ARG_SENTENCES : 
				arg_sentences = String_bool(argument[i+1]); 
				break;

			case NAMED_ARG_WORDS : 
				arg_words = String_bool(argument[i+1]); 
				break;

			case NAMED_ARG_UPPER : 
				arg_upper = String_bool(argument[i+1]); 
				break;

			case NAMED_ARG_LOWER : 
				arg_lower = String_bool(argument[i+1]); 
				break;

			case NAMED_ARG_TEXT : 
				arg_text = i + 1;
				break;

			case NAMED_ARG_START : 
				arg_start = argument[i+1].text;
				break;

			case NAMED_ARG_END : 
				arg_end = argument[i+1].text;
				break;

			case NAMED_ARG_LENGTH : 
				arg_length = argument[i+1].text;
				break;
		}
	}

	StringPos range = ConvertStringPos(arg_start, arg_end, arg_length, argument[arg_text].length);
	bool capitalize = true;

	for (i=range.start; i<range.end; i++) {
		// Default mode - change all characters
		if (!arg_sentences && !arg_words) {
			if (arg_upper && !arg_lower) 
				argument[arg_text].text[i] = toupper(argument[arg_text].text[i]);

			if (!arg_upper && arg_lower) 
				argument[arg_text].text[i] = tolower(argument[arg_text].text[i]);

		// Special mode - change only the first character in sentences or words
		} else {
			if (capitalize  &&  (arg_words || arg_sentences && !isspace(argument[arg_text].text[i]))) {
				capitalize = false;

				if (arg_upper) 
					argument[arg_text].text[i] = toupper(argument[arg_text].text[i]);
			} else
				if (arg_lower) 
					argument[arg_text].text[i] = tolower(argument[arg_text].text[i]);

			// separator between words/sentences
			if ((arg_words  &&  isspace(argument[arg_text].text[i]))  ||  (arg_sentences  &&  (argument[arg_text].text[i]=='.' || argument[arg_text].text[i]=='?' || argument[arg_text].text[i]=='!'))) 
				capitalize = true;
		}
	}

	QWrites(argument[arg_text]);
}
break;










case C_STRING_REPLACECHAR:
{ // Replace single characters

	size_t arg_text     = empty_char_index;
	size_t arg_find     = empty_char_index;
	size_t arg_replace  = empty_char_index;
	bool arg_correspond = false;
	int arg_options     = OPTION_NONE;

	for (size_t i=2; i<argument_num; i+=2) {
		switch (argument_hash[i]) {
			case NAMED_ARG_TEXT : 
				arg_text = i + 1; 
				break;

			case NAMED_ARG_FIND : 
				arg_find = i + 1;
				break;

			case NAMED_ARG_REPLACE : 
				arg_replace = i + 1;
				break;

			case NAMED_ARG_CORRESPOND : 
				arg_correspond = String_bool(argument[i+1]); 
				break;

			case NAMED_ARG_CASESENSITIVE : 
				String_bool(argument[i+1]) 
					? arg_options |= OPTION_CASESENSITIVE 
					: arg_options &= ~OPTION_CASESENSITIVE;
				break;

			case NAMED_ARG_MATCHWORD : 
				String_bool(argument[i+1]) 
					? arg_options |= OPTION_MATCHWORD 
					: arg_options &= ~OPTION_MATCHWORD;
				break;
		}
	}


	for (i=0; i<argument[arg_text].length; i++) {
		char current = argument[arg_text].text[i];

		// Optional: occurrence musn't be surrounded by alphanumeric characters
		if (arg_options & OPTION_MATCHWORD) {
			bool left_cleared  = i==0  ||  (i>0  &&  !isalnum(argument[arg_text].text[i-1])  &&  argument[arg_text].text[i-1]!='_');
			bool right_cleared = i>=argument[arg_text].length  ||  (i<argument[arg_text].length  &&  !isalnum(argument[arg_text].text[i+1])  &&  argument[arg_text].text[i+1]!='_');

			if (!left_cleared  ||  !right_cleared) {
				QWritef("%c", current);
				continue;
			}
		}

		// Search for replacement
		for (size_t j=0; j<argument[arg_find].length; j++)
			if ((arg_options & OPTION_CASESENSITIVE  &&  argument[arg_text].text[i]==argument[arg_find].text[j])  ||  (~arg_options & OPTION_CASESENSITIVE  &&  tolower(argument[arg_text].text[i])==tolower(argument[arg_find].text[j])))
				current = !arg_correspond
					? (argument[arg_replace].length > 0 ? argument[arg_replace].text[0] : '\0')	 // Default mode: replace if given third argument; otherwise remove
					: (j < argument[arg_replace].length ? argument[arg_replace].text[j] : '\0'); // Parallel mode: find corresponding character from the third argument; otherwise remove

		if (current != '\0')
			QWritef("%c", current);
	}
}
break;










case C_STRING_JOIN:
{ // Add characters in the midddle of the string

	size_t arg_text    = empty_char_index;
	size_t arg_merge   = empty_char_index;
	char *arg_position = empty_char;

	for (size_t i=2; i<argument_num; i+=2) {
		switch (argument_hash[i]) {
			case NAMED_ARG_TEXT : 
				arg_text = i + 1;
				break;

			case NAMED_ARG_MERGE : 
				arg_merge = i + 1;
				break;

			case NAMED_ARG_POSITION : 
				arg_position = argument[i+1].text;
				break;
		}
	}

	if (argument[arg_text].length == 0) {
		QWrites(argument[arg_merge]);
		break;
	}

	if (argument[arg_merge].length == 0) {
		QWrites(argument[arg_text]);
		break;
	}

	StringPos range = ConvertStringPos(arg_position, empty_char, empty_char, argument[arg_text].length);

	QWritel(argument[arg_text].text, range.start);
	QWrites(argument[arg_merge]);
	QWritel(argument[arg_text].text+range.start, argument[arg_text].length-range.start);
}
break;










case C_STRING_WORDPOS:
{ // Find where words start

	size_t arg_text = empty_char_index;

	for (size_t i=2; i<argument_num; i+=2) {
		switch (argument_hash[i]) {
			case NAMED_ARG_TEXT : 
				arg_text = i + 1;
				break;
		}
	}

	QWrite("[");

	for (i=0; i<argument[arg_text].length; i++) {
		// Current word
		int initial_type = GetCharType(argument[arg_text].text[i]);

		// Output word position
		if (initial_type != CHAR_TYPE_SPACE)
			QWritef("]+[%u", i);

		// Iterate until current word ends
		while (i<argument[arg_text].length  &&  GetCharType(argument[arg_text].text[i])==initial_type)
			i++;

		// Skip spaces
		while (i<argument[arg_text].length  &&  GetCharType(argument[arg_text].text[i])==CHAR_TYPE_SPACE)
			i++;
	}

	QWrite("]");
}
break;