// -----------------------------------------------------------------
// IGSE DEDICATED COMMANDS
// -----------------------------------------------------------------

case C_IGSE_WRITE:
{ // Write line to a text file

	// Read arguments------------------------------------------------------------
	char *arg_filename  = "";
	char *arg_txt       = "";
	char *arg_escape    = "";
	char *arg_open_mode = "create";
	bool arg_clipboard  = false;
	bool arg_column     = false;
	int arg_line_start  = 0;
	int arg_edit_mode   = IGSE_WRITE_REPLACE;
	int arg_column_num  = 0;
	int arg_line_range  = 1;

	for (int i=2; i<numP; i++) {
		char *arg = stripq(par[i]);
		char *col = strchr(arg, ':');

		if (col == NULL) 
			continue;

		int pos   = col - arg;
		arg[pos]  = '\0';
		char *val = arg+pos+1;

		if (strcmpi(arg,"escape") == 0) {
			arg_escape = val;
			continue;
		}

		if (strcmpi(arg,"file") == 0) {
			arg_filename = val;
			continue;
		}

		if (strcmpi(arg,"line") == 0) {
			arg_line_start = atoi(val);
			continue;
		}

		if (strcmpi(arg,"column") == 0) {
			arg_column_num = atoi(val);
			arg_column     = true;
			continue;
		}

		if (strcmpi(arg,"mode") == 0) {
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
			int modes_size = sizeof(modes) / sizeof(modes[0]);

			for (int j=0; j<modes_size; j++)
				if (strcmpi(val,modes[j]) == 0) {
					arg_edit_mode = j;
					break;
				}
		}

		if (strcmpi(arg,"text") == 0) {
			arg_txt = val;
			continue;
		}

		if (strcmpi(arg,"clip") == 0) {
			arg_clipboard = String2Bool(val);
			continue;
		}

		if (strcmpi(arg,"range") == 0) {
			arg_line_range = atoi(val);
			continue;
		}

		if (strcmpi(arg,"open") == 0) {
			arg_open_mode = val;
			continue;
		}
	};

	
	// File not specified
	if (strcmpi(arg_filename, "") == 0) {
		FWerror(107,0,CommandID,"file name","",0,0,out);
		break;
	};
	
	// Cannot move up the first line
	if (arg_edit_mode==IGSE_WRITE_MOVEUP  &&  arg_line_start==1) {
		FWerror(103,0,CommandID,"arg_line_start","",arg_line_start,0,out);
		break;
	}

	// Verify and update path to the file
	String buf_filename;
	String_init(buf_filename);
	char *ptr_filename = arg_filename;
	
	if (!VerifyPath(&ptr_filename, buf_filename, RESTRICT_TO_MISSION_DIR, CommandID, out))
		break;
	

	// Does clipboard contains text
	HANDLE hClipboardData;

	if (arg_clipboard)
		if (OpenClipboard(NULL)) {
			if (::IsClipboardFormatAvailable(CF_TEXT)) {
				hClipboardData = GetClipboardData(CF_TEXT);
				arg_txt        = (char*)GlobalLock(hClipboardData);
			} else {
				FWerror(21,0,CommandID,"","",0,0,out);
				String_end(buf_filename);
				CloseClipboard(); 
				break;
			}
		} else {
			FWerror(20,GetLastError(),CommandID,"","",0,0,out);
			String_end(buf_filename);
			break;
		}


	// Replace \t and \n
	char tmp[16]	 = "";
	bool numberStart = false;
	int quantity	 = -1;

	for (i=0; arg_escape[i]!='\0'; i++) {
		if (!numberStart  &&  isdigit(arg_escape[i]))
			numberStart = true;

		if (numberStart)
			if (!isdigit(arg_escape[i])) {
				numberStart = false;
				quantity    = atoi(tmp);
				strcpy(tmp, "");
			} else
				sprintf(tmp, "%s%c", tmp, arg_escape[i]);
		
		if (arg_escape[i] == 't') {
			arg_txt  = EscSequences(arg_txt, 0, quantity);
			quantity = -1;
		}

		if (arg_escape[i] == 'n') {
			arg_txt  = EscSequences(arg_txt, 2, quantity);
			quantity = -1;
		}
	};


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

	if (arg_line_start==0  &&  strcmpi(arg_open_mode,"check")!=0) {
		strcpy(open_mode, "ab");

		if (strcmpi(arg_open_mode,"recreate") == 0)
			strcpy(open_mode, "wb");
	};

	FILE *file = fopen(ptr_filename, open_mode);
	if (!file) {
		FWerror(7,errno,CommandID,ptr_filename,"",0,0,out);
		String_end(buf_filename);
		break;
	};

	if (strcmpi(arg_open_mode,"unique") == 0) {
		FWerror(207,0,CommandID,ptr_filename,"",0,0,out);
		String_end(buf_filename);
		fclose(file);
		break;
	};


	// Find file size
	fseek(file, 0, SEEK_END);
	int file_size = ftell(file);
	int original  = ftell(file);

		// Quick append mode
		if (arg_line_start == 0) {
			if (strcmpi(arg_open_mode,"check") == 0)
				freopen(ptr_filename, "ab", file);
			
			if (file_size>0  &&  arg_edit_mode != IGSE_WRITE_APPEND) 
				fprintf(file, "\r\n");

			fprintf(file, "%s", arg_txt);

			if (ferror(file))
				FWerror(7,errno,CommandID,ptr_filename,"",0,0,out);
			else
				FWerror(0,0,CommandID,"","",0,0,out);

			String_end(buf_filename);
			fclose(file);
			break;
		};


	// Allocate buffer
	int new_line_len  = strlen(arg_txt);
	int new_file_size = file_size * (arg_edit_mode==IGSE_WRITE_COPY ? 2 : 1) + new_line_len + 2;
	int new_file_end  = 0;
	int result        = 0;

	String file_contents;
	String_init(file_contents);
	
	result = String_allocate(file_contents, new_file_size);
	if (result != 0) {
		FWerror(10,0,CommandID,"file_contents","",new_file_size,0,out);
		String_end(buf_filename);
		break;
	}

	// Copy text to buffer
	fseek(file, 0, SEEK_SET);
	result = fread(file_contents.pointer, 1, file_size, file);
	file_contents.pointer[file_size] = '\0';

	if (result != file_size) {
		String_end(buf_filename);
		String_end(file_contents);
		
		if (ferror(file))
			FWerror(7,errno,CommandID,ptr_filename,"",0,0,out); 
		else
			FWerror(209,0,CommandID,ptr_filename,"",result,file_size,out);

		fclose(file);
		break;
	}

	fclose(file);



	// Set vars for parsing text
	char *buffer               = file_contents.pointer;
	char *new_line             = arg_txt;
	int line_num               = 0;
	int direction              = arg_line_start > 0 ? 1 : -1;
	int line_start_pos         = direction>0 ? 0 : file_size;
	int last_new_line          = file_size;
	bool took_action           = false;
	int line_shift_dest_pos    = 0;
	int line_shift_source_pos  = 0;
	char *line_shift_dest;
	char *line_shift_source;

	for (i=direction>0 ? 0 : file_size;  direction>0 && i<=file_size || direction<0 && i>=0;  i+=direction) {
		if (buffer[i]=='\n'  ||  direction>0 && i==file_size  ||  direction<0  && i==0) {
			int *separator        = direction>0 ? &i : &last_new_line;
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
				
				int line_len     = carriage_return ? *separator-line_start_pos-1 : *separator-line_start_pos;
				int column_num   = arg_column_num;
				
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

				line_len         = carriage_return ? *separator-line_start_pos-1 : *separator-line_start_pos;				
				int buffer_shift = 0;
				
				if (arg_edit_mode == IGSE_WRITE_REPLACE)
					buffer_shift = new_line_len - line_len;
					
				char *line_end   = line_start + line_len;
				int line_end_pos = carriage_return ? *separator-1 : *separator;
				
				if (arg_edit_mode == IGSE_WRITE_APPEND  ||  arg_edit_mode == IGSE_WRITE_NEW  ||  arg_edit_mode == IGSE_WRITE_INSERT) {
					buffer_shift = new_line_len;
					line_end     = line_start;
					line_end_pos = line_start_pos;
					
					if (arg_edit_mode == IGSE_WRITE_NEW  ||  arg_edit_mode == IGSE_WRITE_INSERT) {
						buffer_shift += 2;
						
						if (arg_edit_mode == IGSE_WRITE_NEW && arg_column)
							buffer_shift += 2;
					}
				}
				
				if (arg_edit_mode == IGSE_WRITE_COPY) {
					buffer_shift = line_len + 2;
					line_end     = line_start;
					line_end_pos = line_start_pos;
					memcpy(line_end+buffer_shift, line_end, file_size-line_end_pos);
					memcpy(line_start+line_len, "\r\n", 2);
				}
				
				if (arg_edit_mode == IGSE_WRITE_DELETE  ||  arg_edit_mode == IGSE_WRITE_CLEAR) {
					buffer_shift = -line_len;
					
					if (arg_edit_mode == IGSE_WRITE_DELETE) {
						revert_separator = false;
						
						if (prev == '\n') {
							line_end     += (carriage_return ? 2 : 1);
							line_end_pos += (carriage_return ? 2 : 1);
							buffer_shift -= (carriage_return ? 2 : 1);
						} else {
							// If last line then we need to remove previous \n
							if (line_start_pos>0  &&  buffer[line_start_pos-1]=='\n')
								buffer_shift--;
								
							if (line_start_pos>1  &&  buffer[line_start_pos-2]=='\r')
								buffer_shift--;
						}
					}
					
					memcpy(line_end+buffer_shift, line_end, file_size-line_end_pos);
				}
				
				if (arg_edit_mode == IGSE_WRITE_REPLACE  ||  arg_edit_mode == IGSE_WRITE_APPEND  ||  arg_edit_mode == IGSE_WRITE_NEW  ||  arg_edit_mode == IGSE_WRITE_INSERT) {
					memcpy(line_end+buffer_shift, line_end, file_size-line_end_pos);

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
					revert_separator  = false;
					
					// line_end is changed to include \r\n
					char *line_end2   = buffer + *separator;
					int line_end2_pos = *separator;
					
					// turn \0 back to \r\n
					buffer[line_end2_pos] = prev;
					if (carriage_return) buffer[line_end2_pos-1] = '\r';
					
					// If current line ends without \r\n then copy \r\n from the previous line
					if (prev == '\0') {
						int offset = 0;
						if (line_start_pos > 0 && buffer[line_start_pos-1] == '\n') offset++;
						if (line_start_pos > 1 && buffer[line_start_pos-2] == '\r') offset++;
						
						memcpy(line_shift_dest+2, line_shift_dest, (line_start_pos-offset)-line_shift_dest_pos);
						
						char to_copy[] = "\r\n";
						memcpy(line_shift_dest, to_copy+2-offset, offset);
						
						line_end2_pos -= 1;
						line_end2      = buffer + line_end2_pos;						
					}
					
					int shifts = line_end_pos - line_shift_source_pos;
					if (buffer[line_end2_pos] == '\n') shifts++;
					if (i>0 && buffer[line_end2_pos-1] == '\r') shifts++;
										
					// Copy characters from the end of the current line to the beginning of the previous line
					for (int j=0; j<shifts; j++) {
						char temp = buffer[line_end2_pos];
						memcpy(line_shift_dest+1, line_shift_dest, line_end2_pos-line_shift_dest_pos);
						buffer[line_shift_dest_pos] = temp;
					}
				}

				*separator       += buffer_shift;
				file_size        += buffer_shift;
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
		// Rewrite the arg_filename
		file = fopen(ptr_filename, "wb");
		if (file) {
			result = fwrite(buffer, 1, file_size, file);

			if (result != file_size) {
				if (ferror(file))
					FWerror(7,errno,CommandID,ptr_filename,"",0,0,out);
				else
					FWerror(210,0,CommandID,ptr_filename,"",result,file_size,out);
			} else
				FWerror(0,0,CommandID,"","",0,0,out);

			fclose(file);
		}
		else 
			FWerror(7,errno,CommandID,ptr_filename,"",0,0,out);
	} else 		
		FWerror(204,0,CommandID,ptr_filename,"",abs(line_num) < abs(arg_line_start) ? arg_line_start : line_range_end,0,out);

	String_end(file_contents);
	String_end(buf_filename);
};
break;












case C_IGSE_LIST:
{ // IGSE List of files

	// Read arguments------------------------------------------------------------
	bool arg_system_time = false;
	int arg_edit_mode    = FILENAMES_AND_ATTRIBUTES;
	char *arg_path       = "*";

	for (int i=2; i<numP; i++) {
		char *arg = stripq(par[i]);
		char *col = strchr(arg, ':');

		if (col == NULL) 
			continue;

		int pos   = col - arg;
		arg[pos]  = '\0';
		char *val = arg+pos+1;

		if (strcmpi(arg,"systemtime") == 0) {
			arg_system_time = String2Bool(val);
			continue;
		}

		if (strcmpi(arg,"onlyname") == 0) {
			if (String2Bool(val))
				arg_edit_mode = FILENAMES_ONLY;
			continue;
		}

		if (strcmpi(arg,"path") == 0) {
			arg_path = val;
			continue;
		}
	}


	// Verify and update path to the file
	String buf_path;
	String_init(buf_path);
	char *ptr_path = arg_path;
	
	if (!VerifyPath(&ptr_path, buf_path, ALLOW_GAME_ROOT_DIR, CommandID, out)) {
		QWrite("[],[]]", out); 
		break;
	}


	// Get list of files
	WIN32_FIND_DATA fd;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	hFind		 = FindFirstFile(ptr_path, &fd);

	if (hFind != INVALID_HANDLE_VALUE) {
		String Names, Attributes;
		String_init(Names);
		String_init(Attributes);

		do {
			if (strcmp(fd.cFileName,".")==0  ||  strcmp(fd.cFileName,"..")==0)
				continue;

			String_append_quotes(Names, "]+[\"", fd.cFileName, "\"");		

			if (arg_edit_mode == FILENAMES_AND_ATTRIBUTES) {
				char data[1024]	= "";
				getAttributes(fd, data, arg_system_time);
				String_append(Attributes,"]+[");
				String_append(Attributes, data);
			}
		} while (FindNextFile(hFind, &fd));
		FindClose(hFind);

		FWerror(0,0,CommandID,"","",0,0,out);
		QWrite("[", out);
		QWrite(Names.pointer, out);
		QWrite("],[", out);
		QWrite(Attributes.pointer, out);
		QWrite("]]", out);

		String_end(Names);
		String_end(Attributes);
	} else {
		FWerror(5,GetLastError(),CommandID,ptr_path,"",0,0,out);
		QWrite("[],[]]",out); 
		break;
	}

	String_end(buf_path);
}
break;








case C_IGSE_LOAD:
{ // load lines from textfile

	// Read arguments ---------------------------------------------------------------
	int arg_line_start        = 0;
	int arg_line_end          = 0;
	int arg_file_offset_num   = 0;
	int arg_limit_line_length = -1;
	int arg_clipboard         = 0;
	int arg_wait              = 0;
	bool arg_file_offset      = false;
	bool Cut                  = false;
	bool arg_execute_output   = false;
	bool arg_delete_file      = false;
	bool arg_split_lines      = false;
	bool arg_clip_append      = false;
	char *arg_filename        = "";
			
	for (int i=1; i<numP; i++) {
		char *arg = stripq(par[i]);
		char *col = strchr(arg, ':');

		if (col == NULL) 
			continue;

		int pos   = col - arg;
		arg[pos]  = '\0';
		char *val = arg+pos+1;

		if (strcmpi(arg,"file") == 0) {
			arg_filename = val;
			continue;
		}

		if (strcmpi(arg,"start") == 0) {
			arg_line_start = atoi(val);
			continue;
		}

		if (strcmpi(arg,"end") == 0) {
			arg_line_end = atoi(val);
			continue;
		}

		if (strcmpi(arg,"offset") == 0) {
			arg_file_offset_num = atoi(val);
			arg_file_offset     = true;
			continue;
		}
			
		if (strcmpi(arg,"cut") == 0) {
			Cut                   = true;
			arg_limit_line_length = atoi(val);
			continue;
		}

		if (strcmpi(arg,"split") == 0) {
			arg_split_lines = String2Bool(val);
			continue;
		}

		if (strcmpi(arg,"mode") == 0) {
			if (strcmpi(val,"execute") == 0) {
				arg_execute_output = true;
				SuppressNextError  = true;
				continue;
			}

			if (strcmpi(val,"clipcopy") == 0) {
				arg_clipboard   = 1;
				arg_clip_append = false;
				continue;
			}

			if (strcmpi(val,"clipappend") == 0) {
				arg_clipboard   = 1,
				arg_clip_append = true;
				continue;
			}
		}

		if (strcmpi(arg,"delete") == 0) {
			arg_delete_file = String2Bool(val);
			continue;
		}

		if (strcmpi(arg,"wait") == 0) {
			arg_wait = atoi(val);
			continue;
		}
	};


	// File not specified
	if (IsWhiteSpace(arg_filename)) {
		FWerror(107,0,CommandID,"arg_filename","",0,0,out);
		
		if (!arg_execute_output)
			QWrite("[],0,false,false,\"\"]", out);

		break;
	};

	// Number validation
	if (arg_line_start < 0) 
		arg_line_start = 0;

	if (arg_line_end < 0) 
		arg_line_end = 0;

	if (arg_line_start>arg_line_end  &&  arg_line_end>0) {
		FWerror(104,0,CommandID,"","",arg_line_start,arg_line_end,out);
		
		if (!arg_execute_output)
			QWrite("[],0,false,false,\"\"]", out);

		break;
	};


	// Verify and update path to the file
	String buf_filename;
	String_init(buf_filename);
	char *ptr_filename = arg_filename;
	int path_type      = VerifyPath(&ptr_filename, buf_filename, ALLOW_GAME_ROOT_DIR, CommandID, out);

	if (path_type == ILLEGAL_PATH) {
		QWrite("[],0,false,false,\"\"]", out);
		break;
	}
	// ------------------------------------------------------------------------------



	// Read arg_filename --------------------------------------------------------------------
	FILE *file;
	
	// Optional arg_edit_mode - wait for the arg_filename to appear
	do {
		file = fopen(ptr_filename, "r");
		if (!file  &&  arg_wait>0) {
			Sleep(10);
			arg_wait--;
		}
	} while (!file  &&  arg_wait>0);


	// If couldn't open
	if (!file) {
		FWerror(7,errno,CommandID,ptr_filename,"",0,0,out);
		
		if (!arg_execute_output)
			QWrite("[],0,false,false,\"\"]", out);
		else
			QWrite("\"\"", out);

		String_end(buf_filename);
		break;
	};


	// Create variables
	int CurrentROW       = 1;
	int error            = 0;
	int single_line_size = 512;
	int lastPos          = 0;
	int len              = 0;
	int LineCount        = 0;
	int clipLEN          = 0;
	bool isEOL           = false;
	char *single_line    = (char*) malloc(single_line_size);
	char *lineNEW;
	char *ret;
	char t[128]          = "";
	char *clipBUF        = NULL;
	char *clipNEW        = NULL;

	if (single_line == NULL) {
		FWerror(10,0,CommandID,"single_line","",single_line_size,0,out);
		
		if (!arg_execute_output)
			QWrite("[],0,false,false,\"\"]", out);

		String_end(buf_filename);
		break;
	};


	// Start reading file from the given position
	if (arg_file_offset) {
		CurrentROW = arg_line_start;
		lastPos    = arg_file_offset_num;
		fseek(file, arg_file_offset_num, SEEK_SET);
	}


	// Assume no error
	if (!arg_execute_output  &&  !arg_clipboard)
		QWrite("[true,0,0,\"\",[",out);


	// Go through text
	while (!feof(file) && (CurrentROW<=arg_line_end && arg_line_end!=0 || arg_line_end==0)) {
		ret = fgets(single_line, single_line_size ,file);

		if (ferror(file)) {
			if (!arg_execute_output)
				QWrite("]];", out); 

			FWerror(7,errno,CommandID,ptr_filename,"",0,0,out);
			error = 1;
		};

		if (error) 
			break;

		if (ret == NULL) 
			strcpy(single_line, "");

		if (isEOL = single_line[strlen(single_line)-1] == '\n') 
			single_line[strlen(single_line)-1] = '\0';

		len = strlen(single_line);


		// If a long single_line then reallocate buffer and read again
		if (!isEOL  &&  !feof(file)  &&  ret!=NULL) {
			single_line_size += 512;
			lineNEW           = (char*) realloc (single_line, single_line_size);

			if (lineNEW != NULL) 
				single_line = lineNEW; 
			else {
				if (!arg_execute_output)
					QWrite("]];", out);

				FWerror(11,0,CommandID,"single_line","",single_line_size,0,out);
				error = 1; 
				break;
			};

			fseek(file, lastPos, SEEK_SET);
			continue;
		};


		// If current single_line number is in range specified by the user
		if (CurrentROW>=arg_line_start  &&  (arg_line_end>0 && CurrentROW<=arg_line_end || arg_line_end==0 && CurrentROW>=arg_line_start)  &&  arg_limit_line_length!=0) {
			LineCount++;

			// Default arg_edit_mode - return text
			if (!arg_execute_output  &&  !arg_clipboard) {
				int i             = 0;
				int cut           = arg_limit_line_length>0 ? arg_limit_line_length : len;
				char *backup_line = single_line;


				// If we segment lines then add additional array
				if (!arg_split_lines) 
					QWrite("]+[[", out); 
				else 
					QWrite("]+[[[", out);


				// If empty single_line then output empty string
				if (len == 0) 
					if (arg_split_lines) 
						QWrite("\"\"", out); 
					else 
						QWrite("\"", out);


				// Split single_line (if optional arg_edit_mode) and return it
				for (; len>0 && (i<len && arg_split_lines || !arg_split_lines && i==0);  i+=cut, backup_line=single_line+i) {
					QWrite("]+[\"", out);
					
					// Split single_line
					int l2 = cut > l2 ? l2 : strlen(backup_line);

					char prev = backup_line[cut];
					if (Cut) 
						backup_line[cut] = '\0';


					// Double the amount of quotation marks
					if (strchr(backup_line,'\"')) {
						char *rep = str_replace(backup_line,"\"","\"\"",0,0);

						if (rep != NULL) {
							QWrite(rep, out);
							free(rep); 
						}
						else {
							if (!arg_split_lines) 
								QWrite("\"]]];",out); 
							else 
								QWrite("\"]]]];",out);

							FWerror(12,0,CommandID,"backup_line (' with '')","",strlen(backup_line),0,out);
							error = 1; 
							break;
						};
					}
					else 
						QWrite(backup_line,out);

					if (arg_split_lines) 
						QWrite("\"",out);

					backup_line[cut] = prev;
				};

				if (error) 
					break;


				// Return additional information
				if (!arg_split_lines) 
					strcpy(t,"\""); 
				else 
					strcpy(t, "]");

				sprintf(t, "%s,\"%d\",%d]", t, lastPos, len);
				QWrite(t, out);
			} else 
				// Copy to clip instead
				if (!arg_execute_output && arg_clipboard) {
					clipLEN += len + 2;
					
					// Copy lines to a temporary buffer
					if (arg_clipboard == 1) {
						clipBUF = (char*) malloc (clipLEN);

						if (clipBUF == NULL) {
							error = 1;
							break;
						};

						strcpy(clipBUF, "");
					} else {
						clipLEN += len + 2;
						clipNEW  = (char*) realloc (clipBUF, clipLEN);

						if (clipNEW != NULL) 
							clipBUF = clipNEW; 
						else {
							error = 1;
							free(clipBUF);
							break;
						};
					};

					strcat(clipBUF, single_line);

					if (isEOL)
						strcat(clipBUF, "\n");

					arg_clipboard++;
				}
			else 
				// Return line itself instead
				if (arg_execute_output)		
					QWrite(single_line, out);
		};

		lastPos = ftell(file);
		CurrentROW++;
	};


	// Copy from a temporary buffer to the arg_clipboard
	if (arg_clipboard  &&  !error) {
		if (!CopyToClip(clipBUF, arg_clip_append, CommandID, out)) {
			error = 1;
			String_end(buf_filename);
			free(clipBUF);
			free(single_line);
			break;
		};

		free(clipBUF);
	};


	int end = feof(file);

	// If couldn't find wanted line
	if (arg_line_start>0  &&  --CurrentROW<arg_line_start) {
		if (!arg_execute_output)
			QWrite("]];", out);

		FWerror(204,0,CommandID,ptr_filename,"",arg_line_start,0,out);
		error = 1;
	};

	free(single_line);
	fclose(file);
	// ------------------------------------------------------------------------------


	// Return additional data
	if (!arg_execute_output  &&  !arg_clipboard  &&  !error)
		QWrite("],", out);

	if (arg_clipboard)
		if (!error)
			QWrite("[true,0,0,\"\",[],",out);
		else
			QWrite("[],",out);

	sprintf(t, "%d,%s,%s,\"%d\"]", LineCount, getBool(len==0), getBool(end), lastPos);

	if (!arg_execute_output  ||  arg_execute_output && error)
		QWrite(t, out);


	// If user wants to delete the arg_filename after reading it
	if (arg_delete_file)
		if (path_type == DOWNLOAD_DIR)
			remove(ptr_filename);
		else
			trashFile(ptr_filename,-1,NULL,0);

	String_end(buf_filename);
	SuppressNextError = false;
}
break;









case C_IGSE_NEWFILE:
{ // IGSE New File

	// Read arguments
	char *arg_filename  = "";
	int arg_edit_mode   = IGSE_NEWFILE_CREATE;
	bool arg_unique     = false;
	bool arg_subdirs    = false;
	bool arg_directory  = false;

	for (int i=2; i<numP; i++) {
		char *arg = stripq(par[i]);
		char *col = strchr(arg, ':');

		if (col == NULL) 
			continue;

		int pos = col - arg;
		arg[pos] = '\0';
		char *val = arg + pos + 1;

		if (strcmpi(arg,"file") == 0) {
			arg_filename = val;
			continue;
		}

		if (strcmpi(arg,"mode") == 0) {		
			if (strcmpi(val,"create") == 0) {
				arg_edit_mode = IGSE_NEWFILE_CREATE;
				continue;
			}
			
			if (strcmpi(val,"delete") == 0) {
				arg_edit_mode = IGSE_NEWFILE_DELETE;
				continue;
			}
			
			if (strcmpi(val,"recreate") == 0) {
				arg_edit_mode = IGSE_NEWFILE_RECREATE;
				continue;
			}
			
			if (strcmpi(val,"check") == 0) {
				arg_edit_mode = IGSE_NEWFILE_CHECK;
				continue;
			}
		}

		if (strcmpi(arg,"unique") == 0) {
			arg_unique = String2Bool(val);
			continue;
		}

		if (strcmpi(arg,"subdirs") == 0) {
			arg_subdirs = String2Bool(val);
			continue;
		}

		if (strcmpi(arg,"directory") == 0) {
			arg_directory = String2Bool(val);
			continue;
		}
	};


	// File not specified
	if (strcmpi(arg_filename, "") == 0) {
		FWerror(107,0,CommandID,"arg_filename","",0,0,out);
		break;
	};


	// Verify and update path to the file
	String buf_filename;
	String_init(buf_filename);
	char *ptr_filename = arg_filename;
	int check_mode     = arg_edit_mode==IGSE_NEWFILE_CHECK ? ALLOW_GAME_ROOT_DIR : RESTRICT_TO_MISSION_DIR;
	int path_type      = VerifyPath(&ptr_filename, buf_filename, check_mode, CommandID, out);
	
	if (path_type == ILLEGAL_PATH)
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

		char *slash = "";
		char *path  = ptr_filename;

		// For each arg_directory along the way
		while (slash = strchr(path,'\\')) {
			// Separate name from the rest of the string
			int pos   = slash - path;
			char prev = path[pos];
			path[pos] = '\0';

			int result = CreateDirectory(ptr_filename, NULL);

			// Move on to the next
			path[pos] = prev;
			path     += pos+1;
		};
	};


	// See if the file is already there
	if (CheckIt) {		
		WIN32_FILE_ATTRIBUTE_DATA fad;
		if (GetFileAttributesEx(ptr_filename, GetFileExInfoStandard, &fad)) {
			ItExists = (fad.dwFileAttributes & 16) ? 2 : 1;

			if (arg_edit_mode == IGSE_NEWFILE_CHECK)
				if (!arg_directory)
					FWerror(0,0,CommandID,"","",0,0,out);
				else
					if (ItExists != 2)
						FWerror(201,0,CommandID,ptr_filename,"",0,0,out);
		} else
			if (arg_edit_mode == IGSE_NEWFILE_CHECK)
				FWerror(5,GetLastError(),CommandID,ptr_filename,"",0,0,out);
	};


	// Trash or delete
	if (DeleteIt) {
		// if the file doesn't exist then ignore error so that we can't delete it
		int ErrorBehaviour = !ItExists ? 1 : 0;

		// Default - trash it
		if (path_type != DOWNLOAD_DIR) {
			if (trashFile(ptr_filename,CommandID,out,ErrorBehaviour))
				FWerror(0,0,CommandID,"","",0,0,out);
			else
				CreateIt = false;
		} else {		
			// ..\fwatch\tmp - delete it
			int result = DeleteWrapper(ptr_filename);

			if (result == 0)
				FWerror(0,0,CommandID,"","",0,0,out);
			else
				if (arg_edit_mode != IGSE_NEWFILE_RECREATE) {
					CreateIt = false;
					FWerror(5,result,CommandID,ptr_filename,"",0,0,out);
				}
		}
	};



	// Create new
	if (CreateIt) {
		// If we want error when it already exists
		if (arg_unique && ItExists  ||  arg_directory && ItExists==1) {
			if (arg_directory  &&  ItExists==2)
				FWerror(208,0,CommandID,ptr_filename,"",0,0,out);
			else
				FWerror(207,0,CommandID,ptr_filename,"",0,0,out);
		} else
			// arg_filename
			if (!arg_directory) {
				FILE *file = fopen(ptr_filename, "a");

				if (file) {
					FWerror(0,0,CommandID,"","",0,0,out);
					fclose(file);
				} else
					FWerror(7,errno,CommandID,ptr_filename,"",0,0,out);
			}
			// folder
			else
				if (CreateDirectory(ptr_filename, NULL)) 
					FWerror(0,0,CommandID,"","",0,0,out);
				else 
					// ignore error that dir already exists
					if (GetLastError() == 183)
						FWerror(0,0,CommandID,"","",0,0,out);
					else
						FWerror(5,GetLastError(),CommandID,"","",0,0,out);
	};

	String_end(buf_filename);
}
break;









