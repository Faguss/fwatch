#pragma once
#include <fstream>		// file operations
#include <windows.h>	// winapi
#include <tlhelp32.h>	// process/module traversing
#include <vector>       // dynamic array
#include <algorithm>	// tolower
#include <sstream>      // for converting int to string
#include <Shlobj.h>		// opening explorer
#include <map>			// associative array for arguments
#include <time.h>		// get current time as unix timestamp
#include <iomanip>		// for url encode
#include <ctype.h>		// isspace
#include <process.h>	// threads
#include <tchar.h>		// ansi/wide macro
#include <functional>   // notl, ptr_fun
#include <shellapi.h>	// shell execute
#include <cmath>        // round

enum WINDOW_CONTROLS {
	TAB,
	GRAY_BACKGROUND,
	LOG_GENERAL,
	LOG_DETAIL,

	TXT_MOD_NAME,
	INPUT_MOD_NAME,
	TXT_DIR_NAME,
	INPUT_DIR_NAME,
	TXT_GAME_VER,
	INPUT_GAME_VER,
	TXT_COMMANDS,
	TXT_DL_SIZE,
	LIST_COMMANDS,
	BUTTON_OPEN_MOD,
	BUTTON_OPEN_TMP,
	BUTTON_REWIND,
	BUTTON_BACK,
	BUTTON_NEXT,
	BUTTON_PLAY,
	TESTING_SEPARATOR,
	TXT_COMMAND_INFO0,
	TXT_COMMAND_INFO1,
	TXT_COMMAND_INFO2,
	TXT_COMMAND_INFO3,
	TXT_DOWNLOADS,
	LIST_DOWNLOADS,
	TXT_FILENAME,
	TXT_DL_ARGS,
	LIST_DL_ARGS,
	BUTTON_JUMP_TO_STEP,
	BUTTON_JUMP_TO_LINE,
	BUTTON_OPEN_DOC,

	EDIT_SCRIPT,
	TXT_LINE_NUMBER,
	BUTTON_SAVETEST,
	BUTTON_RELOAD,
	BUTTON_OPEN_DOC_GENERAL,
	BUTTON_CONVERT_DL,
	BUTTON_INSERT_DTA,

	CONTROLS_MAX
};

enum INSTALLER_TAB {
	INSTALLER_TAB_LOG,
	INSTALLER_TAB_INSTRUCTIONS,
	INSTALLER_TAB_SCRIPT
};

enum INSTALLER_OPERATION {
	OPERATION_NONE,
	OPERATION_MOVE,
	OPERATION_DELETE,
	OPERATION_DELETE_DIR,
	OPERATION_FILEDATE,
	OPERATION_ALIAS
};

struct INSTALLER_OPERATION_LOG {
	size_t instruction_index;
	INSTALLER_OPERATION operation_type;
	std::wstring source;
	std::wstring destination;
	FILETIME modif_time;
};

enum INSTALLER_ORDER {
	ORDER_NONE,
	ORDER_PREV,
	ORDER_NEXT,
	ORDER_REWIND,
	ORDER_PLAY,
	ORDER_PAUSE,
	ORDER_RELOAD,
	ORDER_RETRY,
	ORDER_ABORT,
	ORDER_JUMP
};

enum INSTALLER_STATUS 
{
	INSTALL_PROGRESS,
	INSTALL_WAITINGFORUSER,
	INSTALL_RETRYORABORT,
	INSTALL_PAUSED,
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
	FLAG_CLEAN_DL_LATER  = 0x2000,
	FLAG_CONTINUE        = 0x4000,
	FLAG_DONT_BACKUP     = 0x8000
};

