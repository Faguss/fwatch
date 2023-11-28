#pragma once
#include "common.h"

// String operations -------------------------------------------------------------------------------------
std::wstring utf16(std::string input);
std::string utf8(const wchar_t* input, int input_size);
std::string utf8(std::wstring input);
std::string Trim(std::string s);
std::wstring Trim(std::wstring s);
std::wstring WrapInQuotes(std::wstring text);
std::string UnQuote(std::string text);
std::wstring UnQuote(std::wstring text);
std::wstring MaskNewName(std::wstring path, std::wstring mask);
std::wstring PathLastItem(std::wstring path);
std::wstring PathNoLastItem(std::wstring path, int options=FLAG_NONE);
std::wstring FormatError(DWORD error);
void Tokenize(std::string text, std::string delimiter, std::vector<std::string> &container);
void Tokenize(std::wstring text, std::wstring delimiter, std::vector<std::wstring> &container);
std::wstring ReplaceAll(std::wstring str, const std::wstring& from, const std::wstring& to);
bool Equals(const std::string& a, const std::string& b);
bool Equals(const std::wstring& a, const std::wstring& b);
bool VerifyPath(std::wstring path);
std::wstring Int2StrW(int num, bool leading_zero=false);
std::wstring UInt2StrW(size_t num);
std::wstring Double2StrW(double num);
WORD Str2Short(std::string num);
bool IsURL(std::wstring text);
bool IsModName(std::wstring filename);
std::wstring GetTextBetween(std::wstring &buffer, std::wstring start, std::wstring end, size_t &offset, bool reverse=false);
std::wstring UrlEncode(const std::wstring &value);
std::wstring Lowercase(std::wstring &input);
std::wstring GetFileExtension(std::wstring file_name);
wchar_t VerifyWindowsFileName(std::wstring &file_name);
SYSTEMTIME StringToSystemTime(std::wstring input);
std::wstring SystemTimeToReadableDate(SYSTEMTIME st);
// -------------------------------------------------------------------------------------------------------




// Installer operations ----------------------------------------------------------------------------------
void WriteProgressFile(INSTALLER_STATUS status, std::wstring input);
int WriteModID(std::wstring input_path, std::wstring mod, std::wstring content, std::wstring content2);
INSTALLER_ERROR_CODE ErrorMessage(int string_code, std::wstring message=L"%STR%", INSTALLER_ERROR_CODE error_code=ERROR_COMMAND_FAILED);
void LogMessage(std::wstring input, bool close=false);
void LogMessageDate(bool start_date=true);
std::wstring GetFileContents(std::wstring filename);
size_t ParseInstallationScript(std::wstring &script_file_content, std::vector<Command> &output, bool compare_old_with_new=false);
int ParseWgetLog(std::string &error);
int ParseUnpackLog(std::string &error, std::wstring &file_name);
int ParsePBOLog(std::string &message, std::wstring &exename, std::wstring &file_name);
std::wstring GetMissionDestinationFromSQM(std::wstring path);
static int CALLBACK BrowseCallbackProc(HWND hwnd,UINT uMsg, LPARAM lParam, LPARAM lpData);
std::wstring BrowseFolder(std::wstring saved_path);
DIRECTORY_INFO ScanDirectory(std::wstring path);
DWORD DeleteDirectory(const std::wstring &refcstrRootDirectory, bool bDeleteSubdirectories=true);//func,inst
INSTALLER_ERROR_CODE CreateFileList(std::wstring source, std::wstring destination, std::vector<std::wstring> &sources, std::vector<std::wstring> &destinations, std::vector<bool> &dirs, int options, std::vector<std::wstring> &empty_dirs, size_t &buffer_size, int &recursion);
INSTALLER_ERROR_CODE CreateTimestampList(std::wstring path, size_t path_cut, std::vector<std::wstring> &namelist, std::vector<time_t> &timelist);
INSTALLER_ERROR_CODE Download(std::wstring url, int options=FLAG_NONE, std::wstring log_file_name=L"");
INSTALLER_ERROR_CODE Unpack(std::wstring file_name, std::wstring password=L"", int options=FLAG_NONE);//func,inst
INSTALLER_ERROR_CODE MakeDir(std::wstring path, int options=FLAG_NONE);//inst,func
INSTALLER_ERROR_CODE MoveFiles(std::wstring source, std::wstring destination, std::wstring new_name, int options);
INSTALLER_ERROR_CODE ExtractPBO(std::wstring source, std::wstring destination=L"", std::wstring file_to_unpack=L"", bool silent=false);
INSTALLER_ERROR_CODE ChangeFileDate(std::wstring file_name, FILETIME *ft);
INSTALLER_ERROR_CODE ChangeFileDate(std::wstring file_name, time_t timestamp);
INSTALLER_ERROR_CODE ChangeFileDate(std::wstring file_name, std::wstring timestamp);
INSTALLER_ERROR_CODE RequestExecution(std::wstring path_to_dir, std::wstring file_name);
INSTALLER_ERROR_CODE Condition_If_version(const std::vector<std::wstring> &arguments);
INSTALLER_ERROR_CODE Condition_Else();
INSTALLER_ERROR_CODE Condition_Endif();
INSTALLER_ERROR_CODE Auto_Install(std::wstring file, DWORD attributes, int options=FLAG_NONE, std::wstring password=L"");
void EndModVersion();
void EndMod();
void RollBackInstallation(size_t wanted_pos=UINT_MAX);
void ResetInstallationState();
HWND GetWindowHandle(DWORD input_pid);
// -------------------------------------------------------------------------------------------------------




// Interface ---------------------------------------------------------------------------------------------
void EditMultilineUpdateText(HWND control, std::wstring &text);
void DisableMenu();
void SetCommandInfo(int index, std::wstring title, std::wstring content);
void ShowCommandInfo();
void ShowDownloadInfo();
void SwitchTab(INSTALLER_TAB tab);
void WindowTextToString(HWND control, std::wstring &str);
void SumDownloadSizes(std::vector<LARGE_INTEGER> &download_sizes, size_t instruction_index);
void FillCommandsList();
// -------------------------------------------------------------------------------------------------------