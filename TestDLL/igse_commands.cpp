// -----------------------------------------------------------------
// IGSE DEDICATED COMMANDS
// -----------------------------------------------------------------

case C_IGSE_WRITE:
{ // Write line to a text file

	global.option_error_output = OPTION_ERROR_ARRAY_CLOSE;
	
	enum IGSE_WRITE_MODES {
		IGSE_WRITE_REPLACE,
		IGSE_WRITE_APPEND,
		IGSE_WRITE_NEW,
		IGSE_WRITE_INSERT,
		IGSE_WRITE_COPY,
		IGSE_WRITE_DELETE,
		IGSE_WRITE_CLEAR,
		IGSE_WRITE_MOVEUP,
		IGSE_WRITE_MOVEDOWN
	};

	enum IGSE_WRITE_MODES_OPENING {
		IGSE_WRITE_OPEN_CREATE,
		IGSE_WRITE_OPEN_RECREATE,
		IGSE_WRITE_OPEN_UNIQUE,
		IGSE_WRITE_OPEN_CHECK
	};

	size_t arg_file    = empty_char_index;
	size_t arg_text    = empty_char_index;
	char *arg_escape   = empty_char;
	bool arg_clipboard = false;
	bool arg_column    = false;
	int arg_line_start = 0;
	int arg_edit_mode  = IGSE_WRITE_REPLACE;
	int arg_open_mode  = IGSE_WRITE_OPEN_CREATE;
	int arg_column_num = 0;
	int arg_line_range = 1;

	for (size_t i=2; i<argument_num; i+=2) {
		switch (argument_hash[i]) {
			case NAMED_ARG_ESCAPE :
				arg_escape = argument[i+1].text;
				break;

			case NAMED_ARG_FILE : 
				arg_file = i + 1;
				break;

			case NAMED_ARG_LINE :
				arg_line_start = atoi(argument[i+1].text);
				break;

			case NAMED_ARG_COLUMN :
				arg_column_num = atoi(argument[i+1].text);
				arg_column     = true;
				break;

			case NAMED_ARG_MODE : {
				char modes[][32] = {
					"replace",
					"append",
					"new",
					"insert",
					"copy",
					"delete",
					"clear",
					"moveup",
					"movedown"
				};

				for (int j=0, max=sizeof(modes)/sizeof(modes[0]);  j<max;  j++)
					if (strcmpi(argument[i+1].text,modes[j]) == 0) {
						arg_edit_mode = j;
						break;
					}
			} break;

			case NAMED_ARG_TEXT :
				arg_text = i + 1;
				break;

			case NAMED_ARG_CLIP :
				arg_clipboard = String_bool(argument[i+1]);
				break;

			case NAMED_ARG_RANGE :
				arg_line_range = atoi(argument[i+1].text);
				break;

			case NAMED_ARG_OPEN : {
				char modes[][32] = {
					"create",
					"recreate",
					"unique",
					"check"
				};

				for (int j=0, max=sizeof(modes)/sizeof(modes[0]);  j<max;  j++)
					if (strcmpi(argument[i+1].text,modes[j]) == 0) {
						arg_open_mode = j;
						break;
					}
			} break;
		}
	}

	
	// File not specified
	if (argument[arg_file].length == 0) {
		QWrite_err(FWERROR_PARAM_EMPTY, 1, "arg_file");
		break;
	}
	
	// Cannot move up the first line
	if (arg_edit_mode==IGSE_WRITE_MOVEUP  &&  arg_line_start==1) {
		QWrite_err(FWERROR_PARAM_ONE, 2, "arg_line_start", arg_line_start);
		break;
	}

	// Verify and update path to the file
	StringDynamic buf_filename;
	StringDynamic_init(buf_filename);
	
	if (!VerifyPath(argument[arg_file], buf_filename, OPTION_RESTRICT_TO_MISSION_DIR))
		break;
	

	// Does clipboard contains text
	HANDLE hClipboardData;

	if (arg_clipboard) {
		if (OpenClipboard(NULL)) {
			if (::IsClipboardFormatAvailable(CF_TEXT)) {
				hClipboardData            = GetClipboardData(CF_TEXT);
				argument[arg_text].text   = (char*)GlobalLock(hClipboardData);
				argument[arg_text].length = strlen(argument[arg_text].text);
			} else {
				QWrite_err(FWERROR_CLIP_FORMAT, 0);
				StringDynamic_end(buf_filename);
				CloseClipboard(); 
				break;
			}
		} else {
			QWrite_err(FWERROR_CLIP_OPEN, 1, GetLastError());
			StringDynamic_end(buf_filename);
			break;
		}
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
			String_escape_sequences(argument[arg_text], OPTION_TAB, quantity);
			quantity = -1;
		}

		if (arg_escape[i] == 'n') {
			String_escape_sequences(argument[arg_text], OPTION_CRLF, quantity);
			quantity = -1;
		}
	}


	// Move down is move up
	if (arg_edit_mode == IGSE_WRITE_MOVEDOWN) {
		arg_edit_mode = IGSE_WRITE_MOVEUP;
		arg_line_start++;
	}

	// Default append at the end
	if (!arg_column  &&  (arg_edit_mode==IGSE_WRITE_APPEND  ||  arg_edit_mode == IGSE_WRITE_NEW))
		arg_column_num = -1;

	// Calculate range of lines to modify
	int line_range_end = (abs(arg_line_start) + abs(arg_line_range)) * (arg_line_start>0 ? 1 : -1) + (arg_line_start > 0 ? -1 : 1);
	//---------------------------------------------------------------------------
	


	// Open wanted file -----------------------------------------------------------------
	char open_mode[3] = "rb";

	if (arg_line_start==0  &&  arg_open_mode!=IGSE_WRITE_OPEN_CHECK) {
		strcpy(open_mode, "ab");

		if (arg_open_mode == IGSE_WRITE_OPEN_RECREATE)
			strcpy(open_mode, "wb");
	}

	FILE *file = fopen(argument[arg_file].text, open_mode);
	if (!file) {
		QWrite_err(FWERROR_ERRNO, 2, errno, argument[arg_file].text);
		StringDynamic_end(buf_filename);
		break;
	}

	if (arg_open_mode == IGSE_WRITE_OPEN_UNIQUE) {
		QWrite_err(FWERROR_FILE_EXISTS, 1, argument[arg_file].text);
		StringDynamic_end(buf_filename);
		fclose(file);
		break;
	}


	// Find file size
	if (fseek(file, 0, SEEK_END) != 0) {
		QWrite_err(FWERROR_ERRNO, 2, errno, argument[arg_file].text);
		StringDynamic_end(buf_filename);
		fclose(file);
		break;
	}

	size_t file_size = ftell(file);	//need uint because fread/fwrite returns uint
	if (file_size == 0xFFFFFFFF) {
		QWrite_err(FWERROR_ERRNO, 2, errno, argument[arg_file].text);
		StringDynamic_end(buf_filename);
		fclose(file);
		break;
	}

	// Quick append mode
	if (arg_line_start == 0) {
		if (arg_open_mode == IGSE_WRITE_OPEN_CHECK)
			freopen(argument[arg_file].text, "ab", file);
		
		if (file_size>0  &&  arg_edit_mode != IGSE_WRITE_APPEND) 
			fprintf(file, "\r\n");

		fprintf(file, "%s", argument[arg_text].text);

		if (ferror(file))
			QWrite_err(FWERROR_ERRNO, 2, errno, argument[arg_file].text);
		else
			QWrite_err(FWERROR_NONE, 0);

		StringDynamic_end(buf_filename);
		fclose(file);
		break;
	}


	// Allocate buffer
	StringDynamic file_contents;
	StringDynamic_init(file_contents);

	size_t new_line_len  = argument[arg_text].length;
	size_t new_file_size = file_size * (arg_edit_mode==IGSE_WRITE_COPY ? 2 : 1) + new_line_len + 2 + 1;
	int result           = StringDynamic_allocate(file_contents, new_file_size);

	if (result != 0) {
		QWrite_err(FWERROR_MALLOC, 2, "file_contents", new_file_size);
		StringDynamic_end(buf_filename);
		break;
	}


	// Copy text to buffer
	fseek(file, 0, SEEK_SET);
	size_t bytes_read = fread(file_contents.text, 1, file_size, file);

	if (bytes_read != file_size) {
		StringDynamic_end(buf_filename);
		StringDynamic_end(file_contents);
		QWrite_err(FWERROR_ERRNO, 2, errno, argument[arg_file].text);
		fclose(file);
		break;
	}

	fclose(file);



	// Set vars for parsing text
	char *buffer                 = file_contents.text;
	char *new_line               = argument[arg_text].text;
	int line_num                 = 0;
	int direction                = arg_line_start > 0 ? 1 : -1;
	size_t line_start_pos        = direction>0 ? 0 : file_size;
	size_t last_new_line         = file_size;
	bool took_action             = false;
	size_t line_shift_dest_pos   = 0;
	size_t line_shift_source_pos = 0;
	char *line_shift_dest;
	char *line_shift_source;

	for (i=direction>0 ? 0 : file_size;  direction>0 && i<=file_size || direction<0 && i>=0;  direction>0?i++:i--) {
		if (buffer[i]=='\n'  ||  direction>0 && i==file_size  ||  direction<0  && i==0) {
			size_t *separator     = direction>0 ? &i : &last_new_line;
			char prev             = buffer[*separator];
			bool carriage_return  = false;
			bool revert_separator = true;
			buffer[*separator]    = '\0';

			if (*separator>0  && buffer[*separator-1]=='\r') {
				carriage_return      = true;
				buffer[*separator-1] = '\0';
			}

			if (direction < 0)
				line_start_pos = i + (buffer[i]=='\n' ? 1 : 0);
				
			char *line        = buffer + line_start_pos;
			char *line_start  = buffer + line_start_pos;			
			line_num         += direction;

			if (abs(line_num) == abs(arg_line_start)-1  &&  arg_edit_mode==IGSE_WRITE_MOVEUP) {
				line_shift_dest     = line;
				line_shift_dest_pos = line_start_pos;
			}
			
			if (abs(line_num) >= abs(arg_line_start)  &&  abs(line_num) <= abs(line_range_end)) {
				took_action = true;
				
				size_t line_len   = carriage_return ? *separator-line_start_pos-1 : *separator-line_start_pos;
				size_t column_num = arg_column_num;
				
				// If negative then count from the end of the string			
				if (column_num < 0)
					column_num += line_len + 1;
					
				// If still negative then assume beginning of the string
				if (column_num < 0)
					column_num = 0;
				
				// If too far then assume end of the string
				if (column_num > line_len)
					column_num = line_len;				

				line_start_pos += column_num;
				line_start      = buffer + line_start_pos;

				line_len                    = carriage_return ? *separator-line_start_pos-1 : *separator-line_start_pos;
				size_t buffer_shift_amount  = 0;
				bool buffer_shift_direction = OPTION_RIGHT;

				if (arg_edit_mode == IGSE_WRITE_REPLACE) {
					buffer_shift_amount    = new_line_len >= line_len ? new_line_len-line_len : line_len-new_line_len;
					buffer_shift_direction = new_line_len >= line_len;
				}
					
				char *line_end   = line_start + line_len;
				int line_end_pos = carriage_return ? *separator-1 : *separator;
				
				if (arg_edit_mode == IGSE_WRITE_APPEND  ||  arg_edit_mode == IGSE_WRITE_NEW  ||  arg_edit_mode == IGSE_WRITE_INSERT) {
					buffer_shift_amount = new_line_len;
					line_end            = line_start;
					line_end_pos        = line_start_pos;
					
					if (arg_edit_mode == IGSE_WRITE_NEW  ||  arg_edit_mode == IGSE_WRITE_INSERT) {
						buffer_shift_amount += 2;
						
						if (arg_edit_mode == IGSE_WRITE_NEW && arg_column)
							buffer_shift_amount += 2;
					}
				}
				
				if (arg_edit_mode == IGSE_WRITE_COPY) {
					buffer_shift_amount    = line_len + (carriage_return ? 2 : 1);
					buffer_shift_direction = OPTION_RIGHT;

					shift_buffer_chunk(buffer, line_start_pos, file_size, buffer_shift_amount, buffer_shift_direction);
					memcpy(line_start+line_len, "\r\n", 2);
				}
				
				if (arg_edit_mode == IGSE_WRITE_DELETE  ||  arg_edit_mode == IGSE_WRITE_CLEAR) {
					buffer_shift_amount    = line_len;
					buffer_shift_direction = OPTION_LEFT;
					
					if (arg_edit_mode == IGSE_WRITE_DELETE) {
						revert_separator = false;
						
						if (prev == '\n') {
							line_end            += (carriage_return ? 2 : 1);
							line_end_pos        += (carriage_return ? 2 : 1);
							buffer_shift_amount += (carriage_return ? 2 : 1);
						} else {
							// If last line then we need to remove previous \n
							if (line_start_pos>0  &&  buffer[line_start_pos-1]=='\n') buffer_shift_amount++;
							if (line_start_pos>1  &&  buffer[line_start_pos-2]=='\r') buffer_shift_amount++;
						}
					}
					
					shift_buffer_chunk(buffer, line_end_pos, file_size, buffer_shift_amount, buffer_shift_direction);
				}
				
				if (arg_edit_mode == IGSE_WRITE_REPLACE  ||  arg_edit_mode == IGSE_WRITE_APPEND  ||  arg_edit_mode == IGSE_WRITE_NEW  ||  arg_edit_mode == IGSE_WRITE_INSERT) {
					shift_buffer_chunk(buffer, line_end_pos, file_size, buffer_shift_amount, buffer_shift_direction);

					if (arg_edit_mode == IGSE_WRITE_NEW) {
						memcpy(line_start, "\r\n", 2);
						line_start += 2;

						if (arg_column)
							memcpy(line_start+new_line_len, "\r\n", 2);
					}

					if (arg_edit_mode == IGSE_WRITE_INSERT)
						memcpy(line_start+new_line_len, "\r\n", 2);

					memcpy(line_start, new_line, new_line_len);
				}
				
				if (abs(line_num) == abs(arg_line_start)  &&  arg_edit_mode == IGSE_WRITE_MOVEUP) {
					line_shift_source     = line;
					line_shift_source_pos = line_start_pos;
				}

				if (abs(line_num) == abs(line_range_end)  &&  arg_edit_mode == IGSE_WRITE_MOVEUP) {
					bool previous_line_carriage = buffer[line_start_pos-2] == '\r';

					// Include \r\n in calculations
					line_end     = buffer + *separator;
					line_end_pos = *separator + (prev=='\0' ? 0 : 1);
					line_len     = line_end_pos - line_start_pos;
					
					// turn \0 back to \r\n
					revert_separator   = false;
					buffer[*separator] = prev;
					if (carriage_return) buffer[*separator-1] = '\r';

					// Copy previous line to an extra buffer
					StringDynamic previous_line_backup;
					StringDynamic_init(previous_line_backup);
					int previous_line_len = line_shift_source_pos - line_shift_dest_pos;
					StringDynamic_appendl(previous_line_backup, line_shift_dest, previous_line_len);

					// Move current line to the previous line
					for (int k=line_shift_source_pos; k<line_end_pos; k++)
						buffer[k-previous_line_len] = buffer[k];

					// If current line is without EOL
					if (prev == '\0') {
						if (previous_line_carriage) {
							buffer[line_shift_dest_pos+line_len]   = '\r';
							buffer[line_shift_dest_pos+line_len+1] = '\n';
							line_len          += 2;
							previous_line_len -= 2;
						} else {
							buffer[line_shift_dest_pos+line_len] = '\n';
							line_len          += 1;
							previous_line_len -= 1;
						}
					}

					// Paste line from an extra buffer
					memcpy(buffer+line_shift_dest_pos+line_len, previous_line_backup.text, previous_line_len);
					StringDynamic_end(previous_line_backup);
				}

				*separator       += buffer_shift_amount * (buffer_shift_direction ? 1 : -1);
				file_size        += buffer_shift_amount * (buffer_shift_direction ? 1 : -1);
				buffer[file_size] = '\0';
			}

			if (revert_separator) {
				buffer[*separator] = prev;
	
				if (carriage_return)
					buffer[*separator-1] = '\r';
			}
				
			last_new_line = i;
				
			if (direction > 0)
				line_start_pos = i + 1;
		}
	}

	if (arg_clipboard) {
		GlobalUnlock(hClipboardData);
		CloseClipboard();
	}
	//---------------------------------------------------------------------------

	if (took_action) {
		if ((file = fopen(argument[arg_file].text, "wb"))) {
			size_t bytes_written = fwrite(buffer, 1, file_size, file);

			if (bytes_written != file_size)
				QWrite_err(FWERROR_ERRNO, 2, errno, argument[arg_file].text);				
			else
				QWrite_err(FWERROR_NONE, 0);

			fclose(file);
		} else
			QWrite_err(FWERROR_ERRNO, 2, errno, argument[arg_file].text);
	} else 		
		QWrite_err(FWERROR_FILE_NOLINE, 2, abs(line_num) < abs(arg_line_start) ? arg_line_start : line_range_end, argument[arg_file].text);

	StringDynamic_end(file_contents);
	StringDynamic_end(buf_filename);
}
break;












