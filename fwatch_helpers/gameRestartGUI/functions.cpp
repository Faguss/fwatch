#include "stdafx.h"
#include "resource.h"
#include "common.h"
#include "functions.h"

std::wstring utf16(std::string &input)
{
	if (input.empty())
		return std::wstring();

	std::string *ptr_input = &input;
	std::string crop       = "";

	if (input.size() > INT_MAX) {
		crop      = input.substr(0, INT_MAX);
		ptr_input = &crop;
	}

	int input_size = static_cast<int>((*ptr_input).size());
	int output_size = MultiByteToWideChar(CP_UTF8, 0, (*ptr_input).c_str(), input_size, NULL, 0);

	std::wstring output(output_size, L'\0');
	MultiByteToWideChar(CP_UTF8, 0, (*ptr_input).c_str(), input_size, &output[0], output_size);

	return output;
}

std::string utf8(const wchar_t* input, int input_size)
{
	if (input_size == 0)
		return std::string();
	
	int output_size = WideCharToMultiByte(CP_UTF8, 0, input, input_size, NULL, 0, NULL, NULL);

	std::string output(output_size, '\0');
	WideCharToMultiByte(CP_UTF8, 0, input, input_size, const_cast<char*>(output.c_str()), output_size, NULL, NULL);

	return output;
}

std::string utf8(std::wstring& input)
{
	if (input.empty())
		return std::string();

	std::wstring *ptr_input = &input;
	std::wstring crop       = L"";

	if (input.size() > INT_MAX) {
		crop      = input.substr(0, INT_MAX);
		ptr_input = &crop;
	}

	return utf8((*ptr_input).c_str(), static_cast<int>((*ptr_input).size()));
}

std::wstring FormatError(DWORD error)
{
	if (error == ERROR_SUCCESS)
		return L"\r\n";

	LPTSTR lpMsgBuf = NULL;
	FormatMessageW(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		error,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0,
		NULL
	);

	std::wstring output = L" - " + (std::wstring)lpMsgBuf;

	if (lpMsgBuf != NULL)
		LocalFree(lpMsgBuf);

	return output;
}

bool Equals(const std::string& a, const std::string& b) 
{
    size_t sz = a.size();

    if (b.size() != sz)
        return false;

    for (size_t i = 0; i < sz; ++i)
		if (_strnicmp(a.c_str()+i, b.c_str()+i, 1) != 0)
            return false;

    return true;
}

bool Equals(const std::wstring& a, const std::wstring& b) 
{
    size_t sz = a.size();

    if (b.size() != sz)
        return false;

    for (size_t i=0; i<sz; ++i)
        if (_wcsnicmp(a.c_str()+i, b.c_str()+i, 1) != 0)
			return false;

    return true;
}

DWORD GetProcessID(std::wstring exename)
{
	PROCESSENTRY32 processInfo;
	DWORD pid                = 0;
	processInfo.dwSize 		 = sizeof(processInfo);
	HANDLE processesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	
	if (processesSnapshot != INVALID_HANDLE_VALUE) {
		Process32First(processesSnapshot, &processInfo);
		
		do {
			if (_wcsicmp(processInfo.szExeFile,exename.c_str()) == 0)
				pid = processInfo.th32ProcessID;
		} while (Process32Next(processesSnapshot, &processInfo));

		CloseHandle(processesSnapshot);
	}
    
    return pid; 
}

GameInfo FindProcess(DWORD input_pid, std::wstring input_window_name)
{
    GameInfo info = {NULL, 0};
	HWND handle   = GetTopWindow(NULL);
	
	if (handle) {
		const int window_name_length = 1024;
		wchar_t current_window[window_name_length] = L"";
		DWORD current_pid = 0;
	
		while (handle) {
			GetWindowTextW(handle, current_window, window_name_length);
			GetWindowThreadProcessId(handle, &current_pid);
	
			bool match_name = _wcsnicmp(current_window, input_window_name.c_str(), input_window_name.length()) == 0;
			bool match_pid  = input_pid == current_pid;
	
			if (
				(input_pid!=0 && !input_window_name.empty() && match_name && match_pid) || 
				(input_pid!=0 && input_window_name.empty() && match_pid) ||
				(input_pid==0 && !input_window_name.empty() && match_name)
			) {
				info.handle = handle;
				info.pid    = current_pid;
				break;
			}
			
			handle = GetNextWindow(handle, GW_HWNDNEXT);
		}
	} else
		LogMessage(L"Couldn't get top window" + FormatError(GetLastError()));
	
	return info;
};

