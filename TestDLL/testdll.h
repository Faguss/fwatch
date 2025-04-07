// TestDLL header file.
#ifdef TESTDLL_EXPORTS
#define TESTDLL_API __declspec(dllexport)
#else
#define TESTDLL_API __declspec(dllimport)
#endif

#include <process.h>
#include <stdio.h>

enum RESTORE_MEM_COMMANDS
{
	RESTORE_BRIGHTNESS,		//0
	RESTORE_OBJECT_SHADOWS,	//1
	RESTORE_VEHICLE_SHADOWS,//2
	RESTORE_CLOUDLETS,		//3
	RESTORE_BULLETS,		//4
	RESTORE_CADET = 14,		//14
	RESTORE_VETERAN = 26,	//26
	RESTORE_RADAR = 38,		//38
	RESTORE_MAX_OBJECTS,	//39
	RESTORE_TRACK1,			//40
	RESTORE_TRACK2,			//41
	RESTORE_MAX_LIGHTS,		//42
	RESTORE_TIDE,			//43
	RESTORE_WAVE,			//44
	RESTORE_EXTCAMPOS,		//45
	RESTORE_WAVE_SPEED,		//46
	RESTORE_FOVLEFT,		//47
	RESTORE_FOVTOP,			//48
	RESTORE_UITOPLEFTX,		//49
	RESTORE_UITOPLEFTY,		//50
	RESTORE_UIBOTTOMRIGHTX,	//51
	RESTORE_UIBOTTOMRIGHTY,	//52
	RESTORE_HUD				//53
};
enum RESTORE_MEM_INTEGERS
{
	INT_MAX_OBJECTS,	//0
	INT_MAX_LIGHTS		//1
};
enum RESTORE_MEM_FLOATS
{
	FLOAT_BRIGHTNESS,		//0
	FLOAT_BULLETS,			//1
	FLOAT_RADAR = 11,		//11
	FLOAT_TRACK1,			//12
	FLOAT_TRACK2,			//13
	FLOAT_TIDE,				//14
	FLOAT_WAVE,				//15
	FLOAT_EXTCAMX,			//16
	FLOAT_EXTCAMY,			//17
	FLOAT_EXTCAMZ,			//18
	FLOAT_WAVE_SPEED,		//19
	FLOAT_FOVLEFT,			//20
	FLOAT_FOVTOP,			//21
	FLOAT_UITOPLEFTX,		//22
	FLOAT_UITOPLEFTY,		//23
	FLOAT_UIBOTTOMRIGHTX,	//24
	FLOAT_UIBOTTOMRIGHTY	//25
};
enum RESTORE_MEM_BYTES
{
	BYTE_OBJECT_SHADOWS,	//0
	BYTE_VEHICLE_SHADOWS,	//1
	BYTE_CLOUDLETS,			//2
	BYTE_CADET,				//3
	BYTE_VETERAN = 15		//15
};
enum GAME_VERSION 
{
	VER_UNKNOWN,
	VER_196,
	VER_199,
	VER_201,
	VER_196_SERVER,
	VER_199_SERVER,
	VER_201_SERVER
};

