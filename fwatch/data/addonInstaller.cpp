// AddonInstaller.exe by Faguss (ofp-faguss.com) for Fwatch 1.16
// Russian translation by mju (twitter.com/paumju)


#include <fstream>		// file operations
#include <windows.h>	// winapi
#include <tlhelp32.h>	// process/module traversing
#include <unistd.h>     // for access command
#include <vector>       // dynamic array
#include <algorithm>	// tolower
#include <sstream>      // for converting int to string
#include <Shlobj.h>		// opening explorer
#include <map>			// associative array for arguments
#include <time.h>		// get current time as unix timestamp
#include <iomanip>		// for url encode

using namespace std;

struct GLOBAL_VARIABLES 
{
	bool test_mode;
	bool abort_installer;
	bool skip_modfolder;
	bool restart_game;
	bool end_thread;
	bool run_voice_program;
	bool download_phase;
	bool last_download_attempt;
	int condition_index;
	int command_line_num;
	int installation_steps_current;
	int installation_steps_max;
	int saved_alias_array_size;
	int download_iterator;
	time_t current_mod_version_date;
	float installer_version;
	float script_version;
	string gamerestart_arguments;
	string downloaded_filename;
	string current_mod;
	string missing_modfolders;
	string last_pbo_file;
	string working_directory;
	string command_line;
	string current_mod_new_name;
	string current_mod_version;
	string current_mod_id;
	string current_mod_keepname;
	string downloaded_filename_last;
	vector<int> condition;
	vector<string> downloads;
	vector<string> mod_id;
	vector<string> mod_name;
	vector<string> current_mod_alias;
	string *lang;
	string *lang_eng;
	map<string, string> arguments_table;
	ofstream logfile;
} global = {
	false,
	false,
	false,
	false,
	false,
	true,
	false,
	false,
	-1,
	0,
	0,
	0,
	0,
	0,
	0,
	0.6,
	0,
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	""
};

enum INSTALL_STATUS 
{
	INSTALL_PROGRESS,
	INSTALL_WAITINGFORUSER,
	INSTALL_ERROR,
	INSTALL_ABORTED,
	INSTALL_WARNING,
	INSTALL_DONE
};

enum FUNCTION_FLAGS 
{
	FLAG_NONE            = 0x0,
	FLAG_NO_END_SLASH    = 0x1,
	FLAG_MOVE_FILES      = 0x2,
	FLAG_OVERWRITE       = 0x4,
	FLAG_MATCH_DIRS      = 0x8,
	FLAG_ALLOW_ERROR     = 0x10,
	FLAG_SILENT_MODE     = 0x20,
	FLAG_RUN_EXE         = 0x40,
	FLAG_CREATE_DIR      = 0x80,
	FLAG_DONT_IGNORE     = 0x100,
	FLAG_DEMO_MISSIONS   = 0x200,
	FLAG_RES_ADDONS	     = 0x400,
	FLAG_MATCH_DIRS_ONLY = 0x800,
	FLAG_CLEAN_DL_NOW    = 0x1000,
	FLAG_CLEAN_DL_LATER  = 0x2000
};

enum ERROR_CODES 
{
	ERROR_NONE,
	ERROR_USER_ABORTED,
	ERROR_LOGFILE,
	ERROR_NO_SCRIPT,
	ERROR_COMMAND_FAILED,
	ERROR_WRONG_SCRIPT,
	ERROR_WRONG_ARCHIVE
};

enum STRINGTABLE 
{
	STR_ACTION_INIT,
	STR_ACTION_GETSCRIPT,
	STR_ACTION_READSCRIPT,
	STR_ACTION_CONNECTING,
	STR_ACTION_DOWNLOADING,
	STR_ACTION_DOWNLOADED,
	STR_ACTION_EXTRACTING,
	STR_ACTION_UNPACKINGPBO,
	STR_ACTION_PACKINGPBO,
	STR_ACTION_COPYING,
	STR_ACTION_COPYINGDOWNLOAD,
	STR_ACTION_CLEANING,
	STR_ACTION_PREPARING,
	STR_ACTION_DELETING,
	STR_ACTION_RENAMING,
	STR_ACTION_EDITING,
	STR_ACTION_ABORTED,
	STR_ACTION_DONE,
	STR_ACTION_DONEWARNING,
	STR_PROGRESS,
	STR_ALTTAB,
	STR_ERROR,
	STR_ERROR_LOGFILE,
	STR_ERROR_READSCRIPT,
	STR_ERROR_WRONG_VERSION,
	STR_ERROR_INVERSION,
	STR_ERROR_ONLINE,
	STR_ERROR_EXE,
	STR_ERROR_ARG_COUNT,
	STR_ERROR_FILE_LIST,
	STR_ERROR_NO_FILE,
	STR_ERROR_PATH,
	STR_ERROR_INVALID_SCRIPT,
	STR_ERROR_INVALID_ARG,
	STR_ERROR_BUFFER,
	STR_DOWNLOAD_LEFT,
	STR_DOWNLOAD_TOTAL,
	STR_DOWNLOAD_PATH_ERROR,
	STR_DOWNLOAD_FAILED,
	STR_DOWNLOAD_FIND_ERROR,
	STR_UNPACK_REDO_FILE,
	STR_UNPACK_ERROR,
	STR_MDIR_ERROR,
	STR_AUTO_READ_ATTRI,
	STR_UNPACKPBO_SRC_PATH_ERROR,
	STR_UNPACKPBO_DST_PATH_ERROR,
	STR_MOVE_DST_PATH_ERROR,
	STR_MOVE_ERROR,
	STR_MOVE_TO_ERROR,
	STR_COPY_ERROR,
	STR_MOVE_RENAME_ERROR,
	STR_MOVE_RENAME_TO_ERROR,
	STR_RENAME_DST_PATH_ERROR,
	STR_RENAME_WILDCARD_ERROR,
	STR_RENAME_NO_NAME_ERROR,
	STR_DELETE_PERMANENT_ERROR,
	STR_DELETE_BIN_ERROR,
	STR_ASK_EXE,
	STR_ASK_DLOAD,
	STR_ASK_DLOAD_SELECT,
	STR_IF_NUMBER_ERROR,
	STR_PBO_NAME_ERROR,
	STR_PBO_MAKE_ERROR,
	STR_PBO_UNPACK_ERROR,
	STR_EDIT_READ_ERROR,
	STR_EDIT_WRITE_ERROR,
	STR_ACTION_READMISSIONSQM,
	STR_MAX
};

enum MISSIONSQM_PARSING
{
	SQM_PROPERTY,
	SQM_EQUALITY,
	SQM_VALUE,
	SQM_SEMICOLON,
	SQM_CLASS_NAME,
	SQM_CLASS_INHERIT,
	SQM_CLASS_COLON,
	SQM_CLASS_BRACKET,
	SQM_ENUM_BRACKET,
	SQM_ENUM_CONTENT,
	SQM_EXEC_BRACKET,
	SQM_EXEC_CONTENT,
	SQM_MACRO_CONTENT
};

enum MISSIONSQM_PLAYERCOUNT 
{
	SQM_NONE,
	SQM_SINGLE_PLAYER,
	SQM_MULTI_PLAYER,
	SQM_SINGLE_PLAYER_TEMPLATE,
	SQM_MULTI_PLAYER_TEMPLATE
};




// String operations -------------------------------------------------------------------------------------
	
string Trim(string s)
{
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
	s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
	return s;
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

	// Remove quotation marks
string UnQuote(string text)
{
	if (text.substr(text.length()-1) == "\"")
		text = text.substr(0, text.length()-1);
	
	if (text.substr(0,1) == "\"")
		text = text.substr(1);
		
	return text;	
}

	// Convert number to string and add leading zero
string LeadingZero(int number)
{
	string ret = "";
	
	if (number < 10) 
		ret += "0";
		
	stringstream temp;
	temp << number;
	ret += temp.str();
	
	return ret;
}

	// https://superuser.com/questions/475874/how-does-the-windows-rename-command-interpret-wildcards	https://superuser.com/a/739718
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

string PathLastItem(string path) 
{
	size_t lastSlash = path.find_last_of("\\/");

	if (lastSlash != string::npos)
		return path.substr(lastSlash+1);
	else
		return path;
}

string PathNoLastItem(string path, int options=FLAG_NONE) 
{
	size_t find = path.find_last_of("\\/");

	if (find != string::npos)
		return path.substr(0, find+(options & FLAG_NO_END_SLASH ? 0 : 1));
	else
		return "";
}

	// Windows error message
string FormatError(int error)
{
	if (error == 0) 
		return "\n";

	LPTSTR errorText = NULL;

	FormatMessage(
	FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
	NULL,
	error, 
	MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
	(LPTSTR)&errorText,
	0,
	NULL);

	string ret = "   - " + (string)(char*)errorText + "\n";

	if (errorText != NULL)
		LocalFree(errorText);

	return Trim(ret);
}

void Tokenize(string text, string delimiter, vector<string> &container)
{
	bool first_item   = false;
	bool inQuote      = false;
	char custom_delim = ' ';
	bool use_unQuote  = true;
	
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
			container.push_back(UnQuote(text.substr(begin, pos-begin)));
			begin = -1;
		}
	}
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

bool VerifyPath(string path)
{
	vector<string> directories;
	Tokenize(path, "\\/", directories);

	// Path cannot go back to the parent directory
	for (int i=0; i<directories.size(); i++)
		if (directories[i] == "..")
			return false;

	return true;
}

string Int2Str(int num)
{
    ostringstream text;
    text << num;
    return text.str();
}

string Float2Str(float num)
{
    ostringstream text;
    text << num;
    return text.str();
}

bool IsURL(string text)
{
	return (
		Equals(text.substr(0,7),"http://")  ||  
		Equals(text.substr(0,8),"https://")  ||  
		Equals(text.substr(0,6),"ftp://")  ||  
		Equals(text.substr(0,4),"www.")
	);
}

bool IsModName(string filename)
{
	if (Equals(filename,global.current_mod))
		return true;
		
	for(int i=0; i<global.current_mod_alias.size(); i++)
		if (Equals(filename,global.current_mod_alias[i]))
			return true;
			
	return false;
}

	// https://stackoverflow.com/questions/6691555/converting-narrow-string-to-wide-string
wstring string2wide(const string& input)
{
    if (input.empty())
		return wstring();

	size_t output_length = MultiByteToWideChar(CP_UTF8, 0, input.c_str(), (int)input.length(), 0, 0);
	wstring output(output_length, L'\0');
	MultiByteToWideChar(CP_UTF8, 0, input.c_str(), (int)input.length(), &output[0], (int)input.length());
	
	return output;
}

	// https://mariusbancila.ro/blog/2008/10/20/writing-utf-8-files-in-c/
string wide2string(const wchar_t* input, int input_length)
{
	int output_length = WideCharToMultiByte(CP_UTF8, 0, input, input_length, NULL, 0, NULL, NULL);
      
	if (output_length == 0) 
		return "";
 
	string output(output_length, ' ');
	WideCharToMultiByte(CP_UTF8, 0, input, input_length, const_cast< char* >(output.c_str()), output_length, NULL, NULL); 
 
	return output;
}
 
string wide2string(const wstring& input)
{
   return wide2string(input.c_str(), (int)input.size());
}

string GetTextBetween(string &buffer, string start, string end, size_t &offset, bool reverse=false)
{
	string out  = "";
	size_t pos0 = buffer.find(start, offset);
	
	if (!reverse) {
		if (pos0 != string::npos) {
			size_t pos1 = pos0 + start.length();
			size_t pos2 = buffer.find(end, pos1);
			
			if (pos2 != string::npos) {
				offset = pos1;
				out    = buffer.substr(pos1, pos2-pos1);
			}
		}		
	} else {
		if (pos0 != string::npos) {
			size_t pos1 = buffer.rfind(end, pos0);			
			
			if (pos1 != string::npos) {
				offset      = pos0 + start.length();
				size_t pos2 = pos1 + end.length();
				out         = buffer.substr(pos2, pos0-pos2);
			}
		}			
	}

	return out;
}

	// https://stackoverflow.com/questions/154536/encode-decode-urls-in-c
string url_encode(const string &value) 
{
    ostringstream escaped;
    escaped.fill('0');
    escaped << hex;

    for (string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
        string::value_type c = (*i);

        // Keep alphanumeric and other accepted characters intact
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
            continue;
        }

        // Any other characters are percent-encoded
        escaped << uppercase;
        escaped << '%' << setw(2) << int((unsigned char) c);
        escaped << nouppercase;
    }

    return escaped.str();
}

string lowercase(const string &input) 
{
	string output = input;
	
	for (int i=0; i<output.length(); i++)
		output[i] = tolower(output[i]);
		
	return output;
}
// -------------------------------------------------------------------------------------------------------




// Installer messaging -----------------------------------------------------------------------------------

	// Feedback for the game
void WriteProgressFile(int status, string input)
{
	ofstream progressLog;
	progressLog.open("fwatch\\tmp\\schedule\\install_progress.sqf", ios::out | ios::trunc);

	if (!progressLog.is_open())
		return;
	
	if (status==INSTALL_PROGRESS  &&  global.installation_steps_current>0  &&  global.installation_steps_max>0)
		input = global.lang[STR_PROGRESS] + " " + Int2Str(global.installation_steps_current) + "/" + Int2Str(global.installation_steps_max) + "\\n\\n" + input;

	progressLog << "_auto_restart=" << (global.restart_game ? "true" : "false") 
				<< ";_run_voice_program=" << (global.run_voice_program ? "true" : "false")
				<< ";_install_status=" << status << ";\"" << ReplaceAll(input, "\"", "'") << "\"";
	progressLog.close();
};

	// Write mod identification file
int WriteModID(string modfolder, string content, string content2)
{
	if (global.test_mode)
		return ERROR_NONE;
	
	ofstream ID_file;
	string path = modfolder + "\\__gs_id";
	ID_file.open(path.c_str(), ios::out | ios::trunc);

	if (ID_file.is_open()) {
		SYSTEMTIME st;
		GetLocalTime(&st);
		TIME_ZONE_INFORMATION TimeZoneInfo;
		GetTimeZoneInformation (&TimeZoneInfo);
		char current_date[128] = "";
		sprintf(current_date, ";[%d,%d,%d,%d,%d,%d,%d,%d,%d,false]",
			st.wYear, 
			st.wMonth, 
			st.wDay, 
			st.wDayOfWeek, 
			st.wHour, 
			st.wMinute, 
			st.wSecond, 
			st.wMilliseconds, 
			TimeZoneInfo.Bias * -1
		);
			
		ID_file << content << current_date << ";" << global.current_mod << ";" << content2;
		ID_file.close();
		return ERROR_NONE;
	} else
		return ERROR_LOGFILE;
}

	// Format error message
int ErrorMessage(int string_code, string message="%STR%", int error_code=ERROR_COMMAND_FAILED) 
{
	if (global.current_mod=="?pretendtoinstall" && global.current_mod_version=="-1")
		return ERROR_NONE;
	
	int status              = global.last_download_attempt ? INSTALL_ERROR : INSTALL_PROGRESS;
	string message_eng      = ReplaceAll(message, "%STR%", global.lang_eng[string_code]);
	string message_local    = ReplaceAll(message, "%STR%", global.lang[string_code]);
	string message_complete = "";
	
	// show which command failed
	if (error_code == ERROR_COMMAND_FAILED) {
		message_complete = global.lang[STR_ERROR] + "\\n" + global.current_mod;
		
		if (global.current_mod_version != "")
			message_complete += "\\n" + global.lang[STR_ERROR_INVERSION] + " " + global.current_mod_version;
		
		message_complete += "\\n" + global.lang[STR_ERROR_ONLINE] + " " + Int2Str(global.command_line_num) + "\\n" + (global.download_phase ? "\\n" : global.command_line+"\\n") + message_local;

		if (status == INSTALL_ERROR) {
			global.logfile << "ERROR " << global.current_mod;
			
			if (global.current_mod_version != "")
				global.logfile << " v" << global.current_mod_version;
			
			global.logfile << " line " << Int2Str(global.command_line_num);
			
			if (!global.download_phase)
				global.logfile << ": " << global.command_line;
				
			global.logfile << " - " << ReplaceAll(message_eng, "\\n", " ") << endl;
		}
	}
	
	// just display input message
	if (error_code == ERROR_WRONG_SCRIPT) {
		message_complete = global.lang[STR_ERROR] + "\\n" + message_local;
		global.logfile << "ERROR - " << ReplaceAll(message_eng, "\\n", " ") << endl;
	}
	
	WriteProgressFile(status, message_complete);
	return error_code;
}

	// Separate thread for checking user feedback
void ReceiveInstructions(void *nothing)
{
	fstream instructions;

	while (!global.end_thread  &&  !global.abort_installer) {
		instructions.open("fwatch\\tmp\\schedule\\InstallerInstruction.txt", ios::in);

		if (instructions.is_open()) {
		    string text;

			while (getline(instructions, text)) {
				if (text == "abort")
					global.abort_installer = true;

				if (text == "restart")
					global.restart_game = !global.restart_game;
					
				if (text == "voice")
					global.run_voice_program = !global.run_voice_program;
			}
		}

		instructions.close();
		DeleteFile("fwatch\\tmp\\schedule\\InstallerInstruction.txt");
		Sleep(100);
	}

	_endthread();
}

	// Cancel entire installation
int Abort()
{
	if (global.abort_installer) {
		WriteProgressFile(INSTALL_ABORTED, global.lang[STR_ACTION_ABORTED]);	

		if (global.logfile.is_open()) {
			global.logfile << "Installation aborted by user\n\n--------------\n\n";
			global.logfile.close();
		}

		global.end_thread = true;
		Sleep(300);
		return ERROR_USER_ABORTED;
	}

	return ERROR_NONE;
}
// -------------------------------------------------------------------------------------------------------




// File reading ------------------------------------------------------------------------------------------

	// https://insanecoding.blogspot.com/2011/11/how-to-read-in-file-in-c.html
string GetFileContents(const char *filename)
{
	ifstream file(filename, ios::in | ios::binary);
	string contents;
  
	if (file) {
		file.seekg(0, ios::end);
		contents.resize(file.tellg());
		file.seekg(0, ios::beg);
		file.read(&contents[0], contents.size());
		file.close();
	}
	
	return contents;
}

string GetFileContents(string &filename) 
{
	return GetFileContents(filename.c_str());
}

