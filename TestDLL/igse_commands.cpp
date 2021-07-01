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

	char *arg_filename      = empty_string;
	char *arg_txt           = empty_string;
	char *arg_escape        = empty_string;
	bool arg_clipboard      = false;
	bool arg_column         = false;
	int arg_line_start      = 0;
	int arg_edit_mode       = IGSE_WRITE_REPLACE;
	int arg_open_mode       = IGSE_WRITE_OPEN_CREATE;
	int arg_column_num      = 0;
	int arg_line_range      = 1;
	int arg_filename_length = 0;
	int arg_txt_length      = 0;

	for (size_t i=2; i<argument_num; i+=2) {
		switch (argument_hash[i]) {
			case NAMED_ARG_ESCAPE :
				arg_escape = argument[i+1];
				break;

			case NAMED_ARG_FILE : 
				arg_filename        = argument[i+1];
				arg_filename_length = argument_length[i+1];
				break;

			case NAMED_ARG_LINE :
				arg_line_start = atoi(argument[i+1]);
				break;

			case NAMED_ARG_COLUMN :
				arg_column_num = atoi(argument[i+1]);
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
					if (strcmpi(argument[i+1],modes[j]) == 0) {
						arg_edit_mode = j;
						break;
					}
			} break;

			case NAMED_ARG_TEXT :
				arg_txt        = argument[i+1];
				arg_txt_length = argument_length[i+1];
				break;

			case NAMED_ARG_CLIP :
				arg_clipboard = String2Bool(argument[i+1]);
				break;

			case NAMED_ARG_RANGE :
				arg_line_range = atoi(argument[i+1]);
				break;

			case NAMED_ARG_OPEN : {
				char modes[][32] = {
					"create",
					"recreate",
					"unique",
					"check"
				};

				for (int j=0, max=sizeof(modes)/sizeof(modes[0]);  j<max;  j++)
					if (strcmpi(argument[i+1],modes[j]) == 0) {
						arg_open_mode = j;
						break;
					}
			} break;
		}
	}

	
	// File not specified
	if (arg_filename_length == 0) {
		QWrite_err(FWERROR_PARAM_EMPTY, 1, "arg_filename");
		break;
	}
	
	// Cannot move up the first line
	if (arg_edit_mode==IGSE_WRITE_MOVEUP  &&  arg_line_start==1) {
		QWrite_err(FWERROR_PARAM_ONE, 2, "arg_line_start", arg_line_start);
		break;
	}

	// Verify and update path to the file
	String buf_filename;
	String_init(buf_filename);
	char *ptr_filename = arg_filename;
	
	if (!VerifyPath(&ptr_filename, buf_filename, OPTION_RESTRICT_TO_MISSION_DIR))
		break;
	

	// Does clipboard contains text
	HANDLE hClipboardData;

	if (arg_clipboard) {
		if (OpenClipboard(NULL)) {
			if (::IsClipboardFormatAvailable(CF_TEXT)) {
				hClipboardData = GetClipboardData(CF_TEXT);
				arg_txt        = (char*)GlobalLock(hClipboardData);
			} else {
				QWrite_err(FWERROR_CLIP_FORMAT, 0);
				String_end(buf_filename);
				CloseClipboard(); 
				break;
			}
		} else {
			QWrite_err(FWERROR_CLIP_OPEN, 1, GetLastError());
			String_end(buf_filename);
			break;
		}
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
			arg_txt  = EscSequences(arg_txt, OPTION_TAB, quantity);
			quantity = -1;
		}

		if (arg_escape[i] == 'n') {
			arg_txt  = EscSequences(arg_txt, OPTION_CRLF, quantity);
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

	FILE *file = fopen(ptr_filename, open_mode);
	if (!file) {
		QWrite_err(FWERROR_ERRNO, 2, errno, ptr_filename);
		String_end(buf_filename);
		break;
	}

	if (arg_open_mode == IGSE_WRITE_OPEN_UNIQUE) {
		QWrite_err(FWERROR_FILE_EXISTS, 1, ptr_filename);
		String_end(buf_filename);
		fclose(file);
		break;
	}


	// Find file size
	if (fseek(file, 0, SEEK_END) != 0) {
		QWrite_err(FWERROR_ERRNO, 2, errno, ptr_filename);
		String_end(buf_filename);
		fclose(file);
		break;
	}

	size_t file_size = ftell(file);
	if (file_size == 0xFFFFFFFF) {
		QWrite_err(FWERROR_ERRNO, 2, errno, ptr_filename);
		String_end(buf_filename);
		fclose(file);
		break;
	}

	// Quick append mode
	if (arg_line_start == 0) {
		if (arg_open_mode == IGSE_WRITE_OPEN_CHECK)
			freopen(ptr_filename, "ab", file);
		
		if (file_size>0  &&  arg_edit_mode != IGSE_WRITE_APPEND) 
			fprintf(file, "\r\n");

		fprintf(file, "%s", arg_txt);

		if (ferror(file))
			QWrite_err(FWERROR_ERRNO, 2, errno, ptr_filename);
		else
			QWrite_err(FWERROR_NONE, 0);

		String_end(buf_filename);
		fclose(file);
		break;
	}


	// Allocate buffer
	String file_contents;
	String_init(file_contents);
	size_t new_line_len  = arg_txt_length;
	size_t new_file_size = file_size * (arg_edit_mode==IGSE_WRITE_COPY ? 2 : 1) + new_line_len + 2 + 1;
	int result           = String_allocate(file_contents, new_file_size);

	if (result != 0) {
		QWrite_err(FWERROR_MALLOC, 2, "file_contents", new_file_size);
		String_end(buf_filename);
		break;
	}


	// Copy text to buffer
	fseek(file, 0, SEEK_SET);
	size_t bytes_read = fread(file_contents.text, 1, file_size, file);

	if (bytes_read != file_size) {
		String_end(buf_filename);
		String_end(file_contents);
		QWrite_err(FWERROR_ERRNO, 2, errno, ptr_filename);
		fclose(file);
		break;
	}

	fclose(file);



	// Set vars for parsing text
	char *buffer                 = file_contents.text;
	char *new_line               = arg_txt;
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

					shift_buffer_chunk(buffer, line_end_pos, file_size, buffer_shift_amount, buffer_shift_direction);
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
					String previous_line_backup;
					String_init(previous_line_backup);
					int previous_line_len = line_shift_source_pos - line_shift_dest_pos;
					String_append_len(previous_line_backup, line_shift_dest, previous_line_len);

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
					String_end(previous_line_backup);
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
		if ((file = fopen(ptr_filename, "wb"))) {
			size_t bytes_written = fwrite(buffer, 1, file_size, file);

			if (bytes_written != file_size)
				QWrite_err(FWERROR_ERRNO, 2, errno, ptr_filename);				
			else
				QWrite_err(FWERROR_NONE, 0);

			fclose(file);
		} else
			QWrite_err(FWERROR_ERRNO, 2, errno, ptr_filename);
	} else 		
		QWrite_err(FWERROR_FILE_NOLINE, 2, abs(line_num) < abs(arg_line_start) ? arg_line_start : line_range_end, ptr_filename);

	String_end(file_contents);
	String_end(buf_filename);
}
break;












