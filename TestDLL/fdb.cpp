/*
You may use this source code for personal entertainment purposes only. Any commercial-, education- or military use is strictly forbidden without permission from the author. Any programs compiled from this source and the source code itself should be made available free of charge. Any modified versions must have the source code available for free upon request.
*/

//
// by Kegetys <kegetys@dnainternet.net>
//

// Functions from reading/writing variables from files


#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <windows.h>		// for handle

const char dbprefix[] = "fwatch/mdb/";
#define DBL 11
#define LINE_MAX_LEN 2048
void QWrite_err(int code_primary, int arg_num, ...);
void QWrite(const char *str);							//v1.13
enum FWATCH_ERRORS {
	FWERROR_NONE,
	FWERROR_ERRNO = 7,
	FWERROR_MALLOC = 10,
	FWERROR_PARAM_EMPTY = 107,
	FWERROR_FILE_NOVAR = 203,
	FWERROR_FILE_APPEND = 205
};



// Strip characters and return full name ********************************************************************
char* getName(char *name) {
	static char fname[256];

	int l = strlen(name);

	for(int x=0;x<l;x++) {
		// Get rid of illegal characters
		switch(name[x]) {
			case '/':
			case '\\':
			case '>':
			case '<':
			case ':':
			case '^':
			case '*':
			case '?':
			case ';':
			case ' ':
				name[x] = '_';
				break;
			default:
				// Convert filename to lower case to avoid compatibility problems with different filesystems
				name[x] = tolower(name[x]);
				break;
		};
	}

	// Add prefix
	strcpy(fname, dbprefix);
	strncat(fname, name, 256-DBL);
	return fname;
}
//***********************************************************************************************************







// Check if file exists *************************************************************************************
bool fdbExists(char* file) {
	char *filename = getName(file);
	FILE *f = fopen(filename, "rt");
	if(f) {
		fclose(f);
		return true;
	} else 
		return false;
}
//***********************************************************************************************************







// Delete a file ********************************************************************************************
bool fdbDelete(char* file) {
	char *filename = getName(file);

	if(remove(filename) == -1)
		return false;
	else
		return true;
}
//***********************************************************************************************************







// Get a variable from file *********************************************************************************
char* fdbGet(char* file, char* svar) {

	//v1.13 is filename empty
	if (strcmp(file,"")==0)
	{
		QWrite_err(FWERROR_PARAM_EMPTY, 1, "file");
		return "-1";
	};

	static char line[LINE_MAX_LEN];
	FILE *f;
	char *filename = getName(file);

	for(unsigned int x=0;x < strlen(svar);x++)
		svar[x] = tolower(svar[x]);

		// v1.11 Count how many '=' svar has got--------------------
		size_t eqs = 0;
		size_t max = strlen(svar);
		
		for (x=0; x<max; x++) 
			if (svar[x] == '=') 
				eqs++;
		//----------------------------------------------------------

	f = fopen(filename, "rt");
	if (!f)
	{
		QWrite_err(FWERROR_ERRNO, 2, errno, filename);;		//v1.13 return error code
		return "-1";
	};

	char *ret;
	do {
		// Get var=val from line
		char *var;
		char *val="";
		char *foo;

		ret = fgets(line, LINE_MAX_LEN ,f);

//v1.1 removed lowercasing
//		for(unsigned int x=0;x < strlen(line);x++)
//			line[x] = tolower(line[x]);

		var = line;
		foo = strchr(line, '=');

			// v1.11 Skip additional '=' in the line-------------------------
			// eq is the number of '=' in the variable name
			// eq2 is the number of '=' in the current line
			if (eqs > 0) {
				size_t eqs2 = 0;
				size_t max  = strlen(line);

				for (x=0; x<max; x++) 
					if (line[x]=='=') 
						eqs2++;

				// if line contains enough '=' it could be this
				if (eqs2 > eqs)
					// skip all the '=' that are part of the varname
					for (size_t y=0; y<eqs; y++) 
						foo = strchr(foo+1,'=');
			};
			//---------------------------------------------------------------

		if(foo) {
			// Replace '=' with null character
			line[foo-line] = '\0';
			val = foo+1;

			for(unsigned int x=0;x < strlen(var);x++)		//v1.1 added lowercasing for var name only
				var[x] = tolower(var[x]);

			// Remove CR/LF
			if(val[strlen(val)-1] == 0x0A) val[strlen(val)-1] = '\0';
			if(val[strlen(val)-1] == 0x0D) val[strlen(val)-1] = '\0';

			// Is this the var we are looking for?
			if(!strcmp(var, svar)) {
				fclose(f);
				QWrite_err(FWERROR_NONE, 0);	//v1.13 return error code
				return val;
			}
		}
	} while(ret);
	
	fclose(f);
	QWrite_err(FWERROR_FILE_NOVAR, 2, svar, filename);
	return "-1";
}
//***********************************************************************************************************