DWORD GetOFPArguments(DWORD pid, HANDLE *phandle, std::wstring module_name, int offset_input, std::vector<std::wstring> &container)
{
	MODULEENTRY32 xModule;
	uintptr_t offset_module = 0;
	HANDLE hSnap            = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
	
	if (hSnap != INVALID_HANDLE_VALUE) {
		xModule.dwSize = sizeof(MODULEENTRY32);

		do {
			if (_wcsicmp(xModule.szModule, module_name.c_str()) == 0) {
				offset_module = (uintptr_t)xModule.modBaseAddr;
				break;
			}
		} while (Module32Next(hSnap, &xModule));
		
		CloseHandle(hSnap);
		
		if (offset_module) {
			uintptr_t pointer[] = {offset_module+offset_input, 0, 0};
			uintptr_t modif[]   = {0x0, 0x0};
			uintptr_t max_loops = sizeof(pointer) / sizeof(pointer[0]) - 1;
			SIZE_T stBytes      = 0;
		
			for (uintptr_t i=0; i<max_loops; i++) {
                ReadProcessMemory(*phandle, (LPVOID)pointer[i], &pointer[i+1], 4, &stBytes);
				pointer[i+1] = pointer[i+1] +  modif[i];
			}
			
			const int buffer_length = 1024;
			char buffer[buffer_length] = "";
			ReadProcessMemory(*phandle, (LPVOID)pointer[max_loops], &buffer, buffer_length, &stBytes);
		
			std::string word       = "";
			bool ignore_first_word = true;

			for (int i=0, nullTerminators=0;  i<buffer_length;  i++)
				if (buffer[i] == '\0') {
					nullTerminators++;
					
					if (nullTerminators == 2) 
						break;
						
					if (!word.empty()) {
						size_t isexe = word.find_last_of(".exe");

						if (ignore_first_word  &&  isexe==std::string::npos)
							ignore_first_word = false;
						
						if (ignore_first_word)
							ignore_first_word = false;
						else
							container.push_back(utf16(word));
						
						word = "";
					}
		        } else {
					word += buffer[i];
					nullTerminators = 0;
				}

            return ERROR_SUCCESS;
        } else
            return ERROR_MOD_NOT_FOUND;
    } else
		return GetLastError();
}

void WriteSaveStateFile(std::string input)
{
    std::ofstream file;
    file.open("fwatch\\tmp\\schedule\\restart_info.sqf", std::ios::out | std::ios::app);
    
    if (!file.is_open())
        return;
    
    file << input;
    file.close();
}

std::wstring ReplaceAll(std::wstring str, const std::wstring& from, const std::wstring& to) 
{
    if (from.empty())
        return str;
        
    size_t start_pos = 0;
    
    while ((start_pos = str.find(from, start_pos)) != std::wstring::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
    
    return str;
}

unsigned int HashFNV1A(unsigned int hash, const char *text, size_t text_length, bool lowercase) 
{
    for (size_t i=0; i<text_length; i++)
		hash = (hash ^ (lowercase ? tolower(text[i]) : text[i])) * FNV_PRIME;

    return hash;
}

BinarySearchResult BinarySearchString(const char *buffer, size_t array_size, unsigned int value_to_find, size_t low, size_t high) 
{
	if (array_size && high>=low) {
		size_t mid        = low + (high - low) / 2;
		size_t *mid_value = (size_t*)(buffer + mid*sizeof(size_t));

		if (*mid_value == value_to_find) {
			BinarySearchResult out = {mid,1};
			return out;
		} else
			if (*mid_value > value_to_find) {
				if (mid > 0)
					return BinarySearchString(buffer, array_size, value_to_find, low, mid-1);
				else {
					// target key should be at the start of the array
					BinarySearchResult out = {low, 0};
					return out;
				}
			} else
				if (mid < array_size-1)
					return BinarySearchString(buffer, array_size, value_to_find, mid+1, high);
				else {
					// target key should be at the end of the array
					BinarySearchResult out = {mid+1, 0};
					return out;
				}
	}
	
	BinarySearchResult out = {low,0};
	return out;
}

std::string ReadStartupParams(std::string key_name)
{
	std::ifstream file("fwatch\\tmp\\schedule\\params.bin", std::ios::in | std::ios::binary);
	std::string file_contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	std::string output = "";
  
	if (file.is_open())
		file.close();
	else 
		return output;

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
	} value_pointer;
	
	const char *buffer = file_contents.c_str();
	size_t buffer_size = file_contents.size();
	header             = (igsedb_header*)buffer;
	
	if (header->signature != igsedb_signature || header->version > igsedb_version)
		return output;
	
	size_t minimal_file_size = sizeof(igsedb_header) + header->number_of_keys*sizeof(size_t)*3 + header->number_of_keys*2;
	if (buffer_size < minimal_file_size)
		return output;
	
	// Verify database
	bool db_error = false;
	
	{
		unsigned int hash_previous   = 0;
		size_t pointer_previous      = 0;
		size_t minimal_pointer_value = sizeof(igsedb_header) + header->number_of_keys * sizeof(size_t) * 3;
		
		for (size_t i=0; i<header->number_of_keys; i++) {
			size_t hash_pos    = sizeof(igsedb_header) + i * sizeof(size_t);
			unsigned int *hash = (unsigned int*)(buffer + hash_pos);
			
			if (*hash <= hash_previous) {
				db_error = true;
				break;
			}
			
			hash_previous = *hash;
		}
		
		for (size_t i=0;  i<header->number_of_keys*2 && !db_error;  i++) {
			size_t pointer_pos    = sizeof(igsedb_header) + (header->number_of_keys+i) * sizeof(size_t);
			size_t *pointer_value = (size_t*)(buffer + pointer_pos);
			
			if (i==0  &&  *pointer_value!=minimal_pointer_value) {
				db_error = true;
				break;
			}
				
			if (*pointer_value < minimal_pointer_value) {
				db_error = true;
				break;
			}
			
			if (*pointer_value > buffer_size) {
				db_error = true;
				break;
			}
			
			if ((i>0 && buffer[*pointer_value-1]!='\0') || (i==header->number_of_keys*2-1  &&  buffer[buffer_size-1]!='\0')) {
				db_error = true;
				break;
			}
			
			if (*pointer_value <= pointer_previous) {
				db_error = true;
				break;
			}

			pointer_previous = *pointer_value;
		}
	}
	
	if (db_error)
		return output;
	
	// Find key
	BinarySearchResult key_to_read = BinarySearchString(
		buffer+sizeof(igsedb_header), 
		header->number_of_keys, 
		HashFNV1A(FNV_BASIS, key_name.c_str(), key_name.length(), OPTION_LOWERCASE), 
		0, 
		header->number_of_keys-1
	);
	
	if (!key_to_read.found)
		return output;
	
	// Read pointer
	size_t value_pointer_pos = sizeof(igsedb_header) + (header->number_of_keys*2 + key_to_read.index) * sizeof(size_t);
	memcpy(&value_pointer, buffer+value_pointer_pos, sizeof(igsedb_pointer));
	
	if (key_to_read.index == header->number_of_keys-1)
		value_pointer.end = buffer_size;
		
	// Verify pointer
	size_t minimal_pointer_value = sizeof(igsedb_header) + header->number_of_keys * sizeof(size_t) * 3;
	
	if (value_pointer.start < minimal_pointer_value || value_pointer.end > buffer_size || (key_to_read.index>0 && buffer[value_pointer.start-1]!='\0')  ||  buffer[value_pointer.end-1]!='\0')
		return output;

	// Read value
	for (size_t i=0; i<value_pointer.end-1-value_pointer.start; i++)
		output += (buffer + value_pointer.start)[i];

	return output;
}