string GetFileExtension(string file_name)
{
	string file_extension = file_name.substr( file_name.find_last_of('.')+1 );
	transform(file_extension.begin(), file_extension.end(), file_extension.begin(), ::tolower);
	
	// If extension is a number then it's a wget backup - find real extension
	int Number;
	
	if (file_extension!="7z" && istringstream(file_extension) >> Number) {
		size_t lastDot       = file_name.find_last_of('.');
		size_t secondLastDot = file_name.find_last_of('.', lastDot-1);
		
		if (lastDot!=string::npos  &&  secondLastDot!=string::npos)
			file_extension = file_name.substr( secondLastDot+1, lastDot-secondLastDot-1 );
	}
	
	return file_extension;
}

	// Read wget log to get information about the download
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
		string filename_messages[filename_messages_items][2] = {
			{"Saving to: 'fwatch/tmp/"                          , "'"},
			{") - 'fwatch/tmp/"                                 , "' saved ["},
			{"File 'fwatch/tmp/"                                , "' already there; not retrieving"},
			{"Server file no newer than local file 'fwatch/tmp/", "' -- not retrieving"}
		};
			
		const int error_messages_items = 2;
		string error_messages[error_messages_items] = {"failed","ERROR"};

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
			for (int i=0; i<error_messages_items; i++) {
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

		string tosave = global.lang[STR_ACTION_CONNECTING] + "...";

		if (!size_downloaded.empty()) {
			tosave = global.lang[STR_ACTION_DOWNLOADING] + "...\\n" + 
					 global.downloaded_filename + "\\n\\n" +
					 size_downloaded + (filesize.empty() ? "" : (" / "+filesize+" - "+percentage_downloaded))  + "\\n" + 
					 download_speed + "/s" + "\\n" + 
					 (time_remaining.empty() ? "" : (time_remaining + " " + global.lang[STR_DOWNLOAD_LEFT]));
					 
			if (percentage_downloaded == "100%")
				tosave = global.lang[STR_ACTION_DOWNLOADED] + "\\n" + 
						 global.downloaded_filename + "\\n\\n" +
						 (filesize.empty() ? size_downloaded : filesize) + "\\n" + 
						 "\\n" + 
						 time_remaining + " " + global.lang[STR_DOWNLOAD_TOTAL];
		}

		WriteProgressFile(INSTALL_PROGRESS, tosave);
	} else
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
				if (percent < 3)
					percent = 0;
				else
					percent -= 3;

				percentage  = Trim(text.substr(percent, 4));
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

		string tosave = global.lang[STR_ACTION_EXTRACTING] + "...\\n" + file_name + "\\n" + percentage + "\\n\\n" + current_file;
		WriteProgressFile(INSTALL_PROGRESS, tosave);
	}

	return 0;
}

	// Read MakePbo/ExtractPBO log to get information about un/packing
int ParsePBOLog(string &message, string &exename, string &file_name)
{
	fstream PackLog;
	PackLog.open("fwatch\\tmp\\schedule\\PBOLog.txt", ios::in);

	string verb = "";

	if (exename == "ExtractPbo.exe")
		verb = global.lang[STR_ACTION_UNPACKINGPBO];

	if (exename == "MakePbo.exe")
		verb = global.lang[STR_ACTION_PACKINGPBO];

	if (PackLog.is_open()) {
		string text = "";

		while(getline(PackLog, text)) {
			text = Trim(text);

			if (text.empty())
				continue;

			if (Equals(text.substr(0,4),"cwd="))
				message += "\\n" + text;
			else
				message = text;

			if (message == "noisy extraction of specific files (eg lists them)")
				message = "command line syntax error";
		}

		string tosave = verb + "...\\n" + file_name + "\\n\\n" + message;
		WriteProgressFile(INSTALL_PROGRESS, tosave);
	}

	return 0;
}

int CreateFileList(string source, string destination, vector<string> &sources, vector<string> &destinations, vector<bool> &dirs, int options, vector<string> &empty_dirs, int &buffer_size, int &recursion)
{
	WIN32_FIND_DATAW fd;
	HANDLE hFind        = INVALID_HANDLE_VALUE;
	wstring source_wide = string2wide(source);
	hFind               = FindFirstFileW(source_wide.c_str(), &fd);
	int result          = 0;

	if (hFind == INVALID_HANDLE_VALUE) {
		int error_code = GetLastError();
		
		if (options & FLAG_ALLOW_ERROR && error_code==ERROR_FILE_NOT_FOUND  ||  error_code==ERROR_PATH_NOT_FOUND)
			return ERROR_NONE;

		return ErrorMessage(STR_ERROR_FILE_LIST, "%STR% " + source + "  - " + Int2Str(error_code) + " " + FormatError(error_code));
	}

	recursion++;
	string base_source      = PathNoLastItem(source);
	string base_destination = PathNoLastItem(destination);
	string new_name         = PathLastItem(destination);

	if (new_name.empty())
		new_name = PathLastItem(source);
		
	bool is_source_wildcard      = source.find("*")!=string::npos    ||  source.find("?")!=string::npos;
	bool is_destination_wildcard = new_name.find("*")!=string::npos  ||  new_name.find("?")!=string::npos;

	do {
		if (wcscmp(fd.cFileName,L".")==0  ||  wcscmp(fd.cFileName,L"..")==0)
			continue;
	    
		string file_name       = wide2string(fd.cFileName);
		string new_source      = base_source      + file_name;
		string new_destination = base_destination + (is_destination_wildcard ? MaskNewName(file_name,new_name) : new_name);
		bool is_dir            = fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
		DWORD attributes       = GetFileAttributes(new_destination.c_str());

		if ((is_dir  &&  is_source_wildcard  &&  ~options & FLAG_MATCH_DIRS)  ||  (!is_dir  &&  options & FLAG_MATCH_DIRS_ONLY))
			continue;

		// move modfolder to the game dir when using wildcards
		if (is_dir  &&  options & FLAG_MATCH_DIRS  &&  recursion==0  &&  Equals(destination,global.current_mod_new_name+"\\")  &&  IsModName(file_name)) {
			new_destination = global.current_mod_new_name;
			attributes      = GetFileAttributes(new_destination.c_str());
		}

		// If we need full paths and their totaled length
		if (buffer_size != 0) {
			new_destination = global.working_directory + "\\" + new_destination;
			buffer_size    += new_destination.length() + 1;
		}


		// Check if destination directory already exists
		if (is_dir  &&  (attributes != INVALID_FILE_ATTRIBUTES  &&  attributes & FILE_ATTRIBUTE_DIRECTORY  ||  ~options & FLAG_MOVE_FILES)  &&  buffer_size==0) {
			if (!~options & FLAG_MOVE_FILES)
				CreateDirectory(new_destination.c_str(), NULL);
			else
				empty_dirs.push_back(new_source);

			// If dir already exists then move its contents
			new_source      += "\\*";
			new_destination += "\\";
			result           = CreateFileList(new_source, new_destination, sources, destinations, dirs, options, empty_dirs, buffer_size, recursion);
			
			if (result != ERROR_NONE)
				break;
		} else {
			sources     .push_back(new_source);
			destinations.push_back(new_destination);
			dirs        .push_back(is_dir);
		}
	} while (FindNextFileW(hFind, &fd) != 0);

	recursion--;
	FindClose(hFind);
	return result;
}

string GetMissionDestinationFromSQM(string path)
{
	WriteProgressFile(INSTALL_PROGRESS, global.lang[STR_ACTION_READMISSIONSQM]);
	
	string text_buffer = GetFileContents(path);
	
	if (text_buffer.empty())
		return "Addons";
		
	int expect       = SQM_PROPERTY;
	int word_start   = -1;
	int class_level  = 0;
	int array_level  = 0;
	int player_count = 0;
	bool is_array    = false;
	bool in_quote    = false;
	bool in_mission  = false;
	bool is_wizard   = false;
	char separator   = ' ';
	string property  = "";
	
	for (int i=0; i<text_buffer.size(); i++) {
		char c = text_buffer[i];
		
		switch (expect) {
			case SQM_SEMICOLON : {
				if (c == ';') {
					expect = SQM_PROPERTY;
					continue;
				} else 
					if (!isspace(c))
						expect = SQM_PROPERTY;
			}
			
			case SQM_PROPERTY : {
				if (c == '}') {
					class_level--;

					if (in_mission && class_level==0) {
						i = text_buffer.size();
						break;
					}
									
					expect = SQM_SEMICOLON;
					continue;
				}
				
				if (isalnum(c) || c=='_' || c=='[' || c==']') {
					if (word_start == -1)
						word_start = i;
				} else
					if (word_start >= 0) {
						property    = text_buffer.substr(word_start, i-word_start);
	
						if (property == "class") {
							expect    = SQM_CLASS_NAME;
						} else {
							expect    = SQM_EQUALITY;
							separator = '=';
							is_array  = text_buffer[i-2]=='[' && text_buffer[i-1]==']';
							
							if (property == "player")
								player_count++;
						}

						word_start = -1;
					}
				
				if (separator == ' ')
					break;
			}
			
			case SQM_EQUALITY : {
				if (c == separator) {
					expect++;
					separator = ' ';
				} else 
					if (!isspace(c)) {
						i--;
						separator = ' ';
						expect    = SQM_SEMICOLON;
					}
				
				break;
			}
			
			case SQM_VALUE : {
				if (c == '"')
					in_quote = !in_quote;

				if (!in_quote && c=='{')
					array_level++;

				if (!in_quote && c=='}')
					array_level--;

				if (word_start == -1) {
					if (!isspace(c))
						word_start = i;
				} else {
					if (!in_quote && array_level==0 && (c==';' || c=='\r' || c=='\n')) {
						string word = text_buffer.substr(word_start, i-word_start);
						transform(word.begin(), word.end(), word.begin(), ::tolower);
						
						if (!is_wizard && Equals(property,"position[]") && word.find("wizvar_")!=string::npos)
							is_wizard = true;
				
						word_start = -1;
						expect     = SQM_PROPERTY;
					}
				}
				
				break;
			}
			
			case SQM_CLASS_NAME : {
				if (isalnum(c) || c=='_') {
					if (word_start == -1)
						word_start = i;
				} else
					if (word_start >= 0) {
						if (text_buffer.substr(word_start, i-word_start) == "Mission")
							in_mission = true;

						word_start = -1;
						expect     = SQM_CLASS_BRACKET;
					}
				
				if (expect != SQM_CLASS_BRACKET)
					break;
			}
			
			case SQM_CLASS_BRACKET : {
				if (c == '{') {
					class_level++;
					expect = SQM_PROPERTY;
				} else
					if (!isspace(c)) {
						i--;
						expect = SQM_SEMICOLON;
					}
				
				break;
			}
		}
	}
	
	if (player_count > 1)
		return is_wizard ? "Templates" : "MPMissions";
	else
		return is_wizard ? "SPTemplates" : "Missions";
}

	// callback for BrowseFolder()
static int CALLBACK BrowseCallbackProc(HWND hwnd,UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	if (uMsg == BFFM_INITIALIZED)
		SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);

	return 0;
}

	// https://stackoverflow.com/questions/12034943/win32-select-directory-dialog-from-c-c
string BrowseFolder(string saved_path)
{
	TCHAR path[MAX_PATH];

	const char * path_param = saved_path.c_str();

	BROWSEINFO bi = { 0 };
	bi.lpszTitle  = ("Browse for folder...");
	bi.ulFlags    = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
	bi.lpfn       = BrowseCallbackProc;
	bi.lParam     = (LPARAM) path_param;

	LPITEMIDLIST pidl = SHBrowseForFolder ( &bi );

	if (pidl != 0) {
		//get the name of the folder and put it in path
		SHGetPathFromIDList ( pidl, path );

		//free memory used
		IMalloc * imalloc = 0;
		if ( SUCCEEDED( SHGetMalloc ( &imalloc )) ) {
			imalloc->Free ( pidl );
			imalloc->Release ( );
		}

		return path;
	}

	return "";
}

enum MOD_SUBFOLDERS {
	DIR_NONE,
	DIR_ADDONS,
	DIR_BIN,
	DIR_CAMPAIGNS,
	DIR_DTA,
	DIR_WORLDS,
	DIR_MISSIONS,
	DIR_MPMISSIONS,
	DIR_TEMPLATES,
	DIR_SPTEMPLATES,
	DIR_MISSIONSUSERS,
	DIR_MPMISSIONSUSERS,
	DIR_ISLANDCUTSCENES,
	DIR_MAX
};

string mod_subfolders[] = {
	"",
	"Addons",
	"Bin",
	"Campaigns",
	"Dta",
	"Worlds",
	"Missions",
	"MPMissions",
	"Templates",
	"SPTemplates",
	"MissionsUsers",
	"MPMissionsUsers",
	"IslandCutscenes"
};
		
struct DIRECTORY_INFO {
	int error_code;
	int number_of_files;
	int number_of_dirs;
	int number_of_wanted_mods;
	int number_of_mod_subfolders;
	vector<wstring> file_list;
	vector<DWORD> attributes_list;
	vector<int> mod_sub_id_list;
	vector<bool> is_mod_list;
};

	// Browse files in a given folder and return file list plus some other info
DIRECTORY_INFO ScanDirectory(string path) 
{
	DIRECTORY_INFO output;	
	output.error_code               = ERROR_NONE;
	output.number_of_files          = 0;
	output.number_of_dirs           = 0;
	output.number_of_wanted_mods    = 0;
	output.number_of_mod_subfolders = 0;
	vector<wstring> files;
	
	WIN32_FIND_DATAW FileInformation;
	wstring pattern = string2wide(path) + L"\\*";
	HANDLE hFile    = FindFirstFileW(pattern.c_str(), &FileInformation);

	if (hFile != INVALID_HANDLE_VALUE) {
		do {
			if (wcscmp(FileInformation.cFileName,L".")==0  ||  wcscmp(FileInformation.cFileName,L"..")==0  ||  wcscmp(FileInformation.cFileName,L"$PLUGINSDIR")==0)
				continue;
				
			bool is_mod    = false;
			int mod_sub_id = DIR_NONE;
				
			if (FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				output.number_of_dirs++;
				is_mod = IsModName(wide2string(FileInformation.cFileName));
				
				if (is_mod)
					output.number_of_wanted_mods++;
				
				for (int i=DIR_ADDONS; i<DIR_MAX && mod_sub_id==DIR_NONE; i++)
					if (Equals(wide2string(FileInformation.cFileName),mod_subfolders[i])) {
						output.number_of_mod_subfolders++;
						mod_sub_id = i;
					}
					
				output.file_list.push_back(FileInformation.cFileName);
				output.attributes_list.push_back(FileInformation.dwFileAttributes);
				output.is_mod_list.push_back(is_mod);
				output.mod_sub_id_list.push_back(mod_sub_id);
			} else {
				output.number_of_files++;
				files.push_back(FileInformation.cFileName);
			}
		} while(FindNextFileW(hFile, &FileInformation) == TRUE);
		
		// Files come after directories
		for(int i=0; i<files.size(); i++) {
			output.file_list.push_back(files[i]);
			output.attributes_list.push_back(0);
			output.is_mod_list.push_back(false);
			output.mod_sub_id_list.push_back(DIR_NONE);
		}
		
		FindClose(hFile);
	} else {
		int error_code    = GetLastError();
		output.error_code = ErrorMessage(STR_ERROR_FILE_LIST, "%STR% " + path + " - " + Int2Str(error_code) + " " + FormatError(error_code));
	}

	return output;
}
// -------------------------------------------------------------------------------------------------------




// File writing ------------------------------------------------------------------------------------------

	// Delete file or directory with its contents  http://stackoverflow.com/a/10836193
int DeleteDirectory(const wstring &refcstrRootDirectory, bool bDeleteSubdirectories=true)
{
	bool             bSubdirectory = false;       // Flag, indicating whether subdirectories have been found
	HANDLE           hFile;                       // Handle to directory
	wstring     	 strFilePath;                 // Filepath
	wstring          strPattern;                  // Pattern
	WIN32_FIND_DATAW FileInformation;             // File information

	strPattern = refcstrRootDirectory + L"\\*.*";
	hFile      = FindFirstFileW(strPattern.c_str(), &FileInformation);
	
	if (hFile != INVALID_HANDLE_VALUE) {
		do {
			if (wcscmp(FileInformation.cFileName,L".")==0  ||  wcscmp(FileInformation.cFileName,L"..")==0)
				continue;

			strFilePath.erase();
			strFilePath = refcstrRootDirectory + L"\\" + FileInformation.cFileName;

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
				if (SetFileAttributesW(strFilePath.c_str(), FILE_ATTRIBUTE_NORMAL) == FALSE)
					return GetLastError();

				// Delete file
				if (DeleteFileW(strFilePath.c_str()) == FALSE)
					return GetLastError();
			}
		}
		while (FindNextFileW(hFile, &FileInformation) == TRUE);

		// Close handle
		FindClose(hFile);

		DWORD dwError = GetLastError();
		
		if (dwError != ERROR_NO_MORE_FILES)
      		return dwError;
		else {
			if (!bSubdirectory) {
				// Set directory attributes
				if (SetFileAttributesW(refcstrRootDirectory.c_str(), FILE_ATTRIBUTE_NORMAL) == FALSE)
					return GetLastError();

				// Delete directory
				if (refcstrRootDirectory != L"fwatch\\tmp\\_extracted") {
					if (RemoveDirectoryW(refcstrRootDirectory.c_str()) == FALSE)
						return GetLastError();
				} else
					return 0;
			}
		}
	}

	return 0;
}

int DeleteDirectory(const string &refcstrRootDirectory, bool bDeleteSubdirectories=true) 
{
	return DeleteDirectory(string2wide(refcstrRootDirectory), bDeleteSubdirectories);
}

