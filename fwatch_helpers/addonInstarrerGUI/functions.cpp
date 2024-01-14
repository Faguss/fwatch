#include "stdafx.h"
#include "resource.h"
#include "common.h"
#include "functions.h"

// String operations -------------------------------------------------------------------------------------

std::wstring utf16(std::string input)
{
	// https://stackoverflow.com/questions/6691555/converting-narrow-std::string-to-wide-std::string

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
	// https://mariusbancila.ro/blog/2008/10/20/writing-utf-8-files-in-c/
	
	if (input_size == 0)
		return std::string();
	
	int output_size = WideCharToMultiByte(CP_UTF8, 0, input, input_size, NULL, 0, NULL, NULL);

	std::string output(output_size, '\0');
	WideCharToMultiByte(CP_UTF8, 0, input, input_size, const_cast<char*>(output.c_str()), output_size, NULL, NULL);

	return output;
}

std::string utf8(std::wstring input)
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

std::string Trim(std::string s)
{
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(isspace))));
	s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(isspace))).base(), s.end());
	return s;
}

std::wstring Trim(std::wstring s)
{
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(isspace))));
	s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(isspace))).base(), s.end());
	return s;
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

std::wstring MaskNewName(std::wstring path, std::wstring mask) 
{
	// https://superuser.com/questions/475874/how-does-the-windows-rename-command-interpret-wildcards	https://superuser.com/a/739718
	
	if (mask.empty())
		return path;
	
	if (path.empty())
		return L"";
		
	size_t x       = 0;
	std::wstring R = L"";
	
	for (size_t m=0; m<mask.length(); m++) {
		wchar_t ch    = mask[m];
		bool q_exists = x<path.length();
		wchar_t q     = q_exists          ? path[x]   : L' ';
		wchar_t z     = m<mask.length()-1 ? mask[m+1] : L' ';
		
		if (ch!=L'.'  &&  ch!=L'*'  &&  ch!=L'?') {
			if (q_exists  &&  q!=L'.')
				x++;
			R += ch;
        } else if (ch == L'?') {
            if (q_exists  &&  q!=L'.') {
				R += q;
				x++;
			}
        } else if (ch == L'*'   &&   m == mask.length()-1) {
            while (x < path.length()) 
				R += path[x++];
        } else if (ch == L'*') {
            if (z == L'.') {
                size_t i = path.find_last_of(L'.');
						
				if (i == std::string::npos) {
                    R += path.substr(x, path.length()) + L'.';
                    i  = path.length();
                } else {
					R += path.substr(x, i - x + 1);
					x = i + 1;
					m++;
				}
				
            } else if (z == L'?') {
                R += path.substr(x, path.length());
				m++;
				x  = path.length();
            } else {
                size_t i = path.find_last_of(z);
						
				if (i == std::wstring::npos) {
					R += path.substr(x, path.length()) + z;
					x  = path.length();
					m++;
				} else {
					R += path.substr(x, i - x);
					x  = i + 1;
				}
            }
        } else if (ch == L'.') {
            while (x < path.length()) 
				if (path[x++] == L'.') 
					break;
					
            R += L'.';
        }
    }
	
    while (R[R.length() - 1] == L'.') 
		R = R.substr(0, R.length() - 1);
		
	return R;
}

std::wstring PathLastItem(std::wstring path) 
{
	size_t lastSlash = path.find_last_of(L"\\/");

	if (lastSlash != std::wstring::npos)
		return path.substr(lastSlash+1);
	else
		return path;
}

std::wstring PathNoLastItem(std::wstring path, int options) 
{
	size_t find = path.find_last_of(L"\\/");

	if (find != std::wstring::npos)
		return path.substr(0, find+(options & FLAG_NO_END_SLASH ? 0 : 1));
	else
		return L"";
}

std::wstring FormatError(DWORD error)
{
	if (error == ERROR_SUCCESS) 
		return L"";

	LPTSTR errorText = NULL;

	FormatMessage(
	FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
	NULL,
	error, 
	MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
	(LPTSTR)&errorText,
	0,
	NULL);

	std::wstring ret = Trim(L" - " + (std::wstring)(wchar_t*)errorText);

	if (errorText)
		LocalFree(errorText);

	return ret;
}

void Tokenize(std::string text, std::string delimiter, std::vector<std::string> &container)
{
	bool inQuote      = false;
	size_t word_start = 0;
	bool word_started = false;
	
	// Split line into parts
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
			container.push_back(UnQuote(text.substr(word_start, pos-word_start)));
			word_started = false;
		}
	}
}

