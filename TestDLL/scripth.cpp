/*
You may use this source code for personal entertainment purposes only. Any commercial-, education- or military use is strictly forbidden without permission from the author. Any programs compiled from this source and the source code itself should be made available free of charge. Any modified versions must have the source code available for free upon request.
*/

//
// by Kegetys <kegetys@dnainternet.net>
//

// Script handling functions

#define WIN32_LEAN_AND_MEAN

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <shellapi.h>
#include "testdll.h"
#include <ctype.h>
#include <Mmsystem.h>	//v1.1	for joystick
#include <math.h>		//v1.11 for asin
#include <tlhelp32.h>	//v1.11 for traversing process modules
#include "errno.h"		//v1.13 for errno constants
#include "shlobj.h"		//v1.14 for copying files to the clipboard
#include "global_vars.h"	//1.16


#define SCRIPT_VERSION	1.16		// Script engine version
#define MAX_PARAMS		32			// Maximum parameters for commands		v1.15 increased to 32
#define WGET_MINWAIT	1000
#define PIPEBUFSIZE		4096

int lastWget;
bool firstErrorMSG		= 1;	//v1.14 vars controlling error output
bool ErrorWithinError	= 0;	//v1.14 vars controlling error output
bool SuppressNextError	= 0;	//v1.15 vars controlling error output

typedef struct {
	int	id;
	char cmd[128];
} COMMANDS;