case C_IGSE_LIST:
{ // IGSE List of files

	enum IGSE_LIST_LIMITTO {
		NO_LIMIT     = 0,
		ONLY_FILES   = 1,
		ONLY_FOLDERS = 2
	};

	bool arg_system_time = false;
	bool arg_lowercase   = false;
	bool arg_only_name   = false;
	int arg_limit_to     = NO_LIMIT;
	char default_path[2] = "*";
	String arg_path      = {default_path, 1};

	for (size_t i=2; i<argument_num; i+=2) {
		switch (argument_hash[i]) {
			case NAMED_ARG_SYSTEMTIME :
				arg_system_time = String_bool(argument[i+1]);
				break;

			case NAMED_ARG_ONLYNAME :
				arg_only_name = String_bool(argument[i+1]);
				break;

			case NAMED_ARG_PATH :
				arg_path = argument[i+1];
				break;

			case NAMED_ARG_LOWERCASE : 
				arg_lowercase = String_bool(argument[i+1]);
				break;

			case NAMED_ARG_LIMITTO :
				if (strcmpi(argument[i+1].text,"files") == 0)
					arg_limit_to = ONLY_FILES;
				else
					if (strcmpi(argument[i+1].text,"folders") == 0)
						arg_limit_to = ONLY_FOLDERS;
					else
						arg_limit_to = NO_LIMIT;
				break;
		}
	}


	// Verify and update path to the file
	StringDynamic buf_path;
	StringDynamic_init(buf_path);
	
	if (!VerifyPath(arg_path, buf_path, OPTION_ALLOW_GAME_ROOT_DIR)) {
		QWrite("[],[]]"); 
		break;
	}


	// Get list of files
	WIN32_FIND_DATA file_data;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	hFind		 = FindFirstFile(arg_path.text, &file_data);

	if (hFind != INVALID_HANDLE_VALUE) {
		StringDynamic Names;
		StringDynamic Attributes;
		StringDynamic_init(Names);
		StringDynamic_init(Attributes);

		do {
			if (strcmp(file_data.cFileName,".")==0  ||  strcmp(file_data.cFileName,"..")==0)
				continue;

			if (arg_limit_to==ONLY_FILES  &&  file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				continue;

			if (arg_limit_to==ONLY_FOLDERS  &&  ~file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				continue;

			if (arg_lowercase)
				for (int i=0; file_data.cFileName[i]!='\0'; i++)
					file_data.cFileName[i] = tolower(file_data.cFileName[i]);

			StringDynamic_append(Names, "]+[\"");
			StringDynamic_appendq(Names, file_data.cFileName);
			StringDynamic_append(Names, "\"");

			if (!arg_only_name) {
				StringDynamic_appendf(Attributes, "]+[");
				
				// Name without extension and then extension alone
				char *dot = strrchr(file_data.cFileName, '.');
				char *ext = empty_char;

				if (dot) {
					int pos                  = dot - file_data.cFileName;
					file_data.cFileName[pos] = '\0';
					ext                      = file_data.cFileName + pos + 1;
				}

				StringDynamic_appendf(Attributes, "[\"%s\",\"%s\",[", file_data.cFileName, ext);


				// List of attributes
				if ((file_data.dwFileAttributes & FILE_ATTRIBUTE_NORMAL) == 0) {
					bool add_comma       = false;
					char attribute[][40] = {
						"readonly",
						"hidden",
						"system",
						"",
						"directory",
						"archive",
						"device",
						"",
						"temporary",
						"sparse",
						"reparse",
						"compressed",
						"offline",
						"notcontentindexed",
						"encrypted",
						"integritystream",
						"virtual",
						"noscrubdata"
					};

					for (int bitmask=1,index=0;  bitmask<0x20000;  bitmask*=2,index++) {
						if (bitmask!=8  &&  bitmask!=FILE_ATTRIBUTE_NORMAL  &&  (file_data.dwFileAttributes & bitmask)!=0) {
							if (add_comma) 
								StringDynamic_appendf(Attributes, ","); 
							else 
								add_comma = true;

							StringDynamic_appendf(Attributes, "\"%s\"", attribute[index]);
						}
					}
				}

				StringDynamic_appendf(Attributes, "],");

				// Creation, last access and last write time
				FILETIME *date_list[] = {&file_data.ftCreationTime, &file_data.ftLastAccessTime, &file_data.ftLastWriteTime};

				for (int i=0, max=sizeof(date_list)/sizeof(date_list[0]); i<max; i++) {
					char temp[64] = "";
					FileTimeToString(*date_list[i], arg_system_time, temp);
					StringDynamic_appendf(Attributes, "%s%s", (i>0 ? "," : ""), temp);
				}

				// Size
				FileSize size = DivideBytes(file_data.nFileSizeLow);
				StringDynamic_appendf(Attributes, ",[%f,%f,%f]]", size.bytes, size.kilobytes, size.megabytes);
			}
		} while (FindNextFile(hFind, &file_data));
		FindClose(hFind);

		QWrite_err(FWERROR_NONE, 0);
		QWritef("[%s],[%s]]", Names.text, Attributes.text);
		StringDynamic_end(Names);
		StringDynamic_end(Attributes);
	} else {
		QWrite_err(FWERROR_WINAPI, 2, GetLastError(), arg_path.text);
		QWrite("[],[]]"); 
		break;
	}

	StringDynamic_end(buf_path);
}
break;








case C_IGSE_LOAD:
{ // Load lines from a text file

	enum OUTPUT_TYPE {
		OUTPUT_SQS_CLUSTER,
		OUTPUT_SQS_PARALLEL,
		OUTPUT_EXECUTE,
		OUTPUT_CLIP
	};
	
	size_t arg_file           = empty_char_index;
	int arg_line_start        = 0;
	int arg_line_end          = 0;
	int arg_offset_start      = 0;
	int arg_limit_line_length = -1;
	int arg_wait              = 0;
	int arg_output            = OUTPUT_SQS_PARALLEL;
	bool arg_offset           = false;
	bool arg_split_lines      = false;
	bool arg_delete_file      = false;
	bool arg_clip_append      = false;
	bool arg_output_text      = true;
	bool arg_output_offset    = false;
	bool arg_output_linelen   = false;
			
	for (size_t i=2; i<argument_num; i+=2) {
		switch (argument_hash[i]) {
			case NAMED_ARG_FILE :
				arg_file = i + 1;
				break;

			case NAMED_ARG_START :
				arg_line_start = atoi(argument[i+1].text);
				break;

			case NAMED_ARG_END :
				arg_line_end = atoi(argument[i+1].text);
				break;

			case NAMED_ARG_OFFSET :
				arg_offset_start = atoi(argument[i+1].text);
				arg_offset       = true;
				break;

			case NAMED_ARG_CUT :
				arg_limit_line_length = atoi(argument[i+1].text);
				break;

			case NAMED_ARG_SPLIT :
				arg_split_lines = String_bool(argument[i+1]);
				break;

			case NAMED_ARG_MODE : {
				if (strcmpi(argument[i+1].text,"execute") == 0) {
					arg_output                  = OUTPUT_EXECUTE;
					global.option_error_output |= OPTION_ERROR_ARRAY_SUPPRESS;
				} else
					if (strcmpi(argument[i+1].text,"clipcopy") == 0)
						arg_output    = OUTPUT_CLIP;
					else
						if (strcmpi(argument[i+1].text,"clipappend") == 0) {
							arg_clip_append = true;
							arg_output      = OUTPUT_CLIP;
						} else
							if (strcmpi(argument[i+1].text,"cluster") == 0)
								arg_output = OUTPUT_SQS_CLUSTER;
			} break;

			case NAMED_ARG_DELETE :
				arg_delete_file = String_bool(argument[i+1]);
				break;

			case NAMED_ARG_WAIT :
				arg_wait = atoi(argument[i+1].text);
				break;
				
			case NAMED_ARG_OUTPUT :
				arg_output_text    = false;
				arg_output_offset  = false;
				arg_output_linelen = false;
				
				String item;
				size_t tokenize_pos = 0;
				while ((item = String_tokenize(argument[i+1], ",", tokenize_pos, OPTION_TRIM_SQUARE_BRACKETS)).length > 0) {
					if (strcmpi(item.text,"text") == 0)
						arg_output_text = true;
					else
						if (strcmpi(item.text,"offset") == 0)
							arg_output_offset = true;
						else
							if (strcmpi(item.text,"length") == 0)
								arg_output_linelen = true;
				}
				break;
		}
	}


	// File not specified
	if (argument[arg_file].length == 0) {
		QWrite_err(FWERROR_PARAM_EMPTY, 1, "arg_file");
		
		if (arg_output != OUTPUT_EXECUTE)
			QWrite("[],0,false,false,\"\",[],[]]");

		break;
	}

	// Number validation
	if (arg_line_start>arg_line_end  &&  ((arg_line_start>0 && arg_line_end>0) || (arg_line_start<0 && arg_line_end<0))) {
		QWrite_err(FWERROR_PARAM_RANGE, 4, "arg_line_start", "arg_line_end", arg_line_start, arg_line_end);
		
		if (arg_output != OUTPUT_EXECUTE)
			QWrite("[],0,false,false,\"\",[],[]]");

		break;
	}


	// Verify and update path to the file
	StringDynamic buf_filename;
	StringDynamic_init(buf_filename);
	int path_type = VerifyPath(argument[arg_file], buf_filename, OPTION_ALLOW_GAME_ROOT_DIR);

	if (path_type == PATH_ILLEGAL) {
		if (arg_output != OUTPUT_EXECUTE)
			QWrite("[],0,false,false,\"\",[],[]]");
			
		break;
	}


	FILE *file;
	int c        = 0;
	int line_num = 1;
	
	// Open file (optionaly keep trying again for some time)
	do {
		file = fopen(argument[arg_file].text, "rb");
		if (!file  &&  arg_wait>0) {
			Sleep(10);
			arg_wait--;
		}
	} while (!file  &&  arg_wait>0);

	// Failed to open
	if (!file) {	
		if (arg_output != OUTPUT_EXECUTE) {
			QWrite_err(FWERROR_ERRNO, 2, errno, argument[arg_file].text);
			QWrite("[],0,false,false,\"\",[],[]]");
		}

		StringDynamic_end(buf_filename);
		break;
	}

	
	// Count the number of lines in a file if range start and range end are positive and negative
	if ((arg_line_start<0 && arg_line_end>0)  ||  (arg_line_start>0 && arg_line_end<0)) {
		do {
			c = fgetc(file);
			if (c == '\n') line_num++;
		} while (c != EOF);
		
		// Convert negative to positive line number
		if (arg_line_start < 0)
			arg_line_start = line_num + 1 + arg_line_start;
			
		if (arg_line_end < 0)
			arg_line_end = line_num + 1 + arg_line_end;
			
		// If numbers are still negative
		if (arg_line_start < 0)
			arg_line_start = 1;
			
		if (arg_line_end < 0)
			arg_line_end = 1;
			
		// Validate range
		if (arg_line_start>arg_line_end  &&  ((arg_line_start>0 && arg_line_end>0) || (arg_line_start<0 && arg_line_end<0))) {
			QWrite_err(FWERROR_PARAM_RANGE, 4, "arg_line_start", "arg_line_end", arg_line_start, arg_line_end);
			
			if (arg_output != OUTPUT_EXECUTE)
				QWrite("[],0,false,false,\"\",[],[]]");
	
			break;
		}
	}


	// Find file size
	if (fseek(file, 0, SEEK_END) != 0) {
		QWrite_err(FWERROR_ERRNO, 2, errno, argument[arg_file].text);

		if (arg_output != OUTPUT_EXECUTE)
			QWrite("[],0,false,false,\"\",[],[]]");

		StringDynamic_end(buf_filename);
		fclose(file);
		break;
	}

	int file_size = ftell(file);
	if (file_size == -1) {
		QWrite_err(FWERROR_ERRNO, 2, errno, argument[arg_file].text);

		if (arg_output != OUTPUT_EXECUTE)
			QWrite("[],0,false,false,\"\",[],[]]");

		StringDynamic_end(buf_filename);
		fclose(file);
		break;
	}

	int offset_end = file_size;

	
	// Count the number of lines from the end if range start or range end are negative
	if (arg_line_start<0  ||  arg_line_end<0) {
		fseek(file, -1, SEEK_CUR);
		line_num = 0;
		bool end = false;
		
		while (true) {
			if (ftell(file) == 0)
				end = true;

			c = fgetc(file);
			
			if (c == '\n') {
				line_num--;
				
				if (arg_line_start == line_num) {
					arg_line_start   = 1;
					arg_offset_start = ftell(file);
				}
				
				if (arg_line_end == line_num) {
					arg_line_end = 0;
					offset_end   = ftell(file);
				}
			} 
			
			if (end  ||  (arg_line_start>=0 && arg_line_end>=0))
				break;
			
			fseek(file, -2, SEEK_CUR);
		}
		
		if (arg_line_start < 0)
			arg_line_start = 1;
			
		if (arg_line_end < 0)
			arg_line_end = 1;
	}
	
	fseek(file, arg_offset_start, SEEK_SET);


	StringDynamic sqs_lines;
	StringDynamic sqs_offsets;
	StringDynamic sqs_lengths;
	StringDynamic buf_for_clip;
	StringDynamic_init(sqs_lines);
	StringDynamic_init(sqs_offsets);
	StringDynamic_init(sqs_lengths);
	StringDynamic_init(buf_for_clip);
	
	int line_start  = -1;
	int line_end    = 0;
	int line_split  = 0;
	int line_count  = 0;
	int pos         = -1;
	bool line_empty = false;
	line_num        = arg_offset ? arg_line_start : 1;

	// Go through the file one character at a time
	do {
		c = fgetc(file);
		
		if (c == 0x1F)	// convert unit separator to record separator so that it won't be tokenized by ParseScript()
			c = 0x1E;

		char cc = (char)c;
		
		if (ferror(file))
			break;
		
		pos++;
		
		// If end of the line
		if (c==EOF  ||  c=='\n') {
			line_count++;

			if (c == EOF)
				line_end = pos;
				
			// If line is empty
			if (line_start == -1) {
				line_start = pos;
				line_end   = pos;
				line_empty = true;
				
				if (line_num >= arg_line_start) {
					switch (arg_output) {
						case OUTPUT_SQS_PARALLEL : {
							if (arg_output_text) {
								if (arg_split_lines)
									StringDynamic_appendl(sqs_lines, "]+[[\"", 5);
								else
									StringDynamic_appendl(sqs_lines, "]+[\"", 4);
							}
								
							if (arg_output_offset)
								StringDynamic_appendf(sqs_offsets, "\"%d\"]+[", arg_offset_start + (c==EOF ? pos : pos-1));
						} break;
						
						case OUTPUT_SQS_CLUSTER : {
							StringDynamic_append(sqs_lines, "]+[[");
							
							if (arg_output_text)
								StringDynamic_append(sqs_lines, "\"");
						} break;
					};
				}
			}
				
			int line_length = line_end - line_start;
			
			if (arg_limit_line_length!=-1  &&  line_length>arg_limit_line_length  &&  !arg_split_lines)
				line_length = arg_limit_line_length;
			
			// Save end of line
			if (line_num >= arg_line_start) {
				switch (arg_output) {
					case OUTPUT_EXECUTE      : if (c!=EOF) QWritel(&cc, 1); break;
					case OUTPUT_CLIP         : if (c!=EOF) StringDynamic_appendl(buf_for_clip, &cc, 1); break;
					case OUTPUT_SQS_PARALLEL : {
						if (arg_output_text)
							StringDynamic_append(sqs_lines, "\"");
						
						if (arg_output_linelen)
							StringDynamic_appendf(sqs_lengths, "%d]+[", line_length);
						
						if (arg_output_text && arg_split_lines)
							StringDynamic_append(sqs_lines, "]");
					} break;
					case OUTPUT_SQS_CLUSTER : {
						if (arg_output_text)
							StringDynamic_append(sqs_lines, "\"");
						
						if (arg_output_offset) {
							if (arg_output_text) StringDynamic_append(sqs_lines, ",");
							StringDynamic_appendf(sqs_lines, "\"%d\"", arg_offset_start + line_start);
						}
						
						if (arg_output_linelen) {
							if (arg_output_text || arg_output_offset) StringDynamic_append(sqs_lines, ",");
							StringDynamic_appendf(sqs_lines, "%u", line_length);
						}
						
						StringDynamic_append(sqs_lines, "]");
					} break;
				}
			}

			if (c != EOF) {
				line_num++;
				line_start = -1;
			}
			
			// End parsing if crossed range end
			if ((arg_line_end>0  &&  line_num>arg_line_end)  ||  pos>=offset_end)
				break;
		// When in the middle of the line
		} else {
			// Begin the line
			if (line_start == -1) {
				line_start = pos;
				line_empty = false;
				line_split = 0;
				
				if (line_num >= arg_line_start) {
					switch(arg_output) {
						case OUTPUT_SQS_PARALLEL : {
							if (arg_output_text) {
								if (arg_split_lines)
									StringDynamic_append(sqs_lines, "]+[[\"");
								else
									StringDynamic_append(sqs_lines, "]+[\"");
							}
	
							if (arg_output_offset)
								StringDynamic_appendf(sqs_offsets, "\"%d\"]+[", arg_offset_start + pos);
						} break;
						
						case OUTPUT_SQS_CLUSTER : {
							StringDynamic_append(sqs_lines, "]+[[");
							
							if (arg_output_text)
								StringDynamic_append(sqs_lines, "\"");
						} break;
					}
				}
			}

			if ((c!='\r' && (arg_output==OUTPUT_SQS_PARALLEL || arg_output==OUTPUT_SQS_CLUSTER))  ||  (arg_output!=OUTPUT_SQS_PARALLEL && arg_output!=OUTPUT_SQS_CLUSTER)) {
				// Save current character
				if (line_num>=arg_line_start  &&  (arg_limit_line_length==-1 || (arg_limit_line_length!=-1 && pos-line_start<arg_limit_line_length) || arg_split_lines)) {
					switch (arg_output) {
						case OUTPUT_EXECUTE      : QWritel(&cc, 1); break;
						case OUTPUT_CLIP         : StringDynamic_appendl(buf_for_clip, &cc, 1); break;
						case OUTPUT_SQS_PARALLEL : {
							if (arg_output_text) {
								switch(c) {
									case '\0': break;
									case '"' : StringDynamic_appendl(sqs_lines, "\"\"", 2); break;
									default  : StringDynamic_appendl(sqs_lines, &cc, 1);
								}
							}
								
							line_split++;
							
							if (arg_output_text && arg_split_lines) {
								if (line_split == arg_limit_line_length) {
									StringDynamic_appendl(sqs_lines, "\"]+[\"", 5);
									line_split = 0;
								}
							}
						} break;
						case OUTPUT_SQS_CLUSTER : {
							if (arg_output_text) {
								switch(c) {
									case '\0' : break;
									case '"'  : StringDynamic_appendl(sqs_lines, "\"\"", 2); break;
									default   : StringDynamic_appendl(sqs_lines, &cc, 1);
								}
							}
						} break;
					}						
				}
			}
			
			line_end = pos;
		}
	} while (c != EOF);
	

	// Output saved data
	if (arg_output != OUTPUT_EXECUTE) {
		if (ferror(file))
			QWrite_err(FWERROR_ERRNO, 2, errno, argument[arg_file].text);
		else
			if (arg_line_start>=0 && line_num<arg_line_start)
				QWrite_err(FWERROR_FILE_NOLINE, 2, arg_line_start, argument[arg_file].text);
			else
				if (arg_output == OUTPUT_CLIP) {
					String buf = {buf_for_clip.text, buf_for_clip.length};

					if (CopyToClip(buf, arg_clip_append))
						QWrite_err(FWERROR_NONE, 0);
				} else
					QWrite_err(FWERROR_NONE, 0);
		
		if (arg_output == OUTPUT_SQS_PARALLEL)
			QWritef("[[%s],[%s],[%s]]", sqs_lines.text, sqs_offsets.text, sqs_lengths.text);
		else
			QWritef("[%s]", sqs_lines.text);
			
		QWritef(",%d,%s,%s,\"%d\"]", line_count, getBool(line_empty), getBool(c==EOF), ftell(file));
	}
	
	fclose(file);
	StringDynamic_end(sqs_lines);
	StringDynamic_end(sqs_offsets);
	StringDynamic_end(sqs_lengths);
	StringDynamic_end(buf_for_clip);
	StringDynamic_end(buf_filename);
	

	// If user wants to delete the arg_filename after reading it
	if (arg_delete_file) {
		if (path_type == PATH_DOWNLOAD_DIR)
			remove(argument[arg_file].text);
		else
			trashFile(argument[arg_file], -1);
	}

	global.option_error_output &= ~OPTION_ERROR_ARRAY_SUPPRESS;
}
break;









case C_IGSE_NEW:
{ // IGSE New File

	global.option_error_output = OPTION_ERROR_ARRAY_CLOSE;
	
	enum IGSE_NEWFILE_MODES {
		IGSE_NEWFILE_CREATE,
		IGSE_NEWFILE_DELETE,
		IGSE_NEWFILE_RECREATE,
		IGSE_NEWFILE_CHECK
	};

	size_t arg_file    = empty_char_index;
	int arg_edit_mode  = IGSE_NEWFILE_CREATE;
	bool arg_unique    = false;
	bool arg_subdirs   = false;
	bool arg_directory = false;

	for (size_t i=2; i<argument_num; i+=2) {
		switch (argument_hash[i]) {
			case NAMED_ARG_FILE :
				arg_file = i + 1;
				break;

			case NAMED_ARG_MODE : {
				if (strcmpi(argument[i+1].text,"create") == 0)
					arg_edit_mode = IGSE_NEWFILE_CREATE;
				else 
					if (strcmpi(argument[i+1].text,"delete") == 0)
						arg_edit_mode = IGSE_NEWFILE_DELETE;
					else
						if (strcmpi(argument[i+1].text,"recreate") == 0)
							arg_edit_mode = IGSE_NEWFILE_RECREATE;
						else
							if (strcmpi(argument[i+1].text,"check") == 0)
								arg_edit_mode = IGSE_NEWFILE_CHECK;
			} break;

			case NAMED_ARG_UNIQUE :
				arg_unique = String_bool(argument[i+1]);
				break;

			case NAMED_ARG_SUBDIRS :
				arg_subdirs = String_bool(argument[i+1]);
				break;

			case NAMED_ARG_DIRECTORY :
				arg_directory = String_bool(argument[i+1]);
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

	int check_mode = arg_edit_mode==IGSE_NEWFILE_CHECK ? OPTION_ALLOW_GAME_ROOT_DIR : OPTION_RESTRICT_TO_MISSION_DIR;
	int path_type  = VerifyPath(argument[arg_file], buf_filename, check_mode);
	
	if (path_type == PATH_ILLEGAL)
		break;


	// Choose what actions to take based on arguments
	bool CheckIt  = arg_edit_mode==IGSE_NEWFILE_CHECK   ||  arg_edit_mode==IGSE_NEWFILE_RECREATE  ||  arg_unique  ||  arg_directory;
	bool DeleteIt = arg_edit_mode==IGSE_NEWFILE_DELETE  ||  arg_edit_mode==IGSE_NEWFILE_RECREATE;
	bool CreateIt = arg_edit_mode==IGSE_NEWFILE_CREATE  ||  arg_edit_mode==IGSE_NEWFILE_RECREATE;
	int  ItExists = 0;


	// Create sub-directories specified in the path string
	if (CreateIt && arg_subdirs) {
		// Replace slashes with backslashes
		for (int i=0; argument[arg_file].text[i]!='\0'; i++)
			if (argument[arg_file].text[i] == '/') 
				argument[arg_file].text[i] = '\\';

		char *slash = NULL;
		char *path  = argument[arg_file].text;

		// For each arg_directory along the way
		while ((slash = strchr(path,'\\'))) {
			// Separate name from the rest of the string
			int pos   = slash - path;
			char prev = path[pos];
			path[pos] = '\0';

			CreateDirectory(argument[arg_file].text, NULL);

			// Move on to the next
			path[pos] = prev;
			path     += pos+1;
		}
	}


	// See if the file is already there
	if (CheckIt) {		
		WIN32_FILE_ATTRIBUTE_DATA fad;

		if (GetFileAttributesEx(argument[arg_file].text, GetFileExInfoStandard, &fad)) {
			ItExists = (fad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? 2 : 1;

			if (arg_edit_mode == IGSE_NEWFILE_CHECK) {
				if (!arg_directory) {
					QWrite_err(FWERROR_NONE, 0);
				} else
					if (ItExists != 2)
						QWrite_err(FWERROR_FILE_NOTDIR, 1, argument[arg_file].text);
			}
		} else
			if (arg_edit_mode == IGSE_NEWFILE_CHECK)
				QWrite_err(FWERROR_WINAPI, 2, GetLastError(), argument[arg_file].text);
	}


	// Trash or delete
	if (DeleteIt) {
		// if the file doesn't exist then ignore error so that we can't delete it
		int ErrorBehaviour = !ItExists ? 1 : 0;

		// Default - trash it
		if (path_type != PATH_DOWNLOAD_DIR) {
			if (trashFile(argument[arg_file], ErrorBehaviour))
				QWrite_err(FWERROR_NONE, 0);
			else
				CreateIt = false;
		} else {		
			// ..\fwatch\tmp - delete it
			int result = DeleteWrapper(argument[arg_file].text);

			if (result == 0)
				QWrite_err(FWERROR_NONE, 0);
			else
				if (arg_edit_mode != IGSE_NEWFILE_RECREATE) {
					CreateIt = false;
					QWrite_err(FWERROR_WINAPI, 2, GetLastError(), argument[arg_file].text);
				}
		}
	}


	// Create new
	if (CreateIt) {
		// If we want error when it already exists
		if ((arg_unique && ItExists)  ||  (arg_directory && ItExists==1)) {
			if (arg_directory  &&  ItExists==2)
				QWrite_err(FWERROR_FILE_DIREXISTS, 1, argument[arg_file].text);
			else
				QWrite_err(FWERROR_FILE_EXISTS, 1, argument[arg_file].text);
		} else
			// arg_filename
			if (!arg_directory) {
				FILE *file = fopen(argument[arg_file].text, "a");

				if (file) {
					QWrite_err(FWERROR_NONE, 0);
					fclose(file);
				} else
					QWrite_err(FWERROR_ERRNO, 2, errno, argument[arg_file].text);
			}
			// folder
			else
				if (CreateDirectory(argument[arg_file].text, NULL))
					QWrite_err(FWERROR_NONE, 0);
				else 
					// ignore error that dir already exists
					if (GetLastError() == ERROR_ALREADY_EXISTS)
						QWrite_err(FWERROR_NONE, 0);
					else
						QWrite_err(FWERROR_WINAPI, 1, GetLastError());
	}

	StringDynamic_end(buf_filename);
}
break;









case C_IGSE_RENAME:
case C_IGSE_COPY:
{ // IGSE Rename/Copy

	global.option_error_output = OPTION_ERROR_ARRAY_CLOSE;

	size_t arg_source      = empty_char_index;
	size_t arg_destination = empty_char_index;
	bool arg_overwrite     = false;

	for (size_t i=2; i<argument_num; i+=2) {
		switch (argument_hash[i]) {
			case NAMED_ARG_SOURCE :
				arg_source = i + 1;
				break;

			case NAMED_ARG_DESTINATION :
				arg_destination = i + 1;
				break;

			case NAMED_ARG_OVERWRITE :
				arg_overwrite = String_bool(argument[i+1]);
				break;
		}
	}


	// Files not specified
	if (argument[arg_source].length == 0) {
		QWrite_err(FWERROR_PARAM_EMPTY, 1, "arg_source");
		break;
	}

	if (argument[arg_destination].length == 0) {
		QWrite_err(FWERROR_PARAM_EMPTY, 1, "arg_destination");
		break;
	}


	// Verify and update path to the files
	StringDynamic buf_source;
	StringDynamic buf_destination;
	StringDynamic_init(buf_source);
	StringDynamic_init(buf_destination);

	int path_type_source      = VerifyPath(argument[arg_source]     , buf_source     , argument_hash[0]==C_IGSE_RENAME ? OPTION_RESTRICT_TO_MISSION_DIR : OPTION_ALLOW_GAME_ROOT_DIR);
	int path_type_destination = VerifyPath(argument[arg_destination], buf_destination, OPTION_RESTRICT_TO_MISSION_DIR);

	if (!path_type_source || !path_type_destination)
		break;

	// Rename
	if (argument_hash[0] == C_IGSE_RENAME) {
		if (path_type_source!=PATH_DOWNLOAD_DIR  &&  path_type_destination==PATH_DOWNLOAD_DIR)	// Not allowed to move files to the download directory
			QWrite_err(FWERROR_FILE_MOVETOTMP, 2, argument[arg_source].text, argument[arg_destination].text);
		else {
			if (rename(argument[arg_source].text, argument[arg_destination].text) == 0)
				QWrite_err(FWERROR_NONE, 0);
			else
				QWrite_err(FWERROR_ERRNO, 3, errno, argument[arg_source].text, argument[arg_destination].text);
		}		
	// Copy
	} else {
		if (arg_overwrite  &&  path_type_destination!=PATH_DOWNLOAD_DIR)	// Overwrite means trashing before copying
			trashFile(argument[arg_destination], OPTION_NONE);

		if (CopyFile((LPCTSTR)argument[arg_source].text, (LPCTSTR)argument[arg_destination].text, path_type_destination!=PATH_DOWNLOAD_DIR))
			QWrite_err(FWERROR_NONE, 0);
		else
			QWrite_err(FWERROR_WINAPI, 3, GetLastError(), argument[arg_source].text, argument[arg_destination].text);
	}

	StringDynamic_end(buf_source);
	StringDynamic_end(buf_destination);
}
break;









case C_IGSE_FIND:
{ // IGSE Find

	int arg_line_start         = 0;
	int arg_line_end           = 0;
	int arg_offset             = 0;
	int arg_offset_column      = 0;
	int arg_column_num         = 0;
	int arg_options            = OPTION_NONE;
	int arg_result_limit       = -1;
	size_t arg_file            = empty_char_index;
	size_t arg_text            = empty_char_index;
	size_t arg_replace         = empty_char_index;
	bool arg_column            = false;
	bool arg_ignore_lead_space = false;

	for (size_t i=2; i<argument_num; i+=2) {
		switch (argument_hash[i]) {
			case NAMED_ARG_FILE : 
				arg_file = i + 1;
				break;

			case NAMED_ARG_START :
				arg_line_start = atoi(argument[i+1].text);
				break;

			case NAMED_ARG_END :
				arg_line_end = atoi(argument[i+1].text);
				break;

			case NAMED_ARG_OFFSET : {
				int index   = 0;
				String item = {NULL, 0};
				size_t pos  = 0;
				while ((item = String_tokenize(argument[i+1], ",", pos, OPTION_TRIM_SQUARE_BRACKETS)).length > 0) {
					switch (index) {
						case 0 : arg_offset=atoi(item.text); break;
						case 1 : arg_line_start=atoi(item.text); break;
						case 2 : arg_offset_column=atoi(item.text); break;
					}
					index++;
				}	
			} break;

			case NAMED_ARG_LIMIT :
				arg_result_limit = atoi(argument[i+1].text);
				break;
				
			case NAMED_ARG_TEXT :
				arg_text = i + 1;
				break;

			case NAMED_ARG_REPLACE :
				arg_replace = i + 1;
				break;

			case NAMED_ARG_COLUMN :
				arg_column     = true;
				arg_column_num = atoi(argument[i+1].text);
				break;

			case NAMED_ARG_IGNORELEADSPACE :
				arg_ignore_lead_space = String_bool(argument[i+1]);
				break;

			case NAMED_ARG_CASESENSITIVE :
				String_bool(argument[i+1]) ? arg_options|=OPTION_CASESENSITIVE : arg_options&=~OPTION_CASESENSITIVE;
				break;

			case NAMED_ARG_MATCHWORD :
				String_bool(argument[i+1]) ? arg_options|=OPTION_MATCHWORD : arg_options&=~OPTION_MATCHWORD;
				break;
		}
	}

	// Number validation
	if (arg_line_start>arg_line_end  &&  arg_line_end>0) {
		QWrite_err(FWERROR_PARAM_RANGE, 4, "Start", "End", arg_line_start, arg_line_end);
		QWrite("0,[],[],[\"0\",0,0]]");
		break;
	}
	
	// File not specified
	if (argument[arg_file].length == 0) {
		QWrite_err(FWERROR_PARAM_EMPTY, 1, "arg_file");
		QWrite("0,[],[],[\"0\",0,0]]");
		break;
	}
	
	// Search target not specified
	if (argument[arg_text].length == 0) {
		QWrite_err(FWERROR_PARAM_EMPTY, 1, "arg_text");
		QWrite("0,[],[],[\"0\",0,0]]");
		break;
	}

	// Verify and update path to the file
	StringDynamic buf_filename;
	StringDynamic_init(buf_filename);
	
	if (!VerifyPath(argument[arg_file], buf_filename, arg_replace!=empty_char_index ? OPTION_RESTRICT_TO_MISSION_DIR : OPTION_ALLOW_GAME_ROOT_DIR)) {
		QWrite("0,[],[],[\"0\",0,0]]");
		StringDynamic_end(buf_filename);
		break;	
	}

	// Open file
	FILE *file = fopen(argument[arg_file].text, "rb");
	if (!file) {
		QWrite_err(FWERROR_ERRNO, 2, errno, argument[arg_file].text);
		QWrite("0,[],[],[\"0\",0,0]]");
		StringDynamic_end(buf_filename);
		break;
	}

	// Find file size
	if (fseek(file, 0, SEEK_END) != 0) {
		QWrite_err(FWERROR_ERRNO, 2, errno, argument[arg_file].text);
		QWrite("0,[],[],[\"0\",0,0]]");
		StringDynamic_end(buf_filename);
		fclose(file);
		break;
	}

	int file_size = ftell(file);
	if (file_size == -1) {
		QWrite_err(FWERROR_ERRNO, 2, errno, argument[arg_file].text);
		QWrite("0,[],[],[\"0\",0,0]]");
		StringDynamic_end(buf_filename);
		fclose(file);
		break;
	}
	
	// If requested no results then quit
	if (arg_result_limit == 0) {
		QWrite_err(FWERROR_NONE, 0);
		QWrite("0,[],[],[\"0\",0,0]]");
		StringDynamic_end(buf_filename);
		fclose(file);
		break;
	}


	int pos                   = 0;
	size_t matched_characters = 0;
	int current_line          = 1;
	int current_column        = 0;
	int match_column          = 0;
	int line_last_added       = 0;
	int match_count           = 0;
	int leading_space         = 0;
	bool new_line             = true;
	int c                     = -1;
	char cc                   = '\0';
	char cc_previous          = '\0';
	char left_neighbour       = '\0';
	char right_neighbour      = '\0';
	
	if (arg_offset>0 && arg_replace==empty_char_index) {
		pos            = arg_offset;
		current_line   = arg_line_start;
		current_column = arg_offset_column;
		fseek(file, arg_offset, SEEK_SET);
	} else
		fseek(file, 0, SEEK_SET);

	StringDynamic file_contents_replaced;
	StringDynamic output_line_numbers;
	StringDynamic output_column_numbers;
	StringDynamic_init(file_contents_replaced);
	StringDynamic_init(output_line_numbers);
	StringDynamic_init(output_column_numbers);
	
	// Allocate buffer for a new file if doing a replacement
	if (arg_replace != empty_char_index) {
		int new_file_size          = file_size + 1;
		int new_file_size_replaced = new_file_size + (argument[arg_replace].length>argument[arg_text].length ? new_file_size : 0);
		
		int result = StringDynamic_allocate(file_contents_replaced, new_file_size_replaced);
		if (result != 0) {
			QWrite_err(FWERROR_MALLOC, 2, "file_contents_replaced", file_contents_replaced);
			QWrite("0,[],[],[\"0\",0,0]]");
			StringDynamic_end(buf_filename);
			StringDynamic_end(file_contents_replaced);
			break;
		}
	}
	

	// Go through the file
	// unless reached line range end and it's not replacement mode
	while (
		(c=fgetc(file)) != EOF  &&  
		((current_line<=arg_line_end && arg_line_end!=0 && arg_replace==empty_char_index) || arg_line_end==0 || arg_replace!=empty_char_index)
	) {
		if (ferror(file))
			break;
			
		cc = (char)c;
			
		if (arg_replace != empty_char_index)
			StringDynamic_appendl(file_contents_replaced, &cc, 1);
		
		// Count the amount of leading spaces
		if (isspace(cc)) {
			if (new_line && cc!='\0' &&  cc!='\r' &&  cc!='\n')
				leading_space++;
		} else
			new_line = false;

		// If current line number is in range
		if (current_line>=arg_line_start  &&  ((arg_line_end>0 && current_line<=arg_line_end) || (arg_line_end==0 && current_line>=arg_line_start))) {
			// If character matches
			if (
				arg_options & OPTION_CASESENSITIVE
				? cc == argument[arg_text].text[matched_characters]
				: (cc | 32) == (argument[arg_text].text[matched_characters] | 32)
			) {
				matched_characters++;
				
				// Remember where the phrase started
				if (matched_characters == 1) {
					left_neighbour = cc_previous;
					match_column   = current_column;
				}

				// If matched the entire phrase
				if (matched_characters == argument[arg_text].length) {					
					// Get next character if necessary
					if (arg_options & OPTION_MATCHWORD) {
						right_neighbour = '\0';
						
						if (pos < file_size-1) {
							int temp        = fgetc(file);
							right_neighbour = (char)temp;
							fseek(file, -1, SEEK_CUR);
						}
					}
					
					if (
						// If matching column
						(
							!arg_column  
							||  
							(arg_column && arg_column_num == match_column-(arg_ignore_lead_space ? leading_space : 0))
						) 
						&&
						// If matching the whole word then occurrence musn't be surrounded by graphic characters
						(
							~arg_options & OPTION_MATCHWORD 
							|| 
							(arg_options & OPTION_MATCHWORD && (!isalnum(left_neighbour) && left_neighbour!='_') && (!isalnum(right_neighbour) && right_neighbour!='_'))
						)
					) {
						if (line_last_added != current_line) {
							StringDynamic_appendf(output_line_numbers, "%u]+[", current_line);
							StringDynamic_append(output_column_numbers, "]+[[");
							line_last_added = current_line;
						}
						
						StringDynamic_appendf(output_column_numbers, "%u]+[", match_column);
						match_count++;
						
						if (arg_replace != empty_char_index) {
							file_contents_replaced.length -= matched_characters;
							StringDynamic_appends(file_contents_replaced, argument[arg_replace]);
						}
					}
					
					matched_characters = 0;
				}
			} else
				matched_characters = 0;
		}
		
		if (cc!='\n' && cc!='\r')
			current_column++;
		
		pos++;
		
		// Advance counters on new line
		if (cc == '\n') {
			if (line_last_added == current_line)
				StringDynamic_append(output_column_numbers, "]");
			
			current_line++;
			current_column = 0;
			leading_space  = 0;
			new_line       = true;
		}
		
		// End search if reached result limit
		if (arg_result_limit>=0  &&  match_count>=arg_result_limit)
			break;
		
		cc_previous = cc;
	}
	
	if (line_last_added == current_line)
		StringDynamic_append(output_column_numbers, "]");
		
	if (ferror(file))
		QWrite_err(FWERROR_ERRNO, 2, errno, argument[arg_file].text);
	else {
		// Rewrite file
		if (arg_replace!=empty_char_index && match_count>0) {
			if (freopen(argument[arg_file].text, "wb", file)) {
				size_t bytes_written = fwrite(file_contents_replaced.text, 1, file_contents_replaced.length, file);
				
				if (bytes_written != file_contents_replaced.length)
					QWrite_err(FWERROR_ERRNO, 2, errno, argument[arg_file].text);
				else
					QWrite_err(FWERROR_NONE, 0);

				fclose(file);
			} else
				QWrite_err(FWERROR_ERRNO, 2, errno, argument[arg_file].text);
		} else
			QWrite_err(FWERROR_NONE, 0);
	}

	QWritef("%u,[%s],[%s],[\"%u\",%u,%u]]", match_count, output_line_numbers.text, output_column_numbers.text, pos, current_line, current_column);
	
	StringDynamic_end(buf_filename);
	StringDynamic_end(file_contents_replaced);
	StringDynamic_end(output_line_numbers);
	StringDynamic_end(output_column_numbers);
	fclose(file);
}
break;









case C_IGSE_DB:
{ // IGSE Database

	size_t arg_file            = empty_char_index;
	bool arg_writing_mode      = false;
	bool arg_verify            = false;
	bool arg_unique            = false;
	size_t arg_new_keys        = 0;
	size_t arg_arguments_size  = 0;

	// Count argument length in order to determine required buffer size
	for (size_t i=2; i<argument_num; i+=2) {
		switch (argument_hash[i]) {
			case NAMED_ARG_FILE :
				arg_file = i + 1;
				break;

			case NAMED_ARG_KEY :
				arg_new_keys++;	
				arg_arguments_size += argument[i+1].length + 1 + sizeof(int)*3;
				break;

			case NAMED_ARG_WRITE :
				arg_arguments_size += argument[i+1].length + 1;
				arg_writing_mode    = true;
				break;

			case NAMED_ARG_APPEND :
				arg_arguments_size += argument[i+1].length + 1;
				arg_writing_mode    = true;
				break;

			case NAMED_ARG_RENAME :
				arg_arguments_size += argument[i+1].length + 1;
				arg_writing_mode    = true;
				break;

			case NAMED_ARG_REMOVE :
				arg_writing_mode = true;
				break;

			case NAMED_ARG_LIST :
				arg_verify = true;
				break;

			case NAMED_ARG_VERIFY :
				arg_verify = String_bool(argument[i+1]);
				break;

			case NAMED_ARG_UNIQUE : 
				arg_unique = String_bool(argument[i+1]);
				break;
		}
	}		

	
	// File not specified
	if (argument[arg_file].length == 0) {
		QWrite_err(FWERROR_PARAM_EMPTY, 1, "arg_file");
		QWrite("[],[]]");
		break;
	}

	// Verify and update path to the file
	StringDynamic buf_filename;
	StringDynamic_init(buf_filename);
	
	if (!VerifyPath(argument[arg_file], buf_filename, OPTION_RESTRICT_TO_MISSION_DIR)) {
		QWrite("[],[]]");
		break;
	}


	// Read file signature
	FILE *file                     = fopen(argument[arg_file].text, "rb");
	size_t buffer_size             = 0;
	const __int64 igsedb_signature = 0x00626465736769;
	const int igsedb_version       = 1;
	
	struct igsedb_header {
		__int64 signature;
		size_t version;
		size_t number_of_keys;
	} *header;
	
	struct igsedb_pointer {	// indicates current and the next item in the array of pointers
		size_t start;
		size_t end;
	} key_pointer, value_pointer;

	if (file) {
		igsedb_header header_existing = {0,0,0};
		size_t header_read            = fread(&header_existing, sizeof(igsedb_header), 1, file);

		if (!header_read) {
			QWrite_err(FWERROR_ERRNO, 2, errno, argument[arg_file].text);
			QWrite("[],[]]");
			StringDynamic_end(buf_filename);
			fclose(file);
			break;
		}

		if (header_existing.signature != igsedb_signature) {
			QWrite_err(FWERROR_DB_SIGNATURE, 3, header_existing.signature, igsedb_signature, argument[arg_file].text);
			QWrite("[],[]]");
			StringDynamic_end(buf_filename);
			fclose(file);
			break;
		}

		if (header_existing.version > igsedb_version) {
			QWrite_err(FWERROR_DB_VERSION, 3, header_existing.version, igsedb_version, argument[arg_file].text);
			QWrite("[],[]]");
			StringDynamic_end(buf_filename);
			fclose(file);
			break;
		}

		// Find file size
		if (fseek(file, 0, SEEK_END) != 0) {
			QWrite_err(FWERROR_ERRNO, 2, errno, argument[arg_file].text);
			QWrite("[],[]]");
			StringDynamic_end(buf_filename);
			fclose(file);
			break;
		};

		buffer_size = ftell(file);
		if (buffer_size == 0xFFFFFFFF) {
			QWrite_err(FWERROR_ERRNO, 2, errno, argument[arg_file].text);
			QWrite("[],[]]");
			StringDynamic_end(buf_filename);
			fclose(file);
			break;
		};
		
		fseek(file, 0, SEEK_SET);
		
		// Check for minimal file size (header size + size of all uint arrays + all terminating zeros)
		size_t minimal_file_size = sizeof(igsedb_header) + header_existing.number_of_keys*sizeof(size_t)*3 + header_existing.number_of_keys*2;
		if (buffer_size < minimal_file_size) {
			QWrite_err(FWERROR_DB_SMALL, 3, buffer_size, minimal_file_size, argument[arg_file].text);
			QWrite("[],[]]");
			StringDynamic_end(buf_filename);
			fclose(file);
			break;
		}
	} else {
		if (!arg_writing_mode  ||  (arg_writing_mode && errno!=2)) {
			QWrite_err(FWERROR_ERRNO, 2, errno, argument[arg_file].text);
			QWrite("[],[]]");
			StringDynamic_end(buf_filename);
			break;
		}

		arg_arguments_size += sizeof(igsedb_header);
	}


	// Allocate memory for the data
	StringDynamic file_contents;
	StringDynamic_init(file_contents);
	size_t buffer_size_max = buffer_size + arg_arguments_size + 1;

	if (StringDynamic_allocate(file_contents, buffer_size_max) != 0) {
		QWrite_err(FWERROR_MALLOC, 2, "file_contents", buffer_size_max);
		QWrite("[],[]]");
		StringDynamic_end(buf_filename);
		break;
	}

	char *buffer = file_contents.text;

	
	// Read data from the selected file to the allocated memory
	if (file) {
		size_t bytes_read = fread(buffer, 1, buffer_size, file);
		fclose(file);

		if (bytes_read != buffer_size) {
			QWrite_err(FWERROR_ERRNO, 2, errno, argument[arg_file].text);
			QWrite("[],[]]");
			StringDynamic_end(buf_filename);
			break;
		}
	} else {
		igsedb_header temp = {igsedb_signature, igsedb_version, 0};
		memcpy(buffer, &temp, sizeof(igsedb_header));
	}
	
	header = (igsedb_header*)buffer;


	// Verify database
	bool db_error = false;
	
	if (arg_writing_mode || arg_verify) {
		unsigned int hash_previous   = 0;
		size_t pointer_previous      = 0;
		size_t minimal_pointer_value = sizeof(igsedb_header) + header->number_of_keys * sizeof(size_t) * 3;
		
		for (size_t i=0; i<header->number_of_keys; i++) {
			size_t hash_pos    = sizeof(igsedb_header) + i * sizeof(size_t);
			unsigned int *hash = (unsigned int*)(buffer + hash_pos);
			
			if (*hash <= hash_previous) {
				QWrite_err(FWERROR_DB_HASHORDER, 3, i, i+1, argument[arg_file].text);
				db_error = true;
				break;
			}
			
			hash_previous = *hash;
		}
		
		for (i=0;  i<header->number_of_keys*2 && !db_error;  i++) {
			size_t pointer_pos    = sizeof(igsedb_header) + (header->number_of_keys+i) * sizeof(size_t);
			size_t *pointer_value = (size_t*)(buffer + pointer_pos);
			
			if (i==0  &&  *pointer_value!=minimal_pointer_value) {
				QWrite_err(FWERROR_DB_PTRFIRST, 3, *pointer_value, minimal_pointer_value, argument[arg_file].text);
				db_error = true;
				break;
			}
				
			if (*pointer_value < minimal_pointer_value) {
				QWrite_err(FWERROR_DB_PTRSMALL, 4, i, header->number_of_keys*2-1, *pointer_value, minimal_pointer_value, argument[arg_file].text);
				db_error = true;
				break;
			}
			
			if (*pointer_value > buffer_size) {
				QWrite_err(FWERROR_DB_PTRBIG, 4, i, header->number_of_keys*2-1, *pointer_value, buffer_size, argument[arg_file].text);
				db_error = true;
				break;
			}
			
			if ((i>0 && buffer[*pointer_value-1]!='\0') || (i==header->number_of_keys*2-1  &&  buffer[buffer_size-1]!='\0')) {
				QWrite_err(FWERROR_DB_CONSISTENCY, 4, i+1, header->number_of_keys*2, buffer[*pointer_value-1]!='\0' ? *pointer_value-1 : buffer_size-1, argument[arg_file].text);
				db_error = true;
				break;
			}
			
			if (*pointer_value <= pointer_previous) {
				QWrite_err(FWERROR_DB_PTRORDER, 5, i+1, *pointer_value, i, pointer_previous, argument[arg_file].text);
				db_error = true;
				break;
			}

			pointer_previous = *pointer_value;
		}
	}


	// Go through the arguments and execute them
	unsigned int key_hash = 0;
	char *key_name        = empty_char;
	size_t key_length     = 0;

	BinarySearchResult key_selected;
	StringDynamic MissingKeys;
	StringDynamic List;
	StringDynamic_init(MissingKeys);
	StringDynamic_init(List);

	for (i=2;  i<argument_num && !db_error;  i+=2) {
		char *arg_value         = argument[i+1].text;
		size_t arg_value_length = argument[i+1].length + 1; //argument length with terminating zero
		
		switch(argument_hash[i]) {
			case NAMED_ARG_KEY : {
				key_name     = arg_value;
				key_length   = arg_value_length;
				key_hash     = fnv1a_hash(FNV_BASIS, argument[i+1].text, argument[i+1].length, OPTION_LOWERCASE);
				key_selected = binary_search_str(buffer+sizeof(igsedb_header), header->number_of_keys, key_hash, 0, header->number_of_keys-1);
				
				// Check for hash collision
				if (key_selected.found) {
					if (arg_unique) {
						QWrite_err(FWERROR_DB_KEYEXISTS, 2, arg_value, argument[arg_file].text);
						db_error = true;
						break;
					} else {
						size_t key_pointer_pos = sizeof(igsedb_header) + (header->number_of_keys + key_selected.index) * sizeof(size_t);
						memcpy(&key_pointer, buffer+key_pointer_pos, sizeof(igsedb_pointer));

						if (strncmpi(buffer+key_pointer.start,key_name,key_length) != 0) {
							size_t hash_pos    = sizeof(igsedb_header) + key_selected.index * sizeof(unsigned int);
							unsigned int *hash = (unsigned int*)buffer + hash_pos;
							
							QWrite_err(FWERROR_DB_COLLISION, 5, arg_value, key_hash, buffer+key_pointer_pos, *hash, argument[arg_file].text);
							db_error = true;
							break;
						}
					}
				}
			} break;
			
			case NAMED_ARG_WRITE : 
			case NAMED_ARG_APPEND : {
				if (key_selected.found) {
					// Measure difference in size between the old and new value
					size_t value_pointer_pos = sizeof(igsedb_header) + (header->number_of_keys*2 + key_selected.index) * sizeof(size_t);
					memcpy(&value_pointer, buffer+value_pointer_pos, sizeof(igsedb_pointer));
										
					if (key_selected.index == header->number_of_keys-1)
						value_pointer.end = buffer_size;
						
					// If appending then don't count terminating zero
					if (argument_hash[i] == NAMED_ARG_APPEND) {
						arg_value_length--;
						value_pointer.end--;
					}

					size_t value_old_length     = (argument_hash[i] == NAMED_ARG_WRITE ? (value_pointer.end - value_pointer.start) : 0);
					size_t buffer_shift_amount  = arg_value_length >= value_old_length ?  arg_value_length-value_old_length : value_old_length-arg_value_length;
					bool buffer_shift_direction = arg_value_length >= value_old_length;
					
					// Replace value
					if (key_selected.index < header->number_of_keys-1)
						shift_buffer_chunk(buffer, value_pointer.end, buffer_size, buffer_shift_amount, buffer_shift_direction);

					memcpy(buffer+(argument_hash[i]==NAMED_ARG_WRITE ? value_pointer.start : value_pointer.end), arg_value, arg_value_length + (argument_hash[i]==NAMED_ARG_APPEND ? 1 : 0));
					
					// Adjust pointers to values
					for (size_t j=key_selected.index+1; j<header->number_of_keys; j++) {
						size_t value_pointer_pos = sizeof(igsedb_header) + (header->number_of_keys*2 + j) * sizeof(size_t);
						size_t *value_pointer    = (size_t*)(buffer + value_pointer_pos);
						*value_pointer          += buffer_shift_amount * (buffer_shift_direction ? 1 : -1);
					}
					
					buffer_size += buffer_shift_amount * (buffer_shift_direction ? 1 : -1);
				} else {
					// Start a new database
					if (header->number_of_keys == 0) {
						header->number_of_keys   = 1;
						size_t key_pointer_new   = sizeof(igsedb_header) + sizeof(size_t)*3;
						size_t value_pointer_new = key_pointer_new + key_length;
						
						char *pos = buffer;
						memcpy(pos, header, sizeof(igsedb_header));
						
						pos += sizeof(igsedb_header);
						memcpy(pos, &key_hash, sizeof(key_hash));
						
						pos += sizeof(key_hash);
						memcpy(pos, &key_pointer_new, sizeof(key_pointer_new));
						
						pos += sizeof(key_pointer_new);
						memcpy(pos, &value_pointer_new, sizeof(value_pointer_new));
						
						pos += sizeof(value_pointer_new);
						memcpy(pos, key_name, key_length);
						
						pos += key_length;
						memcpy(pos, arg_value, arg_value_length);
						
						pos         += arg_value_length;
						buffer_size += (pos - buffer);
					} else {
						// Add a new key and value to the database
						size_t key_pointer_new     = 0;
						size_t value_pointer_new   = 0;
						size_t buffer_shift_total  = sizeof(key_hash) + sizeof(key_pointer_new) + sizeof(value_pointer_new) + key_length + arg_value_length;
						size_t buffer_shift_amount = buffer_shift_total;
		
						// If value is not being inserted at the end then shift existing values (to the right of where the new one should be) further to the right to make space
						size_t value_pointer_pos = sizeof(igsedb_header) + (header->number_of_keys*2 + key_selected.index) * sizeof(size_t);
						value_pointer.start   = buffer_size;
						
						if (key_selected.index < header->number_of_keys) {
							memcpy(&value_pointer, buffer+value_pointer_pos, sizeof(igsedb_pointer));
							shift_buffer_chunk(buffer, value_pointer.start, buffer_size, buffer_shift_amount, OPTION_RIGHT);
						}
		
						// Insert new value
						value_pointer_new = value_pointer.start + buffer_shift_amount - arg_value_length;
						memcpy(buffer+value_pointer_new, arg_value, arg_value_length);
						buffer_shift_amount -= arg_value_length;
						
						// Shift remaining values (to the left of the new value) and keys (to the right of the new key) further to the right to make space for the new value
						size_t key_pointer_pos = sizeof(igsedb_header) + (header->number_of_keys + key_selected.index) * sizeof(size_t);
						memcpy(&key_pointer, buffer+key_pointer_pos, sizeof(igsedb_pointer));
						shift_buffer_chunk(buffer, key_pointer.start, value_pointer.start-1, buffer_shift_amount, OPTION_RIGHT);
						
						// Insert new key
						key_pointer_new = key_pointer.start + buffer_shift_amount - key_length;
						memcpy(buffer+key_pointer_new, key_name, key_length);
						buffer_shift_amount -= key_length;
						
						// Shift remaining keys (to the left of the new key) and value offsets (to the right of the new value offset) further to the right to make space for the new value offset
						shift_buffer_chunk(buffer, value_pointer_pos, key_pointer.start-1, buffer_shift_amount, OPTION_RIGHT);

						// Insert new value offset
						memcpy(buffer+value_pointer_pos+buffer_shift_amount-sizeof(value_pointer_new), &value_pointer_new, sizeof(value_pointer_new));
						buffer_shift_amount -= sizeof(value_pointer_new);	
						
						// Shift remaining value offsets (to the left of the new value offset) and key offsets (to the right of the new key offset) further to the right to make space for the new key offset
						shift_buffer_chunk(buffer, key_pointer_pos, value_pointer_pos-1, buffer_shift_amount, OPTION_RIGHT);
						
						// Insert new key offset
						memcpy(buffer+key_pointer_pos+buffer_shift_amount-sizeof(key_pointer_new), &key_pointer_new, sizeof(key_pointer_new));
						buffer_shift_amount -= sizeof(key_pointer_new);
						
						// Shift remaining key offsets (to the left of the new key offset) and hashes (to the right of the new hash) further to the right to make space for the new hash
						size_t hash_pos = sizeof(igsedb_header) + key_selected.index * sizeof(size_t);
						shift_buffer_chunk(buffer, hash_pos, key_pointer_pos-1, buffer_shift_amount, OPTION_RIGHT);

						// Insert new hash
						memcpy(buffer+hash_pos+buffer_shift_amount-sizeof(key_hash), &key_hash, sizeof(key_hash));

						// Adjust number of keys
						header->number_of_keys++;
		
						// Adjust pointers to keys and values
						for(size_t i=0; i<header->number_of_keys*2; i++) {
							size_t pointer_pos = sizeof(igsedb_header) + (header->number_of_keys+i) * sizeof(size_t);
							size_t *pointer    = (size_t*)(buffer + pointer_pos);
							
							if (i < key_selected.index)
								*pointer += sizeof(key_hash) + sizeof(key_pointer_new) + sizeof(value_pointer_new);
							
							if (i > key_selected.index  &&  i<header->number_of_keys+key_selected.index)
								*pointer += sizeof(key_hash) + sizeof(key_pointer_new) + sizeof(value_pointer_new) + key_length;
								
							if (i > header->number_of_keys+key_selected.index)
								*pointer += sizeof(key_hash) + sizeof(key_pointer_new) + sizeof(value_pointer_new) + key_length + arg_value_length;
						}
		
						buffer_size += buffer_shift_total;
					}
				}
			} break;
			
			case NAMED_ARG_RENAME : {
				if (key_selected.found) {
					// Measure difference in size between the old and new key
					size_t key_pointer_pos = sizeof(igsedb_header) + (header->number_of_keys + key_selected.index) * sizeof(size_t);
					memcpy(&key_pointer, buffer+key_pointer_pos, sizeof(igsedb_pointer));
					
					size_t value_old_length     = key_pointer.end - key_pointer.start;
					size_t buffer_shift_amount  = arg_value_length >= value_old_length ? arg_value_length-value_old_length : value_old_length-arg_value_length;
					bool buffer_shift_direction = arg_value_length >= value_old_length;
					
					// Replace key
					shift_buffer_chunk(buffer, key_pointer.end, buffer_size, buffer_shift_amount, buffer_shift_direction);
					memcpy(buffer+key_pointer.start, arg_value, arg_value_length);
					
					// Replace hash
					size_t hash_pos = sizeof(igsedb_header) + key_selected.index * sizeof(unsigned int);
					key_hash        = fnv1a_hash(FNV_BASIS, argument[i+1].text, argument[i+1].length, OPTION_LOWERCASE);
					memcpy(buffer+hash_pos, &key_hash, sizeof(unsigned int));
					
					// Adjust pointers to keys and values
					for (size_t j=0; j<header->number_of_keys*2; j++) {
						size_t pointer_pos = sizeof(igsedb_header) + (header->number_of_keys + j) * sizeof(size_t);
						size_t *pointer    = (size_t*)(buffer + pointer_pos);

						if (j > key_selected.index)
							*pointer += buffer_shift_amount * (buffer_shift_direction ? 1 : -1);
					}
					
					buffer_size += buffer_shift_amount * (buffer_shift_direction ? 1 : -1);
				} else {
					StringDynamic_append(MissingKeys, "]+[\"");
					StringDynamic_appendq(MissingKeys, key_name);
					StringDynamic_append(MissingKeys, "\"");
				}
			} break;
			
			case NAMED_ARG_REMOVE : {
				BinarySearchResult key_to_remove = binary_search_str(buffer+sizeof(igsedb_header), header->number_of_keys, fnv1a_hash(FNV_BASIS, argument[i+1].text, argument[i+1].length, OPTION_LOWERCASE), 0, header->number_of_keys-1);
					
				if (key_to_remove.found) {
					// Measure size of the value under given key
					size_t key_pointer_pos = sizeof(igsedb_header) + (header->number_of_keys + key_to_remove.index) * sizeof(size_t);
					memcpy(&key_pointer, buffer+key_pointer_pos, sizeof(igsedb_pointer));
					
					size_t value_pointer_pos = key_pointer_pos + header->number_of_keys * sizeof(size_t);
					memcpy(&value_pointer, buffer+value_pointer_pos, sizeof(igsedb_pointer));
					
					if (key_to_remove.index == header->number_of_keys-1)
						value_pointer.end = buffer_size;
						
					size_t value_length = value_pointer.end - value_pointer.start;
					
					// Shift hashes in order to remove hash for the selected key
					size_t shift_amount = sizeof(key_hash);
					size_t hash_pos     = sizeof(igsedb_header) + (key_to_remove.index+1) * sizeof(size_t);
					shift_buffer_chunk(buffer, hash_pos, buffer_size, shift_amount, OPTION_LEFT);
					buffer_size -= sizeof(key_hash);
					
					// Shift key pointers array in order to remove pointer for the selected key
					key_pointer_pos = sizeof(igsedb_header) + (header->number_of_keys-1 + key_to_remove.index+1) * sizeof(size_t);
					shift_buffer_chunk(buffer, key_pointer_pos, buffer_size, shift_amount, OPTION_LEFT);
					buffer_size -= sizeof(key_pointer_pos);
							
					// Shift value pointers array in order to remove pointer for the selected value
					value_pointer_pos = sizeof(igsedb_header) + ((header->number_of_keys-1)*2 + key_to_remove.index+1) * sizeof(size_t);
					shift_buffer_chunk(buffer, value_pointer_pos, buffer_size, shift_amount, OPTION_LEFT);
					buffer_size -= sizeof(value_pointer_pos);
	
					// Shift keys in order to remove selected key
					key_pointer.end -= sizeof(key_hash) + sizeof(key_pointer_pos) + sizeof(value_pointer_pos);
					shift_buffer_chunk(buffer, key_pointer.end, buffer_size, arg_value_length, OPTION_LEFT);
					buffer_size -= arg_value_length;
					
					// Shift values
					value_pointer.end -= sizeof(key_hash) + sizeof(key_pointer_pos) + sizeof(value_pointer_pos) + arg_value_length;
					shift_buffer_chunk(buffer, value_pointer.end, buffer_size, value_length, OPTION_LEFT);
					buffer_size -= value_length;
					
					// Adjust number of keys
					header->number_of_keys--;
	
					// Adjust pointers to keys and values
					for (size_t j=0; j<header->number_of_keys*2; j++) {
						size_t pointer_pos = sizeof(igsedb_header) + (header->number_of_keys+j) * sizeof(size_t);
						size_t *pointer    = (size_t*)(buffer + pointer_pos);
						
						if (j < key_to_remove.index)
							*pointer -= sizeof(key_hash) + sizeof(key_pointer_pos) + sizeof(value_pointer_pos);
						
						if (j >= key_to_remove.index  &&  j<header->number_of_keys+key_to_remove.index)
							*pointer -= sizeof(key_hash) + sizeof(key_pointer_pos) + sizeof(value_pointer_pos) + arg_value_length;
							
						if (j >= header->number_of_keys+key_to_remove.index)
							*pointer -= sizeof(key_hash) + sizeof(key_pointer_pos) + sizeof(value_pointer_pos) + arg_value_length + value_length;
					}
				} else {
					StringDynamic_append(MissingKeys, "]+[\"");
					StringDynamic_appendq(MissingKeys, argument[i+1].text);
					StringDynamic_append(MissingKeys, "\"");
				}
			} break;
			
			case NAMED_ARG_READ : {
				BinarySearchResult key_to_read = binary_search_str(buffer+sizeof(igsedb_header), header->number_of_keys, fnv1a_hash(FNV_BASIS, argument[i+1].text, argument[i+1].length, OPTION_LOWERCASE), 0, header->number_of_keys-1);
	
				if (key_to_read.found) {
					size_t value_pointer_pos = sizeof(igsedb_header) + (header->number_of_keys*2 + key_to_read.index) * sizeof(size_t);
					memcpy(&value_pointer, buffer+value_pointer_pos, sizeof(igsedb_pointer));
					
					if (key_to_read.index == header->number_of_keys-1)
						value_pointer.end = buffer_size;
						
					// Check if pointer isn't too small
					size_t minimal_pointer_value = sizeof(igsedb_header) + header->number_of_keys * sizeof(size_t) * 3;
					if (value_pointer.start < minimal_pointer_value) {
						QWrite_err(FWERROR_DB_PTRSMALL, 4, (key_to_read.index+1)*2, header->number_of_keys*2, value_pointer.start, minimal_pointer_value, argument[arg_file].text);
						db_error = true;
						break;
					}
					
					// Check if pointer isn't too big
					if (value_pointer.end > buffer_size) {
						QWrite_err(FWERROR_DB_PTRBIG, 4, (key_to_read.index+1)*2, header->number_of_keys*2, value_pointer.end, buffer_size, argument[arg_file].text);
						db_error = true;
						break;
					}

					// Value must be surrounded by terminating zeros                     
					if ((key_to_read.index>0 && buffer[value_pointer.start-1]!='\0')  ||  buffer[value_pointer.end-1]!='\0') {
						QWrite_err(FWERROR_DB_CONSISTENCY, 4, (key_to_read.index+1)*2, header->number_of_keys*2, buffer[value_pointer.start-1]!='\0' ? value_pointer.start-1 : value_pointer.end-1, argument[arg_file].text);
						db_error = true;
						break;
					}
						
					QWritel(buffer+value_pointer.start, value_pointer.end-1-value_pointer.start);
					QWrite(";");
				} else {
					StringDynamic_append(MissingKeys, "]+[\"");
					StringDynamic_appendq(MissingKeys, argument[i+1].text);
					StringDynamic_append(MissingKeys, "\"");
				}
			} break;
			
			case NAMED_ARG_LIST : {
				if (strcmpi(argument[i+1].text,"values") == 0) {
					for (size_t j=0; j<header->number_of_keys; j++) {
						size_t value_pointer_pos = sizeof(igsedb_header) + (header->number_of_keys*2 + j) * sizeof(size_t);
						memcpy(&value_pointer, buffer+value_pointer_pos, sizeof(igsedb_pointer));
						
						if (j == header->number_of_keys-1)
							value_pointer.end = buffer_size;
	
						QWritel(buffer+value_pointer.start, value_pointer.end-1-value_pointer.start);
						QWrite(";");
					}
				} else {
					if (strcmpi(argument[i+1].text,"keys") == 0) {
						for (size_t j=0; j<header->number_of_keys; j++) {
							int key_pointer_pos = sizeof(igsedb_header) + (header->number_of_keys + j) * sizeof(size_t);
							memcpy(&key_pointer, buffer+key_pointer_pos, sizeof(igsedb_pointer));
	
							StringDynamic_append(List, "]+[\"");
							for (; key_pointer.start<key_pointer.end-1; key_pointer.start++) {
								char *c = buffer+key_pointer.start;
								char cc[2] = "";
								strncpy(cc, c, 1);
								if (*c == '"')
									StringDynamic_append(List, "\"");
								else
									StringDynamic_append(List, cc);
							}
							StringDynamic_append(List, "\"");
						}
					} else 
						if (strcmpi(argument[i+1].text,"all") == 0) {
							StringDynamic_append(List, "[");
	
							for (size_t j=0; j<header->number_of_keys; j++) {
								size_t key_pointer_pos = sizeof(igsedb_header) + (header->number_of_keys + j) * sizeof(size_t);
								memcpy(&key_pointer, buffer+key_pointer_pos, sizeof(igsedb_pointer));
								
								StringDynamic_append(List, "]+[\"");
								for (; key_pointer.start<key_pointer.end-1; key_pointer.start++) {
									char *c = buffer+key_pointer.start;
									char cc[2] = "";
									strncpy(cc, c, 1);
									if (*c == '"')
										StringDynamic_append(List, "\"");
									StringDynamic_append(List, cc);
								}
								StringDynamic_append(List, "\"");
							}
	
							StringDynamic_append(List, "]]+[[");
	
							for (j=0; j<header->number_of_keys; j++) {
								size_t value_pointer_pos = sizeof(igsedb_header) + (header->number_of_keys*2 + j) * sizeof(size_t);
								memcpy(&value_pointer, buffer+value_pointer_pos, sizeof(igsedb_pointer));
								
								if (j == header->number_of_keys-1)
									value_pointer.end = buffer_size;
								
								StringDynamic_append(List, "]+[\"");
								for (; value_pointer.start<value_pointer.end-1; value_pointer.start++) {
									char *c = buffer+value_pointer.start;
									char cc[2] = "";
									strncpy(cc, c, 1);
									if (*c == '"')
										StringDynamic_append(List, "\"");
									StringDynamic_append(List, cc);
								}
								StringDynamic_append(List, "\"");
							}
	
							StringDynamic_append(List, "]");
						}
				}
			} break;
		}
	}


	// Rewrite the file if changes were made
	if (arg_writing_mode && !db_error) {
		char *file_ptr = argument[arg_file].text;
		
		for (size_t i=0;  argument[arg_file].text[i]!='\0';  i++)	// Replace slashes with backslashes
			if (argument[arg_file].text[i] == '/') 
				argument[arg_file].text[i] = '\\';

		// Create subdirectories
		char *path_item;
		while ((path_item = strchr(file_ptr,'\\'))) {
			size_t slash_pos    = path_item - file_ptr;
			char backup         = file_ptr[slash_pos];
			file_ptr[slash_pos] = '\0';

			CreateDirectory(argument[arg_file].text, NULL);
			
			file_ptr[slash_pos] = backup;
			file_ptr           += slash_pos + 1;
		}
		
		if ((file = fopen(argument[arg_file].text, "wb"))) {
			fwrite(buffer, sizeof(*buffer), buffer_size, file);
		}
	}

	if (!db_error) {
		if (file  &&  ferror(file))
			QWrite_err(FWERROR_ERRNO, 2, errno, argument[arg_file].text);
		else
			QWrite_err(FWERROR_NONE, 0);
	}

	QWritef("[%s],[%s]]", List.text, MissingKeys.text);
	StringDynamic_end(file_contents);
	StringDynamic_end(buf_filename);
	StringDynamic_end(List);
	StringDynamic_end(MissingKeys);
		
	if (file)
		fclose(file);
}
break;