void Tokenize(std::wstring text, std::wstring delimiter, std::vector<std::wstring> &container)
{
	bool inQuote      = false;
	size_t word_start = 0;
	bool word_started = false;
	
	// Split line into parts
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

std::wstring ReplaceAll(std::wstring str, const std::wstring& from, const std::wstring& to) 
{
    // http://stackoverflow.com/a/3418285
	
	if (from.empty())
        return str;
        
    size_t start_pos = 0;
    
    while ((start_pos = str.find(from, start_pos)) != std::wstring::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
    
    return str;
}

bool Equals(const std::string& a, const std::string& b) 
{
    // https://stackoverflow.com/questions/11635/case-insensitive-std::string-comparison-in-c#315463
	
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

bool VerifyPath(std::wstring path)
{
	std::vector<std::wstring> directories;
	Tokenize(path, L"\\/", directories);

	// Path cannot go back to the parent directory
	for (size_t i=0; i<directories.size(); i++)
		if (directories[i] == L"..")
			return false;

	return true;
}

std::wstring Int2StrW(int num, bool leading_zero)
{
	const int buffer_size       = 256;
	wchar_t buffer[buffer_size] = L"";
	wchar_t *buffer_ptr         = buffer;
	
	swprintf_s(buffer_ptr, buffer_size, L"%s%d", (leading_zero && num < 10 ? L"0" : L""), num);
	return (std::wstring)buffer;
}

std::wstring UInt2StrW(size_t num)
{
	const int buffer_size       = 16;
	wchar_t buffer[buffer_size] = L"";
	swprintf(buffer, buffer_size, L"%u", num);
	return (std::wstring)buffer;
}

std::wstring Double2StrW(double num)
{
	const int buffer_size       = 16;
	wchar_t buffer[buffer_size] = L"";
	swprintf(buffer, buffer_size, L"%g", num);
	return (std::wstring)buffer;
}

WORD Str2Short(std::string num)
{
	std::stringstream sstream(num);
	WORD result;
    sstream >> result;
    return result;
}

bool IsURL(std::wstring text)
{
	return (
		Equals(text.substr(0,7),L"http://")  ||  
		Equals(text.substr(0,8),L"https://")  ||  
		Equals(text.substr(0,6),L"ftp://")  ||  
		Equals(text.substr(0,4),L"www.")
	);
}

bool IsModName(std::wstring filename)
{
	if (Equals(filename,global.current_mod))
		return true;
		
	for (size_t i=0; i<global.current_mod_alias.size(); i++)
		if (Equals(filename,global.current_mod_alias[i]))
			return true;
			
	return false;
}

std::wstring GetTextBetween(std::wstring &buffer, std::wstring start, std::wstring end, size_t &offset, bool reverse)
{
	std::wstring out = L"";
	size_t pos0      = buffer.find(start, offset);
	
	if (!reverse) {
		if (pos0 != std::wstring::npos) {
			size_t pos1 = pos0 + start.length();
			size_t pos2 = buffer.find(end, pos1);
			
			if (pos2 != std::wstring::npos) {
				offset = pos1;
				out    = buffer.substr(pos1, pos2-pos1);
			}
		}		
	} else {
		if (pos0 != std::wstring::npos) {
			size_t pos1 = buffer.rfind(end, pos0);
			
			if (pos1 != std::wstring::npos) {
				offset      = pos0 + start.length();
				size_t pos2 = pos1 + end.length();
				out         = buffer.substr(pos2, pos0-pos2);
			}
		}			
	}

	return out;
}

std::wstring UrlEncode(const std::wstring &value) 
{
	// https://stackoverflow.com/questions/154536/encode-decode-urls-in-c
	
	std::wstring output         = L"";
	const int buffer_size       = 8;
	wchar_t buffer[buffer_size] = L"";
	
	for (std::wstring::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
		std::wstring::value_type c = (*i);

		if (iswalnum(c) || c == L'-' || c == L'_' || c == L'.' || c == L'~') {
			output += c;
		} else {
			swprintf(buffer, buffer_size, L"%c%x", L'%', c);
			output += buffer;
		}
	}

	return output;
}

std::wstring Lowercase(std::wstring &input)
{
	std::wstring output = L"";
	for (std::wstring::const_iterator i = input.begin(), n = input.end(); i != n; ++i)
		output += towlower((*i));
	return output;
}

std::wstring GetFileExtension(std::wstring file_name)
{
	size_t last_dot = file_name.find_last_of(L'.');
	
	if (last_dot == std::wstring::npos || last_dot == file_name.size()-1)
		return L"";

	file_name                   = Lowercase(file_name);
	std::wstring file_extension = file_name.substr(last_dot+1);
	bool is_number              = true;

	for (std::wstring::const_iterator i = file_extension.begin(), n = file_extension.end(); i != n; ++i)
		if (!iswdigit((*i))) {
			is_number = false;
			break;
		}
	
	// If extension is a number then it's a wget backup - find real extension
	if (is_number) {
		size_t second_last_dot = file_name.find_last_of(L'.', last_dot-1);
		
		if (second_last_dot != std::wstring::npos)
			file_extension = file_name.substr( second_last_dot+1, last_dot-second_last_dot-1 );
	}
	
	return file_extension;
}

wchar_t VerifyWindowsFileName(std::wstring &file_name)
{
	std::wstring forbidden = L"<>:\"\\/|?*";

	for (size_t i=0; i<forbidden.size(); i++) {
		size_t character = file_name.find(forbidden[i]);

		if (character != std::wstring::npos) {
			return forbidden[i];
		}
	}

	return NULL;
}

SYSTEMTIME StringToSystemTime(std::wstring input)
{
	SYSTEMTIME output = {0};
	std::vector<std::string> date_item;
	Tokenize(utf8(input), "-T:+ ", date_item);

	if (date_item.size() == 1) {
		FILETIME ft       = {0};
		LONGLONG ll       = Int32x32To64(_wtoi(input.c_str()), 10000000) + 116444736000000000;
		ft.dwLowDateTime  = (DWORD)ll;
		ft.dwHighDateTime = (DWORD)(ll >> 32);
		FileTimeToSystemTime(&ft, &output);
	} else {
		while(date_item.size() < 6)
			date_item.push_back("0");
			
		output.wYear         = Str2Short(date_item[0].c_str());
		output.wMonth        = Str2Short(date_item[1].c_str());
		output.wDay          = Str2Short(date_item[2].c_str());
		output.wHour         = Str2Short(date_item[3].c_str());
		output.wMinute       = Str2Short(date_item[4].c_str());
		output.wSecond       = Str2Short(date_item[5].c_str());
		output.wMilliseconds = 0;
	}

	return output;
}

std::wstring SystemTimeToReadableDate(SYSTEMTIME st)
{
	return  
		Int2StrW(st.wYear) + L"." + 
		Int2StrW(st.wMonth, LEADING_ZERO) + L"." + 
		Int2StrW(st.wDay, LEADING_ZERO) + L"  " + 
		Int2StrW(st.wHour, LEADING_ZERO) + L":" + 
		Int2StrW(st.wMinute, LEADING_ZERO) + L":" + 
		Int2StrW(st.wSecond, LEADING_ZERO);
}
// -------------------------------------------------------------------------------------------------------




// Installer operations ----------------------------------------------------------------------------------

	// Feedback for the game
void WriteProgressFile(INSTALLER_STATUS status, std::wstring input)
{	
	size_t installation_steps_current = global.instruction_index < global.commands.size() ? global.commands[global.instruction_index].step_num : global.installation_steps_max;

	std::wstring buffer = input;
	
	if (status==INSTALL_PROGRESS  &&  global.installation_steps_max>0)
		buffer = global.lang[STR_PROGRESS] + L" " + UInt2StrW(installation_steps_current) + L"/" + Int2StrW(global.installation_steps_max) + L"\r\n\r\n" + buffer;

	global.last_log_message = buffer;
	EditMultilineUpdateText(global.controls[LOG_DETAIL], buffer);

	if (!global.test_mode) {
		std::ofstream progressLog;
		progressLog.open("fwatch\\tmp\\schedule\\install_progress.sqf", std::ios::out | std::ios::trunc);

		if (progressLog.is_open()) {
			buffer = ReplaceAll(input, L"\r\n", L"\\n");

			if (status==INSTALL_PROGRESS  &&  installation_steps_current>0  &&  global.installation_steps_max>0)
				buffer = global.lang[STR_PROGRESS] + L" " + UInt2StrW(installation_steps_current) + L"/" + Int2StrW(global.installation_steps_max) + L"\\n\\n" + buffer;

			buffer = ReplaceAll(buffer, L"\"", L"'");

			progressLog << "_auto_restart=" << (global.restart_game ? "true" : "false") 
						<< ";_run_voice_program=" << (global.run_voice_program ? "true" : "false")
						<< ";_install_status=" << status << ";\"";

			// Convert message in the buffer from utf16 to Windows Code Page so that the game can read it
			for (size_t i=0; i<buffer.length(); i++) {
				unsigned char wcp;

				switch(buffer[i]) {
					//polish
					case 0x104 : wcp=0xA5; break; //Ą
					case 0x105 : wcp=0xB9; break; //ą
					case 0x106 : wcp=0xC6; break; //Ć
					case 0x107 : wcp=0xE6; break; //ć
					case 0x118 : wcp=0xCA; break; //Ę
					case 0x119 : wcp=0xEA; break; //ę
					case 0x141 : wcp=0xA3; break; //Ł
					case 0x142 : wcp=0xB3; break; //ł
					case 0x143 : wcp=0xD1; break; //Ń
					case 0x144 : wcp=0xF1; break; //ń
					case 0x15A : wcp=0x8C; break; //Ś
					case 0x15B : wcp=0x9C; break; //ś
					case 0x179 : wcp=0x8F; break; //Ź
					case 0x17A : wcp=0x9F; break; //ź
					case 0x17B : wcp=0xAF; break; //Ż
					case 0x17C : wcp=0xBF; break; //ż

					default : 
						if (buffer[i] >= 0x410 && buffer[i] <= 0x44F)	//cyrillic
							wcp = (unsigned char)(buffer[i] - 0x350);
						else
							if (buffer[i] <= 0xFF) //ascii
								wcp = (unsigned char)buffer[i];
							else
								wcp = 0x3F; //unknown
				}

				progressLog << wcp;
			}
				
			progressLog << "\"";
			progressLog.close();
		}
	}
}

	// Write mod identification file
int WriteModID(std::wstring input_path, std::wstring mod, std::wstring content, std::wstring content2)
{
	std::ofstream ID_file;
	std::wstring path = input_path + L"\\__gs_id";
	ID_file.open(path.c_str(), std::ios::out | std::ios::trunc);

	if (ID_file.is_open()) {
		SYSTEMTIME st;
		GetLocalTime(&st);
		TIME_ZONE_INFORMATION TimeZoneInfo;
		GetTimeZoneInformation (&TimeZoneInfo);
		const int buffer_size          = 128;
		char current_date[buffer_size] = "";
		sprintf_s(current_date, buffer_size, ";[%d,%d,%d,%d,%d,%d,%d,%d,%d,false]",
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
			
		ID_file << utf8(content) << current_date << ";" << utf8(mod) << ";" << utf8(content2);
		ID_file.close();
		return ERROR_NONE;
	} else
		return ERROR_LOGFILE;
}

	// Format error message
INSTALLER_ERROR_CODE ErrorMessage(int string_code, std::wstring message, INSTALLER_ERROR_CODE error_code) 
{
	if (global.current_mod==L"?pretendtoinstall" && global.current_mod_version==L"-1")
		return ERROR_NONE;

	INSTALLER_STATUS status       = global.last_download_attempt ? INSTALL_ERROR : INSTALL_PROGRESS;
	std::wstring message_eng      = ReplaceAll(message, L"%STR%", global.lang_eng[string_code]);
	std::wstring message_local    = ReplaceAll(message, L"%STR%", global.lang[string_code]);
	std::wstring message_complete = L"";

	// show which command failed
	if (error_code == ERROR_COMMAND_FAILED) {
		message_complete = global.lang[STR_ERROR] + L"\r\n" + global.current_mod;
		
		if (global.current_mod_version != L"")
			message_complete += L"\r\n" + global.lang[STR_ERROR_INVERSION] + L" " + global.current_mod_version;
		
		message_complete += L"\r\n" + global.lang[STR_ERROR_ONLINE] + L" " + Int2StrW(global.command_line_num) + L"\r\n" + (global.installation_phase==PHASE_DOWNLOADING ? L"\r\n" : global.commands[global.instruction_index].name+L"\r\n") + message_local;

		if (status == INSTALL_ERROR) {
			std::wstring str = L"ERROR " + global.current_mod;
			
			if (global.current_mod_version != L"")
				str += L" v" + global.current_mod_version;
			
			str += L" line " + Int2StrW(global.command_line_num);
			
			if (global.installation_phase != PHASE_DOWNLOADING)
				str += L": " + global.commands[global.instruction_index].name;
				
			str += L" - " + ReplaceAll(message_eng, L"\r\n", L" ");

			LogMessage(str);
		}
	}
	
	// just display input message
	if (error_code == ERROR_WRONG_SCRIPT) {
		message_complete = global.lang[STR_ERROR] + L"\r\n" + message_local;
		LogMessage(L"ERROR - " + ReplaceAll(message_eng, L"\r\n", L" "));
	}
	
	WriteProgressFile(status, message_complete);
	return error_code;
}

void LogMessage(std::wstring input, bool close)
{
	if (!input.empty())
		input += L"\r\n";
	
	if (close)
		input += L"\r\n--------------\r\n\r\n";
	
	if (global.logfile.is_open()) {
		if (!global.test_mode)
			global.logfile << utf8(input);

		if (close)
			global.logfile.close();
	}

	global.buffer_log += input;

	EditMultilineUpdateText(global.controls[LOG_GENERAL], global.buffer_log);
}

void LogMessageDate(bool start_date)
{
	SYSTEMTIME st;
	GetLocalTime(&st);

	if (start_date) {
		LogMessage(L"\r\n--------------\r\n\r\n" + SystemTimeToReadableDate(st));
		
		if (global.test_mode) {
			std::wstring msg = L"Test Mode - " + global.current_mod;
		
			if (!Equals(global.current_mod,global.current_mod_new_name))
				msg += L" as " + global.current_mod_new_name;

			LogMessage(msg);
		}
	} else {
		LogMessage(
			L"All done  " + 
			Int2StrW(st.wHour, LEADING_ZERO) + L":" + 
			Int2StrW(st.wMinute, LEADING_ZERO) + L":" + 
			Int2StrW(st.wSecond, LEADING_ZERO)
		);
	}
}

std::wstring GetFileContents(std::wstring filename)
{
	std::ifstream file(filename.c_str(), std::ios::in | std::ios::binary);
	std::string file_text((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	
	if (file.is_open())
		file.close();
	
	return utf16(file_text);
}

size_t ParseInstallationScript(std::wstring &script_file_content, std::vector<Command> &output, bool compare_old_with_new)
{
	std::vector<Command> new_script;
	global.commands_lines.clear();
	
	size_t word_begin          = 0;  //number of column where a phrase begins
	int word_count             = 1;  //number of found phrases in the current line
	int word_line_num          = 1;	 //line count for the entire script
	int word_line_num_local    = 1;  //line count for single version of the mod
	int command_id             = -1;
	int last_command_line_num  = -1;
	bool in_quote              = false;
	bool remove_quotes         = true;
	bool url_block             = false;
	bool url_line              = false;
	bool word_started          = false;
	bool last_url_list_started = false;

	for (size_t i=0; i<=script_file_content.length(); i++) {
		bool end_of_word = i == script_file_content.length() || iswspace(script_file_content[i]);
		
		// When quote
		if (script_file_content[i] == L'"')
			in_quote = !in_quote;
		
		// If beginning of an url block
		if (script_file_content[i] == L'{' && !word_started) {
			url_block = true;
	
			// if bracket is the first thing in the line then it's auto installation
			if (word_count == 1) {
				last_command_line_num = word_line_num;
				Command current_command   = {};
				current_command.id        = COMMAND_AUTO_INSTALL;
				current_command.ctrl_flow = false;
				current_command.disable   = false;
				current_command.line_num  = word_line_num_local;
				current_command.step_num  = 0;
				current_command.switches  = SWITCH_NONE;
				current_command.password  = L"";
				current_command.timestamp = L"";
				current_command.name      = L"auto_install";
				new_script.push_back(current_command);

				size_t end_of_line = script_file_content.find(L'\n', i);

				if (end_of_line == std::wstring::npos)
					end_of_line = script_file_content.length();
				else
					if (script_file_content[end_of_line-1] == L'\r')
						end_of_line--;

				global.commands_lines.push_back(Int2StrW(word_line_num_local) + L": " + script_file_content.substr(word_begin, end_of_line-word_begin));
			} else
				// if bracket is an argument for the command
				if (command_id != -1) {
					new_script[new_script.size()-1].arguments.push_back(L"<dl>");
				}
			
			continue;
		}
		
		// If ending of an url block
		if (script_file_content[i] == L'}' && url_block) {
			end_of_word = true;
			
			// If there's space between last word and the closing bracket
			if (!word_started) {	
				url_block = false;
				url_line  = false;
				word_count++;
				continue;
			}
		}
		
		// Remember beginning of the word
		if (!end_of_word && !word_started) {
			word_begin   = i;
			word_started = true;
			
			// If custom delimeter - jump to the end of the argument
			if (script_file_content.substr(word_begin,2) == L">>") {
				word_begin   += 3;
				size_t end    = script_file_content.find(script_file_content[i+2], i+3);
				end_of_word   = true;
				i             = end==std::wstring::npos ? script_file_content.length() : end;
				remove_quotes = false;
			}
		}
		
		// When hit end of the word
		if (end_of_word && word_started && !in_quote) {
			std::wstring word = script_file_content.substr(word_begin, i-word_begin);

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
					for (int j=0; match_command_name_to_id[j]!=COMMAND_MAX && command_id==-1; j++)
						if (Equals(word, command_names[j]))
							command_id = match_command_name_to_id[j];
				
				// If so then add it to database, otherwise skip this line
				if (command_id != -1) {
					last_command_line_num = word_line_num;

					Command current_command   = {};
					current_command.id        = (INSTALLER_COMMAND_ID)command_id;
					current_command.ctrl_flow = false;
					current_command.disable   = false;
					current_command.line_num  = word_line_num_local;
					current_command.step_num  = 0;
					current_command.switches  = SWITCH_NONE;
					current_command.password  = L"";
					current_command.timestamp = L"";
					current_command.name      = word;
					new_script.push_back(current_command);

					size_t end_of_line = script_file_content.find(L'\n', i);

					if (end_of_line == std::wstring::npos)
						end_of_line = script_file_content.length();
					else
						if (script_file_content[end_of_line-1] == L'\r')
							end_of_line--;

					global.commands_lines.push_back(Int2StrW(word_line_num_local) + L": " + script_file_content.substr(word_begin, end_of_line-word_begin));
					
					// Check if it's a control flow type of command
					for (int j=0; control_flow_commands[j]!=COMMAND_MAX; j++)
						if (command_id == control_flow_commands[j])
							new_script[new_script.size()-1].ctrl_flow = true;
					
					// If command is an URL then add it to the url database
					if (IsURL(word)) {
						url_line              = true;
						last_url_list_started = true;

						DownloadURL item = {0};
						item.line_num    = word_line_num;
						item.url         = word;
						new_script[new_script.size()-1].downloads.push_back(item);
					}
					
					if (command_id == COMMAND_BEGIN_MOD || command_id == COMMAND_BEGIN_VERSION)
						word_line_num_local = 0;
				} else {
					size_t end = script_file_content.find(L"\n", i);
					i          = (end==std::wstring::npos ? script_file_content.length() : end) - 1;
				}
			} else {
				// Check if URL starts here
				if (!url_line && command_id != COMMAND_ASK_DOWNLOAD)
					url_line = IsURL(word);
					
				// Check if it's a valid command switch
				bool is_switch           = false;
				size_t colon             = word.find(L":");
				std::wstring switch_name = word;
				std::wstring switch_arg  = L"";
				
				if (colon != std::wstring::npos) {
					switch_name = word.substr(0,colon+1);
					switch_arg  = word.substr(colon+1);
				}
				
				for (int switch_index=1, switch_enum=1; switch_enum<SWITCH_MAX && !is_switch; switch_enum*=2, switch_index++) {
					if (Equals(switch_name, command_switches_names[switch_index])) {
						is_switch = true;
						new_script[new_script.size()-1].switches |= switch_enum;
						
						if (switch_enum == SWITCH_PASSWORD)
							new_script[new_script.size()-1].password = switch_arg;
							
						if (switch_enum == SWITCH_TIMESTAMP)
							new_script[new_script.size()-1].timestamp = switch_arg;
					}
				}

				// Add word to the URL database or the arguments database
				if (!is_switch) {
					size_t last_com = new_script.size() - 1;

					if (url_line) {
						if (!last_url_list_started) {
							last_url_list_started = true;
							DownloadURL item = {0};
							item.url         = word;
							item.line_num    = word_line_num;
							new_script[last_com].downloads.push_back(item);
						} else {
							size_t last_url = new_script[last_com].downloads.size()-1;
							new_script[last_com].downloads[last_url].arguments.push_back(word);
						}
					} else {
						new_script[last_com].arguments.push_back(word);
					}
				}
			}
			
			// If ending of an url block
			if (script_file_content[i] == L'}' && url_block) {
				url_block = false;
				url_line  = false;
			}

			word_started = false;
			word_count++;
		}
		
		// When new line
		if (!in_quote && (script_file_content[i] == L'\n' || script_file_content[i] == L'\0')) {
			size_t last = new_script.size()-1;
			
			// Maintain minimal number of arguments
			while (!url_block && command_id != -1 && new_script[last].arguments.size() < command_minimal_arg[new_script[last].id])
				new_script[last].arguments.push_back(L"");
			
			word_count            = 1;
			url_line              = false;
			last_url_list_started = false;
			word_line_num++;
			word_line_num_local++;
		}
	}

	size_t new_instruction_index = 0;

	// Compare new installation with the old one so that installation can be continued from the last common point
	for (size_t i=0; compare_old_with_new && i<global.instruction_index && i<output.size() && i<new_script.size(); i++) {
		if (
			output[i].id != new_script[i].id || 
			output[i].switches != new_script[i].switches ||
			output[i].password != new_script[i].password ||
			output[i].timestamp != new_script[i].timestamp || 
			output[i].arguments.size() != new_script[i].arguments.size() ||
			output[i].downloads.size() != new_script[i].downloads.size()
		)
			break;

		bool match = true;
		for (size_t j=0; j<output[i].arguments.size() && match; j++)
			if (output[i].arguments[j] != new_script[i].arguments[j])
				match = false;

		for (size_t j=0; j<output[i].downloads.size() && match; j++) {
			if (output[i].downloads[j].url != new_script[i].downloads[j].url)
				match = false;

			for (size_t k=0; k<output[i].downloads[j].arguments.size() && match; k++)
				if (output[i].downloads[j].arguments[k] != new_script[i].downloads[j].arguments[k])
					match = false;
		}

		if (match)
			new_instruction_index = i+1;
	}

	output = new_script;
	global.installation_steps_max = 0;
	bool exit = false;
	ResetInstallationState();

	// Pretend to Install
	// Figure out how many steps this installation script has so later we can display progress for the user
	for (size_t i=0;  i<global.commands.size(); i++) {
		if (
			// if modfolder wasn't formally started OR skipping this mod
			((global.current_mod.empty() || global.skip_modfolder) && global.commands[i].id != COMMAND_BEGIN_MOD && global.commands[i].id != COMMAND_INSTALL_VERSION)
			||
			// if version wasn't formally started
			(!global.current_mod.empty() && global.current_mod_version.empty() && global.commands[i].id != COMMAND_BEGIN_VERSION && global.commands[i].id != COMMAND_INSTALL_VERSION)
			||
			// if inside condition block
			(global.condition_index >= 0 && !global.condition[global.condition_index] && !global.commands[i].ctrl_flow)
			||
			exit
		) {
			global.commands[i].disable = true;
			continue;
		}

		// Execute only control flow instructions
		switch(global.commands[i].id) {
			case COMMAND_INSTALL_VERSION : global.script_version=_wtof(global.commands[i].arguments[0].c_str()); break;
			case COMMAND_BEGIN_MOD       : global.current_mod=L"?pretendtoinstall"; break;
			case COMMAND_BEGIN_VERSION   : global.current_mod_version=L"-1"; break;
			case COMMAND_IF_VERSION      : global.commands[i].disable=true; Condition_If_version(global.commands[i].arguments); break;
			case COMMAND_ELSE            : global.commands[i].disable=true; Condition_Else(); break;
			case COMMAND_ENDIF           : global.commands[i].disable=true; Condition_Endif(); break;
			case COMMAND_EXIT            : exit = true;
			default                      : global.installation_steps_max++;
		}

		global.commands[i].step_num = global.installation_steps_max;
	}
	
	ResetInstallationState();
	return new_instruction_index;
}

	// Read wget log to get information about the download
int ParseWgetLog(std::string &error)
{
	std::fstream DownloadLog;
    DownloadLog.open("fwatch\\tmp\\schedule\\downloadLog.txt", std::ios::in);

	if (DownloadLog.is_open()) {
		std::string line                  = "";
		std::string filesize              = "";
		std::string size_downloaded       = "";
		std::string percentage_downloaded = "";
		std::string download_speed        = "";
		std::string time_remaining        = "";
		
		const int filename_messages_items = 4;
		std::string filename_messages[filename_messages_items][2] = {
			{"Saving to: 'fwatch/tmp/"                          , "'"},
			{") - 'fwatch/tmp/"                                 , "' saved ["},
			{"File 'fwatch/tmp/"                                , "' already there; not retrieving"},
			{"Server file no newer than local file 'fwatch/tmp/", "' -- not retrieving"}
		};
			
		const int error_messages_items = 2;
		std::string error_messages[error_messages_items] = {"failed","ERROR"};

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
				double size_num = atof(size_downloaded.c_str());
				
				if (size_num > 1024) {
					double megabytes = size_num / 1024;
					char temp[128]   = "";
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
						global.downloaded_filename = utf16(line.substr(begin+len,  end-(begin+len)));
						break;
					}
				}
			}

			// Get error message
			for (int i=0; i<error_messages_items; i++) {
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

		std::wstring tosave = global.lang[STR_ACTION_CONNECTING] + L"...";

		if (!size_downloaded.empty()) {
			tosave = global.lang[STR_ACTION_DOWNLOADING] + L"...\r\n" + 
					 global.downloaded_filename + L"\r\n\r\n" +
					 utf16(size_downloaded) + (filesize.empty() ? L"" : (L" / "+utf16(filesize)+L" - "+utf16(percentage_downloaded))) + L"\r\n" + 
					 utf16(download_speed) + L"/s" + L"\r\n" + 
					 (time_remaining.empty() ? L"" : (utf16(time_remaining) + L" " + global.lang[STR_DOWNLOAD_LEFT]));
					 
			if (percentage_downloaded == "100%")
				tosave = global.lang[STR_ACTION_DOWNLOADED] + L"\r\n" + 
						 global.downloaded_filename + L"\r\n\r\n" +
						 utf16((filesize.empty() ? size_downloaded : filesize)) + L"\r\n" + 
						 L"\r\n" + 
						 utf16(time_remaining) + L" " + global.lang[STR_DOWNLOAD_TOTAL];
		}

		WriteProgressFile(INSTALL_PROGRESS, tosave);
	} else
		return 1;
	
	return 0;
}

	// Read 7za log to get information about unpacking
int ParseUnpackLog(std::string &error, std::wstring &file_name)
{
	std::fstream UnpackLog;
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
				size_t percent_start = percent<3 ? 0 : percent-3;

				percentage  = Trim(text.substr(percent_start, percent-percent_start+1));
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

		std::wstring tosave = global.lang[STR_ACTION_EXTRACTING] + L"...\r\n" + file_name + L"\r\n" + utf16(percentage) + L"\r\n\r\n" + utf16(current_file);
		WriteProgressFile(INSTALL_PROGRESS, tosave);
	}

	return 0;
}

	// Read MakePbo/ExtractPBO log to get information about un/packing
int ParsePBOLog(std::string &message, std::wstring &exename, std::wstring &file_name)
{
	std::fstream PackLog;
	PackLog.open("fwatch\\tmp\\schedule\\PBOLog.txt", std::ios::in);

	std::wstring verb = L"";

	if (exename == L"ExtractPbo.exe")
		verb = global.lang[STR_ACTION_UNPACKINGPBO];

	if (exename == L"MakePbo.exe")
		verb = global.lang[STR_ACTION_PACKINGPBO];

	if (PackLog.is_open()) {
		std::string text = "";

		while(getline(PackLog, text)) {
			text = Trim(text);

			if (text.empty())
				continue;

			if (Equals(text.substr(0,4),"cwd="))
				message += "\r\n" + text;
			else
				message = text;

			if (message == "noisy extraction of specific files (eg lists them)")
				message = "command line syntax error";
		}

		std::wstring tosave = verb + L"...\r\n" + file_name + L"\r\n\r\n" + utf16(message);
		WriteProgressFile(INSTALL_PROGRESS, tosave);
	}

	return 0;
}

	// Detect mission type
std::wstring GetMissionDestinationFromSQM(std::wstring path)
{
	WriteProgressFile(INSTALL_PROGRESS, global.lang[STR_ACTION_READMISSIONSQM]);

	std::ifstream file(path.c_str(), std::ios::in);
	std::string text_buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	
	if (file.is_open())
		file.close();
	
	if (text_buffer.empty())
		return L"Addons";
		
	int expect           = SQM_PROPERTY;
	int class_level      = 0;
	int array_level      = 0;
	int player_count     = 0;
	size_t word_start    = 0;
	bool word_started    = false;
	bool is_array        = false;
	bool in_quote        = false;
	bool in_mission      = false;
	bool is_wizard       = false;
	char separator       = ' ';
	std::string property = "";
	
	for (size_t i=0; i<text_buffer.size(); i++) {
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
					if (!word_started) {
						word_started = true;
						word_start = i;
					}
				} else
					if (word_started) {
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

						word_started = false;
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

				if (!word_started) {
					if (!isspace(c)) {
						word_started = true;
						word_start = i;
					}
				} else {
					if (!in_quote && array_level==0 && (c==';' || c=='\r' || c=='\n')) {
						std::string word = text_buffer.substr(word_start, i-word_start);
						for (size_t j=0; j<word.size(); j++)
							word[j] = (char)tolower(word[j]);
						
						if (!is_wizard && Equals(property,"position[]") && word.find("wizvar_")!=std::string::npos)
							is_wizard = true;
				
						word_started = false;
						expect     = SQM_PROPERTY;
					}
				}
				
				break;
			}
			
			case SQM_CLASS_NAME : {
				if (isalnum(c) || c=='_') {
					if (!word_started) {
						word_start = i;
						word_started = true;
					}
				} else
					if (word_started) {
						if (text_buffer.substr(word_start, i-word_start) == "Mission")
							in_mission = true;

						word_started = false;
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
		return is_wizard ? L"Templates" : L"MPMissions";
	else
		return is_wizard ? L"SPTemplates" : L"Missions";
}

	// callback for BrowseFolder()
static int CALLBACK BrowseCallbackProc(HWND hwnd,UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	UNREFERENCED_PARAMETER(lParam);

	if (uMsg == BFFM_INITIALIZED)
		SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);

	return 0;
}

	// https://stackoverflow.com/questions/12034943/win32-select-directory-dialog-from-c-c
std::wstring BrowseFolder(std::wstring saved_path)
{
	wchar_t path[MAX_PATH];
	const wchar_t *path_param = saved_path.c_str();

	BROWSEINFO bi = { 0 };
	bi.lpszTitle  = (L"Browse for folder...");
	bi.ulFlags    = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
	bi.lpfn       = BrowseCallbackProc;
	bi.lParam     = (LPARAM)path_param;

	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);

	if (pidl) {
		//get the name of the folder and put it in path
		SHGetPathFromIDList(pidl, path);

		//free memory used
		IMalloc * imalloc = 0;
		if (SUCCEEDED( SHGetMalloc(&imalloc))) {
			imalloc->Free(pidl);
			imalloc->Release();
		}

		return path;
	}

	return L"";
}

	// Browse files in a given folder and return file list plus some other info
DIRECTORY_INFO ScanDirectory(std::wstring path) 
{
	DIRECTORY_INFO output           = {0};	
	output.error_code               = ERROR_NONE;
	output.number_of_files          = 0;
	output.number_of_dirs           = 0;
	output.number_of_wanted_mods    = 0;
	output.number_of_mod_subfolders = 0;
	std::vector<std::wstring> files;
	
	WIN32_FIND_DATAW FileInformation;
	std::wstring pattern = path + L"\\*";
	HANDLE hFile         = FindFirstFile(pattern.c_str(), &FileInformation);

	if (hFile != INVALID_HANDLE_VALUE) {
		do {
			if (wcscmp(FileInformation.cFileName,L".")==0  ||  wcscmp(FileInformation.cFileName,L"..")==0  ||  wcscmp(FileInformation.cFileName,L"$PLUGINSDIR")==0)
				continue;
			
			std::wstring file_name = (std::wstring)FileInformation.cFileName;
			bool is_mod            = false;
			int mod_sub_id         = DIR_NONE;
				
			if (FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				output.number_of_dirs++;
				is_mod = IsModName(file_name);
				
				if (is_mod)
					output.number_of_wanted_mods++;
				
				for (int i=DIR_ADDONS; i<DIR_MAX && mod_sub_id==DIR_NONE; i++)
					if (Equals(file_name,mod_subfolders[i])) {
						output.number_of_mod_subfolders++;
						mod_sub_id = i;
					}
					
				output.file_list.push_back(file_name);
				output.attributes_list.push_back(FileInformation.dwFileAttributes);
				output.is_mod_list.push_back(is_mod);
				output.mod_sub_id_list.push_back(mod_sub_id);
			} else {
				output.number_of_files++;
				files.push_back(file_name);
			}
		} while(FindNextFile(hFile, &FileInformation) == TRUE);
		
		// Files come after directories
		for (size_t i=0; i<files.size(); i++) {
			output.file_list.push_back(files[i]);
			output.attributes_list.push_back(0);
			output.is_mod_list.push_back(false);
			output.mod_sub_id_list.push_back(DIR_NONE);
		}
		
		FindClose(hFile);
	} else {
		DWORD error_code  = GetLastError();
		output.error_code = ErrorMessage(STR_ERROR_FILE_LIST, L"%STR% " + path + L" - " + UInt2StrW(error_code) + L" " + FormatError(error_code));
	}

	return output;
}

DWORD DeleteDirectory(const std::wstring &refcstrRootDirectory, bool bDeleteSubdirectories)
{
	// http://stackoverflow.com/a/10836193
	
	bool             bSubdirectory = false;       // Flag, indicating whether subdirectories have been found
	HANDLE           hFile;                       // Handle to directory
	std::wstring     strFilePath;                 // Filepath
	std::wstring     strPattern;                  // Pattern
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
					DWORD iRC = DeleteDirectory(strFilePath, bDeleteSubdirectories);
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
				if (refcstrRootDirectory != L"fwatch\\tmp\\_extracted" && refcstrRootDirectory != L"fwatch\\tmp\\_backup") {
					if (RemoveDirectoryW(refcstrRootDirectory.c_str()) == FALSE)
						return GetLastError();
				} else
					return ERROR_SUCCESS;
			}
		}
	}

	return ERROR_SUCCESS;
}