case C_IGSE_RENAME:
case C_IGSE_COPY:
{ // IGSE Rename/Copy

	// Read arguments
	char *arg_source   = "";
	char *arg_dest     = "";
	bool arg_overwrite = false;

	for (int i=2; i<numP; i++) {
		char *arg = stripq(par[i]);
		char *col = strchr(arg, ':');

		if (col == NULL) 
			continue;

		int pos   = col - arg;
		arg[pos]  = '\0';
		char *val = arg+pos+1;

		if (strcmpi(arg,"source") == 0) {
			arg_source = val;
			continue;
		}

		if (strcmpi(arg,"destination") == 0) {
			arg_dest = val;
			continue;
		}

		if (strcmpi(arg,"overwrite") == 0) {
			arg_overwrite = String2Bool(val);
			continue;
		}
	};


	// Files not specified
	if (strcmpi(arg_source, "") == 0) {
		FWerror(107,0,CommandID,"source","",0,0,out);
		break;
	};

	if (strcmpi(arg_dest, "") == 0) {
		FWerror(107,0,CommandID,"destination","",0,0,out);
		break;
	};


	// Verify and update path to the files
	String source, dest;
	String_init(source);
	String_init(dest);
	char *ptr_source     = arg_source;
	char *ptr_dest       = arg_dest;
	int check_mode       = CommandID==C_IGSE_RENAME ? RESTRICT_TO_MISSION_DIR : ALLOW_GAME_ROOT_DIR;
	int source_path_type = VerifyPath(&ptr_source, source, check_mode, CommandID, out);
	int dest_path_type   = VerifyPath(&ptr_dest, dest, RESTRICT_TO_MISSION_DIR, CommandID, out);

	if (!source_path_type || !dest_path_type)
		break;

	// Rename
	if (CommandID == C_IGSE_RENAME) {
		if (source_path_type!=DOWNLOAD_DIR  &&  dest_path_type==DOWNLOAD_DIR)	// Not allowed to move files to the download directory
			FWerror(202,0,CommandID,ptr_source,"",0,0,out);
		else {
			if (rename(ptr_source,ptr_dest) == 0) 
				FWerror(0,0,CommandID,"","",0,0,out);
			else 
				FWerror(7,errno,CommandID,ptr_source,ptr_dest,0,0,out);
		};
	// Copy
	} else {
		if (arg_overwrite  &&  dest_path_type!=DOWNLOAD_DIR)	// Overwrite means trashing before copying
			trashFile(ptr_dest,CommandID,out,0);

		if (CopyFile((LPCTSTR)ptr_source, (LPCTSTR)ptr_dest, dest_path_type!=DOWNLOAD_DIR)) 
			FWerror(0,0,CommandID,"","",0,0,out);
		else 
			FWerror(5,GetLastError(),CommandID,ptr_source,ptr_dest,0,0,out);
	};


	String_end(source);
	String_end(dest);
}
break;