// Put a variable to file ***********************************************************************************
bool fdbPut(char* file, char* svar, char* val, bool append) {

	//v1.13 is filename empty
	if (strcmp(file,"")==0)
	{
		QWrite_err(FWERROR_PARAM_EMPTY, 1, "file");
		return false;
	};
	
	FILE *f;
	char* buf = 0;
	size_t bufsize = 0;
	bool appended = false;			//v1.1
	char *filename = getName(file);
	size_t svar_len = strlen(svar);

	for(unsigned int x=0; x<svar_len; x++)
		svar[x] = tolower(svar[x]);

		// v1.11 Count how many '=' svar has got--------------------
		size_t eqs = 0;

		for (x=0; x<svar_len; x++) 
			if (svar[x] == '=') 
				eqs++;
		//----------------------------------------------------------

	f = fopen(filename, "rb");
	if(f) {
		// File exists, read it to memory skipping the var we are currently about to re-save

		// Find file size, allocate buffer
		fseek(f, 0, SEEK_END);
		size_t fsize = ftell(f);
		buf = new char[fsize+LINE_MAX_LEN+2];
		if(!buf)
		{
			QWrite_err(FWERROR_MALLOC, 2, "buf", fsize+LINE_MAX_LEN+2);		//v1.13 return error code
			return false;
		};

		size_t i = 0;
		fseek(f, 0, SEEK_SET);

		char line[LINE_MAX_LEN];
		char *ret;
		while((ret = fgets(line, LINE_MAX_LEN ,f))) {

			// Get var=val from line
			char var[LINE_MAX_LEN];
			char *foo;

//v1.1 removed lowercasing
//			for(unsigned int x=0;x < strlen(line);x++)
//				line[x] = tolower(line[x]);

			foo = strchr(line, '=');

			// v1.11 Skip additional '=' in the line-------------------------
			if (eqs > 0) {
				size_t eqs2 = 0;
				size_t max  = strlen(line);

				for (x=0; x<max; x++) 
					if (line[x] == '=') 
						eqs2++;

				if (eqs2 > eqs)
					for (size_t y=0; y<eqs; y++) 
						foo = strchr(foo+1,'=');
			};
			//---------------------------------------------------------------

			if(foo) {
				// Get var name
				memcpy(var, line, foo-line);
				var[foo-line] = '\0';

				// Is this the var we are going to save?
				if(!strcmp(var, svar)) {
					if(!append) {
						// Skip it
						continue;
					} else {
						// Append to current value
						appended = true;			//v1.1
						size_t ll = strlen(line);
						size_t vl = strlen(val);
						if(ll+vl+1 < LINE_MAX_LEN) {
							char *nval = val;
							
							if(line[ll-1] == '\n') // Remove LF from end
								line[ll-1] = 0x00;
							if(line[ll-2] == '"' && nval[0] == '"' && vl > 1) {
								// Appending two strings
								line[ll-2] = 0x00;
								nval++;
							}
							if(line[ll-2] == ']' && nval[0] == '[' && vl > 1) {
								// Appending arrays
								//v1.1 don't add comma if empty array OR appending empty array
								if (line[ll-3] == '[' || nval[1]==']') line[ll-2]=0x00; else line[ll-2]=','; 
								nval++;
							}

							//strcat(line, "+");
							strcat(line, nval);
							strcat(line, "\n");		//v1.1 fixed appending
							
						} else {
							// Too long line, cant append anymore
							fclose(f);
							delete[] buf;
							QWrite_err(FWERROR_FILE_APPEND, 3, svar, ll+vl+1, filename);		//v1.13 return error code
							return false;
						}
					}
				}
			}
			size_t l = strlen(line);
			memcpy(buf+bufsize, line, l+1);
			bufsize += l;
		};
		fclose(f);
	}

	// Rewrite the file
	f = fopen(filename, "wb");
	if(!f) {
		QWrite_err(FWERROR_ERRNO, 2, errno, filename);		//v1.13 return error code
		delete[] buf;
		return false;
	}

	// Write existing data
	if(buf && bufsize) {
		if(buf[strlen(buf)-1] != '\n')			//v1.13 first append then write
			strcat(buf, "\n");

		fwrite(buf, 1, strlen(buf), f);
		if (ferror(f)) 
			QWrite_err(FWERROR_ERRNO, 2, errno, filename);		//v1.13 return error code
	}

	if(!append || append && !appended) {		//v1.1 if awrite and var doesn't exist then add it
		// Write new var value
		char *foo = new char[strlen(svar) + strlen(val) + 3];
		if(!foo)  {
			delete[] buf;
			QWrite_err(FWERROR_MALLOC, 2, "foo", strlen(svar)+strlen(val)+3);		//v1.13 return error code
			return false;
		}

		sprintf(foo, "%s=%s\n", svar, val);
		fwrite(foo, 1, strlen(foo), f);
		if (ferror(f)) 
			QWrite_err(FWERROR_ERRNO, 2, errno, filename);		//v1.13 return error code
		delete[] foo;
	}

	fclose(f);
	if(buf)
		delete[] buf;

	QWrite_err(FWERROR_NONE, 0);
	return true;
}
//***********************************************************************************************************