INSTALLER_ERROR_CODE CreateFileList(std::wstring source, std::wstring destination, std::vector<std::wstring> &sources, std::vector<std::wstring> &destinations, std::vector<bool> &dirs, int options, std::vector<std::wstring> &empty_dirs, int &recursion)
{
	WIN32_FIND_DATAW fd;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	hFind        = FindFirstFile(source.c_str(), &fd);
	INSTALLER_ERROR_CODE result = ERROR_NONE;

	if (hFind == INVALID_HANDLE_VALUE) {
		DWORD error_code = GetLastError();
		
		if ((options & FLAG_ALLOW_ERROR && error_code==ERROR_FILE_NOT_FOUND)  ||  error_code==ERROR_PATH_NOT_FOUND)
			return ERROR_NONE;

		return ErrorMessage(STR_ERROR_FILE_LIST, L"%STR% " + source + L"  - " + Int2StrW(error_code) + L" " + FormatError(error_code));
	}

	recursion++;
	std::wstring base_source      = PathNoLastItem(source);
	std::wstring base_destination = PathNoLastItem(destination);
	std::wstring new_name         = PathLastItem(destination);
	std::wstring face1            = global.current_mod_new_name + L"\\face.jpg";
	std::wstring face2            = global.current_mod_new_name + L"\\face.paa";
	std::wstring id_file          = global.current_mod_new_name + L"\\__gs_id";

	if (new_name.empty())
		new_name = PathLastItem(source);
		
	bool is_source_wildcard      = source.find(L"*")!=std::wstring::npos    ||  source.find(L"?")!=std::wstring::npos;
	bool is_destination_wildcard = new_name.find(L"*")!=std::wstring::npos  ||  new_name.find(L"?")!=std::wstring::npos;

	do {
		if (wcscmp(fd.cFileName,L".")==0  ||  wcscmp(fd.cFileName,L"..")==0)
			continue;
	    
		std::wstring file_name       = (std::wstring)fd.cFileName;
		std::wstring new_source      = base_source      + file_name;
		std::wstring new_destination = base_destination + (is_destination_wildcard ? MaskNewName(file_name,new_name) : new_name);
		bool is_dir                  = (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) > 0;
		DWORD attributes             = GetFileAttributes(new_destination.c_str());

		if (
			(is_dir && is_source_wildcard && ~options & FLAG_MATCH_DIRS)  ||
			(!is_dir && options & FLAG_MATCH_DIRS_ONLY)  ||  
			Equals(new_source,face1)  ||  
			Equals(new_source,face2)  ||  
			Equals(new_source,id_file)
		)
			continue;

		// Move modfolder to the game dir when using wildcards
		if (is_dir  &&  options & FLAG_MATCH_DIRS  &&  recursion==0  &&  Equals(destination,global.current_mod_new_name+L"\\")  &&  IsModName(file_name)) {
			new_destination = global.current_mod_new_name;
			attributes      = GetFileAttributes(new_destination.c_str());
		}

		// Check if destination directory already exists
		if (is_dir  &&  ((attributes != INVALID_FILE_ATTRIBUTES  &&  attributes & FILE_ATTRIBUTE_DIRECTORY)  ||  ~options & FLAG_MOVE_FILES)) {
			if (options & FLAG_MOVE_FILES)
				empty_dirs.push_back(new_source);
			else
				CreateDirectory(new_destination.c_str(), NULL);

			// If dir already exists then browse its contents
			new_source      += L"\\*";
			new_destination += L"\\";
			result           = CreateFileList(new_source, new_destination, sources, destinations, dirs, options, empty_dirs, recursion);
			
			if (result != ERROR_NONE)
				break;
		} else {
			sources     .push_back(new_source);
			destinations.push_back(new_destination);
			dirs        .push_back(is_dir);
		}
	} while (FindNextFile(hFind, &fd));

	recursion--;
	FindClose(hFind);
	return result;
}

