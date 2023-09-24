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
std::string Int2Str(int num);
std::wstring Int2StrW(int num, bool leading_zero=false);
std::wstring UInt2StrW(size_t num);
size_t Str2UInt(std::string num);
std::wstring Float2StrW(double num);
WORD Str2Short(std::string num);
bool IsURL(std::wstring text);
bool IsModName(std::wstring filename);
std::wstring GetTextBetween(std::wstring &buffer, std::wstring start, std::wstring end, size_t &offset, bool reverse=false);
std::wstring url_encode(const std::wstring &value);
std::wstring lowercase(std::wstring &input);
// -------------------------------------------------------------------------------------------------------




// Installer messaging -----------------------------------------------------------------------------------
void WriteProgressFile(int status, std::wstring input);
int WriteModID(std::wstring modfolder, std::wstring content, std::wstring content2);
DWORD ErrorMessage(int string_code, std::wstring message=L"%STR%", int error_code=ERROR_COMMAND_FAILED);
DWORD WINAPI ReceiveInstructions(__in LPVOID lpParameter);
int isAborted();
HWND GetWindowHandle(DWORD input_pid);
void EditMultilineUpdateText(HWND control, std::wstring &text);
// -------------------------------------------------------------------------------------------------------




// File reading ------------------------------------------------------------------------------------------
std::wstring GetFileContents(std::wstring &filename);
std::wstring GetFileExtension(std::wstring file_name);
int ParseWgetLog(std::string &error);
int ParseUnpackLog(std::string &error, std::wstring &file_name);
int ParsePBOLog(std::string &message, std::wstring &exename, std::wstring &file_name);
DWORD CreateFileList(std::wstring source, std::wstring destination, std::vector<std::wstring> &sources, std::vector<std::wstring> &destinations, std::vector<bool> &dirs, int options, std::vector<std::wstring> &empty_dirs, size_t &buffer_size, int &recursion);
std::wstring GetMissionDestinationFromSQM(std::wstring path);
static int CALLBACK BrowseCallbackProc(HWND hwnd,UINT uMsg, LPARAM lParam, LPARAM lpData);
std::wstring BrowseFolder(std::wstring saved_path);
DIRECTORY_INFO ScanDirectory(std::wstring path);
// -------------------------------------------------------------------------------------------------------




// File writing ------------------------------------------------------------------------------------------
DWORD DeleteDirectory(const std::wstring &refcstrRootDirectory, bool bDeleteSubdirectories=true);
int Download(std::wstring url, int options=FLAG_NONE, std::wstring log_file_name=L"");
int Unpack(std::wstring file_name, std::wstring password=L"", int options=FLAG_NONE);
int MakeDir(std::wstring path, int options=FLAG_NONE);
int MoveFiles(std::wstring source, std::wstring destination, std::wstring new_name, int options);
int ExtractPBO(std::wstring source, std::wstring destination=L"", std::wstring file_to_unpack=L"", bool silent=false);
int ChangeFileDate(std::wstring file_name, FILETIME *ft);
int ChangeFileDate(std::wstring file_name, time_t timestamp);
int ChangeFileDate(std::wstring file_name, std::wstring timestamp);
int CreateTimestampList(std::wstring path, size_t path_cut, std::vector<std::wstring> &namelist, std::vector<time_t> &timelist);
void LogMessage(std::wstring input, bool close=false);
// -------------------------------------------------------------------------------------------------------




// Installer commands that get reused --------------------------------------------------------------------
int RequestExecution(std::wstring path_to_dir, std::wstring file_name);
int Condition_If_version(const std::vector<std::wstring> &arg, int arg_id, int arg_num);
int Condition_Else();
int Condition_Endif();
int Auto_Install(std::wstring file, DWORD attributes, int options=FLAG_NONE, std::wstring password=L"");
void EndModVersion();
void EndMod();