// Valid commands
static enum {
	C_INFO_VERSION,
	C_INFO_DEBUGOUT,
	C_INFO_DATE,				//v1.1
	C_INFO_RESOLUTION,			//v1.13
	C_INFO_STARTTIME,			//v1.13
	C_INFO_TICK,				//v1.15
	C_INFO_ERRORLOG,			//v1.15
	C_STRING_FIRST,
	C_STRING_LAST,
	C_STRING_INDEX,
	C_STRING_LENGTH,
	C_STRING_RANGE,
	C_STRING_TOUPPER,
	C_STRING_TOLOWER,
	C_STRING_TOARRAY,
	C_STRING_LENGTH2,			//v1.1
	C_STRING_REPLACE,			//v1.1
	C_STRING_COMPARE,			//v1.1
	C_STRING_TYPE,				//v1.1
	C_STRING_VARIABLE,			//v1.11
	C_STRING_EMPTY,				//v1.11
	C_STRING_RANGE2,			//v1.12
	C_STRING_TOKENIZE,			//v1.12
	C_STRING_TRIMWHITESPACE,	//v1.12
	C_STRING_OCCURRENCES,		//v1.13
	C_STRING_TOARRAY2,			//v1.13
	C_STRING_DOMAIN,			//v1.13
	C_STRING_UPPERLOWERCASE,	//v1.14
	C_STRING_REPLACECHARS,		//v1.14
	C_STRING_INSERT,			//v1.15
	C_STRING_WORDPOS,			//v1.15
	C_FILE_EXISTS,
	C_FILE_READ,
	C_FILE_WRITE,
	C_FILE_QWRITE,
	C_FILE_AWRITE,
	C_FILE_VARS,
	C_FILE_READVARS,
	C_FILE_REMOVE,
	C_FILE_DELETE,
	C_FILE_WGET,
	C_FILE_DXDLL,				//v1.1
	C_FILE_READ2,				//v1.1
	C_FILE_RENAMEMISSIONS,		//v1.11
	C_FILE_MODLIST,				//v1.11
	C_INPUT_GETKEYS,
	C_INPUT_GETKEY,
	C_INPUT_AGETKEYS,
	C_INPUT_AGETKEY,
	C_INPUT_GETMOUSE,
	C_INPUT_SETMOUSE,
	C_INPUT_LOCK,				//v1.1
	C_INPUT_GETJOYSTICK,		//v1.1
	C_INPUT_MULTI,				//v1.13
	C_MEM_GETCURSOR,			//v1.1
	C_MEM_SETCURSOR,			//v1.1
	C_MEM_GETWORLD,				//v1.1
	C_MEM_GETMAP,				//v1.1
	C_MEM_SETMAP,				//v1.1
	C_MEM_GETOPTICS,			//v1.1
	C_MEM_SETOPTICS,			//v1.1
	C_MEM_GETGRAPHICS,			//v1.1
	C_MEM_SETBRIGHTNESS,		//v1.1
	C_MEM_GETJOYSTICK,			//v1.1
	C_MEM_GETSCROLL,			//v1.1
	C_MEM_GETPLAYERANIM,		//v1.1
	C_MEM_SETPLAYERANIM,		//v1.1
	C_MEM_SETCLOUDLETS,			//v1.11
	C_MEM_SETVQUALITY,			//v1.11
	C_MEM_GETCAM,				//v1.11
	C_MEM_GETCINEMABORDER,		//v1.11
	C_MEM_GETNV,				//v1.11
	C_MEM_GETRESPAWNTYPE,		//v1.11
	C_MEM_SETRESPAWNTYPE,	 	//v1.11
	C_MEM_GETRESSIDE,			//v1.11
	C_MEM_GETDAYLIGHT,			//v1.11
	C_MEM_GETDATE,				//v1.11
	C_MEM_GETPLAYERVIEW,		//v1.11
	C_MEM_SETPLAYERVIEW,		//v1.11
	C_MEM_ERROR,				//v1.12
	C_MEM_GETPLAYERAIM,			//v1.12
	C_MEM_SETPLAYERAIM,			//v1.12
	C_MEM_MODLIST,				//v1.13
	C_MEM_GETDIFFICULTY,		//v1.13
	C_MEM_SETDIFFICULTY,		//v1.13
	C_MEM_ISDIALOG,				//v1.13
	C_MEM_ADMIN,				//v1.13
	C_MEM_GETRADIOBOX,			//v1.13
	C_MEM_SETRADIOBOX,			//v1.13
	C_MEM_SETGRAPHICS,			//v1.13
	C_MEM_SETPLAYERDIR,			//v1.13
	C_MEM_GETSPEEDKEY,			//v1.13
	C_MEM_SETSPEEDKEY,			//v1.13
	C_MEM_GETPLAYERHATCH,		//v1.13
	C_MEM_SETPLAYERHATCH,		//v1.13
	C_MEM_GETPLAYERLADDER,		//v1.13
	C_MEM_SETPLAYERLADDER,		//v1.13
	C_MEM_MULTI,				//v1.13
	C_MEM_MASTERSERVER,			//v1.13
	C_MEM_MISSIONINFO,			//v1.14
	C_MEM_BULLETS,				//v1.14
	C_MEM_GETWEATHER,			//v1.15
	C_MEM_SETWEATHER,			//v1.15
	C_MEM_SETCAM,				//v1.15
	C_MEM_HUD,					//v1.16
	C_RESTART_SERVER,			//v1.11
	C_RESTART_CLIENT,			//v1.11
	C_CLIPBOARD_COPY,			//v1.12
	C_CLIPBOARD_GET,			//v1.12
	C_CLIPBOARD_SIZE,			//v1.12
	C_CLIPBOARD_GETLINE,		//v1.13
	C_CLIPBOARD_TOFILE,			//v1.13
	C_CLIPBOARD_FROMFILE,		//v1.13
	C_CLIPBOARD_COPYFILE,		//v1.14
	C_CLIPBOARD_CUTFILE,		//v1.14
	C_CLIPBOARD_PASTEFILE,		//v1.14
	C_CLASS_LIST,				//v1.13
	C_CLASS_TOKENS,				//v1.13
	C_CLASS_MODIFY,				//v1.13
	C_CLASS_MODTOK,				//v1.13
	C_CLASS_READ,				//v1.16
	C_CLASS_READ2,				//v1.16
	C_CLASS_READSQM,			//v1.16
	C_EXE_SPIG,					//v1.13
	C_EXE_ADDONTEST,			//v1.13
	C_EXE_WGET,					//v1.13
	C_EXE_UNPBO,				//v1.14
	C_EXE_PREPROCESS,			//v1.14
	C_EXE_ADDONINSTALL, 		//v1.16
	C_EXE_WEBSITE,				//v1.16
	C_EXE_MAKEPBO,				//v1.16
	C_IGSE_WRITE,				//v1.1
	C_IGSE_LIST,				//v1.1
	C_IGSE_LOAD,				//v1.1
	C_IGSE_NEWFILE,				//v1.11
	C_IGSE_RENAME,				//v1.11
	C_IGSE_FIND,				//v1.12
	C_IGSE_WRITE2,				//v1.12
	C_IGSE_COPY,				//v1.13
	C_IGSE_WRITE3,				//v1.14
	C_IGSE_DB					//v1.16
};

