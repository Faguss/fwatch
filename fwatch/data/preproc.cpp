
/*PROPRIETARY CODE



*/
#include "errno.h"
#include "windows.h"

/*PROPRIETARY CODE


	
	
	
	
	
	
	
	
	
	
		
	
*/


// Remove leading and trailing whitespace
char* Trim(char *txt)
{
	while (isspace(txt[0])) 
		txt++;

	for (int i=strlen(txt)-1; i>=0 && isspace(txt[i]); i--) 
		txt[i]='\0';

	return txt;
};


// Preprocessor errors description
char errorDesc[][64]  = 
{
	"Missing #endif after condition statement",
	"Failed to #include file",
	"Incorrect #inlcude syntax",
	"Maximal number of recursions was reached",
	"Incorrect macro argument",
	"Incorrect parameter syntax",
	"Missing condition statement before #endif",
	"Unknown directive",
	"Unexpected end of file",
	"Too many parameters",
	"Too few parameters",
	"Incorrect #ifdef/#ifndef/#undef argument",
	"Missing #endif after #else"
};


// Errno description
void FERROR(int errorCode, char *errordesc)
{
	switch(errorCode)
	{
		case EPERM: sprintf(errordesc,"Operation not permitted"); break;
		case ENOENT: sprintf(errordesc,"No such file or directory"); break;
		case ESRCH: sprintf(errordesc,"No such process"); break;
		case EINTR: sprintf(errordesc,"Interrupted function call"); break;
		case EIO: sprintf(errordesc,"Input/output error"); break;
		case ENXIO: sprintf(errordesc,"No such device or address"); break;
		case E2BIG: sprintf(errordesc,"Argument list too long"); break;
		case ENOEXEC: sprintf(errordesc,"Exec format error"); break;
		case EBADF: sprintf(errordesc,"Bad file descriptor"); break;
		case ECHILD: sprintf(errordesc,"No child processes"); break;
		case EAGAIN: sprintf(errordesc,"Resource temporarily unavailable"); break;
		case ENOMEM: sprintf(errordesc,"Not enough memory available for the attempted operator"); break;
		case EACCES: sprintf(errordesc,"Permission denied"); break;
		case EFAULT: sprintf(errordesc,"Bad address"); break;
		case EBUSY: sprintf(errordesc,"Device or resource busy"); break;
		case EEXIST: sprintf(errordesc,"File exists"); break;
		case EXDEV: sprintf(errordesc,"Cross-device link. An attempt was made to move a file to a different device"); break;
		case ENODEV: sprintf(errordesc,"No such device"); break;
		case ENOTDIR: sprintf(errordesc,"Not a directory"); break;
		case EISDIR: sprintf(errordesc,"Is a directory"); break;
		case EINVAL: sprintf(errordesc,"Invalid argument"); break;
		case ENFILE: sprintf(errordesc,"Too many open files in system"); break;
		case EMFILE: sprintf(errordesc,"Too many open files"); break;
		case ENOTTY: sprintf(errordesc,"Inappropriate I/O control operation"); break;
		case EFBIG: sprintf(errordesc,"File too large"); break;
		case ENOSPC: sprintf(errordesc,"No space left on device"); break;
		case ESPIPE: sprintf(errordesc,"Invalid seek"); break;
		case EROFS: sprintf(errordesc,"Read-only file system"); break;
		case EMLINK: sprintf(errordesc,"Too many links"); break;
		case EPIPE: sprintf(errordesc,"Broken pipe"); break;
		case EDOM: sprintf(errordesc,"Invalid math argument"); break;
		case ERANGE: sprintf(errordesc,"Result too large"); break;
		case EDEADLK: sprintf(errordesc,"Command would cause a deadlock"); break;
		case ENAMETOOLONG: sprintf(errordesc,"Filename too long"); break;
		case ENOLCK: sprintf(errordesc,"Too many segment locks open, lock table is full"); break;
		case ENOSYS: sprintf(errordesc,"Function not supported"); break;
		case ENOTEMPTY: sprintf(errordesc,"Directory not empty"); break;
		case EILSEQ: sprintf(errordesc,"Illegal sequence of bytes"); break;

		default: sprintf(errordesc,"Unknown error"); break;
	};
};




