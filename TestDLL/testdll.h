// TestDLL header file.
#ifdef TESTDLL_EXPORTS
#define TESTDLL_API __declspec(dllexport)
#else
#define TESTDLL_API __declspec(dllimport)
#endif

#include <process.h>
#include <stdio.h>

static enum RESTORE_MEM_COMMANDS
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
static enum RESTORE_MEM_INTEGERS
{
	INT_MAX_OBJECTS,	//0
	INT_MAX_LIGHTS		//1
};
static enum RESTORE_MEM_FLOATS
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
static enum RESTORE_MEM_BYTES
{
	BYTE_OBJECT_SHADOWS,	//0
	BYTE_VEHICLE_SHADOWS,	//1
	BYTE_CLOUDLETS,			//2
	BYTE_CADET,				//3
	BYTE_VETERAN = 15		//15
};
static enum GAME_VERSION 
{
	VER_UNKNOWN,
	VER_196,
	VER_199,
	VER_201
};

// Variables for UI position
//ofp: [[0x79F8D0] + 0x8] + 
//cwa: [[0x78E9C8] + 0x8] + 
static enum UI_ELEMENTS
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
	int exe_address;
	bool is_server;

	bool restore_memory[105];
	bool restore_byte[27];
	bool nomap;
	bool CWA;
	bool DedicatedServer;
	bool ErrorLog_Enabled;
	bool ErrorLog_Started;

	char *com_ptr;
	char mission_path[256];
	char mission_path_previous[256];
	int mission_path_savetime;
	DWORD pid;
	int external_program_id;

	int extCamOffset;
	int restore_int[2];
	float restore_float[26];
	int restore_hud_int[ARRAY_SIZE];
	float restore_hud_float[ARRAY_SIZE];
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
	VER_201,
	VER_199,
	VER_196
};

static const int global_exe_num    = sizeof(global_exe_version) / sizeof(global_exe_version[0]);
static const int global_window_num = sizeof(global_window_name) / sizeof(global_window_name[0]);

static const int String_init_len = 512;

struct String {
	char *pointer;
	char stack[String_init_len];
	int current_length;
	int maximal_length;
	bool heap;
};

struct WatchProgramInfo {
	int db_id;
	DWORD pid;
	DWORD exit_code;
	DWORD launch_error;
};








TESTDLL_API LRESULT CALLBACK HookProc(int nCode, WPARAM wParam, LPARAM lParam);

TESTDLL_API void InstallHook();
TESTDLL_API void RemoveHook();

// for dllmain.cpp
void DebugMessage(char *first, ...); 
//extern bool nomap;

#define WH_KEYBOARD_LL 13
typedef struct {
    DWORD vkCode;
    DWORD scanCode;
    DWORD flags;
    DWORD time;
    DWORD dwExtraInfo;
} KBDLLHOOKSTRUCT, *PKBDLLHOOKSTRUCT;

// for scripth.cpp
typedef struct {
	HANDLE handle;
	char	filename[512];
} FULLHANDLE;

void ParseScript(char* com, FULLHANDLE file);
char *formatKey(int c);
inline char* getBool(short b);
bool checkActiveWindow(void);





// for fdb.cpp
char* fdbGet(char* file, char* var, int CommandID, HANDLE out);							//v1.13 additional arguments
bool  fdbPut(char* file, char* svar, char* val, bool append, int CommandID, HANDLE out);//v1.13 additional arguments
bool  fdbPutQ(char* file, char* svar, char* val);
bool  fdbExists(char* file);
char* fdbVars(char* file);
char* fdbReadvars(char* file);
bool  fdbRemove(char* file, char* var);
bool  fdbDelete(char* file);
void  fdbGet2(char* file, char* var, int CommandID, HANDLE out);							//v1.1




// New functions
//1.11
DWORD findProcess(const char* exe_name);

//1.12
char* str_replace(const char *strbuf, const char *strold, const char *strnew, int matchWord, int caseSens);

//1.13
char *strstr2(const char *arg1, const char *arg2, int matchWord, int caseSens);
int strncmpi(const char *ps1, const char *ps2, int n);
void getAttributes(WIN32_FIND_DATA &fd, char *data, int systime);
bool trashFile(char* path, int CommandID, HANDLE out, int ErrorBehaviour);
double rad2deg(double num);
double deg2rad(double num);
void FormatTime(SYSTEMTIME &st, int systime, char *data);
void FormatFileTime(FILETIME &ft, int systime, char *data);
bool CopyToClip(char *txt, bool append, int CommandID, HANDLE out);
char* Trim(char *txt);
void FWerror(int code, int secondaryCode, int CommandID, char* text1, char* text2, int num1, int num2, HANDLE out);
char* EscSequences(char *txt, int mode, int quantity);

//1.14
void PrintFileVersion( TCHAR *pszFilePath );
void PrintDoubleQ(char *txt, HANDLE out);
void SplitStringIntoParts(char *txt, int cut, bool addComma, HANDLE out);
void ReadJoystick(char *data, int customJoyID);
void createPathSqf(LPCSTR lpFileName, int len, int offset);

//1.15
bool IsWhiteSpace(char *txt);
bool String2Bool(char *txt);
char* strtok3(char* str, int CommandID);
void GetFirstTwoWords(char* text, char* buffer, int maxSize);

// Find integer in an integer array
bool IsNumberInArray(int number, const int* array, int array_size);
void CorrectStringPos(int *start, int *end, int length, bool endSet, bool lengthSet, int textSize);
int GetCharType(char c);
void RestoreMemValues(bool isMissionEditor);

typedef char nat_char;
static inline int nat_isdigit(nat_char a);
static inline int nat_isspace(nat_char a);
static inline nat_char nat_toupper(nat_char a);
static int compare_right(nat_char const *a, nat_char const *b);
static int compare_left(nat_char const *a, nat_char const *b);
static int strnatcmp0(nat_char const *a, nat_char const *b, int fold_case);
int strnatcmp(nat_char const *a, nat_char const *b);
int strnatcasecmp(nat_char const *a, nat_char const *b);

void String_init(String &str);
int String_allocate(String &str, int new_maximal_length);
int String_append_len(String &str, char *text, int text_length);
int String_append(String &str, char *text);
int String_append_quotes(String &str, char *left, char *text, char *right);
void String_end(String &str);
int String_readfile(String &str, char *path);
int VerifyPath(char **ptr_filename, String &str, int mode, int CommandID, HANDLE out);
unsigned int fnv_hash (unsigned int hash, char* text, int text_length);
void PurgeComments(char *text, int string_start, int string_end);
char* Output_Nested_Array(char *temp, int level, char *output_strings_name, int j, int *subclass_count);
int DeleteWrapper(char *refcstrRootDirectory);
WatchProgramInfo db_pid_load(int db_id_wanted);
void db_pid_save(WatchProgramInfo input);
void NotifyFwatchAboutErrorLog();
void WriterHeaderInErrorLog(void *ptr_logfile, void *ptr_phandle, bool notify);
void shift_text_in_buffer(char *buffer, int buffer_size, int shift_origin, int shift_size);
void printbuf(FILE **fd, char *buffer, int size);
