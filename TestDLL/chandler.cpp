/*
You may use this source code for personal entertainment purposes only. Any commercial-, education- or military use is strictly forbidden without permission from the author. Any programs compiled from this source and the source code itself should be made available free of charge. Any modified versions must have the source code available for free upon request.
*/


//
// by Kegetys <kegetys@dnainternet.net>
//

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include "testdll.h"

void __cdecl watchFile(void *p);

// This function is called when OFP tries to access a filename starting with "scripts\:"
// with the rest of the filename as the "command" parameter. It must return a file handle
// for OFP to execute after finished.
HANDLE HandleCommand(char *command) {

	if(nomap) {
		// With -nomap parameter OFP uses createfile/readfile so a pipe works

		// Create a pipe where all our stuff will be written on to, this is much faster than a temp file...
		HANDLE rp, wp;
		int ret = CreatePipe(&rp, &wp, NULL, 1024*1024); // Suggest a 1MB pipe

		if(ret) {
			FULLHANDLE file;
			file.handle = wp;

			// Create an unique name for wget pipe
			sprintf(file.filename, "\\\\.\\pipe\\fwatchwgetpipe%d", GetTickCount());

			// Call script handler
			ParseScript(command, file);

			// Close write pipe, return read pipe handle
			CloseHandle(wp);
			return rp;
		}
	} else {
		// Without -nomap ofp uses file mappings, so we must create a temp file, which is slow...

		FULLHANDLE file;

		// Create a temporary file, it will be automatically deleted when OFP closes the handle
		char path[256];
		GetTempPath(256, path);
		GetTempFileName(path, "FWTC", 0, file.filename);
		file.handle = CreateFile(file.filename, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, 
							CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
							// FILE_ATTRIBUTE_NORMAL|FILE_FLAG_DELETE_ON_CLOSE, NULL); 
							// The not-so-well-documented Windows API strikes again: FILE_FLAG_DELETE_ON_CLOSE flag
							// doesn't seem to work with FILE_SHARE_WRITE so we need to take care of the deleting ourselves

		if(file.handle != INVALID_HANDLE_VALUE) {
			// Call script handler		
			ParseScript(command, file);
		}

		// Spawn watcher process
		FULLHANDLE *temp;
		temp = new FULLHANDLE;
		if(!temp) {
			CloseHandle(file.handle);
			return false;
		}
		memcpy(temp, &file, sizeof(FULLHANDLE));
		_beginthread(watchFile, 0, (void*) (temp));

		// Rewind the tape and return handle to OFP
		SetFilePointer(file.handle, 0, NULL, FILE_BEGIN);
		return file.handle;
	}

	return false;
}


// This thread will wait for OFP to close the file handle and then delete the file
void __cdecl watchFile(void *p) {
	FULLHANDLE *h = (FULLHANDLE*) p;
	//DebugMessage("Watching %s...", h->filename);
	BY_HANDLE_FILE_INFORMATION foo;
	int poo, i = 0;

	// Wait until we can't get info from it anymore
	do {
		poo = GetFileInformationByHandle(h->handle, &foo);
		Sleep(250);
		i++;
	} while(poo && i < 100); // Give up after 25 seconds

	// Delete it
	if(!DeleteFile(h->filename)) 
		DebugMessage("Delete Failed for %s!", h->filename);
	/*else
		DebugMessage("Deleted %s.", h->filename);*/		

	delete p; // Release memory
	_endthread();		
}