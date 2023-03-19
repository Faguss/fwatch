// gameRestart by Faguss (ofp-faguss.com) for Fwatch v1.16
// Program restarting Operation Flashpoint game

#include <fstream>		// file operations
#include <windows.h>	// winapi
#include <tlhelp32.h>	// process/module traversing
#include <unistd.h>     // for access command
#include <sstream>      // for converting int to string
#include <vector>       // dynamic array
#include <algorithm>	// tolower
#include <Shlobj.h>		// opening explorer
#include <iostream>		// cout

#define USING_LOLE32 1

using namespace std;

struct GLOBAL_VARIABLES 
{
	string downloaded_filename;
	string working_directory;
	ofstream logfile;
} global = {
	"",
	""
};

struct game_info
{
	HWND handle;
	DWORD pid;
};

enum GAME_EXE_LIST
{
	EXE,
	TITLE
};

enum MOD_LIST
{
	NAME,
	ID,
	ARRAY_SIZE
};




// **************** FUNCTIONS **************************************************

// Compare characters of two strings case insensitive
// http://my.fit.edu/~vkepuska/ece5527/sctk-2.3-rc1/src/rfilter1/include/strncmpi.c
int strncmpi(const char *ps1, const char *ps2, int n)
{
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


// Translate error code to message
string FormatError(int error)
{
	if (error == 0) 
		return "\n";
	
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		error, 
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &lpMsgBuf,
		0,
		NULL
	);

	string message = "   - " + (string)(char*)lpMsgBuf + "\n";
	return message;
};

// https://stackoverflow.com/questions/11635/case-insensitive-string-comparison-in-c#315463
bool Equals(const string& a, const string& b) 
{
    unsigned int sz = a.size();

    if (b.size() != sz)
        return false;

    for (unsigned int i = 0; i < sz; ++i)
        if (tolower(a[i]) != tolower(b[i]))
            return false;

    return true;
}


// Function returns fwatch process ID or -1 if failed
int get_process_id(string exename)
{
	PROCESSENTRY32 processInfo;
	int pid 				 = 0;
	processInfo.dwSize 		 = sizeof(processInfo);
	HANDLE processesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	
	if (processesSnapshot != INVALID_HANDLE_VALUE) {
		Process32First(processesSnapshot, &processInfo);
		
		do {
			if (strcmpi(processInfo.szExeFile,exename.c_str()) == 0)
				pid = processInfo.th32ProcessID;
		}
		while (Process32Next(processesSnapshot, &processInfo));

		CloseHandle(processesSnapshot);
	}; 
    
    return pid; 
};


// Find OFP window by name and/or process id
game_info find_game_instance(DWORD input_pid, string input_name)
{
    game_info info = {NULL, 0};
	HWND handle    = GetTopWindow(NULL);
	
	if (handle) {
		char current_window[1024] = "";
		DWORD current_pid 	      = 0;
	
		while (handle) {
			GetWindowText(handle, current_window, 1023);
			GetWindowThreadProcessId(handle, &current_pid);
	
			bool match_name = strncmpi(current_window, input_name.c_str(), input_name.length()) == 0;
			bool match_pid  = input_pid == current_pid;
	
			if (
				(input_pid!=0 && !input_name.empty() && match_name && match_pid) || 
				(input_pid!=0 && input_name.empty() && match_pid) ||
				(input_pid==0 && !input_name.empty() && match_name)
			) {
				info = (game_info){handle, current_pid};
				break;
			}
			
			handle = GetNextWindow(handle, GW_HWNDNEXT);
		}
	} else
		global.logfile << "Couldn't get top window" << FormatError(GetLastError());
	
	return info;
};



// Format string (convert number to string and add leading zero)
string LeadingZero(int number)
{
	string ret = "";
	
	if (number < 10) 
		ret += "0";
		
	stringstream temp;
	temp << number;
	ret += temp.str();
	
	return ret;
};


// Read process parameters and copy them to a string
int GetProcessParams(int pid, HANDLE *phandle, string moduleName, int offset, vector<string> &container)
{    
    // Get address of wanted module
	MODULEENTRY32 xModule;
	int baseOffset = 0;
	HANDLE hSnap   = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
	
	if (hSnap != INVALID_HANDLE_VALUE) {
		xModule.dwSize = sizeof(MODULEENTRY32);

		do {
			if (lstrcmpi(xModule.szModule, (LPCTSTR)moduleName.c_str()) == 0)
			{
				// Read module base address
				baseOffset = (int)xModule.modBaseAddr;
				break;
			}
		} while (Module32Next(hSnap, &xModule));
		
		CloseHandle(hSnap);
		
		if (baseOffset != 0) {		    
            // Find offset holding arguments
			int pointer[]  = {baseOffset+offset, 0, 0};
			int modif[]    = {0x0, 0x0};
			int max_loops  = sizeof(pointer) / sizeof(pointer[0]) - 1;
			SIZE_T stBytes = 0;
		
			for (int i=0; i<max_loops; i++) {
                ReadProcessMemory(*phandle, (LPVOID)pointer[i], &pointer[i+1], 4, &stBytes);
				pointer[i+1] = pointer[i+1] +  modif[i];
			};
			
			char buffer[1024] = "";
			ReadProcessMemory(*phandle, (LPVOID)pointer[max_loops], &buffer, 1023, &stBytes);
		
			// Add arguments to vector
			string word = "";
			bool ignore_first_word = true;

			for (int i=0, nullTerminators=0;  i<1024;  i++)
				if (buffer[i] == '\0') {
					nullTerminators++;
					
					if (nullTerminators == 2) 
						break;
						
					if (!word.empty()) {
						size_t isexe = word.find_last_of(".exe");
						if (ignore_first_word  &&  isexe==string::npos) {
							ignore_first_word = false;
						}
						
						if (ignore_first_word)
							ignore_first_word = false;
						else
							container.push_back(word);
						
						word = "";
					}
		        } else {
					word += buffer[i];
					nullTerminators = 0;
				}

            return 0;
        }
        else
            return -1;
    }
    else
		return GetLastError();
};


// Leave information for OFP about schedule
void WriteSaveStateFile(string input)
{
    ofstream file;
    file.open ("fwatch\\tmp\\schedule\\restart_info.sqf", ios::out | ios::app);
    
    if (!file.is_open())
        return;
    
    file << input;
    file.close();
}

	// http://stackoverflow.com/a/3418285
string ReplaceAll(string str, const string& from, const string& to) 
{
    if (from.empty())
        return str;
        
    size_t start_pos = 0;
    
    while ((start_pos = str.find(from, start_pos)) != string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
    
    return str;
}


#define FNV_PRIME 16777619u
#define FNV_BASIS 2166136261u

enum OPTIONS_FNVHASH {
	OPTION_LOWERCASE = 1
};

//https://stackoverflow.com/questions/11413860/best-string-hashing-function-for-short-filenames
unsigned int fnv1a_hash(unsigned int hash, const char *text, int text_length, bool lowercase) {
    for (int i=0; i<text_length; i++)
		hash = (hash ^ (lowercase ? tolower(text[i]) : text[i])) * FNV_PRIME;

    return hash;
}

struct BinarySearchResult {
	size_t index;
	bool found;
};

// Search for an unsigned int in a string buffer
BinarySearchResult binary_search_str(const char *buffer, size_t array_size, unsigned int value_to_find, size_t low, size_t high) {
	if (array_size  &&  high>=low) {
		size_t mid        = low + (high - low) / 2;
		size_t *mid_value = (size_t*)(buffer + mid*sizeof(size_t));

		if (*mid_value == value_to_find) {
			BinarySearchResult out = {mid,1};
			return out;
		} else
			if (*mid_value > value_to_find) {
				if (mid > 0)
					return binary_search_str(buffer, array_size, value_to_find, low, mid-1);
				else {
					// target key should be at the start of the array
					BinarySearchResult out = {low, 0};
					return out;
				}
			} else
				if (mid < array_size-1)
					return binary_search_str(buffer, array_size, value_to_find, mid+1, high);
				else {
					// target key should be at the end of the array
					BinarySearchResult out = {mid+1, 0};
					return out;
				}
	}
	
	BinarySearchResult out = {low,0};
	return out;
}

string ReadStartupParams(string key_name)
{
	ifstream file("fwatch\\tmp\\schedule\\params.bin", ios::in | ios::binary);
	string file_contents;
	string output = "";
	size_t buffer_size = 0;
  
	if (file) {
		file.seekg(0, ios::end);
		buffer_size = file.tellg();
		file_contents.resize(buffer_size);
		file.seekg(0, ios::beg);
		file.read(&file_contents[0], file_contents.size());
		file.close();
	} else 
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
	} key_pointer, value_pointer;
	
	const char *buffer = file_contents.c_str();
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
	BinarySearchResult key_to_read = binary_search_str(buffer+sizeof(igsedb_header), header->number_of_keys, fnv1a_hash(FNV_BASIS, key_name.c_str(), key_name.length(), OPTION_LOWERCASE), 0, header->number_of_keys-1);
	
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

	// Read
	for (int i=0; i<value_pointer.end-1-value_pointer.start; i++)
		output += (buffer + value_pointer.start)[i];

	return output;
}


	// Remove quotation marks
string UnQuote(string text)
{
	if (text.substr(text.length()-1) == "\"")
		text = text.substr(0, text.length()-1);
	
	if (text.substr(0,1) == "\"")
		text = text.substr(1);
		
	return text;	
}



void Tokenize(string text, string delimiter, vector<string> &container)
{
	bool first_item = false;
	bool inQuote    = false;
	
	// Split line into parts
	for (int pos=0, begin=-1;  pos<=text.length();  pos++) {
		bool isToken = pos == text.length();
		
		for (int i=0;  !isToken && i<delimiter.length();  i++)
			if (text.substr(pos,1) == delimiter.substr(i,1))
				isToken = true;
				
		if (text.substr(pos,1) == "\"")
			inQuote = !inQuote;
			
		// Mark beginning of the word
		if (!isToken  &&  begin<0)
			begin = pos;
						
		// Mark end of the word
		if (isToken  &&  begin>=0  &&  !inQuote) {
			string part = UnQuote(text.substr(begin, pos-begin));
			container.push_back(part);
			begin = -1;
		}
	}
}

