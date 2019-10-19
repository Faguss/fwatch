// -----------------------------------------------------------------
// FILE OPERATIONS
// -----------------------------------------------------------------

case C_FILE_EXISTS:
{ // Return 1 if name exists, else 0.
	if(numP < 3) {
		QWrite("ERROR: Not enough parameters", out);
		break;
	}

	char *file = stripq(par[2]);
	if(fdbExists(file))
		QWrite("1", out);
	else
		QWrite("-1", out);
}
break;






case C_FILE_READ:
{ // Read val from file fwatcher/mdb/name
	if(numP < 4) {
		QWrite("ERROR: Not enough parameters", out);
		break;
	}

	char *file = stripq(par[2]);
	char *var = stripq(par[3]);
	QWrite(fdbGet(file, var, CommandID, out), out);
}
break;






case C_FILE_WRITE:
{ // Write var=val to file fwatcher/mdb/name
	if(numP < 5) {
		QWrite("ERROR: Not enough parameters", out);
		break;
	}

	char *file = stripq(par[2]);
	char *var = stripq(par[3]);
	char *val = par[4];
	if(fdbPut(file, var, val, false, CommandID, out))			//v1.13 additional arguments
		QWrite("1", out);
	else
		QWrite("-1", out);
}
break;






case C_FILE_QWRITE:
{ // Write var=val to file fwatcher/mdb/name without checking if it already exists
	if(numP < 5) {
		QWrite("ERROR: Not enough parameters", out);
		break;
	}

	char *file = stripq(par[2]);
	char *var = stripq(par[3]);
	char *val = par[4];
	if(fdbPutQ(file, var, val))
		QWrite("1", out);
	else
		QWrite("-1", out);
}
break;






case C_FILE_AWRITE:
{ // Append val to var in file fwatcher/mdb/name
	if(numP < 5) {
		QWrite("ERROR: Not enough parameters", out);
		break;
	}

	char *file = stripq(par[2]);
	char *var = stripq(par[3]);
	char *val = par[4];
	if(fdbPut(file, var, val, true, CommandID, out))			//v1.13 additional arguments
		QWrite("1", out);
	else
		QWrite("-1", out);
}
break;






case C_FILE_VARS:
{ // Return array of vars in file fwatcher/mdb/name
	if(numP < 3) {
		QWrite("ERROR: Not enough parameters", out);
		break;
	}

	char *file = stripq(par[2]);
	char *res = fdbVars(file);
	QWrite(res, out);
	delete[] res;
}
break;






case C_FILE_READVARS:
{ // Return all vars from file fwatcher/mdb/name
	if(numP < 3) {
		QWrite("ERROR: Not enough parameters", out);
		break;
	}

	char *file = stripq(par[2]);
	char *res = fdbReadvars(file);
	QWrite(res, out);
	delete[] res;
}
break;






case C_FILE_REMOVE:
{ // Remove a val from file fwatcher/mdb/name
	if(numP < 4) {
		QWrite("ERROR: Not enough parameters", out);
		break;
	}

	char *file = stripq(par[2]);
	char *var = stripq(par[3]);
	if(fdbRemove(file, var))
		QWrite("1", out);
	else
		QWrite("-1", out);
}
break;






case C_FILE_DELETE:
{ // Remove file fwatcher/mdb/name
	if(numP < 3) {
		QWrite("ERROR: Not enough parameters", out);
		break;
	}

	char *file = stripq(par[2]);
	if(fdbDelete(file))
		QWrite("1", out);
	else
		QWrite("-1", out);
}
break;






