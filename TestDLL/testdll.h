// TestDLL header file.
#ifdef TESTDLL_EXPORTS
#define TESTDLL_API __declspec(dllexport)
#else
#define TESTDLL_API __declspec(dllimport)
#endif

#include <process.h>





//v1.13 global variables to differentiate between OFP and CWA
extern bool CWA;
extern bool DedicatedServer;
extern char GameWindowName[32];
extern char GameServerName[32];
extern char MissionPath[256];	//1.15
extern bool ErrorLog_Enabled;	//1.15
extern bool ErrorLog_Started;	//1.15
extern char* com_ptr;			//v1.15
extern int  RESTORE_INT[2];		//v1.15
extern float RESTORE_FLT[26];	//v1.15
extern bool RESTORE_BYT[27];	//v1.15
extern bool RESTORE_MEM[105];	//v1.15 
extern int PathSqfTime;			//v1.15
extern char lastMissionPath[256];//v1.15
extern int extCamOffset;		//v1.15
extern int Game_Version;		//v1.16
extern int Game_Exe_Address;	//v1.16

static enum RESTORE_MEM_COMMANDS
{
	RESTORE_BRIGHTNESS,		//0
	RESTORE_OSHADOWS,		//1
	RESTORE_VSHADOWS,		//2
	RESTORE_CLOUDLETS,		//3
	RESTORE_BULLETS,		//4
	RESTORE_CADET = 14,		//14
	RESTORE_VETERAN = 26,	//26
	RESTORE_RADAR = 38,		//38
	RESTORE_MAXOBJ,			//39
	RESTORE_TRACK1,			//40
	RESTORE_TRACK2,			//41
	RESTORE_MAXLIGHTS,		//42
	RESTORE_TIDE,			//43
	RESTORE_WAVE,			//44
	RESTORE_EXTCAMPOS,		//45
	RESTORE_WAVESPEED,		//46
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
	INT_MAXOBJ,			//0
	INT_MAXLIGHTS		//1
};
static enum RESTORE_MEM_FLOATS
{
	FLT_BRIGHTNESS,		//0
	FLT_BULLETS,		//1
	FLT_RADAR = 11,		//11
	FLT_TRACK1,			//12
	FLT_TRACK2,			//13
	FLT_TIDE,			//14
	FLT_WAVE,			//15
	FLT_EXTCAMX,		//16
	FLT_EXTCAMY,		//17
	FLT_EXTCAMZ,		//18
	FLT_WAVESPEED,		//19
	FLT_FOVLEFT,		//20
	FLT_FOVTOP,			//21
	FLT_UITOPLEFTX,		//22
	FLT_UITOPLEFTY,		//23
	FLT_UIBOTTOMRIGHTX,	//24
	FLT_UIBOTTOMRIGHTY	//25
};
static enum RESTORE_MEM_BYTES
{
	BYT_OSHADOWS,		//0
	BYT_VSHADOWS,		//1
	BYT_CLOUDLETS,		//2
	BYT_CADET,			//3
	BYT_VETERAN = 15	//15
};
static enum GAME_VERSION 
{
	UNKNOWN,
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



extern int RESTORE_HUD_INT[ARRAY_SIZE];
extern float RESTORE_HUD_FLT[ARRAY_SIZE];





TESTDLL_API LRESULT CALLBACK HookProc(int nCode, WPARAM wParam, LPARAM lParam);

TESTDLL_API void InstallHook();
TESTDLL_API void RemoveHook();

// for dllmain.cpp
void DebugMessage(char *first, ...); 
extern bool nomap;

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
//1.1
bool getMissionDir(char** FileName, int &FileNameLength, int CommandID, HANDLE out);
void listDirFiles (char* path, HANDLE out, int mode, int systime, int CommandID);

//1.11
int findProcess(char* name);

//1.12
bool isAllowedExternalPath (char* path, bool mode);
bool isLeavingDir (char* directory, bool isDownload, bool restrictive, int CommandID, HANDLE out);
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
bool IsNumberInArray(int number, int *array, int max_loops);
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

//1.16
struct String {
	char stack[512];
	char *pointer;
	int current_length;
	int maximal_length;
	bool heap;
};

void String_init(String &str);
void String_end(String &str);
int String_append(String &str, char *text);
int String_append_quotes(String &str, char *left, char *text, char *right);
int String_allocate(String &str, int new_maximal_length);
char* Tokenize(char *string, char *delimiter, int &i, int string_length, char &save);
