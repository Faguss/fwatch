// AddonInstaller.exe by Faguss (ofp-faguss.com) for Fwatch 1.16


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

using namespace std;

struct GLOBAL_VARIABLES 
{
	bool test_mode;
	bool abort_installer;
	bool skip_modfolder;
	bool restart_game;
	bool end_thread;
	bool first_exe;
	bool unpack_set_error;
	bool auto_install_next_file;
	bool run_voice_program;
	int game_ver_index;
	int command_line_num;
	int mirror;
	int current_mod_version_date;
	int instructions_pos;
	int instructions_max;
	int saved_alias_array_size;
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
	vector<int> game_ver_cond;
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
	true,
	false,
	true,
	-1,
	0,
	0,
	0,
	0,
	0,
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
	NO_END_SLASH,
	MOVE_FILES,
	OVERWRITE,
	MATCH_DIRS = 4,
	ALLOW_ERROR = 8,
	SILENT_MODE = 16,
	CUSTOM_DELIMITERS = 1
};

enum MOVE_ARGUMENTS 
{
	SOURCE,
	DESTINATION,
	NEW_NAME
};

enum ERROR_CODES 
{
	NO_ERRORS,
	USER_ABORTED,
	LOGFILE_ERROR,
	NO_SCRIPT,
	COMMAND_FAILED,
	SCRIPT_ERROR
};

enum MIRROR_STATES 
{
	DISABLED,
	ENABLED,
	SKIP_REMAINING	
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
	STR_MOVE_RENAME_ERROR,
	STR_MOVE_RENAME_TO_ERROR,
	STR_RENAME_DST_PATH_ERROR,
	STR_RENAME_WILDCARD_ERROR,
	STR_RENAME_NO_NAME_ERROR,
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

enum MISSIONSQM 
{
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

enum MISSIONSQM_PLAYERCOUNT 
{
	NO_MISSIONSQM,
	SINGLE_PLAYER,
	MULTI_PLAYER
};











	// Functions
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
	};
	
	return file_extension;
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


void Tokenize(string text, string delimiter, vector<string> &container, bool custom_delimiters=false)
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
		if (!isToken  &&  begin<0) {
			begin = pos;
			
			if (custom_delimiters) {
				if (text.substr(begin,2) == ">>") {
					begin      += 3;
					size_t end  = text.find(text[pos+2], pos+3);
					isToken     = true;
					pos         = end==string::npos ? text.length() : end;
					use_unQuote = false;
				}
			}
		}

		// Mark end of the word
		if (isToken  &&  begin>=0  &&  !inQuote) {
			string part = text.substr(begin, pos-begin);
			
			if (use_unQuote)
				part = UnQuote(part);
			else
				use_unQuote = true;
				
			container.push_back(part);
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


//https://stackoverflow.com/questions/6691555/converting-narrow-string-to-wide-string
wstring string2wide(const string& input)
{
    if (input.empty())
		return wstring();

	size_t output_length = MultiByteToWideChar(CP_UTF8, 0, input.c_str(), (int)input.length(), 0, 0);
	wstring output(output_length, L'\0');
	MultiByteToWideChar(CP_UTF8, 0, input.c_str(), (int)input.length(), &output[0], (int)input.length());
	
	return output;
}

//https://mariusbancila.ro/blog/2008/10/20/writing-utf-8-files-in-c/
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
// -------------------------------------------------------------------------------------------------------










// File operations ---------------------------------------------------------------------------------------

	// Feedback for the game
void WriteProgressFile(int status, string input)
{
	ofstream progressLog;
	progressLog.open("fwatch\\tmp\\schedule\\install_progress.sqf", ios::out | ios::trunc);

	if (!progressLog.is_open())
		return;
		
	if (status==INSTALL_PROGRESS  &&  global.instructions_pos>0  &&  global.instructions_max>0)
		input = global.lang[STR_PROGRESS] + " " + Int2Str(global.instructions_pos) + "/" + Int2Str(global.instructions_max) + "\\n\\n" + input;

	progressLog << "_auto_restart=" << (global.restart_game ? "true" : "false") 
				<< ";_run_voice_program=" << (global.run_voice_program ? "true" : "false")
				<< ";_install_status=" << status << ";\"" << ReplaceAll(input, "\"", "\"\"") << "\"";
	progressLog.close();
};


	// Write mod identification file
int WriteModID(string modfolder, string content, string content2)
{
	if (global.test_mode)
		return NO_ERRORS;
	
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
		return NO_ERRORS;
	} else
		return LOGFILE_ERROR;
}



int ErrorMessage(int string_code, string message="%STR%", int error_code=COMMAND_FAILED) 
{
	if (global.current_mod=="parsetest" && global.current_mod_version=="-1")
		return NO_ERRORS;
	
	int status              = global.mirror==ENABLED ? INSTALL_PROGRESS : INSTALL_ERROR;
	string message_eng      = ReplaceAll(message, "%STR%", global.lang_eng[string_code]);
	string message_local    = ReplaceAll(message, "%STR%", global.lang[string_code]);
	string message_complete = "";
	
	// show which command failed
	if (error_code == COMMAND_FAILED) {
		message_complete = global.lang[STR_ERROR] + "\\n" + global.current_mod;
		
		if (global.current_mod_version != "")
			message_complete += "\\n" + global.lang[STR_ERROR_INVERSION] + " " + global.current_mod_version;
		
		if (global.command_line_num > 0)
			message_complete += "\\n" + global.lang[STR_ERROR_ONLINE] + " " + Int2Str(global.command_line_num) + "\\n" + global.command_line;
			
		message_complete += "\\n" + message_local;

		if (status == INSTALL_ERROR) {
			global.logfile << "ERROR " << global.current_mod;
			
			if (global.current_mod_version != "")
				global.logfile << " " << global.current_mod_version;
			
			if (global.command_line_num > 0)	
				global.logfile << " line " << Int2Str(global.command_line_num) << ": " << global.command_line;
				
			global.logfile << " - " << ReplaceAll(message_eng, "\\n", " ") << endl;
		}
	}
	
	// just display input message
	if (error_code == SCRIPT_ERROR) {
		message_complete = global.lang[STR_ERROR] + "\\n" + message_local;
		global.logfile << "ERROR - " << ReplaceAll(message_eng, "\\n", " ") << endl;
	}
	
	WriteProgressFile(status, message_complete);
	return error_code;
}


	// Separate thread for listening to the user
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
		return USER_ABORTED;
	}

	return NO_ERRORS;
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


string ReadStartupParams(string key_to_find)
{
	FILE *f;
	string output;
	
	if (f = fopen("fwatch\\tmp\\schedule\\params.bin","rb")) {
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
						output = text_buffer + offsets[i] + strlen(key_name) + 1;
						string tofind = "_Input=\"";
						size_t find = output.find_first_of(tofind);

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
			if (FileInformation.cFileName[0] != '.') {
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


int Download(string url, int options=0, string log_file_name="")
{
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
		}
		
		find = url.find(output, find);
	}

	string arguments = options & OVERWRITE ? "" : " --no-clobber";
	arguments += " --no-check-certificate --output-file=fwatch\\tmp\\schedule\\downloadLog.txt --directory-prefix=fwatch\\tmp\\ " + url;
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
		if (~options & SILENT_MODE)
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
			return USER_ABORTED;
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
		ErrorMessage(STR_DOWNLOAD_FAILED, "%STR%\\n" + global.downloaded_filename + "\\n\\n" + message, options & SILENT_MODE ? SCRIPT_ERROR : COMMAND_FAILED);		
		string filename = "fwatch\\tmp\\" + global.downloaded_filename;
		DeleteFile(filename.c_str());
	} else
		if (~options & SILENT_MODE)
			global.downloads.push_back(global.downloaded_filename);
	
	return (exit_code!=0 ? COMMAND_FAILED : NO_ERRORS);
}