case C_IGSE_LIST:
{ // IGSE List of files

	bool arg_system_time = false;
	int arg_only_name    = false;
	char default_path[2] = "*";
	char *arg_path       = default_path;

	for (size_t i=2; i<argument_num; i+=2) {
		switch (argument_hash[i]) {
			case NAMED_ARG_SYSTEMTIME :
				arg_system_time = String2Bool(argument[i+1]);
				break;

			case NAMED_ARG_ONLYNAME :
				arg_only_name = String2Bool(argument[i+1]);
				break;

			case NAMED_ARG_PATH :
				arg_path = argument[i+1];
				break;
		}
	}


	// Verify and update path to the file
	String buf_path;
	String_init(buf_path);
	char *ptr_path = arg_path;
	
	if (!VerifyPath(&ptr_path, buf_path, OPTION_ALLOW_GAME_ROOT_DIR)) {
		QWrite("[],[]]"); 
		break;
	}


	// Get list of files
	WIN32_FIND_DATA file_data;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	hFind		 = FindFirstFile(ptr_path, &file_data);

	if (hFind != INVALID_HANDLE_VALUE) {
		String Names;
		String Attributes;
		String_init(Names);
		String_init(Attributes);

		do {
			if (strcmp(file_data.cFileName,".")==0  ||  strcmp(file_data.cFileName,"..")==0)
				continue;

			String_append_quotes(Names, "]+[\"", file_data.cFileName, "\"");

			if (!arg_only_name) {
				String_append_format(Attributes, "]+[");
				
				// Name without extension and then extension alone
				char *dot = strrchr(file_data.cFileName, '.');
				char *ext = empty_string;

				if (dot) {
					int pos                  = dot - file_data.cFileName;
					file_data.cFileName[pos] = '\0';
					ext                      = file_data.cFileName + pos + 1;
				}

				String_append_format(Attributes, "[\"%s\",\"%s\",[", file_data.cFileName, ext);


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
								String_append_format(Attributes, ","); 
							else 
								add_comma = true;

							String_append_format(Attributes, "\"%s\"", attribute[index]);
						}
					}
				}

				String_append_format(Attributes, "],");

				// Creation, last access and last write time
				FILETIME *date_list[] = {&file_data.ftCreationTime, &file_data.ftLastAccessTime, &file_data.ftLastWriteTime};

				for (int i=0, max=sizeof(date_list)/sizeof(date_list[0]); i<max; i++) {
					char temp[64] = "";
					FileTimeToString(*date_list[i], arg_system_time, temp);
					String_append_format(Attributes, "%s%s", (i>0 ? "," : ""), temp);
				}

				// Size
				FileSize size = DivideBytes(file_data.nFileSizeLow);
				String_append_format(Attributes, ",[%f,%f,%f]]", size.bytes, size.kilobytes, size.megabytes);
			}
		} while (FindNextFile(hFind, &file_data));
		FindClose(hFind);

		QWrite_err(FWERROR_NONE, 0);
		QWritef("[%s],[%s]]", Names.text, Attributes.text);
		String_end(Names);
		String_end(Attributes);
	} else {
		QWrite_err(FWERROR_WINAPI, 2, GetLastError(), ptr_path);
		QWrite("[],[]]"); 
		break;
	}

	String_end(buf_path);
}
break;