COMMANDS cmdsList[] = {
	C_INPUT_GETMOUSE, "input getmouse",
	C_INPUT_SETMOUSE, "input setmouse",
	C_INPUT_GETKEYS, "input getkeys",
	C_INPUT_GETKEY, "input getkey",
	C_INPUT_AGETKEYS, "input agetkeys",
	C_INPUT_AGETKEY, "input agetkey",
	C_INPUT_LOCK, "input lock",					//v1.1
	C_INPUT_GETJOYSTICK, "input getjoystick",	//v1.1
	C_INPUT_MULTI, "input multi",				//v1.13
	C_FILE_QWRITE, "file qwrite",
	C_FILE_EXISTS, "file exists",
	C_FILE_READ, "file read",
	C_FILE_AWRITE, "file awrite",
	C_FILE_WRITE, "file write",
	C_FILE_VARS, "file vars",
	C_FILE_READVARS, "file readvars",
	C_FILE_REMOVE, "file remove",
	C_FILE_DELETE, "file delete",
	C_FILE_WGET, "file wget",
	C_FILE_DXDLL, "file dxdll",					//v1.1
	C_FILE_READ2, "file read2",					//v1.1
	C_FILE_RENAMEMISSIONS,"file renamemissions",//v1.11
	C_FILE_MODLIST, "file modlist",				//v1.11
	C_STRING_TOUPPER, "string toupper",
	C_STRING_TOLOWER, "string tolower",
	C_STRING_TOARRAY, "string toarray",
	C_STRING_LENGTH, "string length",
	C_STRING_INDEX, "string index",
	C_STRING_RANGE, "string range",
	C_STRING_FIRST, "string first",
	C_STRING_LAST, "string last",
	C_STRING_LENGTH2, "string size",			//v1.1
	C_STRING_REPLACE, "string replace",			//v1.1
	C_STRING_COMPARE, "string compare",			//v1.1
	C_STRING_TYPE,	  "string isnumber",		//v1.1
	C_STRING_VARIABLE,"string isvariable",		//v1.11
	C_STRING_EMPTY,	  "string isempty",			//v1.11
	C_STRING_RANGE2,  "string cut",				//v1.12
	C_STRING_TOKENIZE, "string tokenize",		//v1.12
	C_STRING_TRIMWHITESPACE, "string trim",		//v1.12
	C_STRING_OCCURRENCES, "string find",		//v1.13
	C_STRING_TOARRAY2,	"string split",			//v1.13
	C_STRING_DOMAIN, "string domain",			//v1.13
	C_STRING_UPPERLOWERCASE, "string case",		//v1.13
	C_STRING_REPLACECHARS, "string replacechar",//v1.14
	C_STRING_INSERT, "string join",				//v1.15
	C_STRING_WORDPOS, "string wordpos",			//v1.15
	C_INFO_VERSION, "info version",
	C_INFO_DEBUGOUT, "info debugout",
	C_INFO_DATE, "info date",					//v1.1
	C_INFO_RESOLUTION, "info resolution",		//v1.13
	C_INFO_STARTTIME, "info starttime",			//v1.13
	C_INFO_TICK, "info tick",					//v1.15
	C_INFO_ERRORLOG, "info errorlog",			//v1.15
	C_MEM_GETCURSOR, "mem getcursor",			//v1.1
	C_MEM_SETCURSOR, "mem setcursor",			//v1.1
	C_MEM_GETWORLD,  "mem getworld",			//v1.1
	C_MEM_GETMAP,  "mem getmap",				//v1.1
	C_MEM_SETMAP,  "mem setmap",				//v1.1
	C_MEM_GETGRAPHICS, "mem getgraphics",		//v1.1
	C_MEM_GETJOYSTICK, "mem getjoystick",		//v1.1
	C_MEM_GETSCROLL, "mem getscroll",			//v1.1
	C_MEM_GETPLAYERANIM, "mem getplayeranim",	//v1.1
	C_MEM_SETPLAYERANIM, "mem setplayeranim",	//v1.1
	C_MEM_GETCAM,		"mem getcam",			//v1.11
	C_MEM_GETCINEMABORDER,"mem getcinemaborder",//v1.11
	C_MEM_GETNV,		"mem getnv",			//v1.11
	C_MEM_GETRESPAWNTYPE, "mem getrespawntype", //v1.11
	C_MEM_SETRESPAWNTYPE, "mem setrespawntype", //v1.11
	C_MEM_GETRESSIDE, "mem getresside",			//v1.11
	C_MEM_GETDAYLIGHT, "mem getdaylight",		//v1.11
	C_MEM_GETDATE, "mem getdate",				//v1.11
	C_MEM_GETPLAYERVIEW, "mem getplayerview",	//v1.11
	C_MEM_SETPLAYERVIEW, "mem setplayerview",	//v1.11
	C_MEM_ERROR, "mem error",					//v1.12
	C_MEM_GETPLAYERAIM, "mem getplayeraim",		//v1.12
	C_MEM_SETPLAYERAIM, "mem setplayeraim",		//v1.12
	C_MEM_MODLIST, "mem modlist",				//v1.13
	C_MEM_GETDIFFICULTY, "mem getdifficulty",	//v1.13
	C_MEM_SETDIFFICULTY, "mem setdifficulty",	//v1.13
	C_MEM_ISDIALOG, "mem isdialog",				//v1.13
	C_MEM_GETRADIOBOX, "mem getradiobox",		//v1.13
	C_MEM_SETRADIOBOX, "mem setradiobox",		//v1.13
	C_MEM_SETGRAPHICS, "mem setgraphics",		//v1.13
	C_MEM_GETSPEEDKEY, "mem getspeedkey",		//v1.13
	C_MEM_SETSPEEDKEY, "mem setspeedkey",		//v1.13
	C_MEM_GETPLAYERHATCH, "mem getplayerhatch",	//v1.13
	C_MEM_SETPLAYERHATCH, "mem setplayerhatch",	//v1.13
	C_MEM_GETPLAYERLADDER,"mem getplayerladder",//v1.13
	C_MEM_SETPLAYERLADDER,"mem setplayerladder",//v1.13
	C_MEM_MULTI, "mem multi",					//v1.13
	C_MEM_MASTERSERVER, "mem masterserver",		//v1.13
	C_MEM_MISSIONINFO, "mem missioninfo",		//v1.14
	C_MEM_BULLETS, "mem bullets",				//v1.14
	C_MEM_GETWEATHER, "mem getweather",			//v1.15
	C_MEM_SETWEATHER, "mem setweather",			//v1.15
	C_MEM_SETCAM, "mem setcam",					//v1.15
	C_MEM_HUD, "mem hud",						//v1.16
	C_RESTART_SERVER, "restart server",			//v1.11
	C_RESTART_CLIENT, "restart client",			//v1.11
	C_CLIPBOARD_COPY, "clip copy",				//v1.12
	C_CLIPBOARD_GET, "clip get",				//v1.12
	C_CLIPBOARD_SIZE, "clip size",				//v1.12
	C_CLIPBOARD_GETLINE, "clip getline",		//v1.13
	C_CLIPBOARD_TOFILE, "clip tofile",			//v1.13
	C_CLIPBOARD_FROMFILE, "clip fromfile",		//v1.13
	C_CLIPBOARD_COPYFILE, "clip copyfile",		//v1.14
	C_CLIPBOARD_PASTEFILE, "clip pastefile",	//v1.14
	C_CLIPBOARD_CUTFILE, "clip cutfile",		//v1.14
	C_CLASS_LIST, "class list",					//v1.13
	C_CLASS_TOKENS, "class token",				//v1.13
	C_CLASS_MODIFY, "class modify",				//v1.13
	C_CLASS_MODTOK, "class modtok",				//v1.13
	C_CLASS_READ, "class read",					//v1.16
	C_CLASS_READ2, "class read2",				//v1.16
	C_CLASS_READSQM, "class readsqm",			//v1.16
	C_EXE_SPIG, "spig exe",						//v1.13
	C_EXE_ADDONTEST, "exe addontest",			//v1.13
	C_EXE_WGET, "exe wget",						//v1.13
	C_EXE_UNPBO, "exe unpbo",					//v1.14
	C_EXE_PREPROCESS, "exe preprocess",			//v1.14
	C_EXE_ADDONINSTALL, "exe addoninstall",		//v1.16
	C_EXE_WEBSITE, "exe website",	 			//v1.16
	C_EXE_MAKEPBO, "exe makepbo",				//v1.16
	C_IGSE_WRITE, "igse write",					//v1.1
	C_IGSE_LIST, "igse list",					//v1.1
	C_IGSE_LOAD, "igse load",					//v1.1
	C_IGSE_NEWFILE, "igse new",					//v1.11
	C_IGSE_RENAME, "igse rename",				//v1.11
	C_IGSE_FIND, "igse find",					//v1.12
	C_IGSE_COPY, "igse copy",					//v1.13
	C_IGSE_DB, "igse db",						//v1.16
	-1, NULL,
};