std::string UnQuote(std::string text)
{
	if (text.substr(text.length()-1) == "\"")
		text = text.substr(0, text.length()-1);
	
	if (text.substr(0,1) == "\"")
		text = text.substr(1);
		
	return text;	
}

std::wstring UnQuote(std::wstring text)
{
	if (text.substr(text.length()-1) == L"\"")
		text = text.substr(0, text.length()-1);
	
	if (text.substr(0,1) == L"\"")
		text = text.substr(1);
		
	return text;	
}

void Tokenize(std::string text, std::string delimiter, std::vector<std::string> &container)
{
	bool inQuote      = false;
	size_t word_start = 0;
	bool word_started = false;
	
	for (size_t pos=0;  pos<=text.length();  pos++) {
		bool isToken = pos == text.length();
		
		for (size_t i=0;  !isToken && i<delimiter.length();  i++)
			if (text.substr(pos,1) == delimiter.substr(i,1))
				isToken = true;
				
		if (text.substr(pos,1) == "\"")
			inQuote = !inQuote;
			
		// Mark beginning of the word
		if (!isToken  &&  !word_started) {
			word_start   = pos;
			word_started = true;
		}
						
		// Mark end of the word
		if (isToken  &&  word_started  &&  !inQuote) {
			std::string part = UnQuote(text.substr(word_start, pos-word_start));
			container.push_back(part);
			word_started = false;
		}
	}
}

void Tokenize(std::wstring text, std::wstring delimiter, std::vector<std::wstring> &container)
{
	bool inQuote      = false;
	size_t word_start = 0;
	bool word_started = false;
	
	for (size_t pos=0;  pos<=text.length();  pos++) {
		bool isToken = pos == text.length();
		
		for (size_t i=0;  !isToken && i<delimiter.length();  i++)
			if (text.substr(pos,1) == delimiter.substr(i,1))
				isToken = true;
				
		if (text.substr(pos,1) == L"\"")
			inQuote = !inQuote;
			
		// Mark beginning of the word
		if (!isToken  &&  !word_started) {
			word_start   = pos;
			word_started = true;
		}
						
		// Mark end of the word
		if (isToken  &&  word_started  &&  !inQuote) {
			std::wstring part = UnQuote(text.substr(word_start, pos-word_start));
			container.push_back(part);
			word_started = false;
		}
	}
}

std::string Int2Str(int num)
{
    std::ostringstream text;
    text << num;
    return text.str();
}

std::wstring Int2StrW(int num, bool leading_zero)
{
	const int buffer_size       = 256;
	wchar_t buffer[buffer_size] = L"";
	wchar_t *buffer_ptr         = buffer;
	
	swprintf_s(buffer_ptr, buffer_size, L"%s%d", (leading_zero && num < 10 ? L"0" : L""), num);
	return (std::wstring)buffer;
}

std::wstring Hex2StrW(int num) {
	std::wstringstream ss;
	ss << std::hex << num;
	return ss.str();
}

WORD Str2Short(std::string num)
{
	std::stringstream sstream(num);
	WORD result;
    sstream >> result;
    return result;
}

std::string Trim(std::string s)
{
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(isspace))));
	s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(isspace))).base(), s.end());
	return s;
}

std::wstring PathLastItem(std::wstring path) 
{
	size_t lastSlash = path.find_last_of(L"\\/");

	if (lastSlash != std::wstring::npos)
		return path.substr(lastSlash+1);
	else
		return path;
}

std::wstring PathNoLastItem(std::wstring path, bool end_slash) 
{
	size_t find = path.find_last_of(L"\\/");

	if (find != std::wstring::npos)
		return path.substr(0, find+end_slash);
	else
		return L"";
}

std::wstring WrapInQuotes(std::wstring text)
{
	for (size_t i=0; i<text.length(); i++)
		if (text.substr(i,1) == L" ") {
			text = L"\"" + text + L"\"";
			break;
		}
	
	return text;
}