// Variables for UI position
//ofp: [[0x79F8D0] + 0x8] + 
//cwa: [[0x78E9C8] + 0x8] + 
enum UI_ELEMENTS
{
	ACTION_X,		// 0x368
	ACTION_Y,		// 0x36C
	ACTION_W,		// 0x370
	ACTION_H,		// 0x374
	ACTION_ROWS,	// 0x378
	ACTION_COLORBACK,	//0x37C
	ACTION_COLORTEXT,	//0x380
	ACTION_COLORSEL,	//0x384
	ACTION_FONT,	// 0x388
	ACTION_SIZE,	// 0x38C
	RADIOMENU_X,	// 0x394
	RADIOMENU_Y,	// 0x398
	RADIOMENU_W,	// 0x39C
	RADIOMENU_H,	// 0x3A0
	TANK_X,			// 0x3A4
	TANK_Y,			// 0x3A8
	TANK_W,			// 0x3AC
	TANK_H,			// 0x3B0
	RADAR_X,		// 0x3B4
	RADAR_Y,		// 0x3B8
	RADAR_W,		// 0x3BC
	RADAR_H,		// 0x3C0
	COMPASS_X,		// 0x3C4
	COMPASS_Y,		// 0x3C8
	COMPASS_W,		// 0x3CC
	COMPASS_H,		// 0x3D0
	HINT_X,			// 0x3E4
	HINT_Y,			// 0x3E8
	HINT_W,			// 0x3EC
	HINT_H,			// 0x3F0
	LEADER_X,		// 0x3F4
	LEADER_Y,		// 0x3F8
	LEADER_W,		// 0x3FC
	LEADER_H,		// 0x400
	GROUPDIR_X,		// 0x404
	GROUPDIR_Y,		// 0x408
	GROUPDIR_W,		// 0x40C
	GROUPDIR_H,		// 0x410
	CHAT_X,
	CHAT_Y,
	CHAT_W,
	CHAT_H,
	CHAT_ROWS,
	CHAT_COLORGLOBAL,
	CHAT_COLORSIDE,
	CHAT_COLORTEAM,
	CHAT_COLORVEHICLE,
	CHAT_COLORDIRECT,
	CHAT_COLORBACK,
	CHAT_FONT,
	CHAT_SIZE,
	CHAT_ENABLE,
	ARRAY_SIZE
};

static const char hud_names[][40] = 
{
	"ACTION_X",
	"ACTION_Y",
	"ACTION_W",
	"ACTION_H",
	"ACTION_ROWS",
	"ACTION_COLORBACK",
	"ACTION_COLORTEXT",
	"ACTION_COLORSEL",
	"ACTION_FONT",
	"ACTION_SIZE",
	"RADIOMENU_X",
	"RADIOMENU_Y",
	"RADIOMENU_W",
	"RADIOMENU_H",
	"TANK_X",
	"TANK_Y",
	"TANK_W",
	"TANK_H",
	"RADAR_X",
	"RADAR_Y",
	"RADAR_W",
	"RADAR_H",
	"COMPASS_X",
	"COMPASS_Y",
	"COMPASS_W",
	"COMPASS_H",
	"HINT_X",
	"HINT_Y",
	"HINT_W",
	"HINT_H",
	"LEADER_X",
	"LEADER_Y",
	"LEADER_W",
	"LEADER_H",
	"GROUPDIR_X",
	"GROUPDIR_Y",
	"GROUPDIR_W",
	"GROUPDIR_H",
	"CHAT_X",
	"CHAT_Y",
	"CHAT_W",
	"CHAT_H",
	"CHAT_ROWS",
	"CHAT_COLORGLOBAL",
	"CHAT_COLORSIDE",
	"CHAT_COLORTEAM",
	"CHAT_COLORVEHICLE",
	"CHAT_COLORDIRECT",
	"CHAT_COLORBACK",
	"CHAT_FONT",
	"CHAT_SIZE",
	"CHAT_ENABLE"
};

static const int hud_int_list[] = 
{
	ACTION_ROWS,
	ACTION_COLORBACK,
	ACTION_COLORTEXT,
	ACTION_COLORSEL,
	ACTION_FONT,
	CHAT_ROWS,
	CHAT_COLORGLOBAL,
	CHAT_COLORSIDE,
	CHAT_COLORTEAM,
	CHAT_COLORVEHICLE,
	CHAT_COLORDIRECT,
	CHAT_COLORBACK,
	CHAT_FONT,
	CHAT_ENABLE
};

static const int hud_color_list[] = 
{
	ACTION_COLORBACK,
	ACTION_COLORTEXT,
	ACTION_COLORSEL,
	CHAT_COLORGLOBAL,
	CHAT_COLORSIDE,
	CHAT_COLORTEAM,
	CHAT_COLORVEHICLE,
	CHAT_COLORDIRECT,
	CHAT_COLORBACK
};