// 1.15 List of commands that use 0x29 to separate arguments
int newArgSystem[] = {
	C_STRING_REPLACE,
	C_STRING_COMPARE,
	C_STRING_TYPE,
	C_STRING_VARIABLE,
	C_STRING_RANGE2,
	C_STRING_TOKENIZE,
	C_STRING_TRIMWHITESPACE,
	C_STRING_OCCURRENCES,
	C_STRING_TOARRAY2,
	C_STRING_UPPERLOWERCASE,
	C_STRING_REPLACECHARS,
	C_STRING_INSERT,
	C_STRING_WORDPOS,
	C_CLASS_LIST,
	C_CLASS_TOKENS,
	C_CLASS_MODIFY,
	C_CLASS_MODTOK,
	C_CLASS_READ,
	C_CLASS_READ2,
	C_CLASS_READSQM,
	C_IGSE_WRITE,
	C_IGSE_FIND,
	C_IGSE_DB,
	C_CLIPBOARD_COPY,
	C_CLIPBOARD_GETLINE
};

// 1.15 List of commands that aggregate other commands
int multiCommands[] = {
	C_INPUT_MULTI,
	C_MEM_MULTI
};

int MemCommands[] = {
	C_MEM_GETCURSOR,
	C_MEM_SETCURSOR,
	C_MEM_GETWORLD,
	C_MEM_GETMAP,
	C_MEM_SETMAP,
	C_MEM_GETOPTICS,
	C_MEM_SETOPTICS,
	C_MEM_GETGRAPHICS,
	C_MEM_SETBRIGHTNESS,
	C_MEM_GETJOYSTICK,
	C_MEM_GETSCROLL,
	C_MEM_GETPLAYERANIM,
	C_MEM_SETPLAYERANIM,
	C_MEM_SETCLOUDLETS,
	C_MEM_SETVQUALITY,
	C_MEM_GETCAM,
	C_MEM_GETCINEMABORDER,
	C_MEM_GETNV,
	C_MEM_GETRESPAWNTYPE,
	C_MEM_SETRESPAWNTYPE,
	C_MEM_GETRESSIDE,
	C_MEM_GETDAYLIGHT,
	C_MEM_GETDATE,
	C_MEM_GETPLAYERVIEW,
	C_MEM_SETPLAYERVIEW,
	C_MEM_ERROR,
	C_MEM_GETPLAYERAIM,
	C_MEM_SETPLAYERAIM,
	C_MEM_MODLIST,
	C_MEM_GETDIFFICULTY,
	C_MEM_SETDIFFICULTY,
	C_MEM_ISDIALOG,
	C_MEM_ADMIN,
	C_MEM_GETRADIOBOX,
	C_MEM_SETRADIOBOX,
	C_MEM_SETGRAPHICS,
	C_MEM_SETPLAYERDIR,
	C_MEM_GETSPEEDKEY,
	C_MEM_SETSPEEDKEY,
	C_MEM_GETPLAYERHATCH,
	C_MEM_SETPLAYERHATCH,
	C_MEM_GETPLAYERLADDER,
	C_MEM_SETPLAYERLADDER,
	C_MEM_MULTI,
	C_MEM_MASTERSERVER,
	C_MEM_MISSIONINFO,
	C_MEM_BULLETS,
	C_MEM_GETWEATHER,
	C_MEM_SETWEATHER,
	C_MEM_SETCAM,
	C_MEM_HUD,
	C_INPUT_MULTI,
	C_INFO_STARTTIME,
	C_EXE_WEBSITE,
	C_FILE_MODLIST
};

