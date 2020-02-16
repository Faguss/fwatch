// Program for monitoring other program's return value
// by Faguss (ofp-faguss.com) 10.05.15

#include <fstream>		// file operations
#include <windows.h>	// winapi

int main(int argc, char *argv[])
{   
    // Exit code and process id
    DWORD ec = 0, 
          pid = 0;
          
	// Read arguments
    for (int i=1; i<argc; i++)
		if (strncmp(argv[i],"-pid=",5)==0) 
            pid=atoi(argv[i]+5);
       
	// Open process
    HANDLE phandle = OpenProcess(PROCESS_ALL_ACCESS, 0, pid);
	if (phandle != NULL)
	{
		// Get return value
        do 
			Sleep(5),
            GetExitCodeProcess(phandle, &ec);
		while (ec == STILL_ACTIVE);
		CloseHandle(phandle);
	};
	
	// Write exit code to a text file
    char filename[128] = "";
    sprintf(filename, "fwatch\\tmp\\%d.pid", pid);
    FILE *fd = fopen(filename, "w");
	fprintf(fd, "[%d]", ec);
	fclose(fd);
	
	int wait    = 0;
	bool exists = true;
	while (wait++ < 5) {
		Sleep(1000);
		bool exists = GetFileAttributes(filename) != INVALID_FILE_ATTRIBUTES;
		if (!exists)
			break;
	}
	
	if (exists)
		unlink(filename);
	
	return 0;
};