int Download_Wrapper(const vector<string> &arg)
{
	if (arg.size() == 0)
		return ErrorMessage(STR_ERROR_ARG_COUNT);
		
	if (arg.size() == 1)
		return Download(arg[0]);
		
	if (arg.size() == 2) {
		string link_with_filename = arg[0] + " \"--output-document=" + arg[1] + "\"";
		return Download(link_with_filename);
	}

	string original_url = arg[0];
	string cookie_file  = "fwatch\\tmp\\__cookies.txt";
	string token_file   = "fwatch\\tmp\\__downloadtoken";
	string arguments    = "";
	string new_url      = original_url;
	int result          = 0;
	bool found_phrase   = false;

	DeleteFile(cookie_file.c_str());
	DeleteFile(token_file.c_str());

	for (int i=1; i<arg.size()-1; i++) {
		arguments  = i==1 ? "--keep-session-cookies --save-cookies " : "--load-cookies ";
		arguments += cookie_file + " --output-document=" + token_file + " " + new_url;
		
		result = Download(arguments, OVERWRITE, new_url);

		if (result != 0)
			return result;

		// Parse downloaded file and find link
		fstream token_file_handle;
		token_file_handle.open(token_file.c_str(), ios::in);
		
		if (token_file_handle.is_open()) {
		    string line = "";
	
			while(getline(token_file_handle, line)) {
				size_t find = line.find(arg[i]);
	
				if (find != string::npos) {
					size_t left_quote  = line.rfind("\"", find);
					size_t right_quote = line.find("\"", find + arg[i].length());
					
					if (left_quote!=string::npos && right_quote!=string::npos) {
						left_quote++;
						found_phrase = true;
						new_url      = line.substr(left_quote, right_quote - left_quote);
						new_url      = ReplaceAll(new_url, "&amp;", "&");
						
						// if relative address
						if (new_url[0] == '/') {
							int offset = 0;
							size_t doubleslash = original_url.find("//");
							
							if (doubleslash != string::npos)
								offset = doubleslash + 2;
							
							size_t slash = original_url.find_first_of("/", offset);
							
							if (slash != string::npos)
								original_url = original_url.substr(0, slash);
							
							new_url = original_url + new_url;
						}
						
						break;
					}
				}
			}

			token_file_handle.close();
		}

		if (!found_phrase)
			return ErrorMessage(STR_DOWNLOAD_FIND_ERROR, "%STR% " + arg[i]);
			
		token_file += Int2Str(i);
	}

	arguments  = "--load-cookies " + cookie_file;
	arguments +=  " \"--output-document=" + arg[arg.size()-1] + "\" " + new_url;
	result     = Download(arguments, 0, new_url);

	if (!global.test_mode) {
		DeleteFile(cookie_file.c_str());
		DeleteFile(token_file.c_str());
	}

	return result;
}


int Unpack(string file_name, string password="")
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
	string arguments = WrapInQuotes(global.working_directory) + (password.empty() ? "" : " -p"+password) + " x -y -o" + destination + "\\ -bb3 -bsp1 " + "\"fwatch\\tmp\\" + file_name + "\"";

	if (!CreateProcess("fwatch\\data\\7z.exe", &arguments[0], NULL, NULL, true, 0, NULL, NULL, &si, &pi)) {		
		int errorCode = GetLastError();
		return ErrorMessage(STR_ERROR_EXE, "%STR% 7z.exe - " + Int2Str(errorCode) + " " + FormatError(errorCode));
	} else
		global.logfile << "Extracting " << file_name << endl;
		
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
			return USER_ABORTED;
		}
		
		ParseUnpackLog(message, file_name);
		GetExitCodeProcess(pi.hProcess, &exit_code);
		Sleep(100);
	}
	while (exit_code == STILL_ACTIVE);

	ParseUnpackLog(message, file_name);

	if (exit_code != 0) {
		global.logfile << exit_code << " - " << message << endl;
		
		if (message.find("Can not open the file as") != string::npos  &&  message.find("archive") != string::npos)
			message += " - " + global.lang[STR_UNPACK_REDO_FILE];

		if (global.unpack_set_error)
			ErrorMessage(STR_UNPACK_ERROR, "%STR%\\n" + file_name + "\\n\\n" + message);
	}

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	CloseHandle(logFile);

	return (exit_code!=0 ? COMMAND_FAILED : NO_ERRORS);
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
			int errorCode = GetLastError();
			if (errorCode != 183)
				return ErrorMessage(STR_MDIR_ERROR, "%STR% " + build_path + " " + Int2Str(errorCode) + " " + FormatError(errorCode));
		} else
			global.logfile << "Created directory " << build_path << endl;
	}
	
	return NO_ERRORS;
}


int CreateFileList(string source, string destination, vector<string> &sources, vector<string> &destinations, vector<bool> &dirs, int options, vector<string> &empty_dirs, int &buffer_size, int &recursion)
{
	WIN32_FIND_DATAW fd;
	HANDLE hFind        = INVALID_HANDLE_VALUE;
	wstring source_wide = string2wide(source);
	hFind               = FindFirstFileW(source_wide.c_str(), &fd);
	int result          = 0;

	if (hFind == INVALID_HANDLE_VALUE) {
		int errorCode = GetLastError();
		if (options & ALLOW_ERROR && errorCode==2 || errorCode==3)
			return 0;

		return ErrorMessage(STR_ERROR_FILE_LIST, "%STR% " + source + "  - " + Int2Str(errorCode) + " " + FormatError(errorCode));
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
		if (fd.cFileName[0] == '.')
			continue;
	    
		string file_name       = wide2string(fd.cFileName);
		string new_source      = base_source      + file_name;
		string new_destination = base_destination + (is_destination_wildcard ? MaskNewName(file_name,new_name) : new_name);
		bool   is_dir          = fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
		int attributes         = GetFileAttributes(new_destination.c_str());

		if (is_dir  &&  is_source_wildcard  &&  ~options & MATCH_DIRS)
			continue;

		// move modfolder to the game dir when using wildcards
		if (is_dir  &&  options & MATCH_DIRS  &&  recursion==0  &&  Equals(destination,global.current_mod_new_name+"\\")  &&  IsModName(file_name)) {
			new_destination = global.current_mod_new_name;
			attributes      = GetFileAttributes(new_destination.c_str());
		}

		// If we need full paths and their totaled length
		if (buffer_size != 0) {
			new_destination = global.working_directory + "\\" + new_destination;
			buffer_size    += new_destination.length() + 1;
		}


		// Check if destination directory already exists
		if (is_dir  &&  (attributes != INVALID_FILE_ATTRIBUTES  &&  attributes & FILE_ATTRIBUTE_DIRECTORY  ||  ~options & MOVE_FILES)  &&  buffer_size==0) {
			if (!~options & MOVE_FILES)
				CreateDirectory(new_destination.c_str(), NULL);
			else
				empty_dirs.push_back(new_source);

			// If dir already exists then move its contents
			new_source      += "\\*";
			new_destination += "\\";
			result           = CreateFileList(new_source, new_destination, sources, destinations, dirs, options, empty_dirs, buffer_size, recursion);
			if (result != 0)
				break;
		} else {
			sources     .push_back(new_source);
			destinations.push_back(new_destination);
			dirs        .push_back(is_dir);
		}
	}
	while (FindNextFileW(hFind, &fd) != 0);

	recursion--;
	FindClose(hFind);
	return result;
}


int MoveFiles(string source, string destination, string new_name, int options)
{
	WriteProgressFile(INSTALL_PROGRESS, global.lang[STR_ACTION_COPYING]+"...");


	// Find files and save them to a list
	vector<string> source_list;
	vector<string> destination_list;
	vector<bool>   is_dir_list;
	vector<string> empty_dirs;
	int buffer_size = 0;
	int recursion   = -1;
	
	int result = CreateFileList(source, destination+new_name, source_list, destination_list, is_dir_list, options, empty_dirs, buffer_size, recursion);

	if (result != 0)
		return result;
	

	DWORD flags       = MOVEFILE_REPLACE_EXISTING;
	bool FailIfExists = false;
	int return_value  = 0;

	if (~options & OVERWRITE) {
		FailIfExists = true;
		flags        = 0;
	}
	
	wstring source_wide;
	wstring destination_wide;


	// For each file in the list
	for (int i=0;  i<source_list.size(); i++) {
		if (Abort())
			return USER_ABORTED;

		// Format path for logging
		string destinationLOG = PathNoLastItem(destination_list[i], NO_END_SLASH);

		if (destinationLOG.empty())
			destinationLOG = "the game folder";
		
		global.logfile << (options & MOVE_FILES ? "Moving" : "Copying") << "  " << ReplaceAll(source_list[i], "fwatch\\tmp\\_extracted\\", "") << "  to  " << destinationLOG;
		
		if (!new_name.empty())
			global.logfile << "  as  " << PathLastItem(destination_list[i]);

		source_wide      = string2wide(source_list[i]);
		destination_wide = string2wide(destination_list[i]);
		
		if (options & MOVE_FILES)
			result = MoveFileExW(source_wide.c_str(), destination_wide.c_str(), flags);
		else
			result = CopyFileW(source_wide.c_str(), destination_wide.c_str(), FailIfExists);

	    if (result == 0) {
			return_value = GetLastError();
			
	    	if (~options & OVERWRITE  &&  return_value==183)
	    		global.logfile << "  - file already exists";
			else
				global.logfile << "  FAILED " << return_value << " " << FormatError(return_value);
		}
			
		global.logfile << endl;
	}


	// Remove empty directories
	if (options & MOVE_FILES)
		for (int i=empty_dirs.size()-1; i>=0; i--)
			RemoveDirectory(empty_dirs[i].c_str());

	return NO_ERRORS;
}


int RequestExecution(string path_to_dir, string file_name)
{		
	global.logfile << "Asking the user to run " << file_name << endl;
	
	string message = global.lang[STR_ASK_EXE] + ":\\n" + file_name + "\\n\\n" + global.lang[STR_ALTTAB];
	WriteProgressFile(INSTALL_WAITINGFORUSER, message);
	
	message = "You must manually run\n" + file_name + "\n\nPress OK to start\nPress CANCEL to skip installing this modfolder";
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
		DWORD exit_code;
		do {
			GetExitCodeProcess(pi.hProcess, &exit_code);
			Sleep(100);
		}
		while (exit_code == STILL_ACTIVE);
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

	return NO_ERRORS;
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
			return USER_ABORTED;
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
			MoveFiles(source, destination, "", MOVE_FILES | OVERWRITE);	
		}

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	CloseHandle(logFile);
	Sleep(600);

	return (exit_code!=0 ? COMMAND_FAILED : NO_ERRORS);
}