case C_IGSE_FIND:
{ // IGSE Find

	// Read arguments -----------------------------------------------------------
	// Assign passed params to vars
	int Start			= 0;
	int End				= 0;
	int StartPos		= 0;
	int Limit			= 0;
	int arg_col_num     = -1;
	int WantedROW		= 0;
	int WantedPos		= 0;
	char *SearchIn		= "";
	char *SearchFor		= "";
	char *ReplaceWith	= "";
	bool PassedStartPos = false;
	bool Replace		= false;
	bool caseSensitive	= false;
	bool matchWord		= false;
	bool ignoreLeadSpace= false;

	for (int i=1; i<numP; i++)
	{
		char *arg = stripq(par[i]);
		pch = strchr(arg, ':');

		if (pch == NULL) 
			continue;

		int pos = pch - arg;
		arg[pos] = '\0';
		char *val = arg + pos + 1;

		if (strcmpi(arg,"file") == 0) 
			SearchIn = Trim(val);

		if (strcmpi(arg,"start") == 0) 
			Start = atoi(val);

		if (strcmpi(arg,"end") == 0) 
			End = atoi(val);

		if (strcmpi(arg,"offset") == 0) 
			StartPos = atoi(val),
			PassedStartPos = true; 
			
		if (strcmpi(arg,"limit") == 0) 
			Limit = atoi(val);

		if (strcmpi(arg,"text") == 0) 
			SearchFor = val;

		if (strcmpi(arg,"replace") == 0) 
			Replace = true, 
			ReplaceWith = val;

		if (strcmpi(arg,"column") == 0) 
			arg_col_num = atoi(val);

		if (strcmpi(arg,"ignoreLeadSpace") == 0)
			ignoreLeadSpace = String2Bool(val);

		if (strcmpi(arg,"caseSensitive") == 0)
			caseSensitive = String2Bool(val);

		if (strcmpi(arg,"matchWord") == 0)
			matchWord = String2Bool(val);
	};


	// Number validation
	if (Start < 0) 
		Start = 0;

	if (End < 0) 
		End = 0;

	if (Limit < 0) 
		Limit = 0;

	if (Start>End  &&  End>0) 
	{
		FWerror(104,0,CommandID,"","",Start,End,out);
		QWrite("0,[],[],\"0\",0]", out);
		break;
	};



	// File not specified
	if (strcmpi(SearchIn,"") == 0)
	{
		FWerror(107,0,CommandID,"file","",0,0,out);
		QWrite("0,[],[],\"0\",0]", out);
		break;
	};


	// Search target not specified
	if (strcmpi(SearchFor,"") == 0)
	{
		FWerror(107,0,CommandID,"text","",0,0,out);
		QWrite("0,[],[],\"0\",0]", out);
		break;
	};


	// Verify and update path to the file
	String buf_filename;
	String_init(buf_filename);
	char *ptr_filename = SearchIn;

	if (!VerifyPath(&ptr_filename, buf_filename, ALLOW_GAME_ROOT_DIR, CommandID, out)) {
		QWrite("0,[],[],\"0\",0]", out);
		break;
	}
	//-------------------------------------------------------------------------



	// Open file ------------------------------------------------------------------------
	FILE *f = fopen(ptr_filename, "r");
	if (!f) 
	{
		FWerror(7,errno,CommandID,ptr_filename,"",0,0,out);
		QWrite("0,[],[],\"0\",0]", out);
		String_end(buf_filename);
		break;
	};


	// Set up vars and allocate buffers
	int single_line_size		= 512;
	int rowsLen		= 20;
	int colsLen		= 20;
	int matches		= 0;
	int lastPos		= 0;
	int lastOccOff	= 0;
	int lastOffset  = -1;
	int CurrentROW	= 1;
	bool isEOL		= false; 
	bool error		= false;

	char *single_line		= (char*) malloc (single_line_size);
	char *rows		= (char*) malloc (rowsLen);
	char *cols		= (char*) malloc (colsLen);
	char *lineNEW;
	char *rowsNEW; 
	char *colsNEW; 
	char *ret;
	char *p;
	char tmp[128]	= "";


	// If failed to allocate
	if (single_line==NULL  ||  rows==NULL  ||  cols==NULL) 
	{
		char failedBuf[20]	= "";
		int failedBufL		= 0;
		
		if (single_line == NULL) 
			strcat(failedBuf,"single_line "), 
			failedBufL += single_line_size;
		else
			free(single_line); 

		if (rows == NULL) 
			strcat(failedBuf,"rows "), 
			failedBufL += rowsLen;
		else
			free(rows); 

		if (cols == NULL) 
			strcat(failedBuf,"cols "), 
			failedBufL += colsLen;
		else
			free(cols); 

		FWerror(10,0,CommandID,failedBuf,"",failedBufL,0,out);
		QWrite("0,[],[],\"0\",0]", out);
		String_end(buf_filename);
		break;
	};

	strcpy(rows, "["); 
	strcpy(cols, "[");


	// If this is a replace operation then allocate buffer for the whole file
	int FileBufPos	= 0;
	int FileBufSize = 0;
	char *FileBuf	= "";
	bool Replaced	= false;

	if (Replace)
	{
		fseek(f, 0, SEEK_END);
		FileBufSize = ftell(f);
		FileBuf = (char*) malloc (FileBufSize);

		if (FileBuf == NULL)
		{
			FWerror(10,0,CommandID,"FileBuf ","",FileBufSize,0,out);
			QWrite("0,[],[],\"0\",0]", out);
			free(single_line); 
			free(rows); 
			free(cols); 
			String_end(buf_filename);
			break;
		};

		fseek(f, 0, SEEK_SET);
	};


	// Start reading file from the given position
	if (PassedStartPos)
		CurrentROW = Start, 
		fseek(f, StartPos, SEEK_SET), 
		lastPos    = StartPos, 
		lastOccOff = StartPos;
	//---------------------------------------------------------------------------


	// Iterate lines ------------------------------------------------------------
	while (!feof(f)  &&  (CurrentROW<=End && End!=0 && !Replace || End==0 || Replace))
	{
		ret = fgets(single_line, single_line_size, f);

		if (ferror(f)) 
			FWerror(7,errno,CommandID,ptr_filename,"",0,0,out), 
			error = 1;

		if (ret == NULL) 
			strcpy(single_line, "");

		isEOL = single_line[strlen(single_line)-1] == '\n';
		

		// If a long single_line then reallocate buffer and read again
		if (!isEOL && !feof(f)  &&  ret!=NULL)
		{
			single_line_size += 512;
			lineNEW = (char*) realloc (single_line, single_line_size);

			if (lineNEW != NULL) 
				single_line = lineNEW; 
			else 
			{	
				FWerror(11,0,CommandID,"single_line","",single_line_size,0,out);
				error = 1; 
				break;
			};

			fseek (f, lastPos, SEEK_SET);
			continue;
		};


		// If current single_line number is in range
		if (CurrentROW>=Start  &&  (End>0 && CurrentROW<=End || End==0 && CurrentROW>=Start))
		{
			char *backup_line		= single_line;
			int i			= 0;
			int CurrentCOL	= 0;
			int maxIterations = 0;
			int leadSpace	= 0;

			// If we want to ignore leading whitespace then count it
			while (ignoreLeadSpace  &&  isspace(single_line[leadSpace])  &&  single_line[leadSpace]!='\0')
				leadSpace++;

			// Find all positions of the occurences in the current single_line
			while (p = strstr2(backup_line, SearchFor, matchWord, caseSensitive))
			{
				maxIterations++;
				if (maxIterations > 10)
					break;
				int pos = p - backup_line;

				// If no limit regarding columns or if occurence is on the wanted column
				if (arg_col_num==-1  ||  arg_col_num!=-1 && arg_col_num == CurrentCOL+pos-leadSpace)
				{
					// Add column number to the return data
					if (i == 0) 
						colsLen += 4, 
						sprintf(tmp, "[%d", CurrentCOL + pos); 
					else 
						sprintf(tmp, "]+[%d", CurrentCOL + pos);

					colsLen += strlen(tmp) + 1;
					colsNEW = (char*) realloc (cols, colsLen);

					if (colsNEW != NULL) 
						cols = colsNEW, 
						strcat(cols, tmp); 
					else 
					{
						FWerror(11,0,CommandID,"cols","",colsLen,0,out);
						error = 1; 
						break;
					};

					i++;
					matches++;
				};

				backup_line = single_line + CurrentCOL + pos + 1;
				CurrentCOL += pos+1;
				lastOccOff = lastPos + CurrentCOL;

				if (Limit!=0  &&  matches>=Limit) 
				{
					lastOffset = lastOccOff;
					break;
				};
			};


			// Add single_line number to array
			if (i > 0)
			{
				strcat(cols, "]]+[");
				sprintf(tmp, "]+[%d", CurrentROW);

				rowsLen += strlen(tmp) + 1;
				rowsNEW = (char*) realloc (rows, rowsLen);

				if (rowsNEW != NULL) 
					rows = rowsNEW, 
					strcat(rows, tmp); 
				else 
				{
					FWerror(11,0,CommandID,"rows","",rowsLen,0,out);
					error = 1; 
					break;
				};
			};


			// Replace occurences
			if (Replace  &&  i>0)
			{
				Replaced  = true;
				char *rep = str_replace(single_line, SearchFor, ReplaceWith, matchWord, caseSensitive);

				if (rep == NULL) 
				{
					FWerror(12,0,CommandID,ReplaceWith,"",strlen(single_line),0,out);
					error = 1; 
					break;
				};


				// If result is larger than original then reallocate buffer for the new file
				unsigned int newLength = strlen(rep);
				unsigned int oldLength = strlen(single_line);

				//if (strlen(single_line) > l)
				if (newLength > oldLength)
				{
					FileBufSize += newLength - oldLength;

					char *FileBufNEW = (char*) realloc (FileBuf, FileBufSize);

					if (FileBufNEW != NULL) 
						FileBuf = FileBufNEW;
					else 
					{
						FWerror(11,0,CommandID,"FileBuf","",rowsLen,0,out);
						error = 1; 
						break;
					};
				};


				// Copy modified single_line to the buffer
				memcpy(FileBuf+FileBufPos, rep, newLength+1),
				FileBufPos += newLength;
				free(rep);
			};
		};

		lastPos = ftell(f);
		CurrentROW++;

		if (Limit!=0  &&  matches>=Limit) 
			break;


		// If this is a replace operation then copy lines to a buffer
		if (Replace)
		{
			if (!Replaced)
			{
				int l = strlen(single_line);
				memcpy(FileBuf+FileBufPos, single_line, l+1);
				FileBufPos += l;
			}
			else
				Replaced = false;
		};
	};


	// If did not stop in the middle of the single_line then point to the next single_line
	if (lastOffset == -1)
	{
		lastOffset = lastPos;

		if (!feof(f))
			CurrentROW++;
	};

	fclose(f); 
	free(single_line);
	//-------------------------------------------------------------------------



	// If this was a replace operation then rewrite the file
	if (Replace  &&  matches>0)
	{
		f = fopen(ptr_filename, "w");
		if (f)
		{
			fwrite(FileBuf, 1, strlen(FileBuf), f);

			if (ferror(f)) 
				error = 1,
				FWerror(7,errno,CommandID,ptr_filename,"",0,0,out);

			fclose(f);
		}
		else 
			error = 1,
			FWerror(7,errno,CommandID,ptr_filename,"",0,0,out);
	};


	// If error
	if (!error)
	{	
		FWerror(0,0,CommandID,"","",0,0,out);
		strcat(rows, "],");
		strcat(cols, "]");
		sprintf(tmp, "%d,", matches);
		QWrite(tmp, out);
		QWrite(rows, out);
		QWrite(cols, out);
		sprintf(tmp, ",\"%d\",%d]", lastOffset, --CurrentROW);
		QWrite(tmp, out);
	}
	else
		QWrite("0,[],[],\"0\",0]", out);


	free(cols); 
	free(rows); 
	String_end(buf_filename);
	
	if (Replace)
		free(FileBuf);
}
break;