static const int hud_offset[] = 
{
	0x368,
	0x36C,
	0x370,
	0x374,
	0x378,
	0x37C,
	0x380,
	0x384,
	0x388,
	0x38C,
	0x394,
	0x398,
	0x39C,
	0x3A0,
	0x3A4,
	0x3A8,
	0x3AC,
	0x3B0,
	0x3B4,
	0x3B8,
	0x3BC,
	0x3C0,
	0x3C4,
	0x3C8,
	0x3CC,
	0x3D0,
	0x3E4,
	0x3E8,
	0x3EC,
	0x3F0,
	0x3F4,
	0x3F8,
	0x3FC,
	0x400,
	0x404,
	0x408,
	0x40C,
	0x410,
	0x0, 
	0x4, 
	0x8,
	0xC,
	0x10,
	0x14,
	0x18,
	0x1C,
	0x20,
	0x24,
	0x28,
	0x2C,
	0x30,
	0x34
};

static const int hud_offset_num = sizeof(hud_offset) / sizeof(hud_offset[0]);


// Global variables used in TestDLL
struct GLOBAL_VARIABLES_TESTDLL {
	int exe_index;
	DWORD exe_address;
	DWORD exe_address_scroll;
	DWORD exe_address_ifc22;
	bool is_server;

	bool restore_memory[105];
	bool restore_byte[27];
	bool nomap;
	bool ErrorLog_Enabled;
	bool ErrorLog_Started;

	int option_error_output;

	TCHAR game_dir[MAX_PATH];
	size_t game_dir_length;
	char mission_path[256];
	size_t mission_path_length;
	char mission_path_previous[256];
	size_t mission_path_previous_length;
	DWORD mission_path_savetime;
	DWORD pid;
	int external_program_id;
	DWORD lastWget;

	int extCamOffset;
	int restore_int[2];
	float restore_float[26];
	int restore_hud_int[ARRAY_SIZE];
	float restore_hud_float[ARRAY_SIZE];
	char path_buffer[512];

	HANDLE out;
	FILE *outf;
};