// Put a variable to file without checking if it exists *****************************************************
bool fdbPutQ(char* file, char* svar, char* val) {
	FILE *f;
	char *filename = getName(file);

	for(unsigned int x=0;x < strlen(svar);x++)
		svar[x] = tolower(svar[x]);

	// Write to end of the file
	f = fopen(filename, "ab");
	if(!f) {
		return false;
	}

	// Write new var value
	// TODO: Might need a check if the existing file ends in a CR/LF
	char *foo = new char[strlen(svar) + strlen(val) + 3];
	if(!foo)
		return false;
	sprintf(foo, "%s=%s\n", svar, val);
	fwrite(foo, 1, strlen(foo), f);
	delete[] foo;

	fclose(f);
	return true;
}
//***********************************************************************************************************







// Append to char array *************************************************************************************
char* append(char* arr, int clen, char* buf, int len) {
	char* foo = new char[clen+len+1];
	if(!foo) {
		// This would not be a very good thing to happen
		delete[] arr;
		return false; 
	}

	foo[clen+len] = 0x00;
	memcpy(foo, arr, clen);
	memcpy(foo+clen, buf, len);
	delete[] arr;
	return foo;
}
//***********************************************************************************************************







// Get a list of vars from file *****************************************************************************
char* fdbVars(char* file) {
	FILE *f;
	bool notfirst = false;
	char *filename = getName(file);

	f = fopen(filename, "rt");
	if(!f) {
		return "-1";
	}

	char line[LINE_MAX_LEN];
	char *ret;
	char *buf = new char[2];
	if(!buf)
		return false;
	buf[0] = '[';
	buf[1] = 0x00;

	while(ret = fgets(line, LINE_MAX_LEN ,f)) {
		// Get var=val from line
		char *var;
		char *foo;

		for(unsigned int x=0;x < strlen(line);x++)
			line[x] = tolower(line[x]);

		var = line;
		foo = strchr(line, '=');

			// v1.11 Skip additional '=' in the line-------------------------
			int eqs=0;
			int eqs2=0;
			for (x=0; x<strlen(line); x++) if(line[x]=='=') eqs2++;

			if (eqs2 > 1)
			{
				// iterate string from the end
				bool insideQuot=false, foundCentral=false;

				for (int y=strlen(line)-1; y>0; y--)
				{
					// first find '=' that separates name from value
					if (!foundCentral)
					{
						if (line[y] == '"') insideQuot=!insideQuot;
						if (line[y] == '=' && !insideQuot) foundCentral=true;
					} else
					{
						// count how many '=' variable name has
						if (line[y] == '=') eqs++;
					};
				};

				// finally skip eqs amount of '='
				for (int z=0; z<eqs; z++) foo=strchr(foo+1,'=');
			};
			//---------------------------------------------------------------

		if(foo) {
			if(notfirst) {
				buf = append(buf, strlen(buf), ",", 1);
				if(!buf)
					return false;
			}

			// Replace '=' with null character
			line[foo-line] = '\0';

			char foo[LINE_MAX_LEN+3];
			sprintf(foo, "\"%s\"", var);
			buf = append(buf, strlen(buf), foo, strlen(foo));
			if(!buf)
				return false;

			notfirst = true;
		}
	};	
	buf = append(buf, strlen(buf), "]", 1);
	if(!buf)
		return false;

	fclose(f);
	//delete[] buf;
	return buf;
}
//***********************************************************************************************************