case C_IGSE_DB:
{ // IGSE Database
//FILE *fd=fopen("fwatch_debug.txt","a");
//fprintf(fd,"com:%s\n",com);
	// Define variables
	char *arg_filename     = "";
	bool arg_writing_mode  = false;
	int arg_new_keys       = 0;
	int arg_arguments_size = 0;

	// Parse arguments
	for (int i=2; i<numP; i++) {
		char *arg = stripq(par[i]);
		char *col = strchr(arg, ':');

		if (col == NULL) 
			continue;

		int pos	  = col-arg;
		arg[pos]  = '\0';
		char *val = Trim(arg + pos + 1);

		if (strcmpi(arg,"file") == 0) {
			arg_filename = val;
			arg[0]       = IGSEDB_FILE;
			continue;
		}

		if (strcmpi(arg,"key") == 0) {
			arg_new_keys++;	
			arg_arguments_size += strlen(val) + 1;
			arg[0]              = IGSEDB_KEY;
			continue;
		}

		if (strcmpi(arg,"write") == 0) {
			arg_arguments_size += strlen(val) + 1;
			arg_writing_mode    = true;
			arg[0]              = IGSEDB_WRITE;
			continue;
		}

		if (strcmpi(arg,"append") == 0) {
			arg_arguments_size += strlen(val) + 1;
			arg_writing_mode    = true;
			arg[0]              = IGSEDB_APPEND;
			continue;
		}

		if (strcmpi(arg,"rename") == 0) {
			arg_arguments_size += strlen(val) + 1;
			arg_writing_mode    = true;
			arg[0]              = IGSEDB_RENAME;
			continue;
		}

		if (strcmpi(arg,"remove") == 0) {
			arg_writing_mode = true;
			arg[0]           = IGSEDB_REMOVE;
			continue;
		}

		if (strcmpi(arg,"read") == 0) {
			arg[0] = IGSEDB_READ;
			continue;
		}

		if (strcmpi(arg,"list") == 0) {
			arg[0] = IGSEDB_LIST;
			continue;
		}
	}		

	
	// File not specified
	if (strcmpi(arg_filename, "") == 0) {
		FWerror(107,0,CommandID,"file","",0,0,out);
		QWrite(",[],[]]", out);
		break;
	};


	// Verify and update path to the file
	String buf_filename;
	String_init(buf_filename);
	char *ptr_filename = arg_filename;
	
	if (!VerifyPath(&ptr_filename, buf_filename, RESTRICT_TO_MISSION_DIR, CommandID, out)) {
		QWrite(",[],[]]", out);
		break;
	}

	// Get number of keys from the selected arg_filename
	FILE *file;
	int existing_keys             = 0;
	int existing_text_buffer_size = 0;

	if (file = fopen(ptr_filename,"rb"))
		fread(&existing_keys, sizeof(existing_keys), 1, file);
	else 
		if (errno != 2) {
			FWerror(7,errno,CommandID,ptr_filename,"",0,0,out);
			QWrite(",[],[]]", out);
			String_end(buf_filename);
			break;
		}


	// Allocate memory for the data
	int offsets_size = (existing_keys + arg_new_keys + 1) * sizeof(int);
	int *offsets     = (int  *) malloc (offsets_size);

	if (offsets == NULL) {
		FWerror(10,0,CommandID,"offsets","",offsets_size,0,out);
		QWrite(",[],[]]", out);
		String_end(buf_filename);
		break;
	}

	offsets[0] = 0;
	
	if (file) {
		fread(offsets+1, sizeof(*offsets), existing_keys, file);
		existing_text_buffer_size = offsets[existing_keys];
	}

	char *text_buffer = (char *) malloc (existing_text_buffer_size + arg_arguments_size);
	if (text_buffer == NULL) {
		FWerror(10,0,CommandID,"text_buffer","",(existing_text_buffer_size + arg_arguments_size),0,out);
		QWrite(",[],[]]", out);
		String_end(buf_filename);
		free(offsets);
		break;
	}


	// Read data from the arg_filename to the allocated memory
	if (file) {
		fread(text_buffer, 1, existing_text_buffer_size, file);
		fclose(file);
	}


	// Retrace through the arguments and actually execute them
	int final_keys             = existing_keys;
	int final_text_buffer_size = existing_text_buffer_size;
	int key_index              = -1;
	int new_key_size           = 0;
	bool found_key             = false;
	char *key_name;
	char *new_key;
	String MissingKeys, List;
	String_init(MissingKeys);
	String_init(List);


	for (i=2; i<numP; i++) {
		char argumentID = par[i][0];
		char *val       = par[i];

		switch (argumentID) {
			case IGSEDB_FILE   : break;
			case IGSEDB_KEY    : val+=4; break;
			case IGSEDB_WRITE  : val+=6; break;
			case IGSEDB_APPEND : 
			case IGSEDB_RENAME : 
			case IGSEDB_REMOVE : val+=7; break;
			case IGSEDB_READ   : 
			case IGSEDB_LIST   : val+=5; break;
		}

		// Find key
		if (argumentID==IGSEDB_KEY  ||  argumentID==IGSEDB_REMOVE  ||  argumentID==IGSEDB_READ) {
			new_key   = val;
			found_key = false;
			
			for (key_index=final_keys-1; key_index>=0; key_index--) {
				key_name = text_buffer + offsets[key_index];
				
				if (strcmpi(key_name, new_key) == 0) {
					found_key = true;
					break;
				}
			}
		
			if (!found_key  &&  (argumentID==IGSEDB_READ || argumentID==IGSEDB_RENAME)) {
				String_append_quotes(MissingKeys, "]+[\"", new_key, "\"");
				continue;
			}
		}

		// Add new key and value / Replace value / Append value / Rename key
		if (argumentID==IGSEDB_WRITE  ||  argumentID==IGSEDB_APPEND  ||  argumentID==IGSEDB_RENAME) {
			if (found_key) {
				int key_length       = strlen(key_name) + 1;
				int old_value_length = offsets[key_index+1] - offsets[key_index] - key_length - 1;
				int new_value_length = strlen(val);
				int difference       = new_value_length - old_value_length;
				int val_start        = offsets[key_index] + key_length;
				int val_end          = val_start + old_value_length;
				
				if (argumentID == IGSEDB_APPEND)
					difference = new_value_length;
				
				if (argumentID == IGSEDB_RENAME) {
					val_start  = offsets[key_index];
					val_end    = val_start + key_length;
					difference = new_value_length - key_length;
				}

				// Shift current values to make room for the new data
				if (difference != 0) {
					memcpy(text_buffer+val_end+difference,  text_buffer+val_end,  final_text_buffer_size-val_end);
					final_text_buffer_size += difference;
					offsets[final_keys]     = final_text_buffer_size;
				}

				if (argumentID == IGSEDB_APPEND)
					val_start += old_value_length;

				// Write new value
				//fprintf(fd,"difference:%d\ntext_buffer+val_start:%d %s\nval:%s\nnew_value_length:%d\n",difference, val_start, text_buffer+val_start,val, new_value_length);
				memcpy(text_buffer+val_start, val, new_value_length);

				// Recalculate value offsets
				if (difference != 0)
					for (int k=key_index+1; k<final_keys; k++)
						offsets[k] += difference;

			} else 
				if (argumentID != IGSEDB_RENAME) {
					int new_key_length = strlen(new_key) + 1;
					int new_val_length = strlen(val) + 1;
					final_keys++;
				
					// Insert new key
					memcpy(text_buffer+final_text_buffer_size, new_key, new_key_length);
					final_text_buffer_size += new_key_length;
				
					// Insert new value
					memcpy(text_buffer+final_text_buffer_size, val, new_val_length);
					final_text_buffer_size += new_val_length;
				
					offsets[final_keys] = final_text_buffer_size;
				}
		}
		

		// Remove key and value
		if (argumentID==IGSEDB_REMOVE  &&  found_key) {
			int val_start = offsets[key_index];
			int val_end   = offsets[key_index+1];
			int val_len   = val_end - val_start;

			// Shift values to the left
			memcpy(text_buffer+val_start,  text_buffer+val_end,  final_text_buffer_size-val_end);
			
			// Recalculate offsets
			for (int k=key_index+1; k<final_keys; k++)
				offsets[k] = offsets[k+1] - val_len;
				
			final_text_buffer_size -= val_len;
			final_keys--;
			offsets[final_keys] = final_text_buffer_size;
		}
		

		// Read value under selected key
		if (argumentID==IGSEDB_READ  &&  found_key) {
			int key_length = strlen(key_name) + 1;
			char *value    = text_buffer + offsets[key_index] + key_length;
			QWrite(value, out);
			QWrite(";", out);
		}
		

		// List all keys / values
		if (argumentID == IGSEDB_LIST) {
			if (strcmpi(val,"values") == 0)
				for (int k=0; k<final_keys; k++) {
					int val_start = offsets[k];
					val_start    += strlen(text_buffer+val_start) + 1;
					QWrite(text_buffer+val_start, out);
					QWrite(";", out);
				}
			else {
				if (strcmpi(val,"keys") == 0)
					for (int k=0; k<final_keys; k++)
						String_append_quotes(List, "]+[\"", text_buffer+offsets[k], "\"");
				else 
					if (strcmpi(val,"all") == 0) {
						String_append(List, "[");

						for (int k=0; k<final_keys; k++) 
							String_append_quotes(List, "]+[\"", text_buffer+offsets[k], "\"");

						String_append(List, "]]+[[");

						for (k=0; k<final_keys; k++) {
							int val_start = offsets[k];
							val_start    += strlen(text_buffer+val_start) + 1;
							String_append_quotes(List, "]+[\"", text_buffer+val_start, "\"");
						}

						String_append(List, "]");
					}
			}
		}
	}


	// Rewrite the arg_filename if we made changes to the data
	if (arg_writing_mode) {
		char *file_ptr = ptr_filename;
		
		for (int i=0; ptr_filename[i]!='\0'; i++)	// Replace slashes with backslashes
			if (ptr_filename[i] == '/') 
				ptr_filename[i] = '\\';

		// Create subdirectories
		while (pch = strchr(file_ptr,'\\'))	{
			int pos       = pch - file_ptr;
			char prev     = file_ptr[pos];
			file_ptr[pos] = '\0';

			CreateDirectory(ptr_filename, NULL);

			file_ptr[pos] = prev;
			file_ptr     += pos+1;
		}
		
		if (file = fopen(ptr_filename, "wb")) {
			fwrite(&final_keys, sizeof(final_keys)  , 1                     , file);
			fwrite(offsets+1  , sizeof(*offsets)    , final_keys            , file);
			fwrite(text_buffer, sizeof(*text_buffer), final_text_buffer_size, file);
		}
	}

	if (file  &&  ferror(file))
		FWerror(7,errno,CommandID,ptr_filename,"",0,0,out);
	else
		FWerror(0,0,CommandID,"","",0,0,out);

	QWrite("[", out);
	QWrite(List.pointer, out);
	QWrite("],[", out);
	QWrite(MissingKeys.pointer, out);
	QWrite("]]", out);

	free(offsets);
	free(text_buffer);
	String_end(buf_filename);
	String_end(List);
	String_end(MissingKeys);
		
	if (file)
		fclose(file);

	//fclose(fd);
}
break;