int MemCommandsDedicated[] = {
	C_MEM_GETWORLD,
	C_MEM_GETRESPAWNTYPE,
	C_MEM_GETRESSIDE,
	C_MEM_GETDAYLIGHT,
	C_MEM_GETDIFFICULTY,
	C_MEM_SETDIFFICULTY,
	C_MEM_BULLETS,
	C_MEM_GETWEATHER,
	C_MEM_SETWEATHER,
	C_MEM_MISSIONINFO,
	C_FILE_MODLIST
};

int MemCommands201[] = {
	C_MEM_GETCURSOR,
	C_MEM_SETCURSOR,
	C_MEM_ERROR,
	C_MEM_MODLIST,
	C_MEM_MASTERSERVER,
	C_MEM_HUD,
	C_INPUT_MULTI,
	C_INFO_STARTTIME,
	C_EXE_WEBSITE,
	C_FILE_MODLIST
};

char* strtok2(char* str);
int matchCmd(char *cmd);
void QWrite(char* str, HANDLE file);
char* stripq(char *str);

//1.16 Function modes (arguments to pass)
enum IGSE_LIST_OPTIONS 
{
	FILENAMES_AND_ATTRIBUTES,
	FILENAMES_ONLY,
	MODFOLDERS
};

enum VERIFY_PATH
{
	ILLEGAL_PATH,
	LEGAL_PATH,
	DOWNLOAD_DIR,
	RESTRICT_TO_MISSION_DIR = 1,
	ALLOW_GAME_ROOT_DIR = 2,
	SUPPRESS_ERROR = 4,
	SUPPRESS_CONVERSION = 8
};