int ParseWgetLog(std::string &error)
{
	std::ifstream DownloadLog;
    DownloadLog.open("fwatch\\tmp\\schedule\\downloadLog.txt", std::ios::in);

	if (DownloadLog.is_open()) {
		std::string line                  = "";
		std::string filesize              = "";
		std::string size_downloaded       = "";
		std::string percentage_downloaded = "";
		std::string download_speed        = "";
		std::string time_remaining        = "";
		
		const int filename_messages_items = 4;
		std::vector<std::string> filename_messages[filename_messages_items];
		filename_messages[0].push_back("Saving to: '");
		filename_messages[0].push_back("'");
		filename_messages[1].push_back(") - '");
		filename_messages[1].push_back("' saved [");
		filename_messages[2].push_back("File '");
		filename_messages[2].push_back("' already there; not retrieving");
		filename_messages[3].push_back("Server file no newer than local file '");
		filename_messages[3].push_back("' -- not retrieving");
			
		std::vector<std::string> error_messages;
		error_messages.push_back("failed");
		error_messages.push_back("ERROR");

		while(getline(DownloadLog, line)) {
			line = Trim(line);

			if (line.empty())
				continue;

			// Get file size
			if (filesize.empty()  &&  line.substr(0,8) == "Length: ") {
				size_t open  = line.find('(');
				size_t close = line.find(')');

				if (open!=std::string::npos  &&  close!=std::string::npos)
					filesize = line.substr( open+1, close-open-1);
			}

			// Get progress bar
			size_t letter_k = line.find("K .");

			if (letter_k == std::string::npos)
				letter_k = line.find("K ,");
			
			if (letter_k != std::string::npos) {			
				size_downloaded = line.substr(0, letter_k);
				int size_num    = atoi(size_downloaded.c_str());
				
				if (size_num > 1024) {
					double megabytes = size_num / 1024.0f;
					char temp[128] = "";
					sprintf_s(temp, 128, "%.0f M", megabytes);
					size_downloaded = (std::string)temp;
				} else
					size_downloaded += " K";
				
				std::string new_download_speed = "";
				size_t percent                 = line.find("% ");
				if (percent != std::string::npos) {
					while(percent>=0  &&  (line[percent]=='%'  ||  isdigit(line[percent])))
						percent--;
						
					std::vector<std::string> Tokens;
					Tokenize(line.substr(percent), " =", Tokens);
					
					if (Tokens.size() > 0)
						percentage_downloaded = Tokens[0];
						
					if (Tokens.size() > 1) {
						new_download_speed = Tokens[1];
					}
						
					if (Tokens.size() > 2)
						time_remaining = Tokens[2];
				} else {
					size_t i = letter_k + 3;
					while(i<line.length() && !isdigit(line[i]))
						i++;
						
					if (i < line.length())
						new_download_speed = line.substr(i);
				}
				
				if (new_download_speed.length() > 0)
					download_speed = new_download_speed.substr(0,new_download_speed.length()-1) + " " + new_download_speed.substr(new_download_speed.length()-1);
			}

			// Get file name
			for (int i=0; i<filename_messages_items; i++) {
				size_t begin = line.find(filename_messages[i][0]);
				
				if (begin != std::string::npos) {
					size_t len = filename_messages[i][0].length();
					size_t end = line.find(filename_messages[i][1], begin+len);
					
					if (end != std::string::npos) {
						std::string cut = line.substr(begin+len,  end-(begin+len));
						global.downloaded_filename = utf16(cut);
						break;
					}
				}
			}

			// Get error message
			for (size_t i=0; i<error_messages.size(); i++) {
				size_t search = line.find(error_messages[i]);
				if (search != std::string::npos) {
					if (i==1)
						error = line.substr(search);
					else
						error = line;
					break;
				} else
					error = line;
			}
		}

		DownloadLog.close();
	}
	else
		return 1;
	
	return 0;
}

int ParseUnpackLog(std::string &error)
{
	std::ifstream UnpackLog;
    UnpackLog.open("fwatch\\tmp\\schedule\\unpackLog.txt", std::ios::in);

    int line_number       = 0;
    int error_until_line  = 0;
	std::string error_msg = "";

	if (UnpackLog.is_open()) {
		std::string text         = "";
		std::string current_file = "";
		std::string percentage   = "";

		while(getline(UnpackLog, text)) {
			text = Trim(text);

			if (text.empty())
				continue;

			line_number++;

			// Get progress percentage
			size_t percent = text.find_last_of("%");

			if (percent != std::string::npos) {
				if (percent < 2)
					percent = 0;
				else
					percent -= 2;

				percentage  = text.substr(percent, 3);
				size_t dash = text.find("- ");

				if (dash != std::string::npos)
					current_file = text.substr(dash+2);					
			}

			// Get error message
			size_t error_pos = text.find("ERROR:");

			if (error_pos != std::string::npos  &&  error=="") {
				error            = text.substr(error_pos);
				error_msg        = text.substr(error_pos+7);
				error_until_line = line_number + 1;
			}

			if (line_number == error_until_line) {
				if (text != error_msg)
					error += " - " + text;
				else
					error_until_line++;
			}

		}

		UnpackLog.close();
	}

	return 0;
}