int Download(string url, int options=FLAG_NONE, string log_file_name="")
{
	if (!global.test_mode  &&  options & (FLAG_CLEAN_DL_NOW | FLAG_CLEAN_DL_LATER)  &&  global.downloads.size()>0  &&  Equals(global.downloaded_filename_last, global.downloads[global.downloads.size()-1])) {
		string filename = "fwatch\\tmp\\" + global.downloaded_filename_last;

		if (DeleteFile(filename.c_str()))
			global.downloads.pop_back();
	}
	
	global.downloaded_filename = PathLastItem(UnQuote(url));

	// Format arguments
	string output = "--output-document=";
	size_t find   = url.find(output);
	
	while(find != string::npos) {
		bool in_quote    = false;
		bool outer_quote = false;
		
		if (find > 0  &&  url[find-1]=='\"') {		// preceding quotation mark
			in_quote    = true;
			outer_quote = true;
		}

		find   += output.length();	// skip to value
		int end = find;
		
		while(end<url.length() && (!isspace(url[end]) || in_quote)) {	// skip to the end of value
			if (url[end]=='"')
				in_quote = !in_quote;

			end++;
		}
		
		// validate
		string path = url.substr(find, end-find);
		path        = PathLastItem(path);
		path        = ReplaceAll(path, "\"", "");
		
		if (path.empty()  ||  path=="..")
			return ErrorMessage(STR_DOWNLOAD_PATH_ERROR);
		
		// reassemble
		url = url.substr(0,find) + (outer_quote ? "" : "\"") + "fwatch\\tmp\\" + path + "\" " + url.substr(end);

		find = url.find(output, find);
	}
	
	output = "-O";
	find   = url.find(output);
	
	while(find != string::npos) {
		if (find>0  &&  isspace(url[find-1])) {	// must precede with whitespace
			find += output.length();
			
			while(find<url.length() && isspace(url[find]))	// skip to value
				find++;
				
			bool in_quote = false;
			int end       = find;
	
			while(end<url.length()  &&  (!isspace(url[end]) || in_quote)) {	// skip to the end of value
				if (url[end] == '"')
					in_quote = !in_quote;

				end++;
			}		

			string path = url.substr(find, end-find);
			path        = PathLastItem(path);
			path        = ReplaceAll(path, "\"", "");
	
			if (path.empty()  ||  path=="..")
				return ErrorMessage(STR_DOWNLOAD_PATH_ERROR);
	
			url = url.substr(0,find) + "\"fwatch\\tmp\\" + path + "\" " + url.substr(end);
		} else
			find += output.length();
		
		find = url.find(output, find);
	}

	string arguments = " --user-agent=\"Mozilla/5.0 (Windows NT 6.1; Win64; x64; rv:103.0) Gecko/20100101 Firefox/103.0\" --tries=1 --no-check-certificate --output-file=fwatch\\tmp\\schedule\\downloadLog.txt --directory-prefix=fwatch\\tmp\\ ";
	
	if (~options & FLAG_OVERWRITE)
		arguments += "--no-clobber ";
	
	if (options & FLAG_SILENT_MODE)
		arguments += "--header=\"ofpgsinstall: 1\" ";

	arguments += url;
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
		return ErrorMessage(STR_ERROR_EXE, "%STR% wget.exe - " + Int2Str(errorCode) + " " + FormatError(errorCode));
	} else
		if (~options & FLAG_SILENT_MODE)
			global.logfile << "Downloading  " << (log_file_name.empty() ? url : log_file_name) << endl;



	// Wait for the program to finish its job
	DWORD exit_code;
	string message = "";

	Sleep(10);

	do {
		if (Abort()) {
			TerminateProcess(pi.hProcess, 0);
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
			
			Sleep(100);
			string filename = "fwatch\\tmp\\" + global.downloaded_filename;
			DeleteFile(filename.c_str());
			return ERROR_USER_ABORTED;
		}

		ParseWgetLog(message);
		GetExitCodeProcess(pi.hProcess, &exit_code);
		Sleep(100);
	}
	while (exit_code == STILL_ACTIVE);

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	
	ParseWgetLog(message);
	
	if (exit_code==1  &&  message.find("not retrieving")!=string::npos) {
		exit_code = 0;
		global.logfile << message << endl;
	}
	
	if (exit_code != 0) {
		string exit_status = "";
		
		switch(exit_code) {
			case 1 : exit_status="Generic error code"; break;
			case 2 : exit_status="Parse error"; break;
			case 3 : exit_status="File I/O error"; break;
			case 4 : exit_status="Network failure"; break;
			case 5 : exit_status="SSL verification failure"; break;
			case 6 : exit_status="Username/password authentication failure"; break;
			case 7 : exit_status="Protocol errors"; break;
			case 8 : exit_status="Server issued an error response"; break;
		}
		
		global.logfile << exit_code << " - " << exit_status << " - " << message << endl;
		ErrorMessage(STR_DOWNLOAD_FAILED, "%STR%\\n" + global.downloaded_filename + "\\n\\n" + message, options & FLAG_SILENT_MODE ? ERROR_WRONG_SCRIPT : ERROR_COMMAND_FAILED);
		string filename = "fwatch\\tmp\\" + global.downloaded_filename;
		DeleteFile(filename.c_str());
	} else
		if (~options & FLAG_SILENT_MODE)
			global.downloads.push_back(global.downloaded_filename);
			
	global.downloaded_filename_last = options & FLAG_CLEAN_DL_LATER ? "" : global.downloaded_filename;
	
	return (exit_code!=0 ? ERROR_COMMAND_FAILED : ERROR_NONE);
}

int Unpack(string file_name, string password="", int options=FLAG_NONE)
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
	string arguments = WrapInQuotes(global.working_directory) + (password.empty() ? "" : " -p"+password) + " x -y -o\"" + destination + "\\\" -bb3 -bsp1 " + "\"fwatch\\tmp\\" + file_name + "\"";

	if (!CreateProcess("fwatch\\data\\7z.exe", &arguments[0], NULL, NULL, true, 0, NULL, NULL, &si, &pi)) {		
		int errorCode = GetLastError();
		return ErrorMessage(STR_ERROR_EXE, "%STR% 7z.exe - " + Int2Str(errorCode) + " " + FormatError(errorCode));
	} else
		global.logfile << "Extracting " << file_name << endl;
		
	Sleep(10);


	// Wait for the program to finish its job
	DWORD exit_code;	
	string message = "";
	int output     = ERROR_NONE;

	do {					
		if (Abort()) {
			TerminateProcess(pi.hProcess, 0);
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
			CloseHandle(logFile);
			return ERROR_USER_ABORTED;
		}
		
		ParseUnpackLog(message, file_name);
		GetExitCodeProcess(pi.hProcess, &exit_code);
		Sleep(100);
	} while (exit_code == STILL_ACTIVE);

	ParseUnpackLog(message, file_name);

	if (exit_code != 0) {
		output = ERROR_COMMAND_FAILED;
		global.logfile << exit_code << " - " << message << endl;
		
		if (message.find("Can not open the file as") != string::npos  &&  message.find("archive") != string::npos) {
			message += " - " + global.lang[STR_UNPACK_REDO_FILE];
			output   = ERROR_WRONG_ARCHIVE;
		}

		if (~options & FLAG_ALLOW_ERROR)
			ErrorMessage(STR_UNPACK_ERROR, "%STR%\\n" + file_name + "\\n\\n" + message);
	}

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	CloseHandle(logFile);

	return output;
}

int MakeDir(string path)
{
	vector<string> directories;
	Tokenize(path, "\\/", directories);

	string build_path = "";
	int result        = 0;

	for (int i=0; i<directories.size(); i++) {
		build_path += (build_path!="" ? "\\" : "") + directories[i];
		result      = CreateDirectory(build_path.c_str(), NULL);

		if (!result) {
			int error_code = GetLastError();

			if (error_code != ERROR_ALREADY_EXISTS)
				return ErrorMessage(STR_MDIR_ERROR, "%STR% " + build_path + " " + Int2Str(error_code) + " " + FormatError(error_code));
		} else
			global.logfile << "Created directory " << build_path << endl;
	}
	
	return ERROR_NONE;
}

int MoveFiles(string source, string destination, string new_name, int options)
{
	WriteProgressFile(INSTALL_PROGRESS, global.lang[STR_ACTION_COPYING]+"...");

	// Optionally create destination directory
	if (options & FLAG_CREATE_DIR) {
		int result = MakeDir(PathNoLastItem(destination));
		
		if (result != ERROR_NONE)
			return result;
	}
	
	// Find files and save them to a list
	vector<string> source_list;
	vector<string> destination_list;
	vector<bool>   is_dir_list;
	vector<string> empty_dirs;
	int buffer_size = 0;
	int recursion   = -1;

	int result = CreateFileList(source, destination+new_name, source_list, destination_list, is_dir_list, options, empty_dirs, buffer_size, recursion);

	if (result != ERROR_NONE)
		return result;
	

	DWORD flags       = MOVEFILE_REPLACE_EXISTING;
	bool FailIfExists = false;
	int return_value  = 0;

	if (~options & FLAG_OVERWRITE) {
		FailIfExists = true;
		flags        = 0;
	}
	
	wstring source_wide;
	wstring destination_wide;

	// For each file in the list
	for (int i=0;  i<source_list.size(); i++) {
		if (Abort())
			return ERROR_USER_ABORTED;

		// Format path for logging
		string destinationLOG = PathNoLastItem(destination_list[i], FLAG_NO_END_SLASH);

		if (destinationLOG.empty())
			destinationLOG = "the game folder";
		
		global.logfile << (options & FLAG_MOVE_FILES ? "Moving" : "Copying") << "  " << ReplaceAll(source_list[i], "fwatch\\tmp\\_extracted\\", "") << "  to  " << destinationLOG;
		
		if (!new_name.empty() && PathLastItem(source_list[i])!=PathLastItem(destination_list[i]))
			global.logfile << "  as  " << PathLastItem(destination_list[i]);

		source_wide      = string2wide(source_list[i]);
		destination_wide = string2wide(destination_list[i]);
		int success      = false;
		
		if (options & FLAG_MOVE_FILES)
			success = MoveFileExW(source_wide.c_str(), destination_wide.c_str(), flags);
		else
			success = CopyFileW(source_wide.c_str(), destination_wide.c_str(), FailIfExists);

	    if (!success) {
			int error_code = GetLastError();
			
	    	if (~options & FLAG_OVERWRITE  &&  error_code==ERROR_ALREADY_EXISTS)
	    		global.logfile << "  - file already exists";
			else {
				global.logfile << endl;
				result = ErrorMessage(
					options & FLAG_MOVE_FILES ? STR_MOVE_ERROR : STR_COPY_ERROR,
					"%STR% " + source_list[i] + " " + global.lang[STR_MOVE_TO_ERROR] + " " + destination_list[i] + " - " + Int2Str(error_code) + " " + FormatError(error_code)
				);
				break;
			}
		}

		global.logfile << endl;
	}

	// Remove empty directories
	if (options & FLAG_MOVE_FILES)
		for (int i=empty_dirs.size()-1; i>=0; i--)
			RemoveDirectory(empty_dirs[i].c_str());

	return result;
}