enum IGSE_WRITE_MODES 
{
	IGSE_WRITE_REPLACE,
	IGSE_WRITE_APPEND,
	IGSE_WRITE_NEW,
	IGSE_WRITE_INSERT,
	IGSE_WRITE_COPY,
	IGSE_WRITE_DELETE,
	IGSE_WRITE_CLEAR,
	IGSE_WRITE_MOVEUP,
	IGSE_WRITE_MOVEDOWN
};

enum IGSE_NEWFILE_MODES 
{
	IGSE_NEWFILE_CREATE,
	IGSE_NEWFILE_DELETE,
	IGSE_NEWFILE_RECREATE,
	IGSE_NEWFILE_CHECK
};

enum IGSE_DB_MODES 
{
	IGSEDB_FILE,
	IGSEDB_KEY,
	IGSEDB_WRITE,
	IGSEDB_APPEND,
	IGSEDB_RENAME,
	IGSEDB_REMOVE,
	IGSEDB_READ,
	IGSEDB_LIST
};

// List of accessible locations
const int allowed_paths_num = 8;
static char allowed_paths[allowed_paths_num][32] = {
	"in-game-script-editor",
	"flashpointcutscenemaker",
	"missioneditor3d",
	"@addontest",
	"fwatch\\idb",
	"fwatch/idb",
	"fwatch\\tmp",
	"fwatch/tmp"
};
static int allowed_paths_len[allowed_paths_num] = {
	21,
	23,
	15,
	10,
	10,
	10,
	10,
	10
};