DWORD DeleteDirectory(const std::wstring &refcstrRootDirectory, bool bDeleteSubdirectories)
{
	bool            bSubdirectory = false;       // Flag, indicating whether subdirectories have been found
	HANDLE          hFile;                       // Handle to directory
	std::wstring    strFilePath;                 // Filepath
	std::wstring    strPattern;                  // Pattern
	WIN32_FIND_DATA FileInformation;             // File information

	strPattern = refcstrRootDirectory + L"\\*.*";
	hFile      = FindFirstFile(strPattern.c_str(), &FileInformation);
	
	if (hFile != INVALID_HANDLE_VALUE) {
		do {
			if (FileInformation.cFileName[0] != L'.') {
				strFilePath.erase();
				strFilePath = refcstrRootDirectory + L"\\" + FileInformation.cFileName;

				if (FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
					if (bDeleteSubdirectories) {
						// Delete subdirectory
						DWORD iRC = DeleteDirectory(strFilePath, bDeleteSubdirectories);
						if (iRC)
							return iRC;
					} else
						bSubdirectory = true;
				} else {
					// Set file attributes
					if (SetFileAttributes(strFilePath.c_str(), FILE_ATTRIBUTE_NORMAL) == FALSE)
						return GetLastError();

					// Delete file
					if (DeleteFile(strFilePath.c_str()) == FALSE)
						return GetLastError();
				}
			}
		}
		while (FindNextFile(hFile, &FileInformation) == TRUE);

		// Close handle
		FindClose(hFile);

		DWORD dwError = GetLastError();
		
		if (dwError != ERROR_NO_MORE_FILES)
      		return dwError;
		else {
			if (!bSubdirectory) {
				// Set directory attributes
				if (SetFileAttributes(refcstrRootDirectory.c_str(), FILE_ATTRIBUTE_NORMAL) == FALSE)
					return GetLastError();

				// Delete directory
				if (refcstrRootDirectory != L"fwatch\\tmp\\_extracted") {
					if (RemoveDirectory(refcstrRootDirectory.c_str()) == FALSE)
						return GetLastError();
				} else
					return 0;
			}
		}
	}

	return 0;
}

DWORD Download(std::wstring url)
{
	global.downloaded_filename = PathLastItem(url);
	std::wstring arguments     = L" --user-agent=\"Mozilla/5.0 (Windows NT 6.1; Win64; x64; rv:103.0) Gecko/20100101 Firefox/103.0\" --tries=1 --output-file=fwatch\\tmp\\schedule\\downloadLog.txt " + url;

	DeleteFile(L"fwatch\\tmp\\schedule\\downloadLog.txt");
				
	PROCESS_INFORMATION pi;
    STARTUPINFO si; 
	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));
	si.cb 		   = sizeof(si);
	si.dwFlags 	   = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;
	
	if (!CreateProcess(L"fwatch\\data\\wget.exe", &arguments[0], NULL, NULL, false, 0, NULL, NULL, &si, &pi)) {
		DWORD error_code = GetLastError();
		LogMessage(L"Failed to launch wget.exe" + FormatError(error_code));
		return error_code;
	} else
		LogMessage(L"Downloading " + url);

	DWORD exit_code;
	std::string message = "";
	Sleep(10);

	do {					
		ParseWgetLog(message);
		GetExitCodeProcess(pi.hProcess, &exit_code);
		Sleep(100);
	} while (exit_code == STILL_ACTIVE);

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	
	ParseWgetLog(message);
	
	if (exit_code != ERROR_SUCCESS)
		LogMessage(L"Failed to download " + global.downloaded_filename + L" - " + Int2StrW(exit_code) + L" - " + utf16(message));
	
	return exit_code;
}

DWORD Unpack(std::wstring file_name, std::wstring password, bool tmp_dir)
{
	std::wstring relative_path = PathNoLastItem(file_name);

	if (Equals(relative_path.substr(0,10), L"_extracted"))
		relative_path = relative_path.substr(11) += L"_extracted";

	std::wstring destination = L"fwatch\\tmp\\_extracted";
	
	if (relative_path != L"")
		destination += L"\\" + relative_path;

	DeleteDirectory(destination);


	SECURITY_ATTRIBUTES sa;
    sa.nLength              = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle       = TRUE;       

    HANDLE logFile = CreateFile(L"fwatch\\tmp\\schedule\\unpackLog.txt",
        FILE_APPEND_DATA,
        FILE_SHARE_WRITE | FILE_SHARE_READ,
        &sa,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
	);
        
	PROCESS_INFORMATION pi;
    STARTUPINFO si; 
	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));
	si.cb 		           = sizeof(si);
	si.dwFlags 	           = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	si.wShowWindow         = SW_HIDE;
	si.hStdInput           = NULL;
	si.hStdOutput          = logFile;
	si.hStdError           = logFile;
	std::wstring arguments = WrapInQuotes(global.working_directory) + (password.empty() ? L"" : L" -p"+password) + L" x -y -bb3 -bsp1 " + (tmp_dir ? (L"-ofwatch\\tmp\\_extracted ") : L"") + file_name;

	if (!CreateProcess(L"fwatch\\data\\7z.exe", &arguments[0], NULL, NULL, true, 0, NULL, NULL, &si, &pi)) {		
		DWORD error_code = GetLastError();
		LogMessage(L"Failed to launch 7z.exe - " + Int2StrW(error_code) + L" - " + FormatError(error_code));
		return error_code;
	} else
		LogMessage(L"Extracting " + file_name);
		
	DWORD exit_code;
	std::string message = "";
	Sleep(10);

	do {					
		ParseUnpackLog(message);
		GetExitCodeProcess(pi.hProcess, &exit_code);
		Sleep(100);
	} while (exit_code == STILL_ACTIVE);

	ParseUnpackLog(message);

	if (exit_code != ERROR_SUCCESS)
		LogMessage(L"Failed to extract " + file_name + L" - " + Int2StrW(exit_code) + L" - " + utf16(message));

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	CloseHandle(logFile);

	return exit_code;
}

