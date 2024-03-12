#pragma once
#include <fstream>		// file operations
#include <windows.h>	// winapi
#include <tlhelp32.h>	// process/module traversing
#include <sstream>      // for converting int to string
#include <vector>       // dynamic array
#include <algorithm>	// tolower
#include <Shlobj.h>		// opening explorer
#include <functional>   // notl, ptr_fun
#include <tchar.h>		// ansi/wide macro
#include <shellapi.h>   // shellexecute

// windows task scheduler
#include <initguid.h>
#include <ole2.h>
#include <mstask.h>
#include <msterr.h>
#include <comdef.h>

const size_t fwatch_build_date_size = 64;

struct GLOBAL_VARIABLES 
{
	char fwatch_build_date[fwatch_build_date_size];
	std::wstring downloaded_filename;
	std::wstring working_directory;
	std::wstring display_buffer;
	HINSTANCE program;
	HWND window;
	HWND dialog_text_field;
	LPWSTR program_arguments;
	std::ofstream logfile;
};

extern GLOBAL_VARIABLES global;

struct GameInfo
{
	HWND handle;
	DWORD pid;
};

struct BinarySearchResult {
	size_t index;
	bool found;
};

#define FNV_PRIME 16777619u
#define FNV_BASIS 2166136261u

enum OPTIONS_FNVHASH {
	OPTION_LOWERCASE = 1
};

enum OPTIONS_INT2STR {
	OPTION_LEADINGZERO = 1
};

enum OPTIONS_WRITEANDSHOWMSG {
	OPTION_CLOSELOG = 1
};

struct MODLIST {
	std::wstring folder_name;
	std::wstring real_name;
	std::wstring id;
	std::wstring version;
	bool force_name;
};

struct INPUT_ARGUMENTS
{
	std::wstring user_arguments;
	std::wstring user_arguments_log;
	std::wstring game_exe;
	std::wstring server_uniqueid;
	std::wstring PBOaddon;
	std::wstring voice_server;
	std::wstring event_url;
	std::wstring update_resource;
	std::wstring username;
	std::wstring maxcustom;
	std::wstring ip;
	std::wstring port;
	std::wstring event_task_name;
	char fwatch_build_date[64];
	bool server_equalmodreq;
	bool steam;
	bool skip_memory_arguments;
	int self_update;
	int event_voice;
	DWORD game_pid;
	std::vector<MODLIST> mods;	
};

struct WINDOWS_TASK_SCHEDULER {
	std::wstring task_name;
	BOOL COM_initialized;
	ITaskScheduler *scheduler;
	ITask *task;
	ITaskTrigger *trigger_interface;
	TASK_TRIGGER trigger;
	LPWSTR comment;
	HRESULT result;
};

enum MOD_SIGNATURE_FILE {
	MOD_ID,
	MOD_VER,
	MOD_DATE,
	MOD_NAME,
	MOD_FORCENAME,
	MOD_SIZE
};

enum OPTIONS_FORMATMESSAGEARRAY {
	OPTION_LOGFILE,
	OPTION_MESSAGEBOX
};

enum OPTIONS_SYSTEMTIMETOSTRING {
	OPTION_WITHOUT_SECONDS,
	OPTION_WITH_SECONDS
};

enum OPTIONS_SELF_UPDATE {
	SELF_UPDATE_DISABLED,
	SELF_UPDATE_AND_START_GAME,
	SELF_UPDATE_AND_START_ITSELF
};

enum MESSAGES_ARRAY {
	MSG_TOP,
	MSG_BOTTOM,
	MSG_SIZE
};

enum COMPARE_TIME_RESULT {
	PAST    = -1,
	PRESENT = 0,
	FUTURE  = 1
};

enum OPTIONS_FORMATSYSTEMTIME {
	OPTION_NOTHING,
	OPTION_DATE,
	OPTION_TIME
};