int ParseMissionSQM(string path) 
{
	WriteProgressFile(INSTALL_PROGRESS, global.lang[STR_ACTION_READMISSIONSQM]);
	
	ifstream file(path.c_str());
	string buffer;
	
	if (file.is_open()) {
		file.seekg(0, ios::end);   
		buffer.reserve(file.tellg());
		file.seekg(0, ios::beg);
	
		buffer.assign((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
		file.close();
	} else 
		return NO_MISSIONSQM;
		
	int expect       = PROPERTY;
	int word_start   = -1;
	int class_level  = 0;
	int array_level  = 0;
	int player_count = 0;
	bool is_array    = false;
	bool in_quote    = false;
	bool in_mission  = false;
	char separator   = ' ';
	
	for (int i=0; i<buffer.size(); i++) {
		char c = buffer[i];
		
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
					class_level--;

					if (in_mission && class_level==0) {
						i = buffer.size();
						break;
					}
									
					expect = SEMICOLON;
					continue;
				}
				
				if (isalnum(c) || c=='_' || c=='[' || c==']') {
					if (word_start == -1)
						word_start = i;
				} else
					if (word_start >= 0) {
						string word = buffer.substr(word_start, i-word_start);
	
						if (word == "class") {
							expect    = CLASS_NAME;
						} else {
							expect    = EQUALITY;
							separator = '=';
							is_array  = buffer[i-2]=='[' && buffer[i-1]==']';
							
							if (word == "player")
								if (++player_count > 1)
									return MULTI_PLAYER;
						}

						word_start = -1;
					}
				
				if (separator == ' ')
					break;
			}
			
			case EQUALITY : {
				if (c == separator) {
					expect++;
					separator = ' ';
				} else 
					if (!isspace(c)) {
						i--;
						separator = ' ';
						expect    = SEMICOLON;
					}
				
				break;
			}
			
			case VALUE : {
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
						word_start = -1;
						expect     = PROPERTY;
					}
				}
				
				break;
			}
			
			case CLASS_NAME : {
				if (isalnum(c) || c=='_') {
					if (word_start == -1)
						word_start = i;
				} else
					if (word_start >= 0) {
						if (buffer.substr(word_start, i-word_start) == "Mission")
							in_mission = true;

						word_start = -1;
						expect     = CLASS_BRACKET;
					}
				
				if (expect != CLASS_BRACKET)
					break;
			}
			
			case CLASS_BRACKET : {
				if (c == '{') {
					class_level++;
					expect = PROPERTY;
				} else
					if (!isspace(c)) {
						i--;
						expect = SEMICOLON;
					}
				
				break;
			}
		}
	}
	
	return SINGLE_PLAYER;
}