int ExtractPBO(string source, string destination="", string file_to_unpack="", bool silent=false) 
{
	WriteProgressFile(INSTALL_PROGRESS, global.lang[STR_ACTION_UNPACKINGPBO]+"...");


	// Program extractpbo.exe has a bug with destination argument
	// It won't work if path has a space wrapping in quotes doesn't do anything
	// Temporarily we have to extract it to D:\Temp\_extractedPBO and then move it to the actual destination
	string destination_temp = "";
	string destinationLOG   = "";
	
	if (!destination.empty()) {
		destination_temp = global.working_directory.substr(0,3) + "temp\\_extractedPBO\\";
		destinationLOG   = destination_temp;
	}

	
	
	// Create log file
	SECURITY_ATTRIBUTES sa;
    sa.nLength              = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle       = TRUE;

    HANDLE logFile = CreateFile(TEXT("fwatch\\tmp\\schedule\\PBOLog.txt"),
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
	si.cb 		   = sizeof(si);
	si.dwFlags 	   = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	si.wShowWindow = SW_SHOW;
	si.hStdInput   = NULL;
	si.hStdOutput  = logFile;
	si.hStdError   = logFile;

	string exename    = "ExtractPbo.exe";
	string executable = "fwatch\\data\\" + exename;
	string arguments  = " -NYP ";
	
	if (!file_to_unpack.empty())
		arguments += " -F " + file_to_unpack + " ";
	
	arguments += WrapInQuotes(source) + " " + destination_temp;

	if (!CreateProcess(&executable[0], &arguments[0], NULL, NULL, true, 0, NULL, NULL, &si, &pi)) {
		int errorCode = GetLastError();
		return ErrorMessage(STR_ERROR_EXE, "%STR% " + exename + " - " + Int2Str(errorCode) + " " + FormatError(errorCode));	
	} else 
		if (!silent) {
			global.logfile << "Extracting  " << source;
			
			if (!destination.empty())
				global.logfile << "  to  " << destinationLOG;
			
			global.logfile << endl;
		}
		
	Sleep(10);


	// Wait for the program to finish its job
	DWORD exit_code;	
	string error_message = "";
	
	do {
		if (Abort()) {
			TerminateProcess(pi.hProcess, 0);
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
			CloseHandle(logFile);
			return ERROR_USER_ABORTED;
		}
		
		ParsePBOLog(error_message, exename, source);
		GetExitCodeProcess(pi.hProcess, &exit_code);
		Sleep(100);
	}
	while (exit_code == STILL_ACTIVE);

	ParsePBOLog(error_message, exename, source);

	if (exit_code != 0)
		ErrorMessage(STR_PBO_UNPACK_ERROR, "%STR% " + Int2Str(exit_code) + " - " + error_message);
	else 
		if (!destination.empty()) {
			string dir_name = PathLastItem(source);
			dir_name        = dir_name.substr(0, dir_name.length()-4);
			source          = destination_temp + dir_name;
			destination     = UnQuote(destination) + dir_name;
			MoveFiles(source, destination, "", FLAG_MOVE_FILES | FLAG_OVERWRITE | FLAG_MATCH_DIRS);	
		}

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	CloseHandle(logFile);
	Sleep(600);

	return (exit_code!=0 ? ERROR_COMMAND_FAILED : ERROR_NONE);
}

	//https://support.microsoft.com/en-us/help/167296/how-to-convert-a-unix-time-t-to-a-win32-filetime-or-systemtime
void UnixTimeToFileTime(time_t t, LPFILETIME pft)
{
	LONGLONG ll;
	ll = Int32x32To64(t, 10000000) + 116444736000000000;
	pft->dwLowDateTime = (DWORD)ll;
	pft->dwHighDateTime = ll >> 32;
}

int ChangeFileDate(string file_name, FILETIME *ft) 
{   
	HANDLE file_handle = CreateFile(file_name.c_str(), FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD error_code   = 0;
	
	if (file_handle != INVALID_HANDLE_VALUE) {
		if (!SetFileTime(file_handle, (LPFILETIME)NULL, (LPFILETIME)NULL, ft))
			error_code = GetLastError();
		CloseHandle(file_handle);
	} else
		error_code = GetLastError();
	
	if (error_code != 0)
		return ErrorMessage(STR_EDIT_WRITE_ERROR, "%STR% " + file_name + " - " + Int2Str(error_code) + " " + FormatError(error_code));
	else
		return ERROR_NONE;
}

int ChangeFileDate(string file_name, int timestamp)
{
	FILETIME ft;
	UnixTimeToFileTime(timestamp, &ft);
	return ChangeFileDate(file_name, &ft);
}

int ChangeFileDate(string file_name, string timestamp)
{
	FILETIME ft;
	vector<string> date_item;
	Tokenize(timestamp, "-T:+ ", date_item);
	
	if (date_item.size() == 1)
		return ChangeFileDate(file_name, atoi(timestamp.c_str()));
	else {
		while(date_item.size() < 6)
			date_item.push_back("0");
			
		SYSTEMTIME st;
		st.wYear         = atoi(date_item[0].c_str());
		st.wMonth        = atoi(date_item[1].c_str());
		st.wDay          = atoi(date_item[2].c_str());
		st.wHour         = atoi(date_item[3].c_str());
		st.wMinute       = atoi(date_item[4].c_str());
		st.wSecond       = atoi(date_item[5].c_str());
		st.wMilliseconds = 0;
		SystemTimeToFileTime(&st, &ft);
		return ChangeFileDate(file_name, &ft);
	}
}

	// Read directory and save file modification dates
int CreateTimestampList(string path, int path_cut, vector<string> &namelist, vector<unsigned int> &timelist) 
{
	WIN32_FIND_DATAW fd;
	string wildcard   = path + "\\*";
	wstring wildcardW = string2wide(wildcard);
	HANDLE hFind      = FindFirstFileW(wildcardW.c_str(), &fd);

	if (hFind == INVALID_HANDLE_VALUE) {
		int error_code = GetLastError();
		
		if (error_code==ERROR_FILE_NOT_FOUND  ||  error_code==ERROR_PATH_NOT_FOUND)
			return 0;

		return ErrorMessage(STR_ERROR_FILE_LIST, "%STR% " + wildcard + "  - " + Int2Str(error_code) + " " + FormatError(error_code));
	}
	
	do {
		if (wcscmp(fd.cFileName,L".")==0  ||  wcscmp(fd.cFileName,L"..")==0)
			continue;
			
		string file_name = wide2string(fd.cFileName);
		string full_path = path + "\\" + file_name;

		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			CreateTimestampList(full_path, path_cut, namelist, timelist);
		else {
			//https://www.gamedev.net/forums/topic/565693-converting-filetime-to-time_t-on-windows/
			ULARGE_INTEGER ull;
			ull.LowPart  = fd.ftLastWriteTime.dwLowDateTime;
			ull.HighPart = fd.ftLastWriteTime.dwHighDateTime;
			time_t stamp = ull.QuadPart / 10000000ULL - 11644473600ULL;
	
			namelist.push_back(full_path.substr(path_cut));
			timelist.push_back(stamp);
		}
	} while (FindNextFileW(hFind, &fd) != 0);
	
	FindClose(hFind);
	return 0;
}
// -------------------------------------------------------------------------------------------------------




// Installer commands that get reused --------------------------------------------------------------------

int RequestExecution(string path_to_dir, string file_name)
{		
	DWORD exit_code = 0;
	
	global.logfile << "Asking the user to run " << file_name << endl;
	
	string message = global.lang[STR_ASK_EXE] + ":\\n" + file_name + "\\n\\n" + global.lang[STR_ALTTAB];
	WriteProgressFile(INSTALL_WAITINGFORUSER, message);
	
	message = "File\n" + file_name + "must be manually run\n\nPress OK to start\nPress CANCEL to skip installing this modfolder";
	int msgboxID = MessageBox(NULL, message.c_str(), "Addon Installer", MB_ICONQUESTION | MB_OKCANCEL | MB_DEFBUTTON1);
	
	if (msgboxID == IDCANCEL)
		global.skip_modfolder = true;
	else {
		CopyFile("Aspect_Ratio.hpp", "Aspect_Ratio.backup", false);
		
		// Execute program
		PROCESS_INFORMATION pi;
	    STARTUPINFO si; 
		ZeroMemory( &si, sizeof(si) );
		ZeroMemory( &pi, sizeof(pi) );
		si.cb 		   = sizeof(si);
		si.dwFlags 	   = STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_SHOW;
		
		string executable = path_to_dir + "\\" + file_name;
		string arguments  = " " + global.working_directory;
		
		if (!CreateProcess(&executable[0], &arguments[0], NULL, NULL, false, 0, NULL, NULL, &si, &pi)) {
			MoveFileEx("Aspect_Ratio.backup", "Aspect_Ratio.hpp", MOVEFILE_REPLACE_EXISTING);
			int errorCode = GetLastError();
			global.logfile << "Failed to launch " << file_name << " - " << errorCode << " " << FormatError(errorCode);
			return errorCode;
		}
	
		// Wait for the program to finish its job
		do {
			GetExitCodeProcess(pi.hProcess, &exit_code);
			Sleep(100);
		} while (exit_code == STILL_ACTIVE);
		
		global.logfile << "Exit code: " << exit_code << endl;
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);

		MoveFileEx("Aspect_Ratio.backup", "Aspect_Ratio.hpp", MOVEFILE_REPLACE_EXISTING);
	}
	
	//string path_to_file = path_to_dir + file_name;
	//int result          = E_FAIL;
	/*ITEMIDLIST *pIDL = ILCreateFromPath(path_to_file.c_str());
	
	if (pIDL != NULL) {
		CoInitialize(NULL);
	    int result = SHOpenFolderAndSelectItems(pIDL, 0, 0, 0) == 0;	    
		CoUninitialize();
	    ILFree(pIDL);
	};*/
	
	//if (result != S_OK)
	//HINSTANCE result =
	 
	//ShellExecute(NULL, "open", path_to_dir.c_str(), NULL, NULL, SW_SHOWDEFAULT);

	return (exit_code!=0 ? ERROR_COMMAND_FAILED : ERROR_NONE);
}

int Condition_If_version(const vector<string> &arg, int arg_id, int arg_num) 
{
	if (arg_num < 2)
		return ErrorMessage(STR_ERROR_ARG_COUNT);
	
	// Process arguments (two or three)
	string op          = "==";
	float given_number = 0;

	if (arg_num >= 2) {
		op           = arg[arg_id];
		given_number = atof(arg[arg_id+1].c_str());
	} else	
		if (isdigit(arg[arg_id][0])  ||  arg[arg_id][0]=='.')
			given_number = atof(arg[arg_id].c_str());
		else
			return ErrorMessage(STR_IF_NUMBER_ERROR);

	
	// Execute condition if not nested or if nested inside a valid condition
	bool result = false;
	
	if (global.condition_index<0  ||  global.condition_index>=0 && global.condition[global.condition_index]) {
		float game_version = atof(global.arguments_table["gameversion"].c_str());

		if (op=="=="  ||  op=="=")
			result = game_version == given_number;
				
		if (op=="!="  ||  op=="<>")
			result = game_version != given_number;
				
		if (op=="<")
			result = game_version < given_number;
				
		if (op==">")
			result = game_version > given_number;	
				
		if (op=="<=")
			result = game_version <= given_number;
				
		if (op==">=")
			result = game_version >= given_number;
	}
	
	global.condition_index++;
	global.condition.push_back(result);
	return ERROR_NONE;
}

int Condition_Else()
{
	// If not nested or nested inside a valid condition then reverse a flag which will enable execution of the commands that follow
	if (global.condition_index==0  ||  (global.condition_index>0  &&  global.condition[global.condition_index-1]))
		global.condition[global.condition_index] = !global.condition[global.condition_index];
	
	return ERROR_NONE;
}

int Condition_Endif()
{
	if (global.condition_index >= 0)
		global.condition_index--;
		
	if (global.condition.size() > 0)
		global.condition.pop_back();

	return ERROR_NONE;
}

int Auto_Install(string file, DWORD attributes, int options=FLAG_NONE, string password="")
{
	if (Abort())
		return ERROR_USER_ABORTED;

	if (file.empty())
		return ERROR_NONE;

	string file_with_path = "fwatch\\tmp\\" + file;
	string file_only      = PathLastItem(file);
	size_t dots           = count(file_only.begin(), file_only.end(), '.');
	
	if (attributes & FILE_ATTRIBUTE_DIRECTORY) {
		// Translate this bit to a local bool so that it won't get carried over
		bool force_install = false;
		
		if (options & FLAG_DONT_IGNORE) {
			options      &= ~FLAG_DONT_IGNORE;
			force_install = true;
		}
		
		// If the folder has the same name as the mod we're installing then merge it
		if (IsModName(file_only)) {
			options &= ~FLAG_RUN_EXE;
			return MoveFiles(file_with_path, "", global.current_mod_new_name, FLAG_MOVE_FILES | FLAG_OVERWRITE | FLAG_MATCH_DIRS);
		}

		// If directory ends with _anim or _anims then move it to IslandCutscenes
		if (file_only.length()>=4 && Equals(file_only.substr(file_only.length()-5),"anim") || file_only.length()>=5 && Equals(file_only.substr(file_only.length()-5),"_anim") || file_only.length()>=6 && Equals(file_only.substr(file_only.length()-5),"_anims")) {
			options &= ~FLAG_RUN_EXE;
			string destination = global.current_mod_new_name + "\\IslandCutscenes\\" + (options & FLAG_RES_ADDONS ? "_Res\\" : "") ;
			return MoveFiles(file_with_path, destination, "", FLAG_MOVE_FILES | FLAG_OVERWRITE | FLAG_CREATE_DIR | FLAG_MATCH_DIRS);
		}

		// If directory has a dot in its name then move it to Missions
		if (dots > 0) {
			string destination = GetMissionDestinationFromSQM(file_with_path+"\\mission.sqm");
		
			if (destination != "Addons") {
				string file_lower = lowercase(file_only);	
				
				if ((Equals(destination,"Missions") || Equals(destination,"MPMissions")) && (options & FLAG_DEMO_MISSIONS || file_lower.find("demo")!=string::npos || file_lower.find("template")!=string::npos))
					destination += "Users";
				
				options &= ~FLAG_RUN_EXE;
				return MoveFiles(file_with_path, global.current_mod_new_name+"\\"+destination+"\\", "", FLAG_MOVE_FILES | FLAG_OVERWRITE | FLAG_CREATE_DIR | FLAG_MATCH_DIRS);
			}
		}

		// Check if the folder name is addons/bin/dta/worlds/campaigns etc.
		int index                        = DIR_NONE;
		bool missions_but_different_name = false;
		
		if (!Equals(file_only,"_extracted")) {
			for (int i=DIR_ADDONS; i<DIR_MAX && index==DIR_NONE; i++)
				if (Equals(file_only,mod_subfolders[i]))
					index = i;
				
			// Folder could be an sp missions folder but named differently
			if (index == DIR_NONE) {
				string overview = file_with_path + "\\overview.html";
				
				if (GetFileAttributes(overview.c_str()) != INVALID_FILE_ATTRIBUTES) {
					index                       = DIR_MISSIONS;
					missions_but_different_name = true;
				}
			}
		}

		// If so then move it to the modfolder
		if (index != DIR_NONE) {
			string destination = global.current_mod_new_name + "\\";
			string new_name    = "";
			
			if (!missions_but_different_name) {
				// If it's a Missions/MPMissions folder and it contains a single subfolder then move that subfolder instead
				if (index==DIR_MISSIONS  ||  index==DIR_MPMISSIONS) {
					string destination_overview_path = global.current_mod_new_name + "\\Missions\\overview.html";
					DIRECTORY_INFO dir               = ScanDirectory(file_with_path);
					
					if (dir.error_code == ERROR_NONE) {
						if (dir.number_of_files==0  &&  dir.number_of_dirs==1) {
							file_only = wide2string(dir.file_list[0]);
							
							// If that subfolder matches mod name then merge files; otherwise copy the entire subfolder
							if ((index==DIR_MISSIONS && IsModName(file_only)) || index==DIR_MPMISSIONS) {
								file          += "\\" + file_only;
								file_with_path = "fwatch\\tmp\\" + file;
								new_name       = DIR_MISSIONS ? "Missions" : "MPMissions";
							}
						}
					} else
						return dir.error_code;
				}
			} else
				destination += "Missions\\";
	
			options &= ~FLAG_RUN_EXE;
			return MoveFiles(file_with_path, destination, new_name, FLAG_MOVE_FILES | FLAG_OVERWRITE | FLAG_CREATE_DIR | FLAG_MATCH_DIRS);
		}
		
		bool scan_directory = true;
		
		// If the folder is some other modfolder then ignore it
		if (!Equals(file_only,"_extracted") && !Equals(file_only,"Res") && !force_install)
			for (int i=DIR_ADDONS; i<=DIR_WORLDS && scan_directory; i++) {
				string mod_subpath = file_with_path + "\\" + mod_subfolders[i];
				DWORD attributes   = GetFileAttributes(mod_subpath.c_str());
				
				if (attributes!=INVALID_FILE_ATTRIBUTES  &&  attributes & FILE_ATTRIBUTE_DIRECTORY)
					scan_directory = false;
			}

		// Normal directory - scan its contents
		if (scan_directory) {
			DIRECTORY_INFO dir = ScanDirectory(file_with_path);
			
			if (dir.error_code != ERROR_NONE)
				return dir.error_code;

			// If archive contains a single dir then set option to force to scan it
			if (dir.number_of_files==0  &&  dir.number_of_dirs==1  &&  Equals(file_with_path,"fwatch\\tmp\\_extracted"))
				options |= FLAG_DONT_IGNORE;
			
			for (int i=0; i<dir.file_list.size(); i++) {
				string file_inside       = wide2string(dir.file_list[i]);
				string path_to_this_file = file + "\\" + file_inside;
				int result               = ERROR_NONE;
				bool is_dir              = dir.attributes_list[i] & FILE_ATTRIBUTE_DIRECTORY;
				
				// Files that are next to the wanted modfolder are moved without looking at them
				if (dir.number_of_wanted_mods>0  &&  (!is_dir || is_dir && dir.mod_sub_id_list[i]!=DIR_MISSIONS && dir.mod_sub_id_list[i]!=DIR_MPMISSIONS && !dir.is_mod_list[i] && _wcsicmp(dir.file_list[i].c_str(),L"Res")!=0 && _wcsicmp(dir.file_list[i].c_str(),L"ResAddons")!=0)) {
					string source      = file_with_path+"\\"+wide2string(dir.file_list[i]);
					string destination = global.current_mod_new_name+"\\";
					string new_name    = "";
					
					// If it's "addons" folder then it probably contains island cutscenes; move it with changed name
					if (dir.mod_sub_id_list[i] == DIR_ADDONS)
						new_name = "IslandCutscenes";
					
					options &= ~FLAG_RUN_EXE;
					result = MoveFiles(source, destination, new_name, FLAG_MOVE_FILES | FLAG_OVERWRITE | FLAG_CREATE_DIR | FLAG_MATCH_DIRS);
				} else {
					file_inside = lowercase(file_inside);

					// If a folder contains keywords then set mission to transfer to "MissionsUsers" instead of "Missions"
					enum KEYWORDS {
						KEY_DEMO,
						KEY_MISSION,
						KEY_USER,
						KEY_EDITOR,
						KEY_TEMPLATE,
						KEY_RES,
						KEY_ADDONS
					};
					vector<string> keywords;
					vector<bool> key_exists;
					keywords.push_back("demo");
					keywords.push_back("mission");
					keywords.push_back("user");
					keywords.push_back("editor");
					keywords.push_back("template");
					keywords.push_back("res");
					keywords.push_back("addons");
					key_exists.resize(keywords.size());
					
					for (int j=0; j<keywords.size(); j++)
						key_exists[j] = file_inside.find(keywords[j]) != string::npos;
					
					if (((key_exists[KEY_DEMO] || key_exists[KEY_EDITOR] || key_exists[KEY_TEMPLATE]) && key_exists[KEY_MISSION]) || key_exists[KEY_USER])
						options |= FLAG_DEMO_MISSIONS;
						
					// If folder name is "res" or contains words "res" and "addons" then island cutscenes inside will go to IslandCutscenes\_res
					if (Equals(file_inside,"Res") || (key_exists[KEY_RES] && key_exists[KEY_ADDONS]))
						options |= FLAG_RES_ADDONS;
					
					result = Auto_Install(path_to_this_file, dir.attributes_list[i], options, password);
				}
				
				if (result != ERROR_NONE)
					return result;
			}
		}
	} else {
		// If it's a file then check its extension
		string file_extension = GetFileExtension(file);

		if (file.substr(0,11) == "_extracted\\")
			file = file.substr(11);
		
		file = WrapInQuotes(file);
		
		enum VALID_EXTENSIONS {
			EXT_PBO,
			EXT_RAR,
			EXT_ZIP,
			EXT_7Z,
			EXT_ACE,
			EXT_EXE,
			EXT_CAB
		};
		
		string valid_extensions[] = {
			"pbo",
			"rar",
			"zip",
			"7z",
			"ace",
			"exe",
			"cab"
		};
		int number_of_extensions = sizeof(valid_extensions) / sizeof(valid_extensions[0]);
		int extension_index      = -1;
		
		for (int i=0; i<number_of_extensions && extension_index<0; i++)
			if (file_extension == valid_extensions[i])
				extension_index = i;
		
		switch(extension_index) {
			// Unpack PBO and detect if it's an addon or a mission
			case EXT_PBO : {
				options           &= ~FLAG_RUN_EXE;
				string destination = "Addons";
				
				if (dots > 1) {
					destination = "MPMissions";
	
					if (ExtractPBO(global.working_directory+"\\"+file_with_path, "", "mission.sqm", 1) == 0) {
						string extracted_dir = PathNoLastItem(file_with_path) + file.substr(0, file.length()-4);
						destination          = GetMissionDestinationFromSQM(extracted_dir+"\\mission.sqm");
						DeleteDirectory(extracted_dir);
					}
				}
					
				return MoveFiles(file_with_path, global.current_mod_new_name+"\\"+destination+"\\", "", FLAG_MOVE_FILES | FLAG_OVERWRITE | FLAG_CREATE_DIR);
			}
			
			// Unpack archive and scan its contents
			case EXT_RAR : 
			case EXT_ZIP : 
			case EXT_7Z  : 
			case EXT_ACE : 
			case EXT_CAB : {
				int result = Unpack(file_with_path.substr(11), password);
								
				if (result == ERROR_NONE)
					return Auto_Install(PathNoLastItem(file_with_path.substr(11)) + "_extracted", FILE_ATTRIBUTE_DIRECTORY, options, password);
				else
					return result;
			}

			// Unpack exe and scan its contents; if failed to unpack and nothing else was installed then ask the user to run it
			case EXT_EXE : {
				int result = Unpack(file_with_path.substr(11), password, FLAG_ALLOW_ERROR);
				
				if (result == ERROR_NONE) {
					string resource_path = PathNoLastItem(file_with_path) + "_extracted\\.rsrc";
					
					// Make sure that this exe is not a program
					if (GetFileAttributes(resource_path.c_str()) == INVALID_FILE_ATTRIBUTES)
						return Auto_Install(PathNoLastItem(file_with_path.substr(11))+"_extracted", FILE_ATTRIBUTE_DIRECTORY, options, password);
				} else
					if (options & FLAG_RUN_EXE)
						return RequestExecution("", file);

				break;
			}
		}
	}
	
	return ERROR_NONE;
}

	// Clean downloads and save mod version
void EndModVersion() 
{
	if (global.current_mod.empty())
		return;
		
	if (global.current_mod_alias.size() > global.saved_alias_array_size)
		global.current_mod_alias.erase(global.current_mod_alias.begin()+global.saved_alias_array_size, global.current_mod_alias.begin()+global.current_mod_alias.size());
		
	WriteProgressFile(INSTALL_PROGRESS, global.lang[STR_ACTION_CLEANING]);
		
	// Remove downloaded files
	if (!global.test_mode) {
		for (int i=0; i<global.downloads.size(); i++) {
			string filename = "fwatch\\tmp\\" + global.downloads[i];

			if (!DeleteFile(filename.c_str())) {
				int errorCode   = GetLastError();
				string errorMSG = FormatError(errorCode);
				global.logfile << "Failed to delete " << filename << " - " << errorCode << " " << errorMSG << endl;
			}
		}

		DeleteDirectory("fwatch\\tmp\\_extracted");
		global.downloads.clear();
	}
	
	WriteModID(global.current_mod_new_name, global.current_mod_id+";"+global.current_mod_version, global.current_mod_keepname);	
}

	// Verify folder, clean up and reset variables
void EndMod()
{
	if (global.current_mod.empty())
		return;
		
	WriteProgressFile(INSTALL_PROGRESS, global.lang[STR_ACTION_CLEANING]);
		
	// Check if folder even exists
	DWORD dir = GetFileAttributesA(global.current_mod_new_name.c_str());

	if (dir==INVALID_FILE_ATTRIBUTES  ||  !(dir & FILE_ATTRIBUTE_DIRECTORY)) {
		if (global.missing_modfolders != "")
			global.missing_modfolders += ", ";

		global.missing_modfolders += global.current_mod;
		global.logfile << "Modfolder " << global.current_mod << " wasn't actually installed!" << endl;
	}

	EndModVersion();
	
	global.current_mod     = "";
	global.skip_modfolder  = false;
	global.condition_index = -1;
	global.condition.clear();
	global.current_mod_alias.clear();
}
// -------------------------------------------------------------------------------------------------------




int main(int argc, char *argv[])
{
	// Process arguments
	global.arguments_table.insert(pair<string,string>("gameversion","1.99"));
	global.arguments_table.insert(pair<string,string>("assignid",""));
	global.arguments_table.insert(pair<string,string>("assignidpath",""));
	global.arguments_table.insert(pair<string,string>("assignname",""));
	global.arguments_table.insert(pair<string,string>("assignkeepname",""));
	global.arguments_table.insert(pair<string,string>("testmod",""));
	global.arguments_table.insert(pair<string,string>("testdir",""));
	global.arguments_table.insert(pair<string,string>("installid",""));
	global.arguments_table.insert(pair<string,string>("installdir",""));
	global.arguments_table.insert(pair<string,string>("downloadscript",""));
	global.arguments_table.insert(pair<string,string>("evoice",""));
	global.arguments_table.insert(pair<string,string>("language","English"));

	// Separate arguments:
	// arguments for this program go to the table
	// arguments for gameRestart.exe go to a separate string
	for (int i=1; i<argc; i++) {
		string current_argument = (string)argv[i];
		bool found              = false;
		
		for (map<string,string>::iterator it=global.arguments_table.begin(); it!=global.arguments_table.end(); ++it) {
			string table_argument = "-" + it->first + "=";
			
			if (Equals(current_argument.substr(0, table_argument.length()), table_argument)) {
				it->second = current_argument.substr(table_argument.length());
				found = true;
				break;
			}
		}
		
		if (!found)
			global.gamerestart_arguments += current_argument + " ";
	}
	
	global.test_mode = !(global.arguments_table["testmod"].empty());
	
	Tokenize(global.arguments_table["installid"] , ",", global.mod_id);
	Tokenize(global.arguments_table["installdir"], ",", global.mod_name);



	
	// Load language
	string stringtable[][STR_MAX] = {
		{
			"Initializing",
			"Fetching installation script",
			"Reading installation script",
			"Connecting",
			"Downloading",
			"Downloaded",
			"Extracting",
			"Unpacking PBO",
			"Packing PBO",
			"Copying",
			"Copying downloaded file to the fwatch\\tmp",
			"Cleaning up",
			"Preparing to install a mod",
			"Deleting",
			"Renaming",
			"Editing",
			"Installation aborted by user",
			"Installation complete!",
			"Done\\nbut mods %MOD% are still missing\\nOpen fwatch\\data\\addonInstallerLog.txt for details",
			"Installation progress:",
			"ALT+TAB to the desktop",
			"ERROR",
			"Can't create logfile",
			"Can't read install script",
			"Incorrect script version",
			"In version",
			"On line",
			"Failed to launch",
			"Not enough arguments",
			"Failed to list files in ",
			"Missing file name argument",
			"Path is leaving current directory",
			"Installation script is invalid",
			"Invalid installator arguments",
			"Failed to allocate buffer",
			"left",
			"total",
			"Invalid download destination",
			"Failed to download",
			"Failed to find",
			"remove this file and download again",
			"Failed to extract",
			"Failed to create directory",
			"Failed to get attributes of",
			"Source path is leaving current directory",
			"Destination path is leaving current directory",
			"Not allowed to move files out of the game directory",
			"Failed to move",
			"to",
			"Failed to copy",
			"Failed to rename",
			"to",
			"New file name contains slashes",
			"Wildcards in the path",
			"Missing new file name",
			"Failed to delete",
			"Failed to move to recycle bin",
			"You must manually run",
			"You must manually download",
			"Select folder with the downloaded file",
			"Missing version number",
			"Not a PBO file",
			"Failed to create PBO",
			"Failed to unpack PBO",
			"Failed to read file",
			"Failed to create file",
			"Reading mission.sqm"
		},
		{
			"Запуск",		//0
			"Получение скрипта установки",		//1
			"Считывание файлов",		//2
			"Подключение",		//3
			"Загрузка",		//4
			"Загрузка завершена",		//5
			"Извлечение файлов",		//6
			"Распаковка архива PBO",		//7
			"Создание архива PBO",		//8
			"Копирование",		//9
			"Копирование загруженного файла в fwatch\tmp",		//10
			"Удаление временных файлов",		//11
			"Начало установки мода",		//12
			"Удаление файлов мода",		//13
			"Переименование файлов мода",		//14
			"Редактирование файлов мода",		//15
			"Установка прервана пользователем",		//16
			"Установка завершена!",		//17
			"Установка завершена\nно отсутствуют моды %MOD%. Дополнительная информация в fwatch\\data\\addonInstallerLog.txt",		//18
			"Процесс установки:",		//19
			"Нажмите ALT+TAB, чтобы свернуть игру",		//20
			"ОШИБКА",		//21
			"Невозможно создать файл журнала установки",		//22
			"Невозможно считать скрипт установки",		//23
			"Неверная версия скрипта установки",		//24
			"В версии",		//25
			"В строке",		//26
			"Ошибка при запуске",		//27
			"Недостаточно аргументов функции",		//28
			"Ошибка при создании списка файлов в папке ",		//29
			"Отсутствует имя файла аргумента",		//30
			"Путь не соответствует текущей папке",		//31
			"Неверный скрипт установки",		//32
			"Неверные аргументы мастера установки",		//33
			"Ошибка при выделении понятий",		//34
			"осталось",		//35
			"всего",		//36
			"Неверный путь установки",		//37
			"Ошибка при загрузке",		//38
			"Совпадений не найдено",		//39
			"удалите этот файл и загрузите снова",		//40
			"Ошибка при извлечении",		//41
			"Ошибка при создании папки",		//42
			"Ошибка при считывании атрибутов файла",		//43
			"Исходный путь не соответствует текущей папке",		//44
			"Путь назначения не соответствует текущей папке",		//45
			"Невозможно переместить файлы из папки игры",		//46
			"Ошибка при перемещении",		//47
			"в",		//48
			"Ошибка при копировании",		//49
			"Ошибка при переименовании файла",		//50
			"в",		//51
			"Новое имя файла содержит слеши",		//52
			"Путь содержит символы подстановки",		//53
			"Отсутствует новое имя файла",		//54
			"Ошибка при удалении",		//55
			"Ошибка при перемещении в Корзину",		//56
			"Необходимо запустить вручную",		//57
			"Необходимо загрузить вручную",		//58
			"Выберите папку с загруженным файлом",		//59
			"Отсутствует номер версии",		//60
			"Не файл типа PBO",		//61
			"Ошибка при создании файла PBO",		//62
			"Ошибка при извлечении из архива PBO",		//63
			"Ошибка при считывании",		//64
			"Ошибка при создании файла",		//65
			"Считывание mission.sqm"		//66
		},
		{
			"Przygotowywanie",
			"Pobieraniu skryptu instalacyjnego",
			"Przetwarzanie skryptu instalacyjnego",
			"Ј№czenie",
			"Pobieranie",
			"Pobrano",
			"Wypakowywanie",
			"Wypakowywanie PBO",
			"Pakowanie PBO",
			"Kopiowanie plikуw",
			"Kopiowanie plikуw do fwatch\\tmp",
			"Porz№dkowanie",
			"Przygotowywanie do instalacji modu",
			"Usuwanie plikуw",
			"Zmienianie nazwy plikуw",
			"Edytowanie plikуw",
			"Instalacja przerwana przez uїytkownika",
			"Instalacja zakoсczona!",
			"Koniec instalacji\\nale brakuje modуw %MOD%\\nSzczegуіy w pliku fwatch\\data\\addonInstallerLog.txt",
			"Postкp instalacji:",
			"ALT+TAB їeby przejњж do pulpitu",
			"BЈҐD",
			"Nie moїna utworzyж zapisu instalacji",
			"Nie moїna odczytaж skryptu instalacyjnego",
			"Niepoprawna wersja skryptu instalacyjnego",
			"W wersji",
			"W linijce",
			"Nie moїna uruchomiж",
			"Brakuje argumentуw",
			"Nie moїna utworzyж listy plikуw z ",
			"Brakuje nazwy pliku",
			"Њcieїka wychodzi poza obecny katalog",
			"Skrypt instalacyjny jest bікdny",
			"Bікdne argumenty instalatora",
			"Nie moїna zarezerwowaж pamiкci",
			"zostaіo",
			"w sumie",
			"Nieprawidіowy katalog docelowy dla њci№gniкtego pliku",
			"Nie moїna pobraж",
			"Nie znaleziono",
			"usuс ten plik i њci№gnij ponownie",
			"Nie moїna rozpakowaж",
			"Nie moїna utworzyж katalogu",
			"Nie moїna odczytaж atrybutуw",
			"Њcieїka џrуdіowa wychodzi poza obecny katalog",
			"Њcieїka docelowa wychodzi poza obecny katalog",
			"Nie moїna przenosiж plikуw poza katalog z gr№",
			"Nie moїna przenieњж",
			"do",
			"Nie moїna skopiowaж",
			"Nie moїna zmieniж nazwy",
			"na",
			"Nowa nazwa pliku zawiera ukoњniki",
			"Њcieїka zawiera symbole zastкpcze",
			"Brakuje nowej nazwy pliku",
			"Nie moїna skasowaж",
			"Nie moїna przenieњж do kosza",
			"Musisz rкcznie uruchomiж",
			"Musisz rкcznie pobraж",
			"Wybierz katalog z pobranym plikiem",
			"Brakuje numeru wersji",
			"Plik nie jest PBO",
			"Nie moїna utworzyж PBO",
			"Nie moїna rozpakowaж PBO",
			"Nie moїna wczytaж pliku",
			"Nie moїna utworzyж pliku",
			"Przetwarzanie mission.sqm"
		}
	};
	
	global.lang_eng = stringtable[0];
	global.lang     = stringtable[0];
	
	if (Equals(global.arguments_table["language"],"Russian"))
		global.lang = stringtable[1];
	
	if (Equals(global.arguments_table["language"],"Polish"))
		global.lang = stringtable[2];

	
	// Find current directory
	TCHAR pwd[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, pwd);
	global.working_directory = (string)pwd;

	// When testing outside of the game change path to the game root dir
	if (global.test_mode) {
		global.working_directory = ReplaceAll(global.working_directory, "\\fwatch\\data", "");		
		SetCurrentDirectory(global.working_directory.c_str());
	}




	// If ordered to create id file for a mod
	if (!global.arguments_table["assignidpath"].empty()  &&  !global.arguments_table["assignid"].empty()) {
		global.current_mod = global.arguments_table["assignname"];
		return WriteModID(global.arguments_table["assignidpath"], global.arguments_table["assignid"], global.arguments_table["assignkeepname"]);
	}



	WriteProgressFile(INSTALL_PROGRESS, global.lang[STR_ACTION_INIT]);


	// Start listen thread
	_beginthread(ReceiveInstructions, 0, (void *)(0));







	// Create a log file
	global.logfile.open("fwatch\\data\\addonInstallerLog.txt", ios::out | ios::app);

	if (!global.logfile.is_open()) {
		WriteProgressFile(INSTALL_ERROR, (global.lang[STR_ERROR]+"\\n"+global.lang[STR_ERROR_LOGFILE]));
		return ERROR_LOGFILE;
	}

	SYSTEMTIME st;
	GetLocalTime(&st);
	global.logfile << "\n--------------\n\n"
			<< st.wYear << "." 
			<< LeadingZero(st.wMonth) << "." 
			<< LeadingZero(st.wDay) << "  " 
			<< LeadingZero(st.wHour) << ":" 
			<< LeadingZero(st.wMinute) << ":" 
			<< LeadingZero(st.wSecond) << endl;
			
			
	// Set up global variables for testing mode
	if (global.test_mode) {
		global.current_mod              = global.arguments_table["testmod"];
		global.current_mod_version      = "1";
		global.current_mod_version_date = time(0);
		
		if (global.arguments_table["testdir"].empty())
			global.current_mod_new_name = global.arguments_table["testmod"];
		else
			global.current_mod_new_name = global.arguments_table["testdir"];
			
		global.logfile << "Test Mode - " << global.current_mod;
		
		if (!Equals(global.current_mod,global.current_mod_new_name))
			global.logfile << " as " << global.current_mod_new_name;
		 
		global.logfile << endl;
	}




	// Define allowed commands
	enum SCRIPTING_COMMANDS_ID {
		COMMAND_AUTO_INSTALL,
		COMMAND_DOWNLOAD,
		COMMAND_UNPACK,
		COMMAND_MOVE,
		COMMAND_COPY,
		COMMAND_MAKEDIR,
		COMMAND_ASK_RUN,
		COMMAND_BEGIN_MOD,
		COMMAND_DELETE,
		COMMAND_RENAME,
		COMMAND_ASK_DOWNLOAD,
		COMMAND_IF_VERSION,
		COMMAND_ELSE,
		COMMAND_ENDIF,
		COMMAND_MAKEPBO,
		COMMAND_EXTRACTPBO,
		COMMAND_EDIT,
		COMMAND_BEGIN_VERSION,
		COMMAND_ALIAS,
		COMMAND_FILEDATE,
		COMMAND_INSTALL_VERSION,
		COMMAND_EXIT
	};
	
	string command_names[] = {
		"auto_install",	
		"download",		
		"get", 			
		"unpack", 		
		"extract", 		
		"move", 		
		"copy", 		
		"makedir",		
		"newfolder", 	
		"ask_run", 		
		"ask_execute", 	
		"begin_mod",	
		"delete",		
		"remove",		
		"rename",		
		"ask_download",	
		"ask_get",		
		"if_version",	
		"else",			
		"endif",		
		"makepbo",		
		"extractpbo",	
		"unpackpbo",	
		"unpbo",		
		"edit",			
		"begin_ver",	
		"alias",
		"merge_with",
		"filedate",
		"install_version",
		"exit",
		"quit"
	};

	int match_command_name_to_id[] = {
		COMMAND_AUTO_INSTALL,
		COMMAND_DOWNLOAD,
		COMMAND_DOWNLOAD,
		COMMAND_UNPACK,
		COMMAND_UNPACK,
		COMMAND_MOVE,
		COMMAND_COPY,
		COMMAND_MAKEDIR,
		COMMAND_MAKEDIR,
		COMMAND_ASK_RUN,
		COMMAND_ASK_RUN,
		COMMAND_BEGIN_MOD,
		COMMAND_DELETE,
		COMMAND_DELETE,
		COMMAND_RENAME,
		COMMAND_ASK_DOWNLOAD,
		COMMAND_ASK_DOWNLOAD,
		COMMAND_IF_VERSION,
		COMMAND_ELSE,
		COMMAND_ENDIF,
		COMMAND_MAKEPBO,
		COMMAND_EXTRACTPBO,
		COMMAND_EXTRACTPBO,
		COMMAND_EXTRACTPBO,
		COMMAND_EDIT,
		COMMAND_BEGIN_VERSION,
		COMMAND_ALIAS,
		COMMAND_ALIAS,
		COMMAND_FILEDATE,
		COMMAND_INSTALL_VERSION,
		COMMAND_EXIT,
		COMMAND_EXIT
	};
	
	int control_flow_commands[] = {
		COMMAND_BEGIN_MOD,
		COMMAND_BEGIN_VERSION,
		COMMAND_IF_VERSION,
		COMMAND_ELSE,
		COMMAND_ENDIF,
		COMMAND_INSTALL_VERSION,
		COMMAND_EXIT
	};
	
	enum COMMAND_SWITCHES {
		SWITCH_NONE,
		SWITCH_PASSWORD       = 0x1,
		SWITCH_NO_OVERWRITE   = 0x2,
		SWITCH_MATCH_DIR      = 0x4,
		SWITCH_KEEP_SOURCE    = 0x8,
		SWITCH_INSERT         = 0x10,
		SWITCH_NEWFILE        = 0x20,
		SWITCH_APPEND         = 0x40,
		SWITCH_MATCH_DIR_ONLY = 0x80,
		SWITCH_TIMESTAMP      = 0x100,
		SWITCH_MAX            = 0x200
	};
	
	string command_switches_names[] = {
		"",
		"/password:",
		"/no_overwrite",
		"/match_dir",
		"/keep_source",
		"/insert",
		"/newfile",
		"/append",
		"/match_dir_only",
		"/timestamp:"
	};
	
	// Automatic filling with empty strings for a command when not enough arguments were passed
	// This is for the commands that can be used with or without arguments
	int command_minimal_arg[] = {
		0,
		0,
		1,
		3,
		3,
		1,
		1,
		0,
		1,
		2,
		0,
		0,
		0,
		0,
		1,
		2,
		0,
		0,
		0,
		0,
		1,
		0
	};

	int number_of_commands = sizeof(command_names) / sizeof(command_names[0]);
	int number_of_switches = sizeof(command_switches_names) / sizeof(command_switches_names[0]);
	int number_of_ctrlflow = sizeof(control_flow_commands) / sizeof(control_flow_commands[0]);




	// Download installation script
	if (!global.arguments_table["downloadscript"].empty()) {
		WriteProgressFile(INSTALL_PROGRESS, global.lang[STR_ACTION_GETSCRIPT]);

		string url = GetFileContents(global.arguments_table["downloadscript"]) + " --verbose \"--output-document=fwatch\\tmp\\installation script\"";
		int result = Download(url, FLAG_OVERWRITE | FLAG_SILENT_MODE);
		DeleteFile(global.arguments_table["downloadscript"].c_str());

		if (result > 0) {
			global.logfile << "\n--------------\n\n";
			global.logfile.close();
			return ERROR_NO_SCRIPT;
		}
	}
	
	// Open installation script
	WriteProgressFile(INSTALL_PROGRESS, global.lang[STR_ACTION_READSCRIPT]);

    vector<string> instructions;
    fstream script_file;
    string script_file_name    = global.test_mode ? "fwatch\\data\\addonInstaller_test.txt" : "fwatch\\tmp\\installation script";
	string script_file_content = GetFileContents(script_file_name);
	
	if (script_file_content.empty()) {
		global.logfile << "Failed to open " << script_file_name << "\n\n--------------\n\n";
		WriteProgressFile(INSTALL_ERROR, (global.lang[STR_ERROR]+"\\n"+global.lang[STR_ERROR_READSCRIPT]));
		global.logfile.close();
		return ERROR_NO_SCRIPT;
	}	
	
	
	// Parse installation script and store instructions in vectors
	int word_begin            = -1; //number of column where a phrase begins
	int word_count            = 1;  //number of found phrases in the current line
	int word_line_num         = 1;	//line count for the entire script
	int word_line_num_local   = 1;  //line count for single version of the mod
	int command_id            = -1;
	int last_command_line_num = -1;
	int last_url_list_id      = -1;
	int url_arg_key           = -1;
	bool in_quote             = false;
	bool remove_quotes        = true;
	bool url_block            = false;
	bool url_line             = false;
	
	// Table storing commands from the script
	struct {
		vector<int> id;				//command enum
		vector<bool> ctrl_flow;		//is it a control flow command
		vector<int> line_num;		//position in the text file
		vector<int> switches;		//bit mask
		vector<int> arg_start;		//arguments starting index in the other table
		vector<int> arg_num;		//number of arguments passed to this command
		vector<int> url_start;		//urls starting index in the other table
		vector<int> url_num;		//number of urls passed to this command
		vector<string> password;	//password switch passed to this command
		vector<string> arguments;	//table storing arguments associated with commands (not parallel)
		vector<string> timestamp;   //timestamp switch passed to this command
	} current_script_command;

	// Table storing download urls associated with the commands	
	struct {
		vector<int> arg_start;		//arguments starting index in the other table
		vector<int> arg_num;		//number of arguments passed to this url
		vector<int> line_num;		//position in the text file
		vector<string> link;		//actual url
		vector<string> arguments;	//table storing url arguments associated with the urls (not parallel)
	} current_script_command_urls;
	
	for (int i=0; i<=script_file_content.length(); i++) {
		bool end_of_word = i==script_file_content.length() || isspace(script_file_content[i]);
		
		// When quote
		if (script_file_content[i] == '"')
			in_quote = !in_quote;
		
		// If beginning of an url block
		if (script_file_content[i] == '{' && word_begin<0) {
			url_block = true;
	
			// if bracket is the first thing in the line then it's auto installation
			if (word_count == 1) {
				last_command_line_num = word_line_num;
				current_script_command.id.push_back(COMMAND_AUTO_INSTALL);
				current_script_command.ctrl_flow.push_back(false);
				current_script_command.line_num.push_back(word_line_num_local);
				current_script_command.switches.push_back(SWITCH_NONE);
				current_script_command.arg_start.push_back(current_script_command.arguments.size());
				current_script_command.arg_num.push_back(0);
				current_script_command.url_start.push_back(current_script_command_urls.link.size());
				current_script_command.url_num.push_back(0);
				current_script_command.password.push_back("");
				current_script_command.timestamp.push_back("");
			} else
				// if bracket is an argument for the command
				if (command_id != -1) {
					current_script_command.arguments.push_back("<dl>");
					current_script_command.arg_num[current_script_command.arg_num.size()-1]++;
				}
			
			continue;
		}
		
		// If ending of an url block
		if (script_file_content[i] == '}' && url_block) {
			end_of_word = true;
			
			// If there's space between last word and the closing bracket
			if (word_begin == -1) {	
				url_block = false;
				url_line  = false;
				word_count++;
				continue;
			}
		}
		
		// Remember beginning of the word
		if (!end_of_word && word_begin<0) {
			word_begin = i;
			
			// If custom delimeter - jump to the end of the argument
			if (script_file_content.substr(word_begin,2) == ">>") {
				word_begin   += 3;
				size_t end    = script_file_content.find(script_file_content[i+2], i+3);
				end_of_word   = true;
				i             = end==string::npos ? script_file_content.length() : end;
				remove_quotes = false;
			}
		}
		
		// When hit end of the word
		if (end_of_word && word_begin>=0 && !in_quote) {
			string word = script_file_content.substr(word_begin, i-word_begin);

			if (remove_quotes)
				word = UnQuote(word);
			else
				remove_quotes = true;
				
			// If first word in the line
			if (word_count == 1 && !url_block) {
				command_id = -1;
				
				// Check if it's a valid command
				if (IsURL(word))
					command_id = COMMAND_AUTO_INSTALL;
				else
					for (int j=0; j<number_of_commands && command_id==-1; j++)
						if (Equals(word, command_names[j]))
							command_id = match_command_name_to_id[j];
				
				// If so then add it to database, otherwise skip this line
				if (command_id != -1) {
					last_command_line_num = word_line_num;
					current_script_command.id.push_back(command_id);
					current_script_command.ctrl_flow.push_back(false);
					current_script_command.line_num.push_back(word_line_num_local);
					current_script_command.switches.push_back(SWITCH_NONE);
					current_script_command.arg_start.push_back(current_script_command.arguments.size());
					current_script_command.arg_num.push_back(0);
					current_script_command.url_start.push_back(current_script_command_urls.link.size());
					current_script_command.url_num.push_back(0);
					current_script_command.password.push_back("");
					current_script_command.timestamp.push_back("");
					
					// Check if it's a control flow type of command
					for (int j=0; j<number_of_ctrlflow; j++)
						if (command_id == control_flow_commands[j])
							current_script_command.ctrl_flow[current_script_command.ctrl_flow.size()-1] = true;
					
					// If command is an URL then add it to the url database
					if (IsURL(word)) {
						url_line         = true;
						last_url_list_id = current_script_command_urls.link.size();
						current_script_command_urls.arg_start.push_back(current_script_command_urls.arguments.size());
						current_script_command_urls.arg_num.push_back(0);
						current_script_command_urls.line_num.push_back(word_line_num);
						current_script_command_urls.link.push_back(word);
						current_script_command.url_num[current_script_command.url_num.size()-1]++;
					}
					
					if (command_id==COMMAND_BEGIN_MOD || command_id==COMMAND_BEGIN_VERSION)
						word_line_num_local = 0;
				} else {
					size_t end = script_file_content.find("\n", i);
					i          = (end==string::npos ? script_file_content.length() : end) - 1;
				}
			} else {
				// Check if URL starts here
				if (!url_line  &&  command_id!=COMMAND_ASK_DOWNLOAD)
					url_line = IsURL(word);
					
				// Check if it's a valid command switch
				bool is_switch     = false;
				size_t colon       = word.find(":");
				string switch_name = word;
				string switch_arg  = "";
				
				if (colon != string::npos) {
					switch_name = word.substr(0,colon+1);
					switch_arg  = word.substr(colon+1);
				}
				
				for (int switch_index=1, switch_enum=1 ; switch_index<number_of_switches && !is_switch; switch_enum*=2, switch_index++)				
					if (Equals(switch_name, command_switches_names[switch_index])) {
						is_switch = true;
						int last  = current_script_command.switches.size() - 1;
						current_script_command.switches[last] |= switch_enum;
						
						if (switch_enum == SWITCH_PASSWORD)
							current_script_command.password[last] = switch_arg;
							
						if (switch_enum == SWITCH_TIMESTAMP)
							current_script_command.timestamp[last] = switch_arg;
					}

				// Add word to the URL database or the arguments database
				if (!is_switch) {
					if (url_line) {
						if (last_url_list_id == -1) {
							last_url_list_id = word_line_num;
							current_script_command_urls.arg_start.push_back(current_script_command_urls.arguments.size());
							current_script_command_urls.arg_num.push_back(0);
							current_script_command_urls.line_num.push_back(word_line_num);
							current_script_command_urls.link.push_back(word);
							current_script_command.url_num[current_script_command.url_num.size()-1]++;
						} else {							
							current_script_command_urls.arguments.push_back(word);
							current_script_command_urls.arg_num[current_script_command_urls.arg_num.size()-1]++;
						}
					} else {
						current_script_command.arguments.push_back(word);
						current_script_command.arg_num[current_script_command.arg_num.size()-1]++;
					}
				}
			}
			
			// If ending of an url block
			if (script_file_content[i] == '}' && url_block) {
				url_block = false;
				url_line  = false;
			}

			word_begin = -1;
			word_count++;
		}
		
		// When new line
		if (!in_quote && (script_file_content[i]=='\n' || script_file_content[i]=='\0')) {
			int last = current_script_command.id.size() - 1;
			
			// Maintain minimal number of arguments
			while (!url_block && command_id!=-1 && current_script_command.arg_num[last] < command_minimal_arg[current_script_command.id[last]]) {
				current_script_command.arguments.push_back("");
				current_script_command.arg_num[last]++;
			}
			
			word_count       = 1;
			url_line         = false;
			last_url_list_id = -1;
			word_line_num++;
			word_line_num_local++;
		}
	}


	







	// Pretend to Install
	// Figure out how many steps this installation script has so we can later display progress for the user
	for (int i=0;  i<current_script_command.id.size(); i++) {
		if (
			// if modfolder wasn't formally started OR skipping this mod
			((global.current_mod.empty() || global.skip_modfolder)  &&  current_script_command.id[i]!=COMMAND_BEGIN_MOD  &&  current_script_command.id[i]!=COMMAND_INSTALL_VERSION)
			||
			// if version wasn't formally started
			(!global.current_mod.empty()  &&  global.current_mod_version.empty()  &&  current_script_command.id[i]!=COMMAND_BEGIN_VERSION  &&  current_script_command.id[i]!=COMMAND_INSTALL_VERSION)
			||
			// if inside condition block
			(global.condition_index >= 0  &&  !global.condition[global.condition_index]  &&  !current_script_command.ctrl_flow[i])
		)
			continue;

		// Execute only control flow instructions
		switch(current_script_command.id[i]) {
			case COMMAND_INSTALL_VERSION : global.script_version=atof(current_script_command.arguments[current_script_command.arg_start[i]].c_str()); break;
			case COMMAND_BEGIN_MOD       : global.current_mod="?pretendtoinstall"; break;
			case COMMAND_BEGIN_VERSION   : global.current_mod_version="-1"; break;
			case COMMAND_IF_VERSION      : Condition_If_version(current_script_command.arguments, current_script_command.arg_start[i], current_script_command.arg_num[i]); break;
			case COMMAND_ELSE            : Condition_Else(); break;
			case COMMAND_ENDIF           : Condition_Endif(); break;
			case COMMAND_EXIT            : i=current_script_command.id.size(); break;
			default                      : global.installation_steps_max++;
		}
	}
	
	// Reset variables
	global.current_mod         = "";
	global.current_mod_version = "";
	global.condition_index     = -1;
	global.condition.clear();
	global.current_mod_alias.clear();
	
	if (global.test_mode) {
		global.current_mod              = global.arguments_table["testmod"];
		global.current_mod_version      = "1";
		global.current_mod_version_date = time(0);
		
		if (global.arguments_table["testdir"].empty())
			global.current_mod_new_name = global.arguments_table["testmod"];
		else
			global.current_mod_new_name = global.arguments_table["testdir"];
	} else
		// If wrong version
		if (global.script_version==0  ||  global.installer_version < global.script_version) {
			global.logfile << "Version mismatch. Script version: " << global.script_version << "  Program version: " << global.installer_version << "\n\n--------------\n\n";
			WriteProgressFile(INSTALL_ERROR, (global.lang[STR_ERROR_WRONG_VERSION] + "\\n" + Int2Str(global.script_version) + " vs " + Float2Str(global.installer_version)));
			global.logfile.close();
			return ERROR_WRONG_SCRIPT;
		}
	
	
	
	

	// Install for Real
	for (int i=0;  i<current_script_command.id.size(); i++) {
		if (Abort())
			return ERROR_USER_ABORTED;

		// Update global variables
		for(int j=0; j<number_of_commands; j++)
			if (current_script_command.id[i] == match_command_name_to_id[j])
				global.command_line = command_names[j];

		global.command_line_num      = current_script_command.line_num[i];
		global.download_iterator     = current_script_command.url_start[i];
		global.last_download_attempt = true;

		if (
			// if modfolder wasn't formally started OR skipping this mod
			((global.current_mod.empty() || global.skip_modfolder)  &&  current_script_command.id[i]!=COMMAND_BEGIN_MOD)
			||
			// if version wasn't formally started
			(!global.current_mod.empty()  &&  global.current_mod_version.empty()  &&  current_script_command.id[i]!=COMMAND_BEGIN_VERSION)
			||
			// if inside condition block
			(global.condition_index >= 0  &&  !global.condition[global.condition_index]  &&  !current_script_command.ctrl_flow[i])
		)
			continue;

		if (!current_script_command.ctrl_flow[i])
			global.installation_steps_current++;
		
		int command_result   = ERROR_NONE;
		int failed_downloads = 0;

		// Check if there's an URL list for this command
		if (current_script_command.url_num[i] > 0) {
			Download_Phase:
			global.download_phase = true;
			int last_url          = current_script_command.url_start[i] + current_script_command.url_num[i] - 1;
			
			// For each url
			for (;  global.download_iterator<=last_url;  global.download_iterator++) {
				int j                        = global.download_iterator;
				int download_flags           = current_script_command.id[i] == COMMAND_DOWNLOAD ? FLAG_CLEAN_DL_LATER : FLAG_CLEAN_DL_NOW;
				global.last_download_attempt = j == last_url;
				global.command_line_num      = current_script_command_urls.line_num[j];
				
				// Check how many url arguments
				if (current_script_command_urls.arg_num[j] == 0)
					command_result = Download(current_script_command_urls.link[j], download_flags);
				else 
				if (current_script_command_urls.arg_num[j] == 1)
					command_result = Download(current_script_command_urls.link[j] + " \"--output-document=" + current_script_command_urls.arguments[current_script_command_urls.arg_start[j]] + "\"", download_flags);
				else {
					string original_url     = current_script_command_urls.link[j];
					string cookie_file_name = "fwatch\\tmp\\__cookies.txt";
					string token_file_name  = "fwatch\\tmp\\__downloadtoken";
					string wget_arguments   = "";
					string new_url          = original_url;
					string POST             = "";
					int result              = 0;
					int last_url_arg        = current_script_command_urls.arg_start[j] + current_script_command_urls.arg_num[j] - 1;
					bool found_phrase       = false;
				
					DeleteFile(cookie_file_name.c_str());
					DeleteFile(token_file_name.c_str());
				
					for (int k=current_script_command_urls.arg_start[j]; k<last_url_arg; k++) {
						wget_arguments = "";
						
						if (!POST.empty()) {
							wget_arguments += "--post-data=" + POST + " ";
							POST            = "";
						}
					
						wget_arguments += (k==current_script_command_urls.arg_start[j] ? "--keep-session-cookies --save-cookies " : "--load-cookies ") + cookie_file_name + " --output-document=" + token_file_name + " " + new_url;
						command_result  = Download(wget_arguments, FLAG_OVERWRITE, new_url);
				
						if (command_result != ERROR_NONE)
							goto Finished_downloading;
				
						// Parse downloaded file and find link
						string token_file_buffer = GetFileContents(token_file_name);
					    bool is_href             = Equals(current_script_command_urls.arguments[k].substr(0,6),"href=\"") || Equals(current_script_command_urls.arguments[k].substr(0,6),"href=''");
						size_t find              = token_file_buffer.find(current_script_command_urls.arguments[k]);
			
						if (find != string::npos) {
							size_t left_quote  = string::npos;
							size_t right_quote = string::npos;
							
							if (is_href)
								left_quote = find+5;
							else
								for(int j=find; j>=0 && left_quote==string::npos; j--)
									if (token_file_buffer[j]=='\"' || token_file_buffer[j]=='\'')
										left_quote = j;
									
							for(int k=find+(is_href ? 6 : 0); k<token_file_buffer.length() && right_quote==string::npos; k++)
								if (token_file_buffer[k]=='\"' || token_file_buffer[k]=='\'')
									right_quote = k;
							
							if (left_quote!=string::npos && right_quote!=string::npos) {
								left_quote++;
								found_phrase     = true;
								string found_url = ReplaceAll(token_file_buffer.substr(left_quote, right_quote - left_quote), "&amp;", "&");
								
								// if relative address
								if (found_url[0] == '/') {
									int offset         = 0;
									size_t doubleslash = original_url.find("//");
									
									if (doubleslash != string::npos)
										offset = doubleslash + 2;
									
									size_t slash = original_url.find_first_of("/", offset);
									
									if (slash != string::npos)
										original_url = original_url.substr(0, slash);
									
									found_url = original_url + found_url;
								} else
									if (!IsURL(found_url)) {
										size_t last_slash = new_url.find_last_of("/");
										
										if (last_slash != string::npos)
											new_url = new_url.substr(0, last_slash+1);
										
										found_url = new_url + found_url;
									}
									
								// Check if it's a form
								if (left_quote>8  &&  token_file_buffer.substr(left_quote-8, 7)=="action=") {
									size_t offset = 0;
									string form   = GetTextBetween(token_file_buffer, "</form>", "<form", left_quote, true);
									string input  = GetTextBetween(form, "<input", ">", offset);
									
									while (!input.empty()) {
										vector<string> attributes;
										Tokenize(input," ", attributes);
										string name  = "";
										string value = "";
										
										for (int j=0; j<attributes.size(); j++) {
											if (attributes[j].substr(0,5) == "name=")
												name = ReplaceAll(attributes[j].substr(5), "\"", "");
												
											if (attributes[j].substr(0,6) == "value=")
												value = ReplaceAll(attributes[j].substr(6), "\"", "");
										}
										
										if (!name.empty()) {
											size_t replacement = token_file_buffer.find("input[name="+name);
											
											if (replacement != string::npos) {
												size_t new_value = token_file_buffer.find("'", replacement+13+name.length());
												
												if (new_value != string::npos) {
													new_value++;
													size_t end_value = token_file_buffer.find("'", new_value);
													
													if (end_value != string::npos)
														value = token_file_buffer.substr(new_value, end_value-new_value);
												}
											}
											
											POST += (POST.empty() ? "" : "&") + url_encode(name) + "=" + url_encode(value);
										}
										
										input = GetTextBetween(form, "<input", ">", offset);
									}
								}
								
								new_url = found_url;
							}
						}

						if (!found_phrase) {
							command_result = ErrorMessage(STR_DOWNLOAD_FIND_ERROR, "%STR% " + current_script_command_urls.arguments[k]);
							goto Finished_downloading;
						}
							
						token_file_name += Int2Str(k - current_script_command_urls.arg_start[j]);
					}

					wget_arguments = "--load-cookies " + cookie_file_name;
					
					if (!POST.empty())
						wget_arguments += " --post-data=\"" + POST + "\" ";
					
					wget_arguments +=  " \"--output-document=" + current_script_command_urls.arguments[last_url_arg] + "\" " + new_url;
					command_result  = Download(wget_arguments, download_flags, new_url);
				
					if (!global.test_mode) {
						DeleteFile(cookie_file_name.c_str());
						DeleteFile(token_file_name.c_str());
					}
				}
		
				Finished_downloading:
				if (command_result == ERROR_NONE)
					break;
				else
					failed_downloads++;
			}
		}
		
		
		
		// If download was successful then execute command
		if (current_script_command.url_num[i]==0 || (current_script_command.url_num[i]>0 && failed_downloads<current_script_command.url_num[i])) {
			int first               = current_script_command.arg_num[i]>0 ? current_script_command.arg_start[i] : -1;
			global.download_phase   = false;
			
			switch(current_script_command.id[i]) {
				case COMMAND_BEGIN_MOD : {
					if (current_script_command.arg_num[i] < 4) {
						command_result = ErrorMessage(STR_ERROR_INVALID_SCRIPT, "%STR%", ERROR_WRONG_SCRIPT);
						break;
					}
					
					if (!global.current_mod.empty())
						EndMod();
						
					WriteProgressFile(INSTALL_PROGRESS, global.lang[STR_ACTION_PREPARING]);
				
					global.current_mod          = current_script_command.arguments[first];
					global.current_mod_id       = current_script_command.arguments[first + 1];
					global.current_mod_keepname = current_script_command.arguments[first + 2];
					global.command_line_num     = 0;
					global.current_mod_version  = "";
					
					// Make a list of mod aliases for the entire installation
					vector<string> aliases;
					Tokenize(current_script_command.arguments[first + 3], " ", aliases);
					
					for (int j=0; j<aliases.size(); j++)
						global.current_mod_alias.push_back(aliases[j]);
						
					global.saved_alias_array_size = global.current_mod_alias.size();
					
					
					// Find to which folder we should install the mod
					for (int j=0;  j<global.mod_id.size(); j++)
						if (Equals(global.current_mod_id,global.mod_id[j]))
							global.current_mod_new_name = global.mod_name[j];
				
					if (global.current_mod_new_name.empty()) {
						command_result = ErrorMessage(STR_ERROR_INVALID_ARG, "%STR%", ERROR_WRONG_SCRIPT);
						break;
					}
				
					
					bool activate_rename = false;
					
					// Check if modfolder already exists
					DWORD dir = GetFileAttributesA(global.current_mod_new_name.c_str());
				
					if (dir != INVALID_FILE_ATTRIBUTES) {
						activate_rename = true;
						
						if (dir & FILE_ATTRIBUTE_DIRECTORY) {						
							string mod_id_filename = global.current_mod_new_name + "\\__gs_id";
							string mod_id_contents = GetFileContents(mod_id_filename);
							
							if (!mod_id_contents.empty()) {
								vector<string> mod_id_items;
								Tokenize(mod_id_contents, ";", mod_id_items);
								
								if (mod_id_items.size() > 0)
									activate_rename = mod_id_items[0] != global.current_mod_id;
							}
						}
					}
						
					// Rename current modfolder to make space for a new one
					if (activate_rename) {
						string rename_src = global.current_mod_new_name;
						string rename_dst = "";
						int tries         = 1;
						int last_error    = 0;
						
						do {
							rename_dst = global.current_mod_new_name + "_old" + (tries>1 ? Int2Str(tries) : "");
							
							if (MoveFileEx(rename_src.c_str(), rename_dst.c_str(), 0))
								last_error = 0;
							else {
								tries++;
								last_error = GetLastError();
								
								if (last_error != ERROR_ALREADY_EXISTS) {
									command_result = ErrorMessage(STR_MOVE_RENAME_ERROR, "%STR% " + rename_src + " " + global.lang[STR_MOVE_RENAME_TO_ERROR] + " " + rename_dst + " - " + Int2Str(last_error) + " " + FormatError(last_error));
									goto End_command_execution;
								}
							}
						} while (last_error == ERROR_ALREADY_EXISTS);
						
						global.logfile << "Renaming existing " << rename_src << " to " << rename_dst << endl;
					}
				
					break;
				}

				case COMMAND_BEGIN_VERSION : {
					if (current_script_command.arg_num[i] >= 2) {
						if (!global.current_mod_version.empty())
							EndModVersion();
						
						global.current_mod_version      = current_script_command.arguments[first];
						global.current_mod_version_date = atoi(current_script_command.arguments[first+1].c_str());
						global.command_line_num         = 0;
					} else
						command_result = ErrorMessage(STR_ERROR_INVALID_SCRIPT, "%STR%", ERROR_WRONG_SCRIPT);

					break;
				}
				
				case COMMAND_ALIAS : {
					if (current_script_command.arg_num[i] == 0)
						global.current_mod_alias.clear();
					else 
						for(int j=current_script_command.arg_start[i]; j<current_script_command.arg_start[i]+current_script_command.arg_num[i]; j++)
							global.current_mod_alias.push_back(current_script_command.arguments[j]);
					break;
				}

				case COMMAND_IF_VERSION : command_result=Condition_If_version(current_script_command.arguments, current_script_command.arg_start[i], current_script_command.arg_num[i]); break;
				case COMMAND_ELSE       : command_result=Condition_Else(); break;
				case COMMAND_ENDIF      : command_result=Condition_Endif(); break;
				case COMMAND_EXIT       : i=current_script_command.id.size(); break;

				case COMMAND_AUTO_INSTALL :  {
					global.logfile << "Auto installation" << endl; 
					string file = global.downloaded_filename;
					
					if (current_script_command.arg_num[i] > 0)
						file = current_script_command.arguments[first];
					
					string file_with_path = "fwatch\\tmp\\" + file;
					DWORD attributes      = GetFileAttributes(file_with_path.c_str());
					
					if (attributes != INVALID_FILE_ATTRIBUTES) {
						command_result = Auto_Install(file, attributes, FLAG_RUN_EXE, current_script_command.password[i]);
						
						// If not an archive but there are still backup links then go back to download
						if (!global.last_download_attempt && command_result==ERROR_WRONG_ARCHIVE) {
							file = "fwatch\\tmp\\" + file;
							int result = DeleteFile(file.c_str());
							file       = "<dl>";
							global.downloads.pop_back();
							global.download_iterator++;
							goto Download_Phase;
						}
					} else {
						int error_code = GetLastError();
						command_result = ErrorMessage(STR_AUTO_READ_ATTRI, "%STR% " + file + " - " + Int2Str(error_code) + " " + FormatError(error_code));
					}
					
					break;
				}

				case COMMAND_UNPACK : {
					string *file_name = &current_script_command.arguments[first];

					if (Equals(*file_name,"<download>")  ||  Equals(*file_name,"<dl>")  ||  (*file_name).empty())
						*file_name = global.downloaded_filename;

					if (!(*file_name).empty()) {
						command_result = Unpack(*file_name, current_script_command.password[i]);
						
						// If not an archive but there are still backup links then go back to download
						if (!global.last_download_attempt && command_result==ERROR_WRONG_ARCHIVE) {
							*file_name = "fwatch\\tmp\\" + *file_name;
							int result = DeleteFile((*file_name).c_str());
							*file_name = "<dl>";
							global.downloads.pop_back();
							global.download_iterator++;
							goto Download_Phase;
						}
					} else
						command_result = ErrorMessage(STR_ERROR_NO_FILE);
						
					break;
				}
				
				case COMMAND_MOVE :  
				case COMMAND_COPY : {
					string *source      = &current_script_command.arguments[first];
					string *destination = &current_script_command.arguments[first + 1];
					string *new_name    = &current_script_command.arguments[first + 2];

					if ((*source).empty()) {
						command_result = ErrorMessage(STR_ERROR_NO_FILE);
						break;
					}

					bool is_download_dir = true;
					int options          = FLAG_OVERWRITE | (current_script_command.id[i]==COMMAND_MOVE ? FLAG_MOVE_FILES : FLAG_NONE);

					if (current_script_command.switches[i] & SWITCH_NO_OVERWRITE)
						options &= ~FLAG_OVERWRITE;

					if (current_script_command.switches[i] & SWITCH_MATCH_DIR)
						options |= FLAG_MATCH_DIRS;
						
					if (current_script_command.switches[i] & SWITCH_MATCH_DIR_ONLY)
						options |= FLAG_MATCH_DIRS | FLAG_MATCH_DIRS_ONLY;

					for (int j=first; j<=first+2; j++)
						if (!VerifyPath(current_script_command.arguments[j])) {
							command_result = ErrorMessage(STR_ERROR_PATH);
							goto End_command_execution;
						}


					// Format source path
					if (Equals(*source,"<download>")  ||  Equals(*source,"<dl>")) {
						*source = "fwatch\\tmp\\" + global.downloaded_filename;
						
						if (options & FLAG_MOVE_FILES)
							global.downloads.pop_back();
					} else 
						if (Equals((*source).substr(0,5),"<mod>")) {
							*source         = global.current_mod_new_name + (*source).substr(5);
							is_download_dir = false;
						} else
							if (Equals((*source).substr(0,7),"<game>\\")) {
								is_download_dir = false;
								
								if (~options & FLAG_MOVE_FILES)
									*source = (*source).substr(7);
								else {
									command_result = ErrorMessage(STR_MOVE_DST_PATH_ERROR);
									break;
								}
							} else
								*source = "fwatch\\tmp\\_extracted\\" + *source;
				
					// If user selected directory then move it along with its sub-folders
					bool source_is_dir = false;
					
					if ((*source).find("*")==string::npos && (*source).find("?")==string::npos && GetFileAttributes((*source).c_str()) & FILE_ATTRIBUTE_DIRECTORY) {
						options      |= FLAG_MATCH_DIRS;
						source_is_dir = true;
					}
				
				
					// Format destination path
					bool destination_passed = !(*destination).empty();
					
					if (*destination == ".")
						*destination = "";
					
					*destination = global.current_mod_new_name + "\\" + *destination;
					
					if ((*destination).substr((*destination).length()-1) != "\\")
						*destination += "\\";
				
					// If user wants to move modfolder then change destination to the game directory
					if (is_download_dir  &&  IsModName(PathLastItem(*source))  &&  !destination_passed) {
						*destination = "";
						*new_name    = Equals(global.current_mod,global.current_mod_new_name) ? "" : global.current_mod_new_name;
						options     |= FLAG_MATCH_DIRS;
					} else {
						// Otherwise create missing directories in the destination path
						
						// if user wants to copy directory and give it a new name then first create a new directory with wanted name in the destination location
						if (~options & FLAG_MOVE_FILES  &&  source_is_dir  &&  !(*new_name).empty())
							command_result = MakeDir(*destination + *new_name);
						else
							command_result = MakeDir(PathNoLastItem(*destination));
							
						if (command_result != 0)
							break;
					}
						
				
					// Format new name 
					// 3rd argument - new name
					if ((*new_name).find("\\")!=string::npos  ||  (*new_name).find("/")!=string::npos) {
						command_result = ErrorMessage(STR_RENAME_DST_PATH_ERROR);
						break;
					}

					command_result = MoveFiles(*source, *destination, *new_name, options);
					break;
				}
				
				case COMMAND_MAKEDIR : {
					string *path = &current_script_command.arguments[first];
					
					if (VerifyPath(*path))
						command_result = MakeDir(global.current_mod_new_name + "\\" + *path);
					else
						command_result = ErrorMessage(STR_ERROR_PATH);
				
					break;
				}
				
				case COMMAND_DELETE : {
					WriteProgressFile(INSTALL_PROGRESS, global.lang[STR_ACTION_DELETING]+"...");
					
					string *file_name = &current_script_command.arguments[first];
					int options       = FLAG_MOVE_FILES | FLAG_ALLOW_ERROR;
					
					if (current_script_command.switches[i] & SWITCH_MATCH_DIR)
						options |= FLAG_MATCH_DIRS;
						
					if (!VerifyPath(*file_name)) {
						command_result = ErrorMessage(STR_ERROR_PATH);
						break;
					}
				
					// Format source path
					bool trash = false;
				
					if (Equals(*file_name,"<download>")  ||  Equals(*file_name,"<dl>")  ||  (*file_name).empty()) {
						if (global.downloaded_filename.empty()) {
							command_result = ErrorMessage(STR_ERROR_NO_FILE);
							break;
						}
					
						*file_name = "fwatch\\tmp\\" + global.downloaded_filename;
					} else {
						*file_name = global.current_mod_new_name + "\\" + *file_name;
						trash      = true;
					}
				
				
					// Find files and save them to a list
					vector<string> source_list;
					vector<string> destination_list;
					vector<bool>   is_dir_list;
					vector<string> empty_dirs;
					int buffer_size = 1;
					int recursion   = -1;
				
					command_result = CreateFileList(*file_name, PathNoLastItem(*file_name), source_list, destination_list, is_dir_list, options, empty_dirs, buffer_size, recursion);
				
					if (command_result != ERROR_NONE)
						break;
				
				
					// Allocate buffer for the file list
					char *file_list;
					int base_path_len = global.working_directory.length() + 1;
					int buffer_pos    = 0;
					wstring temp;
					
					if (trash) {
						file_list = new char[buffer_size*2];
				
						if (!file_list) {
							command_result = ErrorMessage(STR_ERROR_BUFFER, "%STR% " + Int2Str(buffer_size));
							break;
						}
					}
				
				  
					// For each file in the list
					for (int j=0; j<destination_list.size(); j++) {
						if (Abort()) {
							command_result = ERROR_USER_ABORTED;
							goto End_command_execution;
						}
						
						if (trash) {
							temp            = string2wide(destination_list[j]);
							int name_length = (temp.length()+1) * 2;
							global.logfile << "Trashing " << destination_list[j].substr(base_path_len) << endl;
							memcpy(file_list+buffer_pos, temp.c_str(), name_length);
							buffer_pos     += name_length;
						} else {
							global.logfile << "Deleting  " << destination_list[j].substr(base_path_len);
							int error_code = 0;
							
							if (is_dir_list[j])
								error_code = DeleteDirectory(destination_list[j]);
							else
								if (!DeleteFileW(string2wide(destination_list[j]).c_str()))
									error_code = GetLastError();

							if (error_code != 0) {
								command_result = ErrorMessage(STR_DELETE_PERMANENT_ERROR, "%STR% " + source_list[i] + "  - " + Int2Str(error_code) + " " + FormatError(error_code));
								break;
							}
								
							global.logfile << endl;
						}
					}
				
					if (trash) {
						memcpy(file_list+buffer_pos, "\0\0", 2);
				
						// Trash file
						SHFILEOPSTRUCTW shfos;
						shfos.hwnd     = NULL;
						shfos.wFunc    = FO_DELETE;
						shfos.pFrom    = (LPCWSTR)file_list;
						shfos.pTo      = NULL;
						shfos.fFlags   = FOF_SILENT | FOF_NOCONFIRMATION | FOF_ALLOWUNDO;
						int result     = SHFileOperationW(&shfos);
								
					    if (result != 0)
							command_result = ErrorMessage(STR_DELETE_BIN_ERROR, "%STR% - " + Int2Str(result) + " " + FormatError(result));
				
						delete[] file_list;
					}

					break;
				}
				
				case COMMAND_RENAME : {
					WriteProgressFile(INSTALL_PROGRESS, global.lang[STR_ACTION_RENAMING]+"...");
				
					string *source      = &current_script_command.arguments[first];
					string *destination = &current_script_command.arguments[first + 1];
					int options         = FLAG_MOVE_FILES;
					
					if (current_script_command.switches[i] & SWITCH_MATCH_DIR)
						options |= FLAG_MATCH_DIRS;
						
					for (int j=first; j<=first+1; j++)
						if (!VerifyPath(current_script_command.arguments[j])) {
							command_result = ErrorMessage(STR_ERROR_PATH);
							goto End_command_execution;
						}

				
					// Format source path
					if ((*source).empty()) {
						command_result = ErrorMessage(STR_RENAME_NO_NAME_ERROR, "%STR%");
						break;
					}
					
					if (Equals(*source,"<download>")  ||  Equals(*source,"<dl>"))	{
						if (global.downloaded_filename.empty()) {
							command_result = ErrorMessage(STR_ERROR_NO_FILE);
							break;
						}
						
						*source = "fwatch\\tmp\\_extracted\\" + global.downloaded_filename;
					} else
						*source = global.current_mod_new_name + "\\" + *source;
				
					string relative_path = PathNoLastItem(*source);
					bool source_wildcard = false;
				
					if (relative_path.find("*")!=string::npos  ||  relative_path.find("?")!=string::npos) {
						command_result = ErrorMessage(STR_RENAME_WILDCARD_ERROR, "%STR%");
						break;
					}
					
				
					// Format new name
					if ((*destination).empty()) {
						command_result = ErrorMessage(STR_RENAME_NO_NAME_ERROR);
						break;
					}
					
					if ((*destination).find("\\")!=string::npos  ||  (*destination).find("/")!=string::npos) {
						command_result = ErrorMessage(STR_RENAME_DST_PATH_ERROR);
						break;
					}
						
							
					// Find files and save them to a list
					vector<string> source_list;
					vector<string> destination_list;
					vector<bool>   is_dir_list;
					vector<string> empty_dirs;
					int buffer_size = 0;
					int recursion   = -1;

					command_result = CreateFileList(*source, relative_path+*destination, source_list, destination_list, is_dir_list, options, empty_dirs, buffer_size, recursion);

					if (command_result != 0)
						break;

					wstring source_wide;
					wstring destination_wide;


					// For each file on the list
					for (int j=0;  j<source_list.size(); j++) {
						if (Abort()) {
							command_result = ERROR_USER_ABORTED;
							break;
						}

						// Format path for logging
						global.logfile << "Renaming  " << ReplaceAll(source_list[j], "fwatch\\tmp\\_extracted\\", "") << "  to  " << PathLastItem(destination_list[j]);

						// Rename
						source_wide      = string2wide(source_list[j]);
						destination_wide = string2wide(destination_list[j]);
						int result       = MoveFileExW(source_wide.c_str(), destination_wide.c_str(), 0);

					    if (!result) {
							int error_code = GetLastError();
							global.logfile << endl;
							command_result = ErrorMessage(STR_MOVE_RENAME_ERROR, "%STR% " + source_list[j] + " " + global.lang[STR_MOVE_RENAME_TO_ERROR] + " " + destination_list[j] + " - " + Int2Str(error_code) + " " + FormatError(error_code));
							break;
					    }
					    
					    global.logfile << endl;
					}

					break;
				}
				
				case COMMAND_ASK_RUN : {
					string *file_name = &current_script_command.arguments[first];
					
					if (Equals(*file_name,"<download>")  ||  Equals(*file_name,"<dl>")  ||  (*file_name).empty())
						*file_name = global.downloaded_filename;
				
					if ((*file_name).empty()) {
						command_result = ErrorMessage(STR_ERROR_NO_FILE);
						break;
					}
				
					if (!VerifyPath(*file_name)) {
						command_result = ErrorMessage(STR_ERROR_PATH);
						break;
					}
				
					string path_to_dir = global.working_directory;
					
					if ((*file_name).substr(0,6) == "<mod>\\") {
						*file_name   = (*file_name).substr(6);
						path_to_dir += "\\" + global.current_mod_new_name + "\\";
					} else
						path_to_dir += "\\fwatch\\tmp\\";
				
					command_result = RequestExecution(path_to_dir, *file_name);
					break;
				}

				case COMMAND_ASK_DOWNLOAD : {
					if (current_script_command.arg_num[i] < 2) {
						command_result = ErrorMessage(STR_ERROR_ARG_COUNT);
						break;
					}
					
					string *file_name   = &current_script_command.arguments[first];
					string *url         = &current_script_command.arguments[first + 1];
					string download_dir = GetFileContents("fwatch\\tmp\\schedule\\DownloadDir.txt");
					bool move           = false;
					fstream config;
					
					// Check if file already exists
					string path1 = download_dir + "\\" + *file_name;
					string path2 = "fwatch\\tmp\\" + *file_name;
					
					if (GetFileAttributes(path1.c_str()) != INVALID_FILE_ATTRIBUTES) {
						global.logfile << "Found " << path1 << endl;
						move = true;
					} else
						if (GetFileAttributes(path2.c_str()) != INVALID_FILE_ATTRIBUTES) {
							global.downloaded_filename = *file_name;
							global.logfile << "Found " << path2 << endl;
							global.downloads.push_back(global.downloaded_filename);
						} else {
							string message = global.lang[STR_ASK_DLOAD] + ":\\n" + *file_name + "\\n\\n" + global.lang[STR_ALTTAB];
							WriteProgressFile(INSTALL_WAITINGFORUSER, message);
							
							ShellExecute(0, 0, (*url).c_str(), 0, 0 , SW_SHOW);
							global.logfile << "Opened " << *url << endl;
							
							message      = "You must manually download\n" + *file_name + "\n\nPress OK once download has finished\nPress CANCEL to skip installing this modfolder";
							int msgboxID = MessageBox(NULL, message.c_str(), "Addon Installer", MB_ICONQUESTION | MB_OKCANCEL | MB_DEFBUTTON1);
							
							if (msgboxID == IDCANCEL)
								global.skip_modfolder = true;
							else {
								if (download_dir.empty()  ||  GetFileAttributes(path1.c_str()) == INVALID_FILE_ATTRIBUTES) {
									WriteProgressFile(INSTALL_WAITINGFORUSER, global.lang[STR_ASK_DLOAD_SELECT] + "\\n\\n" + global.lang[STR_ALTTAB]);
									
									download_dir = BrowseFolder("");
									printf("%s", download_dir.c_str());
									
									fstream config("fwatch\\tmp\\schedule\\DownloadDir.txt", ios::out | ios::trunc);
					
									if (config.is_open()) {
										config << download_dir;
										config.close();
									}
								}
				
								move = true;
							}
						}
						
					if (move) {
						WriteProgressFile(INSTALL_PROGRESS, global.lang[STR_ACTION_COPYINGDOWNLOAD]);
						
						string source      = download_dir + "\\" + *file_name;
						string destination = global.working_directory + "\\fwatch\\tmp\\" + *file_name;
						int result         = MoveFileEx(source.c_str(), destination.c_str(), MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING);
						
						global.logfile << "Moving " << source << "  to  " << destination << endl;
						
						if (!result) {
							int error_code = GetLastError();
							command_result = ErrorMessage(STR_MOVE_ERROR, "%STR% " + source + " " + global.lang[STR_MOVE_TO_ERROR] + " " + destination + " - " + Int2Str(error_code) + " " + FormatError(error_code));
						} else {
							global.downloaded_filename = *file_name;
							global.downloads.push_back(global.downloaded_filename);
						}
					}
					
					break;
				}
										
				case COMMAND_MAKEPBO : {
					WriteProgressFile(INSTALL_PROGRESS, global.lang[STR_ACTION_PACKINGPBO]+"...");
					
					string *file_name = &current_script_command.arguments[first];
										
					if (!VerifyPath(*file_name)) {
						command_result = ErrorMessage(STR_ERROR_PATH);
						break;
					}			
				
					// Format source path
					if ((*file_name).empty()) {
						if (global.last_pbo_file.empty()) {
							command_result = ErrorMessage(STR_ERROR_NO_FILE);
							break;
						}
					
						*file_name = global.last_pbo_file;
					} else
						*file_name = global.current_mod_new_name + "\\" + *file_name;
				
				
					// Create log file
					SECURITY_ATTRIBUTES sa;
				    sa.nLength              = sizeof(sa);
				    sa.lpSecurityDescriptor = NULL;
				    sa.bInheritHandle       = TRUE;       
				
				    HANDLE logFile = CreateFile(TEXT("fwatch\\tmp\\schedule\\PBOLog.txt"),
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
					si.cb 		   = sizeof(si);
					si.dwFlags 	   = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
					si.wShowWindow = SW_SHOW;
					si.hStdInput   = NULL;
					si.hStdOutput  = logFile;
					si.hStdError   = logFile;
					
					string exename         = "MakePbo.exe";
					string executable      = "fwatch\\data\\" + exename;
					string arguments       = " -NRK \"" + *file_name + "\"";
					string pbo_name        = *file_name + ".pbo";
					string pbo_name_backup = "";
					
					if (GetFileAttributes(pbo_name.c_str()) != INVALID_FILE_ATTRIBUTES) {
						int tries         = 2;
						int last_error    = 0;
						
						do {
							pbo_name_backup = pbo_name + Int2Str(tries);
							
							if (MoveFileEx(pbo_name.c_str(), pbo_name_backup.c_str(), 0))
								last_error = 0;
							else {
								tries++;
								last_error = GetLastError();
								
								if (last_error != ERROR_ALREADY_EXISTS) {
									command_result = ErrorMessage(STR_MOVE_RENAME_ERROR, "%STR% " + pbo_name + " " + global.lang[STR_MOVE_RENAME_TO_ERROR] + " " + pbo_name_backup + " - " + Int2Str(last_error) + " " + FormatError(last_error));
									goto End_command_execution;
								}
							}
						} while (last_error == ERROR_ALREADY_EXISTS);
					}
				
					if (!CreateProcess(&executable[0], &arguments[0], NULL, NULL, true, 0, NULL, NULL, &si, &pi)) {
						if (!pbo_name_backup.empty())
							MoveFileEx(pbo_name_backup.c_str(), pbo_name.c_str(), MOVEFILE_REPLACE_EXISTING);
						
						int error_code = GetLastError();
						command_result = ErrorMessage(STR_ERROR_EXE, "%STR% " + exename + " - " + Int2Str(error_code) + " " + FormatError(error_code));
					} else
						global.logfile << "Creating a PBO file out of " << *file_name << endl;
						
					Sleep(10);
				

					// Wait for the program to finish its job
					DWORD exit_code;
					string message = "";
					
					do {					
						if (Abort()) {
							TerminateProcess(pi.hProcess, 0);
							CloseHandle(pi.hProcess);
							CloseHandle(pi.hThread);
							CloseHandle(logFile);
							command_result = ERROR_USER_ABORTED;
							goto End_command_execution;
						}
						
						ParsePBOLog(message, exename, *file_name);
						GetExitCodeProcess(pi.hProcess, &exit_code);
						Sleep(100);
					} while(exit_code == STILL_ACTIVE);
					
					ParsePBOLog(message, exename, *file_name);
				
					CloseHandle(pi.hProcess);
					CloseHandle(pi.hThread);
					CloseHandle(logFile);
					Sleep(1000);
					
					
					// Need to fix the pbo timestamps after makepbo
					if (exit_code == 0) {
						vector<string> sourcedir_name;
						vector<unsigned int> sourcedir_time;
						command_result = CreateTimestampList(*file_name, (*file_name).length()+1, sourcedir_name, sourcedir_time);
				
						if (command_result == ERROR_NONE) {
							FILE *f = fopen(pbo_name.c_str(), "rb");
							if (f) {
								fseek(f, 0, SEEK_END);
								int file_size = ftell(f);
								fseek(f, 0, SEEK_SET);
								
								char *buffer = (char*) malloc(file_size+1);
								
								if (buffer != NULL) {
									memset(buffer, 0, file_size+1);
									fread(buffer, 1, file_size, f);
									
									const int name_max  = 512;
									char name[name_max] = "";
									int name_len        = 0;
									int file_count      = 0;
									int file_pos        = 0;
									 
									while (file_pos < file_size) {
										memset(name, 0, name_max);
										name_len = 0;
								
										for (int i=0; i<name_max-1; i++) {
											char c = buffer[file_pos++];
								
											if (c != '\0')
												name[name_len++] = c;
											else
												break;
										}
								
										unsigned long MimeType  = *((unsigned long*)&buffer[file_pos]);
										unsigned long TimeStamp = *((unsigned long*)&buffer[file_pos+12]);
										unsigned long Datasize  = *((unsigned long*)&buffer[file_pos+16]);
								
										file_pos += 20;
								
										if (name_len == 0) {
											if (file_count==0 && MimeType==0x56657273 && TimeStamp==0 && Datasize==0) {
												int value_len = 0;
												bool is_name  = true;
												
												while (file_pos < file_size) {
													if (buffer[file_pos++] != '\0')
														value_len++;
													else {
														if (is_name && value_len==0)
															break;
														else {
															is_name   = !is_name;
															value_len = 0;
														}
													}
												}
											} else
												break;
										} else {
											for (int i=0; i<sourcedir_name.size(); i++)
												if (strcmp(sourcedir_name[i].c_str(), name) == 0) {
													if (sourcedir_time[i] != TimeStamp)
														memcpy(buffer+file_pos-8, &sourcedir_time[i], 4);
													
													break;
												}
										}
											
										file_count++;
									}
									
									freopen(pbo_name.c_str(), "wb", f);
									int bytes_written = 0;
									
									if (f) {
										bytes_written = fwrite(buffer, 1, file_size, f);
										fclose(f);
									}
									
									free(buffer);
										
									if (bytes_written != file_size) {
										if (!pbo_name_backup.empty())
											MoveFileEx(pbo_name_backup.c_str(), pbo_name.c_str(), MOVEFILE_REPLACE_EXISTING);
										
										command_result = ErrorMessage(STR_EDIT_WRITE_ERROR, "%STR% " + Int2Str(bytes_written) + "/" + Int2Str(file_size));
										break;
									}
								}
							} else {
								if (!pbo_name_backup.empty())
									MoveFileEx(pbo_name_backup.c_str(), pbo_name.c_str(), MOVEFILE_REPLACE_EXISTING);
									
								command_result = ErrorMessage(STR_EDIT_READ_ERROR, "%STR% " + Int2Str(errno) + " - " + (string)strerror(errno));
								break;
							}
						} else {
							if (!pbo_name_backup.empty())
								MoveFileEx(pbo_name_backup.c_str(), pbo_name.c_str(), MOVEFILE_REPLACE_EXISTING);
							break;
						}
				
						if (current_script_command.switches[i] & SWITCH_TIMESTAMP)
							command_result = ChangeFileDate(pbo_name, current_script_command.timestamp[i]);
						else
							command_result = ChangeFileDate(pbo_name, global.current_mod_version_date);

						if (command_result == ERROR_NONE) {
							if (~current_script_command.switches[i] & SWITCH_KEEP_SOURCE) {
								WriteProgressFile(INSTALL_PROGRESS, global.lang[STR_ACTION_DELETING]+"...");
								global.logfile << "Removing " << *file_name << " directory" << endl;
								global.last_pbo_file = "";
								DeleteDirectory(*file_name);
							}
						
							if (!pbo_name_backup.empty())
								DeleteFile(pbo_name_backup.c_str());
						} else
							if (!pbo_name_backup.empty())
								MoveFileEx(pbo_name_backup.c_str(), pbo_name.c_str(), MOVEFILE_REPLACE_EXISTING);
					} else {
						if (!pbo_name_backup.empty())
							MoveFileEx(pbo_name_backup.c_str(), pbo_name.c_str(), MOVEFILE_REPLACE_EXISTING);
							
						command_result = ErrorMessage(STR_PBO_MAKE_ERROR, "%STR% " + Int2Str(exit_code) + " - " + message);
					}

					break;
				}
				
				case COMMAND_EXTRACTPBO : {
					string *source      = &current_script_command.arguments[first];
					string *destination = &current_script_command.arguments[first + 1];
					
					// Verify source argument
					if ((*source).empty()) {
						command_result = ErrorMessage(STR_ERROR_NO_FILE);
						break;
					}
						
					if (!VerifyPath(*source)) {
						command_result = ErrorMessage(STR_UNPACKPBO_SRC_PATH_ERROR);
						break;
					}
					
					if (GetFileExtension(*source) != "pbo") {
						command_result = ErrorMessage(STR_PBO_NAME_ERROR);
						break;
					}
					
					bool is_game_dir = false;
					
					if (Equals((*source).substr(0,7),"<game>\\")) {
						global.last_pbo_file = "";
						*source              = (*source).substr(7);
						is_game_dir          = true;
					} else {
						global.last_pbo_file = global.current_mod_new_name + "\\" + (*source).substr(0, (*source).length()-4);
						*source              = global.current_mod_new_name + "\\" + *source;
					}
				
				
					// Verify destination argument				
					if (!VerifyPath(*destination)) {
						command_result = ErrorMessage(STR_UNPACKPBO_DST_PATH_ERROR);
						break;
					}
					
					// Process optional 2nd argument: extraction destination
					if (!(*destination).empty()  ||  is_game_dir) {
						if (*destination == ".")
							*destination = "";
				
						command_result = MakeDir(global.current_mod_new_name + "\\" + *destination);
				
						if (command_result != 0)
							break;
				
						// Create path to the extracted directory for use with MakePbo function
						global.last_pbo_file = global.current_mod_new_name + "\\" + *destination + "\\" + PathLastItem((*source).substr(0, (*source).length()-4));
						*destination         = global.working_directory    + "\\" + global.current_mod_new_name + "\\" + *destination;
						
						if ((*destination).substr((*destination).length()-1) != "\\")
							*destination += "\\";
					}
				
					command_result = ExtractPBO(*source, *destination);
					break;
				}
				
				case COMMAND_EDIT : {
					WriteProgressFile(INSTALL_PROGRESS, global.lang[STR_ACTION_EDITING]+"...");
				
					if (current_script_command.arg_num[i] < 3) {
						command_result = ErrorMessage(STR_ERROR_ARG_COUNT);
						break;
					}
					
					string *file_name   = &current_script_command.arguments[first];
					string *wanted_text = &current_script_command.arguments[first + 2];
					int wanted_line     = atoi(current_script_command.arguments[first+1].c_str());
				
					if ((*file_name).empty()) {
						command_result = ErrorMessage(STR_ERROR_NO_FILE);
						break;
					}
					
					if (Equals(*file_name,"<download>")  ||  Equals(*file_name,"<dl>")) {
						if (global.downloaded_filename.empty()) {
							command_result = ErrorMessage(STR_ERROR_NO_FILE);
							break;
						}
				
						*file_name = "fwatch\\tmp\\" + global.downloaded_filename;
					} else 
						*file_name = global.current_mod_new_name + "\\" + *file_name;
				
					if (!VerifyPath(*file_name)) {
						command_result = ErrorMessage(STR_ERROR_PATH);
						break;
					}
				
					vector<string> contents;
				    fstream file;
					int line_number        = 0;
					bool ends_with_newline = true;
				    
				    if (~current_script_command.switches[i] & SWITCH_NEWFILE) {
				    	global.logfile << "Editing line " << wanted_line << " in " << *file_name << endl;
				    	
				    	file.open((*file_name).c_str(), ios::in);
				    	
						if (file.is_open()) {
							string line;
						
							while (getline(file, line)) {
								line_number++;
								
								if (file.eof())
									ends_with_newline = false;
								
								if (line_number == wanted_line) {
									string new_line = current_script_command.switches[i] & SWITCH_APPEND ? line+*wanted_text : *wanted_text;
								
									contents.push_back(new_line);
									
									if (current_script_command.switches[i] & SWITCH_INSERT) {
										contents.push_back(line);
										line_number++;
									}
								} else 
									contents.push_back(line);
							}
							
							if (current_script_command.switches[i] & SWITCH_INSERT  &&  (wanted_line==0 || wanted_line > line_number)) {
								contents.push_back(*wanted_text);
								line_number++;
							}
							
							file.close();
						} else {
							command_result = ErrorMessage(STR_EDIT_READ_ERROR);
							break;
						}
					} else {
						global.logfile << "Creating new file " << *file_name << endl;
						contents.push_back(*wanted_text);
						
						// Trash the file
						int buffer_pos  = 0;
						int buffer_size = (*file_name).length() + 3;
						char *file_list = new char[buffer_size*2];
				
						if (file_list) {
							wstring file_name_wide = string2wide(*file_name);
							int name_length        = (file_name_wide.length()+1) * 2;
							memcpy(file_list, file_name_wide.c_str(), name_length);
							memcpy(file_list+name_length, "\0\0", 2);
							
							SHFILEOPSTRUCTW shfos;
							shfos.hwnd   = NULL;
							shfos.wFunc  = FO_DELETE;
							shfos.pFrom  = (LPCWSTR)file_list;
							shfos.pTo    = NULL;
							shfos.fFlags = FOF_SILENT | FOF_NOCONFIRMATION | FOF_ALLOWUNDO;
							int result   = SHFileOperationW(&shfos);
				
							if (result!=0  &&  result!=1026  &&  result!=2)
								global.logfile << "Trashing FAILED " << result << " " << FormatError(result) << endl;
							
							delete[] file_list;
						}
					}
				    	
				    	
				    // Write file
					file.open((*file_name).c_str(), ios::out | ios::trunc);
					
					if (file.is_open()) {
						for (int j=0; j<contents.size(); j++) {
							file << contents[j];
							
							if (j+1 < line_number  ||  j+1==line_number && ends_with_newline)
								file << endl;
						}
				
						file.close();
						
						if (current_script_command.switches[i] & SWITCH_TIMESTAMP)
							command_result = ChangeFileDate(*file_name, current_script_command.timestamp[i]);
						else
				    		command_result = ChangeFileDate(*file_name, global.current_mod_version_date);
					} else {
						command_result = ErrorMessage(STR_EDIT_WRITE_ERROR);
						break;
					}
				
					break;
				}
				
				case COMMAND_FILEDATE : {
					if (current_script_command.arg_num[i] < 2) {
						command_result = ErrorMessage(STR_ERROR_ARG_COUNT);
						break;
					}
					
					string *file_name = &current_script_command.arguments[first];
					string *date_text = &current_script_command.arguments[first + 1];
					
					if (!VerifyPath(*file_name)) {
						command_result = ErrorMessage(STR_ERROR_PATH);
						break;
					}

					*file_name = global.current_mod_new_name + "\\" + *file_name;
					command_result = ChangeFileDate(*file_name, *date_text);
					break;
				}
			}
		}

		End_command_execution:
		if (command_result == ERROR_USER_ABORTED)
			return command_result;
		else
			if (command_result != ERROR_NONE) {
				global.logfile << "Installation error - aborting\n\n--------------\n\n";
				global.logfile.close();
				return command_result;
			}
	}







   
    // Clean up after the last mod
	if (!global.current_mod.empty())
		EndMod();


	// Finish log file
	if (global.missing_modfolders.empty()) {
		WriteProgressFile(INSTALL_DONE, global.lang[STR_ACTION_DONE]);
		GetLocalTime(&st);
		global.logfile << "All done  " << LeadingZero(st.wHour) << ":" << LeadingZero(st.wMinute) << ":" << LeadingZero(st.wSecond) << endl;
	} else {
		string message = ReplaceAll(global.lang[STR_ACTION_DONEWARNING], "%MOD%", global.missing_modfolders);
		WriteProgressFile(INSTALL_WARNING, message);
		global.logfile << "WARNING: Installation completed but modfolders " << global.missing_modfolders << " are still missing" << endl;
		global.restart_game = false;
	}
	
	
	
	// If user wants to restart the game after installation
	if (global.restart_game) {
		DeleteFile("fwatch\\tmp\\schedule\\install_progress.sqf");
		
		if (global.run_voice_program)
			global.gamerestart_arguments += " -evoice=" + global.arguments_table["evoice"];
		
		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		ZeroMemory(&si, sizeof(si));
		ZeroMemory(&pi, sizeof(pi));
		si.cb          = sizeof(si);
		si.dwFlags     = STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_HIDE;
		string param   = WrapInQuotes(global.working_directory) + " " + global.gamerestart_arguments;
	 
		if (CreateProcess("fwatch\\data\\gameRestart.exe", &param[0], NULL, NULL, TRUE, HIGH_PRIORITY_CLASS, NULL, NULL, &si, &pi))
			global.logfile << "Executing gameRestart.exe  " << global.gamerestart_arguments << endl;
		else
			global.logfile << "Failed to launch gameRestart.exe " << FormatError(GetLastError()) << endl;
	 
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	
	global.logfile << "\n--------------\n\n";
	global.logfile.close();
	
	

	// Close listen thread
	global.end_thread = true;
	Sleep(300);
	
    return ERROR_NONE;
}
