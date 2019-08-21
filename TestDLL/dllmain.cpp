#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlwapi.h>
#include "testdll.h"
#include "..\apihijack.h"
#include <stdio.h>

// Text buffer for sprintf
char Work[256];

HINSTANCE hDLL;
bool nomap;

//v1.13 global vars for OFP and CWA
bool CWA				= 0;						//is Cold War Assault calling
bool DedicatedServer	= 0;						//is dedicated server calling
char GameWindowName[32]	= "Operation Flashpoint";	//game window name
char GameServerName[32]	= "OFPR_Server.exe";		//server exe name

char MissionPath[256]	= "";						//v1.15 path to the current mission
bool ErrorLog_Enabled	= 0;						//v1.15 save error messages to text file
bool ErrorLog_Started	= 0;						//v1.15 is this the first entry in the log
char *com_ptr			= "";						//v1.15 pointer to com param so FWerror function can access it
int PathSqfTime			= 0;						//v1.15 for creating "path.sqf" file
char lastMissionPath[256] = "";						//v1.15 for creating "path.sqf" file
int extCamOffset		= 0;						//v1.15 for restoring extCameraPosition value
int Game_Version		= 0;						//v1.16 for determining memory offsets
int Game_Exe_Address    = 0;						//v1.16 for 2.01 memory offsets

//v1.15 values for restoring changed mem settings
int  RESTORE_INT[2];
float RESTORE_FLT[26];
bool RESTORE_BYT[27];
bool RESTORE_MEM[105];
int RESTORE_HUD_INT[ARRAY_SIZE];
float RESTORE_HUD_FLT[ARRAY_SIZE];







// Function pointer types.
typedef HANDLE (WINAPI *NewCreateFileA_Type)(LPCSTR lpFileName, DWORD  dwDesiredAccess, DWORD  dwShareMode, LPSECURITY_ATTRIBUTES  lpSecurityAttributes, DWORD  dwCreationDistribution, DWORD  dwFlagsAndAttributes, HANDLE  hTemplateFile);

// Function prototypes.
HANDLE WINAPI NewCreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD  dwShareMode, LPSECURITY_ATTRIBUTES  lpSecurityAttributes, DWORD  dwCreationDistribution, DWORD  dwFlagsAndAttributes, HANDLE  hTemplateFile);

HANDLE HandleCommand(char *command); // in chandler.cpp

// Hook structure.
enum
{
    D3DFN_CreateFileA = 0
};

SDLLHook D3DHook = 
{
    "KERNEL32.DLL",
    false, NULL,		// Default hook disabled, NULL function pointer.
    {
        {"CreateFileA", NewCreateFileA},
        {NULL, NULL}
    }
};

// Output debug string
void DebugMessage(char *first, ...)
{
	va_list     argptr;
	char        Message[512];

	va_start (argptr,first);
	vsprintf (Message,first,argptr);
	va_end   (argptr);

	OutputDebugString(Message);  
}

// Hook function.
HANDLE WINAPI NewCreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD  dwShareMode, LPSECURITY_ATTRIBUTES  lpSecurityAttributes, DWORD  dwCreationDistribution, DWORD  dwFlagsAndAttributes, HANDLE  hTemplateFile) {
	//DebugMessage("fwatch: NewCreateFileA %s %d %d %d", lpFileName, dwFlagsAndAttributes, dwShareMode, dwCreationDistribution);
	int len = strlen(lpFileName);

		//v1.14 Get path to mission dir and save that info to a file
		if (!strchr(lpFileName, ':'))
		{
			if (!strncmpi("init.sqs", lpFileName+len-8, 8))
				createPathSqf(lpFileName, len, -8);

			if (!strncmpi(".html", lpFileName+len-5, 5))
			{
				char *pch = strstr(lpFileName,"briefing.");
				if (pch != NULL)
					createPathSqf(lpFileName, len, pch - lpFileName);
			};

			if (!strncmpi(".pbo", lpFileName+len-4, 4))
				if (!strncmpi("missions\\", lpFileName, 9)  ||  !strncmpi("mpmissions\\", lpFileName, 11))
					createPathSqf(lpFileName, len, -4);

			if (!strncmpi("description.ext", lpFileName+len-15, 15))
				if (!strncmpi("campaigns\\", lpFileName, 10))
					createPathSqf(lpFileName, len, -15);
		};


	//v1.15  Recognise syntax
	bool NewSyntax = !strncmp(":", lpFileName, 1);

	if(!strncmp("scripts\\:", lpFileName, 9) || NewSyntax) {
		// This filename is meant for us
		if(dwFlagsAndAttributes == 128) {
			// Script is being opened for execution

			char *command = (char*)lpFileName + (NewSyntax ? 1 : 9);

			//DebugMessage("fwatch: command: %s", command);

			// Call HandleCommand, and return returned file handle to OFP
			return HandleCommand(command);
		} else {
			// Just a check if the file is there, return something for OFP to read
			HANDLE rp, wp;
			CreatePipe(&rp, &wp, NULL, 256);

			DWORD foo;
			char str[] = "shei masa fum de ma";
			WriteFile(wp, str, strlen(str), &foo, NULL);
			CloseHandle(wp);
			return rp;
		}
	} else if(len > 16) {
		if(!strcmp("fwatch_check.sqf", lpFileName+(len-16))) {
			// Any access attempt to a file named fwatch_check.sqf will be redirected to fwatch/data/true.sqf

			DebugMessage("fwatch: presence check");
			strcpy((char*) lpFileName, "fwatch\\data\\true.sqf");

		}
	}



	// Call real CreateFileA
    NewCreateFileA_Type OldFn = 
        (NewCreateFileA_Type)D3DHook.Functions[D3DFN_CreateFileA].OrigFn;

    return OldFn(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDistribution, dwFlagsAndAttributes, hTemplateFile);
}

