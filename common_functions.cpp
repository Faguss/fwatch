// Functions used by both TestLauncher and TestDLL

// Find process by exename by traversing the list; returns PID number
DWORD findProcess(const char* exe_name) {
	PROCESSENTRY32 processInfo;
	processInfo.dwSize = sizeof(processInfo);
	DWORD output       = 0;
	DWORD pid_self     = GetCurrentProcessId();

	HANDLE processesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (processesSnapshot != INVALID_HANDLE_VALUE) {
		Process32First(processesSnapshot, &processInfo);

		do {
			DWORD pid = processInfo.th32ProcessID;

			if (strcmpi(processInfo.szExeFile,exe_name)==0  &&  pid!=pid_self)
				output = pid;
		} 
		while (output==0 && Process32Next(processesSnapshot, &processInfo));

		CloseHandle(processesSnapshot);
	}

	return output;
}




// Compare characters of two strings case insensitive
// http://my.fit.edu/~vkepuska/ece5527/sctk-2.3-rc1/src/rfilter1/include/strncmpi.c
int strncmpi(const char *ps1, const char *ps2, int n) {
	char *px1     = (char *)ps1;
	char *px2     = (char *)ps2;
	int indicator = 9999;
	int i         = 0;

	while (indicator == 9999) {
		if (++i > n) 
			indicator = 0;
		else {
			if (*px1 == '\0') {
				if (*px2 == '\0') 
					indicator = 0; 
				else	
					indicator = -1;
			} else {
				if (toupper((int)*px1)  <  toupper((int)*px2)) 
					indicator = -1; 
				else {
					if (toupper((int)*px1)  >  toupper((int)*px2)) 
						indicator = 1; 
					else 
						px1 += 1, 
						px2 += 1;
				}
			}
		}
	}

	return indicator;
}




// Remove whitespace from a string
char* Trim(char *txt) {
	while (isspace(txt[0])) 
		txt++;

	for (int i=strlen(txt)-1;  i>=0 && isspace(txt[i]);  i--) 
		txt[i] = '\0';

	return txt;
}




// Find integer in an integer array
bool IsNumberInArray(int number, const int* array, int array_size) {
	for (int i=0;  i<array_size;  i++)
		if (number == array[i])
			return true;

	return false;
}




// Custom string initilization
void String_init(String &str) {
	memset(str.stack, 0, String_init_capacity);
	str.text     = str.stack;
	str.length   = 0;
	str.capacity = String_init_capacity;
}




// Custom string heap allocation
int String_allocate(String &str, size_t new_capacity) {
	if (str.capacity >= new_capacity)
		return 0;

	char *new_pointer;

	if (str.text == str.stack) {
		new_pointer = (char *) malloc(new_capacity);
		if (!new_pointer)
			return new_capacity;

		memset(new_pointer, 0, new_capacity);
		memcpy(new_pointer, str.text, str.length);
	} else {
		new_pointer = (char *) realloc(str.text, new_capacity);
		if (!new_pointer)
			return new_capacity;

		memset(new_pointer+str.length, 0, new_capacity-str.length);
	}

	str.text     = new_pointer;
	str.capacity = new_capacity;
	return 0;
}




// Custom string concatenation
int String_append_len(String &str, const char *input, size_t input_length) {
	if (input_length > 0) {
		size_t new_length = str.length + input_length;

		if (new_length+1 > str.capacity) {
			size_t new_capacity = str.length + (input_length<String_init_capacity ? String_init_capacity : input_length) + 1;
			
			if (String_allocate(str, new_capacity) != 0)
				return new_capacity;
		}

		memcpy(str.text+str.length, input, input_length);
		memset(str.text+new_length, 0, 1);
		str.length = new_length;
	}

	return 0;
}

int String_append(String &str, const char *text) {
	return String_append_len(str, text, strlen(text));
}