enum INSTALLER_ERROR_CODE
{
	ERROR_NONE,
	ERROR_USER_ABORTED,
	ERROR_LOGFILE,
	ERROR_NO_SCRIPT,
	ERROR_COMMAND_FAILED,
	ERROR_WRONG_SCRIPT,
	ERROR_WRONG_ARCHIVE,
	ERROR_PAUSED
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
	STR_ASK_RETRYORABORT,
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

enum MOD_SUBFOLDERS {
	DIR_NONE,
	DIR_ADDONS,
	DIR_BIN,
	DIR_CAMPAIGNS,
	DIR_DTA,
	DIR_MISSIONS,
	DIR_MPMISSIONS,
	DIR_TEMPLATES,
	DIR_SPTEMPLATES,
	DIR_MISSIONSUSERS,
	DIR_MPMISSIONSUSERS,
	DIR_ISLANDCUTSCENES,
	DIR_MAX
};
	
struct DIRECTORY_INFO {
	int error_code;
	int number_of_files;
	int number_of_dirs;
	int number_of_wanted_mods;
	int number_of_mod_subfolders;
	std::vector<std::wstring> file_list;
	std::vector<DWORD> attributes_list;
	std::vector<int> mod_sub_id_list;
	std::vector<bool> is_mod_list;
};

enum INSTALLER_COMMAND_ID {
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
	COMMAND_EXIT,
	COMMAND_MAX
};

struct DownloadURL {
	int line_num;
	std::wstring url;
	std::vector<std::wstring> arguments;
};

struct Command {
	INSTALLER_COMMAND_ID id;
	int line_num;
	int step_num;
	int switches;
	bool ctrl_flow;
	bool disable;
	std::wstring name;
	std::wstring password;
	std::wstring timestamp;
	std::vector<std::wstring> arguments;
	std::vector<DownloadURL> downloads;
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

enum INSTALLATION_PHASE {
	PHASE_WAITING,
	PHASE_DOWNLOADING,
	PHASE_EXECUTING
};

struct GLOBAL_VARIABLES
{
	HWND window;
	HANDLE thread_installer;
	HANDLE thread_receiver;
	HMENU window_menu;
	bool test_mode;
	bool skip_modfolder;
	bool restart_game;
	bool run_voice_program;
	bool last_download_attempt;
	int condition_index;
	int command_line_num;
	int installation_steps_max;
	INSTALLATION_PHASE installation_phase;
	INSTALLER_ORDER order;
	size_t saved_alias_array_size;
	size_t download_iterator;
	size_t instruction_index;
	time_t current_mod_version_date;
	double installer_version;
	double script_version;
	std::wstring program_arguments;
	std::wstring buffer_log;
	std::wstring buffer_status;
	std::wstring gamerestart_arguments;
	std::wstring downloaded_filename;
	std::wstring current_mod;
	std::wstring missing_modfolders;
	std::wstring last_pbo_file;
	std::wstring working_directory;
	std::wstring current_mod_new_name;
	std::wstring current_mod_version;
	std::wstring current_mod_id;
	std::wstring current_mod_keepname;
	std::wstring downloaded_filename_last;
	std::wstring last_log_message;
	std::wstring game_version;
	std::vector<bool> condition;
	std::vector<std::wstring> downloads;
	std::vector<std::wstring> mod_id;
	std::vector<std::wstring> mod_name;
	std::vector<std::wstring> current_mod_alias;
	const std::wstring *lang;
	const std::wstring *lang_eng;
	std::map<std::wstring, std::wstring> arguments_table;
	std::ofstream logfile;
	std::vector<INSTALLER_OPERATION_LOG> rollback;
	HWND controls[CONTROLS_MAX];
	std::vector<Command> commands;
	std::vector<std::wstring> commands_lines;
};

extern const std::wstring command_names[];
extern const int match_command_name_to_id[];
extern const int control_flow_commands[];
extern const size_t command_minimal_arg[];
extern const std::wstring command_switches_names[];
extern const std::wstring mod_subfolders[];
extern GLOBAL_VARIABLES global;
extern const std::wstring stringtable[][STR_MAX];

#define ID_BASE              200
#define DOCUMENTATION_URL    L"https://ofp-faguss.com/schedule/install_scripts"
#define PATH_TO_TEST_CFG     L"fwatch\\data\\addonInstaller_cfg.bin"
#define PATH_TO_TEST_SCRIPT  L"fwatch\\data\\addonInstaller_test.txt"
#define LEADING_ZERO         true
#define CLOSE_LOG            true
#define COMPARE_OLD_WITH_NEW true
#define DATE_START           true
#define DATE_END             false