// Parse a script command
void ParseScript(char* com, FULLHANDLE file) {

	HANDLE out = file.handle;

	//v1.15  FWerror can see command line
	com_ptr = com;


	//v1.15  Figure out command name
	const int CmdLen		= 32;
	char CmdName[CmdLen]	= "";
	GetFirstTwoWords(com, CmdName, CmdLen);


	//v1.13  Store command id in a separate var so it can be passed to the error function
	int CommandID = matchCmd(CmdName);		


	//v1.15  Returning error message before creating buffers
	if (CommandID == -1)
	{
		QWrite("ERROR: Unknown command ", out);
		QWrite(com,out);						//v1.13 additional info
		return;
	};


	//v1.15  If there are special properties about this command that we need to know about
	bool newArgumentSystem	= IsNumberInArray(CommandID, newArgSystem, sizeof(newArgSystem) / sizeof(newArgSystem[0]));
	bool isMULTI			= IsNumberInArray(CommandID, multiCommands, sizeof(multiCommands) / sizeof(multiCommands[0]));
	bool isMemCommand		= IsNumberInArray(CommandID, MemCommands, sizeof(MemCommands) / sizeof(MemCommands[0]));
	bool isMemDedicated		= IsNumberInArray(CommandID, MemCommandsDedicated, sizeof(MemCommandsDedicated) / sizeof(MemCommandsDedicated[0]));


	//v1.1  If a memory command then open process
	SIZE_T stBytes	= 0;
	HANDLE phandle	= NULL;
	DWORD pid		= GetCurrentProcessId();
	bool openedProc = false;

	if (isMemCommand)
	{
		//v1.11  Quit if command not allowed for the dedicated server
		if (DedicatedServer  &&  !isMemDedicated)
			return;

		//v1.16 Quit if unsupported mem command
		if (Game_Version==VER_201 && !IsNumberInArray(CommandID, MemCommands201, sizeof(MemCommands201) / sizeof(MemCommands201[0])))
			return;

		phandle = OpenProcess(PROCESS_ALL_ACCESS, 0, pid);
		if (phandle == 0)
		{
			if (CommandID == C_INFO_STARTTIME)
				FWerror(0,GetLastError(),CommandID,"","",0,0,out);
			else
				QWrite("ERROR: Couldn't get a handle",out);

			return;
		};

		if (Game_Version==VER_201  &&  Game_Exe_Address==0) {
			HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
			MODULEENTRY32 xModule;
			memset (&xModule, 0, sizeof(xModule));
			xModule.dwSize = sizeof(xModule);
 
			if (hSnap != INVALID_HANDLE_VALUE) {
				if (Module32First(hSnap, &xModule)) {
					Game_Exe_Address = (int)xModule.modBaseAddr;
				};
				
				CloseHandle(hSnap);
			}
		}

		openedProc = true;
	};



	

	// Tokenize the string
	int l = strlen(com);
	char *str = new char[l+1];
	if (!str) return;
	memcpy(str, com, l+1);

	str = !newArgumentSystem ? strtok2(str) : strtok3(str,CommandID);
	char *pch = strtok(str, !newArgumentSystem ? "\n" : "\a");
	char *par[MAX_PARAMS];
	int numP = 0;

	while (pch != NULL && numP < MAX_PARAMS) {
		par[numP] = pch;
		pch = strtok(NULL, !newArgumentSystem ? "\n" : "\a");
		numP++;
	}


	firstErrorMSG = 1;

	switch(CommandID) 
	{
		//v1.13 Moved all commands into different files for convenience
		#include "info_commands.cpp"
		#include "string_commands.cpp"
		#include "file_commands.cpp"
		#include "input_commands.cpp"
		#include "mem_commands.cpp"
		#include "exe_commands.cpp"
		#include "clip_commands.cpp"
		#include "class_commands.cpp"
		#include "igse_commands.cpp"

		default:
			QWrite("ERROR: Unknown command ", out); QWrite(com,out);	//v1.13 additional info
			break;
	};

	if (openedProc) 
		CloseHandle(phandle);	//v1.1 if a memory operation then close handle

	delete[] str;
	return;
}



	//v1.13 Moved all functions to a separate file for convenience
	#include "_Functions.cpp"