case C_FILE_WGET:
{ // Get a http/ftp file with wget
  // Execute wget, outputing to a named pipe
  // Not very clean but it works...
	if(numP < 3) {
		QWrite("ERROR: Not enough parameters", out);
		break;
	}

	if((GetTickCount() - lastWget) < WGET_MINWAIT) {
		// Return -1 if user is calling wget too often
		QWrite("-1", out);
		break;
	}

	// wget should always return an empty file if an error occurs so no handling should be required for it
	// TODO: maybe should return some sort of error message for OFP...

	HANDLE pipe;
	if(nomap)
		// Create a named pipe for wget to write into					
		pipe = CreateNamedPipe(file.filename, PIPE_ACCESS_DUPLEX, PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
								PIPE_UNLIMITED_INSTANCES, PIPEBUFSIZE, PIPEBUFSIZE, NMPWAIT_USE_DEFAULT_WAIT, NULL);

	char *cline = new char[256+strlen(file.filename)+strlen(par[2])];
	sprintf(cline, "-q --tries=1 --output-document=\"%s\" --timeout=3 --user-agent=fwatch/%.2f %s", file.filename, SCRIPT_VERSION, par[2]);

	// Win23 API bloat
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;
	ZeroMemory(&pi, sizeof(pi));

	// Execute wget
	CreateProcess("fwatch/data/wget.exe", cline, NULL, NULL, TRUE, HIGH_PRIORITY_CLASS, NULL, NULL, &si, &pi);
	delete[] cline;
	lastWget = GetTickCount();
				
	if(nomap) {
		// Wait for wget to do its thing
		char *buf = new char[PIPEBUFSIZE+1];
		if(!buf)
			break;

		DWORD st, foo = 1, foo2;
		do {					
			// Check exit code
			GetExitCodeProcess(pi.hProcess, &st);
			Sleep(5); // Lets not eat all cpu cycles

			// Read named pipe and output to the out pipe for OFP
			ReadFile(pipe, buf, PIPEBUFSIZE, &foo, NULL);
			//buf[foo] = 0x00;
			if(foo)
				WriteFile(out, buf, foo, &foo2, NULL);

			// Wait until the process has exited and no more data in pipe
		} while(st == STILL_ACTIVE || foo);

		WriteFile(out, "\0", 1, &foo2, NULL);
		CloseHandle(pipe); 
		delete[] buf;
	} else {
		DWORD st;
		do {					
			// Check exit code
			GetExitCodeProcess(pi.hProcess, &st);
			Sleep(10); // Lets not eat all cpu cycles
			// Wait until the process has exited
		} while(st == STILL_ACTIVE);
	}
}
break;





case C_FILE_DXDLL:
{ // Check if dxdll is in OFP folder

	FILE *f = fopen("d3d8.dll", "r");

	if (f) 
		fclose(f),
		QWrite("true", out);
	else 
		QWrite("false", out);
}
break;






case C_FILE_READ2:
{ // Read variable from selected file (any path)

	if (numP < 4) 
	{
		QWrite(":file read2 ERROR - not enough parameters", out);
		break;
	}

	char *file	= stripq(par[2]);
	char *var	= stripq(par[3]);

	fdbGet2(file, var, CommandID, out);
}
break;






case C_FILE_RENAMEMISSIONS:		
{ // Replace mpmissions

	if (numP < 4)
	{
		FWerror(100,0,CommandID,"","",numP,4,out);
		break;
	};

	par[2] = stripq(par[2]);
	par[3] = stripq(par[3]);


	// Rename current MPMissions to a given name
	if (rename("MPMissions",par[2]))
	{
		// If succeeded then rename given folder to MPMissions
		if (rename(par[3],"MPMissions"))
			FWerror(0,0,CommandID,"","",0,0,out);
		else
			// Bring back last change
			FWerror(7,errno,CommandID,par[3],"",0,0,out),
			rename(par[2],"MPMissions");
	}
	else 
		FWerror(7,errno,CommandID,"MPMissions","",0,0,out);
}
break;






case C_FILE_MODLIST:		
{ // Return list of modfolders in the game folder

	int base		  = 0;
	int pointer       = 0;
	char username[30] = "";

	if (!DedicatedServer) {
		switch(Game_Version) {
			case VER_196 : base=0x7DD184; break;
			case VER_199 : base=0x7CC144; break;
			case VER_201 : base=Game_Exe_Address+0x714C10; break;
		}

		if (base != 0) {
			ReadProcessMemory(phandle, (LPVOID)base,		  &pointer,  4, &stBytes);
			ReadProcessMemory(phandle, (LPVOID)(pointer+0x8), &username, 30, &stBytes);
		}
	}

	listDirFiles(username, out, MODFOLDERS, false, CommandID);
}
break;