int Auto_Install(string file, string password="")
{
	if (Abort())
		return USER_ABORTED;

	string file_with_path = "fwatch\\tmp\\" + file;
	string file_only      = PathLastItem(file);
	bool FirstFile        = file.find_last_of("\\") == string::npos;
	size_t dots           = count(file_only.begin(), file_only.end(), '.');
	int attributes        = GetFileAttributesW(string2wide(file_with_path).c_str());


	// Get file attributes to see if it's a directory
	if (attributes == INVALID_FILE_ATTRIBUTES) {
		int errorCode   = GetLastError();
		string errorMSG = FormatError(errorCode);
		
		if (FirstFile)
			return ErrorMessage(STR_AUTO_READ_ATTRI, "%STR% " + file + " - " + Int2Str(errorCode) + " " + errorMSG);
		else {
			global.logfile << "Failed to get attributes of " << file << " - " << errorCode + " " << errorMSG << endl;
			return NO_ERRORS;
		}
	} else
		if (attributes & FILE_ATTRIBUTE_DIRECTORY) {
			// If it's a modfolder that we're installing
			if (IsModName(file_only)) {
				int result = MoveFiles(file_with_path, "", global.current_mod_new_name, MOVE_FILES | OVERWRITE | MATCH_DIRS);
				
				if (FirstFile && result != 0)
					return result;
				else
					return NO_ERRORS;
			} else {
				string mod_subfolders[] = {
					"addons",
					"bin",
					"campaigns",
					"dta",
					"worlds",
					"missions",
					"mpmissions"
				};
				
				int max = sizeof(mod_subfolders) / sizeof(mod_subfolders[0]);
				
				// Check directory name
				bool browse         = true;
				bool test_modfolder = false;
		
				for (int i=0; i<max; i++) {
					if (!test_modfolder) {
						if (Equals(file_only,mod_subfolders[i])) {
							wstring missions_sub_dir;
							
							// If "Missions" subfolder contains a single subfolder then move that subfolder instead
							if (Equals(file_only,"missions")) {
								int number_of_files = 0;
								int number_of_dirs  = 0;
								
								HANDLE hFile;
								WIN32_FIND_DATAW FileInformation;
								wstring pattern = string2wide(file_with_path) + L"\\*.*";
								hFile = FindFirstFileW(pattern.c_str(), &FileInformation);
								
								if (hFile != INVALID_HANDLE_VALUE) {
									do {
										if (FileInformation.cFileName[0] == '.')
											continue;
											
										missions_sub_dir = FileInformation.cFileName;
										
										if (FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
											number_of_dirs++;
										else
											number_of_files++;
									} 
									while(FindNextFileW(hFile, &FileInformation) == TRUE);
									
									FindClose(hFile);
								}

								if (number_of_files==0  &&  number_of_dirs==1) {
									wstring src = string2wide(file_with_path);
									wstring dst = src;
									src         += L"\\" + missions_sub_dir;
									dst         += L"\\Missions";
									MoveFileW(src.c_str(), dst.c_str());
									global.logfile << "Renaming " << wide2string(src) << " to " << wide2string(dst) << endl;
									file += "\\Missions";
								}
							}

							// Move the entire folder to the game\mod directory
							MakeDir(global.current_mod_new_name);
							int result = MoveFiles("fwatch\\tmp\\"+file, global.current_mod_new_name+"\\", "", MOVE_FILES | OVERWRITE);
							
							if (FirstFile  &&  result != 0)
								return result;
							else
								return NO_ERRORS;
						}
					} else {
						// Test if a subfolder is some other modfolder - if so then ignore it
						if (mod_subfolders[i] == "missions"  ||  mod_subfolders[i] == "mpmissions")
							continue;
							
						string mod_subpath = file_with_path + "\\" + mod_subfolders[i];
						int attributes     = GetFileAttributesW(string2wide(mod_subpath).c_str());
						
						if (attributes != INVALID_FILE_ATTRIBUTES && attributes & FILE_ATTRIBUTE_DIRECTORY && !IsModName(file)) {
							browse = false;
							break;
						}
					}

					if (!test_modfolder  &&  i == max-1  &&  !Equals(file_only,"_extracted")) {
						i              = -1;
						test_modfolder = true;
					}
				}
		
				// If directory has a dot in its name then move it to missions
				if (dots > 0) {
					int result = ParseMissionSQM(file_with_path+"\\mission.sqm");
				
					if (result != NO_MISSIONSQM) {
						string destination = result==SINGLE_PLAYER ? "Missions" : "MPMissions";						
						MakeDir(global.current_mod_new_name+"\\"+destination);
						int result = MoveFiles(file_with_path, global.current_mod_new_name+"\\"+destination+"\\", "", MOVE_FILES | OVERWRITE);
								
						if (FirstFile && result != 0)
							return result;
						else
							return NO_ERRORS;
					}
				}

				if (!browse) 
					return NO_ERRORS;



				// Normal directory - search through its contents
				HANDLE hFile;
				WIN32_FIND_DATAW FileInformation;
				wstring pattern = string2wide(file_with_path) + L"\\*";
				hFile           = FindFirstFileW(pattern.c_str(), &FileInformation);

				if (hFile == INVALID_HANDLE_VALUE) {
					int errorCode   = GetLastError();
					string errorMSG = FormatError(errorCode);
					
					if (FirstFile)
						return ErrorMessage(STR_ERROR_FILE_LIST, "%STR% " + file + " - " + Int2Str(errorCode) + " " + errorMSG);
					else {
						global.logfile << "Failed to list files in " << file << " - " << errorCode + " " << errorMSG << endl;
						return NO_ERRORS;
					}
				}

				do {
					if (FileInformation.cFileName[0] != '.') {
						string path_to_this_file = file + "\\" + wide2string(FileInformation.cFileName);
						Auto_Install(path_to_this_file);
					}
				} 
				while(FindNextFileW(hFile, &FileInformation) == TRUE);
				
				FindClose(hFile);
			}
		}
	else {
		string file_extension = GetFileExtension(file);
		bool extract          = false;
		
		if (file.substr(0,11) == "_extracted\\")
			file = file.substr(11);
		
		file = WrapInQuotes(file);
			
		if (file_extension == "pbo") {
			string destination = "addons";
			
			if (dots > 1) {
				destination = "MPMissions";

				if (ExtractPBO(global.working_directory+"\\"+file_with_path, "", "mission.sqm", 1) == 0) {
					string extracted_dir = PathNoLastItem(file_with_path) + file.substr(0, file.length()-4);

					switch(ParseMissionSQM(extracted_dir+"\\mission.sqm")) {
						case NO_MISSIONSQM : destination="addons"; break;
						case SINGLE_PLAYER : destination="Missions"; break;
						case MULTI_PLAYER  : destination="MPMissions"; break;
					}
					
					DeleteDirectory(extracted_dir);
				}
			}
			
			MakeDir(global.current_mod_new_name+"\\"+destination);
			int result = MoveFiles(file_with_path, global.current_mod_new_name+"\\"+destination+"\\", "", MOVE_FILES | OVERWRITE);
			
			if (FirstFile  &&  result != 0)
				return result;
			else
				return NO_ERRORS;
		} else {
			string archives[] = {
				"rar",
				"zip",
				"7z",
				"ace",
				"exe"
			};
			
			int max = sizeof(archives) / sizeof(archives[0]);
			
			for (int i=0; i<max; i++)
				if (file_extension == archives[i])
					extract = true;
			
			// Allow to extract only one executable
			if (file_extension=="exe"  &&  !global.first_exe)
				extract = false;
							
			if (extract) {
				global.unpack_set_error = FirstFile  &&  file_extension!="exe";
				int result              = Unpack(file_with_path.substr(11), password);
				password                = "";
				
				// If exe unpacking failed then ask user to run it
				if (file_extension == "exe") {
					if (result > 0  &&  global.first_exe) {
						global.first_exe = false;
						result           = RequestExecution("", file);
					}
					
					global.first_exe = false;
				}
								
				if (FirstFile  &&  result != 0)
					return result;
					
				string extracted_files = PathNoLastItem(file_with_path.substr(11)) + "_extracted";
				Auto_Install(extracted_files);
			}
		}
	}
	
	return NO_ERRORS;
};


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
			int result      = DeleteFile(filename.c_str());

			if (!result) {
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
	
	global.current_mod    = "";
	global.first_exe      = true;
	global.skip_modfolder = false;
	global.game_ver_index = -1;
	global.game_ver_cond.clear();
	global.current_mod_alias.clear();
}


	//https://support.microsoft.com/en-us/help/167296/how-to-convert-a-unix-time-t-to-a-win32-filetime-or-systemtime
void UnixTimeToFileTime(time_t t, LPFILETIME pft)
{
	LONGLONG ll;
	ll = Int32x32To64(t, 10000000) + 116444736000000000;
	pft->dwLowDateTime = (DWORD)ll;
	pft->dwHighDateTime = ll >> 32;
}


int ChangeFileDate(string file_name) 
{
	FILETIME ft;
	UnixTimeToFileTime(global.current_mod_version_date, &ft);
   
	HANDLE file_handle = CreateFile(file_name.c_str(), FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	bool result = SetFileTime(file_handle, (LPFILETIME)NULL, (LPFILETIME)NULL, &ft);
	CloseHandle(file_handle);
	
	if (result)
		return NO_ERRORS;
	else
		return GetLastError();
}
// -------------------------------------------------------------------------------------------------------










// Scripting commands ------------------------------------------------------------------------------------

int SCRIPT_Download(const vector<string> &arg) 
{
	vector<string> Download_Arguments;

	for (int i=1;  i<arg.size();  i++)
		Download_Arguments.push_back(arg[i]);

	return Download_Wrapper(Download_Arguments);
}


int SCRIPT_Unpack(const vector<string> &arg) 
{
	string file_name    = "";
	string password     = "";
	bool save_downl_arg = false;
	vector<string> Download_Arguments;

	for (int i=1, j=0;  i<arg.size();  i++) {
		if (Equals(arg[i].substr(0,10), "/password:")) {
			password = arg[i].substr(10);
			continue;
		}

		if (IsURL(arg[i]))
			save_downl_arg = true;
			
		if (save_downl_arg) {
			Download_Arguments.push_back(arg[i]);
			continue;
		}

		if (file_name.empty())
			file_name = arg[i];
	}

	if (file_name.empty()) {
		if (Download_Arguments.size() > 0) {
			int result = Download_Wrapper(Download_Arguments);
			
			if (result > 0)
				return result;
		}
		
		file_name = global.downloaded_filename;
	}

	if (file_name.empty())
		return ErrorMessage(STR_ERROR_NO_FILE);

	return Unpack(file_name, password);
}


int SCRIPT_Move(const vector<string> &arg) 
{
	vector<string> path;
	vector<string> Download_Arguments;
	path.push_back("");
	path.push_back("");
	path.push_back("");

	bool is_download_dir = true;
	bool save_downl_arg  = false;
	int options          = OVERWRITE | (Equals(arg[0],"move") ? MOVE_FILES : 0);
	
	for (int i=1, j=0;  i<arg.size();  i++) {
		if (Equals(arg[i], "/no_overwrite")) {
			options &= ~OVERWRITE;
			continue;
		}
		
		if (Equals(arg[i], "/match_dir")) {
			options |= MATCH_DIRS;
			continue;
		}
		
		if (j==0  &&  IsURL(arg[i])) {
			save_downl_arg = true;
			path[j++] = "<download>";
		}
			
		if (save_downl_arg) {
			if (arg[i] == "|") {
				save_downl_arg = false;
				continue;
			}
	
			Download_Arguments.push_back(arg[i]);
			continue;
		}

		if (j<3  &&  path[j].empty()) {
			if (!VerifyPath(arg[i]))
				return ErrorMessage(STR_ERROR_PATH);

			if (arg[i] != ".")
				path[j++] = arg[i];
		}
	}

	
	// Format source path
	if (path[SOURCE].empty())
		return ErrorMessage(STR_ERROR_NO_FILE);

	if (Download_Arguments.size() > 0) {
		int result = Download_Wrapper(Download_Arguments);

		if (result > 0)
			return result;
			
		if (options & MOVE_FILES)
			global.downloads.pop_back();
	}
	
	if (Equals(path[SOURCE],"<download>")  ||  Equals(path[SOURCE],"<dl>"))
		path[SOURCE] = "fwatch\\tmp\\" + global.downloaded_filename;
	else 
		if (Equals(path[SOURCE].substr(0,5),"<mod>")) {
			path[SOURCE]    = global.current_mod_new_name + path[SOURCE].substr(5);
			is_download_dir = false;
		} else
			if (Equals(path[SOURCE].substr(0,7),"<game>\\")) {
				is_download_dir = false;
				
				if (~options & MOVE_FILES)
					path[SOURCE] = path[SOURCE].substr(7);
				else
					return ErrorMessage(STR_MOVE_DST_PATH_ERROR);
			} else
				path[SOURCE] = "fwatch\\tmp\\_extracted\\" + path[SOURCE];

	// If user selected directory then move it along with its sub-folders
	if (path[SOURCE].find("*")==string::npos && path[SOURCE].find("?")==string::npos && GetFileAttributes(path[SOURCE].c_str()) & FILE_ATTRIBUTE_DIRECTORY)
		options |= MATCH_DIRS;

	// Format destination path
	bool destination_passed = !path[DESTINATION].empty();
	path[DESTINATION]       = global.current_mod_new_name + "\\" + path[DESTINATION];
	
	if (path[DESTINATION].substr(path[DESTINATION].length()-1) != "\\")
		path[DESTINATION] += "\\";

	// If user wants to move modfolder then change destination to the game directory
	if (is_download_dir  &&  IsModName(PathLastItem(path[SOURCE]))  &&  !destination_passed) {
		path[DESTINATION] = "";
		path[NEW_NAME]    = Equals(global.current_mod,global.current_mod_new_name) ? "" : global.current_mod_new_name;
		options          |= MATCH_DIRS;
	} else {
		// Otherwise create missing directories in the destination path
		int result = MakeDir(PathNoLastItem(path[DESTINATION]));
		if (result != 0)
			return result;
	}
		

	// Format new name 
	// 3rd argument - new name
	if (path[NEW_NAME].find("\\")!=string::npos  ||  path[NEW_NAME].find("/")!=string::npos)
		return ErrorMessage(STR_RENAME_DST_PATH_ERROR);

	return MoveFiles(path[SOURCE], path[DESTINATION], path[NEW_NAME], options);
}


int SCRIPT_MakeDir(const vector<string> &arg) 
{
	string directory = "";

	if (arg.size() >= 2)
		directory = arg[1];	
	
	if (!VerifyPath(directory))
		return ErrorMessage(STR_ERROR_PATH);

	return MakeDir(global.current_mod_new_name + "\\" + directory);
}


int SCRIPT_RequestExecution(const vector<string> &arg) 
{
	string file_name    = "";
	bool save_downl_arg = false;
	vector<string> Download_Arguments;

	for (int i=1, j=0;  i<arg.size();  i++) {
		if (IsURL(arg[i]))
			save_downl_arg = true;

		if (save_downl_arg) {
			Download_Arguments.push_back(arg[i]);
			continue;
		}

		if (file_name.empty())
			file_name = arg[i];
	}

	if (file_name.empty()  &&  Download_Arguments.size()==0)
		file_name = global.downloaded_filename;
	else {
		int result = Download_Wrapper(Download_Arguments);
		
		if (result > 0)
			return result;
			
		file_name = global.downloaded_filename;
	}

	if (file_name.empty())
		return ErrorMessage(STR_ERROR_NO_FILE);

	if (!VerifyPath(file_name))
		return ErrorMessage(STR_ERROR_PATH);

	string path_to_dir = global.working_directory;
	
	if (file_name.substr(0,6) == "<mod>\\") {
		file_name    = file_name.substr(6);
		path_to_dir += "\\" + global.current_mod_new_name + "\\";
	} else
		path_to_dir += "\\fwatch\\tmp\\";

	return RequestExecution(path_to_dir, file_name);
}


int SCRIPT_StartMod(const vector<string> &arg) 
{
	if (arg.size() < 4)
		return ErrorMessage(STR_ERROR_INVALID_SCRIPT, "%STR%", SCRIPT_ERROR);
	
	if (!global.current_mod.empty())
		EndMod();
		
	WriteProgressFile(INSTALL_PROGRESS, global.lang[STR_ACTION_PREPARING]);

	global.current_mod          = arg[1];
	global.current_mod_id       = arg[2];
	global.current_mod_keepname = arg[3];
	global.command_line_num     = 0;
	global.current_mod_version  = "";
	
	// Make a list of mod aliases for the entire installation
	vector<string> aliases;
	Tokenize(arg[4], " ", aliases);
	
	for (int i=0; i<aliases.size(); i++)
		global.current_mod_alias.push_back(aliases[i]);
		
	global.saved_alias_array_size = global.current_mod_alias.size();
	
	
	// Find to which folder we should install the mod
	for (int i=0;  i<global.mod_id.size(); i++)
		if (Equals(global.current_mod_id,global.mod_id[i]))
			global.current_mod_new_name = global.mod_name[i];

	if (global.current_mod_new_name.empty())
		return ErrorMessage(STR_ERROR_INVALID_ARG, "%STR%", SCRIPT_ERROR);

	
	bool activate_rename = false;
	
	// Check if modfolder already exists
	DWORD dir = GetFileAttributesA(global.current_mod_new_name.c_str());

	if (dir != INVALID_FILE_ATTRIBUTES) {
		activate_rename = true;
		
		if (dir & FILE_ATTRIBUTE_DIRECTORY) {
			fstream id_file;
			string id_file_name = global.current_mod_new_name+"\\__gs_id";
			id_file.open(id_file_name.c_str(), ios::in);
			
			if (id_file.is_open()) {
				string text;
				getline(id_file, text);
				vector<string> parts;
				Tokenize(text, ";", parts);
				
				if (parts.size() > 0)
					activate_rename = parts[0] != global.current_mod_id;
						
				id_file.close();
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
				if (last_error != 183) {
					ErrorMessage(STR_MOVE_RENAME_ERROR, "%STR% " + rename_src + " "+global.lang[STR_MOVE_RENAME_TO_ERROR]+" " + rename_dst + " - " + Int2Str(last_error) + " " + FormatError(last_error));
					return COMMAND_FAILED;
				}
			}
		} while (last_error == 183);
		
		global.logfile << "Renaming existing " << rename_src << " to " << rename_dst << endl;
	}

	return NO_ERRORS;
}


int SCRIPT_Delete(const vector<string> &arg) 
{
	WriteProgressFile(INSTALL_PROGRESS, global.lang[STR_ACTION_DELETING]+"...");


	vector<string> path;
	path.push_back("");
	
	int options = MOVE_FILES | ALLOW_ERROR;
	
	for (int i=1, j=0;  i<arg.size();  i++) {
		if (Equals(arg[i], "/match_dir")) {
			options |= MATCH_DIRS;
			continue;
		}

		if (j<1  &&  path[j].empty()) {
			if (!VerifyPath(arg[i]))
				return ErrorMessage(STR_ERROR_PATH);
			
			path[j++] = arg[i];
		}
	}


	// Format source path
	bool trash = false;

	if (path[SOURCE].empty()) {
		if (global.downloaded_filename.empty())
			return ErrorMessage(STR_ERROR_NO_FILE);
	
		path[SOURCE] = "fwatch\\tmp\\" + global.downloaded_filename;
	} else {
		path[SOURCE] = global.current_mod_new_name + "\\" + path[SOURCE];
		trash        = true;
	}


	// Find files and save them to a list
	vector<string> source_list;
	vector<string> destination_list;
	vector<bool>   is_dir_list;
	vector<string> empty_dirs;
	int buffer_size  = 1;
	int recursion    = -1;

	int result = CreateFileList(path[SOURCE], PathNoLastItem(path[SOURCE]), source_list, destination_list, is_dir_list, options, empty_dirs, buffer_size, recursion);

	if (result != 0)
		return result;


	// Allocate buffer for the file list
	char *file_list;
	int base_path_len = global.working_directory.length() + 1;
	int buffer_pos    = 0;
	wstring temp;
	
	if (trash) {
		file_list = new char[buffer_size*2];

		if (!file_list)
			return ErrorMessage(STR_ERROR_BUFFER, "%STR% " + Int2Str(buffer_size));
	}

  
	// For each file in the list
	for (int i=0; i<destination_list.size(); i++) {
		if (Abort())
			return USER_ABORTED;
		
		if (trash) {
			temp            = string2wide(destination_list[i]);
			int name_length = (temp.length()+1) * 2;
			global.logfile << "Trashing " << destination_list[i].substr(base_path_len) << endl;
			memcpy(file_list+buffer_pos, temp.c_str(), name_length);
			buffer_pos     += name_length;
		} else {
			global.logfile << "Deleting  " << destination_list[i].substr(base_path_len);
			int result = DeleteDirectory(destination_list[i]);
			
			if (result != 0)
				global.logfile << "  FAILED " << result << " " << FormatError(result);
				
			global.logfile << endl;
		}
	}

	if (trash) {
		memcpy(file_list+buffer_pos, "\0\0", 2);

		// Trash file
		SHFILEOPSTRUCTW shfos;
		shfos.hwnd   = NULL;
		shfos.wFunc  = FO_DELETE;
		shfos.pFrom  = (LPCWSTR)file_list;
		shfos.pTo    = NULL;
		shfos.fFlags = FOF_SILENT | FOF_NOCONFIRMATION | FOF_ALLOWUNDO;
		int result   = SHFileOperationW(&shfos);
				
	    if (result != 0)
			global.logfile << "Trashing FAILED " << result << " " << FormatError(result) << endl;

		delete[] file_list;
	}

	return NO_ERRORS;
}


int SCRIPT_Rename(const vector<string> &arg) 
{
	WriteProgressFile(INSTALL_PROGRESS, global.lang[STR_ACTION_RENAMING]+"...");

	vector<string> path;
	path.push_back("");
	path.push_back("");

	int options = MOVE_FILES;
	
	for (int i=1, j=0;  i<arg.size();  i++) {
		if (Equals(arg[i], "/match_dir")) {
			options |= MATCH_DIRS;
			continue;
		}

		if (j<2  &&  path[j].empty()) {
			if (!VerifyPath(arg[i]))
				return ErrorMessage(STR_ERROR_PATH, "%STR%");
			
			path[j++] = arg[i];
		}
	}

	// Format source path
	if (path[SOURCE].empty())
		return ErrorMessage(STR_RENAME_NO_NAME_ERROR, "%STR%");
	
	if (Equals(path[SOURCE],"<download>")  ||  Equals(path[SOURCE],"<dl>"))	
		path[SOURCE]  = "fwatch\\tmp\\_extracted\\" + global.downloaded_filename;
	else
		path[SOURCE] = global.current_mod_new_name + "\\" + path[SOURCE];

	string relative_path = PathNoLastItem(path[SOURCE]);
	string lastItem      = PathLastItem(path[SOURCE]);
	bool source_wildcard = false;

	if (relative_path.find("*")!=string::npos  ||  relative_path.find("?")!=string::npos)
		return ErrorMessage(STR_RENAME_WILDCARD_ERROR, "%STR%");
	

	// Format new name
	if (path[DESTINATION].empty())
		return ErrorMessage(STR_RENAME_NO_NAME_ERROR);
	
	if (path[DESTINATION].find("\\")!=string::npos  ||  path[DESTINATION].find("/")!=string::npos)
		return ErrorMessage(STR_RENAME_DST_PATH_ERROR);
		
			
	// Find files and save them to a list
	vector<string> source_list;
	vector<string> destination_list;
	vector<bool>   is_dir_list;
	vector<string> empty_dirs;
	int buffer_size = 0;
	int recursion   = -1;
	
	int result = CreateFileList(path[SOURCE], relative_path+path[DESTINATION], source_list, destination_list, is_dir_list, options, empty_dirs, buffer_size, recursion);

	if (result != 0)
		return result;

	wstring source_wide;
	wstring destination_wide;
	

	// For each file on the list
	for (int i=0;  i<source_list.size(); i++) {
		if (Abort())
			return USER_ABORTED;

		// Format path for logging
		global.logfile << "Renaming  " << ReplaceAll(source_list[i], "fwatch\\tmp\\_extracted\\", "") << "  to  " << PathLastItem(destination_list[i]);
		
		// Rename
		source_wide      = string2wide(source_list[i]);
		destination_wide = string2wide(destination_list[i]);
		result = MoveFileExW(source_wide.c_str(), destination_wide.c_str(), 0);
			
	    if (!result) {
			int errorCode = GetLastError();
			global.logfile << "  FAILED " << errorCode << " " << FormatError(errorCode);
	    }
	    
	    global.logfile << endl;
	}

	return NO_ERRORS;
}

static int CALLBACK BrowseCallbackProc(HWND hwnd,UINT uMsg, LPARAM lParam, LPARAM lpData)
{

    if(uMsg == BFFM_INITIALIZED)
    {
        std::string tmp = (const char *) lpData;
        //std::cout << "path: " << tmp << std::endl;
        SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
    }

    return 0;
}

//https://stackoverflow.com/questions/12034943/win32-select-directory-dialog-from-c-c
std::string BrowseFolder(std::string saved_path)
{
    TCHAR path[MAX_PATH];

    const char * path_param = saved_path.c_str();

    BROWSEINFO bi = { 0 };
    bi.lpszTitle  = ("Browse for folder...");
    bi.ulFlags    = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    bi.lpfn       = BrowseCallbackProc;
    bi.lParam     = (LPARAM) path_param;

    LPITEMIDLIST pidl = SHBrowseForFolder ( &bi );

    if ( pidl != 0 )
    {
        //get the name of the folder and put it in path
        SHGetPathFromIDList ( pidl, path );

        //free memory used
        IMalloc * imalloc = 0;
        if ( SUCCEEDED( SHGetMalloc ( &imalloc )) )
        {
            imalloc->Free ( pidl );
            imalloc->Release ( );
        }

        return path;
    }

    return "";
}


int SCRIPT_RequestDownload(const vector<string> &arg) 
{
	if (arg.size() < 3)
		return ErrorMessage(STR_ERROR_ARG_COUNT);

	string file_name    = arg[1];
	string url          = arg[2];
	string download_dir = "";
	int return_value    = NO_ERRORS;
	bool move           = false;
	fstream config;
	
	config.open("fwatch\\tmp\\schedule\\DownloadDir.txt", ios::in);
	
	if (config.is_open()) {
		getline(config, download_dir);
		config.close();
	}
			
	// Check if file already exists
	string path1 = download_dir + "\\" + file_name;
	string path2 = "fwatch\\tmp\\" + file_name;
	
	if (GetFileAttributes(path1.c_str()) != INVALID_FILE_ATTRIBUTES) {
		global.logfile << "Found " << path1 << endl;
		move = true;
	}
	else
		if (GetFileAttributes(path2.c_str()) != INVALID_FILE_ATTRIBUTES) {
			global.downloaded_filename = file_name;
			global.logfile << "Found " << path2 << endl;
			global.downloads.push_back(global.downloaded_filename);
		} else {
			string message = global.lang[STR_ASK_DLOAD] + ":\\n" + file_name + "\\n\\n" + global.lang[STR_ALTTAB];
			WriteProgressFile(INSTALL_WAITINGFORUSER, message);
			
			ShellExecute(0, 0, url.c_str(), 0, 0 , SW_SHOW);
			global.logfile << "Opened " << url << endl;
			
			message      = "You must manually download\n" + file_name + "\n\nPress OK once download has finished\nPress CANCEL to skip installing this modfolder";
			int msgboxID = MessageBox(NULL, message.c_str(), "Addon Installer", MB_ICONQUESTION | MB_OKCANCEL | MB_DEFBUTTON1);
			
			if (msgboxID == IDCANCEL)
				global.skip_modfolder = true;
			else {
				if (download_dir.empty()  ||  GetFileAttributes(path1.c_str()) == INVALID_FILE_ATTRIBUTES) {
					WriteProgressFile(INSTALL_WAITINGFORUSER, global.lang[STR_ASK_DLOAD_SELECT] + "\\n\\n" + global.lang[STR_ALTTAB]);
					
					download_dir = BrowseFolder("");
					printf("%s", download_dir.c_str());
					
					config.open("fwatch\\tmp\\schedule\\DownloadDir.txt", ios::out | ios::trunc);
	
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
		
		string source      = download_dir + "\\" + file_name;
		string destination = global.working_directory + "\\fwatch\\tmp\\" + file_name;
		int result         = MoveFileEx(source.c_str(), destination.c_str(), MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING);
		
		global.logfile << "Moving " << source << "  to  " << destination << endl;
		
		if (result == 0) {
			return_value = GetLastError();
			global.logfile << "FAILED " << return_value << " " << FormatError(return_value);
		} else {
			global.downloaded_filename = file_name;
			global.downloads.push_back(global.downloaded_filename);
		}
	}

	return return_value;
}


int SCRIPT_If_version(const vector<string> &arg) 
{
	if (arg.size() < 2)
		return ErrorMessage(STR_ERROR_ARG_COUNT);
	
	// Process arguments (two or three)
	string op          = "==";
	float given_number = 0;
	
	if (arg.size() >= 3) {
		op           = arg[1];
		given_number = atof(arg[2].c_str());
	} else {
		char first_letter = arg[1][0];
		
		if (isdigit(first_letter)  ||  first_letter=='.')
			given_number = atof(arg[1].c_str());
		else
			return ErrorMessage(STR_IF_NUMBER_ERROR);
	}

	
	// Execute condition
	bool result = false;
	
	if (global.game_ver_index > 0  &&  !global.game_ver_cond[global.game_ver_index])
		result = false;
	else {	
		float game_version = atof(global.arguments_table["gameversion"].c_str());

		if (op=="=="  ||  op=="=")
			result = game_version == given_number;
				
		if (op=="<>"  ||  op=="!=")
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
	
	global.game_ver_index++;
	global.game_ver_cond.push_back(result);
	return NO_ERRORS;
}


int SCRIPT_Else(const vector<string> &arg) 
{
	if (global.game_ver_index >= 0) {

		if (global.game_ver_index > 0  &&  !global.game_ver_cond[global.game_ver_index-1]) {
		} else
			global.game_ver_cond[global.game_ver_index] = !global.game_ver_cond[global.game_ver_index];

	}
	
	return NO_ERRORS;
}


int SCRIPT_Endif(const vector<string> &arg) 
{
	if (global.game_ver_index >= 0)
		global.game_ver_index--;
		
	if (global.game_ver_cond.size() > 0)
		global.game_ver_cond.pop_back();

	return NO_ERRORS;
}


int SCRIPT_MakePBO(const vector<string> &arg) 
{
	WriteProgressFile(INSTALL_PROGRESS, global.lang[STR_ACTION_PACKINGPBO]+"...");

		
	vector<string> path;
	path.push_back("");
	
	bool delete_afterwards = true;
	
	for (int i=1, j=0;  i<arg.size();  i++) {
		if (Equals(arg[i], "/no_delete")) {
			delete_afterwards = !delete_afterwards;
			continue;
		}

		if (j<1  &&  path[j].empty()) {
			if (!VerifyPath(arg[i]))
				return ErrorMessage(STR_ERROR_PATH);
			
			path[j++] = arg[i];
		}
	}


	// Format source path
	if (path[SOURCE].empty()) {
		if (global.last_pbo_file.empty())
			return ErrorMessage(STR_ERROR_NO_FILE);
	
		path[SOURCE] = global.last_pbo_file;
	} else
		path[SOURCE] = global.current_mod_new_name + "\\" + path[SOURCE];


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
	
	string exename    = "MakePbo.exe";
	string executable = "fwatch\\data\\" + exename;
	string arguments  = " -NRK \"" + path[SOURCE] + "\"";

	if (!CreateProcess(&executable[0], &arguments[0], NULL, NULL, true, 0, NULL, NULL, &si, &pi)) {
		int errorCode = GetLastError();
		return ErrorMessage(STR_ERROR_EXE, "%STR% " + exename + " - " + Int2Str(errorCode) + " " + FormatError(errorCode));
	} else
		global.logfile << "Creating a PBO file out of " << path[SOURCE] << endl;
		
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
			return USER_ABORTED;
		}
		
		ParsePBOLog(message, exename, path[SOURCE]);
		GetExitCodeProcess(pi.hProcess, &exit_code);
		Sleep(100);
	}
	while (exit_code == STILL_ACTIVE);
	
	ParsePBOLog(message, exename, path[SOURCE]);

	if (exit_code != 0)
		ErrorMessage(STR_PBO_MAKE_ERROR, "%STR% " + Int2Str(exit_code) + " - " + message);
	else {
		ChangeFileDate(path[SOURCE]+".pbo");
		
		if (delete_afterwards) {
			global.logfile << "Removing " << path[SOURCE] << " directory" << endl;
			global.last_pbo_file = "";
			DeleteDirectory(path[SOURCE]);
		}
	}
	
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	CloseHandle(logFile);
	Sleep(1000);
	
	return (exit_code!=0 ? COMMAND_FAILED : NO_ERRORS);
}


int SCRIPT_ExtractPBO(const vector<string> &arg) 
{
	// Verify source argument	
	string source = "";
	
	if (arg.size() >= 2)
		source = arg[1];
	else
		return ErrorMessage(STR_ERROR_NO_FILE);
		
	if (!VerifyPath(source))
		return ErrorMessage(STR_UNPACKPBO_SRC_PATH_ERROR);
	
	if (GetFileExtension(source) != "pbo")
		return ErrorMessage(STR_PBO_NAME_ERROR);
	
	bool game_dir = false;
	
	if (Equals(source.substr(0,7),"<game>\\")) {
		global.last_pbo_file = "";
		source               = source.substr(7);
		game_dir             = true;
	} else {
		global.last_pbo_file = global.current_mod_new_name + "\\" + source.substr(0, source.length()-4);
		source               = global.current_mod_new_name + "\\" + source;
	}


	// Verify destination argument
	string destination = "";
	
	if (arg.size() >= 3) {
		destination = arg[2];

		if (!VerifyPath(destination))
			return ErrorMessage(STR_UNPACKPBO_DST_PATH_ERROR);
	}
	
	// Process optional 2nd argument: extraction destination
	if (!destination.empty()  ||  game_dir) {
		if (destination == ".")
			destination = "";

		int result = MakeDir(global.current_mod_new_name + "\\" + destination);

		if (result != 0)
			return result;

		// Create path to the extracted directory for use with MakePbo function
		global.last_pbo_file = global.current_mod_new_name + "\\" + destination + "\\" + PathLastItem(source.substr(0, source.length()-4));
		destination          = global.working_directory    + "\\" + global.current_mod_new_name + "\\" + destination;
		
		if (destination.substr(destination.length()-1) != "\\")
			destination += "\\";
	}

	return ExtractPBO(source, destination);
}


int SCRIPT_EditLine(const vector<string> &arg) 
{
	WriteProgressFile(INSTALL_PROGRESS, global.lang[STR_ACTION_EDITING]+"...");

	if (arg.size() < 3)
		return ErrorMessage(STR_ERROR_ARG_COUNT);
		
	vector<string> arg2;
	arg2.push_back("");
	arg2.push_back("");
	arg2.push_back("");

	bool insert = false;
	bool create = false;
	bool append = false;

	for (int i=1, j=0;  i<arg.size();  i++) {
		if (Equals(arg[i], "/insert")) {
			insert = true;
			continue;
		}
		
		if (Equals(arg[i], "/newfile")) {
			create = true;
			continue;
		}
		
		if (Equals(arg[i], "/append")) {
			append = true;
			continue;
		}

		if (j<3  &&  arg2[j].empty())
			arg2[j++] = arg[i];
	}
	

	string file_name = arg2[0];	
	int wanted_line  = atoi(arg2[1].c_str());
	
	if (Equals(file_name,"<download>")  ||  Equals(file_name,"<dl>"))
		file_name = "fwatch\\tmp\\" + global.downloaded_filename;
	else 
		file_name = global.current_mod_new_name + "\\" + file_name;

	if (file_name.empty())
		return ErrorMessage(STR_ERROR_NO_FILE);

	if (!VerifyPath(file_name))
		return ErrorMessage(STR_ERROR_PATH);

	vector<string> contents;
    fstream file;
	int line_number = 0;
	bool ends_with_newline = true;
    
    if (!create) {
    	global.logfile << "Editing line " << wanted_line << " in " << file_name << endl;
    	
    	file.open(file_name.c_str(), ios::in);
		if (file.is_open()) {
			string line;
		
			while (getline(file, line)) {
				line_number++;
				
				if (file.eof())
					ends_with_newline = false;
				
				if (line_number == wanted_line) {					
					string new_line = append ? line+arg2[2] : arg2[2];
				
					contents.push_back(new_line);
					
					if (insert) {
						contents.push_back(line);
						line_number++;
					}
				} else 
					contents.push_back(line);
			}
			
			if (insert && (wanted_line==0 || wanted_line > line_number)) {
				contents.push_back(arg2[2]);
				line_number++;
			}
			
			file.close();
		} else 
			return ErrorMessage(STR_EDIT_READ_ERROR);
	} else {
		global.logfile << "Creating new file " << file_name << endl;
		contents.push_back(arg2[2]);
		
		// Trash the file
		int buffer_pos  = 0;
		int buffer_size = file_name.length() + 3;
		char *file_list = new char[buffer_size*2];

		if (file_list) {
			wstring file_name_wide = string2wide(file_name);
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
	file.open(file_name.c_str(), ios::out | ios::trunc);
	if (file.is_open()) {
		for (int i=0; i<contents.size(); i++) {
			file << contents[i];
			
			if (i+1 < line_number || i+1==line_number && ends_with_newline)
				file << endl;
		}

		file.close();
	} else
		return ErrorMessage(STR_EDIT_WRITE_ERROR);

    ChangeFileDate(file_name);

	return NO_ERRORS;
}


int SCRIPT_StartVersion(const vector<string> &arg) 
{
	if (arg.size() < 3)
		return ErrorMessage(STR_ERROR_INVALID_SCRIPT, "%STR%", SCRIPT_ERROR);

	if (!global.current_mod_version.empty())
		EndModVersion();
	
	global.current_mod_version      = arg[1];
	global.current_mod_version_date = atoi(arg[2].c_str());
	global.command_line_num         = 0;
	return NO_ERRORS;
}


int SCRIPT_AutoInstall(const vector<string> &arg)
{
	string password = "";
	vector<string> Download_Arguments;

	for (int i=0, j=0;  i<arg.size();  i++) {
		if (Equals(arg[i].substr(0,10), "/password:")) {
			password = arg[i].substr(10);
			continue;
		}

		Download_Arguments.push_back(arg[i]);
	}

	int result = Download_Wrapper(Download_Arguments);
	
	if (result != NO_ERRORS)
		return result;

	global.logfile << "Auto installation" << endl;
	return Auto_Install(global.downloaded_filename, password);
}


int SCRIPT_Alias(const vector<string> &arg)
{
	if (arg.size() == 0)
		global.current_mod_alias.clear();
	else 
		for(int i=0; i<arg.size(); i++)
			global.current_mod_alias.push_back(arg[i]);

	return NO_ERRORS;
}
// -------------------------------------------------------------------------------------------------------



















	// Main Program

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
			"Failed to rename",
			"to",
			"New file name contains slashes",
			"Wildcards in the path",
			"Missing new file name",
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
			"В версии",		//24
			"В строке",		//25
			"Ошибка при запуске",		//26
			"Недостаточно аргументов функции",		//27
			"Ошибка при создании списка файлов в папке ",		//28
			"Отсутствует имя файла аргумента",		//29
			"Путь не соответствует текущей папке",		//30
			"Неверный скрипт установки",		//31
			"Неверные аргументы мастера установки",		//32
			"Ошибка при выделении понятий",		//33
			"осталось",		//34
			"всего",		//35
			"Неверный путь установки",		//36
			"Ошибка при загрузке",		//37
			"Совпадений не найдено",		//38
			"удалите этот файл и загрузите снова",		//39
			"Ошибка при извлечении",		//40
			"Ошибка при создании папки",		//41
			"Ошибка при считывании атрибутов файла",		//42
			"Исходный путь не соответствует текущей папке",		//43
			"Путь назначения не соответствует текущей папке",		//44
			"Невозможно переместить файлы из папки игры",		//45
			"Ошибка при переименовании файла",		//46
			"в",		//47
			"Новое имя файла содержит слеши",		//48
			"Путь содержит символы подстановки",		//49
			"Отсутствует новое имя файла",		//50
			"Необходимо запустить вручную",		//51
			"Необходимо загрузить вручную",		//52
			"Выберите папку с загруженным файлом",		//53
			"Отсутствует номер версии",		//54
			"Не файл типа PBO",		//55
			"Ошибка при создании файла PBO",		//56
			"Ошибка при извлечении из архива PBO",		//57
			"Ошибка при считывании",		//58
			"Ошибка при создании файла",	//59
			"Считывание mission.sqm"		//60
		}
	};
	
	global.lang_eng = stringtable[0];
	global.lang     = stringtable[0];
	
	if (Equals(global.arguments_table["language"],"Russian"))
		global.lang = stringtable[1];


	
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
		return LOGFILE_ERROR;
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
	struct {
		string name;
		int (*execute)(const vector<string> &arg);
	} 
	Commands[] = {
		"auto_install",		SCRIPT_AutoInstall,
		"download",			SCRIPT_Download,
		"get", 				SCRIPT_Download,
		"unpack", 			SCRIPT_Unpack,
		"extract", 			SCRIPT_Unpack,
		"move", 			SCRIPT_Move,
		"copy", 			SCRIPT_Move,
		"makedir",			SCRIPT_MakeDir,
		"newfolder", 		SCRIPT_MakeDir,
		"ask_run", 			SCRIPT_RequestExecution,
		"ask_execute", 		SCRIPT_RequestExecution,
		"begin_mod",		SCRIPT_StartMod,
		"delete",			SCRIPT_Delete,
		"remove",			SCRIPT_Delete,
		"rename",			SCRIPT_Rename,
		"ask_download",		SCRIPT_RequestDownload,
		"ask_get",			SCRIPT_RequestDownload,
		"if_version",		SCRIPT_If_version,
		"else",				SCRIPT_Else,
		"endif",			SCRIPT_Endif,
		"makepbo",			SCRIPT_MakePBO,
		"extractpbo",		SCRIPT_ExtractPBO,
		"unpackpbo",		SCRIPT_ExtractPBO,
		"unpbo",			SCRIPT_ExtractPBO,
		"edit",				SCRIPT_EditLine,
		"begin_ver",		SCRIPT_StartVersion,
		"alias",			SCRIPT_Alias
	};

	int number_of_commands = sizeof(Commands) / sizeof(Commands[0]);




	// Download installation script
	if (!global.arguments_table["downloadscript"].empty()) {
		WriteProgressFile(INSTALL_PROGRESS, global.lang[STR_ACTION_GETSCRIPT]);

		string url = "";
		fstream url_file;

		url_file.open(global.arguments_table["downloadscript"].c_str(), ios::in);
		if (url_file.is_open()) {
			getline(url_file, url);
			url_file.close();
		};

		url       += " --verbose \"--output-document=fwatch\\tmp\\installation script\"";
		int result = Download(url, OVERWRITE | SILENT_MODE);
		DeleteFile(global.arguments_table["downloadscript"].c_str());

		if (result > 0) {
			global.logfile << "\n--------------\n\n";
			global.logfile.close();
			return NO_SCRIPT;
		}
	}
	
	// Open script file and store lines in a vector
	WriteProgressFile(INSTALL_PROGRESS, global.lang[STR_ACTION_READSCRIPT]);

    vector<string> instructions;
    fstream script_file;
    string script_file_name = global.test_mode ? "fwatch\\data\\addonInstaller_test.txt" : "fwatch\\tmp\\installation script";

	script_file.open(script_file_name.c_str(), ios::in);

	if (script_file.is_open()) {
		string script_line;

		while(getline(script_file, script_line)) {
			script_line = Trim(script_line);
			
			instructions.push_back(script_line);
			
			// Do preliminary parsing to determine number of steps
			vector<string> command_arguments;
			Tokenize(script_line, " \t\r\n", command_arguments, CUSTOM_DELIMITERS);
			
			if (command_arguments.size() == 0)
				continue;
	
			// If url then keep j==0 which is auto installation; otherwise search for a command
			int j = 0;
			if (!IsURL(script_line))
				for (; j<number_of_commands; j++)
					if (Equals(command_arguments[0], Commands[j].name))
						break;
									
			if (j == number_of_commands)
				continue;
				
			// if modfolder wasn't formally started OR skipping this mod
			if ((global.current_mod.empty() || global.skip_modfolder)  &&  Commands[j].execute!=SCRIPT_StartMod)
				continue;
				
			// if version wasn't formally started
			if (!global.current_mod.empty()  &&  global.current_mod_version.empty()  &&  Commands[j].execute!=SCRIPT_StartVersion)
				continue;
	
			// if inside condition block
			if (global.game_ver_index >= 0  &&  !global.game_ver_cond[global.game_ver_index]  &&  Commands[j].execute!=SCRIPT_StartMod  &&  Commands[j].execute!=SCRIPT_StartVersion  &&  Commands[j].execute!=SCRIPT_If_version  &&  Commands[j].execute!=SCRIPT_Else  &&  Commands[j].execute!=SCRIPT_Endif)
				continue;
	
			// /mirror switch enables error fallthrough
			bool is_mirror = false;
			for (int k=0;  k<command_arguments.size();  k++)
				if (Equals(command_arguments[k].substr(0,10), "/mirror")) {
					is_mirror = true;
					break;
				}
					
			if (is_mirror)
				continue;
		
			if (Commands[j].execute==SCRIPT_StartMod)
				global.current_mod = "parsetest";
			else 
			if (Commands[j].execute==SCRIPT_StartVersion)
				global.current_mod_version = "-1";
			else
			if (Commands[j].execute==SCRIPT_If_version || Commands[j].execute==SCRIPT_Else || Commands[j].execute==SCRIPT_Endif)
				Commands[j].execute(command_arguments);
			else
				global.instructions_max++;
		}
		
		script_file.close();
		
		if (!global.test_mode)
			DeleteFile(script_file_name.c_str());
			
		// Reset variables
		global.current_mod         = "";
		global.current_mod_version = "";
		global.game_ver_index      = -1;
		global.game_ver_cond.clear();
		global.current_mod_alias.clear();
		
		if (global.test_mode) {
			global.current_mod              = global.arguments_table["testmod"];
			global.current_mod_version      = "1";
			global.current_mod_version_date = time(0);
			
			if (global.arguments_table["testdir"].empty())
				global.current_mod_new_name = global.arguments_table["testmod"];
			else
				global.current_mod_new_name = global.arguments_table["testdir"];
		}
	} else {
		global.logfile << "Failed to open " << script_file_name << "\n\n--------------\n\n";
		WriteProgressFile(INSTALL_ERROR, (global.lang[STR_ERROR]+"\\n"+global.lang[STR_ERROR_READSCRIPT]));
		global.logfile.close();
		return NO_SCRIPT;
	}

	







	int result = 0;

	// Parse stored text
	for (int i=0;  i<instructions.size(); i++) {
		if (Abort())
			return USER_ABORTED;

		global.command_line_num++;
		global.command_line     = instructions[i];
		bool manual_install     = false;
		global.unpack_set_error = true;

		vector<string> command_arguments;
		Tokenize(instructions[i], " \t\r\n", command_arguments, CUSTOM_DELIMITERS);
		
		if (command_arguments.size() == 0)
			continue;

		// If url then keep j==0 which is auto installation; otherwise search for a command
		int j = 0;
		if (!IsURL(instructions[i]))
			for (; j<number_of_commands; j++)
				if (Equals(command_arguments[0], Commands[j].name))
					break;

		if (j == number_of_commands)
			continue;
			

		// if modfolder wasn't formally started OR skipping this mod
		if ((global.current_mod.empty() || global.skip_modfolder)  &&  Commands[j].execute!=SCRIPT_StartMod)
			continue;
			
		// if version wasn't formally started
		if (!global.current_mod.empty()  &&  global.current_mod_version.empty()  &&  Commands[j].execute!=SCRIPT_StartVersion)
			continue;

		// if inside condition block
		if (global.game_ver_index >= 0  &&  !global.game_ver_cond[global.game_ver_index]  &&  Commands[j].execute!=SCRIPT_StartMod  &&  Commands[j].execute!=SCRIPT_StartVersion  &&  Commands[j].execute!=SCRIPT_If_version  &&  Commands[j].execute!=SCRIPT_Else  &&  Commands[j].execute!=SCRIPT_Endif)
			continue;


		// /mirror switch enables error fallthrough
		bool is_mirror = false;
		for (int k=0;  k<command_arguments.size();  k++)
			if (Equals(command_arguments[k].substr(0,10), "/mirror")) {
				is_mirror = true;
				command_arguments.erase(command_arguments.begin() + k);
				break;
			}
				
		if (global.mirror == SKIP_REMAINING) {
			global.mirror = is_mirror ? SKIP_REMAINING : DISABLED;
			continue;
		} else
			global.mirror = is_mirror ? ENABLED : DISABLED;

		if (Commands[j].execute!=SCRIPT_StartMod  &&  Commands[j].execute!=SCRIPT_StartVersion  &&  Commands[j].execute!=SCRIPT_If_version  &&  Commands[j].execute!=SCRIPT_Else  &&  Commands[j].execute!=SCRIPT_Endif)
			global.instructions_pos++;
		
		result = Commands[j].execute(command_arguments);
		
		if (result == USER_ABORTED)
			return result;
		else
			if (result != NO_ERRORS  &&  global.mirror==DISABLED) {
				global.logfile << "Installation error - aborting\n\n--------------\n\n";
				global.logfile.close();
				return result;
			}
			
		if (global.mirror == ENABLED)
			if (result == NO_ERRORS)
				global.mirror = SKIP_REMAINING;
			else
				global.instructions_pos--;
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
		else {
			int errorCode   = GetLastError();
			string errorMSG = FormatError(errorCode);
			global.logfile << "Failed to launch gameRestart.exe " << errorMSG << endl;
		}
	 
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	
	global.logfile << "\n--------------\n\n";
	global.logfile.close();
	
	

	// Close listen thread
	global.end_thread = true;
	Sleep(300);
	
    return NO_ERRORS;
}