std::wstring Decrypt(std::wstring sentence) 
{
	#include "keys.cpp"
	std::vector<int> numbers;
	std::vector<std::string> words;
	Tokenize(utf8(sentence), "a", words);
	std::string decrypted = "";

	for (size_t i=0; i<words.size(); i++)	{
		std::string digits = "";
		
		for (size_t j=0; j<words[i].length(); j++) {
			char digit = words[i][j];
			digits    += Int2Str(digit-98);
		}
		
		numbers.push_back(atoi(digits.c_str()));
	}
	
	for (size_t i=0; i<numbers.size(); i++) {
		int result = 1;
		int key    = decrypt_key;
		
		while(key > 0) {
			if (key %2 == 1)
				result = (result * numbers[i]) % modulus_key;
				
			numbers[i] = numbers[i] * numbers[i] % modulus_key;
			key        = key / 2;
		}
		
		decrypted += (char)result;
		
	}
	
	return utf16(decrypted);
}

void LogMessage(std::wstring input, bool close)
{
	if (close)
		input += L"\r\n\r\n--------------\r\n\r\n";
	else
		input += L"\r\n";
	
	if (global.logfile.is_open()) {
		global.logfile << utf8(input);

		if (close)
			global.logfile.close();
	}

	global.display_buffer += input;

	if (global.dialog_text_field)
		SetWindowText(global.dialog_text_field, global.display_buffer.c_str());
}

bool RenameWithBackup(std::wstring &rename_src, std::wstring &rename_dst)
{
	if (GetFileAttributes(rename_dst.c_str()) != INVALID_FILE_ATTRIBUTES) {
		std::wstring backup_dst = L"";
		int tries               = 1;
		DWORD last_error        = 0;

		do {
			backup_dst = backup_dst + L"_renamed" + (tries>1 ? Int2StrW(tries) : L"");
			
			if (MoveFileEx(rename_dst.c_str(), backup_dst.c_str(), 0)) {
				last_error = 0;
			} else {
				tries++;
				last_error = GetLastError();
				if (last_error != ERROR_ALREADY_EXISTS) {
					LogMessage(L"Failed to rename " + rename_dst + L" to "	+ backup_dst + L" - " + FormatError(GetLastError()));
					return false;
				}
			}
		} while (last_error == ERROR_ALREADY_EXISTS);

		LogMessage(L"Renamed " + rename_dst + L" to " + backup_dst);
	}

	BOOL result = MoveFileEx(rename_src.c_str(), rename_dst.c_str(), 0);
	if (result) {
		LogMessage(L"Renamed " + rename_src + L" to " + rename_dst);
		return true;
	} else {
		LogMessage(L"Failed to rename " + rename_src + L" to " + rename_dst + L" - " + FormatError(GetLastError()));
		return false;
	}
}

std::vector<std::wstring> ReadModID(std::wstring folder_name) 
{
	std::vector<std::wstring> id_file_array;
	std::wstring path_to_id = folder_name + L"\\__gs_id";
	std::ifstream mod_id_file(path_to_id.c_str(), std::ios::in);
	
	if (mod_id_file.is_open()) {
		std::string id_file_text((std::istreambuf_iterator<char>(mod_id_file)), std::istreambuf_iterator<char>());
		Tokenize(utf16(id_file_text), L";", id_file_array);
		mod_id_file.close();
	}
	
	return id_file_array;
}

void SelectFileInExplorer(std::wstring path_to_file)
{
	path_to_file = global.working_directory + path_to_file;

	ITEMIDLIST *pIDL = ILCreateFromPath(path_to_file.c_str());
	if (pIDL != NULL) {
		HRESULT result = CoInitialize(NULL);
		UNREFERENCED_PARAMETER(result);

		if (SHOpenFolderAndSelectItems(pIDL, 0, 0, 0) != S_OK)
			ShellExecute(NULL, L"open", PathNoLastItem(path_to_file).c_str(), NULL, NULL, SW_SHOWDEFAULT);

		CoUninitialize();
		ILFree(pIDL);
	}
}