// Read all vars from file **********************************************************************************
char* fdbReadvars(char* file) {
	FILE *f;
	char *filename = getName(file);

	f = fopen(filename, "rt");
	if(!f) {
		return "-1";
	}

	char line[LINE_MAX_LEN];
	char *ret;
	char *buf = new char[1];
	if(!buf)
		return false;
	buf[0] = 0x00;

	while(ret = fgets(line, LINE_MAX_LEN ,f)) {
		// Get var=val from line
		char *var;
		char *val;
		char *foo;

		for(unsigned int x=0;x < strlen(line);x++)
			line[x] = tolower(line[x]);

		var = line;
		foo = strchr(line, '=');
		if(foo) {

			// Replace '=' with null character
			line[foo-line] = '\0';
			val = foo+1;

			// Remove CR/LF
			if(val[strlen(val)-1] == 0x0A) val[strlen(val)-1] = '\0';
			if(val[strlen(val)-1] == 0x0D) val[strlen(val)-1] = '\0';

			char foo[LINE_MAX_LEN+3];
			sprintf(foo, "%s=%s;", var, val);
			buf = append(buf, strlen(buf), foo, strlen(foo));
			if(!buf)
				return false;

		}
	};	

	buf = append(buf, strlen(buf), "1;", 2);
	if(!buf)
		return false;

	fclose(f);
	//delete[] buf;
	return buf;
}
//***********************************************************************************************************







