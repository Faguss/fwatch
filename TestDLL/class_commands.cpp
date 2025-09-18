// -----------------------------------------------------------------
// CLASS OPERATIONS
// -----------------------------------------------------------------

case C_CLASS_READ:
{
	const int predefined_capacity = 8;

	// Read arguments------------------------------------------------------------
	size_t arg_file       = empty_char_index;
	size_t arg_path       = empty_char_index;
	char *arg_wrap        = empty_char;
	size_t arg_find       = empty_char_index;
	int arg_offset        = 0;
	int arg_classpath_pos = -1;
	int arg_level         = predefined_capacity;
	bool arg_verify       = false;
	size_t arg_lowercase  = empty_char_index;
	bool arg_trimdollar   = false;
	bool arg_find_require = false;

	for (size_t i=2; i<argument_num; i+=2) {
		switch (argument_hash[i]) {
			case NAMED_ARG_FILE : 
				arg_file = i + 1;
				break;

			case NAMED_ARG_PATH :
				arg_path = i + 1;
				break;
			
			case NAMED_ARG_OFFSET : 
				arg_offset = atoi(argument[i+1].text);
				break;
			
			case NAMED_ARG_FIND :
			case NAMED_ARG_PICK :
				arg_find_require = argument_hash[i] == NAMED_ARG_FIND;
				arg_find         = i + 1;
				break;

			case NAMED_ARG_WRAP : 
				arg_wrap = argument[i+1].text;
				break;
			
			case NAMED_ARG_PATHPOS : 
				arg_classpath_pos = atoi(argument[i+1].text);
				break;
			
			case NAMED_ARG_MAXLEVEL : 
				arg_level = atoi(argument[i+1].text);
				break;

			case NAMED_ARG_VERIFY : 
				arg_verify = String_bool(argument[i+1]);
				break;

			case NAMED_ARG_LOWERCASE : 
				arg_lowercase = i + 1;
				break;

			case NAMED_ARG_TRIMDOLLAR : 
				arg_trimdollar = String_bool(argument[i+1]);
				break;
		}
	}
	
	// File not specified
	if (argument[arg_file].length == 0) {
		QWrite_err(FWERROR_PARAM_EMPTY, 1, "arg_file");
		QWrite("[],[],0]");
		break;
	}

	// Verify and update path to the file
	StringDynamic buf_filename;
	StringDynamic_init(buf_filename);

	if (!VerifyPath(argument[arg_file], buf_filename, OPTION_ALLOW_MOST_LOCATIONS)) {
		QWrite("[],[],0]");
		break;
	}


	// Class path
	char *classpath[predefined_capacity];
	int classpath_capacity = arg_level!=predefined_capacity ? arg_level : predefined_capacity;
	int classpath_current  = 0;
	int classpath_size     = 0;
	size_t arg_path_pos    = 0;
	String item;

	while ((item = String_tokenize(argument[arg_path], ",", arg_path_pos, OPTION_TRIM_SQUARE_BRACKETS)).length > 0  &&  classpath_size<classpath_capacity) {
		String_trim_quotes(item);
		classpath[classpath_size++] = item.text;
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
	size_t arg_find_pos = 0;

	while ((item = String_tokenize(argument[arg_find], ",", arg_find_pos, OPTION_TRIM_SQUARE_BRACKETS)).length>0  &&  properties_to_find_size<properties_to_find_capacity) {
		String_trim_quotes(item);

		if (item.length > 0)
			properties_to_find[properties_to_find_size++] = item.text;
	}

	// Lowercase output data
	enum CLASS_READ_LOWERCASE {
		CLASS_READ_LOWER_CLASS    = 0x2,
		CLASS_READ_LOWER_PROPERTY = 0x4,
		CLASS_READ_LOWER_VALUE    = 0x8
	};

	const char lowercaseflags[][16] = {"class", "property", "value"};
	int lowercase_flag = 0;
	arg_find_pos       = 0;

	while ((item = String_tokenize(argument[arg_lowercase], ",", arg_find_pos, OPTION_TRIM_SQUARE_BRACKETS)).length>0) {
		String_trim_quotes(item);

		for (int i=0, j=2; i<sizeof(lowercaseflags)/sizeof(lowercaseflags[0]); i++, j*=2)
			if (strcmpi(item.text,lowercaseflags[i]) == 0)
				lowercase_flag |= j;
	}
	//---------------------------------------------------------------------------
	


	// Open wanted file -----------------------------------------------------------------	
	FILE *file = fopen(argument[arg_file].text, "rb");
	if (!file) {
		QWrite_err(FWERROR_ERRNO, 2, errno, argument[arg_file].text);
		QWrite("[],[],0]");
		StringDynamic_end(buf_filename);
		break;
	}

	// Find file size
	if (fseek(file, 0, SEEK_END) != 0) {
		QWrite_err(FWERROR_ERRNO, 2, errno, argument[arg_file].text);
		QWrite("[],[],0]");
		StringDynamic_end(buf_filename);
		fclose(file);
		break;
	};

	size_t file_size = ftell(file);
	if (file_size == 0xFFFFFFFF) {
		QWrite_err(FWERROR_ERRNO, 2, errno, argument[arg_file].text);
		QWrite("[],[],0]");
		StringDynamic_end(buf_filename);
		fclose(file);
		break;
	} else
		file_size -= arg_offset;

	// Allocate buffer
	StringDynamic file_contents;
	StringDynamic_init(file_contents);
	
	int result = StringDynamic_allocate(file_contents, file_size+1);
	if (result != 0) {
		QWrite_err(FWERROR_MALLOC, 2, "file_contents", file_size+1);
		QWrite("[],[],0]");
		StringDynamic_end(buf_filename);
		break;
	}

	// Copy text to buffer
	fseek(file, arg_offset, SEEK_SET);
	size_t bytes_read = fread(file_contents.text, 1, file_size, file);
	file_contents.text[file_size] = '\0';

	if (bytes_read != file_size) {		
		QWrite_err(FWERROR_ERRNO, 2, errno, argument[arg_file].text);		
		StringDynamic_end(buf_filename);
		StringDynamic_end(file_contents);
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
	
	StringDynamic output_class[predefined_capacity];
	StringDynamic output_inherit[predefined_capacity];
	StringDynamic output_bytes[predefined_capacity];
	StringDynamic output_property[predefined_capacity];
	StringDynamic output_value[predefined_capacity];
	
	StringDynamic *all_output_strings[] = {
		output_class,
		output_inherit,
		output_bytes,
		output_property,
		output_value
	};
	const int all_output_strings_num = sizeof(all_output_strings) / sizeof(all_output_strings[0]);
	
	for (int z=0; z<classpath_capacity; z++)
		for (int j=0; j<all_output_strings_num; j++) {
			StringDynamic_init(all_output_strings[j][z]);
			StringDynamic_append(all_output_strings[j][z], "[");
		}
		
	StringDynamic output_classpath_bytes;
	StringDynamic_init(output_classpath_bytes);
	StringDynamic_append(output_classpath_bytes, "[");
  
	int comment           = NONE;
	int expect            = PROPERTY;
	int word_start        = -1;
	int class_level       = classpath_current;
	int array_level       = 0;
	int max_output_level  = 0;
	int parenthesis_level = 0;
	int property_start    = 0;
	int property_end      = 0;
	int line_num          = 1;
	int column_num        = 0;
	int properties_found  = false;
	bool first_char       = true;
	bool is_array         = false;
	bool in_quote         = false;
	bool macro            = false;
	bool is_inherit       = false;
	bool classpath_done   = false;
	bool classpath_match  = false;
	bool purge_comment    = false;
	bool syntax_error     = false;
	bool copy_property    = false;
	bool value_quoted     = false;
	char separator        = ' ';
	char *text            = file_contents.text;
#define CURRENT_OUTPUT_LEVEL class_level - classpath_current

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
						i = file_size;
						continue;
					}
					
					// When left subclass within target class
					if (!arg_verify && classpath_current==classpath_size && class_level>=classpath_current && !classpath_done) {
						int level = class_level - classpath_current;

						for (int j=level==0 ? 1 : level; j<classpath_capacity; j++)
							for (int k=0; k<all_output_strings_num; k++)
								StringDynamic_append(all_output_strings[k][j], "]");
					}
						
					expect = SEMICOLON;
					class_level--;

					// End parsing when couldn't find wanted classes
					if (!arg_verify && class_level < classpath_current)
						i = file_size;

					// Syntax error: excess closing brackets
					/*if (arg_verify  &&  class_level < 0) {
						column_num--;
						separator    = ' ';
						syntax_error = true;
						goto class_read_end_parsing;
					}*/

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
									copy_property  = false;
									value_quoted   = false;

									int level = class_level - classpath_current;
									if (!arg_verify && classpath_current==classpath_size && class_level>=classpath_current && level < classpath_capacity && !classpath_done) {
										copy_property  = properties_to_find_size == 0;

										for (int j=0; j<properties_to_find_size && !copy_property; j++)
											if (_strcmpi(text+property_start,properties_to_find[j]) == 0)
												copy_property = true;

										if (copy_property) {
											if (purge_comment) {
												purge_comment = false;
												PurgeComments(text, property_start, property_end);
											}

											properties_found++;
											char *property = text + property_start;

											if (lowercase_flag & CLASS_READ_LOWER_PROPERTY)
												for (int z=0; z<property[z]!='\0'; z++)
													property[z] = tolower(property[z]);

											StringDynamic_appendf(output_property[CURRENT_OUTPUT_LEVEL], "]+[\"%s\"", property);
										}
									}
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
					if (expect == EQUALITY)
						expect = VALUE;
					else
						if (expect == ENUM_BRACKET)
							expect = ENUM_CONTENT;
						else
							if (expect == EXEC_BRACKET)
								expect = EXEC_CONTENT;

					separator = ' ';
				} else 
					if (expect==EQUALITY && c=='(') {
						expect            = MACRO_CONTENT;
						separator         = ' ';
						parenthesis_level = 1;
					} else
						if (expect == ENUM_BRACKET) { // ignore what's between "enum" keyword and bracket
							if (c != '{')
								break;
						} else
							if (!isspace(c)) {	//syntax error
								if (arg_verify) {
									syntax_error = true;
									goto class_read_end_parsing;
								} else {
									i--;
									separator = ' ';
									expect    = SEMICOLON;
								}
							}
				
				break;
			}
			
			case VALUE : {
				if (c == '"') {
					if (word_start == -1)
						value_quoted = true;

					if (value_quoted)
						in_quote = !in_quote;
				}

				if (is_array && !in_quote) {
					if (c=='{' || c=='[') {
						array_level++;

						if (!arg_verify && copy_property && classpath_current==classpath_size && class_level>=classpath_current && !classpath_done) {
							StringDynamic_append(output_value[CURRENT_OUTPUT_LEVEL], "]+[");
							for(int x=0, z=array_level>1 ? array_level-1 : array_level; x<z; x++)
								StringDynamic_append(output_value[CURRENT_OUTPUT_LEVEL], "[");
						}
					}
				}

				if (word_start == -1) {
					if (!isspace(c) && (!is_array || (is_array && (c!='{' && c!='[' && c!=',' && c!=';' && c!='}')))) {
						word_start = i;
					}

					// If empty scalar value
					if (!is_array && c==';') {
						word_start = i;
						i--;
						continue;
					}
				} else {
					if (!in_quote && ((!is_array && (c==';' || c=='\r' || c=='\n')) || (is_array && (c==',' || c==';' || c=='}' || c==']')))) {
						if (!arg_verify && word_start!=-1 && copy_property && classpath_current==classpath_size && class_level>=classpath_current && !classpath_done) {
							if (purge_comment) {
								purge_comment = false;
								PurgeComments(text, word_start, i);
							}

							// Trim string
							int word_end = i;
							for (int z=i-1; z>word_start && isspace(text[z]); z--, word_end--);

							if (arg_trimdollar) {
								bool quote = text[word_start] == '\"';

								if (strncmpi(text+word_start+quote, "$STR", 4) == 0) {
									word_start++;
									if (quote)
										text[word_start] = '\"';
								}
							}
	
							text[word_end] = '\0';
							char *value = text + word_start;
							size_t length = word_end - word_start;
		
							if (lowercase_flag & CLASS_READ_LOWER_VALUE)
								for (size_t j=0; j<length; j++)
									value[j] = tolower(value[j]);
												
							StringDynamic_append(output_value[CURRENT_OUTPUT_LEVEL], "]+[");
											
							if (wrap == YES_WRAP || (wrap==NODOUBLE_WRAP && value[0] != '\"' && value[length-1] != '\"')) {
								StringDynamic_append(output_value[CURRENT_OUTPUT_LEVEL], "\"");
								StringDynamic_appendq(output_value[CURRENT_OUTPUT_LEVEL], value);
								StringDynamic_append(output_value[CURRENT_OUTPUT_LEVEL], "\"");
							} else
								StringDynamic_appendl(output_value[CURRENT_OUTPUT_LEVEL], value, length);
						}

						if (!is_array) {
							expect = SEMICOLON;
							copy_property = false;
						}

						word_start = -1;
					}
				}

				if (is_array && !in_quote && (c=='}' || c==']')) {
					if (!arg_verify && copy_property && classpath_current==classpath_size && class_level>=classpath_current && !classpath_done) {
						for (int z=array_level>1?array_level-1:array_level; word_start == -1 && z > 0; z--)
							StringDynamic_append(output_value[CURRENT_OUTPUT_LEVEL], "]");
					}

					array_level--;
					
					if (is_array && array_level == 0) {
						expect = SEMICOLON;
						copy_property = false;
					}
				}
			} break;
			
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
							StringDynamic *output_array = expect==CLASS_NAME ? output_class : output_inherit;
							int level                   = class_level - classpath_current;

							if (level < classpath_capacity && !classpath_done) {
								if (lowercase_flag & CLASS_READ_LOWER_CLASS)
									for (int z=word_start; text[z]!='\0'; z++)
										text[z] = tolower(text[z]);

								StringDynamic_appendf(output_array[level], "]+[\"%s\"", text+word_start);
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
								
								if (level < classpath_capacity && !classpath_done)
									StringDynamic_append(output_inherit[level], "]+[\"\"");
							}
						}
						
						if (classpath_match) {
							StringDynamic_appendf(output_classpath_bytes, "]+[\"%d\"", i+1);
							classpath_match = false;
						}
								
						if (!arg_verify && classpath_current==classpath_size && class_level>=classpath_current && !classpath_done) {
							int level = class_level - classpath_current;

							if (level < classpath_capacity)
								StringDynamic_appendf(output_bytes[level], "]+[\"%d\"", i+1);
						}
						
						class_level++;
						expect = PROPERTY;
							
						if (!arg_verify && classpath_current==classpath_size && class_level>classpath_current && !classpath_done) {
							int level = class_level - classpath_current;
							
							if (level > max_output_level)
								max_output_level = level;

							for (int j=level==0 ? 1 : level; j<classpath_capacity; j++)
								for (int k=0; k<all_output_strings_num; k++)
									StringDynamic_append(all_output_strings[k][j], "]+[[");
						}
					} else
						if (!isspace(c)) {	//syntax error
							if (arg_verify) {
								separator = '{';
								syntax_error = true;
								goto class_read_end_parsing;
							} else {
								i--;
								expect = SEMICOLON;
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
	class_read_end_parsing:

	char output_strings_name[all_output_strings_num][16] = {
		"class",
		"inherit",
		"bytes",
		"property",
		"value"
	};

	for (z=0; z<classpath_capacity && z<=max_output_level; z++) {
		for (int j=0; j<all_output_strings_num; j++) {
			QWritef("_output_%s%d=%s];", output_strings_name[j], z, all_output_strings[j][z].text);
			StringDynamic_end(all_output_strings[j][z]);
		}
	}
	
	if (arg_verify) {
		//ignore the unclosed classes
		/*if (!syntax_error && class_level>0) {
			syntax_error = true;
			separator    = '}';
		}*/

		if (syntax_error) {
			char error_msg[32]   = "";
			char encountered[16] = "end of file";
			char insteadof[16]   = "nothing";

			if (i < file_size)
				sprintf(encountered, "%c", text[i]);

			if (separator != ' ')
				sprintf(insteadof, "%c", separator);

			sprintf(error_msg, "%s instead of %s", encountered, insteadof);
			QWrite_err(FWERROR_CLASS_SYNTAX, 4, error_msg, line_num, column_num, argument[arg_file].text);
		} else
			QWrite_err(FWERROR_NONE, 0);
	} else {
		if (classpath_current < classpath_size)
			QWrite_err(FWERROR_CLASS_PARENT, 4, classpath[classpath_current], classpath_current, ++classpath_size, argument[arg_file].text);
		else
			if (arg_find_require && properties_to_find_size>0 && properties_found==0) {
				for (size_t i=0; i<argument[arg_find].length; i++)
					if (argument[arg_find].text[i] == '\"')
						argument[arg_find].text[i] = ' ';

				QWrite_err(FWERROR_CLASS_NOVAR, 2, argument[arg_find].text, argument[arg_file].text);
			} else
				QWrite_err(FWERROR_NONE, 0);
	}

	QWrite("[");
	
	for (z=0; z<classpath_capacity && z<=max_output_level; z++) {
		if (z == 0)
			QWrite("[");
		else
			QWrite(",[");

		for (int j=0; j<all_output_strings_num; j++)
			QWritef("%c_output_%s%d", (j==0 ? ' ' : ','), output_strings_name[j], z);

		QWrite("]");
	}

	QWritef("],%s],%d]", output_classpath_bytes.text, classpath_current);

	StringDynamic_end(output_classpath_bytes);
	StringDynamic_end(file_contents);
	StringDynamic_end(buf_filename);
}
break;












case C_CLASS_READSQM:
{ // Convert SQM file to SQF format

	global.option_error_output = OPTION_ERROR_ARRAY_CLOSE;
	size_t arg_file            = empty_char_index;
	bool arg_readsetpos        = false;
	
	for (size_t i=2; i<argument_num; i+=2) {
		switch (argument_hash[i]) {
			case NAMED_ARG_FILE : 
				arg_file = i + 1;
				break;

			case NAMED_ARG_READSETPOS :
				arg_readsetpos = String_bool(argument[i+1]);
				break;
		}
	}

	// File not specified
	if (argument[arg_file].length == 0) {
		QWrite_err(FWERROR_PARAM_EMPTY, 1, "arg_file");
		break;
	}

	// Verify and update path to the file
	StringDynamic buf_filename;
	StringDynamic_init(buf_filename);

	if (!VerifyPath(argument[arg_file], buf_filename, OPTION_ALLOW_MOST_LOCATIONS))
		break;
	//---------------------------------------------------------------------------
	


	// Open wanted file -----------------------------------------------------------------	
	FILE *file = fopen(argument[arg_file].text, "rb");
	if (!file) {
		QWrite_err(FWERROR_ERRNO, 2, errno, argument[arg_file].text);
		StringDynamic_end(buf_filename);
		break;
	}

	// Find file size
	if (fseek(file, 0, SEEK_END) != 0) {
		QWrite_err(FWERROR_ERRNO, 2, errno, argument[arg_file].text);
		StringDynamic_end(buf_filename);
		fclose(file);
		break;
	};

	size_t file_size = ftell(file);
	if (file_size == 0xFFFFFFFF) {
		QWrite_err(FWERROR_ERRNO, 2, errno, argument[arg_file].text);
		StringDynamic_end(buf_filename);
		fclose(file);
		break;
	}

	// Allocate buffer
	StringDynamic file_content_dynamic;
	StringDynamic_init(file_content_dynamic);
	
	int result = StringDynamic_allocate(file_content_dynamic, file_size+1);
	if (result != 0) {
		QWrite_err(FWERROR_MALLOC, 2, "file_content_dynamic", file_size);
		StringDynamic_end(buf_filename);
		break;
	}

	// Copy text to buffer
	fseek(file, 0, SEEK_SET);
	size_t bytes_read = fread(file_content_dynamic.text, 1, file_size, file);

	if (bytes_read != file_size) {		
		QWrite_err(FWERROR_ERRNO, 2, errno, argument[arg_file].text);
		StringDynamic_end(buf_filename);
		StringDynamic_end(file_content_dynamic);
		fclose(file);
		break;
	}

	file_content_dynamic.length = bytes_read;
	fclose(file);

	String file_content;
	file_content.text   = file_content_dynamic.text;
	file_content.length = file_content_dynamic.length;

	int inside         = 0;
	const int capacity = 8;
	
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

	bool class_vehicles_started = false;
	int units_in_this_group     = 0;
	SQM_ParseState state;
	SQM_Init(state);
	int items_expect[capacity]  = {0};
	int items_current[capacity] = {0};
	int items_highest[capacity] = {-1};
	int inside_backup[capacity] = {0};
	int opened_classes_backup[capacity][capacity] = {0};
	SQM_ParseState state_backup[capacity];
	
	for (i=0; i<capacity; i++)
		items_highest[i] = -1;

	while (state.i < file_content.length) {
		switch (SQM_Parse(file_content, state, SQM_ACTION_GET_NEXT_ITEM, empty_string)) {
			case SQM_OUTPUT_PROPERTY : {
				if (inside & CLASS_MISSION) {
					// Items have to be output in order - check if that is the case
					bool in_order = true;
					for (int y=state.class_level-1; y>=0; y--)
						if (items_highest[y]!=-1  &&  items_current[y]!=items_expect[y])
							in_order = false;

					// Convert array format by replacing brackets
					if (in_order  &&  state.property.text[state.property.length-1]==']' && state.property.text[state.property.length-2]=='[') {
						state.property.text[state.property.length-1] = ' ';
						state.property.text[state.property.length-2] = ' ';
						
						bool in_quote = false;
	
						for (size_t i=0; i<state.value.length; i++) {
							if (!in_quote  &&  state.value.text[i]=='{')
								state.value.text[i] = '[';
	
							if (!in_quote  &&  state.value.text[i]=='}')
								state.value.text[i] = ']';
	
							if (state.value.text[i] == '"')
								in_quote = !in_quote;
						}
					}
					
					// Remove "side=" from a group
					if (in_order  &&  inside & CLASS_GROUPS  &&  inside & CLASS_ITEM  &&  ~inside & CLASS_VEHICLES) {
						if (strncmpi("side",state.property.text,state.property.length)==0) {
							if (state.value.text[state.value.length-1] == ';')
								state.value.length--;
							
							QWrites(state.value);
						}
					}

					// Remember this position so we can come back here when items aren't in order
					if (strncmpi("items",state.property.text,state.property.length) == 0) {
						state_backup[state.class_level]  = state;
						inside_backup[state.class_level] = inside;
						
						for (int y=0; y<capacity; y++)
							opened_classes_backup[state.class_level][y] = opened_classes[y];
					}

					// Convert properties to local vars by adding underscore
					if (
						((inside & CLASS_GROUPS && inside & CLASS_VEHICLES && inside & CLASS_ITEM && state.class_level==5) || 
						(inside & CLASS_GROUPS && inside & CLASS_WAYPOINTS && inside & CLASS_ITEM && state.class_level==5) ||
						(~inside & CLASS_GROUPS && (inside & CLASS_VEHICLES || inside & CLASS_MARKERS || inside & CLASS_SENSORS) && inside & CLASS_ITEM) ||
						(inside & CLASS_EFFECTS) || 
						inside & CLASS_INTEL)
						&&
						in_order
					) {
						QWrite("_");
						QWritel(state.property.text, state.value_end - state.property_start);

						// Extract height number from init="this setpos"
						if (arg_readsetpos && inside & CLASS_VEHICLES && strncmpi("init",state.property.text,state.property.length)==0 && strncmpi("\"this setPos",state.value.text,12)==0) {
							int level      = 0;
							int comma      = 0;
							char *height   = NULL;
							bool is_number = true;
							size_t z       = 0;
							
							for (; z<state.value.length; z++) {
								if (level == 1) {
									if (height)
										if (state.value.text[z]!=']' && state.value.text[z]!='.' && !isspace(state.value.text[z]) && !isdigit(state.value.text[z]))
											is_number = false;
									
									if (state.value.text[z] == ',')
										comma++;

									if (comma==2  &&  !height)
										height = state.value.text + z + 1;
								}
								
								if (state.value.text[z] == '[')
									level++;

								if (state.value.text[z]==']') {
									level--;

									if (level == 0)
										break;
								}
							}

							if (height && is_number) {
								state.value.text[z] = '\0';
								QWrite("_setpos=[");

								if (tolower(state.value.text[12]) == 'a')	//setposasl
									QWrite("2,");
								else
									QWrite("1,");	//setpos

								QWrite(height);
								QWrite("];");
								state.value.text[z] = ']';
							}
						}
					}
				}
			} break;
			
			case SQM_OUTPUT_CLASS : {
				for (int z=0; z<class_max; z++) {
					if (strncmpi(state.class_name.text,class_names[z],strlen(class_names[z])) == 0) {
						int level             = state.class_level - 1;
						int opened            = class_ids[z];
						opened_classes[level] = opened;
						inside               |= opened;

						bool in_order = true;
						for (int y=level-1; y>=0; y--)
							if (items_highest[y]!=-1  &&  items_current[y]!=items_expect[y])
								in_order = false;

						if (inside & CLASS_MISSION  &&  in_order) {
							switch (opened) {
								case CLASS_INTEL  : QWrite("_Intel={"); break;
								case CLASS_GROUPS : QWrite("_Groups=["); break;
								
								case CLASS_ITEM : {
									// Find out class item number
									char backup                             = file_content.text[state.class_name_end];
									file_content.text[state.class_name_end] = '\0';
									int item_number                         = atoi(state.class_name.text+4);
									file_content.text[state.class_name_end] = backup;
									items_current[level]                    = item_number;
									
									// Remember the highest item number
									if (items_current[level] > items_highest[level])
										items_highest[level] = items_current[level];

									// Output only if the item number is correct
									if (items_current[level] == items_expect[level]) {
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
									}
								} break;
								
								case CLASS_WAYPOINTS : {
									units_in_this_group=0; 
									QWrite(",["); 
								} break;
								
								case CLASS_VEHICLES : {														
									if (~inside & CLASS_GROUPS)
										QWrite("_Vehicles=[");
									else {
										if (!class_vehicles_started) {	// class group might not contain class vehicles so add comma only if it does
											QWrite(",");
											class_vehicles_started = true;
										}
										
										QWrite("[");
									}
								} break;
								
								case CLASS_MARKERS : QWrite("_Markers=["); break;
								case CLASS_SENSORS : QWrite("_Sensors=["); break;
								case CLASS_EFFECTS : QWrite("_Effects={"); break;
							}
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
				
				if (inside & CLASS_MISSION) {
					// Loop back if the item with the highest number wasn't printed yet
					if (closed!=CLASS_ITEM  &&  items_expect[level+1]<=items_highest[level+1]) {
						state  = state_backup[level+1];
						inside = inside_backup[level+1];
						
						for (int y=0; y<capacity; y++)
							opened_classes[y] = opened_classes_backup[level+1][y];

						continue;
					} else {
						// Reset items counters
						items_expect[level+1]  = 0;
						items_current[level+1] = 0;
						items_highest[level+1] = -1;
					}

					bool in_order = true;
					for (int y=level-1; y>=0; y--)
						if (items_highest[y]!=-1  &&  items_current[y]!=items_expect[y])
							in_order = false;

					if (in_order)
						switch (closed) {
							case CLASS_INTEL    : QWrite("};"); break;
							case CLASS_GROUPS   : QWrite("];"); break;
							case CLASS_ITEM : {
								// Output only if the item number is correct
								if (items_current[level] == items_expect[level]) {
									items_expect[level]++;

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
								}
							} break;
							
							case CLASS_WAYPOINTS : QWrite("]"); break;
							
							case CLASS_VEHICLES : {
								if (~inside & CLASS_GROUPS)
									QWrite("];");
								else {
									class_vehicles_started = false;
									QWrite("]");
								}
							} break;
							
							case CLASS_MARKERS : QWrite("];"); break;
							case CLASS_SENSORS : QWrite("];"); break;
							case CLASS_EFFECTS : QWrite("};"); break;
						}
				}
			} break;
		}
	}

	class_readsqm_endparsing:

	//---------------------------------------------------------------------------
	QWrite_err(FWERROR_NONE, 0);

	StringDynamic_end(file_content_dynamic);
	StringDynamic_end(buf_filename);
}
break;













case C_CLASS_WRITE : 
{ // Modify class and properties in a file

	size_t arg_file           = empty_char_index;
	size_t arg_merge          = empty_char_index;
	size_t arg_path           = empty_char_index;
	size_t arg_deleteclass    = empty_char_index;
	size_t arg_deleteproperty = empty_char_index;
	size_t arg_renameclass    = empty_char_index;
	size_t arg_renameproperty = empty_char_index;
	size_t arg_to             = empty_char_index;
    size_t arg_offset         = empty_char_index;	
	size_t arg_setpos         = empty_char_index;
	const int setpos_size     = 128;
	int actions_specified     = 0;
	
	for (size_t i=2; i<argument_num; i+=2) {
		switch(argument_hash[i]) {
			case NAMED_ARG_FILE :
				arg_file = i + 1;
				break;
				
			case NAMED_ARG_MERGE : 
				arg_merge = i + 1;
				if (argument[i+1].length > 0) actions_specified++;
				break;
				
			case NAMED_ARG_PATH :
				arg_path = i + 1;
				break;
				
			case NAMED_ARG_DELETECLASS : 
				arg_deleteclass = i + 1;
				if (argument[i+1].length > 0) actions_specified++;
				break;
				
			case NAMED_ARG_DELETEPROPERTY : 
				arg_deleteproperty = i + 1;
				if (argument[i+1].length > 0) actions_specified++;
				break;
				
			case NAMED_ARG_RENAMECLASS : 
				arg_renameclass = i + 1;
				break;
				
			case NAMED_ARG_RENAMEPROPERTY : 
				arg_renameproperty = i + 1;
				break;
				
			case NAMED_ARG_TO : 
				arg_to = i + 1;
				if (argument[i+1].length > 0) actions_specified++;
				break;
				
			case NAMED_ARG_OFFSET :
				arg_offset = i + 1;
				break;

			case NAMED_ARG_SETPOS :
				arg_setpos = i + 1;
				if (argument[i+1].length > 0) actions_specified++;
				break;
		}
	}
	
	// File not specified
	if (argument[arg_file].length == 0) {
		QWrite_err(FWERROR_PARAM_EMPTY, 1, "arg_file");
		QWrite("0]");
		break;
	}

	// Check if any action arguments were specified
	if (argument[arg_to].length>0 && (argument[arg_renameclass].length==0 || argument[arg_renameproperty].length==0))
		actions_specified--;

	if (actions_specified == 0) {
		QWrite_err(FWERROR_PARAM_ACTION, 0);
		QWrite("0]");
		break;
	}
	
	// Verify and update path to the file
	StringDynamic buf_filename;
	StringDynamic_init(buf_filename);

	if (!VerifyPath(argument[arg_file], buf_filename, OPTION_LIMIT_WRITE_LOCATIONS)) {
		QWrite("0]");
		break;
	}
		


	// Open wanted file -----------------------------------------------------------------	
	FILE *file = fopen(argument[arg_file].text, "rb");
	if (!file) {
		QWrite_err(FWERROR_ERRNO, 2, errno, argument[arg_file].text);
		QWrite("0]");
		StringDynamic_end(buf_filename);
		break;
	}

	// Find file size
	if (fseek(file, 0, SEEK_END) != 0) {
		QWrite_err(FWERROR_ERRNO, 2, errno, argument[arg_file].text);
		QWrite("0]");
		StringDynamic_end(buf_filename);
		fclose(file);
		break;
	};

	size_t file_size = ftell(file);
	if (file_size == 0xFFFFFFFF) {
		QWrite_err(FWERROR_ERRNO, 2, errno, argument[arg_file].text);
		QWrite("0]");
		StringDynamic_end(buf_filename);
		fclose(file);
		break;
	};

	// Allocate buffer
	StringDynamic file_contents_dynamic;
	StringDynamic_init(file_contents_dynamic);

	int buffer_max = file_size + 1 + argument[arg_merge].length + (arg_setpos!=empty_char_index ? setpos_size : 0) + 128;
	int result     = StringDynamic_allocate(file_contents_dynamic, buffer_max);
	if (result != 0) {
		QWrite_err(FWERROR_MALLOC, 2, "file_contents_dynamic", buffer_max);
		QWrite("0]");
		StringDynamic_end(buf_filename);
		break;
	}

	// Copy text to buffer
	fseek(file, 0, SEEK_SET);
	size_t bytes_read            = fread(file_contents_dynamic.text, 1, file_size, file);
	file_contents_dynamic.length = bytes_read;

	if (bytes_read != file_size) {		
		QWrite_err(FWERROR_ERRNO, 2, errno, argument[arg_file].text);
		StringDynamic_end(buf_filename);
		StringDynamic_end(file_contents_dynamic);
		QWrite("0]");
		fclose(file);
		break;
	}

	fclose(file);

	SQM_ParseState source_state;
	SQM_Init(source_state);
	
	char *classpath[SQM_CLASSPATH_CAPACITY];
	String item;
	int classpath_current = 0;
	int classpath_size    = 0;
	size_t arg_path_pos   = 0;
	
	while ((item = String_tokenize(argument[arg_path],",",arg_path_pos,OPTION_TRIM_SQUARE_BRACKETS)).length>0  &&  classpath_size<SQM_CLASSPATH_CAPACITY)
		classpath[classpath_size++] = item.text;
	
	if (argument[arg_offset].length != 0) {
		source_state.i    = strtoul(argument[arg_offset].text, NULL, 0);
		classpath_current = classpath_size;
	} else {
		for (int i=0; i<classpath_size; i++) {
			String path          = {classpath[i], strlen(classpath[i])};
			String file_contents = {file_contents_dynamic.text, file_contents_dynamic.length};

			if (SQM_Parse(file_contents, source_state, SQM_ACTION_FIND_CLASS, path)) {
				classpath_current++;
			} else
				break;
				
		}
	}
	
	// If classes given in the path were found
	if (classpath_current == classpath_size) {
		SQM_ParseState source_state_backup = source_state;
		result                             = 0;
		bool save_changes                  = false;
		
		// Merge classes	
		if (argument[arg_merge].length > 0) {
			SQM_ParseState merge_state;
			SQM_Init(merge_state);

			if (SQM_Merge(argument[arg_merge], merge_state, file_contents_dynamic, source_state, NULL))
				save_changes = true;
		}
		
		// Delete property
		if (argument[arg_deleteproperty].length > 0) {
			source_state         = source_state_backup;
			String file_contents = {file_contents_dynamic.text, file_contents_dynamic.length};

			if ((result = SQM_Parse(file_contents, source_state, SQM_ACTION_FIND_PROPERTY, argument[arg_deleteproperty]))) {
				size_t removed_length = source_state.value_end - source_state.property_start;
				
				shift_buffer_chunk(file_contents.text, source_state.value_end, file_contents.length, removed_length, OPTION_LEFT);
				
				file_contents_dynamic.length -= removed_length;
				save_changes                  = true;
			} else {
				QWrite_err(FWERROR_CLASS_NOVAR, 2, argument[arg_deleteproperty].text, argument[arg_file].text);
				goto class_write_end;
			}
		}
		
		// Delete class
		if (argument[arg_deleteclass].length > 0) {
			source_state         = source_state_backup;
			String file_contents = {file_contents_dynamic.text, file_contents_dynamic.length};
	
			if ((result = SQM_Parse(file_contents, source_state, SQM_ACTION_FIND_CLASS, argument[arg_deleteclass]))) {
				SQM_Parse(file_contents, source_state, SQM_ACTION_FIND_CLASS_END, empty_string);
				
				size_t removed_length = source_state.i - source_state.class_start;
				
				shift_buffer_chunk(file_contents.text, source_state.i, file_contents.length, removed_length, OPTION_LEFT);
				
				file_contents_dynamic.length -= removed_length;
				save_changes                  = true;
			} else {
				QWrite_err(FWERROR_CLASS_NOCLASS, 2, arg_deleteclass, argument[arg_file].text);
				goto class_write_end;
			}
		}
		
		// Rename property
		if (argument[arg_renameproperty].length>0  &&  argument[arg_to].length>0) {
			source_state         = source_state_backup;
			String file_contents = {file_contents_dynamic.text, file_contents_dynamic.length};
	
			if ((result = SQM_Parse(file_contents, source_state, SQM_ACTION_FIND_PROPERTY, argument[arg_renameproperty]))) {
				size_t shift_amount  = argument[arg_to].length >= source_state.property.length ? argument[arg_to].length-source_state.property.length : source_state.property.length-argument[arg_to].length;
				bool shift_direction = argument[arg_to].length >= source_state.property.length;
				save_changes         = true;
				
				shift_buffer_chunk(file_contents.text, source_state.property_end, file_contents.length, shift_amount, shift_direction);
				memcpy(source_state.property.text, argument[arg_to].text, argument[arg_to].length);
				
				file_contents_dynamic.length += shift_amount * (shift_direction ? 1 : -1);
			} else {
				QWrite_err(FWERROR_CLASS_NOVAR, 2, arg_renameproperty, argument[arg_file].text);
				goto class_write_end;
			}
		} else
			// Rename class
			if (argument[arg_renameclass].length > 0  &&  argument[arg_to].length > 0) {
				source_state         = source_state_backup;
				String file_contents = {file_contents_dynamic.text, file_contents_dynamic.length};
				
				if ((result = SQM_Parse(file_contents, source_state, SQM_ACTION_FIND_CLASS, argument[arg_renameclass]))) {
					size_t current_name_length = source_state.class_name_full_end - source_state.class_name_start;
					size_t shift_amount        = argument[arg_to].length >= current_name_length ? argument[arg_to].length-current_name_length : current_name_length-argument[arg_to].length;
					bool shift_direction       = argument[arg_to].length >= current_name_length;
					save_changes               = true;
					
					shift_buffer_chunk(file_contents.text, source_state.class_name_full_end, file_contents.length, shift_amount, shift_direction);
					memcpy(source_state.class_name.text, argument[arg_to].text, argument[arg_to].length);
					
					file_contents_dynamic.length += shift_amount * (shift_direction ? 1 : -1);
				} else {
					QWrite_err(FWERROR_CLASS_NOCLASS, 2, arg_renameclass, argument[arg_file].text);
					goto class_write_end;
				}
			}

		// Add init line that changes the object's height
		if (arg_setpos != empty_char_index) {
			String item[4];
			size_t pos   = 0;
			source_state = source_state_backup;
			
			for (int i=0; i<4; i++)
				item[i] = String_tokenize(argument[arg_setpos],",",pos,OPTION_TRIM_SQUARE_BRACKETS);

			enum SQM_SETPOS {
				SQM_SETPOS_NONE,
				SQM_SETPOS_REL,
				SQM_SETPOS_ASL
			};
			int command_type = atoi(item[0].text);

			if (command_type == SQM_SETPOS_REL  ||  command_type == SQM_SETPOS_ASL) {
				char setpos_line[setpos_size];
				sprintf(setpos_line, "init=\"this setPos%s [%s, %s, %s]; \";", (command_type==SQM_SETPOS_ASL ? "ASL" : ""), item[1].text, item[2].text, item[3].text);

				String merge = {setpos_line, strlen(setpos_line)};
				SQM_ParseState merge_state;
				SQM_Init(merge_state);

				if (SQM_Merge(merge, merge_state, file_contents_dynamic, source_state, setpos_line))
					save_changes = true;
			}
		}
		

		// Rewrite file
		if (save_changes) {
			if ((file = fopen(argument[arg_file].text, "wb"))) {
				fwrite(file_contents_dynamic.text, 1, file_contents_dynamic.length, file);
	
				if (ferror(file)) 
					QWrite_err(FWERROR_ERRNO, 2, errno, argument[arg_file].text);
				else
					QWrite_err(FWERROR_NONE, 0);
	
				fclose(file);
			} else 
				QWrite_err(FWERROR_ERRNO, errno, argument[arg_file].text);
		} else 
			QWrite_err(FWERROR_NONE, 0);
	} else
		if (classpath_current < classpath_size)
			QWrite_err(FWERROR_CLASS_PARENT, 4, classpath[classpath_current], classpath_current, classpath_size, argument[arg_file].text);
		else
			QWrite_err(FWERROR_NONE, 0);

	class_write_end:
	QWritef("%d]", classpath_current);
	
	StringDynamic_end(file_contents_dynamic);
	StringDynamic_end(buf_filename);
} 
break;