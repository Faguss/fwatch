// gameRestart by Faguss (ofp-faguss.com) for Fwatch v1.16
// Program restarting Operation Flashpoint game



	// Headers
#include <fstream>		// file operations
#include <windows.h>	// winapi
#include <tlhelp32.h>	// process/module traversing
#include <unistd.h>     // for access command
#include <sstream>      // for converting int to string
#include <vector>       // dynamic array
#include <algorithm>	// tolower
#include <sstream>		// converting int to string

using namespace std;

struct GLOBAL_VARIABLES 
{
	string downloaded_filename;
	string working_directory;
	ofstream logfile;
	HWND game_window_handle;
} global = {
	"",
	""
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


// Find OFP window only by name
int findOFPwindow(string window_wanted)
{
	DWORD pid          = 0;
	global.game_window_handle = FindWindow(NULL, window_wanted.c_str());
	
	if (global.game_window_handle != 0) 
		GetWindowThreadProcessId(global.game_window_handle, &pid);
		
	return pid;
};


// Find OFP window by name and process id
int findOFPwindow(string window_wanted, DWORD pid_wanted)
{
    global.game_window_handle = GetTopWindow(NULL);
	
	if (!global.game_window_handle) {
		global.logfile << "Couldn't get top window" << FormatError(GetLastError());
		return -1;
	}

	char current_window[1024] = "";
	string current_window2    = "";
	DWORD pid_current 	      = 0;

	while (global.game_window_handle) {
		GetWindowText(global.game_window_handle, current_window, 1023);
		GetWindowThreadProcessId(global.game_window_handle, &pid_current);	
		WINDOWINFO wi;

		current_window2 = (string)current_window;

		if (pid_current==pid_wanted  &&  Equals(current_window2,window_wanted))
			break;
		else
			pid_current = 0;
		
		global.game_window_handle = GetNextWindow(global.game_window_handle, GW_HWNDNEXT);
	};
	
	return pid_current;
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


string ReadStartupParams(string key_to_find)
{
	FILE *f = fopen("fwatch\\tmp\\schedule\\params.bin","rb");
	string output = "";
	
	if (!key_to_find.empty()  &&  f) {
		int existing_keys = 0;
		fread(&existing_keys, sizeof(existing_keys), 1, f);
		int *offsets = (int  *) malloc (existing_keys + 1);
		
		if (offsets != NULL) {
			offsets[0] = 0;
			fread(offsets+1, sizeof(*offsets), existing_keys, f);
			int existing_text_buffer_size = offsets[existing_keys];
			char *text_buffer = (char *) malloc (existing_text_buffer_size);
			
			if (text_buffer != NULL) {
				fread(text_buffer, sizeof(char), existing_text_buffer_size, f);
				char *key_name;
				int i = 0;
				
				for (; i<existing_keys; i++) {
					key_name = text_buffer + offsets[i];
					
					if (strcmpi(key_name, key_to_find.c_str()) == 0) {
						output        = text_buffer + offsets[i] + strlen(key_name) + 1;
						string tofind = "_Input=\"";
						size_t find   = output.find_first_of(tofind);

						if (find != string::npos)
							output = output.substr(find + tofind.length(), output.length() - tofind.length() - 1);

						break;
					}
				}

				free(text_buffer);
			}			
			
			free(offsets);
		}
		
		fclose(f);
	}

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
		string text        = "";
		string filesize    = "";
		bool foundFileName = false;
		string Progress[]  = {"", "", "", ""};

		while(getline(DownloadLog, text)) {
			text = Trim(text);

			if (text.empty())
				continue;

			// Get file size
			if (filesize==""  &&  text.substr(0,8) == "Length: ") {
				size_t open  = text.find('(');
				size_t close = text.find(')');

				if (open!=string::npos  &&  close!=string::npos)
					filesize = text.substr( open+1, close-open-1);
			}

			// Get progress bar
			if (text.find("0K ") != string::npos  &&  text.find("% ") != string::npos) {
				vector<string> Tokens;
				Tokenize(text, " .=", Tokens);
				
				for (int i=0; i<Tokens.size(); i++)
					Progress[i] = Tokens[i];
			}

			// Get file name
			const int items = 4;
			vector<string> SearchFor[items];

			SearchFor[0].push_back("Saving to: `fwatch/tmp/");
			SearchFor[0].push_back("'");

			SearchFor[1].push_back(") - `fwatch/tmp/");
			SearchFor[1].push_back("' saved [");

			SearchFor[2].push_back("File `fwatch/tmp/");
			SearchFor[2].push_back("' already there; not retrieving");

			SearchFor[3].push_back("Server file no newer than local file `fwatch/tmp/");
			SearchFor[3].push_back("' -- not retrieving");

			for (int i=0; i<items; i++) {
				size_t search1 = text.find(SearchFor[i][0]);
				size_t search2 = text.find(SearchFor[i][1]);

				if (search1!=string::npos  &&  search2!=string::npos) {
					global.downloaded_filename = text.substr( search1 + SearchFor[i][0].length(),  search2 - search1 - SearchFor[i][0].length());
					foundFileName              = true;
					break;
				}
			}

			// Get error message
			size_t search1 = text.find("failed");
			size_t search2 = text.find("ERROR");

			if (search1 != string::npos)
				error = text;

			if (search2 != string::npos)
				error = text.substr(search2);
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
			size_t errormsg = text.find("ERROR:");

			if (errormsg != string::npos  &&  error=="") {
				error            = text.substr(errormsg);
				error_until_line = line_number + 1;
			}

			if (line_number == error_until_line)
				error += " - " + text;

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


int Download(string url)
{
	// Format arguments
	global.downloaded_filename = PathLastItem(url);
	string arguments           = " --no-clobber --output-file=fwatch\\tmp\\schedule\\downloadLog.txt --directory-prefix=fwatch\\tmp\\ " + url;

	unlink("fwatch\\tmp\\schedule\\downloadLog.txt");


				
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
	
	if (exit_code != 0)
		global.logfile << "Failed to download " << global.downloaded_filename << " - " << exit_code << " - " << message << endl;
	
	return exit_code;
}


int Unpack(string file_name)
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
	string arguments = WrapInQuotes(global.working_directory) + " x -y -o" + destination + "\\ -bb3 -bsp1 " + "\"fwatch\\tmp\\" + file_name + "\"";

	if (!CreateProcess("fwatch\\data\\7z.exe", &arguments[0], NULL, NULL, true, 0, NULL, NULL, &si, &pi)) {		
		int errorCode = GetLastError();
		global.logfile << "Failed to launch 7z.exe - " << errorCode << " " << FormatError(errorCode);
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

	if (exit_code != 0)
		global.logfile << "Failed to extract " << file_name << " - " << exit_code << " - " << message << endl;

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


int MoveFiles(string source, string destination, string new_name, bool is_move, bool overwrite, bool match_dirs=false)
{
	// Find files and save them to a list
	vector<string> source_list;
	vector<string> destination_list;
	vector<bool>   is_dir_list;
	vector<string> empty_dirs;
	int buffer_size = 0;
	
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
			
	    	if (!overwrite  &&  return_value==183)
	    		global.logfile << "  - file already exists";
			else
				global.logfile << "  FAILED " << return_value << " " << FormatError(return_value);
		}
			
		global.logfile << endl;
	}


	// Remove empty directories
	if (is_move)
		for (int i=empty_dirs.size()-1; i>=0; i--)
			RemoveDirectory(empty_dirs[i].c_str());
	
	return 0;
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
			digits += Int2Str(digit-0);
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




	// Process arguments ---------------------------------------
	GetCurrentDirectory(MAX_PATH, pwd);
	
	string user_arguments     = " ";
	string user_arguments_log = " ";
	string game_exe           = "";
	string server_uniqueid    = "";
	string PBOaddon   		  = "";
	string voice_server       = "";
	bool self_update		  = false;
	bool server_equalmodreq   = false;
	bool scheduled            = false;
	DWORD game_pid            = 0;
	FILETIME schedule;
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
				server_equalmodreq = Equals(value,"true");
				continue;
			}
			
			if (Equals(name,"-serveruniqueid")) {
				server_uniqueid = value;
				continue;
			}
			
			if (Equals(name,"-servertime")) {
				vector<string> date;
				Tokenize(value, "[,]", date);
				
				if (date.size() >= 8) {
					scheduled = true;
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
				}
				
				continue;
			}

			if (Equals(name,"-selfupdate")) {
				self_update = Equals(value,"true");
				continue;
			}
			
			if (Equals(name,"-econnect") || Equals(name,"-eport") || Equals(name,"-epassword")) {
				user_arguments     += "-" + name.substr(2) + "=" + Decrypt(value) + " ";
				user_arguments_log += "-" + name.substr(2) + "=hidden ";
				continue;
			}
			
			if (Equals(name,"-evoice")) {
				if (value.substr(0,12) == "ts3server://")
					voice_server = "ts3server://" + Decrypt(value.substr(12));
					
				if (value.substr(0,12) == "mumble://")
					voice_server = "mumble://" + Decrypt(value.substr(9));
				
				continue;
			}
		}
		
		user_arguments     += namevalue + " ";
		user_arguments_log += namevalue + " ";
	}

	
	global.logfile << "Arguments: " << global.working_directory << user_arguments_log << endl;



	// If scheduled to restart at a specific time --------------
	if (scheduled) {
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
	vector<string> game[2];
	game[EXE].push_back("ArmAResistance.exe");
	game[EXE].push_back("ColdWarAssault.exe");
	game[EXE].push_back("flashpointresistance.exe");
	game[EXE].push_back("ofp.exe");
	game[EXE].push_back("flashpointbeta.exe");
	game[EXE].push_back("operationflashpoint.exe");
	game[EXE].push_back("operationflashpointbeta.exe");

	game[TITLE].push_back("Arma Resistance");
	game[TITLE].push_back("Cold War Assault");
	game[TITLE].push_back("Operation Flashpoint");
	game[TITLE].push_back("Operation Flashpoint");
	game[TITLE].push_back("Operation Flashpoint");
	game[TITLE].push_back("Operation Flashpoint");
	game[TITLE].push_back("Operation Flashpoint");

	string game_window = "";
	bool got_handle    = false;

	for (int i=0; i<game[EXE].size(); i++)
		if (!game_exe.empty()) {
			if (Equals(game[EXE][i],game_exe)) {
				game_window = game[TITLE][i];
				break;
			}
		} else {
			if (game_pid = findOFPwindow(game[TITLE][i])) {
				game_exe    = game[EXE][i];
				game_window = game[TITLE][i];
				got_handle  = true;
				break;
			}
		}		


	// if window wasn't found then pick existing exe
	if (game_window.empty())
		for (int i=0; i<game[EXE].size(); i++)
			if (GetFileAttributes(game[EXE][i].c_str()) != INVALID_FILE_ATTRIBUTES) {
				game_exe    = game[EXE][i];
				game_window = game[TITLE][i];
				break;
			}

	// if no exe then exit
	if (game_window.empty()) {
		global.logfile << "Can't find game executable\n";
		global.logfile.close();
		return 2;
	}

	// get window handle
	if (!got_handle) {
		if (game_pid != 0)
			findOFPwindow(game_window, game_pid);
		else
			game_pid = findOFPwindow(game_window);
	}
	
	

	
	
	


















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

	if (!nolaunch)
		filtered_game_arguments += fwatch_arguments;

	if (global.game_window_handle) {
		HANDLE game_handle = OpenProcess(PROCESS_ALL_ACCESS, 0, game_pid);
		
		// Read game parameters
		if (game_handle != 0) {
			vector<string> all_game_arguments;
			int result = GetProcessParams(game_pid, &game_handle, "ifc22.dll", 0x2C154, all_game_arguments);
			if (result == 0) {				
				string log_arguments = "";
				
				for (int i=0; i<all_game_arguments.size(); i++) {
					log_arguments = log_arguments + all_game_arguments[i] + " ";	// log every legible param
					
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
					global.logfile << "Couldn't find ifc22.dll" << endl;
				else {
					string errorMSG = FormatError(GetLastError());
					global.logfile << "Can't get module list" << errorMSG;
				}
					
				global.logfile.close();
			}
			
			CloseHandle(game_handle);
		} else {
			global.logfile << "Can't access game process" << FormatError(GetLastError());
			global.logfile.close();
			return 3;
		}
				

		// Shutdown the game
		bool close 	   = PostMessage(global.game_window_handle, WM_CLOSE, 0, 0);
		int isRunning  = 1;
		int tries 	   = 0;			
			
		do {
			isRunning = findOFPwindow(game_window, game_pid);
			Sleep(500);
		} while (close && isRunning && ++tries<7);
			
		if (isRunning) {
			game_handle = OpenProcess(PROCESS_ALL_ACCESS, 0, game_pid);
			
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
						if (Equals(data[4],"true")) {
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
							int tries         = 2;
							string rename_src = required_mods[NAME][i];
							string rename_dst = required_mods[NAME][i] + "_" + Int2Str(tries);
							
							while (rename(rename_src.c_str() , rename_dst.c_str()) != 0) {
								rename_dst = required_mods[NAME][i] + "_renamed" + (tries>2 ? Int2Str(tries) : "");
								Sleep(100);
								tries++;
							}
							
							global.logfile << "Renamed " << rename_src << " to " << rename_dst << endl;
						}
						
						// now rename the actual required mod to its proper name
						if (rename(mods[NAME][j].c_str(), required_mods[NAME][i].c_str()) == 0) {
							global.logfile << "Renamed " << mods[NAME][j] << " to " << required_mods[NAME][i] << endl;
							mods[NAME][j] = required_mods[NAME][i];
						}
						else {
							global.logfile << "Failed to rename " << mods[NAME][j] << " to " << required_mods[NAME][i] << " - " << FormatError(GetLastError());
							global.logfile.close();
							return 7;
						}
					}

					if (add_param_name) {
						user_arguments += "-mod=";
						add_param_name  = false;
					} else
						user_arguments += ";";
					
					user_arguments += mods[NAME][j];
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
							int tries         = 2;
							string rename_src = data[3];
							string rename_dst = data[3] + "_" + Int2Str(tries);
							
							while (rename(rename_src.c_str() , rename_dst.c_str()) != 0) {
								rename_dst = required_mods[NAME][i] + "_renamed" + (tries>2 ? Int2Str(tries) : "");
								Sleep(100);
								tries++;
							}
							
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

			user_arguments += (i==0 ? "-mod=" : ";") + required_mods[NAME][i];
		}
		
	user_arguments += " ";
	

	// Read from a file user's startup parameters for this server
	if (!server_uniqueid.empty()) {
		string params = ReadStartupParams(server_uniqueid);
		
		if (server_equalmodreq) {
			vector<string> params_array;
			Tokenize(params, " ", params_array);
			
			for (int i=0; i<params_array.size(); i++) {
				if (Equals(params_array[i].substr(0,5),"-mod="))
					continue;
					
				user_arguments += params_array[i] + " ";
			}
		} else
			user_arguments += params + " ";
	}





	
	// Self-update ---------------------------------------------
	if (self_update) {
		Sleep(1000);
		int result;
		DeleteFile("fwatch\\tmp\\fwatch_self_update.rar");
		if (result=Download("http://ofp-faguss.com/fwatch/download/fwatch_self_update.rar") != 0) {
			global.logfile << "Download failed\n\n--------------\n\n";
			global.logfile.close();
			return result;
		}
		
		Unpack(global.downloaded_filename);
		
		if (GetFileAttributes("fwatch\\tmp\\_extracted\\fwatch\\data\\gameRestart.exe") != INVALID_FILE_ATTRIBUTES) {
			DeleteFile("fwatch\\data\\gameRestart_old.exe");
			rename("fwatch\\data\\gameRestart.exe", "fwatch\\data\\gameRestart_old.exe");
		}
		
		MoveFiles("fwatch\\tmp\\_extracted\\*" , "", "", true, true, true);
		DeleteFile("fwatch\\tmp\\fwatch_self_update.rar");
		if (!DeleteFile("fwatch\\tmp\\schedule\\schedule.bin")) {
			string errorMSG = FormatError(GetLastError());
			global.logfile << "Failed to remove file " << errorMSG;
		}
		
		DeleteFile("fwatch\\data\\libeay32.dll");
		DeleteFile("fwatch\\data\\libiconv2.dll");
		DeleteFile("fwatch\\data\\libintl3.dll");
		DeleteFile("fwatch\\data\\libssl32.dll");
		DeleteFile("fwatch\\data\\sortMissions.exe");
	} else 
		DeleteFile("fwatch\\data\\gameRestart_old.exe");
	





	
	// Start a new game ----------------------------------------
	string launch_exe     = global.working_directory + "\\" + (nolaunch ? game_exe : "fwatch.exe");
	string launch_arg     = " " + global.working_directory + " " + filtered_game_arguments + user_arguments + " " + (nolaunch ? "" : fwatch_arguments);
	string launch_arg_log = " " + global.working_directory + " " + filtered_game_arguments + user_arguments_log + " " + (nolaunch ? "" : fwatch_arguments);
	
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
				launch_arg = " " + (string)SteamPath + " -applaunch 65790 " + global.working_directory + " -nomap " + filtered_game_arguments + user_arguments;
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
