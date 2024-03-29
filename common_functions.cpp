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
char* String_trim_space(String &input) {
	while (isspace(input.text[0])) {
		input.text++;
		input.length--;
	}

	for (size_t i=input.length-1;  i>=0 && isspace(input.text[i]);  i--) {
		input.text[i] = '\0';
		input.length--;
	}

	return input.text;
}




// Find integer in an integer array
bool IsNumberInArray(int number, const int* array, int array_size) {
	for (int i=0;  i<array_size;  i++)
		if (number == array[i])
			return true;

	return false;
}




// Custom string initilization
void StringDynamic_init(StringDynamic &buffer) {
	memset(buffer.stack, 0, StringDynamic_init_capacity);

	buffer.text     = buffer.stack;
	buffer.length   = 0;
	buffer.capacity = StringDynamic_init_capacity;
}




// Custom string heap allocation
int StringDynamic_allocate(StringDynamic &buffer, size_t new_capacity) {
	if (buffer.capacity >= new_capacity)
		return 0;

	char *new_pointer;

	if (buffer.text == buffer.stack) {
		new_pointer = (char *) malloc(new_capacity);
		if (!new_pointer)
			return new_capacity;

		memset(new_pointer, 0, new_capacity);
		memcpy(new_pointer, buffer.text, buffer.length);
	} else {
		new_pointer = (char *) realloc(buffer.text, new_capacity);
		if (!new_pointer)
			return new_capacity;

		memset(new_pointer+buffer.length, 0, new_capacity-buffer.length);
	}

	buffer.text     = new_pointer;
	buffer.capacity = new_capacity;
	return 0;
}




// Dynamic string concatenation with given length
int StringDynamic_appendl(StringDynamic &buffer, const char *input, size_t input_length) {
	if (input_length > 0) {
		size_t new_length = buffer.length + input_length;

		if (new_length+1 > buffer.capacity) {
			size_t new_capacity = buffer.length + (input_length<StringDynamic_init_capacity ? StringDynamic_init_capacity : input_length) + 1;
			
			if (StringDynamic_allocate(buffer, new_capacity) != 0)
				return new_capacity;
		}

		memcpy(buffer.text+buffer.length, input, input_length);
		memset(buffer.text+new_length, 0, 1);
		buffer.length = new_length;
	}

	return 0;
}

// Dynamic string concatenation without given length
int StringDynamic_append(StringDynamic &buffer, const char *input) {
	return StringDynamic_appendl(buffer, input, strlen(input));
}

// Dynamic string concatenation with String struct
int StringDynamic_appends(StringDynamic &buffer, String &input) {
	return StringDynamic_appendl(buffer, input.text, input.length);
}

int StringDynamic_appendsd(StringDynamic &buffer, StringDynamic &input) {
	return StringDynamic_appendl(buffer, input.text, input.length);
}

// Dynamic string concatenation doubling the quotation marks
int StringDynamic_appendq(StringDynamic &buffer, const char *input) {
	size_t pos   = 0;
	char *quote  = NULL;
	int result   = 0;
	
	while ((quote = strchr(input, '"'))) {
		pos    = quote - input;
		result = StringDynamic_appendl(buffer, input, pos+1);
		result = StringDynamic_append(buffer, "\"");
		input  = quote + 1;
	}
	
	result = StringDynamic_append(buffer, input);
	return result;
}




// Custom string deallocation
void StringDynamic_end(StringDynamic &buffer) {
	if (buffer.text != buffer.stack) {
		free(buffer.text);
		buffer.text     = buffer.stack;
		buffer.length   = 0;
		buffer.capacity = StringDynamic_init_capacity;
	}
}




// Copy file contents to a custom string
int StringDynamic_readfile(StringDynamic &buffer, const char *path) {
	StringDynamic_init(buffer);

	FILE *f = fopen(path, "rb");
	if (!f)
		return errno;

	fseek(f, 0, SEEK_END);
	int file_size = ftell(f);
	fseek(f, 0, SEEK_SET);

	if (StringDynamic_allocate(buffer, file_size+1) != 0) 
		return -1;

	int bytes_read          = fread(buffer.text, 1, file_size, f);
	buffer.length           = bytes_read;
	buffer.text[bytes_read] = '\0';

	if (bytes_read == file_size)
		return 0;
	else {
		StringDynamic_end(buffer);
		return -2;
	}
}




// Custom string concatenation according to a given format
int StringDynamic_appendf(StringDynamic &buffer, const char *format, ...) {
	int space_free     = 0;
	int space_required = 0;
	
	va_list args;
	va_start(args, format);
	
	do {
		space_free     = buffer.capacity - buffer.length;
		space_required = _vsnprintf(buffer.text+buffer.length, space_free, format, args);

		// https://stackoverflow.com/questions/43178776/vsnprintf-returns-the-size-over-given-buffer-size
		if (space_required == -1)
			space_required = buffer.capacity * 2;

		if (space_required+1 > space_free) {
			int result = StringDynamic_allocate(buffer, buffer.capacity+((space_required+1)-space_free));
			if (result != 0) {
				va_end(args);
				return result;
			}
		} else
			buffer.length += space_required;
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



// Split string by selected characters
String String_tokenize(String &source, const char *delimiter, size_t &i, int options) 
{
	size_t word_start = 0;
	bool in_brackets  = false;
	bool word_started = false;
	
	if (i==0  &&  options & OPTION_TRIM_SQUARE_BRACKETS  &&  source.text[0]=='['  &&  source.text[source.length-1]==']') {
		source.text++;
		source.length -= 2;
		source.text[source.length] = '\0';
	}

	for (;  i<=source.length;  i++) {
		// Ignore delimeters inside square brackets		
		if (options & OPTION_SKIP_SQUARE_BRACKETS) {
			if (source.text[i] == '[')
				in_brackets = true;
				
			if (source.text[i] == ']')
				in_brackets = false;
		}

		bool is_delimeter = false;

		// Check if current character is a delimiter
		if (!in_brackets)
			for (int j=0;  delimiter[j]!='\0' && !is_delimeter;  j++)
				if (source.text[i] == delimiter[j])
					is_delimeter = true;

		// Mark beginning of the word
		if (!word_started  &&  (!is_delimeter || i==source.length)) {
			word_start   = i;
			word_started = true;
		}

		// End the word
		if (word_started  &&  (is_delimeter ||  i==source.length)) {
			source.text[i] = '\0';
			String item    = {source.text+word_start, i-word_start};
			i++;
			return item;
		}
	}

	String item = {source.text+source.length, 0};
	return item;
}






// Shift substring in a string buffer to the left or right
void shift_buffer_chunk(char *buffer, size_t chunk_start, size_t chunk_end, size_t shift_distance, bool rightwards) {
	if (rightwards)
		for (size_t i=chunk_end+shift_distance;  i>=chunk_start+shift_distance;  i--)
			buffer[i] = buffer[i-shift_distance];
	else
		for (size_t i=chunk_start;  i<chunk_end;  i++)
			buffer[i-shift_distance] = buffer[i];
}