string Int2Str(int num)
{
    ostringstream text;
    text << num;
    return text.str();
}


string Trim(string s)
{
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
	s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
	return s;
}


string PathLastItem(string path) 
{
	size_t lastSlash = path.find_last_of("\\/");

	if (lastSlash != string::npos)
		return path.substr(lastSlash+1);
	else
		return path;
}


string PathNoLastItem(string path, bool end_slash=true) 
{
	size_t find = path.find_last_of("\\/");

	if (find != string::npos)
		return path.substr(0, find+end_slash);
	else
		return "";
}


string WrapInQuotes(string text)
{
	for (int i=0; i<text.length(); i++)
		if (text.substr(i,1) == " ") {
			text = "\"" + text + "\"";
			break;
		}
	
	return text;
}


	// Read wget log to get information about download
int ParseWgetLog(string &error)
{
	fstream DownloadLog;
    DownloadLog.open("fwatch\\tmp\\schedule\\downloadLog.txt", ios::in);

	if (DownloadLog.is_open()) {
		string line                  = "";
		string filesize              = "";
		string size_downloaded       = "";
		string percentage_downloaded = "";
		string download_speed        = "";
		string time_remaining        = "";
		
		const int filename_messages_items = 4;
		vector<string> filename_messages[filename_messages_items];
		filename_messages[0].push_back("Saving to: '");
		filename_messages[0].push_back("'");
		filename_messages[1].push_back(") - '");
		filename_messages[1].push_back("' saved [");
		filename_messages[2].push_back("File '");
		filename_messages[2].push_back("' already there; not retrieving");
		filename_messages[3].push_back("Server file no newer than local file '");
		filename_messages[3].push_back("' -- not retrieving");
			
		vector<string> error_messages;
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

				if (open!=string::npos  &&  close!=string::npos)
					filesize = line.substr( open+1, close-open-1);
			}

			// Get progress bar
			size_t letter_k = line.find("K .");

			if (letter_k == string::npos)
				letter_k = line.find("K ,");
			
			if (letter_k != string::npos) {			
				size_downloaded = line.substr(0, letter_k);
				int size_num    = atoi(size_downloaded.c_str());
				
				if (size_num > 1024) {
					double megabytes = size_num / (1024);
					char temp[128] = "";
					sprintf(temp, "%.0f M", megabytes);
					size_downloaded = (string)temp;
				} else
					size_downloaded += " K";
				
				string new_download_speed = "";
				size_t percent            = line.find("% ");
				if (percent != string::npos) {
					while(percent>=0  &&  (line[percent]=='%'  ||  isdigit(line[percent])))
						percent--;
						
					vector<string> Tokens;
					Tokenize(line.substr(percent), " =", Tokens);
					
					if (Tokens.size() > 0)
						percentage_downloaded = Tokens[0];
						
					if (Tokens.size() > 1) {
						new_download_speed = Tokens[1];
					}
						
					if (Tokens.size() > 2)
						time_remaining = Tokens[2];
				} else {
					int i = letter_k + 3;
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
				
				if (begin != string::npos) {
					int len      = filename_messages[i][0].length();
					size_t end   = line.find(filename_messages[i][1], begin+len);
					
					if (end != string::npos) {
						global.downloaded_filename = line.substr(begin+len,  end-(begin+len));
						break;
					}
				}
			}

			// Get error message
			for (int i=0; i<error_messages.size(); i++) {
				size_t search = line.find(error_messages[i]);
				if (search != string::npos) {
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



	// Read 7za log to get information about unpacking
int ParseUnpackLog(string &error, string &file_name)
{
	fstream UnpackLog;
    UnpackLog.open ("fwatch\\tmp\\schedule\\unpackLog.txt", ios::in);

    int line_number      = 0;
    int error_until_line = 0;
	string error_msg   = "";

	if (UnpackLog.is_open()) {
		string text         = "";
		string current_file = "";
		string percentage   = "";
		bool foundFileName  = false;

		while(getline(UnpackLog, text)) {
			text = Trim(text);

			if (text.empty())
				continue;

			line_number++;

			// Get progress percentage
			size_t percent = text.find_last_of("%");

			if (percent != string::npos) {
				if (percent < 2)
					percent = 0;
				else
					percent -= 2;

				percentage  = text.substr(percent, 3);
				size_t dash = text.find("- ");

				if (dash != string::npos)
					current_file = text.substr(dash+2);					
			}

			// Get error message
			size_t error_pos = text.find("ERROR:");

			if (error_pos != string::npos  &&  error=="") {
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


	// Delete file or directory with its contents  http://stackoverflow.com/a/10836193
int DeleteDirectory(const string &refcstrRootDirectory, bool bDeleteSubdirectories=true)
{
	bool            bSubdirectory = false;       // Flag, indicating whether subdirectories have been found
	HANDLE          hFile;                       // Handle to directory
	string     	    strFilePath;                 // Filepath
	string          strPattern;                  // Pattern
	WIN32_FIND_DATA FileInformation;             // File information

	strPattern = refcstrRootDirectory + "\\*.*";
	hFile      = FindFirstFile(strPattern.c_str(), &FileInformation);
	
	if (hFile != INVALID_HANDLE_VALUE) {
		do {
			if (FileInformation.cFileName[0] != '.') {
				strFilePath.erase();
				strFilePath = refcstrRootDirectory + "\\" + FileInformation.cFileName;

				if (FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
					if (bDeleteSubdirectories) {
						// Delete subdirectory
						int iRC = DeleteDirectory(strFilePath, bDeleteSubdirectories);
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
				if (refcstrRootDirectory != "fwatch\\tmp\\_extracted") {
					if (RemoveDirectory(refcstrRootDirectory.c_str()) == FALSE)
						return GetLastError();
				} else
					return 0;
			}
		}
	}

	return 0;
}


int Download(string url, string &error_text)
{
	// Format arguments
	global.downloaded_filename = PathLastItem(url);
	string arguments           = " --user-agent=\"Mozilla/5.0 (Windows NT 6.1; Win64; x64; rv:103.0) Gecko/20100101 Firefox/103.0\" --tries=1 --output-file=fwatch\\tmp\\schedule\\downloadLog.txt " + url;

	remove("fwatch\\tmp\\schedule\\downloadLog.txt");


				
	// Execute program
	PROCESS_INFORMATION pi;
    STARTUPINFO si; 
	ZeroMemory( &si, sizeof(si) );
	ZeroMemory( &pi, sizeof(pi) );
	si.cb 		   = sizeof(si);
	si.dwFlags 	   = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOW;
	
	if (!CreateProcess("fwatch\\data\\wget.exe", &arguments[0], NULL, NULL, false, 0, NULL, NULL, &si, &pi)) {
		int errorCode = GetLastError();
		global.logfile << "Failed to launch wget.exe - " << errorCode << " " << FormatError(errorCode);
		error_text += "Failed to launch wget.exe - " + Int2Str(errorCode) + " " + FormatError(errorCode);
		return errorCode;
	} else
		global.logfile << "Downloading  " << url << endl;



	// Wait for the program to finish its job
	DWORD exit_code;
	string message = "";
	
	Sleep(10);
					
	do {					
		ParseWgetLog(message);
		GetExitCodeProcess(pi.hProcess, &exit_code);
		Sleep(100);
	}
	while (exit_code == STILL_ACTIVE);

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	
	ParseWgetLog(message);
	
	if (exit_code != 0) {
		global.logfile << "Failed to download " << global.downloaded_filename << " - " << exit_code << " - " << message << endl;
		error_text + "Failed to download " + global.downloaded_filename + " - " + Int2Str(exit_code) + " - " + message + "\n";
	}
	
	return exit_code;
}


int Unpack(string file_name, string password, string &error_text, bool tmp_dir=false)
{
	// Get subdirectories
	string relative_path = PathNoLastItem(file_name);

	if (Equals(relative_path.substr(0,10), "_extracted"))
		relative_path = relative_path.substr(11) += "_extracted";

	// Clean destination directory
	string destination = "fwatch\\tmp\\_extracted";
	
	if (relative_path != "")
		destination += "\\" + relative_path;

	DeleteDirectory(destination);


	// Create log file
	SECURITY_ATTRIBUTES sa;
    sa.nLength              = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle       = TRUE;       

    HANDLE logFile = CreateFile(TEXT("fwatch\\tmp\\schedule\\unpackLog.txt"),
        FILE_APPEND_DATA,
        FILE_SHARE_WRITE | FILE_SHARE_READ,
        &sa,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL );
        
	// Execute program
	PROCESS_INFORMATION pi;
    STARTUPINFO si; 
	ZeroMemory( &si, sizeof(si) );
	ZeroMemory( &pi, sizeof(pi) );
	si.cb 		     = sizeof(si);
	si.dwFlags 	     = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	si.wShowWindow   = SW_SHOW;
	si.hStdInput     = NULL;
	si.hStdOutput    = logFile;
	si.hStdError     = logFile;
	string arguments = WrapInQuotes(global.working_directory) + (password.empty() ? "" : " -p"+password) + " x -y -bb3 -bsp1 " + (tmp_dir ? ("-ofwatch\\tmp\\_extracted ") : "") + file_name;

	if (!CreateProcess("fwatch\\data\\7z.exe", &arguments[0], NULL, NULL, true, 0, NULL, NULL, &si, &pi)) {		
		int errorCode = GetLastError();
		global.logfile << "Failed to launch 7z.exe - " << errorCode << " " << FormatError(errorCode);
		error_text += "Failed to launch 7z.exe - " + Int2Str(errorCode) + FormatError(errorCode);
		return errorCode;
	} else
		global.logfile << "Extracting " << file_name << endl;
		
	Sleep(10);


	// Wait for the program to finish its job
	DWORD exit_code;
	string message = "";

	do {					
		ParseUnpackLog(message, file_name);
		GetExitCodeProcess(pi.hProcess, &exit_code);
		Sleep(100);
	}
	while (exit_code == STILL_ACTIVE);

	ParseUnpackLog(message, file_name);

	if (exit_code != 0) {
		global.logfile << "Failed to extract " << file_name << " - " << exit_code << " - " << message << endl;
		error_text += "Failed to extract " + file_name + " - " + Int2Str(exit_code) + " - " + message;
	}

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	CloseHandle(logFile);

	return exit_code;
}


	// https://superuser.com/questions/475874/how-does-the-windows-rename-command-interpret-wildcards
string MaskNewName(string path, string mask) 
{
	if (mask.empty())
		return path;
	
	if (path.empty())
		return "";
		
	int    x = 0;
	string R = "";
	
	for (int m=0; m<mask.length(); m++) {
		char ch       = mask[m];
		bool q_exists = x<path.length();
		char q        = q_exists          ? path[x]   : ' ';
		char z        = m<mask.length()-1 ? mask[m+1] : ' ';
		
		if (ch!='.'  &&  ch!='*'  &&  ch!='?') {
			if (q_exists  &&  q!='.')
				x++;
			R += ch;
        } else if (ch == '?') {
            if (q_exists  &&  q!='.') {
				R += q;
				x++;
			}
        } else if (ch == '*'   &&   m == mask.length()-1) {
            while (x < path.length()) 
				R += path[x++];
        } else if (ch == '*') {
            if (z == '.') {
                int i = path.length()-1;
                for (;  i>=0;  i--) 
					if (path[i] == '.') 
						break;
						
                if (i < 0) {
                    R += path.substr(x, path.length()) + '.';
                    i  = path.length();
                } else {
					R += path.substr(x, i - x + 1);
					x = i + 1;
					m++;
				}
				
            } else if (z == '?') {
                R += path.substr(x, path.length());
				m++;
				x  = path.length();
            } else {
                int i = path.length()-1;
                for (;  i>=0;  i--) 
					if (path[i] == z) 
						break;
						
                if (i < 0) {
					R += path.substr(x, path.length()) + z;
					x = path.length();
					m++;
				} else {
					R += path.substr(x, i - x);
					x = i + 1;
				}
            }
        } else if (ch == '.') {
            while (x < path.length()) 
				if (path[x++] == '.') 
					break;
					
            R += '.';
        }
    }
	
    while (R[R.length() - 1] == '.') 
		R = R.substr(0, R.length() - 1);
		
	return R;
}

int CreateFileList(string source, string destination, vector<string> &sources, vector<string> &destinations, vector<bool> &dirs, bool is_move, vector<string> &empty_dirs, int &buffer_size, bool match_dirs)
{
	WIN32_FIND_DATA fd;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	hFind        = FindFirstFile(source.c_str(), &fd);
	int result   = 0;

	if (hFind == INVALID_HANDLE_VALUE) {
		int errorCode = GetLastError();
		global.logfile << "Failed to list files in " << source << "  - " << errorCode << " " + FormatError(errorCode) << endl;
		return errorCode;
	}

	string base_source      = PathNoLastItem(source);
	string base_destination = PathNoLastItem(destination);
	string new_name         = PathLastItem(destination);

	if (new_name.empty())
		new_name = PathLastItem(source);

	bool is_source_wildcard      = source.find("*")!=string::npos    ||  source.find("?")!=string::npos;
	bool is_destination_wildcard = new_name.find("*")!=string::npos  ||  new_name.find("?")!=string::npos;

	do {
		if (fd.cFileName[0] == '.')
			continue;
		
		string file_name       = string(fd.cFileName);
		string new_source      = base_source      + file_name;
		string new_destination = base_destination + (is_destination_wildcard ? MaskNewName(file_name,new_name) : new_name);
		bool   is_dir          = fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
		int attributes         = GetFileAttributes(new_destination.c_str());
		
		if (is_dir  &&  is_source_wildcard  &&  !match_dirs)
			continue;

		// If we need full paths and their totaled length
		if (buffer_size != 0) {
			new_destination = global.working_directory + "\\" + new_destination;
			buffer_size    += new_destination.length() + 1;
		}


		// Check if destination directory already exists
		if (is_dir  &&  (attributes != INVALID_FILE_ATTRIBUTES  &&  attributes & FILE_ATTRIBUTE_DIRECTORY  ||  !is_move)  &&  buffer_size==0) {
			if (!is_move)
				CreateDirectory(new_destination.c_str(), NULL);
			else
				empty_dirs.push_back(new_source);

			// If dir already exists then move its contents
			new_source      += "\\*";
			new_destination += "\\";
			result           = CreateFileList(new_source, new_destination, sources, destinations, dirs, is_move, empty_dirs, buffer_size, match_dirs);
			if (result != 0)
				break;
		} else {
			sources     .push_back(new_source);
			destinations.push_back(new_destination);
			dirs        .push_back(is_dir);
		}
	}
	while (FindNextFile(hFind, &fd) != 0);

	FindClose(hFind);
	return result;
}


int MoveFiles(string source, string destination, string new_name, bool is_move, bool overwrite, bool match_dirs, string &error_text)
{
	// Find files and save them to a list
	vector<string> source_list;
	vector<string> destination_list;
	vector<bool>   is_dir_list;
	vector<string> empty_dirs;
	int buffer_size = 0;
	int final_result = 0;
	
	int result = CreateFileList(source, destination+new_name, source_list, destination_list, is_dir_list, is_move, empty_dirs, buffer_size, match_dirs);

	if (result != 0)
		return result;
	

	DWORD flags       = MOVEFILE_REPLACE_EXISTING;
	bool FailIfExists = false;
	int return_value  = 0;

	if (!overwrite) {
		FailIfExists = true;
		flags        = 0;
	}


	// For each file in the list
	for (int i=0;  i<source_list.size(); i++) {
		// Format path for logging
		string destinationLOG = PathNoLastItem(destination_list[i], 0);

		if (destinationLOG.empty())
			destinationLOG = "the game folder";
		
		global.logfile << (is_move ? "Moving" : "Copying") << "  " << ReplaceAll(source_list[i], "fwatch\\tmp\\_extracted\\", "") << "  to  " << destinationLOG;
		
		if (!new_name.empty())
			global.logfile << "  as  " << PathLastItem(destination_list[i]);

		if (is_move)
			result = MoveFileEx(source_list[i].c_str(), destination_list[i].c_str(), flags);
		else
			result = CopyFile(source_list[i].c_str(), destination_list[i].c_str(), FailIfExists);

	    if (result == 0) {
			return_value = GetLastError();
			
	    	/*if (!overwrite  &&  return_value==183)
	    		global.logfile << "  - file already exists";
			else */{
				final_result = 1;
				global.logfile << "  FAILED " << return_value << " " << FormatError(return_value);
				error_text += "Failed to move " + source_list[i] + FormatError(return_value);
			}
		}
			
		global.logfile << endl;
	}
	
	if (source_list.size() == 0)
		final_result = 1;


	// Remove empty directories
	if (is_move)
		for (int i=empty_dirs.size()-1; i>=0; i--)
			RemoveDirectory(empty_dirs[i].c_str());
	
	return final_result;
}

string Decrypt(string sentence) 
{
	int decrypt_key = 0;
	int modulus_key = 0;
	vector<int> numbers;
	vector<string> words;
	Tokenize(sentence, "a", words);
	string decrypted = "";

	for (int i=0; i<words.size(); i++)	{
		string digits = "";
		
		for (int j=0; j<words[i].length(); j++) {
			char digit = words[i][j];
			digits += Int2Str(digit-98);
		}
		
		numbers.push_back(atoi(digits.c_str()));
	}
	
	for (int i=0; i<numbers.size(); i++) {
		int result = 1;
		int key    = decrypt_key;
		
		while(key > 0) {
			if (key %2 == 1)
				result = (result * numbers[i]) % modulus_key;
				
			numbers[i] = numbers[i] * numbers[i] % modulus_key;
			key        = key / 2;
		}
		
		char letter = result;
		decrypted += letter;
		
	}
	
	return decrypted;
}
// *****************************************************************************


















// **************** MAIN PROGRAM ***********************************************

int main(int argc, char *argv[])
{
	// Set working directory to the game root folder -----------
	TCHAR pwd[MAX_PATH];
	GetCurrentDirectory(MAX_PATH,pwd);
	global.working_directory = ReplaceAll((string)pwd, "\\fwatch\\data", "");
	SetCurrentDirectory(global.working_directory.c_str());





	// Process arguments ---------------------------------------
	GetCurrentDirectory(MAX_PATH, pwd);
	
	string user_arguments     = " ";
	string user_arguments_log = " ";
	string game_exe           = "";
	string server_uniqueid    = "";
	string PBOaddon   		  = "";
	string voice_server       = "";
	string server_time        = "";
	string update_resource    = "";
	string username           = "";
	string maxcustom          = "";
	string ip                 = "";
	string port               = "";
	
	bool query_server         = false;
	bool self_update		  = false;
	bool server_equalmodreq   = false;
	DWORD game_pid            = 0;
	vector<string> required_mods[2];
	
	for (int i=1; i<argc; i++) {
		string namevalue = (string)argv[i];
		size_t separator = namevalue.find_first_of('=');
		
		if (separator != string::npos) {
			string name  = namevalue.substr(0,separator);
			string value = namevalue.substr(separator+1);
			
			if (Equals(name,"-mod")) {
				vector<string> temp_array;
				Tokenize(value, ";", temp_array);
				
				for (int i=0; i<temp_array.size(); i++)
					required_mods[NAME].push_back(temp_array[i]);
					
				continue;
			}
			
			if (Equals(name,"-modid")) {
				vector<string> temp_array;
				Tokenize(value, ";", temp_array);
				
				for (int i=0; i<temp_array.size(); i++)
					required_mods[ID].push_back(temp_array[i]);
					
				continue;
			}

			if (Equals(name,"-pbo")) {
				PBOaddon = value;
				continue;
			}
			
			if (Equals(name,"-pid")) {
				game_pid = atoi(value.c_str()); 
				continue;
			}
			
			if (Equals(name,"-run")) {
				game_exe = value;
				continue;
			}
			
			if (Equals(name,"-serverequalmodreq")) {
				server_equalmodreq = Equals(value,"true") || Equals(value,"1");
				continue;
			}
			
			if (Equals(name,"-serveruniqueid")) {
				server_uniqueid = value;
				continue;
			}
			
			if (Equals(name,"-servertime")) {
				server_time = value;				
				continue;
			}

			if (Equals(name,"-updateresource")) {
				update_resource = value;
				continue;
			}

			if (Equals(name,"-selfupdate")) {
				self_update = Equals(value,"true") || Equals(value,"1");
				continue;
			}

			if (Equals(name,"-econnect") || Equals(name,"-eport") || Equals(name,"-epassword")) {
				if (Equals(name,"-econnect")) {
					ip = Decrypt(value);
				}
				
				if (Equals(name,"-eport")) {
					port = Decrypt(value);
				}
				
				user_arguments     += "-" + name.substr(2) + "=" + Decrypt(value) + " ";
				user_arguments_log += "-" + name.substr(2) + "=hidden ";
				continue;
			}
			
			if (Equals(name,"-evoice")) {
				if (value.substr(0,12) == "ts3server://") {
					string ip        = value.substr(12);
					string query     = "";
					size_t query_pos = ip.find("?");
					
					// Decrypt query string
					if (query_pos != string::npos) {
						query = ip.substr(query_pos+1);
						ip    = ip.substr(0, query_pos);
						
						vector<string> query_string_array;
						Tokenize(query, "&", query_string_array);
						query = "";
						
						for (int i=0; i<query_string_array.size(); i++) {							
							vector<string> var;
							Tokenize(query_string_array[i], "=", var);
							
							if (var.size() > 1) {
								if (Equals(var[0],"password"))
									var[1] = Decrypt(var[1]);
									
								if (Equals(var[0],"channelpassword"))
									var[1] = Decrypt(var[1]);
									
								query += (query!="" ? "&" : "") + var[0] + "=" + var[1];
							} else
								if (var.size() > 0)
									query += var[0];
						}
					}

					voice_server = "ts3server://" + Decrypt(ip) + (query!="" ? "?" : "") + query;
				}
					
				if (value.substr(0,9) == "mumble://")
					voice_server = "mumble://" + Decrypt(value.substr(9));
					
				if (value.substr(0,19) == "https://discord.gg/")
					voice_server = "https://discord.gg/" + Decrypt(value.substr(19));
				
				if (value.substr(0,20) == "https://s.team/chat/")
					voice_server = "https://s.team/chat/" + Decrypt(value.substr(20));
					
				continue;
			}
			
			if (Equals(name,"-voice")) {
				voice_server = value;
				continue;
			}
			
			if (Equals(name,"-maxcustom")) {
				maxcustom = value;
				continue;
			}
			
			if (Equals(name,"-plrname")) {
				username = value;
				continue;
			}
			
			if (Equals(name,"-queryserver")) {
				query_server = Equals(value,"true") || Equals(value,"1");
				continue;
			}
			
			if (Equals(name,"-connect")) {
				ip = value;
			}
			
			if (Equals(name,"-port")) {
				port = value;
			}
		}
		
		user_arguments     += namevalue + " ";
		user_arguments_log += namevalue + " ";
	}
	
	// Open discord/steam url
	if (voice_server.substr(0,19) == "https://discord.gg/" || voice_server.substr(0,20) == "https://s.team/chat/") {
		ShellExecute(NULL, "open", voice_server.c_str(), NULL, NULL, SW_SHOWNORMAL);
		return 0;
	}

	// Download server info
	if (query_server) {
		if (ip.empty() || Equals(ip,"localhost") || ip.substr(0,8)=="192.168." || ip=="127.0.0.1") {
			return 1;
		}
			
		string filename   = "fwatch\\tmp\\schedule\\queryserver" + server_uniqueid + ".txt";		
		string url        = "--output-document=" + filename + " https://ofp-api.ofpisnotdead.com/" + ip + ":" + (port.empty() ? "2302" : port);
		string error_text = "";
		int result        = Download(url, error_text);
		
		if (result != 0) {
			return 2;
		}

		ifstream file(filename.c_str(), ios::in);
		string contents;
		
		if (!file) {
			return 3;
		}
	  
		file.seekg(0, ios::end);
		contents.resize(file.tellg());
		file.seekg(0, ios::beg);
		file.read(&contents[0], contents.size());
		file.close();
			
		if (contents == "{\"error\":\"timeout\"}") {
			return 4;
		}
		
		vector<string> to_find;
		to_find.push_back("\"gstate\":");
		to_find.push_back("\"numplayers\":");
		to_find.push_back("\"gametype\":");
		to_find.push_back("\"mapname\":");
		vector<string> output;
		cout << "[";
		
		for (int i=0; i<to_find.size(); i++) {
			string value = "";
			size_t start = contents.find(to_find[i]);
			
			if (start != string::npos) {
				start     += to_find[i].length();
				size_t end = contents.find(",", start);
				
				if (end != string::npos)
					value = contents.substr(start, end-start);
			}
			
			if (i != 0)
				cout << ",";
			
			if (i==0 || i==1)
				value = ReplaceAll(value, "\"", "");
				
			cout << value;
		}
		
		string to_find2 = "\"player\":";
		string players  = "";
		size_t start    = contents.find(to_find2);
		
		while (start != string::npos) {
			start     += to_find2.length();
			size_t end = contents.find(",", start);
				
			if (end != string::npos) {				
				if (!players.empty())
					players += "\\n";
					
				players += contents.substr(start, end-start);
			}
			
			start = contents.find(to_find2, start+1);
		}
		
		cout << ",\"" << ReplaceAll(players, "\"", "") << "\"]";
		remove(filename.c_str());
		return 0;
	}




	// Create log file -----------------------------------------
	SYSTEMTIME st;
	GetLocalTime(&st);
	
	global.logfile.open("fwatch\\data\\gameRestartLog.txt", ios::out | ios::app);
	global.logfile << "\n--------------\n\n" 
			<< st.wYear << "." 
			<< LeadingZero(st.wMonth) << "." 
			<< LeadingZero(st.wDay) << "  " 
			<< LeadingZero(st.wHour) << ":" 
			<< LeadingZero(st.wMinute) << ":" 
			<< LeadingZero(st.wSecond) << endl;
	
	global.logfile << "Arguments: " << global.working_directory << user_arguments_log << endl;



	// If scheduled to restart at a specific time --------------
	if (!server_time.empty()) {
		FILETIME schedule;
		vector<string> date;
		Tokenize(server_time, "[,]", date);
		
		if (date.size() >= 8) {
			SYSTEMTIME st;
			st.wYear         = atoi(date[0].c_str());
			st.wMonth        = atoi(date[1].c_str());
			st.wDay          = atoi(date[2].c_str());
			st.wHour         = atoi(date[4].c_str());
			st.wMinute       = atoi(date[5].c_str());
			st.wSecond       = atoi(date[6].c_str());
			st.wMilliseconds = atoi(date[7].c_str());

			SystemTimeToFileTime(&st      , &schedule);
			FileTimeToSystemTime(&schedule, &st);						
			global.logfile << "Game is scheduled to restart at " << LeadingZero(st.wHour) << ":" << LeadingZero(st.wMinute) << endl;
			
			char processID[12] = "";
			sprintf(processID, "%d", GetCurrentProcessId());
			
			string temp = "_gamerestart_pid=" + (string)processID + ";";
			WriteSaveStateFile(temp);
		} else {
			global.logfile << "Incorrect date passed" << endl;
			global.logfile.close();
			return 1;
		}
				
        // Keep checking time
		SYSTEMTIME st;
		FILETIME now;
		fstream instructions;
		bool abort = false;
		int result = 0;

		do {
			// Read instructions from user
			instructions.open ("fwatch\\tmp\\schedule\\RestartInstruction.txt", ios::in);
			
			if (instructions.is_open()) {
				string text = "";
                while(getline(instructions, text)) {					
                    if (text == "abort") {
                        abort = true;
                    }
                }
						
				instructions.close();
			}
			
			if (abort) {
                WriteSaveStateFile("_gamerestart_pid=0;");
                DeleteFile("fwatch\\tmp\\schedule\\RestartInstruction.txt");
                global.logfile << "Scheduled restart aborted by user\n";
				global.logfile.close();
				return 1;
			}
					
			Sleep(1000);
			GetLocalTime(&st);
			
			SystemTimeToFileTime(&st, &now);
			result = CompareFileTime(&schedule, &now);
		}
		while (result >= 0);
		
		// Make not everybody connect at once
		Sleep(rand() % 5000);

		WriteSaveStateFile("_gamerestart_pid=0;");
	}





	// Detect which game and executable -----------------------
	enum GAME_VERSION {
		VER_UNKNOWN,
		VER_196,
		VER_199,
		VER_201
	};

	string exe_name_list[] = {
		"armaresistance.exe",
		"coldwarassault.exe",
		"flashpointresistance.exe",
		"ofp.exe",
		"flashpointbeta.exe",
		"operationflashpoint.exe",
		"operationflashpointbeta.exe",
		"armaresistance_server.exe",
		"coldwarassault_server.exe",
		"ofpr_server.exe"
	};
	
	string window_name_list[] = {
		"ArmA Resistance",
		"Cold War Assault",
		"Operation Flashpoint",
		"Operation Flashpoint",
		"Operation Flashpoint",
		"Operation Flashpoint",
		"Operation Flashpoint",
		"ArmA Resistance Console",
		"Cold War Assault Console",
		"Operation Flashpoint Console"
	};
	
	int exe_version_list[] = {
		VER_201,
		VER_199,
		VER_196,
		VER_196,
		VER_196,
		VER_196,
		VER_196,
		VER_201,
		VER_199,
		VER_196
	};
	
	int exe_num    = sizeof(exe_version_list) / sizeof(exe_version_list[0]);
	int window_num = sizeof(window_name_list) / sizeof(window_name_list[0]);

	string game_window    = "";
	bool got_handle       = false;
	bool dedicated_server = false;
	game_info game;
	int game_version      = VER_196;
	
	for (int i=0; i<exe_num; i++)
		if (!game_exe.empty()) {		// if executable name is known then match corresponding window name
			if (Equals(exe_name_list[i],game_exe)) {
				game_window  = window_name_list[i];
				game_version = exe_version_list[i];
				dedicated_server = i>=7;
				break;
			}
		} else if (i<exe_num-3) {	// if executable name is not known then search for process with corresponding window name
			game = find_game_instance(game_pid, window_name_list[i]);
			
			if (game.pid != 0) {
				game_exe     = exe_name_list[i];
				game_window  = window_name_list[i];
				game_version = exe_version_list[i];
				got_handle   = true;
				break;
			}
		}


	// If exe is not known or if there's no game running then check existing executables in the game directory
	if (game_window.empty())
		for (int i=0; i<exe_num-3; i++)
			if (GetFileAttributes(exe_name_list[i].c_str()) != INVALID_FILE_ATTRIBUTES) {
				game_exe     = exe_name_list[i];
				game_window  = window_name_list[i];
				game_version = exe_version_list[i];
				break;
			}

	// If nothing was found
	if (game_window.empty()) {
		global.logfile << "Can't find game executable\n";
		global.logfile.close();
		return 2;
	}
	
	

	// Get game window handle
	if (!got_handle)
		game = find_game_instance(game_pid, game_window);
	
	

	
	
	


















    // Is fwatch -nolaunch?
	int fwatch_pid          = 0;
	bool nolaunch           = false;
	bool steam              = false;
	string fwatch_arguments = " ";
	fstream fwatch_info;
	fwatch_info.open("fwatch_info.sqf", ios::in);

	if (fwatch_info.is_open()) {
	    string data_line;
		getline(fwatch_info, data_line);
		vector<string> data_array;
		Tokenize(data_line, "[,]\" ", data_array);
		
			for (int i=0; i<data_array.size(); i++) {
				if (i==0)
					fwatch_pid = atoi(data_array[i].c_str());
				else {
					vector<string> param_array;
					Tokenize(data_array[i], " ", param_array);
					
					for (int j=0; j<param_array.size(); j++) {
						if (Equals(param_array[j],"-nolaunch"))
							nolaunch = true;
						
						if (Equals(param_array[j],"-steam")) {
							nolaunch = true;
							steam    = true;
						}
						
						if (
							Equals(param_array[j].substr(0,9),"-gamespy=") ||
							Equals(param_array[j].substr(0,9),"-udpsoft=") ||
							Equals(param_array[j].substr(0,5),"-run=") ||
							Equals(param_array[j],"-reporthost") ||
							Equals(param_array[j],"-removenomap")
						)
							fwatch_arguments += param_array[j] + " ";
					}
				}
			}		
		
		fwatch_info.close();
	}
	
	
	
	
		
	// Access game process
	string filtered_game_arguments = " ";

	/*if (!nolaunch)
		filtered_game_arguments += fwatch_arguments;*/

	if (game.handle) {
		HANDLE game_handle = OpenProcess(PROCESS_ALL_ACCESS, 0, game.pid);
		
		// Read game parameters
		if (game.handle != 0) {
			vector<string> all_game_arguments;
			char module_name[32] = "ifc22.dll";
			
			if (dedicated_server)
				strcpy(module_name, "ijl15.dll");
			
			int result = GetProcessParams(game.pid, &game_handle, module_name, dedicated_server ? 0x4FF20 : 0x2C154, all_game_arguments);
			if (result == 0) {
				string log_arguments = "";
				
				for (int i=0; i<all_game_arguments.size(); i++) {
					if (
						!Equals(all_game_arguments[i].substr(0,9),"-connect=") &&
						!Equals(all_game_arguments[i].substr(0,6),"-port=") &&
						!Equals(all_game_arguments[i].substr(0,10),"-password=")
					)
						log_arguments = log_arguments + all_game_arguments[i] + " ";	// log params except for the ones that could be hidden
					
					if (
						!Equals(all_game_arguments[i].substr(0,5),"-mod=") &&
						!Equals(all_game_arguments[i].substr(0,9),"-connect=") &&
						!Equals(all_game_arguments[i].substr(0,6),"-port=") &&
						!Equals(all_game_arguments[i].substr(0,10),"-password=")
					)
						filtered_game_arguments += all_game_arguments[i] + " ";
				}
				
				global.logfile << "Game arguments: " << log_arguments << endl;	
			} else {
				if (result == -1)
					global.logfile << "Couldn't find " << module_name << endl;
				else {
					string errorMSG = FormatError(GetLastError());
					global.logfile << "Can't get module list" << errorMSG;
				}
			}
			
			CloseHandle(game_handle);
		} else {
			global.logfile << "Can't access game process" << FormatError(GetLastError());
			global.logfile.close();
			return 3;
		}
				

		// Shutdown the game
		bool close = PostMessage(game.handle, WM_CLOSE, 0, 0);
		int tries  = 0;
			
		do {
			game = find_game_instance(game.pid, game_window);
			Sleep(500);
		} while (close && game.pid!=0 && ++tries<7);
			
		if (game.pid!=0) {
			game_handle = OpenProcess(PROCESS_ALL_ACCESS, 0, game.pid);
			
			if (TerminateProcess(game_handle, 0)) 
				global.logfile << "Game terminated\n"; 
			else {
				global.logfile << "Couldn't terminate the game" << FormatError(GetLastError());
				global.logfile.close();
				CloseHandle(game_handle);
				return 4;
			}

			CloseHandle(game_handle);
		} else
			global.logfile << "Game closed\n";
	} else
		global.logfile << "Can't find the game window\n";

		




	
	
	// Wait for Fwatch termination
	int tries = 0;
	while (!nolaunch  &&  get_process_id("fwatch.exe")) {
		Sleep(100);

		if (++tries > 50) {
			global.logfile << "Fwatch didn't quit" << endl;
			global.logfile.close();
			return 5;
		}
	}
	
	if (nolaunch) {
		if (!steam)
            global.logfile << "Fwatch -nolaunch" << endl;
        else
            global.logfile << "Fwatch -steam" << endl;
    } else 
		global.logfile << "Waited for Fwatch to quit" << endl;






	
	// Create a pbo file if ordered ----------------------------
	if (PBOaddon!=""  &&  PBOaddon.find("..\\")==string::npos) {
        string PBOexec = global.working_directory + "\\fwatch\\data\\MakePbo.exe";
		vector<string> PBOarg;
		
		PBOarg.push_back(global.working_directory + "\\@AddonTest\\ -NRK @AddonTest\\addons\\" + PBOaddon);
		PBOarg.push_back(global.working_directory + "\\@AddonTest\\ -NRK @AddonTest\\Campaigns\\AddonTest");
		
        for (int i=0; i<PBOarg.size(); i++) {
			global.logfile << PBOexec << endl << PBOarg[i] << endl;

			// Execute
			PROCESS_INFORMATION pi2;
			STARTUPINFO si2; 
			ZeroMemory(&si2, sizeof(si2));
			ZeroMemory(&pi2, sizeof(pi2));
			si2.cb 			= sizeof(si2);
			si2.dwFlags 	= STARTF_USESHOWWINDOW;
			si2.wShowWindow = SW_HIDE;
			
			if (!CreateProcess(&PBOexec[0], &PBOarg[i][0], NULL, NULL, false, 0, NULL, NULL, &si2, &pi2)) {
				global.logfile << "MakePBO failure" << FormatError(GetLastError());
			};

			DWORD st2;
			do {					
				GetExitCodeProcess(pi2.hProcess, &st2);
				Sleep(100);
			}
			while(st2 == STILL_ACTIVE);

			global.logfile << "MakePBO result: " << st2 << endl;
			CloseHandle(pi2.hProcess);
			CloseHandle(pi2.hThread);
		}
	}






	
	// Game schedule options -----------------------------------	
	if (required_mods[ID].size() > 0) {
		vector<string> mods[ARRAY_SIZE];
		vector<bool> force_name;
			
		WIN32_FIND_DATA fd;
		HANDLE hFind = INVALID_HANDLE_VALUE;
		hFind        = FindFirstFile("*", &fd);

		if (hFind == INVALID_HANDLE_VALUE) {
			global.logfile << "Failed to list files " << FormatError(GetLastError());
			global.logfile.close();
			return 6;
		}

		do {
			if (fd.cFileName[0] == '.'  ||  !(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				continue;

			string folder_name = (string)fd.cFileName;
			string path_to_id  = folder_name + "\\__gs_id";
			fstream mod_id_file;
			mod_id_file.open(path_to_id.c_str(), ios::in);
			
			if (mod_id_file.is_open()) {
				string data_line;
				getline(mod_id_file, data_line);
				mod_id_file.close();

				vector<string> data;
				Tokenize(data_line, ";", data);

				if (data.size() > 0) {
					mods[NAME].push_back(folder_name);
					mods[ID].push_back(data[0]);
					
					bool is_force = false;
					
					if (data.size() >= 5) {
						if (Equals(data[4],"true") || Equals(data[4],"1")) {
							is_force = true;
							
							for (int i=0; i<required_mods[ID].size(); i++)
								if (Equals(data[0],required_mods[ID][i]))
									required_mods[NAME][i] = data[3];
						}
					}
					
					force_name.push_back(is_force);
				}
			}
		} while (FindNextFile(hFind, &fd) != 0);
		FindClose(hFind);

		bool add_param_name = true;

		for (int i=0; i<required_mods[ID].size(); i++) {		// for each required mod id
			for (int j=0; j<mods[ID].size(); j++) {				// find matching id within a list of existing modfolders
				if (required_mods[ID][i] == mods[ID][j]) {
					if ((server_equalmodreq || force_name[j])   &&   !Equals(required_mods[NAME][i],mods[NAME][j])) {	// if there's a name conflict					
						if (GetFileAttributes(required_mods[NAME][i].c_str()) != INVALID_FILE_ATTRIBUTES) {	// if so then rename to something else
							string rename_src = required_mods[NAME][i];
							string rename_dst = "";
							int tries         = 1;
							int last_error    = 0;
							
							do {
								rename_dst = required_mods[NAME][i] + "_renamed" + (tries>1 ? Int2Str(tries) : "");
								if (MoveFileEx(rename_src.c_str(), rename_dst.c_str(), 0))
									last_error = 0;
								else {
									tries++;
									last_error = GetLastError();
									if (last_error != 183) {
										global.logfile << "Failed to rename " << rename_src << " to " << rename_dst << " - " << FormatError(last_error);
										global.logfile.close();
										return 7;
									}
								}
							} while (last_error == 183);
							
							global.logfile << "Renamed " << rename_src << " to " << rename_dst << endl;
						}
						
						// now rename the actual required mod to its proper name
						if (rename(mods[NAME][j].c_str(), required_mods[NAME][i].c_str()) == 0) {
							global.logfile << "Renamed " << mods[NAME][j] << " to " << required_mods[NAME][i] << endl;
							mods[NAME][j] = required_mods[NAME][i];
						} else {
							global.logfile << "Failed to rename " << mods[NAME][j] << " to " << required_mods[NAME][i] << " - " << FormatError(GetLastError());
							global.logfile.close();
							return 7;
						}
					}

					if (add_param_name) {
						user_arguments     += "-mod=";
						user_arguments_log += "-mod=";
						add_param_name  = false;
					} else
						user_arguments += ";";
					
					user_arguments     += mods[NAME][j];
					user_arguments_log += mods[NAME][j];
				}
			}
		}
	} else
		for (int i=0; i<required_mods[NAME].size(); i++) {
			string path_to_id  = required_mods[NAME][i] + "\\__gs_id";
			fstream mod_id_file;
			mod_id_file.open(path_to_id.c_str(), ios::in);

			if (mod_id_file.is_open()) {
				string data_line;
				getline(mod_id_file, data_line);
				mod_id_file.close();
				vector<string> data;
				Tokenize(data_line, ";", data);

				// Force original modfolder name
				if (data.size() >= 5) {
					if (Equals(data[4],"true")  &&  !Equals(required_mods[NAME][i],data[3])) {
						if (GetFileAttributes(data[3].c_str()) != INVALID_FILE_ATTRIBUTES) {
							string rename_src = data[3];
							string rename_dst = "";
							int tries         = 1;
							int last_error    = 0;
							
							do {
								rename_dst = data[3] + "_renamed" + (tries>1 ? Int2Str(tries) : "");
								if (MoveFileEx(rename_src.c_str(), rename_dst.c_str(), 0))
									last_error = 0;
								else {
									tries++;
									last_error = GetLastError();
									if (last_error != 183) {
										global.logfile << "Failed to rename " << rename_src << " to " << rename_dst << " - " << FormatError(last_error);
										global.logfile.close();
										return 7;
									}
								}
							} while (last_error == 183);
							
							global.logfile << "Renamed " << rename_src << " to " << rename_dst << endl;
						}
						
						// now rename the actual required mod to its proper name
						if (rename(required_mods[NAME][i].c_str(), data[3].c_str()) == 0) {
							global.logfile << "Renamed " << required_mods[NAME][i] << " to " << data[3] << endl;
							required_mods[NAME][i] = data[3];
						} else {
							global.logfile << "Failed to rename " << required_mods[NAME][i] << " to " << data[3] << " - " << FormatError(GetLastError());
							global.logfile.close();
							return 7;
						}
					}
				}
			}

			user_arguments     += (i==0 ? "-mod=" : ";") + required_mods[NAME][i];
			user_arguments_log += (i==0 ? "-mod=" : ";") + required_mods[NAME][i];
		}
		
	user_arguments     += " ";
	user_arguments_log += " ";
	

	// Read from a file user's startup parameters for this server
	if (!server_uniqueid.empty()) {
		string params = ReadStartupParams(server_uniqueid);
		
		if (server_equalmodreq) {
			vector<string> params_array;
			Tokenize(params, " ", params_array);
			
			for (int i=0; i<params_array.size(); i++) {
				if (Equals(params_array[i].substr(0,5),"-mod="))
					continue;
					
				user_arguments     += params_array[i] + " ";
				user_arguments_log += params_array[i] + " ";
			}
		} else
			user_arguments     += params + " ";
			user_arguments_log += params + " ";
	}





	
	// Self-update ---------------------------------------------
	if (self_update) {
		char url[]        = "http://ofp-faguss.com/fwatch/116test";
		string error_text = "";
		int result        = 0;
		
		if (nolaunch) {
			if (fwatch_pid == 0)
				fwatch_pid = get_process_id("fwatch.exe");
			
			HANDLE fwatch_handle = OpenProcess(PROCESS_ALL_ACCESS, 0, fwatch_pid);
			
			if (fwatch_handle == NULL) {
				int last_error  = GetLastError();
				string errorMSG = FormatError(last_error);
				global.logfile << "Failed to access Fwatch process " << last_error << errorMSG << endl;
				global.logfile.close();
				error_text += "Failed to access Fwatch process " + Int2Str(last_error) + errorMSG;
				
				if (last_error == 5)
					error_text += "\n\nSet gameRestart.exe to run as admin and try again";
				else
					error_text += "\n\nYou have to download and update manually";
				
				int msgboxID = MessageBox(NULL, error_text.c_str(), "Fwatch self-update", MB_OK | MB_ICONSTOP);
				
				if (last_error == 5) {
					CHAR pwd[MAX_PATH];
					GetCurrentDirectory(MAX_PATH,pwd);				
					string path_to_file = (string)pwd + "\\fwatch\\data\\gameRestart.exe";
					
					#if USING_LOLE32 == 1
						ITEMIDLIST *pIDL = ILCreateFromPath(path_to_file.c_str());
						if (pIDL != NULL) {
							CoInitialize(NULL);
						    if (SHOpenFolderAndSelectItems(pIDL, 0, 0, 0) != S_OK)
						    	ShellExecute(NULL, "open", pwd, NULL, NULL, SW_SHOWDEFAULT);
							CoUninitialize();
						    ILFree(pIDL);
						}
					#else
						ShellExecute(NULL, "open", pwd, NULL, NULL, SW_SHOWDEFAULT);
					#endif
				}
				else
					ShellExecute(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);
				
				return 1;
			}
			
			bool terminated = TerminateProcess(fwatch_handle, 0);
			CloseHandle(fwatch_handle);
			
			if (!terminated) {
				int last_error  = GetLastError();
				string errorMSG = FormatError(last_error);
				global.logfile << "Failed to close Fwatch " << last_error << errorMSG << endl;
				global.logfile.close();
				error_text += "Failed to close Fwatch " + Int2Str(last_error) + errorMSG;
				error_text += "\n\nYou have to download and update manually";
				int msgboxID = MessageBox(NULL, error_text.c_str(), "Fwatch self-update", MB_OK | MB_ICONSTOP);
				ShellExecute(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);
				return 1;
			}
		}
			
		Sleep(1000);

		DeleteFile("fwatch\\data\\libeay32.dll");
		DeleteFile("fwatch\\data\\libiconv2.dll");
		DeleteFile("fwatch\\data\\libintl3.dll");
		DeleteFile("fwatch\\data\\libssl32.dll");
		DeleteFile("fwatch\\data\\sortMissions.exe");

		string file_name = "fwatch\\tmp\\schedule\\schedule.bin";
		if (!DeleteFile(file_name.c_str())) {
			string errorMSG = FormatError(GetLastError());
			global.logfile << "Failed to delete " << file_name << errorMSG;
			error_text += "Failed to delete " + file_name + errorMSG;
		}
		
		string rename_src = "fwatch\\data\\gameRestart.exe";
		string rename_dst = "fwatch\\data\\gameRestart_old.exe";
		DeleteFile(rename_dst.c_str());
		result = MoveFileEx(rename_src.c_str(), rename_dst.c_str(), MOVEFILE_REPLACE_EXISTING);
		if (result == 0) {
			int last_error = GetLastError();
			global.logfile << "Failed to rename " << rename_src << " to " << rename_dst << FormatError(last_error) << endl;
			error_text += "Failed to rename " + rename_src + " to " + rename_dst + FormatError(last_error);
			global.logfile.close();
			error_text += "\n\nYou have to download and update manually";
			int msgboxID = MessageBox(NULL, error_text.c_str(), "Fwatch self-update", MB_OK | MB_ICONSTOP);
			ShellExecute(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);
			return result;
		}
		
		
		// Try update from two mirrors
		vector<string> download_mirrors;
		download_mirrors.push_back("http://ofp-faguss.com/fwatch/download/fwatch_self_update.7z");
		download_mirrors.push_back("http://faguss.paradoxstudio.uk/fwatch/download/fwatch_self_update.7z");
		
		for (int i=0; i<download_mirrors.size(); i++) {
			result = Download(download_mirrors[i], error_text);
			
			if (result != 0 && i==download_mirrors.size()-1) {
				global.logfile << "Download failed\n\n--------------\n\n";
				global.logfile.close();
				error_text += "\n\nYou have to download and update manually";
				int msgboxID = MessageBox(NULL, error_text.c_str(), "Fwatch self-update", MB_OK | MB_ICONSTOP);
				ShellExecute(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);
				return result;
			}
			
			// Delete files manually to make sure they're not being accessed
			result = 0;
			int tries = 4;
			
			do {
				if (!DeleteFile("fwatch.dll")) {
					result = GetLastError();
					if (result == 2)
						result = 0;
					if (result != 0) {
						Sleep(500);
						tries--;
					}
					global.logfile << "Delete fwatch.dll " << result << endl;
				} else {
					global.logfile << "Delete fwatch.dll success" << endl;
				}
			} while (result != 0 && tries>=0);
			
			if (result != 0) {
				global.logfile << "Delete failed\n\n--------------\n\n";
				global.logfile.close();
				error_text += "Failed to delete fwatch.dll " + FormatError(result);
				int msgboxID = MessageBox(NULL, error_text.c_str(), "Fwatch self-update", MB_OK | MB_ICONSTOP);
				ShellExecute(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);
				return result;
			}
			
			result = Unpack(global.downloaded_filename, "fwatch", error_text);
			DeleteFile(global.downloaded_filename.c_str());
			
			if (result == 0)
				break;
			
			if (result != 0 && i==download_mirrors.size()-1) {
				global.logfile << "Unpacking failed\n\n--------------\n\n";
				global.logfile.close();
				
				if (error_text.find("Can not open the file as") != string::npos)
					error_text += "\n\nYou have to download and update manually";
				else
					error_text += "\n\nYou have to unpack files manually";
				
				int msgboxID = MessageBox(NULL, error_text.c_str(), "Fwatch self-update", MB_OK | MB_ICONSTOP);
				
				if (error_text.find("Can not open the file as") != string::npos) {
					ShellExecute(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);
				} else {
					CHAR pwd[MAX_PATH];
					GetCurrentDirectory(MAX_PATH,pwd);				
					string path_to_file = (string)pwd + "\\" + global.downloaded_filename;
					
					#if USING_LOLE32 == 1
						ITEMIDLIST *pIDL = ILCreateFromPath(path_to_file.c_str());
						if (pIDL != NULL) {
							CoInitialize(NULL);
						    if (SHOpenFolderAndSelectItems(pIDL, 0, 0, 0) != S_OK)
						    	ShellExecute(NULL, "open", pwd, NULL, NULL, SW_SHOWDEFAULT);
							CoUninitialize();
						    ILFree(pIDL);
						};
					#else
						ShellExecute(NULL, "open", pwd, NULL, NULL, SW_SHOWDEFAULT);
					#endif
				}
				
				return result;			
			}
		}
		
		
		if (steam) {
			MessageBox(NULL, "Update complete. Start the Fwatch again", "Fwatch self-update", MB_OK | MB_ICONINFORMATION);
			global.logfile << "Operation successful\n\n--------------\n\n";
			global.logfile.close();	
			return 0;
		}
				
		if (nolaunch) {
			Sleep(500);
			PROCESS_INFORMATION pi;
		    STARTUPINFO si; 
			ZeroMemory( &si, sizeof(si) );
			ZeroMemory( &pi, sizeof(pi) );
			si.cb 		   = sizeof(si);
			si.dwFlags 	   = STARTF_USESHOWWINDOW;
			si.wShowWindow = SW_SHOW;
		
			string launch_exe = global.working_directory + "\\" + "fwatch.exe";
			string launch_arg = " " + global.working_directory + " -nolaunch " + fwatch_arguments;
			
			CreateProcess(&launch_exe[0], &launch_arg[0], NULL, NULL, false, 0, NULL, NULL, &si, &pi);
			CloseHandle(pi.hProcess);
		    CloseHandle(pi.hThread);
		    Sleep(500);
		}
	} else 
		DeleteFile("fwatch\\data\\gameRestart_old.exe");
		
	if (update_resource != "") {
		string error_text = "";
		char url[]        = "http://ofp-faguss.com/fwatch/116test";
		int result        = 0;
		
		vector<string> download_mirrors2;
		download_mirrors2.push_back("http://ofp-faguss.com/fwatch/download/ofp_aspect_ratio207.7z");
		download_mirrors2.push_back("http://faguss.paradoxstudio.uk/fwatch/download/ofp_aspect_ratio207.7z");
		
		for (int i=0; i<download_mirrors2.size(); i++) {
			result = Download(download_mirrors2[i], error_text);
			if (result == 0)
				break;
		}
		
		if (result != 0) {
			global.logfile << "Download failed\n\n--------------\n\n";
			global.logfile.close();
			error_text += "\n\nYou have to download and update manually";
			int msgboxID = MessageBox(NULL, error_text.c_str(), "Resource update", MB_OK | MB_ICONSTOP);
			ShellExecute(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL);
			return result;
		}
				
		result = Unpack(global.downloaded_filename, "", error_text, true);
		DeleteFile(global.downloaded_filename.c_str());
		
		if (result != 0) {
			global.logfile << "Unpacking failed\n\n--------------\n\n";
			global.logfile.close();
			int msgboxID = MessageBox(NULL, error_text.c_str(), "Resource update", MB_OK | MB_ICONSTOP);		
			return result;			
		}
		
		string source = "fwatch\\tmp\\_extracted\\Files\\";
		string destination = "bin\\";
		
		if (update_resource == "1.96") {
			source += "OFP Resistance 1.96";
			destination = "Res\\bin\\";
		}
			
		if (update_resource == "1.99")
			source += "ArmA Cold War Assault 1.99";
			
		if (update_resource == "2.01")
			source += "ArmA Resistance 2.01";

		string destination_folder = destination;
		source += "\\Resource.cpp";
		destination += "Resource.cpp";
		int tries = 1;
		int last_error = 0;
		string destination_backup = "";
		
		do {
			destination_backup = destination_folder + "Resource_backup" + (tries>1 ? Int2Str(tries) : "") + ".cpp";
			if (MoveFileEx(destination.c_str(), destination_backup.c_str(), 0))
				last_error = 0;
			else {
				tries++;
				last_error = GetLastError();
				if (last_error != 183) {
					string message = "Failed to rename " + destination + " to " + destination_backup + " - " + FormatError(last_error);
					global.logfile << message;
					global.logfile.close();
					int msgboxID = MessageBox(NULL, message.c_str(), "Resource update", MB_OK | MB_ICONSTOP);		
					return 7;
				}
			}
		} while (last_error == 183);
		
		if (!MoveFileEx(source.c_str(), destination.c_str(), 0)) {
			string message = "Failed to move " + source + " to " + destination + " - " + FormatError(GetLastError());
			global.logfile << message;
			global.logfile.close();
			int msgboxID = MessageBox(NULL, message.c_str(), "Resource update", MB_OK | MB_ICONSTOP);		
			return 7;
		}
		
		if (steam) {
			MessageBox(NULL, "Update complete. Start the Fwatch again", "Fwatch self-update", MB_OK | MB_ICONINFORMATION);
			global.logfile << "Operation successful\n\n--------------\n\n";
			global.logfile.close();	
			return 0;
		}
				
		if (nolaunch) {
			Sleep(500);
			PROCESS_INFORMATION pi;
		    STARTUPINFO si; 
			ZeroMemory( &si, sizeof(si) );
			ZeroMemory( &pi, sizeof(pi) );
			si.cb 		   = sizeof(si);
			si.dwFlags 	   = STARTF_USESHOWWINDOW;
			si.wShowWindow = SW_SHOW;
		
			string launch_exe = global.working_directory + "\\" + "fwatch.exe";
			string launch_arg = " " + global.working_directory + " -nolaunch " + fwatch_arguments;
			
			CreateProcess(&launch_exe[0], &launch_arg[0], NULL, NULL, false, 0, NULL, NULL, &si, &pi);
			CloseHandle(pi.hProcess);
		    CloseHandle(pi.hThread);
		    Sleep(500);
		}
	}





	
	// Check for face textures in modfolders -------------------
	vector<string> mod_list;
	vector<string> arguments_split;
	Tokenize(user_arguments, " ", arguments_split);
	string player_name = "";
	
	// Get list of selected mods
	for (int i=0; i<arguments_split.size(); i++) {
		string namevalue = arguments_split[i];
		size_t separator = namevalue.find_first_of('=');
		
		if (separator != string::npos) {
			string name  = namevalue.substr(0,separator);
			string value = namevalue.substr(separator+1);
			
			if (Equals(name,"-mod")) {
				vector<string> temp_array;
				Tokenize(value, ";", temp_array);
				
				for (int i=0; i<temp_array.size(); i++)
					mod_list.push_back(temp_array[i]);
			}
			
			if (Equals(name,"-name")) {
				player_name = value;
			}
		}
	}
	
	// Before launching the game do the face texture replacement
	if (mod_list.size() > 0) {
		string mod_face;
		string user_face;
		string user_face_backup;
		string changelog;

		enum FACE_TYPES {
			NONE,
			PAA,
			JPG
		};

		int face_type = NONE;
		string face_extensions[] = {"paa", "jpg"};
		int face_extensions_length = sizeof(face_extensions) / sizeof(face_extensions[0]);

		// Find mod with custom face file starting from the last
		for (int i=mod_list.size()-1; i>=0 && face_type==NONE; i--) {
			for (int j=0; j<face_extensions_length; j++) {
				mod_face = mod_list[i] + "\\face." + face_extensions[j];

				if (GetFileAttributes(mod_face.c_str()) != 0xFFFFFFFF) {
					face_type = j + 1;

					// Need player name from the registry
					if (player_name.length() == 0) {
						HKEY key_handle = 0;
						char value[1024] = "";
						DWORD value_size = sizeof(value);
						LONG result      = RegOpenKeyEx(HKEY_CURRENT_USER, game_version==VER_199 ? "SOFTWARE\\Bohemia Interactive Studio\\ColdWarAssault" : "SOFTWARE\\Codemasters\\Operation Flashpoint", 0, KEY_READ, &key_handle);

						if (result == ERROR_SUCCESS) {
							DWORD data_type = REG_SZ;

							if (RegQueryValueEx(key_handle, "Player Name", 0, &data_type, (BYTE*)value, &value_size) == ERROR_SUCCESS)
								player_name = (string)value;
						}

						RegCloseKey(key_handle);

						if (player_name.length() == 0)
							break;
					}

					// Check if backup already exists
					bool already_exists = false;
					for (int k=0; k<face_extensions_length && !already_exists; k++) {
						user_face = "Users\\" + player_name + "\\face." + face_extensions[k] + "backup";

						if (GetFileAttributes(user_face.c_str()) != 0xFFFFFFFF)
							already_exists = true;
					}

					if (already_exists)
						break;

					// Backup current face
					for (int k=0; k<face_extensions_length; k++) {
						user_face        = "Users\\" + player_name + "\\face." + face_extensions[k];
						user_face_backup = user_face + "backup";

						if (MoveFileEx(user_face.c_str(), user_face_backup.c_str(), 0)) {
							changelog += user_face + "?" + user_face_backup + "\n";
						}
					}

					// Move the face from mod
					user_face = "Users\\" + player_name + "\\face." + face_extensions[j];

					fstream out;
					out.open("fwatch\\data\\user_rename.txt", ios::out | ios::app);

					if (MoveFileEx(mod_face.c_str(), user_face.c_str(), 0))
						out << mod_face << "?" << user_face << "\n";

					out << changelog;
					out.close();
					break;
				}
			}
		}
	}
	





	
	// Check if custom files are below limit -------------------
	if (!maxcustom.empty() && !username.empty()) {
		string changelog   = "";
		string path_to_cfg = "Users\\" + username + "\\UserInfo.cfg";
		int face_offset    = 0;
		char *face_ptr     = NULL;
		FILE *f            = fopen(path_to_cfg.c_str(),"rb");
		bool rewrite       = false;
		int file_size      = 0;
		double size_limit  = strtod(maxcustom.c_str(), NULL);
		char *file_buffer;
		HANDLE hFind;
		WIN32_FIND_DATA fd;
		
		if (f) {
			fseek(f, 0, SEEK_END);
			file_size = ftell(f);
			fseek(f, 0, SEEK_SET);
			file_buffer	        = new char[file_size+1];
			int result		    = fread(file_buffer, 1, file_size, f);
			file_buffer[result] = '\0';
			face_ptr            = strstr(file_buffer, "face=\"Custom\"");
		}
		
		// Get custom face size
		if (face_ptr != NULL) {
			string path_to_face = "Users\\" + username + "\\face.paa";
			double face_size    = 0;

			hFind = FindFirstFile(path_to_face.c_str(), &fd);
			if (hFind != INVALID_HANDLE_VALUE) {
				face_size = fd.nFileSizeLow;
				FindClose(hFind);
			} else {
				path_to_face = "Users\\" + username + "\\face.jpg";
				hFind        = FindFirstFile(path_to_face.c_str(), &fd);
				
				if (hFind != INVALID_HANDLE_VALUE) {
					face_size = fd.nFileSizeLow;
					FindClose(hFind);
				}
			}
			
			if (f && face_size > size_limit && face_size <= 102400) {
				memcpy(face_ptr, "face=\"Face52\"", 13);
				rewrite   = true;
				changelog = path_to_cfg + "\n";
			}			
		}
		
		if (f) {
			if (rewrite) {
				freopen(path_to_cfg.c_str(), "wb", f);
				if (f) {
					fwrite(file_buffer, 1, file_size, f);
					global.logfile << "Changed custom face to a default one due to server file limit" << endl;
				}
			}
			
			delete[] file_buffer;
			
			if (f)
				fclose(f);
		}
		
		// Get custom sound size
		string path_to_sound = "Users\\" + username + "\\sound\\*.*";
		hFind                = FindFirstFile(path_to_sound.c_str(), &fd);
		
		if (hFind != INVALID_HANDLE_VALUE) {
			do {
				if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					continue;

				// If above limit then move it out of the folder
				if (fd.nFileSizeLow > size_limit && fd.nFileSizeLow <= 51200) {
					string rename_src  = "Users\\" + username + "\\sound\\" + (string)fd.cFileName;
					string rename_base = "Users\\" + username + "\\" + (string)fd.cFileName;
					string rename_dst  = "";
					int tries          = 1;
					int last_error     = 0;
					
					do {
						rename_dst = rename_base + (tries>1 ? Int2Str(tries) : "");
						if (MoveFileEx(rename_src.c_str(), rename_dst.c_str(), 0)) {
							last_error = 0;
							changelog += rename_src + "?" + rename_dst + "\n";
						} else {
							tries++;
							last_error = GetLastError();
							if (last_error != 183) {
								global.logfile << "Failed to rename " << rename_src << " to " << rename_dst << " - " << FormatError(last_error);
								global.logfile.close();
								return 7;
							}
						}
					} while (last_error == 183);
					
					global.logfile << "Renamed " << rename_src << " to " << rename_dst << endl;
				}
			}
			while (FindNextFile(hFind, &fd) != 0);
			
			FindClose(hFind);
		}
		
		// Save to file what was renamed
		if (!changelog.empty()) {
			fstream out;
			out.open("fwatch\\data\\user_rename.txt", ios::out | ios::app);
			out << changelog;
			out.close();
		}
	}





	
	// Start a new game ----------------------------------------
	string launch_exe     = global.working_directory + "\\" + (nolaunch ? game_exe : "fwatch.exe");
	string launch_arg     = " \"" + global.working_directory + "\" " + filtered_game_arguments + user_arguments + " " + (nolaunch ? "" : fwatch_arguments);
	string launch_arg_log = " \"" + global.working_directory + "\" " + filtered_game_arguments + user_arguments_log + " " + (nolaunch ? "" : fwatch_arguments);	
	
	// Sort arguments to deal with the OFP arguments bug
	vector<string> temp_array;
	Tokenize(launch_arg, " ", temp_array);
	string with_equality    = "";
	string without_equality = "";

	for (int i=0; i<temp_array.size(); i++) {
		if (temp_array[i].find_first_of("=") != string::npos)
			with_equality += temp_array[i] + " ";
		else
			without_equality += temp_array[i] + " ";
	}
	
	launch_arg = without_equality + with_equality;
	
	temp_array.clear();
	Tokenize(launch_arg_log, " ", temp_array);
	with_equality    = "";
	without_equality = "";

	for (int i=0; i<temp_array.size(); i++) {
		if (temp_array[i].find_first_of("=") != string::npos)
			with_equality += temp_array[i] + " ";
		else
			without_equality += temp_array[i] + " ";
	}
	
	launch_arg_log = without_equality + with_equality;
	
	
	if (steam) {
		HKEY hKey			     = 0;
		char SteamPath[MAX_PATH] = {0};
		char SteamExe[MAX_PATH]	 = {0};
		DWORD dwType		     = 0;
		DWORD SteamPathSize	     = sizeof(SteamPath);
		DWORD SteamExeSize	     = sizeof(SteamExe);

		if (RegOpenKey(HKEY_CURRENT_USER,"Software\\Valve\\Steam",&hKey) == ERROR_SUCCESS) {
			dwType    = REG_SZ;
			bool key1 = RegQueryValueEx(hKey, "SteamPath", 0, &dwType, (BYTE*)SteamPath, &SteamPathSize) == ERROR_SUCCESS;
			bool key2 = RegQueryValueEx(hKey, "SteamExe",  0, &dwType, (BYTE*)SteamExe,  &SteamExeSize)	 == ERROR_SUCCESS;

			if (key1 && key2) {
				launch_exe = (string)SteamExe;
				launch_arg = " -applaunch 65790 ";
				
				if (filtered_game_arguments.find("-nomap")==string::npos && user_arguments.find("-nomap")==string::npos)
					launch_arg += " -nomap ";
					
				launch_arg += filtered_game_arguments + user_arguments;
			} else {
                string errorMSG = FormatError(GetLastError());
                global.logfile << "Couldn't read registry key" << errorMSG;
            }
		} else {
            string errorMSG = FormatError(GetLastError());
            global.logfile << "Couldn't open registry" << errorMSG;
        }

		RegCloseKey(hKey);
	}

	global.logfile << launch_exe << endl << launch_arg_log << endl;
	
	
	// Execute
	PROCESS_INFORMATION pi;
    STARTUPINFO si; 
	ZeroMemory( &si, sizeof(si) );
	ZeroMemory( &pi, sizeof(pi) );
	si.cb 		   = sizeof(si);
	si.dwFlags 	   = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOW;

	if (!CreateProcess(&launch_exe[0], &launch_arg[0], NULL, NULL, false, 0, NULL, NULL, &si, &pi)) {
		string errorMSG = FormatError(GetLastError());
		global.logfile << "Couldn't start the game" << errorMSG;
		MessageBox(NULL, "Failed to launch the game.\nPlease refer to the fwatch\\data\\gameRestartLog.txt.", "gameRestart", MB_OK|MB_ICONERROR );
		global.logfile.close();
		return 8;
	}
	
	CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread); 

	global.logfile << "Operation successful\n\n--------------\n\n";
	global.logfile.close();
	
	if (!voice_server.empty())
		ShellExecute(NULL, "open", voice_server.c_str(), NULL, NULL, SW_SHOWNORMAL);
	
	return 0;
};