void ProcessArguments(LPWSTR command_line, INPUT_ARGUMENTS &input)
{
	std::vector<std::wstring> argv;
	Tokenize(command_line, L" ", argv);

	for (size_t i=0; i<argv.size(); i++) {
		std::wstring namevalue = (std::wstring)argv[i];
		size_t separator       = namevalue.find_first_of(L'=');
	
		if (separator != std::wstring::npos) {
			std::wstring name  = namevalue.substr(0,separator);
			std::wstring value = namevalue.substr(separator+1);
		
			if (Equals(name,L"-mod")) {
				std::vector<std::wstring> mods_name_list;
				Tokenize(value, L";", mods_name_list);
			
				for (size_t j=0; j<mods_name_list.size(); j++) {
					MODLIST new_item = {mods_name_list[j], L"", L"", L"", false};
					input.mods.push_back(new_item);
				}
				
				continue;
			}
		
			if (Equals(name,L"-modid")) {
				std::vector<std::wstring> mods_id_list;
				Tokenize(value, L";", mods_id_list);
			
				for (size_t j=0; j<mods_id_list.size(); j++) {
					MODLIST new_item = {L"", L"", mods_id_list[j], L"", false};
					input.mods.push_back(new_item);
				}
				
				continue;
			}

			if (Equals(name,L"-pbo")) {
				input.PBOaddon = value;
				continue;
			}
		
			if (Equals(name,L"-pid")) {
				input.game_pid = _wtoi(value.c_str()); 
				continue;
			}
		
			if (Equals(name,L"-run")) {
				input.game_exe = value;
				continue;
			}
		
			if (Equals(name,L"-serverequalmodreq")) {
				input.server_equalmodreq = Equals(value,L"true") || Equals(value,L"1");
				continue;
			}
		
			if (Equals(name,L"-serveruniqueid")) {
				input.server_uniqueid = value;
				continue;
			}

			if (Equals(name,L"-updateresource")) {
				input.update_resource = value;
				continue;
			}

			if (Equals(name,L"-selfupdate")) {
				if (Equals(value,L"true") || Equals(value,L"1"))
					input.self_update = SELF_UPDATE_AND_START_GAME;
				continue;
			}

			if (Equals(name,L"-econnect") || Equals(name,L"-eport") || Equals(name,L"-epassword")) {
				if (Equals(name,L"-econnect")) {
					input.ip = Decrypt(value);
				}
			
				if (Equals(name,L"-eport")) {
					input.port = Decrypt(value);
				}
			
				input.user_arguments     += L"-" + name.substr(2) + L"=" + Decrypt(value) + L" ";
				input.user_arguments_log += L"-" + name.substr(2) + L"=hidden ";
				continue;
			}
		
			if (Equals(name,L"-evoice")) {
				if (value.substr(0,12) == L"ts3server://") {
					std::wstring ip    = value.substr(12);
					std::wstring query = L"";
					size_t query_pos   = ip.find(L"?");
				
					// Decrypt query string
					if (query_pos != std::wstring::npos) {
						query = ip.substr(query_pos+1);
						ip    = ip.substr(0, query_pos);
					
						std::vector<std::wstring> query_string_array;
						Tokenize(query, L"&", query_string_array);
						query = L"";
					
						for (size_t j=0; j<query_string_array.size(); j++) {
							std::vector<std::wstring> var;
							Tokenize(query_string_array[j], L"=", var);
						
							if (var.size() > 1) {
								if (Equals(var[0],L"password"))
									var[1] = Decrypt(var[1]);
								
								if (Equals(var[0],L"channelpassword"))
									var[1] = Decrypt(var[1]);
								
								query += (query!=L"" ? L"&" : L"") + var[0] + L"=" + var[1];
							} else
								if (var.size() > 0)
									query += var[0];
						}
					}

					input.voice_server = L"ts3server://" + Decrypt(ip) + (query!=L"" ? L"?" : L"") + query;
				}
				
				if (value.substr(0,9) == L"mumble://")
					input.voice_server = L"mumble://" + Decrypt(value.substr(9));
				
				if (value.substr(0,19) == L"https://discord.gg/")
					input.voice_server = L"https://discord.gg/" + Decrypt(value.substr(19));
			
				if (value.substr(0,20) == L"https://s.team/chat/")
					input.voice_server = L"https://s.team/chat/" + Decrypt(value.substr(20));
				
				continue;
			}
		
			if (Equals(name,L"-voice")) {
				input.voice_server = value;
				continue;
			}
		
			if (Equals(name,L"-maxcustom")) {
				input.maxcustom = value;
				continue;
			}
		
			if (Equals(name,L"-plrname")) {
				input.username = value;
				continue;
			}
		
			if (Equals(name,L"-connect")) {
				input.ip = value;
			}
		
			if (Equals(name,L"-port")) {
				input.port = value;
			}

			if (Equals(name,L"-eventurl")) {
				input.event_url = value;
				continue;
			}

			if (Equals(name,L"-eventvoice")) {
				input.event_voice = _wtoi(value.c_str());
				continue;
			}

			if (Equals(name,L"-eventtask")) {
				input.event_task_name = value;
				continue;
			}

			if (Equals(name,L"-steam")) {
				input.steam = Equals(value,L"true") || Equals(value,L"1");
				continue;
			}

			if (Equals(name,L"-skipmemarg")) {
				input.skip_memory_arguments = Equals(value,L"true") || Equals(value,L"1");
				continue;
			}
		}
	
		input.user_arguments     += namevalue + L" ";
		input.user_arguments_log += namevalue + L" ";
	}
}

void readStringFromBinaryFile(FILE *f, std::wstring &destination) 
{
	std::string buffer = "";
	size_t length = 0;
	fread(&length, sizeof(length), 1, f);
	for(size_t i=0; i<length; i++) {
		int c = fgetc(f);
		if (c != EOF)
			buffer += (char)c;
	}
	destination = utf16(buffer);
}