// Custom string concatenation with quotes around
int String_append_quotes(String &str, const char *left, char *text, const char *right) {
	String_append(str, left);

	char *quote = strchr(text, '\"');

	if (!quote) 
		String_append(str, text); 
	else {
		int pos     = 0;
		int lastPos = 0;
		int len     = strlen(text);
		
		do {
			pos       = quote - text;
			text[pos] = '\0';
			String_append(str, text+lastPos);
			String_append(str, "\"\"");
			text[pos] = '\"';
			lastPos = pos + 1;
		} while ((quote = strchr(text+lastPos, '\"')));

		if (lastPos < len)
			String_append(str, text+lastPos);

	}

	String_append(str, right);
	return 0;
}




// Custom string deallocation
void String_end(String &str) {
	if (str.text != str.stack) {
		free(str.text);
		str.text     = str.stack;
		str.length   = 0;
		str.capacity = String_init_capacity;
	}
}




// Copy file contents to a custom string
int String_readfile(String &str, char *path) {
	String_init(str);

	FILE *f = fopen(path, "rb");
	if (!f)
		return errno;

	fseek(f, 0, SEEK_END);
	int file_size = ftell(f);
	fseek(f, 0, SEEK_SET);

	if (String_allocate(str, file_size+1) != 0) 
		return -1;

	int bytes_read       = fread(str.text, 1, file_size, f);
	str.length           = bytes_read;
	str.text[bytes_read] = '\0';

	if (bytes_read == file_size)
		return 0;
	else {
		String_end(str);
		return -2;
	}
}




// Custom string concatenation according to a given format
int String_append_format(String &str, const char *format, ...) {
	int space_free     = 0;
	int space_required = 0;
	
	va_list args;
	va_start(args, format);
	
	do {
		space_free     = str.capacity - str.length;
		space_required = _vsnprintf(str.text+str.length, space_free, format, args);

		// https://stackoverflow.com/questions/43178776/vsnprintf-returns-the-size-over-given-buffer-size
		if (space_required == -1)
			space_required = str.capacity * 2;

		if (space_required+1 > space_free) {
			int result = String_allocate(str, str.capacity+((space_required+1)-space_free));
			if (result != 0) {
				va_end(args);
				return result;
			}
		} else
			str.length += space_required;
	} while (space_required+1 > space_free);

	va_end(args);
	return 0;
}




// Load from a file pid and exit code under given key
WatchProgramInfo db_pid_load(int db_id_wanted) {
	WatchProgramInfo ret = {0,0,0,0};
	FILE *file           = fopen("fwatch\\data\\pid.db","rb");

	if (file) {
		fseek(file, 0, SEEK_END);
		int file_size = ftell(file);
		fseek(file, 0, SEEK_SET);

		bool found = false;

		while (ftell(file) < file_size) {
			fread(&ret, sizeof(WatchProgramInfo), 1, file);

			if (ret.db_id == db_id_wanted) {
				found = true;
				break;
			}
		}

		if (!found) {
			ret.db_id        = 0;
			ret.exit_code    = 0;
			ret.launch_error = 0;
			ret.pid          = 0;
		}

		fclose(file);
	}

	return ret;
}



// Save pid and exit code under given key to a file
void db_pid_save(WatchProgramInfo input) {
	FILE *file = fopen("fwatch\\data\\pid.db","rb");

	int record_num = 0;

	if (file) {
		fseek(file, 0, SEEK_END);
		record_num = ftell(file) / sizeof(WatchProgramInfo);
		fseek(file, 0, SEEK_SET);
	}

	WatchProgramInfo *record = (WatchProgramInfo *) malloc((record_num+1) * sizeof(WatchProgramInfo));

	if (!record) {
		fclose(file);
		return;
	}

	if (file) {
		fread(record, sizeof(WatchProgramInfo), record_num, file);
		fclose(file);
	}

	int index = -1;

	for (int i=0; i<record_num && index==-1; i++)
		if (record[i].db_id == input.db_id)
			index = i;

	if (index == -1)
		index = record_num++;

	record[index] = input;

	if ((file = fopen("fwatch\\data\\pid.db", "wb"))) {
		fwrite(record, sizeof(WatchProgramInfo), record_num, file);
		fclose(file);
	}

	free(record);
}