case C_IGSE_LOAD:
{ // Load lines from a text file

	enum OUTPUT_TYPE {
		OUTPUT_SQS,
		OUTPUT_SQS_SEPARATE,
		OUTPUT_EXECUTE,
		OUTPUT_CLIP
	};
	
	char *arg_filename        = empty_string;
	int arg_filename_length   = 0;
	int arg_line_start        = 0;
	int arg_line_end          = 0;
	int arg_offset_start      = 0;
	int arg_limit_line_length = -1;
	int arg_wait              = 0;
	int arg_output            = OUTPUT_SQS;
	bool arg_offset           = false;
	bool arg_split_lines      = false;
	bool arg_delete_file      = false;
	bool arg_clip_append      = false;
			
	for (size_t i=2; i<argument_num; i+=2) {
		switch (argument_hash[i]) {
			case NAMED_ARG_FILE :
				arg_filename        = argument[i+1];
				arg_filename_length = argument_length[i+1];
				break;

			case NAMED_ARG_START :
				arg_line_start = atoi(argument[i+1]);
				break;

			case NAMED_ARG_END :
				arg_line_end = atoi(argument[i+1]);
				break;

			case NAMED_ARG_OFFSET :
				arg_offset_start = atoi(argument[i+1]);
				arg_offset       = true;
				break;

			case NAMED_ARG_CUT :
				arg_limit_line_length = atoi(argument[i+1]);
				break;

			case NAMED_ARG_SPLIT :
				arg_split_lines = String2Bool(argument[i+1]);
				break;

			case NAMED_ARG_MODE : {
				if (strcmpi(argument[i+1],"execute") == 0) {
					arg_output                  = OUTPUT_EXECUTE;
					global.option_error_output |= OPTION_ERROR_ARRAY_SUPPRESS;
				} else
					if (strcmpi(argument[i+1],"clipcopy") == 0)
						arg_output    = OUTPUT_CLIP;
					else
						if (strcmpi(argument[i+1],"clipappend") == 0) {
							arg_clip_append = true;
							arg_output      = OUTPUT_CLIP;
						} else
							if (strcmpi(argument[i+1],"separate") == 0)
								arg_output = OUTPUT_SQS_SEPARATE;
			} break;

			case NAMED_ARG_DELETE :
				arg_delete_file = String2Bool(argument[i+1]);
				break;

			case NAMED_ARG_WAIT :
				arg_wait = atoi(argument[i+1]);
				break;
		}
	}


	// File not specified
	if (arg_filename_length == 0) {
		QWrite_err(FWERROR_PARAM_EMPTY, 1, "arg_filename");
		
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
	String buf_filename;
	String_init(buf_filename);
	char *ptr_filename = arg_filename;
	int path_type      = VerifyPath(&ptr_filename, buf_filename, OPTION_ALLOW_GAME_ROOT_DIR);

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
		file = fopen(ptr_filename, "rb");
		if (!file  &&  arg_wait>0) {
			Sleep(10);
			arg_wait--;
		}
	} while (!file  &&  arg_wait>0);

	// Failed to open
	if (!file) {	
		if (arg_output != OUTPUT_EXECUTE) {
			QWrite_err(FWERROR_ERRNO, 2, errno, ptr_filename);
			QWrite("[],0,false,false,\"\",[],[]]");
		}

		String_end(buf_filename);
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
		QWrite_err(FWERROR_ERRNO, 2, errno, ptr_filename);
		if (arg_output != OUTPUT_EXECUTE)
			QWrite("[],0,false,false,\"\",[],[]]");
		String_end(buf_filename);
		fclose(file);
		break;
	};

	size_t file_size = ftell(file);
	if (file_size == 0xFFFFFFFF) {
		QWrite_err(FWERROR_ERRNO, 2, errno, ptr_filename);
		if (arg_output != OUTPUT_EXECUTE)
			QWrite("[],0,false,false,\"\",[],[]]");
		String_end(buf_filename);
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


	String sqs_lines;
	String sqs_offsets;
	String sqs_lengths;
	String buf_for_clip;
	String_init(sqs_lines);
	String_init(sqs_offsets);
	String_init(sqs_lengths);
	String_init(buf_for_clip);
	
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
						case OUTPUT_SQS_SEPARATE : {
							if (arg_split_lines)
								String_append_len(sqs_lines, "]+[[\"", 5);
							else
								String_append_len(sqs_lines, "]+[\"", 4);
								
							String_append_format(sqs_offsets, "\"%d\"]+[", arg_offset_start + (c==EOF ? pos : pos-1));
						} break;
						
						case OUTPUT_SQS : {
							String_append(sqs_lines, "]+[[\"");
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
					case OUTPUT_CLIP         : if (c!=EOF) String_append_len(buf_for_clip, &cc, 1); break;
					case OUTPUT_SQS_SEPARATE : {
						String_append(sqs_lines, "\"");
						String_append_format(sqs_lengths, "%d]+[", line_length);
						
						if (arg_split_lines)
							String_append(sqs_lines, "]");
					} break;
					case OUTPUT_SQS : {
						String_append_format(sqs_lines, "\",\"%d\"]", arg_offset_start + line_start);
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
			if ((c!='\r' && (arg_output==OUTPUT_SQS_SEPARATE || arg_output==OUTPUT_SQS))  ||  (arg_output!=OUTPUT_SQS_SEPARATE && arg_output!=OUTPUT_SQS)) {
				// Begin the line
				if (line_start == -1) {
					line_start = pos;
					line_empty = false;
					line_split = 0;
					
					if (line_num >= arg_line_start) {
						switch(arg_output) {
							case OUTPUT_SQS_SEPARATE : {
								if (arg_split_lines) {
									String_append(sqs_lines, "]+[[\"");
								} else
									String_append(sqs_lines, "]+[\"");
		
								String_append_format(sqs_offsets, "\"%d\"]+[", arg_offset_start + pos);
							} break;
							
							case OUTPUT_SQS : {
								String_append(sqs_lines, "]+[[\"");
							} break;
						}
					}
				}
				
				// Save current character
				if (line_num>=arg_line_start  &&  (arg_limit_line_length==-1 || (arg_limit_line_length!=-1 && pos-line_start<arg_limit_line_length) || arg_split_lines)) {
					switch (arg_output) {
						case OUTPUT_EXECUTE      : QWritel(&cc, 1); break;
						case OUTPUT_CLIP         : String_append_len(buf_for_clip, &cc, 1); break;
						case OUTPUT_SQS_SEPARATE : {
							if (c == '"')
								String_append_len(sqs_lines, "\"\"", 2);
							else
								String_append_len(sqs_lines, &cc, 1);
								
							line_split++;
							
							if (arg_split_lines) {
								if (line_split == arg_limit_line_length) {
									String_append_len(sqs_lines, "\"]+[\"", 5);
									line_split = 0;
								}
							}
						} break;
						case OUTPUT_SQS : {
							if (c == '"')
								String_append_len(sqs_lines, "\"\"", 2);
							else
								String_append_len(sqs_lines, &cc, 1);							
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
			QWrite_err(FWERROR_ERRNO, 2, errno, ptr_filename);
		else
			if (arg_line_start>=0 && line_num<arg_line_start)
				QWrite_err(FWERROR_FILE_NOLINE, 2, arg_line_start, ptr_filename);
			else
				if (arg_output == OUTPUT_CLIP) {
					if (CopyToClip(buf_for_clip.text, buf_for_clip.length, arg_clip_append))
						QWrite_err(FWERROR_NONE, 0);
				} else
					QWrite_err(FWERROR_NONE, 0);
		
		if (arg_output == OUTPUT_SQS_SEPARATE)
			QWritef("[[%s],[%s],[%s]]", sqs_lines.text, sqs_offsets.text, sqs_lengths.text);
		else
			QWritef("[%s]", sqs_lines.text);
			
		QWritef(",%d,%s,%s,\"%d\"]", line_count, getBool(line_empty), getBool(c==EOF), ftell(file));
	}
	
	fclose(file);
	String_end(sqs_lines);
	String_end(sqs_offsets);
	String_end(sqs_lengths);
	String_end(buf_for_clip);
	String_end(buf_filename);
	

	// If user wants to delete the arg_filename after reading it
	if (arg_delete_file) {
		if (path_type == PATH_DOWNLOAD_DIR)
			remove(ptr_filename);
		else
			trashFile(ptr_filename, strlen(ptr_filename), -1);
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

	char *arg_filename      = empty_string;
	int arg_filename_length = 0;
	int arg_edit_mode       = IGSE_NEWFILE_CREATE;
	bool arg_unique         = false;
	bool arg_subdirs        = false;
	bool arg_directory      = false;

	for (size_t i=2; i<argument_num; i+=2) {
		switch (argument_hash[i]) {
			case NAMED_ARG_FILE :
				arg_filename        = argument[i+1];
				arg_filename_length = argument_length[i+1];
				break;

			case NAMED_ARG_MODE : {
				if (strcmpi(argument[i+1],"create") == 0)
					arg_edit_mode = IGSE_NEWFILE_CREATE;
				else 
					if (strcmpi(argument[i+1],"delete") == 0)
						arg_edit_mode = IGSE_NEWFILE_DELETE;
					else
						if (strcmpi(argument[i+1],"recreate") == 0)
							arg_edit_mode = IGSE_NEWFILE_RECREATE;
						else
							if (strcmpi(argument[i+1],"check") == 0)
								arg_edit_mode = IGSE_NEWFILE_CHECK;
			} break;

			case NAMED_ARG_UNIQUE :
				arg_unique = String2Bool(argument[i+1]);
				break;

			case NAMED_ARG_SUBDIRS :
				arg_subdirs = String2Bool(argument[i+1]);
				break;

			case NAMED_ARG_DIRECTORY :
				arg_directory = String2Bool(argument[i+1]);
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
	int check_mode     = arg_edit_mode==IGSE_NEWFILE_CHECK ? OPTION_ALLOW_GAME_ROOT_DIR : OPTION_RESTRICT_TO_MISSION_DIR;
	int path_type      = VerifyPath(&ptr_filename, buf_filename, check_mode);
	
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
		for (int i=0; ptr_filename[i]!='\0'; i++)
			if (ptr_filename[i] == '/') 
				ptr_filename[i] = '\\';

		char *slash = NULL;
		char *path  = ptr_filename;

		// For each arg_directory along the way
		while ((slash = strchr(path,'\\'))) {
			// Separate name from the rest of the string
			int pos   = slash - path;
			char prev = path[pos];
			path[pos] = '\0';

			CreateDirectory(ptr_filename, NULL);

			// Move on to the next
			path[pos] = prev;
			path     += pos+1;
		}
	}


	// See if the file is already there
	if (CheckIt) {		
		WIN32_FILE_ATTRIBUTE_DATA fad;

		if (GetFileAttributesEx(ptr_filename, GetFileExInfoStandard, &fad)) {
			ItExists = (fad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? 2 : 1;

			if (arg_edit_mode == IGSE_NEWFILE_CHECK) {
				if (!arg_directory) {
					QWrite_err(FWERROR_NONE, 0);
				} else
					if (ItExists != 2)
						QWrite_err(FWERROR_FILE_NOTDIR, 1, ptr_filename);
			}
		} else
			if (arg_edit_mode == IGSE_NEWFILE_CHECK)
				QWrite_err(FWERROR_WINAPI, 2, GetLastError(), ptr_filename);
	}


	// Trash or delete
	if (DeleteIt) {
		// if the file doesn't exist then ignore error so that we can't delete it
		int ErrorBehaviour = !ItExists ? 1 : 0;

		// Default - trash it
		if (path_type != PATH_DOWNLOAD_DIR) {
			if (trashFile(ptr_filename, strlen(ptr_filename), ErrorBehaviour))
				QWrite_err(FWERROR_NONE, 0);
			else
				CreateIt = false;
		} else {		
			// ..\fwatch\tmp - delete it
			int result = DeleteWrapper(ptr_filename);

			if (result == 0)
				QWrite_err(FWERROR_NONE, 0);
			else
				if (arg_edit_mode != IGSE_NEWFILE_RECREATE) {
					CreateIt = false;
					QWrite_err(FWERROR_WINAPI, 2, GetLastError(), ptr_filename);
				}
		}
	}


	// Create new
	if (CreateIt) {
		// If we want error when it already exists
		if ((arg_unique && ItExists)  ||  (arg_directory && ItExists==1)) {
			if (arg_directory  &&  ItExists==2)
				QWrite_err(FWERROR_FILE_DIREXISTS, 1, ptr_filename);
			else
				QWrite_err(FWERROR_FILE_EXISTS, 1, ptr_filename);
		} else
			// arg_filename
			if (!arg_directory) {
				FILE *file = fopen(ptr_filename, "a");

				if (file) {
					QWrite_err(FWERROR_NONE, 0);
					fclose(file);
				} else
					QWrite_err(FWERROR_ERRNO, 2, errno, ptr_filename);
			}
			// folder
			else
				if (CreateDirectory(ptr_filename, NULL))
					QWrite_err(FWERROR_NONE, 0);
				else 
					// ignore error that dir already exists
					if (GetLastError() == ERROR_ALREADY_EXISTS)
						QWrite_err(FWERROR_NONE, 0);
					else
						QWrite_err(FWERROR_WINAPI, 1, GetLastError());
	}

	String_end(buf_filename);
}
break;









case C_IGSE_RENAME:
case C_IGSE_COPY:
{ // IGSE Rename/Copy

	global.option_error_output = OPTION_ERROR_ARRAY_CLOSE;

	char *arg_source      = empty_string;
	char *arg_dest        = empty_string;
	bool arg_overwrite    = false;
	int arg_source_length = 0;
	int arg_dest_length   = 0;

	for (size_t i=2; i<argument_num; i+=2) {
		switch (argument_hash[i]) {
			case NAMED_ARG_SOURCE :
				arg_source        = argument[i+1];
				arg_source_length = argument_length[i+1];
				break;

			case NAMED_ARG_DESTINATION :
				arg_dest        = argument[i+1];
				arg_dest_length = argument_length[i+1];
				break;

			case NAMED_ARG_OVERWRITE :
				arg_overwrite = String2Bool(argument[i+1]);
				break;
		}
	}


	// Files not specified
	if (arg_source_length == 0) {
		QWrite_err(FWERROR_PARAM_EMPTY, 1, "arg_source");
		break;
	}

	if (arg_dest_length == 0) {
		QWrite_err(FWERROR_PARAM_EMPTY, 1, "destination");
		break;
	}


	// Verify and update path to the files
	String source, dest;
	String_init(source);
	String_init(dest);
	char *ptr_source     = arg_source;
	char *ptr_dest       = arg_dest;
	int check_mode       = argument_hash[0]==C_IGSE_RENAME ? OPTION_RESTRICT_TO_MISSION_DIR : OPTION_ALLOW_GAME_ROOT_DIR;
	int source_path_type = VerifyPath(&ptr_source, source, check_mode);
	int dest_path_type   = VerifyPath(&ptr_dest, dest, OPTION_RESTRICT_TO_MISSION_DIR);

	if (!source_path_type || !dest_path_type)
		break;

	// Rename
	if (argument_hash[0] == C_IGSE_RENAME) {
		if (source_path_type!=PATH_DOWNLOAD_DIR  &&  dest_path_type==PATH_DOWNLOAD_DIR)	// Not allowed to move files to the download directory
			QWrite_err(FWERROR_FILE_MOVETOTMP, 2, ptr_source, ptr_dest);
		else {
			if (rename(ptr_source,ptr_dest) == 0)
				QWrite_err(FWERROR_NONE, 0);
			else
				QWrite_err(FWERROR_ERRNO, 3, errno, ptr_source, ptr_dest);
		}		
	// Copy
	} else {
		if (arg_overwrite  &&  dest_path_type!=PATH_DOWNLOAD_DIR)	// Overwrite means trashing before copying
			trashFile(ptr_dest, strlen(ptr_dest), 0);

		if (CopyFile((LPCTSTR)ptr_source, (LPCTSTR)ptr_dest, dest_path_type!=PATH_DOWNLOAD_DIR))
			QWrite_err(FWERROR_NONE, 0);
		else
			QWrite_err(FWERROR_WINAPI, 3, GetLastError(), ptr_source, ptr_dest);
	}

	String_end(source);
	String_end(dest);
}
break;









case C_IGSE_FIND:
{ // IGSE Find

	int Start			 = 0;
	int End				 = 0;
	int start_pos        = 0;
	int Limit			 = 0;
	int arg_col_num      = -1;
	int options          = OPTION_NONE;
	char *SearchIn		 = empty_string;
	char *SearchFor		 = empty_string;
	char *ReplaceWith	 = empty_string;
	bool start_pos_set   = false;
	bool Replace		 = false;
	bool ignoreLeadSpace = false;

	for (size_t i=2; i<argument_num; i+=2) {
		switch (argument_hash[i]) {
			case NAMED_ARG_FILE :
				SearchIn = argument[i+1];
				break;

			case NAMED_ARG_START :
				Start = atoi(argument[i+1]);
				break;

			case NAMED_ARG_END :
				End = atoi(argument[i+1]);
				break;

			case NAMED_ARG_OFFSET :
				start_pos     = atoi(argument[i+1]);
				start_pos_set = true; 
				break;

			case NAMED_ARG_LIMIT :
				Limit = atoi(argument[i+1]);
				break;

			case NAMED_ARG_TEXT :
				SearchFor = argument[i+1];
				break;

			case NAMED_ARG_REPLACE :
				Replace     = true;
				ReplaceWith = argument[i+1];
				break;

			case NAMED_ARG_COLUMN :
				arg_col_num = atoi(argument[i+1]);
				break;

			case NAMED_ARG_IGNORELEADSPACE :
				ignoreLeadSpace = String2Bool(argument[i+1]);
				break;

			case NAMED_ARG_CASESENSITIVE :
				String2Bool(argument[i+1]) ? options|=OPTION_CASESENSITIVE : options&=~OPTION_CASESENSITIVE;
				break;

			case NAMED_ARG_MATCHWORD :
				String2Bool(argument[i+1]) ? options|=OPTION_MATCHWORD : options&=~OPTION_MATCHWORD;
				break;
		}
	}


	// Number validation
	if (Start < 0) 
		Start = 0;

	if (End < 0) 
		End = 0;

	if (Limit < 0) 
		Limit = 0;

	if (Start>End  &&  End>0) {
		QWrite_err(FWERROR_PARAM_RANGE, 4, "Start", "End", Start, End);
		QWrite("0,[],[],\"0\",0]");
		break;
	}


	// File not specified
	if (strcmpi(SearchIn,"") == 0) {
		QWrite_err(FWERROR_PARAM_EMPTY, 1, "SearchIn");
		QWrite("0,[],[],\"0\",0]");
		break;
	}


	// Search target not specified
	if (strcmpi(SearchFor,"") == 0) {
		QWrite_err(FWERROR_PARAM_EMPTY, 1, "SearchFor");
		QWrite("0,[],[],\"0\",0]");
		break;
	}


	// Verify and update path to the file
	String buf_filename;
	String_init(buf_filename);
	char *ptr_filename = SearchIn;

	if (!VerifyPath(&ptr_filename, buf_filename, OPTION_ALLOW_GAME_ROOT_DIR)) {
		QWrite("0,[],[],\"0\",0]");
		break;
	}
	//-------------------------------------------------------------------------



	// Open file ------------------------------------------------------------------------
	FILE *f = fopen(ptr_filename, "r");
	if (!f) {
		QWrite_err(FWERROR_ERRNO, 2, errno, ptr_filename);
		QWrite("0,[],[],\"0\",0]");
		String_end(buf_filename);
		break;
	}


	// Set up vars and allocate buffers
	int single_line_size = 512;
	int rowsLen          = 20;
	int colsLen          = 20;
	int matches          = 0;
	int lastPos          = 0;
	int lastOccOff       = 0;
	int lastOffset       = -1;
	int CurrentROW       = 1;
	bool isEOL           = false; 
	bool error           = false;

	char *single_line = (char*) malloc (single_line_size);
	char *rows        = (char*) malloc (rowsLen);
	char *cols        = (char*) malloc (colsLen);
	char *lineNEW;
	char *rowsNEW; 
	char *colsNEW; 
	char *ret;
	char *p;
	char tmp[128] = "";


	// If failed to allocate
	if (single_line==NULL  ||  rows==NULL  ||  cols==NULL) {
		char failedBuf[20] = "";
		int failedBufL     = 0;
		
		if (!single_line) {
			strcat(failedBuf,"single_line ");
			failedBufL += single_line_size;
		} else
			free(single_line); 

		if (!rows) {
			strcat(failedBuf,"rows ");
			failedBufL += rowsLen;
		} else
			free(rows); 

		if (!cols) {
			strcat(failedBuf,"cols ");
			failedBufL += colsLen;
		} else
			free(cols); 

		QWrite_err(FWERROR_MALLOC, 2, "failedBuf", failedBufL);
		QWrite("0,[],[],\"0\",0]");
		String_end(buf_filename);
		break;
	}

	strcpy(rows, "["); 
	strcpy(cols, "[");


	// If this is a replace operation then allocate buffer for the whole file
	int FileBufPos  = 0;
	int FileBufSize = 0;
	char *FileBuf   = empty_string;
	bool Replaced   = false;

	if (Replace) {
		fseek(f, 0, SEEK_END);
		FileBufSize = ftell(f);
		FileBuf     = (char*) malloc (FileBufSize);

		if (!FileBuf) {
			QWrite_err(FWERROR_MALLOC, 2, "FileBuf", FileBufSize);
			QWrite("0,[],[],\"0\",0]");
			free(single_line); 
			free(rows); 
			free(cols); 
			String_end(buf_filename);
			break;
		}

		fseek(f, 0, SEEK_SET);
	}


	// Start reading file from the given position
	if (start_pos_set) {
		CurrentROW = Start;
		fseek(f, start_pos, SEEK_SET);
		lastPos    = start_pos;
		lastOccOff = start_pos;
	}
	//---------------------------------------------------------------------------


	// Iterate lines ------------------------------------------------------------
	while (!feof(f)  &&  ((CurrentROW<=End && End!=0 && !Replace) || End==0 || Replace)) {
		ret = fgets(single_line, single_line_size, f);

		if (ferror(f)) {
			QWrite_err(FWERROR_ERRNO, 2, errno, ptr_filename);
			error = 1;
		}

		if (!ret) 
			strcpy(single_line, "");

		isEOL = single_line[strlen(single_line)-1] == '\n';
		

		// If a long single_line then reallocate buffer and read again
		if (!isEOL && !feof(f)  &&  ret!=NULL) {
			single_line_size += 512;
			lineNEW = (char*) realloc (single_line, single_line_size);

			if (lineNEW) 
				single_line = lineNEW; 
			else {	
				QWrite_err(FWERROR_REALLOC, 2, "single_line", single_line_size);
				error = 1; 
				break;
			}

			fseek (f, lastPos, SEEK_SET);
			continue;
		}


		// If current single_line number is in range
		if (CurrentROW>=Start  &&  ((End>0 && CurrentROW<=End) || (End==0 && CurrentROW>=Start))) {
			char *backup_line = single_line;
			int i             = 0;
			int CurrentCOL    = 0;
			int maxIterations = 0;
			int leadSpace     = 0;

			// If we want to ignore leading whitespace then count it
			while (ignoreLeadSpace  &&  isspace(single_line[leadSpace])  &&  single_line[leadSpace]!='\0')
				leadSpace++;

			// Find all positions of the occurences in the current single_line
			while ((p = strstr2(backup_line, strlen(backup_line), SearchFor, strlen(SearchFor), options))) {
				maxIterations++;

				if (maxIterations > 10)
					break;

				int pos = p - backup_line;

				// If no limit regarding columns or if occurence is on the wanted column
				if (arg_col_num==-1  ||  (arg_col_num!=-1 && arg_col_num == CurrentCOL+pos-leadSpace)) {
					// Add column number to the return data
					if (i == 0) {
						colsLen += 4;
						sprintf(tmp, "[%d", CurrentCOL + pos); 
					} else 
						sprintf(tmp, "]+[%d", CurrentCOL + pos);

					colsLen += strlen(tmp) + 1;
					colsNEW  = (char*) realloc (cols, colsLen);

					if (colsNEW) {
						cols = colsNEW;
						strcat(cols, tmp); 
					} else {
						QWrite_err(FWERROR_REALLOC, 2, "cols" ,colsLen);
						error = 1; 
						break;
					}

					i++;
					matches++;
				}

				backup_line = single_line + CurrentCOL + pos + 1;
				CurrentCOL += pos+1;
				lastOccOff  = lastPos + CurrentCOL;

				if (Limit!=0  &&  matches>=Limit) {
					lastOffset = lastOccOff;
					break;
				}
			}


			// Add single_line number to array
			if (i > 0) {
				strcat(cols, "]]+[");
				sprintf(tmp, "]+[%d", CurrentROW);

				rowsLen += strlen(tmp) + 1;
				rowsNEW  = (char*) realloc (rows, rowsLen);

				if (rowsNEW) {
					rows = rowsNEW;
					strcat(rows, tmp); 
				} else {
					QWrite_err(FWERROR_REALLOC, 2, "rows", rowsLen);
					error = 1; 
					break;
				}
			}


			// Replace occurences
			if (Replace  &&  i>0) {
				Replaced  = true;

				char *rep = str_replace(single_line, SearchFor, ReplaceWith, options);
				if (!rep) {
					QWrite_err(FWERROR_STR_REPLACE, 2, ReplaceWith, strlen(single_line));
					error = 1; 
					break;
				}

				// If result is larger than original then reallocate buffer for the new file
				unsigned int newLength = strlen(rep);
				unsigned int oldLength = strlen(single_line);

				//if (strlen(single_line) > l)
				if (newLength > oldLength) {
					FileBufSize += newLength - oldLength;

					char *FileBufNEW = (char*) realloc (FileBuf, FileBufSize);

					if (FileBufNEW) 
						FileBuf = FileBufNEW;
					else {
						QWrite_err(FWERROR_REALLOC, 2, "FileBuf", rowsLen);
						error = 1; 
						break;
					}
				}

				// Copy modified single_line to the buffer
				memcpy(FileBuf+FileBufPos, rep, newLength+1),
				FileBufPos += newLength;
				free(rep);
			}
		}

		lastPos = ftell(f);
		CurrentROW++;

		if (Limit!=0  &&  matches>=Limit) 
			break;


		// If this is a replace operation then copy lines to a buffer
		if (Replace) {
			if (!Replaced) {
				int l = strlen(single_line);
				memcpy(FileBuf+FileBufPos, single_line, l+1);
				FileBufPos += l;
			} else
				Replaced = false;
		}
	}


	// If did not stop in the middle of the single_line then point to the next single_line
	if (lastOffset == -1) {
		lastOffset = lastPos;

		if (!feof(f))
			CurrentROW++;
	}

	fclose(f); 
	free(single_line);
	//-------------------------------------------------------------------------



	// If this was a replace operation then rewrite the file
	if (Replace  &&  matches>0) {
		f = fopen(ptr_filename, "w");
		if (f) {
			fwrite(FileBuf, 1, strlen(FileBuf), f);

			if (ferror(f)) {
				error = 1;
				QWrite_err(FWERROR_ERRNO, 2, errno, ptr_filename);
			}

			fclose(f);
		} else {
			error = 1;
			QWrite_err(FWERROR_ERRNO, 2, errno, ptr_filename);
		}
	}


	// If error
	if (!error) {	
		QWrite_err(FWERROR_NONE, 0);
		QWritef("%d,%s],%s],\"%d\",%d]", matches, rows, cols, lastOffset, --CurrentROW);
	} else
		QWrite("0,[],[],\"0\",0]");


	free(cols); 
	free(rows); 
	String_end(buf_filename);
	
	if (Replace)
		free(FileBuf);
}
break;









case C_IGSE_DB:
{ // IGSE Database

	char *arg_filename         = empty_string;
	bool arg_writing_mode      = false;
	bool arg_list              = false;
	size_t arg_new_keys        = 0;
	size_t arg_arguments_size  = 0;
	size_t arg_filename_length = 0;

	// Count argument length in order to determine required buffer size
	for (size_t i=2; i<argument_num; i+=2) {
		switch (argument_hash[i]) {
			case NAMED_ARG_FILE :
				arg_filename        = argument[i+1];
				arg_filename_length = argument_length[i+1];
				break;

			case NAMED_ARG_KEY :
				arg_new_keys++;	
				arg_arguments_size += argument_length[i+1] + 1 + sizeof(int)*3;
				break;

			case NAMED_ARG_WRITE :
				arg_arguments_size += argument_length[i+1] + 1;
				arg_writing_mode    = true;
				break;

			case NAMED_ARG_APPEND :
				arg_arguments_size += argument_length[i+1] + 1;
				arg_writing_mode    = true;
				break;

			case NAMED_ARG_RENAME :
				arg_arguments_size += argument_length[i+1] + 1;
				arg_writing_mode    = true;
				break;

			case NAMED_ARG_REMOVE :
				arg_writing_mode = true;
				break;

			case NAMED_ARG_LIST :
				arg_list = true;
				break;
		}
	}		

	
	// File not specified
	if (arg_filename_length == 0) {
		QWrite_err(FWERROR_PARAM_EMPTY, 1, "arg_filename");
		QWrite("[],[]]");
		break;
	}

	// Verify and update path to the file
	String buf_filename;
	String_init(buf_filename);
	char *ptr_filename = arg_filename;
	
	if (!VerifyPath(&ptr_filename, buf_filename, OPTION_RESTRICT_TO_MISSION_DIR)) {
		QWrite("[],[]]");
		break;
	}


	// Read file signature
	FILE *file = fopen(ptr_filename,"rb");
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
			QWrite_err(FWERROR_ERRNO, 2, errno, ptr_filename);
			QWrite("[],[]]");
			String_end(buf_filename);
			fclose(file);
			break;
		}

		if (header_existing.signature != igsedb_signature) {
			QWrite_err(FWERROR_DB_SIGNATURE, 3, header_existing.signature, igsedb_signature, ptr_filename);
			QWrite("[],[]]");
			String_end(buf_filename);
			fclose(file);
			break;
		}

		if (header_existing.version > igsedb_version) {
			QWrite_err(FWERROR_DB_VERSION, 3, header_existing.version, igsedb_version, ptr_filename);
			QWrite("[],[]]");
			String_end(buf_filename);
			fclose(file);
			break;
		}

		// Find file size
		if (fseek(file, 0, SEEK_END) != 0) {
			QWrite_err(FWERROR_ERRNO, 2, errno, ptr_filename);
			QWrite("[],[]]");
			String_end(buf_filename);
			fclose(file);
			break;
		};

		buffer_size = ftell(file);
		if (buffer_size == 0xFFFFFFFF) {
			QWrite_err(FWERROR_ERRNO, 2, errno, ptr_filename);
			QWrite("[],[]]");
			String_end(buf_filename);
			fclose(file);
			break;
		};
		
		fseek(file, 0, SEEK_SET);
		
		// Check for minimal file size (header size + size of all uint arrays + all terminating zeros)
		size_t minimal_file_size = sizeof(igsedb_header) + header_existing.number_of_keys*sizeof(size_t)*3 + header_existing.number_of_keys*2;
		if (buffer_size < minimal_file_size) {
			QWrite_err(FWERROR_DB_SMALL, 3, buffer_size, minimal_file_size, ptr_filename);
			QWrite("[],[]]");
			String_end(buf_filename);
			fclose(file);
			break;
		}
	} else {
		if (!arg_writing_mode  ||  (arg_writing_mode && errno!=2)) {
			QWrite_err(FWERROR_ERRNO, 2, errno, ptr_filename);
			QWrite("[],[]]");
			String_end(buf_filename);
			break;
		}

		arg_arguments_size += sizeof(igsedb_header);
	}


	// Allocate memory for the data
	String file_contents;
	String_init(file_contents);
	size_t buffer_size_max = buffer_size + arg_arguments_size + 1;

	if (String_allocate(file_contents, buffer_size_max) != 0) {
		QWrite_err(FWERROR_MALLOC, 2, "file_contents", buffer_size_max);
		QWrite("[],[]]");
		String_end(buf_filename);
		break;
	}

	char *buffer = file_contents.text;

	
	// Read data from the selected file to the allocated memory
	if (file) {
		size_t bytes_read = fread(buffer, 1, buffer_size, file);
		fclose(file);

		if (bytes_read != buffer_size) {
			QWrite_err(FWERROR_ERRNO, 2, errno, ptr_filename);
			QWrite("[],[]]");
			String_end(buf_filename);
			break;
		}
	} else {
		igsedb_header temp = {igsedb_signature, igsedb_version, 0};
		memcpy(buffer, &temp, sizeof(igsedb_header));
	}
	
	header = (igsedb_header*)buffer;


	// Verify database
	bool db_error = false;
	
	if (arg_writing_mode || arg_list) {
		unsigned int hash_previous   = 0;
		size_t pointer_previous      = 0;
		size_t minimal_pointer_value = sizeof(igsedb_header) + header->number_of_keys * sizeof(size_t) * 3;
		
		for (size_t i=0; i<header->number_of_keys; i++) {
			size_t hash_pos    = sizeof(igsedb_header) + i * sizeof(size_t);
			unsigned int *hash = (unsigned int*)(buffer + hash_pos);
			
			if (*hash <= hash_previous) {
				QWrite_err(FWERROR_DB_HASHORDER, 3, i, i+1, ptr_filename);
				db_error = true;
				break;
			}
			
			hash_previous = *hash;
		}
		
		for (i=0;  i<header->number_of_keys*2 && !db_error;  i++) {
			size_t pointer_pos    = sizeof(igsedb_header) + (header->number_of_keys+i) * sizeof(size_t);
			size_t *pointer_value = (size_t*)(buffer + pointer_pos);
			
			if (i==0  &&  *pointer_value!=minimal_pointer_value) {
				QWrite_err(FWERROR_DB_PTRFIRST, 3, *pointer_value, minimal_pointer_value, ptr_filename);
				db_error = true;
				break;
			}
				
			if (*pointer_value < minimal_pointer_value) {
				QWrite_err(FWERROR_DB_PTRSMALL, 4, i, header->number_of_keys*2-1, *pointer_value, minimal_pointer_value, ptr_filename);
				db_error = true;
				break;
			}
			
			if (*pointer_value > buffer_size) {
				QWrite_err(FWERROR_DB_PTRBIG, 4, i, header->number_of_keys*2-1, *pointer_value, buffer_size, ptr_filename);
				db_error = true;
				break;
			}
			
			if ((i>0 && buffer[*pointer_value-1]!='\0') || (i==header->number_of_keys*2-1  &&  buffer[buffer_size-1]!='\0')) {
				QWrite_err(FWERROR_DB_CONSISTENCY, 4, i+1, header->number_of_keys*2, buffer[*pointer_value-1]!='\0' ? *pointer_value-1 : buffer_size-1, ptr_filename);
				db_error = true;
				break;
			}
			
			if (*pointer_value <= pointer_previous) {
				QWrite_err(FWERROR_DB_PTRORDER, 5, i+1, *pointer_value, i, pointer_previous, ptr_filename);
				db_error = true;
				break;
			}

			pointer_previous = *pointer_value;
		}
	}


	// Go through the arguments and execute them
	unsigned int key_hash = 0;
	char *key_name        = empty_string;
	size_t key_length     = 0;
	BinarySearchResult key_selected;
	String MissingKeys, List;
	String_init(MissingKeys);
	String_init(List);

	for (i=2;  i<argument_num && !db_error;  i+=2) {
		char *arg_value         = argument[i+1];
		size_t arg_value_length = argument_length[i+1] + 1; //argument length with terminating zero
		
		switch(argument_hash[i]) {
			case NAMED_ARG_KEY : {
				key_name     = arg_value;
				key_length   = arg_value_length;
				key_hash     = fnv1a_hash(FNV_BASIS, argument[i+1], argument_length[i+1], OPTION_LOWERCASE);
				key_selected = binary_search_str(buffer+sizeof(igsedb_header), header->number_of_keys, key_hash, 0, header->number_of_keys-1);
				
				// Check for hash collision
				size_t key_pointer_pos = sizeof(igsedb_header) + (header->number_of_keys + key_selected.index) * sizeof(size_t);
				memcpy(&key_pointer, buffer+key_pointer_pos, sizeof(igsedb_pointer));
				
				if (key_selected.found  &&  strncmpi(buffer+key_pointer.start,key_name,key_length) != 0) {
					size_t hash_pos    = sizeof(igsedb_header) + key_selected.index * sizeof(unsigned int);
					unsigned int *hash = (unsigned int*)buffer + hash_pos;
					
					QWrite_err(FWERROR_DB_COLLISION, 5, arg_value, key_hash, buffer+key_pointer_pos, *hash, ptr_filename);
					db_error = true;
					break;
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
					key_hash        = fnv1a_hash(FNV_BASIS, argument[i+1], argument_length[i+1], OPTION_LOWERCASE);
					memcpy(buffer+hash_pos, &key_hash, sizeof(unsigned int));
					
					// Adjust pointers to keys and values
					for (size_t j=0; j<header->number_of_keys*2; j++) {
						size_t pointer_pos = sizeof(igsedb_header) + (header->number_of_keys + j) * sizeof(size_t);
						size_t *pointer    = (size_t*)(buffer + pointer_pos);

						if (j > key_selected.index)
							*pointer += buffer_shift_amount * (buffer_shift_direction ? 1 : -1);
					}
					
					buffer_size += buffer_shift_amount * (buffer_shift_direction ? 1 : -1);
				} else
					String_append_quotes(MissingKeys, "]+[\"", key_name, "\"");
			} break;
			
			case NAMED_ARG_REMOVE : {
				BinarySearchResult key_to_remove = binary_search_str(buffer+sizeof(igsedb_header), header->number_of_keys, fnv1a_hash(FNV_BASIS, argument[i+1], argument_length[i+1], OPTION_LOWERCASE), 0, header->number_of_keys-1);
					
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
				} else
					String_append_quotes(MissingKeys, "]+[\"", argument[i+1], "\"");
			} break;
			
			case NAMED_ARG_READ : {
				BinarySearchResult key_to_read = binary_search_str(buffer+sizeof(igsedb_header), header->number_of_keys, fnv1a_hash(FNV_BASIS, argument[i+1], argument_length[i+1], OPTION_LOWERCASE), 0, header->number_of_keys-1);
	
				if (key_to_read.found) {
					size_t value_pointer_pos = sizeof(igsedb_header) + (header->number_of_keys*2 + key_to_read.index) * sizeof(size_t);
					memcpy(&value_pointer, buffer+value_pointer_pos, sizeof(igsedb_pointer));
					
					if (key_to_read.index == header->number_of_keys-1)
						value_pointer.end = buffer_size;
						
					// Check if pointer isn't too small
					size_t minimal_pointer_value = sizeof(igsedb_header) + header->number_of_keys * sizeof(size_t) * 3;
					if (value_pointer.start < minimal_pointer_value) {
						QWrite_err(FWERROR_DB_PTRSMALL, 4, (key_to_read.index+1)*2, header->number_of_keys*2, value_pointer.start, minimal_pointer_value, ptr_filename);
						db_error = true;
						break;
					}
					
					// Check if pointer isn't too big
					if (value_pointer.end > buffer_size) {
						QWrite_err(FWERROR_DB_PTRBIG, 4, (key_to_read.index+1)*2, header->number_of_keys*2, value_pointer.end, buffer_size, ptr_filename);
						db_error = true;
						break;
					}

					// Value must be surrounded by terminating zeros                     
					if ((key_to_read.index>0 && buffer[value_pointer.start-1]!='\0')  ||  buffer[value_pointer.end-1]!='\0') {
						QWrite_err(FWERROR_DB_CONSISTENCY, 4, (key_to_read.index+1)*2, header->number_of_keys*2, buffer[value_pointer.start-1]!='\0' ? value_pointer.start-1 : value_pointer.end-1, ptr_filename);
						db_error = true;
						break;
					}
						
					QWritel(buffer+value_pointer.start, value_pointer.end-1-value_pointer.start);
					QWrite(";");
				} else
					String_append_quotes(MissingKeys, "]+[\"", argument[i+1], "\"");
			} break;
			
			case NAMED_ARG_LIST : {
				if (strcmpi(argument[i+1],"values") == 0) {
					for (size_t j=0; j<header->number_of_keys; j++) {
						size_t value_pointer_pos = sizeof(igsedb_header) + (header->number_of_keys*2 + j) * sizeof(size_t);
						memcpy(&value_pointer, buffer+value_pointer_pos, sizeof(igsedb_pointer));
						
						if (j == header->number_of_keys-1)
							value_pointer.end = buffer_size;
	
						QWritel(buffer+value_pointer.start, value_pointer.end-1-value_pointer.start);
						QWrite(";");
					}
				} else {
					if (strcmpi(argument[i+1],"keys") == 0) {
						for (size_t j=0; j<header->number_of_keys; j++) {
							int key_pointer_pos = sizeof(igsedb_header) + (header->number_of_keys + j) * sizeof(size_t);
							memcpy(&key_pointer, buffer+key_pointer_pos, sizeof(igsedb_pointer));
	
							String_append(List, "]+[\"");
							for (; key_pointer.start<key_pointer.end-1; key_pointer.start++) {
								char *c = buffer+key_pointer.start;
								char cc[2] = "";
								strncpy(cc, c, 1);
								if (*c == '"')
									String_append(List, "\"");
								else
									String_append(List, cc);
							}
							String_append(List, "\"");
						}
					} else 
						if (strcmpi(argument[i+1],"all") == 0) {
							String_append(List, "[");
	
							for (size_t j=0; j<header->number_of_keys; j++) {
								size_t key_pointer_pos = sizeof(igsedb_header) + (header->number_of_keys + j) * sizeof(size_t);
								memcpy(&key_pointer, buffer+key_pointer_pos, sizeof(igsedb_pointer));
								
								String_append(List, "]+[\"");
								for (; key_pointer.start<key_pointer.end-1; key_pointer.start++) {
									char *c = buffer+key_pointer.start;
									char cc[2] = "";
									strncpy(cc, c, 1);
									if (*c == '"')
										String_append(List, "\"");
									String_append(List, cc);
								}
								String_append(List, "\"");
							}
	
							String_append(List, "]]+[[");
	
							for (j=0; j<header->number_of_keys; j++) {
								size_t value_pointer_pos = sizeof(igsedb_header) + (header->number_of_keys*2 + j) * sizeof(size_t);
								memcpy(&value_pointer, buffer+value_pointer_pos, sizeof(igsedb_pointer));
								
								if (j == header->number_of_keys-1)
									value_pointer.end = buffer_size;
								
								String_append(List, "]+[\"");
								for (; value_pointer.start<value_pointer.end-1; value_pointer.start++) {
									char *c = buffer+value_pointer.start;
									char cc[2] = "";
									strncpy(cc, c, 1);
									if (*c == '"')
										String_append(List, "\"");
									String_append(List, cc);
								}
								String_append(List, "\"");
							}
	
							String_append(List, "]");
						}
				}
			} break;
		}
	}


	// Rewrite the file if changes were made
	if (arg_writing_mode && !db_error) {
		char *file_ptr = ptr_filename;
		
		for (size_t i=0; ptr_filename[i]!='\0'; i++)	// Replace slashes with backslashes
			if (ptr_filename[i] == '/') 
				ptr_filename[i] = '\\';

		// Create subdirectories
		char *path_item;
		while ((path_item = strchr(file_ptr,'\\'))) {
			size_t slash_pos    = path_item - file_ptr;
			char backup         = file_ptr[slash_pos];
			file_ptr[slash_pos] = '\0';

			CreateDirectory(ptr_filename, NULL);
			
			file_ptr[slash_pos] = backup;
			file_ptr           += slash_pos + 1;
		}
		
		if ((file = fopen(ptr_filename, "wb"))) {
			fwrite(buffer, sizeof(*buffer), buffer_size, file);
		}
	}

	if (!db_error) {
		if (file  &&  ferror(file))
			QWrite_err(FWERROR_ERRNO, 2, errno, ptr_filename);
		else
			QWrite_err(FWERROR_NONE, 0);
	}

	QWritef("[%s],[%s]]", List.text, MissingKeys.text);
	String_end(file_contents);
	String_end(buf_filename);
	String_end(List);
	String_end(MissingKeys);
		
	if (file)
		fclose(file);
}
break;