// CBT Hook-style injection.
BOOL APIENTRY DllMain( HINSTANCE hModule, DWORD fdwReason, LPVOID lpReserved )
{
   	if ( fdwReason == DLL_PROCESS_ATTACH )  // When initializing....
    {
        memset(RESTORE_MEM, 0, 105);
		hDLL = hModule;

        // We don't need thread notifications for what we're doing.  Thus, get
        // rid of them, thereby eliminating some of the overhead of this DLL
        DisableThreadLibraryCalls( hModule );

        // Only hook the APIs if this is the ofp proess.
        GetModuleFileName( GetModuleHandle( NULL ), Work, sizeof(Work) );
        PathStripPath( Work );

		DebugMessage("fwatch: checking process %s", Work);
		

			//v1.13 set global var for ofp or cwa
			if (!stricmp(Work, "coldwarassault.exe") || !stricmp(Work, "coldwarassault_server.exe")) {
				CWA = 1;
				Game_Version = VER_199;
				strcpy(GameWindowName,"Cold War Assault");
			} else if (!stricmp(Work, "armaresistance.exe") || !stricmp(Work, "armaresistance_server.exe")) {
				CWA = 1;
				Game_Version = VER_201;
				strcpy(GameWindowName,"ArmA Resistance");
			} else {
				CWA = 0;
				Game_Version = VER_196;
				strcpy(GameWindowName,"Operation Flashpoint");
			}

			//v1.13 set global var for sever
			if (!stricmp(Work, "coldwarassault_server.exe") || !stricmp(Work, "ofpr_server.exe") || !stricmp(Work, "armaresistance_server.exe"))
				DedicatedServer=1;
			else
				DedicatedServer=0;


		//v1.13 condition for ofp or cwa
		//v1.1 added ofp.exe (PL version) and //v1.11 operationflashpointbeta.exe
        if
		(
			!CWA &&
			(!stricmp(Work, "flashpointresistance.exe") ||
		     !stricmp(Work, "flashpointbeta.exe") || 
		     !stricmp(Work, "operationflashpoint.exe") || 
		     !stricmp(Work, "ofpr_server.exe") || 
		     !stricmp(Work, "ofp.exe") || 
		     !stricmp(Work, "operationflashpointbeta.exe"))
			
			||

			CWA && 
			(!stricmp(Work, "coldwarassault.exe") ||
			 !stricmp(Work, "armaresistance.exe") ||
			 !stricmp(Work, "coldwarassault_server.exe") ||
			 !stricmp(Work, "armaresistance_server.exe"))
		)
		{          
			HookAPICalls(&D3DHook);

			// Is nomap used?
			// Crappy method to check for it...
			int l = strlen(GetCommandLine());
			char *cl = new char[l+1];
			strcpy(cl, GetCommandLine());
			for(int x=0;x < l;x++) 
				cl[x] = tolower(cl[x]);
			nomap = strstr(cl,"-nomap") != 0;
			delete[] cl;

			if(!nomap)
				DebugMessage("fwatch: Warning! -nomap not used, cannot use pipes for data transfer.");
		}

		if(!stricmp(Work,"fwatch.exe"))
			HookAPICalls(&D3DHook);
    }

    return TRUE;
}

// This segment must be defined as SHARED in the .DEF
#pragma data_seg (".HookSection")		
// Shared instance for all processes.
HHOOK hHook = NULL;	
#pragma data_seg ()

TESTDLL_API LRESULT CALLBACK HookProc(int nCode, WPARAM wParam, LPARAM lParam) 
{
    return CallNextHookEx( hHook, nCode, wParam, lParam); 
}

TESTDLL_API void InstallHook()
{
    hHook = SetWindowsHookEx(WH_CBT, HookProc, hDLL, 0); 
    OutputDebugString( "fwatch: hook installed.\n" );
}

TESTDLL_API void RemoveHook()
{
    UnhookWindowsHookEx(hHook);
	OutputDebugString( "fwatch: hook removed.\n" );
}