INSTALLER_ERROR_CODE CreateTimestampList(std::wstring path, size_t path_cut, std::vector<std::wstring> &namelist, std::vector<time_t> &timelist) 
{
	WIN32_FIND_DATAW fd;
	std::wstring wildcard = path + L"\\*";
	HANDLE hFind          = FindFirstFile(wildcard.c_str(), &fd);

	if (hFind == INVALID_HANDLE_VALUE) {
		DWORD error_code = GetLastError();
		
		if (error_code == ERROR_FILE_NOT_FOUND || error_code == ERROR_PATH_NOT_FOUND)
			return ERROR_NONE;

		return ErrorMessage(STR_ERROR_FILE_LIST, L"%STR% " + wildcard + L"  - " + UInt2StrW(error_code) + L" " + FormatError(error_code));
	}
	
	do {
		if (wcscmp(fd.cFileName,L".") == 0 || wcscmp(fd.cFileName,L"..") == 0)
			continue;
			
		std::wstring file_name = (std::wstring)fd.cFileName;
		std::wstring full_path = path + L"\\" + file_name;

		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			CreateTimestampList(full_path, path_cut, namelist, timelist);
		else {
			//https://www.gamedev.net/forums/topic/565693-converting-filetime-to-time_t-on-windows/
			ULARGE_INTEGER ull = {0};
			ull.LowPart  = fd.ftLastWriteTime.dwLowDateTime;
			ull.HighPart = fd.ftLastWriteTime.dwHighDateTime;
			time_t stamp = ull.QuadPart / 10000000ULL - 11644473600ULL;
	
			namelist.push_back(full_path.substr(path_cut));
			timelist.push_back(stamp);
		}
	} while (FindNextFile(hFind, &fd));
	
	FindClose(hFind);
	return ERROR_NONE;
}

INSTALLER_ERROR_CODE Download(std::wstring url, int options, std::wstring log_file_name)
{
	// Delete previously downloaded file
	if (
		!global.test_mode && 
		options & (FLAG_CLEAN_DL_NOW | FLAG_CLEAN_DL_LATER) &&
		global.downloads.size()>0 && 
		Equals(global.downloaded_filename_last, global.downloads[global.downloads.size()-1])
	) {
		std::wstring filename = L"fwatch\\tmp\\" + global.downloaded_filename_last;

		if (DeleteFile(filename.c_str()))
			global.downloads.pop_back();
	}
	
	global.downloaded_filename = PathLastItem(UnQuote(url));

	// Format arguments
	std::wstring output = L"--output-document=";
	size_t find         = url.find(output);
	
	while(find != std::wstring::npos) {
		bool in_quote    = false;
		bool outer_quote = false;
		
		if (find > 0 && url[find-1] == L'\"') {		// preceding quotation mark
			in_quote    = true;
			outer_quote = true;
		}

		find      += output.length();	// skip to value
		size_t end = find;
		
		while(end<url.length() && (!iswspace(url[end]) || in_quote)) {	// skip to the end of value
			if (url[end]==L'"')
				in_quote = !in_quote;

			end++;
		}
		
		// validate
		std::wstring path = url.substr(find, end-find);
		path              = PathLastItem(path);
		path              = ReplaceAll(path, L"\"", L"");
		
		if (path.empty() || path == L"..")
			return ErrorMessage(STR_DOWNLOAD_PATH_ERROR);
		
		// reassemble
		global.downloaded_filename = path;
		url = url.substr(0,find) + (outer_quote ? L"" : L"\"") + L"fwatch\\tmp\\" + path + L"\" " + url.substr(end);

		find = url.find(output, find);
	}
	
	output = L"-O";
	find   = url.find(output);
	
	while(find != std::wstring::npos) {
		if (find > 0 && iswspace(url[find-1])) {	// must precede with whitespace
			find += output.length();
			
			while(find<url.length() && iswspace(url[find]))	// skip to value
				find++;
				
			bool in_quote = false;
			size_t end    = find;
	
			while(end < url.length() && (!iswspace(url[end]) || in_quote)) {	// skip to the end of value
				if (url[end] == L'"')
					in_quote = !in_quote;

				end++;
			}		

			std::wstring path = url.substr(find, end-find);
			path              = PathLastItem(path);
			path              = ReplaceAll(path, L"\"", L"");
	
			if (path.empty() || path == L"..")
				return ErrorMessage(STR_DOWNLOAD_PATH_ERROR);
	
			global.downloaded_filename = path;
			url = url.substr(0,find) + L"\"fwatch\\tmp\\" + path + L"\" " + url.substr(end);
		} else
			find += output.length();
		
		find = url.find(output, find);
	}

	std::wstring arguments = L" --user-agent=\"Mozilla/5.0 (Windows NT 6.1; Win64; x64; rv:103.0) Gecko/20100101 Firefox/103.0\" --tries=1 --no-check-certificate --output-file=fwatch\\tmp\\schedule\\downloadLog.txt --directory-prefix=fwatch\\tmp\\ ";
	
	if (options & FLAG_CONTINUE)
		arguments += L" --continue ";
	else
		if (~options & FLAG_OVERWRITE)
			arguments += L"--no-clobber ";
	
	if (options & FLAG_SILENT_MODE)
		arguments += L"--header=\"ofpgsinstall: 1\" ";

	arguments += url;
	DeleteFile(L"fwatch\\tmp\\schedule\\downloadLog.txt");

				
	// Execute program
	PROCESS_INFORMATION pi;
    STARTUPINFO si; 
	ZeroMemory( &si, sizeof(si) );
	ZeroMemory( &pi, sizeof(pi) );
	si.cb 		   = sizeof(si);
	si.dwFlags 	   = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;
	
	if (!CreateProcess(L"fwatch\\data\\wget.exe", &arguments[0], NULL, NULL, false, 0, NULL, NULL, &si, &pi)) {
		DWORD errorCode = GetLastError();
		return ErrorMessage(STR_ERROR_EXE, L"%STR% wget.exe - " + Int2StrW(errorCode) + L" " + FormatError(errorCode));
	} else
		if (~options & FLAG_SILENT_MODE)
			LogMessage(L"Downloading  " + (log_file_name.empty() ? url : log_file_name));



	// Wait for the program to finish its job
	DWORD exit_code     = STILL_ACTIVE;
	std::string message = "";

	Sleep(10);

	do {
		if (global.order == ORDER_ABORT) {
			TerminateProcess(pi.hProcess, 0);
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
			return ERROR_USER_ABORTED;
		}

		if (global.order == ORDER_PAUSE) {
			HWND window = GetWindowHandle(pi.dwProcessId);
			if (window) {
				PostMessage(window, WM_CLOSE, 0, 0);
				exit_code = 0;
				break;
			}
		}

		ParseWgetLog(message);
		GetExitCodeProcess(pi.hProcess, &exit_code);
		Sleep(100);
	} while (exit_code == STILL_ACTIVE);

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	if (global.order == ORDER_PAUSE)
		return ERROR_PAUSED;
	
	ParseWgetLog(message);
	
	if (exit_code == 1 && message.find("not retrieving") != std::string::npos) {
		exit_code = 0;
		LogMessage(utf16(message));
	}
	
	if (exit_code) {
		std::wstring exit_status = L"";
		
		switch(exit_code) {
			case 1 : exit_status=L"Generic error code"; break;
			case 2 : exit_status=L"Parse error"; break;
			case 3 : exit_status=L"File I/O error"; break;
			case 4 : exit_status=L"Network failure"; break;
			case 5 : exit_status=L"SSL verification failure"; break;
			case 6 : exit_status=L"Username/password authentication failure"; break;
			case 7 : exit_status=L"Protocol errors"; break;
			case 8 : exit_status=L"Server issued an error response"; break;
		}
		
		LogMessage(UInt2StrW(exit_code) + L" - " + exit_status + L" - " + utf16(message));
		ErrorMessage(STR_DOWNLOAD_FAILED, L"%STR%\r\n" + global.downloaded_filename + L"\r\n\r\n" + utf16(message), options & FLAG_SILENT_MODE ? ERROR_WRONG_SCRIPT : ERROR_COMMAND_FAILED);
	} else
		if (~options & FLAG_SILENT_MODE)
			global.downloads.push_back(global.downloaded_filename);
			
	global.downloaded_filename_last = options & FLAG_CLEAN_DL_LATER ? L"" : global.downloaded_filename;
	
	return (exit_code!=ERROR_SUCCESS ? ERROR_COMMAND_FAILED : ERROR_NONE);
}

