#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlwapi.h>
#include "testdll.h"
#include "..\apihijack.h"
#include <stdio.h>
#include <tlhelp32.h>	// Traversing process modules

GLOBAL_VARIABLES_TESTDLL global = {
	0,   // exe_index
	0,   // exe_address
	0,	 // exe_address_scroll
	0,   // exe_address_ifc22
	0,   // is_server

	{0}, // RESTORE_MEM
	{0}, // RESTORE_BYT
	0,   // nomap
	0,   // ErrorLog_Enabled
	0,   // ErrorLog_Started

	0,   // option_error_output

	"",  // current_dir
	0,   // current_dir_length;
	"",  // mission_path
	0,   // mission_path_length
	"",  // mission_path_previous
	0,   // mission_path_previous_length
	0,   // mission_path_savetime
	0,   // pid
	0,	 // external_program_id
	0,   // lastWget
	
	0,   // extCamOffset
	{0}, // RESTORE_INT
	{0}, // RESTORE_FLT
	{0}, // RESTORE_HUD_INT
	{0}  // RESTORE_HUD_FLT
};

HINSTANCE hDLL;







// Function pointer types.
typedef HANDLE (WINAPI *NewCreateFileA_Type)(LPCSTR lpFileName, DWORD  dwDesiredAccess, DWORD  dwShareMode, LPSECURITY_ATTRIBUTES  lpSecurityAttributes, DWORD  dwCreationDistribution, DWORD  dwFlagsAndAttributes, HANDLE  hTemplateFile);

// Function prototypes.
HANDLE WINAPI NewCreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD  dwShareMode, LPSECURITY_ATTRIBUTES  lpSecurityAttributes, DWORD  dwCreationDistribution, DWORD  dwFlagsAndAttributes, HANDLE  hTemplateFile);

HANDLE HandleCommand(String &command, bool nomap); // in chandler.cpp

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
void DebugMessage(const char *first, ...)
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
	size_t len = strlen(lpFileName);

	// Check if game is starting a new mission; if so then save mission dir path to a file
	if (!strchr(lpFileName, ':')) {
		if (!strncmpi("\\init.sqs", lpFileName+len-9, 9))
			createPathSqf(lpFileName, len, -8);

		if (!strncmpi(".html", lpFileName+len-5, 5)) {
			char *pch = strstr(lpFileName,"briefing.");

			if (pch)
				createPathSqf(lpFileName, len, pch - lpFileName);
		}

		if (!strncmpi(".pbo", lpFileName+len-4, 4))
			if (!strncmpi("missions\\", lpFileName, 9)  ||  !strncmpi("mpmissions\\", lpFileName, 11))
				createPathSqf(lpFileName, len, -4);

		if (!strncmpi("description.ext", lpFileName+len-15, 15))
			if (!strncmpi("campaigns\\", lpFileName, 10))
				createPathSqf(lpFileName, len, -15);
	}


	if(!strncmp(":", lpFileName, 1)  ||  !strncmp("scripts\\:", lpFileName, 9)) {
		// This filename is meant for us
		if(dwFlagsAndAttributes == 128) {
			// Script is being opened for execution

			String com = {
				(char*)lpFileName + (!strncmp(":", lpFileName, 1) ? 1 : 9),
				len - (!strncmp(":", lpFileName, 1) ? 1 : 9)
			};

			//DebugMessage("fwatch: command: %s", command);

			// Call HandleCommand, and return returned file handle to OFP
			return HandleCommand(com, global.nomap);
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

		// On CWA there shouldn't be "Res" folder so we redirect from res\addons to addons
		if (global_exe_version[global.exe_index]!=VER_196  &&  strncmpi(lpFileName,"anims\\..\\Res\\addons\\",20)==0) {
			if (len < 511) {
				strcpy(global.path_buffer, lpFileName);
				shift_buffer_chunk(global.path_buffer, 13, len+1, 4, OPTION_LEFT);
				lpFileName = global.path_buffer;
			}
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
        memset(global.restore_memory, 0, 105);
		hDLL = hModule;

        // We don't need thread notifications for what we're doing.  Thus, get
        // rid of them, thereby eliminating some of the overhead of this DLL
        DisableThreadLibraryCalls( hModule );

        // Only hook the APIs if this is the ofp proess.
		char exe_name[256];
        GetModuleFileName( GetModuleHandle( NULL ), exe_name, sizeof(exe_name) );
        PathStripPath( exe_name );

		DebugMessage("fwatch: checking process %s", exe_name);
		
		// Check if it's a valid executable
		for (int i=0; i<global_exe_num; i++) {
			if (stricmp(exe_name, global_exe_name[i]) == 0) {
				global.exe_index = i;
				global.is_server = strstr(global_exe_name[i],"_server.exe") != NULL;
				global.pid       = GetCurrentProcessId();
				global.outf      = NULL;

				GetCurrentDirectory(MAX_PATH, global.game_dir);
				global.game_dir_length = strlen(global.game_dir);

				// Find addresses of modules
				HANDLE phandle = OpenProcess(PROCESS_ALL_ACCESS, 0, global.pid);

				if (phandle) {
					HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, global.pid);

					if (hSnap != INVALID_HANDLE_VALUE) {
						MODULEENTRY32 xModule;
						memset(&xModule, 0, sizeof(xModule));
						xModule.dwSize = sizeof(xModule);

						if (Module32First(hSnap, &xModule)) {
							global.exe_address = (DWORD)xModule.modBaseAddr;
							xModule.dwSize     = sizeof(MODULEENTRY32);

							do {
								if (lstrcmpi(xModule.szModule, (LPCTSTR)"ifc22.dll") == 0)
									global.exe_address_ifc22 = (DWORD)xModule.modBaseAddr;

								if (lstrcmpi(xModule.szModule, (LPCTSTR)"dinput8.dll") == 0) {
									global.exe_address_scroll = (DWORD)xModule.modBaseAddr;
									
									// Distance to scroll depends on module size
									if (xModule.modBaseSize == 233472)
										global.exe_address_scroll += 0x2D848;	// old computers
									else
										global.exe_address_scroll += 0x2C1C8;	// new computers
								}
							} while (Module32Next(hSnap, &xModule));
						}
						
						CloseHandle(hSnap);
					}

					CloseHandle(phandle);
				}

				HookAPICalls(&D3DHook);

				// Is nomap used?	
				// Crappy method to check for it...
				int l    = strlen(GetCommandLine());
				char *cl = new char[l+1];

				strcpy(cl, GetCommandLine());

				for (int x=0; x<l; x++) 
					cl[x] = tolower(cl[x]);

				global. nomap = strstr(cl,"-nomap") != 0;
				delete[] cl;

				if (!global.nomap)
					DebugMessage("fwatch: Warning! -nomap not used, cannot use pipes for data transfer.");
			}
		}

		if (!stricmp(exe_name,"fwatch.exe"))
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