bool WTS_OpenTask(WINDOWS_TASK_SCHEDULER &wts, std::wstring task_name) 
{
	ZeroMemory(&wts.trigger, sizeof(TASK_TRIGGER));
	wts.trigger.cbTriggerSize = sizeof(TASK_TRIGGER);

	wts.task_name         = task_name;
	wts.COM_initialized   = false;
	wts.scheduler         = NULL;
	wts.task              = NULL;
	wts.trigger_interface = NULL;
	wts.comment           = NULL;
	wts.result            = CoInitialize(NULL);

	if (SUCCEEDED(wts.result)) {
		wts.COM_initialized = true;
		wts.result = CoCreateInstance(CLSID_CTaskScheduler, NULL, CLSCTX_INPROC_SERVER, IID_ITaskScheduler, (void **)&wts.scheduler);
		if (FAILED(wts.result)) {
			WTS_Error(wts, L"CoInitialize");
			return false;
		}
	} else {
		WTS_Error(wts, L"CoCreateInstance");
		return false;
	}

	wts.result = wts.scheduler->Activate(task_name.c_str(), IID_ITask, (IUnknown**)&wts.task);
	if (FAILED(wts.result)) return WTS_Error(wts, L"ITaskScheduler::Activate");

	wts.result = wts.task->GetComment(&wts.comment);
	if (FAILED(wts.result)) return WTS_Error(wts, L"ITask::GetComment");

	WORD trigger_count = 0;
	wts.result = wts.task->GetTriggerCount(&trigger_count);
	if (FAILED(wts.result)) return WTS_Error(wts, L"ITask::GetTriggerCount");

	if (trigger_count == 0) {
		std::wstring message = L"Windows Task Scheduler error. There are no triggers for this task";
		MessageBox(NULL, message.c_str(), L"Scheduled OFP Launch", MB_OK | MB_ICONSTOP);
		LogMessage(message, OPTION_CLOSELOG);
		WTS_CloseTask(wts);
		return false;
	}

	wts.result = wts.task->GetTrigger(0, &wts.trigger_interface);
	if (FAILED(wts.result)) return WTS_Error(wts, L"ITask::GetTrigger");

	wts.result = wts.trigger_interface->GetTrigger(&wts.trigger);
	if (FAILED(wts.result)) return WTS_Error(wts, L"ITaskTrigger::GetTrigger");

	return true;
}

bool WTS_SaveTask(WINDOWS_TASK_SCHEDULER &wts, TASK_TRIGGER &new_trigger)
{
	bool output = false;
	wts.result = wts.trigger_interface->SetTrigger(&new_trigger);

	if (SUCCEEDED(wts.result)) {
		IPersistFile *job_file = NULL;
		wts.result = wts.task->QueryInterface(IID_IPersistFile, (void **)&job_file);

		if (SUCCEEDED(wts.result)) {
			wts.result = job_file->Save(NULL, TRUE);

			if (SUCCEEDED(wts.result))
				output = true;
		}

		job_file->Release();
	}

	return output;
}

void WTS_CloseTask(WINDOWS_TASK_SCHEDULER &wts) 
{
	if (wts.comment != NULL) CoTaskMemFree(wts.comment);
	if (wts.trigger_interface != NULL) wts.trigger_interface->Release();
	if (wts.task != NULL) wts.task->Release();
	if (wts.scheduler != NULL) wts.scheduler->Release();
	if (wts.COM_initialized) {CoUninitialize(); wts.COM_initialized=false;};
}

bool WTS_Error(WINDOWS_TASK_SCHEDULER &wts, std::wstring input_description)
{
	if (FAILED(wts.result)) {
		_com_error err(wts.result);
		std::wstring message = L"There was a problem with accessing task from the Windows Task Scheduler\r\n\r\nTask name: " + wts.task_name + L"\r\nOperation: " + input_description + L"\r\n0x" + Hex2StrW(wts.result) + L" - " + (std::wstring)err.ErrorMessage();
		MessageBox(NULL, message.c_str(), L"Scheduled OFP Launch", MB_OK | MB_ICONSTOP);
		LogMessage(message, OPTION_CLOSELOG);
		WTS_CloseTask(wts);
		return false;
	} else
		return true;
}

DWORD ReadLocalMods(std::vector<MODLIST> &local_mods)
{
	WIN32_FIND_DATA fd;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	hFind        = FindFirstFile(L"*", &fd);

	if (hFind == INVALID_HANDLE_VALUE)
		return GetLastError();

	do {
		if (wcscmp(fd.cFileName,L".")==0 || wcscmp(fd.cFileName,L"..")==0 || !(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			continue;

		std::wstring folder_name = (std::wstring)fd.cFileName;
		std::vector<std::wstring> id_file_array = ReadModID(folder_name);
				
		if (id_file_array.size() >= MOD_SIZE) {
			MODLIST item;
			item.folder_name = folder_name;
			item.real_name   = id_file_array[MOD_NAME];
			item.id          = id_file_array[MOD_ID];
			item.version     = id_file_array[MOD_VER];
			item.force_name  = Equals(id_file_array[MOD_FORCENAME],L"true") || Equals(id_file_array[MOD_FORCENAME],L"1");
			local_mods.push_back(item);
		}
	} while (FindNextFile(hFind, &fd));

	FindClose(hFind);
	return 0;
}

std::wstring FormatMessageArray(std::vector<std::wstring> &message_list, int type)
{
	std::wstring output = L"";

	for (size_t i=0; i<message_list.size(); i++) {
		if (i > 0)
			output += (type==OPTION_MESSAGEBOX ? L"\r\n\r\n": L"\r\n");

		output += message_list[i];
	}

	return output;
}

std::wstring FormatSystemTime(SYSTEMTIME &date, int type)
{
	std::wstring output =
		Int2StrW(date.wYear) + L"." + 
		Int2StrW(date.wMonth, OPTION_LEADINGZERO) + L"." + 
		Int2StrW(date.wDay, OPTION_LEADINGZERO) + L" ";

	if (type == OPTION_LOGFILE)
		output += L" ";

	output += 
		Int2StrW(date.wHour, OPTION_LEADINGZERO) + L":" + 
		Int2StrW(date.wMinute, OPTION_LEADINGZERO);

	if (type == OPTION_LOGFILE)
		output += L":" + Int2StrW(date.wSecond, OPTION_LEADINGZERO);
		
	return output;
}