INSTALLER_ERROR_CODE Unpack(std::wstring file_name, std::wstring password, int options)
{
	// Get subdirectories
	std::wstring relative_path = PathNoLastItem(file_name);

	if (Equals(relative_path.substr(0,10), L"_extracted"))
		relative_path = relative_path.substr(11) += L"_extracted";

	// Clean destination directory
	std::wstring unpack_destination = L"fwatch\\tmp\\_extracted";
	
	if (relative_path != L"")
		unpack_destination += L"\\" + relative_path;

	// Backup files in test mode; otherwise clean directory
	if (global.test_mode) {
		std::vector<std::wstring> source_list;
		std::vector<std::wstring> destination_list;
		std::vector<bool>         is_dir_list;
		std::vector<std::wstring> empty_dirs;
		int recursion = -1;

		INSTALLER_ERROR_CODE result = CreateFileList(unpack_destination, L"fwatch\\tmp\\_backup\\__extracted", source_list, destination_list, is_dir_list, FLAG_MOVE_FILES | FLAG_MATCH_DIRS, empty_dirs, recursion);
		if (result != ERROR_NONE)
			return result;

		for (size_t i=0;  i<source_list.size(); i++) {
			if (global.order == ORDER_ABORT)
				return ERROR_USER_ABORTED;

			MakeDir(PathNoLastItem(destination_list[i]), FLAG_SILENT_MODE);

			std::wstring final_destination = destination_list[i];
			DWORD backup_attr              = GetFileAttributes(final_destination.c_str());
			int backup_num                 = 1;

			while (backup_attr != INVALID_FILE_ATTRIBUTES) {
				final_destination = destination_list[i] + Int2StrW(++backup_num);
				backup_attr       = GetFileAttributes(final_destination.c_str());
			}

			if (MoveFileEx(source_list[i].c_str(), final_destination.c_str(), MOVEFILE_REPLACE_EXISTING)) {
				INSTALLER_OPERATION_LOG backup = {0};
				backup.instruction_index       = global.instruction_index;
				backup.operation_type          = OPERATION_MOVE;
				backup.source                  = final_destination;
				backup.destination             = source_list[i];
				global.rollback.push_back(backup);
			} else {
				DWORD error_code = GetLastError();
				LogMessage(L"Failed to backup " + source_list[i]);
				result = ErrorMessage(STR_MOVE_ERROR,
					L"%STR% " + source_list[i] + L" " + global.lang[STR_MOVE_TO_ERROR] + L" " + destination_list[i] + L" - " + Int2StrW(error_code) + L" " + FormatError(error_code)
				);
				return ERROR_COMMAND_FAILED;
			}
		}
	} else
		DeleteDirectory(unpack_destination);


	// Create log file
	SECURITY_ATTRIBUTES sa = {0};
    sa.nLength              = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle       = TRUE;       

    HANDLE logFile = CreateFile(L"fwatch\\tmp\\schedule\\unpackLog.txt",
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
	si.wShowWindow = SW_HIDE;
	si.hStdInput   = NULL;
	si.hStdOutput  = logFile;
	si.hStdError   = logFile;

	std::wstring arguments = 
		WrapInQuotes(global.working_directory) + 
		(password.empty() ? L"" : L" -p"+password) + 
		L" x -y -o\"" + 
		unpack_destination + 
		L"\\\" -bb3 -bsp1 " + 
		L"\"fwatch\\tmp\\" + 
		file_name + 
		L"\"";

	if (!CreateProcess(L"fwatch\\data\\7z.exe", &arguments[0], NULL, NULL, true, 0, NULL, NULL, &si, &pi)) {		
		DWORD errorCode = GetLastError();
		return ErrorMessage(STR_ERROR_EXE, L"%STR% 7z.exe - " + UInt2StrW(errorCode) + L" " + FormatError(errorCode));
	} else
		LogMessage(L"Extracting " + file_name);
		
	Sleep(10);


	// Wait for the program to finish its job
	DWORD exit_code             = STILL_ACTIVE;	
	std::string message         = "";
	INSTALLER_ERROR_CODE output = ERROR_NONE;

	do {					
		if (global.order == ORDER_ABORT) {
			TerminateProcess(pi.hProcess, 0);
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
			CloseHandle(logFile);
			return ERROR_USER_ABORTED;
		}

		if (global.order == ORDER_PAUSE) {
			TerminateProcess(pi.hProcess, 0);
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
			CloseHandle(logFile);
			return ERROR_PAUSED;
		}
		
		ParseUnpackLog(message, file_name);
		GetExitCodeProcess(pi.hProcess, &exit_code);
		Sleep(100);
	} while (exit_code == STILL_ACTIVE);

	ParseUnpackLog(message, file_name);

	if (exit_code != ERROR_SUCCESS) {
		output = ERROR_COMMAND_FAILED;
		LogMessage(Int2StrW(exit_code) + L" - " + utf16(message));
		
		if (message.find("Can not open the file as") != std::string::npos  &&  message.find("archive") != std::string::npos)
			output = ERROR_WRONG_ARCHIVE;

		if (~options & FLAG_ALLOW_ERROR)
			ErrorMessage(STR_UNPACK_ERROR, L"%STR%\r\n" + file_name + L"\r\n\r\n" + utf16(message));
	}

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	CloseHandle(logFile);

	if (global.test_mode) {
		INSTALLER_OPERATION_LOG backup = {0};
		backup.instruction_index       = global.instruction_index;
		backup.source                  = unpack_destination;
		backup.operation_type          = OPERATION_DELETE_DIR;
		global.rollback.push_back(backup);
	}

	return output;
}

INSTALLER_ERROR_CODE MakeDir(std::wstring path, int options)
{
	std::vector<std::wstring> directories;
	Tokenize(path, L"\\/", directories);

	std::wstring build_path = L"";
	int result              = 0;

	for (size_t i=0; i<directories.size(); i++) {
		build_path += (build_path!=L"" ? L"\\" : L"") + directories[i];
		result      = CreateDirectory(build_path.c_str(), NULL);

		if (~options & FLAG_SILENT_MODE) {
			if (!result) {
				DWORD error_code = GetLastError();

				if (error_code != ERROR_ALREADY_EXISTS)
					return ErrorMessage(STR_MDIR_ERROR, L"%STR% " + build_path + L" " + Int2StrW(error_code) + L" " + FormatError(error_code));
			} else {
				INSTALLER_OPERATION_LOG backup = {0};
				backup.instruction_index       = global.instruction_index;
				backup.operation_type          = OPERATION_DELETE_DIR;
				backup.source                  = build_path;
				global.rollback.push_back(backup);
				LogMessage(L"Created directory " + build_path);
			}
		}
	}
	
	return ERROR_NONE;
}

INSTALLER_ERROR_CODE MoveFiles(std::wstring source, std::wstring destination, std::wstring new_name, int options)
{
	WriteProgressFile(INSTALL_PROGRESS, global.lang[STR_ACTION_COPYING]+L"...");

	// Optionally create destination directory
	if (options & FLAG_CREATE_DIR) {
		INSTALLER_ERROR_CODE result = MakeDir(PathNoLastItem(destination));
		
		if (result != ERROR_NONE)
			return result;
	}
	
	// Find files and save them to a list
	std::vector<std::wstring> source_list;
	std::vector<std::wstring> destination_list;
	std::vector<bool>         is_dir_list;
	std::vector<std::wstring> empty_dirs;
	int recursion = -1;

	INSTALLER_ERROR_CODE result = CreateFileList(source, destination+new_name, source_list, destination_list, is_dir_list, options, empty_dirs, recursion);

	if (result != ERROR_NONE)
		return result;
	

	DWORD flags       = MOVEFILE_REPLACE_EXISTING;
	bool FailIfExists = false;

	if (~options & FLAG_OVERWRITE) {
		FailIfExists = true;
		flags        = 0;
	}
	
	std::wstring source_wide;
	std::wstring destination_wide;

	// For each file in the list
	for (size_t i=0;  i<source_list.size(); i++) {
		if (global.order == ORDER_ABORT)
			return ERROR_USER_ABORTED;

		// Make backup
		INSTALLER_OPERATION_LOG backup = {0};
		backup.instruction_index       = global.instruction_index;
		backup.operation_type          = OPERATION_NONE;

		if (~options & FLAG_DONT_BACKUP) {
			bool internal_move       = Equals(source_list[i].substr(0,global.current_mod_new_name.length()),global.current_mod_new_name);
			DWORD dest_attr          = GetFileAttributes(destination_list[i].c_str());
			std::wstring backup_path = L"fwatch\\tmp\\_backup\\" + destination_list[i];

			// If replacing a file
			if (dest_attr != INVALID_FILE_ATTRIBUTES && options & FLAG_OVERWRITE) {
				MakeDir(PathNoLastItem(backup_path), FLAG_SILENT_MODE);

				DWORD backup_attr = GetFileAttributes(backup_path.c_str());
				int backup_num    = 1;

				while (backup_attr != INVALID_FILE_ATTRIBUTES) {
					backup_path = L"fwatch\\tmp\\_backup\\" + destination_list[i] + Int2StrW(++backup_num);
					backup_attr = GetFileAttributes(backup_path.c_str());
				}

				if (MoveFileEx(destination_list[i].c_str(), backup_path.c_str(), MOVEFILE_REPLACE_EXISTING)) {
					backup.operation_type = OPERATION_MOVE;
					backup.source         = backup_path;
					backup.destination    = destination_list[i];
					global.rollback.push_back(backup);
				} else {
					DWORD error_code = GetLastError();
					LogMessage(L"Failed to backup " + destination_list[i]);
					result = ErrorMessage(STR_MOVE_ERROR,
						L"%STR% " + destination_list[i] + L" " + global.lang[STR_MOVE_TO_ERROR] + L" " + backup_path + L" - " + Int2StrW(error_code) + L" " + FormatError(error_code)
					);
					break;
				}

				if (internal_move || global.test_mode) {
					backup.source      = destination_list[i];
					backup.destination = source_list[i];
				} else {
					backup.operation_type = OPERATION_NONE;
				}
			} else 
				// If not replacing a file
				if (dest_attr == INVALID_FILE_ATTRIBUTES) {
					if (internal_move || global.test_mode) {
						backup.operation_type = OPERATION_MOVE;
						backup.source         = destination_list[i];
						backup.destination    = source_list[i];
					} else {
						backup.operation_type = is_dir_list[i] ? OPERATION_DELETE_DIR : OPERATION_DELETE;
						backup.source         = destination_list[i];
					}
				}
		}

		// Format path for logging
		std::wstring destinationLOG = PathNoLastItem(destination_list[i], FLAG_NO_END_SLASH);

		if (destinationLOG.empty())
			destinationLOG = L"the game folder";
		
		std::wstring logstr = L"";

		if (options & FLAG_MOVE_FILES)
			logstr = L"Moving";
		else
			logstr = L"Copying";
		
		logstr += L"  " + ReplaceAll(source_list[i], L"fwatch\\tmp\\_extracted\\", L"") + L"  to  " + destinationLOG;
		
		if (!new_name.empty() && PathLastItem(source_list[i]) != PathLastItem(destination_list[i]))
			logstr += L"  as  " + PathLastItem(destination_list[i]);

		BOOL success = false;
		
		if (options & FLAG_MOVE_FILES)
			success = MoveFileEx(source_list[i].c_str(), destination_list[i].c_str(), flags);
		else
			success = CopyFile(source_list[i].c_str(), destination_list[i].c_str(), FailIfExists);

	    if (success) {
			if (backup.operation_type != OPERATION_NONE)
				global.rollback.push_back(backup);
			LogMessage(logstr);
		} else {
			DWORD error_code = GetLastError();
			
			if (~options & FLAG_OVERWRITE && error_code == ERROR_ALREADY_EXISTS) {
	    		logstr += L"  - file already exists";
				LogMessage(logstr);
			} else {
				LogMessage(logstr);
				result = ErrorMessage(
					options & FLAG_MOVE_FILES ? STR_MOVE_ERROR : STR_COPY_ERROR,
					L"%STR% " + source_list[i] + L" " + global.lang[STR_MOVE_TO_ERROR] + L" " + destination_list[i] + L" - " + Int2StrW(error_code) + L" " + FormatError(error_code)
				);
				break;
			}
		}
	}

	// Remove empty directories
	if (result != ERROR_NONE && options & FLAG_MOVE_FILES && empty_dirs.size() > 0) {
		size_t i = empty_dirs.size();
		do {
			i--;
			RemoveDirectory(empty_dirs[i].c_str());
		} while (i > 0);
	}

	return result;
}

INSTALLER_ERROR_CODE ExtractPBO(std::wstring source, std::wstring destination, std::wstring file_to_unpack, bool silent) 
{
	WriteProgressFile(INSTALL_PROGRESS, global.lang[STR_ACTION_UNPACKINGPBO]+L"...");


	// Program extractpbo.exe has a bug with destination argument
	// It won't work if path has a space wrapping in quotes doesn't do anything
	// Temporarily we have to extract it to D:\Temp\_extractedPBO and then move it to the actual destination
	std::wstring destination_temp = L"";
	std::wstring destinationLOG   = L"";
	
	if (!destination.empty()) {
		destination_temp = global.working_directory.substr(0,3) + L"temp\\_extractedPBO\\";
		destinationLOG   = destination_temp;
	}

	std::wstring addon_name = PathLastItem(source);
	size_t dot = addon_name.find_last_of(L".");

	if (dot != std::wstring::npos)
		addon_name = addon_name.substr(0, dot);

	std::wstring extracted_addon_dir = L"";
	if (destination.empty()) {
		extracted_addon_dir = PathNoLastItem(source) + addon_name;
	} else {
		if (Equals(destination.substr(0,global.working_directory.length()), global.working_directory))
			extracted_addon_dir = destination.substr(global.working_directory.length()+1) + addon_name;
		else
			extracted_addon_dir = destination + addon_name;
	}

	DWORD dest_attr = GetFileAttributes(extracted_addon_dir.c_str());
	if (dest_attr != INVALID_FILE_ATTRIBUTES) {
		std::wstring backup_path = L"fwatch\\tmp\\_backup\\" + extracted_addon_dir;
		MakeDir(PathNoLastItem(backup_path), FLAG_SILENT_MODE);

		DWORD backup_attr = GetFileAttributes(backup_path.c_str());
		int backup_num    = 1;

		while (backup_attr != INVALID_FILE_ATTRIBUTES) {
			backup_path = L"fwatch\\tmp\\_backup\\" + extracted_addon_dir + Int2StrW(++backup_num);
			backup_attr = GetFileAttributes(backup_path.c_str());
		}

		if (MoveFileEx(extracted_addon_dir.c_str(), backup_path.c_str(), MOVEFILE_REPLACE_EXISTING)) {
			INSTALLER_OPERATION_LOG backup = {0};
			backup.instruction_index       = global.instruction_index;
			backup.operation_type          = OPERATION_MOVE;
			backup.source                  = backup_path;
			backup.destination             = extracted_addon_dir;
			global.rollback.push_back(backup);
		} else {
			DWORD error_code = GetLastError();
			LogMessage(L"Failed to backup " + extracted_addon_dir);
			return ErrorMessage(STR_MOVE_ERROR,
				L"%STR% " + extracted_addon_dir + L" " + global.lang[STR_MOVE_TO_ERROR] + L" " + backup_path + L" - " + Int2StrW(error_code) + L" " + FormatError(error_code)
			);
		}
	}

	
	// Create log file
	SECURITY_ATTRIBUTES sa = {0};
    sa.nLength              = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle       = TRUE;

    HANDLE logFile = CreateFile(L"fwatch\\tmp\\schedule\\PBOLog.txt",
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
	si.wShowWindow = SW_HIDE;
	si.hStdInput   = NULL;
	si.hStdOutput  = logFile;
	si.hStdError   = logFile;

	std::wstring exename    = L"ExtractPbo.exe";
	std::wstring executable = L"fwatch\\data\\" + exename;
	std::wstring arguments  = L" -NYP ";
	
	if (!file_to_unpack.empty())
		arguments += L" -F " + file_to_unpack + L" ";
	
	arguments += WrapInQuotes(source) + L" " + destination_temp;

	if (!CreateProcess(&executable[0], &arguments[0], NULL, NULL, true, 0, NULL, NULL, &si, &pi)) {
		DWORD errorCode = GetLastError();
		return ErrorMessage(STR_ERROR_EXE, L"%STR% " + exename + L" - " + Int2StrW(errorCode) + L" " + FormatError(errorCode));	
	} else 
		if (!silent)
			LogMessage(L"Extracting  " + source + (destination.empty() ? L"" : (L"  to  " + destinationLOG)));
		
	Sleep(10);


	// Wait for the program to finish its job
	DWORD exit_code           = STILL_ACTIVE;
	std::string error_message = "";
	
	do {
		if (global.order == ORDER_ABORT) {
			TerminateProcess(pi.hProcess, 0);
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
			CloseHandle(logFile);
			return ERROR_USER_ABORTED;
		}
		
		ParsePBOLog(error_message, exename, source);
		GetExitCodeProcess(pi.hProcess, &exit_code);
		Sleep(100);
	} while (exit_code == STILL_ACTIVE);

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	CloseHandle(logFile);
	Sleep(600);

	ParsePBOLog(error_message, exename, source);
	
	if (exit_code != ERROR_SUCCESS)
		ErrorMessage(STR_PBO_UNPACK_ERROR, L"%STR% " + UInt2StrW(exit_code) + L" - " + utf16(error_message));
	else 
		if (!destination.empty()) {
			std::wstring dir_name = PathLastItem(source);
			dir_name              = dir_name.substr(0, dir_name.length()-4);
			source                = destination_temp + dir_name;
			destination           = UnQuote(destination) + dir_name;
			MoveFiles(source, destination, L"", FLAG_MOVE_FILES | FLAG_OVERWRITE | FLAG_MATCH_DIRS | FLAG_DONT_BACKUP);
		}

	INSTALLER_OPERATION_LOG backup = {0};
	backup.instruction_index       = global.instruction_index;
	backup.operation_type          = OPERATION_DELETE_DIR;
	backup.source                  = extracted_addon_dir;
	global.rollback.push_back(backup);

	return (exit_code!=ERROR_SUCCESS ? ERROR_COMMAND_FAILED : ERROR_NONE);
}

INSTALLER_ERROR_CODE ChangeFileDate(std::wstring file_name, FILETIME *ft) 
{   
	HANDLE file_handle = CreateFile(file_name.c_str(), FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD error_code   = ERROR_SUCCESS;
	
	if (file_handle != INVALID_HANDLE_VALUE) {
		if (!SetFileTime(file_handle, (LPFILETIME)NULL, (LPFILETIME)NULL, ft))
			error_code = GetLastError();
		CloseHandle(file_handle);
	} else
		error_code = GetLastError();
	
	if (error_code)
		return ErrorMessage(STR_EDIT_WRITE_ERROR, L"%STR% " + file_name + L" - " + UInt2StrW(error_code) + L" " + FormatError(error_code));
	else
		return ERROR_NONE;
}

INSTALLER_ERROR_CODE ChangeFileDate(std::wstring file_name, time_t timestamp)
{
	// https://support.microsoft.com/en-us/help/167296/how-to-convert-a-unix-time-t-to-a-win32-filetime-or-systemtime
	
	FILETIME ft       = {0};
	LONGLONG ll       = Int32x32To64(timestamp, 10000000) + 116444736000000000;
	ft.dwLowDateTime  = (DWORD)ll;
	ft.dwHighDateTime = (DWORD)(ll >> 32);
	return ChangeFileDate(file_name, &ft);
}

INSTALLER_ERROR_CODE ChangeFileDate(std::wstring file_name, std::wstring timestamp)
{
	FILETIME ft;
	std::vector<std::string> date_item;
	Tokenize(utf8(timestamp), "-T:+ ", date_item);
	
	if (date_item.size() == 1)
		return ChangeFileDate(file_name, _wtoi(timestamp.c_str()));
	else {
		while(date_item.size() < 6)
			date_item.push_back("0");
			
		SYSTEMTIME st    = {0};
		st.wYear         = Str2Short(date_item[0].c_str());
		st.wMonth        = Str2Short(date_item[1].c_str());
		st.wDay          = Str2Short(date_item[2].c_str());
		st.wHour         = Str2Short(date_item[3].c_str());
		st.wMinute       = Str2Short(date_item[4].c_str());
		st.wSecond       = Str2Short(date_item[5].c_str());
		st.wMilliseconds = 0;
		SystemTimeToFileTime(&st, &ft);
		return ChangeFileDate(file_name, &ft);
	}
}

INSTALLER_ERROR_CODE RequestExecution(std::wstring path_to_dir, std::wstring file_name)
{		
	DWORD exit_code = ERROR_SUCCESS;
	
	LogMessage(L"Asking the user to run " + file_name);
	
	std::wstring message = global.lang[STR_ASK_EXE] + L":\r\n" + file_name + L"\r\n\r\n" + global.lang[STR_ALTTAB];
	WriteProgressFile(INSTALL_WAITINGFORUSER, message);
	
	message = L"File\n" + file_name + L"must be manually run\n\nPress OK to start\nPress CANCEL to skip installing this modfolder";
	int msgboxID = MessageBox(NULL, message.c_str(), L"Addon Installer", MB_ICONQUESTION | MB_OKCANCEL | MB_DEFBUTTON1);
	
	if (msgboxID == IDCANCEL)
		global.skip_modfolder = true;
	else {
		CopyFile(L"Aspect_Ratio.hpp", L"Aspect_Ratio.backup", false);
		
		// Execute program
		PROCESS_INFORMATION pi;
	    STARTUPINFO si; 
		ZeroMemory( &si, sizeof(si) );
		ZeroMemory( &pi, sizeof(pi) );
		si.cb 		   = sizeof(si);
		si.dwFlags 	   = STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_SHOW;
		
		std::wstring executable = path_to_dir + L"\\" + file_name;
		std::wstring arguments  = L" " + global.working_directory;
		
		if (!CreateProcess(&executable[0], &arguments[0], NULL, NULL, false, 0, NULL, NULL, &si, &pi)) {
			MoveFileEx(L"Aspect_Ratio.backup", L"Aspect_Ratio.hpp", MOVEFILE_REPLACE_EXISTING);
			DWORD errorCode = GetLastError();
			LogMessage(L"Failed to launch " + file_name + L" - " + UInt2StrW(errorCode) + L" " + FormatError(errorCode));
			return ERROR_COMMAND_FAILED;
		}
	
		// Wait for the program to finish its job
		do {
			GetExitCodeProcess(pi.hProcess, &exit_code);
			Sleep(100);
		} while (exit_code == STILL_ACTIVE);
		
		LogMessage(L"Exit code: " + UInt2StrW(exit_code));
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);

		MoveFileEx(L"Aspect_Ratio.backup", L"Aspect_Ratio.hpp", MOVEFILE_REPLACE_EXISTING);
	}
	
	std::wstring path_to_file = path_to_dir + file_name;
	ITEMIDLIST *pIDL          = ILCreateFromPath(path_to_file.c_str());
	
	if (pIDL != NULL) {
		HRESULT result = CoInitialize(NULL);
		UNREFERENCED_PARAMETER(result);

	    if (SHOpenFolderAndSelectItems(pIDL, 0, 0, 0) != S_OK)
	    	ShellExecute(NULL, L"open", path_to_dir.c_str(), NULL, NULL, SW_SHOWDEFAULT);

		CoUninitialize();
	    ILFree(pIDL);
	};

	return (exit_code!=ERROR_SUCCESS ? ERROR_COMMAND_FAILED : ERROR_NONE);
}

INSTALLER_ERROR_CODE Condition_If_version(const std::vector<std::wstring> &arguments) 
{
	if (arguments.size() < 1)
		return ErrorMessage(STR_ERROR_ARG_COUNT);
	
	// Process arguments (two or three)
	std::wstring op     = L"==";
	double given_number = 0;

	if (arguments.size() >= 2) {
		op           = arguments[0];
		given_number = _wtof(arguments[1].c_str());
	} else	
		if (iswdigit(arguments[0][0]) || arguments[0][0] == L'.')
			given_number = _wtof(arguments[0].c_str());
		else
			return ErrorMessage(STR_IF_NUMBER_ERROR);

	
	// Execute condition if not nested or if nested inside a valid condition
	bool result = false;
	
	if (global.condition_index<0  ||  global.condition_index>=0 && global.condition[global.condition_index]) {
		double game_version = _wtof(global.game_version.c_str());

		if (op == L"==" || op == L"=")
			result = game_version == given_number;
				
		if (op == L"!=" || op == L"<>")
			result = game_version != given_number;
				
		if (op == L"<")
			result = game_version < given_number;
				
		if (op == L">")
			result = game_version > given_number;	
				
		if (op == L"<=")
			result = game_version <= given_number;
				
		if (op == L">=")
			result = game_version >= given_number;
	}
	
	global.condition_index++;
	global.condition.push_back(result);
	return ERROR_NONE;
}

INSTALLER_ERROR_CODE Condition_Else()
{
	// If not nested or nested inside a valid condition then reverse a flag which will enable execution of the commands that follow
	if (global.condition_index == 0 || (global.condition_index > 0 && global.condition[global.condition_index-1]))
		global.condition[global.condition_index] = !global.condition[global.condition_index];
	
	return ERROR_NONE;
}

INSTALLER_ERROR_CODE Condition_Endif()
{
	if (global.condition_index >= 0)
		global.condition_index--;
		
	if (global.condition.size() > 0)
		global.condition.pop_back();

	return ERROR_NONE;
}

INSTALLER_ERROR_CODE Auto_Install(std::wstring file, DWORD attributes, int options, std::wstring password)
{
	if (global.order == ORDER_ABORT)
		return ERROR_USER_ABORTED;

	if (file.empty())
		return ERROR_NONE;

	std::wstring file_with_path = L"fwatch\\tmp\\" + file;
	std::wstring file_only      = PathLastItem(file);
	size_t dots                 = count(file_only.begin(), file_only.end(), L'.');
	
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
			return MoveFiles(file_with_path, L"", global.current_mod_new_name, FLAG_MOVE_FILES | FLAG_OVERWRITE | FLAG_MATCH_DIRS);
		}

		// If directory ends with _anim or _anims then move it to IslandCutscenes
		if (
			file_only.length() >= 4 && 
			Equals(file_only.substr(file_only.length()-5),L"anim") || 
			file_only.length() >= 5 && 
			Equals(file_only.substr(file_only.length()-5),L"_anim") || 
			file_only.length() >= 6 && 
			Equals(file_only.substr(file_only.length()-5),L"_anims")
		) {
			options &= ~FLAG_RUN_EXE;
			std::wstring destination = global.current_mod_new_name + L"\\IslandCutscenes\\" + (options & FLAG_RES_ADDONS ? L"_Res\\" : L"") ;
			return MoveFiles(file_with_path, destination, L"", FLAG_MOVE_FILES | FLAG_OVERWRITE | FLAG_CREATE_DIR | FLAG_MATCH_DIRS);
		}

		// If directory has a dot in its name then move it to Missions
		if (dots > 0) {
			std::wstring destination = GetMissionDestinationFromSQM(file_with_path+L"\\mission.sqm");
		
			if (destination != L"Addons") {
				std::wstring file_lower = Lowercase(file_only);	
				
				if (
					(Equals(destination,L"Missions") || Equals(destination,L"MPMissions")) && 
					(options & FLAG_DEMO_MISSIONS || file_lower.find(L"demo") != std::wstring::npos || file_lower.find(L"template") != std::wstring::npos)
				)
					destination += L"Users";
				
				options &= ~FLAG_RUN_EXE;
				return MoveFiles(file_with_path, global.current_mod_new_name+L"\\"+destination+L"\\", L"", FLAG_MOVE_FILES | FLAG_OVERWRITE | FLAG_CREATE_DIR | FLAG_MATCH_DIRS);
			}
		}

		// Check if the folder name is addons/bin/dta/campaigns etc.
		int index                        = DIR_NONE;
		bool missions_but_different_name = false;
		
		if (!Equals(file_only,L"_extracted")) {
			for (int i=DIR_ADDONS; i<DIR_MAX && index==DIR_NONE; i++)
				if (Equals(file_only,mod_subfolders[i]))
					index = i;
				
			// Folder could be an sp missions folder but named differently
			if (index == DIR_NONE) {
				std::wstring overview = file_with_path + L"\\overview.html";
				
				if (GetFileAttributes(overview.c_str()) != INVALID_FILE_ATTRIBUTES) {
					index                       = DIR_MISSIONS;
					missions_but_different_name = true;
				}
			}
		}

		// If so then move it to the modfolder
		if (index != DIR_NONE) {
			std::wstring destination = global.current_mod_new_name + L"\\";
			std::wstring new_name    = L"";
			
			if (!missions_but_different_name) {
				// If it's a Missions/MPMissions folder and it contains a single subfolder then move that subfolder instead
				if (index==DIR_MISSIONS  ||  index==DIR_MPMISSIONS) {
					std::wstring destination_overview_path = global.current_mod_new_name + L"\\Missions\\overview.html";
					DIRECTORY_INFO dir                     = ScanDirectory(file_with_path);
					
					if (dir.error_code == ERROR_NONE) {
						if (dir.number_of_files == 0 && dir.number_of_dirs == 1) {
							file_only = dir.file_list[0];
							
							// If that subfolder matches mod name then merge files; otherwise copy the entire subfolder
							if ((index == DIR_MISSIONS && IsModName(file_only)) || index == DIR_MPMISSIONS) {
								file          += L"\\" + file_only;
								file_with_path = L"fwatch\\tmp\\" + file;
								new_name       = DIR_MISSIONS ? L"Missions" : L"MPMissions";
							}
						}
					} else
						return ERROR_COMMAND_FAILED;
				}
			} else
				destination += L"Missions\\";
	
			options &= ~FLAG_RUN_EXE;
			return MoveFiles(file_with_path, destination, new_name, FLAG_MOVE_FILES | FLAG_OVERWRITE | FLAG_CREATE_DIR | FLAG_MATCH_DIRS);
		}
		
		bool scan_directory = true;
		
		// If the folder is some other modfolder then ignore it
		if (!Equals(file_only,L"_extracted") && !Equals(file_only,L"Res") && !force_install)
			for (int i=DIR_ADDONS; i<=DIR_DTA && scan_directory; i++) {
				std::wstring mod_subpath = file_with_path + L"\\" + mod_subfolders[i];
				DWORD attributes_subdir  = GetFileAttributes(mod_subpath.c_str());
				
				if (attributes_subdir!=INVALID_FILE_ATTRIBUTES  &&  attributes_subdir & FILE_ATTRIBUTE_DIRECTORY)
					scan_directory = false;
			}

		// Normal directory - scan its contents
		if (scan_directory) {
			DIRECTORY_INFO dir = ScanDirectory(file_with_path);
			
			if (dir.error_code != ERROR_NONE)
				return ERROR_COMMAND_FAILED;

			// If archive contains a single dir then set option to force to scan it
			if (dir.number_of_files == 0 && dir.number_of_dirs == 1 && Equals(file_with_path,L"fwatch\\tmp\\_extracted"))
				options |= FLAG_DONT_IGNORE;
			
			for (size_t i=0; i<dir.file_list.size(); i++) {
				std::wstring file_inside       = dir.file_list[i];
				std::wstring path_to_this_file = file + L"\\" + file_inside;
				int result                     = ERROR_NONE;
				bool is_dir                    = (dir.attributes_list[i] & FILE_ATTRIBUTE_DIRECTORY) > 0;
				
				// Files that are next to the wanted modfolder are moved without looking at them
				if (
					dir.number_of_wanted_mods > 0 && 
					(
						!is_dir || 
						is_dir && 
						dir.mod_sub_id_list[i] != DIR_MISSIONS && 
						dir.mod_sub_id_list[i] != DIR_MPMISSIONS && 
						!dir.is_mod_list[i] && 
						_wcsicmp(dir.file_list[i].c_str(),L"Res") != 0 && 
						_wcsicmp(dir.file_list[i].c_str(),L"ResAddons") != 0
					)
				) {
					std::wstring source      = file_with_path + L"\\" + dir.file_list[i];
					std::wstring destination = global.current_mod_new_name + L"\\";
					std::wstring new_name    = L"";
					
					// If it's "addons" folder then it probably contains island cutscenes; move it with changed name
					if (dir.mod_sub_id_list[i] == DIR_ADDONS)
						new_name = L"IslandCutscenes";
					
					options &= ~FLAG_RUN_EXE;
					result   = MoveFiles(source, destination, new_name, FLAG_MOVE_FILES | FLAG_OVERWRITE | FLAG_CREATE_DIR | FLAG_MATCH_DIRS);
				} else {
					file_inside = Lowercase(file_inside);

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
					std::vector<std::wstring> keywords;
					std::vector<bool> key_exists;
					keywords.push_back(L"demo");
					keywords.push_back(L"mission");
					keywords.push_back(L"user");
					keywords.push_back(L"editor");
					keywords.push_back(L"template");
					keywords.push_back(L"res");
					keywords.push_back(L"addons");
					key_exists.resize(keywords.size());
					
					for (size_t j=0; j<keywords.size(); j++)
						key_exists[j] = file_inside.find(keywords[j]) != std::wstring::npos;
					
					if (((key_exists[KEY_DEMO] || key_exists[KEY_EDITOR] || key_exists[KEY_TEMPLATE]) && key_exists[KEY_MISSION]) || key_exists[KEY_USER])
						options |= FLAG_DEMO_MISSIONS;
						
					// If folder name is "res" or contains words "res" and "addons" then island cutscenes inside will go to IslandCutscenes\_res
					if (Equals(file_inside,L"Res") || (key_exists[KEY_RES] && key_exists[KEY_ADDONS]))
						options |= FLAG_RES_ADDONS;
					
					result = Auto_Install(path_to_this_file, dir.attributes_list[i], options, password);
				}
				
				if (result != ERROR_NONE)
					return ERROR_COMMAND_FAILED;
			}
		}
	} else {
		// If it's a file then check its extension
		std::wstring file_extension = GetFileExtension(file);

		if (file.substr(0,11) == L"_extracted\\")
			file = file.substr(11);
		
		file = WrapInQuotes(file);
		
		enum VALID_EXTENSIONS {
			EXT_PBO,
			EXT_RAR,
			EXT_ZIP,
			EXT_7Z,
			EXT_ACE,
			EXT_EXE,
			EXT_CAB,
			EXT_INVALID
		};
		
		std::wstring valid_extensions[] = {
			L"pbo",
			L"rar",
			L"zip",
			L"7z",
			L"ace",
			L"exe",
			L"cab"
		};
		size_t number_of_extensions = sizeof(valid_extensions) / sizeof(valid_extensions[0]);
		size_t extension_index      = EXT_INVALID;
		
		for (size_t i=0; i<number_of_extensions && extension_index==EXT_INVALID; i++)
			if (file_extension == valid_extensions[i])
				extension_index = i;
		
		switch(extension_index) {
			// Unpack PBO and detect if it's an addon or a mission
			case EXT_PBO : {
				options                 &= ~FLAG_RUN_EXE;
				std::wstring destination = L"Addons";
				
				if (dots > 1) {
					destination = L"MPMissions";
	
					if (ExtractPBO(global.working_directory+L"\\"+file_with_path, L"", L"mission.sqm", 1) == 0) {
						std::wstring extracted_dir = PathNoLastItem(file_with_path) + file.substr(0, file.length()-4);
						destination                = GetMissionDestinationFromSQM(extracted_dir+L"\\mission.sqm");
						DeleteDirectory(extracted_dir);
					}
				}
					
				return MoveFiles(file_with_path, global.current_mod_new_name+L"\\"+destination+L"\\", L"", FLAG_MOVE_FILES | FLAG_OVERWRITE | FLAG_CREATE_DIR);
			}
			
			// Unpack archive and scan its contents
			case EXT_RAR : 
			case EXT_ZIP : 
			case EXT_7Z  : 
			case EXT_ACE : 
			case EXT_CAB : {
				INSTALLER_ERROR_CODE result = Unpack(file_with_path.substr(11), password);
								
				if (result == ERROR_NONE)
					return Auto_Install(PathNoLastItem(file_with_path.substr(11)) + L"_extracted", FILE_ATTRIBUTE_DIRECTORY, options, password);
				else
					return result;
			}

			// Unpack exe and scan its contents; if failed to unpack and nothing else was installed then ask the user to run it
			case EXT_EXE : {
				int result = Unpack(file_with_path.substr(11), password, FLAG_ALLOW_ERROR);
				
				if (result == ERROR_NONE) {
					std::wstring resource_path = PathNoLastItem(file_with_path) + L"_extracted\\.rsrc";
					
					// Make sure that this exe is not a program
					if (GetFileAttributes(resource_path.c_str()) == INVALID_FILE_ATTRIBUTES)
						return Auto_Install(PathNoLastItem(file_with_path.substr(11))+L"_extracted", FILE_ATTRIBUTE_DIRECTORY, options, password);
				} else
					if (options & FLAG_RUN_EXE)
						return RequestExecution(L"", file);

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
		for (size_t i=0; i<global.downloads.size(); i++) {
			std::wstring filename = L"fwatch\\tmp\\" + global.downloads[i];

			if (!DeleteFile(filename.c_str())) {
				DWORD errorCode       = GetLastError();
				std::wstring errorMSG = FormatError(errorCode);
				LogMessage(L"Failed to delete " + filename + L" - " + UInt2StrW(errorCode) + L" " + errorMSG);
			}
		}

		DeleteDirectory(L"fwatch\\tmp\\_extracted");
		global.downloads.clear();
	}

	// Remove backup
	DeleteDirectory(L"fwatch\\tmp\\_backup");
	global.rollback.clear();
	
	WriteModID(global.current_mod_new_name, global.current_mod, global.current_mod_id+L";"+global.current_mod_version, global.current_mod_keepname);	
}

	// Verify folder, clean up and reset variables
void EndMod()
{
	if (global.current_mod.empty())
		return;
		
	// Check if folder even exists
	DWORD dir = GetFileAttributes(global.current_mod_new_name.c_str());

	if (dir == INVALID_FILE_ATTRIBUTES || !(dir & FILE_ATTRIBUTE_DIRECTORY)) {
		if (global.missing_modfolders != L"")
			global.missing_modfolders += L", ";

		global.missing_modfolders += global.current_mod;
		LogMessage(L"Modfolder " + global.current_mod + L" wasn't actually installed!");
	}
	
	if (global.test_mode)
		return;

	WriteProgressFile(INSTALL_PROGRESS, global.lang[STR_ACTION_CLEANING]);
	EndModVersion();
	
	global.current_mod     = L"";
	global.skip_modfolder  = false;
	global.condition_index = -1;
	global.condition.clear();
	global.current_mod_alias.clear();
}

void RollBackInstallation(size_t wanted_pos)
{
	size_t to_delete = 0;

	for (
		std::vector<INSTALLER_OPERATION_LOG>::reverse_iterator current_op=global.rollback.rbegin(); 
		current_op!=global.rollback.rend() && (wanted_pos==UINT_MAX || (wanted_pos!=UINT_MAX && current_op->instruction_index>=wanted_pos)); 
		++current_op
	) {
		std::wstring a = current_op->source.c_str();
		std::wstring b = current_op->destination.c_str();
		to_delete++;

		switch(current_op->operation_type) {
			case OPERATION_MOVE: {
				MakeDir(PathNoLastItem(current_op->destination).c_str(), FLAG_SILENT_MODE);
				MoveFileEx(current_op->source.c_str(), current_op->destination.c_str(), MOVEFILE_REPLACE_EXISTING);
			} break;

			case OPERATION_DELETE: {
				DeleteFile(current_op->source.c_str());
			} break;

			case OPERATION_DELETE_DIR : {
				DeleteDirectory(current_op->source.c_str());
			} break;

			case OPERATION_FILEDATE : {
				ChangeFileDate(current_op->source.c_str(), &(current_op->modif_time));
			} break;

			case OPERATION_ALIAS : {
				global.current_mod_alias.clear();
				Tokenize(current_op->source, L" ", global.current_mod_alias);
			} break;
		}
	}

	for (size_t i=0; i<to_delete; i++)
		global.rollback.pop_back();
}

void ResetInstallationState()
{
	global.skip_modfolder           = false;
	global.last_download_attempt    = false;
	global.saved_alias_array_size   = 0;
	global.download_iterator        = 0;
	global.downloaded_filename_last = L"";

	if (!global.test_mode) {
		global.current_mod         = L"";
		global.current_mod_version = L"";
	}

	global.condition_index     = -1;
	global.condition.clear();
	global.current_mod_alias.clear();
}

HWND GetWindowHandle(DWORD input_pid)
{
	HWND handle = GetTopWindow(NULL);
	
	if (handle) {
		DWORD current_pid = 0;
	
		while (handle) {
			GetWindowThreadProcessId(handle, &current_pid);
	
			if (input_pid == current_pid)
				return handle;
			
			handle = GetNextWindow(handle, GW_HWNDNEXT);
		}
	}
	
	return NULL;
};
// -------------------------------------------------------------------------------------------------------




// Interface ---------------------------------------------------------------------------------------------

void EditMultilineUpdateText(HWND control, std::wstring &text)
{
	if (control) {
		SetWindowText(control, text.c_str());
		SendMessage(control, EM_SETSEL, 0, -1);
		SendMessage(control, EM_SETSEL, ULONG_MAX, -1);
		SendMessage(control, EM_SCROLLCARET, 0, 0);
	}
}

void EnableWindowMenu(bool yes)
{
	int id[] = {ID_PROCESS_ABORT, ID_PROCESS_PAUSE, ID_OPTIONS_RESTARTGAME};
	for (int i=0; i<sizeof(id)/sizeof(id[0]); i++)
		EnableMenuItem(global.window_menu, id[i], yes ? 0 : MF_GRAYED);
}

void SetCommandInfo(int index, std::wstring title, std::wstring content) 
{
	if (!content.empty()) {
		index += TXT_COMMAND_INFO0;
		std::wstring temp = title + L": " + content;
		SetWindowText(global.controls[index], temp.c_str());
		ShowWindow(global.controls[index], SW_SHOW);
	}
}

void ShowCommandInfo()
{
	for (int i=TXT_COMMAND_INFO0; i<=TXT_COMMAND_INFO3; i++)
		SetWindowText(global.controls[i], L"");

	LRESULT result = SendMessage(global.controls[LIST_COMMANDS], LB_GETCURSEL, 0, 0);
	if (result == LB_ERR) {
		for (int i=TXT_COMMAND_INFO0; i<=BUTTON_CONVERT_DL; i++)
			ShowWindow(global.controls[i], SW_HIDE);
		return;
	}
	size_t sel = (size_t)result;

	ShowWindow(global.controls[TXT_DOWNLOADS], SW_HIDE);
	ShowWindow(global.controls[LIST_DOWNLOADS], SW_HIDE);
	ShowWindow(global.controls[TXT_FILENAME], SW_HIDE);
	ShowWindow(global.controls[TXT_DL_ARGS], SW_HIDE);
	ShowWindow(global.controls[LIST_DL_ARGS], SW_HIDE);
	SendMessage(global.controls[LIST_DOWNLOADS], LB_RESETCONTENT, 0, 0);

	if (sel >= global.commands.size())
		return;

	std::wstring download_file_name = L"";

	if (global.commands[sel].downloads.size() > 0) {
		ShowWindow(global.controls[TXT_DOWNLOADS], SW_SHOW);
		ShowWindow(global.controls[LIST_DOWNLOADS], SW_SHOW);

		size_t arg_size = global.commands[sel].downloads[0].arguments.size();
		if (arg_size > 0) {
			download_file_name = global.commands[sel].downloads[0].arguments[arg_size-1];
		} else {
			download_file_name = PathLastItem(global.commands[sel].downloads[0].url);
		}

		for (size_t i=0; i<global.commands[sel].downloads.size(); i++) {
			std::wstring text = Int2StrW(global.commands[sel].downloads[i].line_num) + L": " + global.commands[sel].downloads[i].url;
			SendMessageW(global.controls[LIST_DOWNLOADS], LB_ADDSTRING, 0, (LPARAM)text.c_str());
		}

		LRESULT result_dl = SendMessage(global.controls[LIST_DOWNLOADS], LB_GETCURSEL, 0, 0);
		if (result_dl != LB_ERR) {
			size_t sel_dl = (size_t)result_dl;
			if (sel_dl < global.commands[sel].downloads.size()) {
				SendMessage(global.controls[LIST_DOWNLOADS], LB_SETCURSEL, sel_dl, 0);
				ShowDownloadInfo();
			}
		}
	}

	ShowWindow(global.controls[BUTTON_OPEN_DOC], SW_SHOW);
	ShowWindow(global.controls[BUTTON_JUMP_TO_LINE], SW_SHOW);
	ShowWindow(global.controls[BUTTON_JUMP_TO_STEP], SW_SHOW);
	EnableWindow(global.controls[BUTTON_JUMP_TO_STEP], !global.commands[sel].disable && global.instruction_index!=sel);

	switch(global.commands[sel].id) {
		case COMMAND_AUTO_INSTALL : {
			std::wstring file = L"fwatch\\tmp\\" + (global.commands[sel].arguments.size()>0 ? global.commands[sel].arguments[0] : download_file_name);
			SetCommandInfo(0, L"Automatically install", file);
			SetCommandInfo(1, L"Password", global.commands[sel].password);
		} break;
		case COMMAND_UNPACK : {
			std::wstring file_name = global.commands[sel].arguments[0];

			if (Equals(file_name,L"<download>") || Equals(file_name,L"<dl>") || (file_name).empty())
				file_name = download_file_name;

			std::wstring relative_path = PathNoLastItem(file_name);

			if (Equals(relative_path.substr(0,10), L"_extracted"))
				relative_path = relative_path.substr(11) += L"_extracted";

			std::wstring destination = L"fwatch\\tmp\\_extracted";
	
			if (relative_path != L"")
				destination += L"\\" + relative_path;

			SetCommandInfo(0, L"Source", L"fwatch\\tmp\\"+file_name);
			SetCommandInfo(1, L"Destination", destination);
			SetCommandInfo(2, L"Password", global.commands[sel].password);
		} break;
		case COMMAND_COPY :
		case COMMAND_MOVE : {
			if (!global.commands[sel].arguments[0].empty()) {
				std::wstring source      = global.commands[sel].arguments[0];
				std::wstring destination = global.commands[sel].arguments[1];
				std::wstring new_name    = global.commands[sel].arguments[2];
				bool is_download_dir     = true;

				if (Equals(source,L"<download>")  ||  Equals(source,L"<dl>")) {
					source = L"fwatch\\tmp\\" + download_file_name;
				} else 
					if (Equals(source.substr(0,5),L"<mod>")) {
						source          = global.current_mod_new_name + source.substr(5);
						is_download_dir = false;
					} else
						if (Equals(source.substr(0,7),L"<game>\\")) {
							is_download_dir = false;

							if (global.commands[sel].id == COMMAND_COPY)
								source = source.substr(7);
							else {
								SetCommandInfo(0, L"Error", global.lang[STR_MOVE_DST_PATH_ERROR]);
								break;
							}
						} else
							source = L"fwatch\\tmp\\_extracted\\" + source;
				
				if (destination == L".")
					destination = L"";
					
				destination = global.current_mod_new_name + L"\\" + destination;
				
				// If user wants to move modfolder then change destination to the game directory
				if (is_download_dir && IsModName(PathLastItem(source)) && global.commands[sel].arguments[1].empty()) {
					destination = L"game dir";
					new_name    = Equals(global.current_mod,global.current_mod_new_name) ? L"" : global.current_mod_new_name;
				}

				SetCommandInfo(0, L"Source", source);
				SetCommandInfo(1, L"Destination", destination);
				SetCommandInfo(2, L"New name", new_name);

				std::wstring flags = L"";
				if (global.commands[sel].switches & SWITCH_NO_OVERWRITE)
					flags += L"don't overwrite";

				if (global.commands[sel].switches & SWITCH_MATCH_DIR_ONLY) {
					if (!flags.empty())
						flags += L", ";
					flags += L"match only folders";
				} else
					if (global.commands[sel].switches & SWITCH_MATCH_DIR) {
						if (!flags.empty())
							flags += L", ";
						flags += L"match files and folders";
					}

				SetCommandInfo(3, L"Switches", flags);
			} else {
				SetCommandInfo(0, L"Error", L"not enough arguments");
			}
		} break;
		case COMMAND_MAKEDIR : {
			SetCommandInfo(0, L"Path", global.current_mod_new_name + L"\\" + global.commands[sel].arguments[0]);
		} break;
		case COMMAND_ASK_RUN : {
			SetCommandInfo(0, L"File", global.commands[sel].arguments[0]);
		} break;
		case COMMAND_DELETE : {
			SetCommandInfo(0, L"Source", global.current_mod_new_name + L"\\" + global.commands[sel].arguments[0]);
			SetCommandInfo(1, L"Match folders", (global.commands[sel].switches & SWITCH_MATCH_DIR ? L"yes" : L""));
		} break;
		case COMMAND_RENAME : {
			std::wstring source = global.current_mod_new_name + L"\\" + global.commands[sel].arguments[0];
			std::wstring destination = PathNoLastItem(source) + global.commands[sel].arguments[1];

			SetCommandInfo(0, L"Source", source);
			SetCommandInfo(1, L"Destination", destination);
			SetCommandInfo(2, L"Match folders", (global.commands[sel].switches & SWITCH_MATCH_DIR ? L"yes" : L""));
		} break;
		case COMMAND_ASK_DOWNLOAD : {
			if (global.commands[sel].arguments.size() >= 2) {
				SetCommandInfo(0, L"File", global.commands[sel].arguments[0]);
				SetCommandInfo(1, L"URL", global.commands[sel].arguments[1]);
			} else {
				SetCommandInfo(0, L"Error", L"not enough arguments");
			}
		} break;
		case COMMAND_IF_VERSION : {
			if (global.commands[sel].arguments.size() >= 2) {
				SetCommandInfo(0, L"Operator", global.commands[sel].arguments[0]);
				SetCommandInfo(1, L"Number", global.commands[sel].arguments[1]);
			} else 
				if (global.commands[sel].arguments.size() > 0) {
					SetCommandInfo(0, L"Operator", L"==");
					SetCommandInfo(1, L"Number", global.commands[sel].arguments[0]);
				} else {
					SetCommandInfo(0, L"Error", L"not enough arguments");
				}
		} break;
		case COMMAND_MAKEPBO : {
			SetCommandInfo(0, L"Source", global.current_mod_new_name + L"\\" + global.commands[sel].arguments[0]);
			SetCommandInfo(1, L"Keep source", (global.commands[sel].switches & SWITCH_KEEP_SOURCE ? L"yes" : L"no"));

			if (!global.commands[sel].timestamp.empty())
				SetCommandInfo(2, L"Modification date", SystemTimeToReadableDate(StringToSystemTime(global.commands[sel].timestamp)));
		} break;
		case COMMAND_EXTRACTPBO : {
			std::wstring source      = global.commands[sel].arguments[0];
			std::wstring destination = global.commands[sel].arguments[1];
			std::wstring pbo_dir     = PathLastItem(global.commands[sel].arguments[0]);

			if (Equals(pbo_dir.substr(pbo_dir.length()-4),L".pbo")) {
				pbo_dir = pbo_dir.substr(0, pbo_dir.length()-4);
			}

			std::wstring a = PathNoLastItem(source);

			if (destination.empty()) {
				destination = global.current_mod_new_name + L"\\" + PathNoLastItem(source) + pbo_dir + L"\\";
			} else {
				destination = global.current_mod_new_name + L"\\" + destination + L"\\" + pbo_dir + L"\\";
			}

			if (Equals(source.substr(0,7),L"<game>\\")) {
				source = source.substr(7);
				if (global.commands[sel].arguments[1].empty())
					destination = global.current_mod_new_name + L"\\" + pbo_dir;
			} else 
				source = global.current_mod_new_name + L"\\" + source;


			SetCommandInfo(0, L"Source", source);
			SetCommandInfo(1, L"Destination", destination);
		} break;
		case COMMAND_EDIT : {
			if (global.commands[sel].arguments.size() >= 3) {
				SetCommandInfo(0, L"File", global.commands[sel].arguments[0]);
				SetCommandInfo(1, L"Line number", global.commands[sel].arguments[1]);
				SetCommandInfo(2, L"Text", global.commands[sel].arguments[2]);
				
				std::wstring switch_txt = L"";
				if (global.commands[sel].switches & SWITCH_NEWFILE)
					switch_txt = L"create new file";
				else
					if (global.commands[sel].switches & SWITCH_INSERT)
						switch_txt = L"insert line";
					else
						if (global.commands[sel].switches & SWITCH_APPEND)
							switch_txt = L"append to the end";

				if (!global.commands[sel].timestamp.empty()) {
					if (!switch_txt.empty())
						switch_txt += L"; modification date:";

					SYSTEMTIME st = StringToSystemTime(global.commands[sel].timestamp);
					switch_txt += SystemTimeToReadableDate(st);
				}

				SetCommandInfo(3, L"Switches", switch_txt);
			} else
				SetCommandInfo(0, L"Error", L"not enough arguments");
		} break;
		case COMMAND_ALIAS : {
			if (global.commands[sel].arguments.size() > 0)
				SetCommandInfo(0, L"Names", global.commands[sel].arguments[0]);
			else
				SetCommandInfo(0, L"Names", L"clear all");
		} break;
		case COMMAND_FILEDATE : {
			if (global.commands[sel].arguments.size() >= 2) {
				SetCommandInfo(0, L"File",  global.current_mod_new_name + L"\\" + global.commands[sel].arguments[0]);
				SetCommandInfo(1, L"Modification date", SystemTimeToReadableDate(StringToSystemTime(global.commands[sel].arguments[1])));
			} else {
				SetCommandInfo(0, L"Error", L"not enough arguments");
			}
		} break;
	}
}

void ShowDownloadInfo()
{
	int idc[]    = {LIST_COMMANDS, LIST_DOWNLOADS, LIST_DL_ARGS};
	size_t sel[] = {UINT_MAX, UINT_MAX, UINT_MAX};

	for (int i=0; i<sizeof(idc)/sizeof(idc[0]); i++) {
		LRESULT result = SendMessage(global.controls[idc[i]], LB_GETCURSEL, 0, 0);
		if (result != LB_ERR)
			sel[i] = (size_t)result;
	}

	ShowWindow(global.controls[TXT_DL_ARGS], SW_HIDE);
	ShowWindow(global.controls[LIST_DL_ARGS], SW_HIDE);
	ShowWindow(global.controls[TXT_FILENAME], SW_SHOW);

	size_t arguments_number        = global.commands[sel[0]].downloads[sel[1]].arguments.size();
	std::wstring download_filename = PathLastItem(UnQuote(global.commands[sel[0]].downloads[sel[1]].url));

	if (arguments_number > 0) {
		if (arguments_number > 1) {
			ShowWindow(global.controls[TXT_DL_ARGS], SW_SHOW);
			ShowWindow(global.controls[LIST_DL_ARGS], SW_SHOW);
			SendMessage(global.controls[LIST_DL_ARGS], LB_RESETCONTENT, 0, 0);

			for (size_t i=0; i<arguments_number-1; i++) 
				SendMessageW(global.controls[LIST_DL_ARGS], LB_ADDSTRING, 0, (LPARAM)global.commands[sel[0]].downloads[sel[1]].arguments[i].c_str());
		}

		download_filename = global.commands[sel[0]].downloads[sel[1]].arguments[arguments_number-1];
	}

	std::wstring temp = L"File name: " + download_filename;
	SetWindowText(global.controls[TXT_FILENAME], temp.c_str());
}

void SwitchTab(INSTALLER_TAB tab)
{
	for (int i=LOG_GENERAL; i<CONTROLS_MAX; i++)
		ShowWindow(global.controls[i], SW_HIDE);

	switch(tab) {
		case INSTALLER_TAB_LOG : {
			ShowWindow(global.controls[LOG_GENERAL], SW_SHOW);
			ShowWindow(global.controls[LOG_DETAIL], SW_SHOW);
		} break;

		case INSTALLER_TAB_INSTRUCTIONS : {
			for (int i=LOG_DETAIL; i<=BUTTON_JUMP_TO_LINE; i++)
				ShowWindow(global.controls[i], SW_SHOW);
			ShowCommandInfo();
		} break;

		case INSTALLER_TAB_SCRIPT : {
			for (int i=EDIT_SCRIPT; i<CONTROLS_MAX; i++)
				ShowWindow(global.controls[i], SW_SHOW);
			SetFocus(global.controls[EDIT_SCRIPT]);
		} break;
	}

	TabCtrl_SetCurSel(global.controls[TAB], tab);
}

void WindowTextToString(HWND control, std::wstring &str)
{
	int length = GetWindowTextLength(control);
	str.reserve(length+1);
	str.resize(length);
	GetWindowText(control, &str[0], length+1);
}

void SumDownloadSizes(std::vector<LARGE_INTEGER> &download_sizes, size_t instruction_index)
{
	__int64 bytes = 0;

	for (size_t i=0; i<download_sizes.size() && i<instruction_index; i++)
		bytes += download_sizes[i].QuadPart;

    wchar_t names[][3] = {L"B", L"KB", L"MB", L"GB"};
	std::wstring formatted = L"Total Download Size: ";
    
    if (bytes > 0) {
		__int64 current_unit = 1073741824;

        for (int i=sizeof(names)/sizeof(names[0])-1; i>=0; i--, current_unit/=1024) {
            if (bytes >= current_unit) {
                __int64 multiples = bytes / current_unit;
                bytes -= multiples * current_unit;
    
				#define round_macro(x) floor(x+0.5)
				double rounded = (double)multiples + (round_macro((double)bytes / (double)current_unit * 10.0) / 10.0);

                formatted += Double2StrW(rounded) + L" " + (std::wstring)names[i];
                break;
            }
        }
    } else
        formatted += L"0 " + (std::wstring)names[0];

	SetWindowText(global.controls[TXT_DL_SIZE], formatted.c_str());
}

void FillCommandsList() {
	SendMessage(global.controls[LIST_COMMANDS], LB_RESETCONTENT, 0, 0);

	for (size_t i=0; i<global.commands_lines.size(); i++)
		SendMessageW(global.controls[LIST_COMMANDS], LB_ADDSTRING, 0, (LPARAM)global.commands_lines[i].c_str());
}

bool ReadTextInputs() {
	std::wstring new_mod = L"";
	WindowTextToString(global.controls[INPUT_MOD_NAME], new_mod);
	Trim(new_mod);

	if (new_mod.empty()) {
		MessageBox(global.window, L"Mod name cannot be empty!", L"Addon Installer", MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

	wchar_t is_forbidden = VerifyWindowsFileName(new_mod);
	if (is_forbidden != NULL) {
		std::wstring msg = L"Mod name cannot contain |";
		msg[msg.length()-1] = is_forbidden;
		MessageBox(global.window, msg.c_str(), L"Addon Installer", MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

	std::wstring new_dir_name = L"";
	WindowTextToString(global.controls[INPUT_DIR_NAME], new_dir_name);
	Trim(new_dir_name);

	if (!new_dir_name.empty()) {
		is_forbidden = VerifyWindowsFileName(new_dir_name);
		if (is_forbidden != NULL) {
			std::wstring msg = L"Dir name cannot contain |";
			msg[msg.length()-1] = is_forbidden;
			MessageBox(global.window, msg.c_str(), L"Addon Installer", MB_OK | MB_ICONEXCLAMATION);
			return false;
		}
	}

	std::wstring new_game_version = L"";
	WindowTextToString(global.controls[INPUT_GAME_VER], new_game_version);
	Trim(new_game_version);

	if (new_game_version != global.game_version) {
		if (new_game_version.empty()) {
			MessageBox(global.window, L"Game version cannot be empty!", L"Addon Installer", MB_OK | MB_ICONEXCLAMATION);
			global.order = ORDER_NONE;
			return false;
		}

		bool is_number = true;
		for (size_t i=0; i<new_game_version.length(); i++) {
			if (!iswdigit(new_game_version[i]) && new_game_version[i]!=L'.') {
				is_number = false;
				break;
			}
		}

		if (!is_number) {
			MessageBox(global.window, L"Game version must be a number", L"Addon Installer", MB_OK | MB_ICONEXCLAMATION);
			global.order = ORDER_NONE;
			return false;
		}
	}

	global.game_version = new_game_version;
	global.current_mod = new_mod;

	if (new_dir_name.empty())
		global.current_mod_new_name = global.current_mod;
	else
		global.current_mod_new_name = new_dir_name;

	// Save config
	HANDLE hFile = CreateFile(PATH_TO_TEST_CFG, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile != INVALID_HANDLE_VALUE) {
		int id_to_write[] = {INPUT_MOD_NAME, INPUT_DIR_NAME, INPUT_GAME_VER};
		DWORD bytesWritten;

		for (int i=0; i<sizeof(id_to_write)/sizeof(id_to_write[0]); i++) {
			std::wstring text = L"";
			WindowTextToString(global.controls[id_to_write[i]], text);

			WriteFile(hFile, &id_to_write[i], sizeof(id_to_write[0]), &bytesWritten, NULL);
			DWORD length = (DWORD)(text.length()+1) * sizeof(wchar_t);
			WriteFile(hFile, &length, sizeof(length), &bytesWritten, NULL);
			WriteFile(hFile, text.c_str(), length, &bytesWritten, NULL);
		}

		CloseHandle(hFile);
	}

	return true;
}
// -------------------------------------------------------------------------------------------------------