int main (int argc, char* argv[])
{
	// If first argument has exe name then it's drag drop
	// working directory is user documents - change it to the program directory
	char *p				= "";
	TCHAR pwd[MAX_PATH];

	if (p = strstr(argv[0],"preproc.exe"))
	{
		int pos = p - argv[0];
		strncpy(pwd, argv[0], pos);
		pwd[pos-1] = '\0';
		SetCurrentDirectory(pwd);
	};


	// No arguments given - show information
	if (argc <= 1)
	{
		printf("preproc.exe\npreprocess given file according to OFP syntax\n\n\tUsage:\n\tpreproc [-silent] [-nolog] [-merge] inputfile [outputfile]\n\n");
		system("pause");
		return 0;
	};


	// Display current directory
	TCHAR pwd2[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, pwd2);


	// Parse given arguments
	bool silent		= false;
	bool merge		= false;
	bool nolog		= false;
	char *filename1 = "";
	char *filename2 = "";

	for (int i=1; i<argc; i++)
	{  
		if (strcmp(argv[i],"-silent") == 0) 
			silent = true;
		else
			if (strcmp(argv[i],"-merge") == 0) 
				merge = true;
		else
			if (strcmp(argv[i],"-nolog") == 0) 
				nolog = true;
		else
			if (strcmp(filename1,"") == 0)
				filename1 = Trim(argv[i]);
		else
			filename2 = Trim(argv[i]);
	};


	// Initialize classes
	/*PROPRIETARY CODE
	*/
	int	error					= 0;
	int error_errno				= 0;
	char errordesc[256]			= "";


	// Preprocess
	if (strcmp(filename1,"") == 0) 
		error = 1;
	else {
		SetCurrentDirectory(argv[0]);
		char *lastSlash = strrchr(filename1, '\\');
		if (lastSlash != NULL)
			filename1 = lastSlash + 1;
		int result = /*PROPRIETARY CODE*/
		SetCurrentDirectory(pwd2);

		if (result)
		{		
			if (strcmp(filename2,"") != 0) 
			{
				FILE *f = fopen(filename2, merge ? "a" : "w");

				if (f)
				{	
					fprintf(f, /*PROPRIETARY CODE*/);

					if (ferror(f))
						error_errno = errno,
						error		= 4;
					fclose(f);
				}
				else
					error_errno = errno,
					error = 3;
			};
		}
		else
			error = 2;
	}
	

	// Output results
	if (nolog) {}
	else
	if (silent)
	{
		char codeFile[128] = "";
		sprintf(codeFile, "fwatch\\tmp\\%d.pid", GetCurrentProcessId());

		FILE *f2 = fopen(codeFile, "w");
		if (f2)
		{
			//fprintf(f2, "\"%s\";\n\"%s\";\n\"%s\";\n", pwd2, filename1, filename2);
			fprintf(f2, "[%d, %d, %d, \"", error, /*PROPRIETARY CODE*/, error_errno);

			// Write error description
			switch (error)
			{
				case 0 : break;
				case 1 : fprintf(f2,"Input file not specified"); break;
				case 2 : fprintf(f2,errorDesc[/*PROPRIETARY CODE*/]); break;
				case 3 : FERROR(error_errno,errordesc);fprintf(f2,errordesc); break;
				case 4 : FERROR(error_errno,errordesc);fprintf(f2,errordesc); break;
				default : fprintf(f2,"Unknown error");
			};

			fprintf(f2, "\", \"%d\", \"", /*PROPRIETARY CODE*/);

			char *quote = strchr(/*PROPRIETARY CODE*/, '\"');

			if (quote == NULL) 
				fprintf(f2, /*PROPRIETARY CODE*/);
			else
			{
				char tmp[2] = "";

				for (unsigned int i=0;  i<strlen(/*PROPRIETARY CODE*/);  i++)
				{
					if (/*PROPRIETARY CODE*/ == '\"') 
						fprintf(f2,"\"");

					sprintf(tmp, "%c", /*PROPRIETARY CODE*/);
					fprintf(f2, tmp);
				};
			};

			fprintf(f2, "\", \"%d\"]", /*PROPRIETARY CODE*/);
			fclose(f2);
		};
	}
	else
	{
		printf("Current directory:\n%s\n\n",pwd2);

		if (strcmp(filename2,"") == 0) 
			printf("Output filename not given so it's a syntax check\n");
		
		switch (error)
		{
			case 0  : printf("No error"); break;
			case 1  : printf("Input file not specified\n%s",filename1); break;
			case 2  : printf("Preprocessor error %d\n%s\nLine: %d\nText: %s", /*PROPRIETARY CODE*/); break;
			case 3  : FERROR(error_errno,errordesc); printf("Failed to create output filen%d - %s",errno,errordesc); break;
			case 4  : FERROR(error_errno,errordesc); printf("Failed to write to file\n%d - %s",errno,errordesc); break;
			default : printf("Unknown error");
		};

		printf("\n\n");
		system("pause");

	};


	return error;


	//printf("result:%d\npcount:%d\nerror:%d\ncurline:%d\ntext:%s\n",result,processed.pcount(),preproc.error,preproc.curline+1,preproc.text2);
	//return 0;
};