static const char global_exe_name[][32] = {
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

static const char global_window_name[][32] = {
	"ArmA Resistance",
	"Cold War Assault",
	"Operation Flashpoint",
	"ArmA Resistance Console",
	"Cold War Assault Console",
	"Operation Flashpoint Console"
};

static const char *global_exe_window[] = {
	global_window_name[0],
	global_window_name[1],
	global_window_name[2],
	global_window_name[2],
	global_window_name[2],
	global_window_name[2],
	global_window_name[2],
	global_window_name[3],
	global_window_name[4],
	global_window_name[5]
};

static const int global_exe_version[] = {
	VER_201,
	VER_199,
	VER_196,
	VER_196,
	VER_196,
	VER_196,
	VER_196,
	VER_201_SERVER,
	VER_199_SERVER,
	VER_196_SERVER
};

static const int global_exe_num    = sizeof(global_exe_version) / sizeof(global_exe_version[0]);
static const int global_window_num = sizeof(global_window_name) / sizeof(global_window_name[0]);

static const int StringDynamic_init_capacity = 1024;

struct StringDynamic {
	char *text;
	size_t length;
	size_t capacity;
	char stack[StringDynamic_init_capacity];
};

struct String {
	char *text;
	size_t length;
};

struct WatchProgramInfo {
	int db_id;
	DWORD pid;
	DWORD exit_code;
	DWORD launch_error;
};

struct BinarySearchResult {
	size_t index;
	bool found;
};

struct FileSize {
	double bytes;
	double kilobytes;
	double megabytes;
};

struct StringPos {
	size_t start;
	size_t end;
};

const unsigned int SQM_CLASSPATH_CAPACITY = 8;

struct SQM_ParseState {
	// For parsing
	size_t i;               // position in the buffer
	size_t word_start;      // starting position of the current word
	int comment;            // if we're currently in a comment
	int expect;             // what kind of string we're expecting next
	int class_level;        // inside how many classes
	int array_level;        // inside how many brackets
	int parenthesis_level;  // inside how many parentheses
	bool word_started;      // indicates if word_start has been used
	bool first_char;        // have we passed first character in the line
	bool is_array;          // is current property an array
	bool in_quote;          // are we passing through a string value
	bool macro;             // is current word a  preprocessor directive
	bool is_inherit;        // is word an inhheritance class name
	bool purge_comment;     // should the current comment be removed from the text
	bool value_quoted;      // does the property value begin with a quotation mark
	char separator;         // character to expect that will end the current word
	char empty_char[1];     // default value for string pointers

	// Output
	String property;            // Last encountered property
	size_t property_start;
	size_t property_end;
	String value;               // Last encountered value
	size_t value_start;
	size_t value_end;
	String class_name;          // Last encountered class
	size_t class_start;
	size_t class_end;
	size_t class_name_end;
	size_t class_length;
	size_t class_name_start;    // including the word class itself
	size_t class_name_full_end; // including the inherit
	String inherit;             // Last encountered class inherit name
	size_t scope_end;           // End of current class
};

enum SQM_EXPECT {
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

enum SQM_COMMENT {
	SQM_NONE,
	SQM_LINE,
	SQM_BLOCK
};

enum SQM_OUTPUT {
	SQM_OUTPUT_END_OF_SCOPE,
	SQM_OUTPUT_PROPERTY,
	SQM_OUTPUT_CLASS
};

enum SQM_ACTION {
	SQM_ACTION_GET_NEXT_ITEM,
	SQM_ACTION_FIND_PROPERTY,
	SQM_ACTION_FIND_CLASS,
	SQM_ACTION_FIND_CLASS_END,
	SQM_ACTION_FIND_CLASS_END_CONVERT
};

#define FNV_PRIME 16777619u
#define FNV_BASIS 2166136261u

enum OPTIONS_VERIFYPATH {
	OPTION_LIMIT_WRITE_LOCATIONS = 0x1,
	OPTION_ALLOW_MOST_LOCATIONS  = 0x2,
	OPTION_SUPPRESS_ERROR        = 0x4,
	OPTION_SUPPRESS_CONVERSION   = 0x8,
	OPTION_ASSUME_TRAILING_SLASH = 0x10,

	PATH_ILLEGAL      = 0,
	PATH_LEGAL        = 0x1,
	PATH_DOWNLOAD_DIR = 0x2
};

enum OPTIONS_STRSTR2 {
	OPTION_NONE          = 0,
	OPTION_CASESENSITIVE = 0x1,
	OPTION_MATCHWORD     = 0x2,
	OPTION_REVERSE       = 0x4
};

enum OPTIONS_GETCHARTYPE {
	CHAR_TYPE_NEW_LINE,
	CHAR_TYPE_SPACE,
	CHAR_TYPE_LETTER,
	CHAR_TYPE_MISC
};

enum OPTIONS_ESCSEQUENCES {
	OPTION_TAB,
	OPTION_LF,
	OPTION_CRLF
};

enum OPTIONS_FNVHASH {
	OPTION_LOWERCASE = 1
};

enum OPTIONS_SHIFT_BUFFER_CHUNK {
	OPTION_LEFT,
	OPTION_RIGHT
};

enum OPTIONS_QWRITE_ERR {
	OPTION_ERROR_ARRAY_STARTED  = 0x1,
	OPTION_ERROR_ARRAY_LOCAL    = 0x2,
	OPTION_ERROR_ARRAY_CLOSE    = 0x4,
	OPTION_ERROR_ARRAY_SUPPRESS = 0x8,
	OPTION_ERROR_ARRAY_NESTED   = 0x10
};

enum OPTIONS_TOKENIZE {
	OPTION_SKIP_SQUARE_BRACKETS = 0x1,
	OPTION_TRIM_SQUARE_BRACKETS = 0x2
};

enum FWATCH_ERRORS {
	FWERROR_NONE,
	FWERROR_UNKNOWN_COMMAND,
	FWERROR_NO_PROCESS,
	FWERROR_GAME_WINDOW_NOT_IN_FRONT,
	FWERROR_COMMAND_ILLEGAL_ON_SERVER,
	FWERROR_WINAPI,
	FWERROR_SHFILEOP,
	FWERROR_ERRNO,
	FWERROR_HRESULT,

	FWERROR_MALLOC = 10,
	FWERROR_REALLOC,
	FWERROR_STR_REPLACE,

	FWERROR_CLIP_OPEN = 20,
	FWERROR_CLIP_FORMAT,
	FWERROR_CLIP_CLEAR,
	FWERROR_CLIP_COPY,
	FWERROR_CLIP_EMPTY,
	FWERROR_CLIP_EFFECT,
	FWERROR_CLIP_LOCK,

	FWERROR_PARAM_FEW = 100,
	FWERROR_PARAM_LTZERO,
	FWERROR_PARAM_ZERO,
	FWERROR_PARAM_ONE,
	FWERROR_PARAM_RANGE,
	FWERROR_PARAM_PATH_LIMITED,
	FWERROR_PARAM_ACTION,
	FWERROR_PARAM_EMPTY,
	FWERROR_PARAM_PATH_RESTRICTED,

	FWERROR_FILE_EMPTY = 200,
	FWERROR_FILE_NOTDIR,
	FWERROR_FILE_MOVETOTMP,
	FWERROR_FILE_NOVAR,
	FWERROR_FILE_NOLINE,
	FWERROR_FILE_APPEND,
	FWERROR_FILE_LINEREPLACE,
	FWERROR_FILE_EXISTS,
	FWERROR_FILE_DIREXISTS,
	FWERROR_FILE_READ,
	FWERROR_FILE_WRITE,

	FWERROR_CLASS_PARENT = 250,
	FWERROR_CLASS_EXISTS,
	FWERROR_CLASS_NOCLASS,
	FWERROR_CLASS_NOVAR,
	FWERROR_CLASS_NOITEM,
	FWERROR_CLASS_NOTARRAY,
	FWERROR_CLASS_SYNTAX,

	FWERROR_DB_SIGNATURE = 280,
	FWERROR_DB_VERSION,
	FWERROR_DB_SMALL,
	FWERROR_DB_COLLISION,
	FWERROR_DB_HASHORDER,
	FWERROR_DB_PTRORDER,
	FWERROR_DB_PTRSMALL,
	FWERROR_DB_PTRBIG,
	FWERROR_DB_PTRFIRST,
	FWERROR_DB_CONSISTENCY,
	FWERROR_DB_KEYEXISTS
};










TESTDLL_API LRESULT CALLBACK HookProc(int nCode, WPARAM wParam, LPARAM lParam);

TESTDLL_API void InstallHook();
TESTDLL_API void RemoveHook();

// for scripth.cpp
typedef struct {
	HANDLE handle;
	char   filename[512];
} FULLHANDLE;

// string
void      ParseScript(String &command, FULLHANDLE file);
const     char* getBool(short b);
int       strncmpi(const char *ps1, const char *ps2, int n);
int       GetCharType(char c);
StringPos ConvertStringPos(char *range_start, char *range_end, char *range_length, size_t text_length);
int       VerifyPath(String &pointer, StringDynamic &buffer, int options);
void      PurgeComments(char *text, int string_start, int string_end);
void      shift_buffer_chunk(char *buffer, size_t chunk_start, size_t chunk_end, size_t shift_distance, bool rightwards);
char*     str_replace(const char *strbuf, const char *strold, const char *strnew, int options);	//TODO: remove
char*     strstr2_old(const char *arg1, size_t arg1_len, const char *arg2, size_t arg2_len, int options); //TODO: remove
void      SQM_Init(SQM_ParseState &input);
int       SQM_Parse(String &input, SQM_ParseState &state, int action_type, String &to_find);
int       SQM_Merge(String &merge, SQM_ParseState &merge_state, StringDynamic &source_dynamic, SQM_ParseState &source_state, char *setpos_line);

// winapi
void DebugMessage(const char *first, ...); 
bool checkActiveWindow(void);
void SystemTimeToString(SYSTEMTIME &st, bool systime, char *str);
void FileTimeToString(FILETIME &ft, bool systime, char *str);
bool CopyToClip(String &input, bool append);
bool trashFile(String file, int error_behaviour);
int  DeleteWrapper(char *refcstrRootDirectory);

// files
void             createPathSqf(LPCSTR lpFileName, size_t len, int offset);
void             db_pid_save(WatchProgramInfo input);
WatchProgramInfo db_pid_load(int db_id_wanted);
void             FindCustomFaceTexture(const char *input_path, char *custom_filename, DWORD *bytes_biggest_file);

// math
double             rad2deg(double num);
double             deg2rad(double num);
bool               IsNumberInArray(int number, const int *array, int array_size);
unsigned int       fnv1a_hash (unsigned int hash, char *text, size_t text_length, bool lowercase);
int                binary_search(unsigned item_to_find, unsigned int *array, int low, int high);
BinarySearchResult binary_search_str(char *buffer, size_t array_size, unsigned int value_to_find, size_t low, size_t high);
FileSize           DivideBytes(double bytes);

// misc
void RestoreMemValues(bool isMissionEditor);
void NotifyFwatchAboutErrorLog();
void WriterHeaderInErrorLog(void *ptr_logfile, void *ptr_phandle, bool notify);

// Output for the game
void QWrite(const char *str);
void QWritef(const char *format, ...);
void QWritel(const char *input, size_t input_length);
void QWrites(String &input);
void QWrite_err(int code_primary, int arg_num, ...);
void QWrite_format_key(int c);
void QWrite_joystick(int customJoyID);
void QWriteq(const char *str);
void QWritesq(String input);

// String
bool   String_bool(String &input);
char*  String_find(String &source, String &to_find, int options);
char*  String_trim_quotes(String &input);
char*  String_trim_space(String &input);
String String_tokenize(String &source, const char *delimiter, size_t &i, int options);
void   String_escape_sequences(String input, int mode, int quantity);

// StringDynamic
void StringDynamic_init(StringDynamic &buffer);
void StringDynamic_end(StringDynamic &buffer);
int  StringDynamic_allocate(StringDynamic &buffer, size_t new_maximal_length);
int  StringDynamic_readfile(StringDynamic &buffer, const char *path);
int  StringDynamic_append(StringDynamic &buffer, const char *input);
int  StringDynamic_appendl(StringDynamic &buffer, const char *input, size_t text_length);
int  StringDynamic_appendf(StringDynamic &buffer, const char *format, ...);
int  StringDynamic_appends(StringDynamic &buffer, String &input);
int  StringDynamic_appendq(StringDynamic &buffer, const char *input);

// for fdb.cpp
char* fdbGet(char* file, char* var);
bool  fdbPut(char* file, char* svar, char* val, bool append);
bool  fdbPutQ(char* file, char* svar, char* val);
bool  fdbExists(char* file);
char* fdbVars(char* file);
char* fdbReadvars(char* file);
bool  fdbRemove(char* file, char* var);
bool  fdbDelete(char* file);
void  fdbGet2(char* file, char* var);

// strnatcmp.c
typedef char nat_char;
static inline int nat_isdigit(nat_char a);
static inline int nat_isspace(nat_char a);
static inline nat_char nat_toupper(nat_char a);
static int compare_right(nat_char const *a, nat_char const *b);
static int compare_left(nat_char const *a, nat_char const *b);
static int strnatcmp0(nat_char const *a, nat_char const *b, int fold_case);
int strnatcmp(nat_char const *a, nat_char const *b);
int strnatcasecmp(nat_char const *a, nat_char const *b);