// Remove a var from file ***********************************************************************************
bool fdbRemove(char* file, char* svar) {
	FILE *f;
	bool found = false;
	char* buf = 0;
	int bufsize = 0;
	char *filename = getName(file);

	for(unsigned int x=0;x < strlen(svar);x++)
		svar[x] = tolower(svar[x]);

		// v1.11 Count how many '=' svar has got--------------------
		int eqs = 0;
		for (x=0; x<strlen(svar); x++) if(svar[x]=='=') eqs++;
		//----------------------------------------------------------

	f = fopen(filename, "rb");
	if(!f) {
		return false;
	}

	// Find file size, allocate buffer
	fseek(f, 0, SEEK_END);
	int fsize = ftell(f);
	buf = new char[fsize+2];
	if(!buf)
		return false;

	int i = 0;
	fseek(f, 0, SEEK_SET);

	char line[LINE_MAX_LEN];
	char *ret;
	while(ret = fgets(line, LINE_MAX_LEN ,f)) {
		// Get var=val from line
		char var[LINE_MAX_LEN];
		char *foo;

		for(unsigned int x=0;x < strlen(line);x++)
			line[x] = tolower(line[x]);

		foo = strchr(line, '=');

			// v1.11 Skip additional '=' in the line-------------------------
			if (eqs > 0)
			{
				int eqs2 = 0;
				for (x=0; x<strlen(line); x++) if(line[x]=='=') eqs2++;

				if (eqs2 > eqs)
					for (int y=0; y<eqs; y++) foo=strchr(foo+1,'=');
			};
			//---------------------------------------------------------------

		if(foo) {
			// Get var name
			memcpy(var, line, foo-line);
			var[foo-line] = '\0';

			// Is this the var we are going to delete?
			if(!strcmp(var, svar)) {
				// Skip it
				found = true;
				continue;
			}
		}

		int l = strlen(line);
		memcpy(buf+bufsize, line, l+1);
		bufsize += l;
	};
	fclose(f);
	
	if(!found) {
		// Var wasn't in the file so dont bother rewriting
		delete[] buf;
		return false;
	}

	// Rewrite the file
	f = fopen(filename, "wb");
	if(!f) {
		delete[] buf;
		return false;
	}

	// Write existing data
	if(buf && bufsize) {
		fwrite(buf, 1, strlen(buf), f);
		if(buf[strlen(buf)-1] != '\n')
			fwrite((const void *) '\n', 1, 1, f);
	}
	fclose(f);

	delete[] buf;
	return true;
}
//***********************************************************************************************************







//v1.1 Read variable from any file **************************************************************************
void fdbGet2(char* file, char* svar)
{
	if (strcmp(file,"")==0)
	{
		QWrite_err(FWERROR_PARAM_EMPTY, 1, "file");
		return;
	};
	
	static char line[LINE_MAX_LEN];
	FILE *f;

	for(unsigned int x=0;x < strlen(svar);x++)
		svar[x] = tolower(svar[x]);

	f = fopen(file, "rt");
	if (!f)
	{
		QWrite_err(FWERROR_ERRNO, 2, errno, file);
		QWrite("-1");
		return;
	};


	char *ret;
	do {
		// Get var=val from line
		char *var;
		char *val="";
		char *foo;

		ret = fgets(line, LINE_MAX_LEN ,f);
		var = line;
		
		foo = strchr(line, '=');		// foo is now: =[value]

		if(foo) {
			
			// Replace brackets
			if (strchr(line,'[]') && strchr(line,'{') && strchr(line,'}'))
			{
				int l = strlen(line);
				for (int i=0; i<l; i++) if (line[i]=='{') {line[i]='['; break;};
				for (i=l-1; i>=0; i--) if (line[i]=='}') {line[i]=']'; break;};
			};
			
			// Replace '=' with null character
			line[foo-line] = '\0';		// line is now: [var name]
			val = foo+1;

			for(unsigned int x=0;x < strlen(var);x++)		//lowercase var name
				var[x] = tolower(var[x]);

			// Remove CR/LF and semi-colon
			if(val[strlen(val)-1] == '\n') val[strlen(val)-1] = '\0';
			if(val[strlen(val)-1] == '\r') val[strlen(val)-1] = '\0';
			if(val[strlen(val)-1] == ';') val[strlen(val)-1] = '\0';

			// Is this the var we are looking for?
			if(!strcmp(var, svar)) {
				fclose(f);
				QWrite_err(FWERROR_NONE, 0);
				QWrite(val);
				return;
			}
		}
	} while(ret);
	
	fclose(f);
	QWrite_err(FWERROR_FILE_NOVAR, 2, svar, file);
	QWrite("-1");
	return;
}
//***********************************************************************************************************