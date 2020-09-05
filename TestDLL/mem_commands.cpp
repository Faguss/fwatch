// -----------------------------------------------------------------
// MEMORY OPERATIONS
// -----------------------------------------------------------------

case C_MEM_GETCURSOR:
{ // Get mouse cursor position from memory

	float X     = -1;
	float Y     = -1;
	int address = 0;

	switch(game_version) {
		case VER_196 : address=0x79E94C; break;
		case VER_199 : address=0x78DA44; break;
		case VER_201 : address=global.exe_address+0x71611C; break;
	}

	if (address != 0) {
		ReadProcessMemory(phandle, (LPVOID)address,	    &X, 4, &stBytes);
		ReadProcessMemory(phandle, (LPVOID)(address+4), &Y, 4, &stBytes);
	}

	char tmp[32] = "";
	sprintf(tmp, "[%.6f,%.6f]", ++X/2, ++Y/2);
	QWrite(tmp, out);
}
break;






case C_MEM_SETCURSOR:
{ // Write mouse cursor position to memory

	if (numP < 4) {
		QWrite(":mem setcursor ERROR - not enough parameters", out);
		break;
	}

	float X     = (float)atof(par[2]) * 2 - 1;
	float Y     = (float)atof(par[3]) * 2 - 1;
	int address = 0;

	switch(game_version) {
		case VER_196 : address=0x79E94C; break;
		case VER_199 : address=0x78DA44; break;
		case VER_201 : address=global.exe_address+0x71611C; break;
	}

	if (address != 0) {
		WriteProcessMemory(phandle, (LPVOID)address,     &X, 4, &stBytes);
		WriteProcessMemory(phandle, (LPVOID)(address+4), &Y, 4, &stBytes);
	}
}
break;






case C_MEM_GETWORLD:
{ // Read island shortcut from memory

	char island[80] = "";
	int address     = 0;
	
	switch(game_version) {
		case VER_196 : address=!global.DedicatedServer ? 0x7DD130 : 0x75A3E8;
		case VER_199 : address=!global.DedicatedServer ? 0x7CC0F0 : 0x75A478;
	}

	if (!address)
		break;

	ReadProcessMemory(phandle, (LPVOID)address, &island, 80, &stBytes);

	// Simple or extended info
	if (numP<=2 || strcmpi(par[2],"extended")!=0)
		QWrite(island, out);
	else {
		QWrite("[\"", out);
		QWrite(island, out);

		// Get island size
		int base      = !global.CWA ? 0x7B3ACC : 0x7A2C0C;
		int pointer   = 0;
		int landSize  = 0;
		char tmp[128] = "";

		ReadProcessMemory(phandle, (LPVOID)base,			&pointer, 4, &stBytes);
		ReadProcessMemory(phandle, (LPVOID)(pointer+0x8),	&landSize, 4, &stBytes);


		// Get geographical coordinates
		float latitude  = 0;
		float longitude = 0;

		base = !global.DedicatedServer ? (!global.CWA ? 0x79F8D0 : 0x78E9C8) : (!global.CWA ? 0x71F738 : 0x71F788);
		ReadProcessMemory(phandle, (LPVOID)base, &pointer, 4, &stBytes);
		pointer += 0x7DC;

		ReadProcessMemory(phandle, (LPVOID)pointer,		&latitude,  4, &stBytes);
		ReadProcessMemory(phandle, (LPVOID)(pointer+4),	&longitude, 4, &stBytes);

		sprintf(tmp, "\",%d,%.6f,%.6f]", landSize, rad2deg(latitude), rad2deg(longitude));
		QWrite(tmp, out);
	}
}
break;






case C_MEM_SETMAP:
{ // Change map state in memory

	if (numP < 3) 
	{
		QWrite(":mem setmap ERROR - not enough parameters", out);
		break;
	}

	// If user passed boolean then replace it with integer
	if (strcmpi(par[2],"true") == 0) 
		strcpy(par[2], "1");

	if (strcmpi(par[2],"false") == 0) 
		strcpy(par[2], "0");

	int base = !global.CWA ? 0x7B4028 : 0x7A3128;
	int	pointer = 0;
	int	isMap = atoi(par[2]);

	// if user passed value different than zero
	if (isMap != 0) 
		isMap = 1;

	ReadProcessMemory (phandle, (LPVOID)base,			 &pointer, 4, &stBytes);     
	WriteProcessMemory(phandle, (LPVOID)(pointer+0x7CF), &isMap,   1, &stBytes);
}
break;








case C_MEM_GETGRAPHICS:
{ // Get graphic options values from memory

	int pointer		= 0;
	int base		= !global.CWA ? 0x789D88 : 0x778E80;
	int resBase		= !global.CWA ? 0x7DCFB4 : 0x7CBF74;
	int resX		= 0;
	int resY		= 0;
	int refresh		= 0;
	int oshad		= 0; 
	int vshad		= 0; 
	int cloud		= 0;
	int blood		= 0;
	int multitex	= 0;
	int maxobjects	= 0;
	int maxlights	= 0;

	float bright			 = 0;
	float gamma				 = 0;
	float frame				 = 0;
	float vqual				 = 0;
	float tedet				 = 0;
	float visibility		 = 0;
	float horizonz			 = 0;
	float objectsz			 = 0;
	float radarz			 = 0;
	float shadowsz			 = 0;
	float tracktimetolive	 = 0;
	float invtracktimetolive = 0;
	float fovUI[6];
	float current_terrain_detail = 0;
	


	// Read common pointer for fullscren resolution, refresh, brightness, gamma, multitexturing
	ReadProcessMemory(phandle, (LPVOID)base, &pointer, 4, &stBytes);


	// Resolution windowed
	ReadProcessMemory(phandle, (LPVOID)resBase,		&resX, 4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)(resBase+4), &resY, 4, &stBytes);

	if (resX == 0)		// if fullscreen
	{
		ReadProcessMemory(phandle, (LPVOID)(pointer+0x128), &resX, 4, &stBytes);
		ReadProcessMemory(phandle, (LPVOID)(pointer+0x12C), &resY, 4, &stBytes);
	};


	// Refresh rate, brightness, gamma, multitexturing
	ReadProcessMemory(phandle, (LPVOID)(pointer+0x2C),	&bright,  4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)(pointer+0x39),	&multitex,1, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)(pointer+0x138),	&refresh, 4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)(pointer+0x758),	&gamma,	  4, &stBytes);

	if (multitex > 0)	// 0 for disabled, 256 for enabled 
		multitex = 1;


	// field of view and interface
	for (int i=0; i<6; i++)
		ReadProcessMemory(phandle, (LPVOID)(pointer+0x40+i*4), &fovUI[i], 4, &stBytes);


	// Framerate, terrain detail, visual quality
	base = !global.CWA ? 0x7B4028 : 0x7A3128;
	ReadProcessMemory(phandle, (LPVOID)base,			&pointer, 4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)(pointer+0x524), &frame,	4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)(pointer+0x53C), &vqual,	4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)(pointer+0x590), &tedet,	4, &stBytes);


	// Visibility
	ReadProcessMemory(phandle, (LPVOID)(!global.CWA ? 0x7DD068 : 0x7CC028), &horizonz, 4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)(!global.CWA ? 0x7DD06C : 0x7CC02C), &objectsz, 4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)(!global.CWA ? 0x7DD06C : 0x7CC02C), &radarz, 4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)(!global.CWA ? 0x7DD070 : 0x7CC030), &visibility, 4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)(!global.CWA ? 0x7DD074 : 0x7CC034), &radarz, 4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)(!global.CWA ? 0x7DD078 : 0x7CC038), &shadowsz, 4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)(!global.CWA ? 0x7DD07C : 0x7CC03C), &maxobjects, 4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)(!global.CWA ? 0x7DD080 : 0x7CC040), &tracktimetolive, 4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)(!global.CWA ? 0x7DD084 : 0x7CC044), &invtracktimetolive, 4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)(!global.CWA ? 0x7DD08C : 0x7CC04C), &maxlights, 4, &stBytes);

	// Object shadows, Vehicle Shadows, Cloudlets
	base = !global.CWA ? 0x79F8D0 : 0x78E9C8;
	ReadProcessMemory(phandle, (LPVOID)base,			&pointer,4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)(pointer+0x5B0), &oshad,	 1, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)(pointer+0x5B1), &vshad,	 1, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)(pointer+0x5B2), &cloud,	 1, &stBytes);


	// Blood
	base = !global.CWA ? 0x7DD0BB : 0x7CC07B;
	ReadProcessMemory(phandle, (LPVOID)base, &blood, 1, &stBytes);

	// Current terrain detail
	base = !global.CWA ? 0x7B3ACC : 0x7A2C0C;
	ReadProcessMemory(phandle, (LPVOID)base, &pointer, 4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)(pointer+0x2C), &current_terrain_detail, 4, &stBytes);


	char tmp[256] = "";
	sprintf (tmp, "[%d,%d,%d,%s,%s,%s,%s,%s,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%d,%.6f,%.6f,%d,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f]", 
			resX, 
			resY, 
			refresh,
			getBool(multitex), 
			getBool(oshad), 
			getBool(vshad), 
			getBool(cloud), 
			getBool(blood),
			bright, 
			gamma, 
			frame, 
			vqual, 
			visibility, 
			tedet,
			horizonz,
			objectsz,
			radarz,
			shadowsz,
			maxobjects,
			tracktimetolive,
			invtracktimetolive,
			maxlights,
			fovUI[0],
			fovUI[1],
			fovUI[2],
			fovUI[3],
			fovUI[4],
			fovUI[5],
			current_terrain_detail
	);

	QWrite(tmp, out);
}
break;






case C_MEM_SETGRAPHICS:
{ // Change graphical options

	if (numP < 3) 
		break;

	// brightness, gamma, vquality, objectshadows, vehicleshadows, cloudlets, visibility
	// b, g, vq, os, vs, c, v

	// Read pointers
	int p1 = 0;
	int	p2 = 0;
	int	p3 = 0;
	int b1 = !global.CWA ? 0x789D88 : 0x778E80;
	int b2 = !global.CWA ? 0x7B4028 : 0x7A3128;
	int b3 = !global.CWA ? 0x79F8D0 : 0x78E9C8;

	ReadProcessMemory(phandle, (LPVOID)b1, &p1, 4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)b2, &p2, 4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)b3, &p3, 4, &stBytes);


	// Parse input
	for (int i=2; i<numP; i++) {
		char *arg = stripq(par[i]);
		char *pch = strchr(arg, ':');

		if (pch==NULL) 
			continue;

		int pos   = pch - arg;
		arg[pos]  = '\0';
		char *val =	Trim(arg+pos+1);

		if (strcmpi(val, "true") == 0) 
			strcpy(val, "1");

		if (strcmpi(val, "false") == 0) 
			strcpy(val,"0");

		float val1 = (float)atof(val);
		int val2   = atoi(val);
		int val3   = val2;

		if (val2 < 0) 
			val2 = 0;

		if (val2 > 1) 
			val2 = 1;
		

		// Modify
		if (strcmpi(arg,"brightness") == 0) {
			if (!global.restore_memory[RESTORE_BRIGHTNESS]) {
				global.restore_memory[RESTORE_BRIGHTNESS] = 1;
				ReadProcessMemory(phandle,(LPVOID)(p1+0x2C), &global.restore_float[FLOAT_BRIGHTNESS], 4, &stBytes);
			}

			WriteProcessMemory(phandle,(LPVOID)(p1+0x2C), &val1, 4, &stBytes);
		}

		/*game doesn't refresh gamma changes
		if (strcmpi(arg,"gamma")==0 || strcmpi(arg,"g")==0)
			WriteProcessMemory(phandle,(LPVOID)(p1+0x758), &val, 4, &stBytes);*/

		if (strcmpi(arg,"visualquality") == 0)
			WriteProcessMemory(phandle,(LPVOID)(p2+0x53C), &val1, 4, &stBytes);

		if (strcmpi(arg,"objectshadows") == 0) {
			if (!global.restore_memory[RESTORE_OBJECT_SHADOWS]) {
				global.restore_memory[RESTORE_OBJECT_SHADOWS] = 1;
				ReadProcessMemory(phandle,(LPVOID)(p3+0x5B0), &global.restore_byte[BYTE_OBJECT_SHADOWS], 1, &stBytes);
			}

			WriteProcessMemory(phandle, (LPVOID)(p3+0x5B0), &val2, 1, &stBytes);
		}

		if (strcmpi(arg,"vehicleshadows") == 0) {
			if (!global.restore_memory[RESTORE_VEHICLE_SHADOWS]) {
				global.restore_memory[RESTORE_VEHICLE_SHADOWS] = 1;
				ReadProcessMemory(phandle,(LPVOID)(p3+0x5B1), &global.restore_byte[BYTE_VEHICLE_SHADOWS], 1, &stBytes);
			}

			WriteProcessMemory(phandle, (LPVOID)(p3+0x5B1), &val2, 1, &stBytes);
		}

		if (strcmpi(arg,"cloudlets") == 0) {
			if (!global.restore_memory[RESTORE_CLOUDLETS]) {
				global.restore_memory[RESTORE_CLOUDLETS] = 1;
				ReadProcessMemory(phandle,(LPVOID)(p3+0x5B2), &global.restore_byte[BYTE_CLOUDLETS], 1, &stBytes);
			}

			WriteProcessMemory(phandle, (LPVOID)(p3+0x5B2), &val2, 1, &stBytes);
		}


		if (strcmpi(arg,"landscapedistance") == 0)
			WriteProcessMemory(phandle, (LPVOID)(!global.CWA ? 0x7DD068 : 0x7CC028), &val1, 4, &stBytes);

		if (strcmpi(arg,"objectsdistance") == 0)
			WriteProcessMemory(phandle, (LPVOID)(!global.CWA ? 0x7DD06C : 0x7CC02C), &val1, 4, &stBytes);

		if (strcmpi(arg,"viewdistance") == 0)
			WriteProcessMemory(phandle, (LPVOID)(!global.CWA ? 0x7DD070 : 0x7CC030), &val1, 4, &stBytes);

		if (strcmpi(arg,"radardistance") == 0) {
			if (!global.restore_memory[RESTORE_RADAR]) {
				global.restore_memory[RESTORE_RADAR] = 1;
				ReadProcessMemory(phandle,(LPVOID)(!global.CWA ? 0x7DD074 : 0x7CC034), &global.restore_float[FLOAT_RADAR], 4, &stBytes);
			}

			WriteProcessMemory(phandle, (LPVOID)(!global.CWA ? 0x7DD074 : 0x7CC034), &val1, 4, &stBytes);
		}

		if (strcmpi(arg,"shadowsdistance") == 0)
			WriteProcessMemory(phandle, (LPVOID)(!global.CWA ? 0x7DD078 : 0x7CC038), &val1, 4, &stBytes);

		if (strcmpi(arg,"maxobjects") == 0) {
			if (!global.restore_memory[RESTORE_MAX_OBJECTS]) {
				global.restore_memory[RESTORE_MAX_OBJECTS] = 1;
				ReadProcessMemory(phandle,(LPVOID)(!global.CWA ? 0x7DD07C : 0x7CC03C), &global.restore_int[INT_MAX_OBJECTS], 4, &stBytes);
			}

			WriteProcessMemory(phandle, (LPVOID)(!global.CWA ? 0x7DD07C : 0x7CC03C), &val3, 4, &stBytes);
		}

		if (strcmpi(arg,"tracktime") == 0) {
			if (!global.restore_memory[RESTORE_TRACK1]) {
				global.restore_memory[RESTORE_TRACK1] = 1;
				ReadProcessMemory(phandle,(LPVOID)(!global.CWA ? 0x7DD080 : 0x7CC040), &global.restore_float[FLOAT_TRACK1], 4, &stBytes);
			}

			WriteProcessMemory(phandle, (LPVOID)(!global.CWA ? 0x7DD080 : 0x7CC040), &val1, 4, &stBytes);
		}

		if (strcmpi(arg,"invtracktime") == 0) {
			if (!global.restore_memory[RESTORE_TRACK2]) {
				global.restore_memory[RESTORE_TRACK2] = 1;
				ReadProcessMemory(phandle,(LPVOID)(!global.CWA ? 0x7DD084 : 0x7CC044), &global.restore_float[FLOAT_TRACK2], 4, &stBytes);
			}

			WriteProcessMemory(phandle, (LPVOID)(!global.CWA ? 0x7DD084 : 0x7CC044), &val1, 4, &stBytes);
		}

		if (strcmpi(arg,"maxlights") == 0) {
			if (!global.restore_memory[RESTORE_MAX_LIGHTS]) {
				global.restore_memory[RESTORE_MAX_LIGHTS] = 1;
				ReadProcessMemory(phandle,(LPVOID)(!global.CWA ? 0x7DD08C : 0x7CC04C), &global.restore_int[INT_MAX_LIGHTS], 4, &stBytes);
			}

			WriteProcessMemory(phandle, (LPVOID)(!global.CWA ? 0x7DD08C : 0x7CC04C), &val3, 4, &stBytes);
		}

		if (strcmpi(arg,"fovLeft") == 0) {
			if (!global.restore_memory[RESTORE_FOVLEFT]) {
				global.restore_memory[RESTORE_FOVLEFT] = 1;
				ReadProcessMemory(phandle,(LPVOID)(LPVOID)(p1+0x40), &global.restore_float[FLOAT_FOVLEFT], 4, &stBytes);
			}

			WriteProcessMemory(phandle, (LPVOID)(p1+0x40), &val1, 4, &stBytes);
		}

		if (strcmpi(arg,"fovTop") == 0) {
			if (!global.restore_memory[RESTORE_FOVTOP]) {
				global.restore_memory[RESTORE_FOVTOP] = 1;
				ReadProcessMemory(phandle,(LPVOID)(LPVOID)(p1+0x40+4), &global.restore_float[FLOAT_FOVTOP], 4, &stBytes);
			}

			WriteProcessMemory(phandle, (LPVOID)(p1+0x40+4), &val1, 4, &stBytes);
		}

		if (strcmpi(arg,"uiTopLeftX") == 0) {
			if (!global.restore_memory[RESTORE_UITOPLEFTX]) {
				global.restore_memory[RESTORE_UITOPLEFTX] = 1;
				ReadProcessMemory(phandle,(LPVOID)(LPVOID)(p1+0x40+8), &global.restore_float[FLOAT_UITOPLEFTX], 4, &stBytes);
			}

			WriteProcessMemory(phandle, (LPVOID)(p1+0x40+8), &val1, 4, &stBytes);
		}

		if (strcmpi(arg,"uiTopLeftY") == 0) {
			if (!global.restore_memory[RESTORE_UITOPLEFTY]) {
				global.restore_memory[RESTORE_UITOPLEFTY] = 1;
				ReadProcessMemory(phandle,(LPVOID)(LPVOID)(p1+0x40+12), &global.restore_float[FLOAT_UITOPLEFTY], 4, &stBytes);
			}

			WriteProcessMemory(phandle, (LPVOID)(p1+0x40+12), &val1, 4, &stBytes);
		}

		if (strcmpi(arg,"uiBottomRightX") == 0) {
			if (!global.restore_memory[RESTORE_UIBOTTOMRIGHTX]) {
				global.restore_memory[RESTORE_UIBOTTOMRIGHTX] = 1;
				ReadProcessMemory(phandle,(LPVOID)(LPVOID)(p1+0x40+16), &global.restore_float[FLOAT_UIBOTTOMRIGHTX], 4, &stBytes);
			}

			WriteProcessMemory(phandle, (LPVOID)(p1+0x40+16), &val1, 4, &stBytes);
		}

		if (strcmpi(arg,"uiBottomRightY") == 0) {
			if (!global.restore_memory[RESTORE_UIBOTTOMRIGHTY]) {
				global.restore_memory[RESTORE_UIBOTTOMRIGHTY] = 1;
				ReadProcessMemory(phandle,(LPVOID)(LPVOID)(p1+0x40+20), &global.restore_float[FLOAT_UIBOTTOMRIGHTY], 4, &stBytes);
			}

			WriteProcessMemory(phandle, (LPVOID)(p1+0x40+20), &val1, 4, &stBytes);
		}
	}
}
break;






case C_MEM_GETJOYSTICK:
{ // Get joystick values from memory

	char tmp[256]	= "";
	bool addComma	= false;
	int i			= 0;
	int but			= 0;
	int pov			= 0;
	int povAngle	= 65535;
	int	base		= !global.CWA ? 0x79E994 : 0x78DA8C;


	// AXES --------------------------------------------------------
	float axisX		= 0;
	float axisY		= 0;
	float axisZ		= 0;
	float axisR1	= 0;
	float axisR2	= 0;
	ReadProcessMemory(phandle, (LPVOID)base,	 &axisX, 4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)(base+4), &axisY, 4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)(base+8), &axisR2,4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)(base+20),&axisZ, 4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)(base+24),&axisR1,4, &stBytes);
	
	if (axisR1 == 0) 
		axisR1 = axisR2;

	sprintf(tmp, "[%.6f,%.6f,%.6f,%.6f,[", axisX,axisY,axisZ,axisR1);
	// -------------------------------------------------------------

	// BUTTONS -----------------------------------------------------
	base = !global.CWA ? 0x79E96C : 0x78DA64;

	for (i=0; i<8; i++)
	{	
		ReadProcessMemory(phandle, (LPVOID)base, &but, 1, &stBytes);

		if (but != 0)
		{
			if (addComma) 
				strcat(tmp,","); 
			else 
				addComma = 1;

			sprintf(tmp, "%s\"JOY%d\"", tmp, i+1);
		};

		base += 4;
	};
	// -------------------------------------------------------------

	// POV CHECK ---------------------------------------------------
	// one of the eight bytes indicates pov status
	base = !global.CWA ? 0x79E95C : 0x78DA4C;

	for (i=0; i<8; i++)
	{
		ReadProcessMemory(phandle,(LPVOID)(base+i),&pov,1,&stBytes);

		if (pov != 0) 
			break;
	};
	// -------------------------------------------------------------

	// POV PRINT ---------------------------------------------------
	if (i!=8  &&  addComma) 
		strcat(tmp,",");

	switch(i)
	{
		case 0 : povAngle=0; strcat(tmp,"\"JOYPOVUP\"]"); break;
		case 1 : povAngle=45; strcat(tmp,"\"JOYPOVUPRIGHT\"]"); break;
		case 2 : povAngle=90; strcat(tmp,"\"JOYPOVRIGHT\"]"); break;
		case 3 : povAngle=135; strcat(tmp,"\"JOYPOVDOWNRIGHT\"]"); break;
		case 4 : povAngle=180; strcat(tmp,"\"JOYPOVDOWN\"]"); break;
		case 5 : povAngle=225; strcat(tmp,"\"JOYPOVDOWNLEFT\"]"); break;
		case 6 : povAngle=270; strcat(tmp,"\"JOYPOVLEFT\"]"); break;
		case 7 : povAngle=315; strcat(tmp,"\"JOYPOVUPLEFT\"]"); break;
		default : strcat(tmp,"]");
	};

	sprintf(tmp, "%s,%d]", tmp, povAngle);
	QWrite(tmp, out);
	// -------------------------------------------------------------
}
break;






case C_MEM_GETSCROLL:	
{ // Get mouse scroll counter value from memory

	// Get list of modules under this game instance
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, global.pid);
	MODULEENTRY32 xModule;
 
	if (hSnap == INVALID_HANDLE_VALUE) 
		break;

	xModule.dwSize = sizeof(MODULEENTRY32);

	if (Module32First(hSnap, &xModule) == 0) 
	{
		CloseHandle(hSnap);
		break;
	} 
 

	// Search for dinput8.dll
	int offset = 0;
	int scroll = 0;

	do 
	{
		if (lstrcmpi(xModule.szModule, (LPCTSTR)"dinput8.dll") == 0)
		{
			// Read module base address
			offset = (int)xModule.modBaseAddr;
			
			// Distance to scroll depends on module size
			if (xModule.modBaseSize == 233472)
				offset += 0x2D848;	// old computers
			else
				offset += 0x2C1C8;	// new computers

			break;
		}
	} 
	while (Module32Next(hSnap, &xModule));

	CloseHandle(hSnap);
	ReadProcessMemory(phandle, (LPVOID)offset, &scroll ,4, &stBytes);	

	char tmp[16] = "";
	sprintf(tmp, "%d", scroll/120);		// one scroll movement changes value by 120
	QWrite(tmp, out);
}
break;














case C_MEM_SETPLAYERANIM:		
{ // Set player animation code in memory

	if (numP < 3) 
	{
		QWrite(":mem setplayeranim ERROR - not enough parameters", out);
		break;
	};

	int pointer[4]	= {0x7B4028, 0, 0, 0};
	int	modif[3]	= {0x788, 0x8, 0x708};

	if (global.CWA) 
		pointer[0]	= 0x7A3128, 
		modif[0]	= 0x78C, 
		modif[2]	= 0x718;

	bool restartPath = false;
	int animcode	 = atoi(par[2]);
	int max_loops	 = (sizeof(pointer) / sizeof(pointer[0])) - 1;

	if (animcode < 0) 
		break;


	// Unlike in getplayeranim we're reading 3 times instead of 4
	// because we don't need value under the last address; just the address
	for (int i=0; i<max_loops; i++)
	{
		ReadProcessMemory(phandle, (LPVOID)pointer[i], &pointer[i+1], 4, &stBytes);

		if (!restartPath  &&  pointer[i+1]==0) 
		{
			i			= -1;
			restartPath = true;

			if (!global.CWA) 
				modif[0] = 0x784; 
			else 
				pointer[0]	= 0x78E9C8,
				modif[0]	= 0x7A8;

			continue;
		};

		pointer[i+1] = pointer[i+1] +  modif[i];
	};


	WriteProcessMemory(phandle, (LPVOID)pointer[max_loops], &animcode, 4, &stBytes);
}
break;






case C_MEM_GETCINEMABORDER:
{ // Get showCinemaBorder value from the memory

	int cin		= 0;
	int offset	= !global.CWA ? 0x76D1D0 : 0x755678;

	ReadProcessMemory(phandle, (LPVOID)offset, &cin, 1, &stBytes);
	QWrite(getBool(cin),out);
};
break;






case C_MEM_GETRESPAWNTYPE:
{ // Get respawn value from the memory

	int pointer		= 0;
	int respawn		= 0;
	int	base		= !global.DedicatedServer ? (!global.CWA ? 0x78337C : 0x77246C) : (!global.CWA ? 0x7031D8 : 0x703228);

	ReadProcessMemory(phandle, (LPVOID)base,			&pointer,    4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)(pointer+0x6C),	&respawn,    1, &stBytes);

	char tmp[4] = "";
	sprintf(tmp, "%d", respawn);
	QWrite(tmp, out);
};
break;






case C_MEM_SETRESPAWNTYPE:
{ // Set respawn value in the memory

	if (numP < 3)
	{
		QWrite(":mem setrespawntype ERROR - not enough parameters", out);
		break;
	}

	int pointer = 0;
	int respawn = atoi(par[2]);
	int base	= !global.CWA ? 0x78337C : 0x77246C;
		/*base = !DedicatedServer ? (!CWA ? 0x78337C : 0x77246C) : (!CWA ? 0x7031D8 : 0x703228);*/
		
	ReadProcessMemory (phandle, (LPVOID)base,			&pointer, 4, &stBytes);
	WriteProcessMemory(phandle, (LPVOID)(pointer+0x6C), &respawn, 1, &stBytes);
};
break;






case C_MEM_GETRESSIDE:
{ // Get resistance friendliness values from the memory

	int west = 0;
	int east = 0;
	int	base = !global.DedicatedServer ? (!global.CWA ? 0x786850 : 0x775938 ) : (!global.CWA ? 0x7066A8 : 0x7066F8);

	ReadProcessMemory(phandle, (LPVOID)base,     &east, 4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)(base+4), &west, 4, &stBytes);

	if (west>0  &&  east==0) 
		QWrite("west", out);
	else
		if (west==0  &&  east>0) 
			QWrite("east",out);
	else
		if (west>0  &&  east>0) 
			QWrite("everybody",out);
	else
		QWrite("nobody", out);
};
break;






case C_MEM_GETDAYLIGHT:
{ // Get brightness related values from the memory

	float daytime	= 0;
	float daybright = 0;
	int pointer		= 0;
	int pointer2	= 0;
	int	base		= !global.DedicatedServer ? (!global.CWA ? 0x7B4028 : 0x7A3128 ) : (!global.CWA ? 0x733E88 : 0x733F20);

	ReadProcessMemory(phandle, (LPVOID)base,           &pointer,  4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)(pointer+0xB0), &pointer2, 4, &stBytes);

	ReadProcessMemory(phandle, (LPVOID)(pointer2+0x64), &daytime,  4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)(pointer2+0x8C), &daybright,4, &stBytes);

	char tmp[32] = "";
	sprintf(tmp, "[%.6f,%.6f]", daytime,daybright);
	QWrite(tmp, out);
	
	// First value is G from RGB ground lighting colour but it can be used to 
	// check time of day
	
	// Second value is presumably brightness multiplier but it's read-only
};
break;






case C_MEM_GETDATE:
{ // Get mission date from the memory

	int day		= 0;
	int dayweek = 0;
	int dayyear = 0;
	int month	= 0;
	int year	= 0;
	int	off[] = 
	{
		!global.CWA ? 0x780608 : 0x76F7C8,
		!global.CWA ? 0x7DD3FC : 0x7CC3BC, 
		!global.CWA ? 0x7DD400 : 0x7CC3C0, 
		!global.CWA ? 0x7DD408 : 0x7CC3C8, 
		!global.CWA ? 0x7DD40C : 0x7CC3CC
	};

	ReadProcessMemory(phandle, (LPVOID)off[0], &year,    4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)off[1], &day,     4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)off[2], &month,   4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)off[3], &dayweek, 4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)off[4], &dayyear, 4, &stBytes);

	char tmp[64] = "";
	sprintf(tmp, "[%d,%d,%d,%d,%d]", 1900+year, month+1, day, dayweek, dayyear+1);
	QWrite(tmp, out);	
};
break;






case C_MEM_SETPLAYERVIEW:
{ // Change camera view type in the memory

	if (numP < 3) 
	{
		QWrite(":mem setplayerview ERROR - not enough parameters", out);
		break;
	}

	par[2] = stripq(par[2]);

	int pointer = 0;
	int mode	= -1;
	int	base	= !global.CWA ? 0x7B4028 : 0x7A3128;


	// Convert string to number
	if (strcmpi(par[2],"INTERNAL") == 0) 
		mode = 0; 
	else
		if (strcmpi(par[2],"GUNNER") == 0) 
			mode = 1; 
	else
		if (strcmpi(par[2],"EXTERNAL") == 0) 
			mode = 2; 
	else
		if (strcmpi(par[2],"GROUP") == 0) 
			mode = 3; 
	else 
		break;


	ReadProcessMemory (phandle, (LPVOID)base,			 &pointer, 4, &stBytes);
	WriteProcessMemory(phandle, (LPVOID)(pointer+0x860), &mode,    1, &stBytes);
};
break;






case C_MEM_ERROR:
{ // Get error message from the memory
	
// [[[0x789D88] + 0x68] + 0x1C] + 0x00	OFP
// [[[0x778E80] + 0x68] + 0x1C] + 0x00	CWA
	
	// Base address and offsets
	int pointer[4]	= {0, 0, 0, 0};
	int modif[3]	= {0x68, 0x1C, 0};
	int max_loops	= sizeof(pointer) / sizeof(pointer[0]) - 1;

	switch(game_version) {
		case VER_196 : pointer[0]=0x789D88; break;
		case VER_199 : pointer[0]=0x778E80; break;
		case VER_201 : pointer[0]=global.exe_address+0x6D6A10; break;
	}

	if (pointer[0] == 0)
		break;

	// Read values
	for (int i=0; i<max_loops; i++) {
		ReadProcessMemory(phandle, (LPVOID)pointer[i], &pointer[i+1], 4, &stBytes);
		pointer[i+1] = pointer[i+1] +  modif[i];
	};

	// if went nowhere
	if (pointer[max_loops] == 0) 
		break;

	char errorMSG[512] = "";
	ReadProcessMemory(phandle, (LPVOID)pointer[max_loops], &errorMSG, 512, &stBytes);

	// Default mode - just return the text
	if (numP == 2)
		QWrite(errorMSG, out);
	// Optional mode - copy text to clipboard
	else 
		if (numP > 2)
			if (strcmpi(par[2],"clip") == 0)
				if (CopyToClip(errorMSG, false, CommandID, out))
					FWerror(0,0,CommandID,"","",0,0,out);
};
break;






case C_MEM_SETPLAYERAIM:
{ // Set player's mouse target and gun direction and angle

	if (numP < 3)
	{
		QWrite(":mem setplayeraim ERROR - not enough parameters", out);
		break;
	}


	// Find where values are stored
	int pointer[4]	= {0x7B3ACC,0,0,0};
	int modif[3]	= {0x38, 0x8, 0x7C};
	int pointer2[4] = {0x7B4028,0,0,0};
	int modif2[3]	= {0x784, 0x8, 0x474};
	int max_loops	= sizeof(pointer) / sizeof(pointer[0]) - 1;

	if (global.CWA) 
		pointer[0]	= 0x78E9C8,
		pointer2[0] = 0x7A3128,
		modif[0]	= 0x8,
		modif[1]	= 0x7C,
		modif2[2]	= 0x484;


	for (int i=0; i<max_loops; i++)
	{
		// There's one less loop in CWA version
		if (!global.CWA  ||  global.CWA  &&  i<max_loops-1)
		{
			ReadProcessMemory(phandle, (LPVOID)pointer[i], &pointer[i+1], 4, &stBytes);
			pointer[i+1] = pointer[i+1] +  modif[i];
		};		
		
		ReadProcessMemory(phandle, (LPVOID)pointer2[i], &pointer2[i+1], 4, &stBytes);
		pointer2[i+1] = pointer2[i+1] +  modif2[i];
	};


	// Parse input
	i				= 0;
	int j			= !global.CWA ? max_loops : max_loops-1;
	char *arguments = com+17;
	char *pch		= strtok (arguments, "[,]\" ");

	while (pch != NULL)
	{	
		if (strcmp(pch,"-") == 0) 
		{
			i++; 
			pch = strtok (NULL, "[,]\" "); 
			continue;
		}

		double num = atof(pch);
		float fnum = (float)num;

		switch(i) 
		{
			// Mouse target direction
			case 0:
			{
				float m_sin = (float)sin(deg2rad(num));
				float m_cos = (float)cos(deg2rad(num));

				WriteProcessMemory(phandle, (LPVOID)pointer[j],     &m_sin,    4, &stBytes);
				WriteProcessMemory(phandle, (LPVOID)(pointer[j]+8), &m_cos,    4, &stBytes);
			}
			break;

			// Gun offset
			case 1:
			{
				num  = sin(deg2rad(num));
				fnum = float(num*-1);

				WriteProcessMemory(phandle, (LPVOID)pointer2[max_loops],	 &fnum, 4, &stBytes);
				WriteProcessMemory(phandle, (LPVOID)(pointer2[max_loops]+4), &fnum, 4, &stBytes);
			}
			break;

			// Mouse target pitch
			case 2: WriteProcessMemory(phandle, (LPVOID)(pointer[j]+4), &fnum, 4, &stBytes); break;

			// Gun pitch
			case 3: WriteProcessMemory(phandle, (LPVOID)(pointer2[max_loops]+8),  &fnum, 4, &stBytes); break;

			// Gun velocity horiz
			case 4: WriteProcessMemory(phandle, (LPVOID)(pointer2[max_loops]+20), &fnum, 4, &stBytes); break;

			// Gun velocity vert
			case 5: WriteProcessMemory(phandle, (LPVOID)(pointer2[max_loops]+16), &fnum, 4, &stBytes); break;

			// Gun pitch2
			case 6: WriteProcessMemory(phandle, (LPVOID)(pointer2[max_loops]+12), &fnum, 4, &stBytes); break;
		};

		i++;
		pch = strtok (NULL, "[,]\" ");
	};
}
break;






case C_MEM_MODLIST:
{ // Get list of modfolders that user selected

//[[ifc22.dll + 0x2C154] + 0x0] + 0x0

	// Get list of dll's
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, global.pid);

 	if (hSnap == INVALID_HANDLE_VALUE) {
		QWrite("[]", out); 
		break;
	}
	
	MODULEENTRY32 xModule;
	memset (&xModule, 0, sizeof(xModule));
	xModule.dwSize = sizeof(xModule);

	if (Module32First(hSnap, &xModule) == 0) {
		CloseHandle(hSnap);
		QWrite("[]", out);
		break;
	};

	xModule.dwSize = sizeof(MODULEENTRY32);
 

	// Game arguments are stored in IFC22.dll - find it's address
	int baseOffset = 0;

	do {
		if (lstrcmpi(xModule.szModule, (LPCTSTR)"ifc22.dll") == 0) {
			// Read module base address
			baseOffset = (int)xModule.modBaseAddr + 0x2C154;
			break;
		}
	} 
	while (Module32Next(hSnap, &xModule));

	CloseHandle(hSnap);


    // Find offset holding arguments, read 512 bytes
	char parameters[512] = "";
	ReadProcessMemory(phandle, (LPVOID)baseOffset, &baseOffset, 4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)baseOffset, &baseOffset, 4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)baseOffset, &parameters, 512, &stBytes);


	// Replace \0 with spaces (untokenize) until we hit \0\0
	int emptyChars = 0;

	for (int i=0; i<512; i++)
		if (parameters[i] == '\0')
        {
			emptyChars++;

			if (emptyChars == 2) 
				break;

			parameters[i] = ' ';
        }
		else 
			emptyChars = 0;


	// Find all selected modfolder names in the command line
	QWrite("[", out);
	char *txt = parameters;
	char *pch = "";

	// For each parameter
	while (pch = strchr(txt,' '))
	{
		// Separate it from the rest
		int pos  = pch - txt;
		txt[pos] = '\0';

		// If it has equality sign
		char *foo = strchr(txt, '=');

		if (foo)				
		{
			// If it's "-mod=" parameter
			if (strncmpi(txt,"-mod=",5) == 0)
			{
				// Tokenize on semi-colon
				char *mod = strtok(txt+5, ";");

				while (mod != NULL)
				{
					QWrite("]+[\"", out); 
					QWrite(mod, out); 
					QWrite("\"", out);
					mod = strtok(NULL, ";");
				};
			};
		};

		// Move on to the next
		txt += pos+1;
	};

	QWrite("]", out);
}
break;






case C_MEM_GETDIFFICULTY:
case C_MEM_SETDIFFICULTY:
{ // Get difficulty options

/*
	OFP
[[0x78337C] + 0x60]	client
[[0x783378] + 0x60]	player server
0x007DD0C8			sp cadet
0x007DD0D4			sp veteran
0x0075A380			server cadet
0x0075A38C			server veteran
	CWA
[[0x77246C] + 0x60] + 0x8	client
[[0x772468] + 0x60] + 0x8	player server
0x007CC088					sp cadet
0x007CC094					sp veteran
0x0075A410					server cadet
0x0075A41C					server veteran
*/

	// Incorrect number of parameters
	if (CommandID==C_MEM_GETDIFFICULTY  &&  numP<3) {
		QWrite("[]", out); 
		break;
	}

	if (CommandID==C_MEM_SETDIFFICULTY  &&  numP<4) {
		QWrite("false", out); 
		break;
	}


	bool SinglePlayer = strcmpi(par[2],"sp")==0 || strcmpi(par[2],"false")==0;
	bool MultiPlayer  = strcmpi(par[2],"mp")==0 || strcmpi(par[2],"true")==0;
	bool veteran      = false;
	int base          = 0;
	int pointer       = 0;
	int offset        = 0;
	int	offsets[][4]  = 
	{
		{0x7DD0C8, 0x7DD0D4, 0x75A380, 0x75A38C},	//ofp
		{0x7CC088, 0x7CC094, 0x75A410, 0x75A41C}	//cwa
	};


	// Select address from the array based on the arguments
	if (SinglePlayer  ||  global.DedicatedServer) {
		if (CommandID==C_MEM_GETDIFFICULTY  &&  numP<4) {		// not enough params
			QWrite("[]", out); 
			break;
		}

		int i = !global.CWA ? 0 : 1;	// which game
		int j = 0;				// which difficulty

		if (strcmpi(par[3],"veteran")==0  ||  strcmpi(par[3],"false")==0) 
			veteran = true,
			j++;

		if (global.DedicatedServer) 
			j += 2;

		offset = offsets[i][j];
	} else 
		if (MultiPlayer) {
			// player server
			base = !global.CWA ? 0x783378 : 0x772468;
			ReadProcessMemory(phandle, (LPVOID)base,           &pointer, 4, &stBytes);
			ReadProcessMemory(phandle, (LPVOID)(pointer+0x60), &offset,  4, &stBytes);

			// player client
			if (offset == 0) {
				base = !global.CWA ? 0x78337C : 0x77246C;
				ReadProcessMemory(phandle, (LPVOID)base,			&pointer, 4, &stBytes);
				ReadProcessMemory(phandle, (LPVOID)(pointer+0x60),	&offset,  4, &stBytes);
			}

			if (offset==0) {
				if (CommandID == C_MEM_GETDIFFICULTY)
					QWrite("[]",out); 
				else
					QWrite("false",out);
				break;
			}
		} else {
			if (CommandID == C_MEM_GETDIFFICULTY)
				QWrite("[]",out); 
			else
				QWrite("false",out);
			break;
		}


	// Now read memory
	if (CommandID == C_MEM_GETDIFFICULTY) {
		// Read twelve bytes one by one
		int setting = 0;
		QWrite("[", out);

		for (int i=0; i<12; i++) {
			ReadProcessMemory(phandle, (LPVOID)(offset+i), &setting,  1, &stBytes);

			if (i > 0) 
				QWrite(",",out);

			QWrite(getBool(setting), out);
		}

		QWrite("]", out);
	} else {
		// Change twelve bytes one by one
		int i     = 0;
		char *pch = strtok(par[4], "[,]\" ");

		while (pch!=NULL  &&  i<12) {
			int setting = -1;

			if (strcmpi(pch,"false")==0  ||  strcmpi(pch,"0")==0) 
				setting = 0;
			
			if (strcmpi(pch,"true")==0  ||  strcmpi(pch,"1")==0) 
				setting = 1;

			if (setting >= 0) {
				int pos = veteran ? RESTORE_VETERAN : RESTORE_CADET;
				int pos2= veteran ? BYTE_VETERAN    : BYTE_CADET;

				if ((SinglePlayer || global.DedicatedServer)  &&  !global.restore_memory[pos+i]) {	
					global.restore_memory[pos+i] = 1;
					ReadProcessMemory(phandle,(LPVOID)(offset+i), &global.restore_byte[pos2+i], 1, &stBytes);
				}

				WriteProcessMemory(phandle, (LPVOID)(offset+i), &setting,  1, &stBytes);
			}

			pch = strtok(NULL, "[,]\" ");
			i++;
		}
	}
}
break;






case C_MEM_SETRADIOBOX:
{ // Show hide radio options

	if (numP < 3) 
		break;

	int base		= !global.CWA ? 0x79F8D0 : 0x78E9C8;
	int	pointer		= 0;
	int pointer2	= 0;
	int value		= 0;

	if (strcmp(par[2],"true")==0  ||  strcmp(par[2],"1")==0) 
		value = 1;
	
	ReadProcessMemory(phandle, (LPVOID)base,			&pointer,  4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)(pointer+8),		&pointer2, 4, &stBytes);
	WriteProcessMemory(phandle,(LPVOID)(pointer2+0x2D4),&value,	   1, &stBytes);
}
break;






case C_MEM_GETSPEEDKEY:
{ // Get forward/backward movement

	// Optional mode activated by argument
	bool rawMode = false;

	if (numP > 2)
		if (strcmpi(par[2],"raw") == 0) 
			rawMode = true;
	

	// In order: fast (E), forward (W), slow (Q), reverse (S)
	int offset[]	= {0x0, 0x8, 0x4, 0xC};
	int	weight[]	= {3,2,1,-2};
	int	max_loops	= sizeof(offset)/sizeof(offset[0]);
	int	speed		= 0;
	int quantity	= 0;
	int current		= 0;
	int	base		= !global.CWA ? 0x79E9C2 : 0x78DABA;
		

	// Read those four values
	for (int i=0; i<max_loops; i++)
	{
		ReadProcessMemory(phandle, (LPVOID)(base+offset[i]), &current, 2, &stBytes);

		switch(current)
		{
			// Assign multiplier based on value
			case 16256 : quantity=1; break;
			case 16384 : quantity=2; break;
			case 16448 : quantity=3; break;
			case 16512 : quantity=4; break;
			case 16544 : quantity=5; break;
			case 16576 : quantity=6; break;

			default : 
				if (current > 16576) 
					quantity = 7; 
				else 
					quantity = 0;
		};

		speed += weight[i] * quantity;		// Sum up

		// if optional mode then save values that we've read to array
		offset[i] = current;
	};


	// For optional mode return array
	if (rawMode) 
		QWrite("[\"", out);


	// Convert number to string
	if (speed >= 3) 
		QWrite("fast",out); 
	else
		if (speed == 2) 
			QWrite("forward", out); 
	else
		if (speed == 1) 
			QWrite("slow", out); 
	else
		if (speed == 0) 
			QWrite("stop", out); 
	else
		if (speed < 0) 
			QWrite("reverse",out);


	// For optional mode print out values (that we've saved) in array
	if (rawMode)
	{
		QWrite("\"", out);

		for (i=0; i<max_loops; i++)
		{
			char tmp[32] = "";
			sprintf(tmp, ",%d", offset[i]);
			QWrite(tmp, out);
		};

		QWrite("]", out);
	}
}
break;






case C_MEM_SETSPEEDKEY:
{ // Set forward/backward movement

	if (numP < 3) 
		break;

	int offset[]	= {0x0, 0x8, 0x4, 0xC};
	int	max_loops	= sizeof(offset)/sizeof(offset[0]);
	int	base		= !global.CWA ? 0x79E9C2 : 0x78DABA;
	int	val			= 16256;
	int	i			= -1;
	
	
	// String to number
	if (strcmpi(par[2],"fast") == 0) 
		i = 0;
	else 
		if (strcmpi(par[2],"slow") == 0) 
			i = 2;
	else 
		if (strcmpi(par[2],"forward") == 0) 
			i = 1;
	else 
		if (strcmpi(par[2],"reverse") == 0) 
			i = 3;


	// For a particular speed change one of the offsets
	if (i >= 0)
		WriteProcessMemory(phandle, (LPVOID)(base+offset[i]), &val, 2, &stBytes);


	// For "stop" set all of them to zero
	if (strcmpi(par[2],"stop") == 0)
		for (i=0; i<max_loops; i++)
			val = 0,
			WriteProcessMemory(phandle, (LPVOID)(base+offset[i]), &val, 2, &stBytes);
}
break;






case C_MEM_GETPLAYERHATCH:
{ // Is player turned in/out

/*
	OFP
[[[0x7B4028] + 0x784] +0x8] +0x4CE	commander
[[[0x7B4028] + 0x784] +0x8] +0x4D2	driver
[[[0x7B4028] + 0x784] +0x8] +0x4D6	gunner
	CWA
[[[0x7A3128] + 0x784] +0x8] +0x4DE	commander
[[[0x7A3128] + 0x784] +0x8] +0x4E2	driver
[[[0x7A3128] + 0x784] +0x8] +0x4E6	gunner
*/

	int pointer[4]	= {0x7B4028,0,0,0};
	int	modif[3]	= {0x784, 0x8, 0x4CE};
	int	max_loops	= sizeof(pointer) / sizeof(pointer[0]) - 1;

	if (global.CWA) 
		pointer[0]	= 0x7A3128, 
		modif[2]	+= 0x10;

	short int values[3] = {0,0,0};
	char tmp[32]		= "";


	// Path to the offset
	for (int i=0; i<max_loops; i++)
	{
		ReadProcessMemory(phandle, (LPVOID)pointer[i], &pointer[i+1], 4, &stBytes);
		pointer[i+1] = pointer[i+1] +  modif[i];
	};


	// Read seats
	for (int j=0; j<3; j++)
	{
		ReadProcessMemory(phandle, (LPVOID)(pointer[i]+4*j), &values[j], 2, &stBytes);

		if (values[j]!=0  &&  values[j]!=16256) 
			values[j] = 0;
		else 
			if (values[j] == 16256) 
				values[j] = 1;
		else 
			if (values[j]==0) 
				values[j] = 2;
	};


	// Parameter specified - return specific seat
	if (numP >= 3)
	{
		if (strcmpi(par[2],"commander") == 0) 
			j = 0;

		if (strcmpi(par[2],"driver") == 0)
			j = 1;

		if (strcmpi(par[2],"gunner")==0) 
			j = 2;

		sprintf(tmp, "%d", values[j]);
	}
	// No parameters - return array with all seats
	else
		sprintf(tmp, "[%d,%d,%d]", values[1],values[2],values[0]);

	QWrite(tmp, out);
}
break;






case C_MEM_SETPLAYERHATCH:
{ // Turn in / out player

	if (numP < 3) 
		break;

	int pointer[4]	= {0x7B4028, 0, 0, 0};
	int modif[3]	= {0x784, 0x8, 0x0};
	int max_loops	= sizeof(pointer) / sizeof(pointer[0]) - 1;


	if (strcmpi(par[2],"driver") == 0) 
		modif[2] = 0x4D2;

	if (strcmpi(par[2],"gunner") == 0) 
		modif[2] = 0x4D6;

	if (strcmpi(par[2],"commander") == 0) 
		modif[2] = 0x4CE;

	if (modif[2] == 0) 
		break;

	if (global.CWA) 
		pointer[0]	= 0x7A3128, 
		modif[2]	+= 0x10;


	for (int i=0; i<max_loops; i++)
	{
		ReadProcessMemory(phandle, (LPVOID)pointer[i], &pointer[i+1], 4, &stBytes);
		pointer[i+1] = pointer[i+1] +  modif[i];
	};


	short int value = -1;
	ReadProcessMemory(phandle, (LPVOID)pointer[i], &value, 2, &stBytes);

	if (value!=0  &&  value!=16256) // if player isn't in a vehicle
		break;	


	// If passed an extra argument - change to specific value
	if (numP > 3) 
		if (strcmpi(par[3],"true") == 0) 
			value = 0; 
		else 
			value = 16256;
	// Default - just reverse
	else
		if (value == 0) 
			value = 16256; 
		else 
			value = 0;


	WriteProcessMemory(phandle, (LPVOID)pointer[i], &value, 2, &stBytes);
}
break;






case C_MEM_GETPLAYERLADDER:
{ // Player's position on ladder and it's ID

/*
	OFP
[[[0x786CA0] + 0x0] + 0x8] + 0x740	ladder ID
[[[0x786CA0] + 0x0] + 0x8] + 0x744	ladder pos
	CWA
[[[0x775D88] + 0x0] + 0x8] + 0x750
[[[0x775D88] + 0x0] + 0x8] + 0x754
*/

	int pointer[4]	= {0x786CA0, 0, 0, 0};
	int	modif[3]	= {0x0, 0x8, 0x740};
	int	max_loops	= sizeof(pointer) / sizeof(pointer[0]) - 1;
	int	id			= -1;
	float pos		= 0;

	if (global.CWA) 
		pointer[0]	= 0x775D88, 
		modif[2]	= 0x750;


	for (int i=0; i<max_loops; i++)
	{
		ReadProcessMemory(phandle, (LPVOID)pointer[i], &pointer[i+1], 4, &stBytes);
		pointer[i+1] = pointer[i+1] +  modif[i];		
	};

	ReadProcessMemory(phandle, (LPVOID)pointer[max_loops], &id, 4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)(pointer[max_loops]+4), &pos, 4, &stBytes);


	char tmp[32] = "";
	sprintf(tmp, "[%.6f,%d]", pos,id);
	QWrite(tmp, out);
}
break;






case C_MEM_SETPLAYERLADDER:
{ // Change ladder and/or position on it

	if (numP < 3)
	{
		QWrite("ERROR: Not enough parameters", out); 
		break;
	};


	int pointer[4]	= {0x786CA0,0,0,0};
	int	modif[3]	= {0x0, 0x8, 0x740};
	int	max_loops	= sizeof(pointer) / sizeof(pointer[0]) - 1;

	if (global.CWA) 
		pointer[0]	= 0x775D88, 
		modif[2]	= 0x750;


	for (int i=0; i<max_loops; i++)
	{
		ReadProcessMemory(phandle, (LPVOID)pointer[i], &pointer[i+1], 4, &stBytes);
		pointer[i+1] = pointer[i+1] +  modif[i];
	};


	// Change position on the ladder
	float pos = (float)atof(par[2]);

	if (strcmp(par[2],"-") != 0)
		WriteProcessMemory(phandle, (LPVOID)(pointer[max_loops]+4), &pos, 4, &stBytes);


	// Change ladder
	if (numP>3)
	{
		if (strcmp(par[3],"-") == 0) 
			break;

		int currID	= -1;
		int ID		= atoi(par[3]);


		// Only if player is already on a ladder
		ReadProcessMemory(phandle, (LPVOID)pointer[max_loops], &currID, 4, &stBytes);

		if (currID>=0  &&  ID>=0) 
			WriteProcessMemory(phandle, (LPVOID)pointer[max_loops], &ID, 4, &stBytes);
	};
}
break;






case C_MEM_MASTERSERVER:
{ // Get or set address for the browser

	if (numP < 3) {
		QWrite("ERROR: Not enough parameters", out); 
		break;
	};

	char serv1[64]   = "";
	char serv2[19]   = "";
	int master_base1 = 0;
	int	master_base2 = 0;
	int	inMenuOff    = 0;
	int inMenu       = 0;

	switch(game_version) {
		case VER_196 : master_base1=0x76EBC0; master_base2=0x775F58; inMenuOff=0x75E254; break;
		case VER_199 : master_base1=0x756530; master_base2=0x75D7F0; inMenuOff=0x74B58C; break;
		case VER_201 : master_base1=global.exe_address+0x6C9298; master_base2=global.exe_address+0x6C9560; break;
	}

	if (master_base1 == 0) {
		QWrite("false", out);
		break;
	}

	// Optional mode - read instead of writing
	if (numP==3  &&  strcmpi(par[2],"get")==0) {
		ReadProcessMemory(phandle, (LPVOID)master_base1, &serv1, 64, &stBytes);
		ReadProcessMemory(phandle, (LPVOID)master_base2, &serv2, 19, &stBytes);
		
		char tmp[128] = "";
		sprintf(tmp, "[\"%s\",\"%s\"]", serv1, serv2);
		QWrite(tmp, out);
		break;
	}

	// Is it main menu
	if (inMenu != 0) {
		ReadProcessMemory(phandle, (LPVOID)inMenuOff, &inMenu, 4, &stBytes);

		if (!inMenu) {
			QWrite("false",out); 
			break;
		}
	}

	// Change addresses
	par[3] = stripq(par[3]);
	strncpy(serv1, par[3], 63);

	if (numP > 4) 
		par[4] = stripq(par[4]),
		strncpy(serv2, par[4], 18);

	if (strcmp(serv1,"") != 0)
		WriteProcessMemory(phandle, (LPVOID)master_base1, &serv1, 64, &stBytes);

	if (strcmp(serv2,"") != 0)
		WriteProcessMemory(phandle, (LPVOID)master_base2, &serv2, 19, &stBytes);	

	QWrite("true",out);
}
break;






case C_MEM_MISSIONINFO:
{ // Get mission name

	bool addComma		= false;
	char nameSTR[256]	= "";
	char *namePTR		= nameSTR;

	//0x7DD0E0				mission name
	//0x7DD130				world name
	//[0x7DD180] + 0x8		pbo name
	//[0x78324C] + 0x114	briefing name 1
	//[0x786880] + 0x8		briefing name 2
	//[0x786884] + 0x8		briefing desc
	//[0x786C30] + 0x8		campaign name

	int offset[]=
	{
		!global.DedicatedServer ? (!global.CWA ? 0x7DD0E0 : 0x7CC0A0) : (!global.CWA ? 0x75A398 : 0x75A428),
		!global.DedicatedServer ? (!global.CWA ? 0x7DD130 : 0x7CC0F0) : (!global.CWA ? 0x75A3E8 : 0x75A478),
		!global.DedicatedServer ? (!global.CWA ? 0x7DD180 : 0x7CC140) : (!global.CWA ? 0x75A438 : 0x75A4C8),
		!global.DedicatedServer ? (!global.CWA ? 0x78324C : 0x77233C) : (!global.CWA ? 0x7030AC : 0x7030FC),
		!global.DedicatedServer ? (!global.CWA ? 0x786880 : 0x775968) : (!global.CWA ? 0x7066D8 : 0x706728),
		!global.DedicatedServer ? (!global.CWA ? 0x786884 : 0x77596C) : (!global.CWA ? 0x7066DC : 0x70672C),
		!global.CWA ? 0x786C30 : 0x775D18
	};

	int	modif[]=
	{
		0,
		0,
		0x8,
		0x114,
		0x8,
		0x8,
		0x8
	};

	int	temp		= 0;
	int max_loops	= sizeof(offset) / sizeof(offset[0]);

	QWrite("[",out);


	// Read from all the addresses
	for (int i=0; i<max_loops; i++)
	{
		strcpy(nameSTR, "");
		namePTR = nameSTR;
		
		// First two - just read string
		if (i <= 1)
			ReadProcessMemory(phandle, (LPVOID)offset[i], &nameSTR, 255, &stBytes);
		else
			// Otherwise read pointer
			if (!global.DedicatedServer  || global.DedicatedServer && i<max_loops-1)
			{
				ReadProcessMemory(phandle, (LPVOID)offset[i], &temp, 4, &stBytes);	

				if (temp != 0) 
					ReadProcessMemory(phandle, (LPVOID)(temp+modif[i]), &nameSTR, 255, &stBytes);

				// if briefing 1 is available then skip briefing 2 (next value)
				if (i == 3)
					if (temp == 0)
						continue;
					else
						i++;
			};


		if (addComma)
			QWrite(",", out);
		else
			addComma = true;


		// If a name from the stringtable
		if (nameSTR[0]=='@'  ||  strncmpi(nameSTR,"$STR",4)==0)
			namePTR = nameSTR+1,
			QWrite("localize ", out);


		QWrite("\"", out);
		PrintDoubleQ(namePTR, out);
		QWrite("\"", out);
	};

	QWrite("]",out);
}
break;






case C_MEM_BULLETS:
{ // Get/Set bullets properties

	DWORD old			= 0;
	bool addComma		= false;
	bool set[]			= {0,0,0,0,0,0,0,0,0,0};
	float value[]		= {0,0,0,0,0,0,0,0,0,0};
	char tmp[256]		= "[";
	char argument[][14] =
	{
		"gravacc",
		"bullet",
		"shell",
		"rocket",
		"bomb",
		"smoke",
		"flare",
		"flareduration",
		"pipebomb",
		"timebomb"
	};
	int offset[] =
	{
		!global.DedicatedServer ? (!global.CWA ? 0x71D518 : 0x710520) : (!global.CWA ? 0x6ABDE8 : 0x6ABDA8),	//gravity acceleration	9.8065996170043945
		!global.DedicatedServer ? (!global.CWA ? 0x5F1570 : 0x5F818C) : (!global.CWA ? 0x5374B8 : 0x537671),	//bullet lifetime		3
		!global.DedicatedServer ? (!global.CWA ? 0x5F16D2 : 0x5F7ACB) : (!global.CWA ? 0x534F9C : 0x53517D),	//shell  lifetime		20
		!global.DedicatedServer ? (!global.CWA ? 0x5F1527 : 0x4857B3) : (!global.CWA ? 0x533A91 : 0x533C4A),	//rocket lifetime		10
		!global.DedicatedServer ? (!global.CWA ? 0x5F147B : 0x487867) : (!global.CWA ? 0x5357C7 : 0x5359A8),	//bomb lifetime			120
		!global.DedicatedServer ? (!global.CWA ? 0x5F178F : 0x487986) : (!global.CWA ? 0x5371A1 : 0x53735A),	//smoke lifetime		60
		!global.DedicatedServer ? (!global.CWA ? 0x5F12AD : 0x5F7D5D) : (!global.CWA ? 0x536D09 : 0x536EEA),	//flare lifetime		17
		!global.DedicatedServer ? (!global.CWA ? 0x7137A0 : 0x7067A0) : (!global.CWA ? 0x6A66C0 : 0x6A66C0),	//flare duration		15
		!global.DedicatedServer ? (!global.CWA ? 0x5F14BF : 0x485820) : (!global.CWA ? 0x5345F9 : 0x5347DA),	//pipebomb lifetime		3.402823466E38 (7F7FFFFF)
		!global.DedicatedServer ? (!global.CWA ? 0x5F1818 : 0x5F789B) : (!global.CWA ? 0x5347F7 : 0x5349D8)	//timebomb lifetime		20
	};
	int max_loops = (sizeof(offset) / sizeof(offset[0]));


	// Parse input
	for (int i=2; i<numP; i++)
	{
		char *arg = stripq(par[i]);
		char *pch = strchr(arg, ':');

		if (pch == NULL) 
			continue;

		int pos		= pch - arg;
		arg[pos]	= '\0';
		char *val	= Trim(arg+pos+1);
		float num	= (float) atof (val);

		// If name matches then queue for change
		for (int j=0; j<max_loops; j++)
			if (strcmpi(arg,argument[j]) == 0)
				set[j] = 1,
				value[j] = num;
	};


	// Write / Read values
	for (i=0; i<max_loops; i++)
	{
		if ((i==0 || i==7)  &&  set[i])
			VirtualProtectEx(phandle, (LPVOID)offset[i], 256, PAGE_EXECUTE_READWRITE, &old);
		
		if (set[i])
		{
			if (!global.restore_memory[RESTORE_BULLETS+i])
			{
				global.restore_memory[RESTORE_BULLETS+i] = 1;
				ReadProcessMemory(phandle,(LPVOID)offset[i], &global.restore_float[FLOAT_BULLETS+i], 4, &stBytes);
			};
			
			WriteProcessMemory(phandle, (LPVOID)offset[i], &value[i], 4, &stBytes);
		};

		int result = ReadProcessMemory(phandle, (LPVOID)offset[i], &value[i], 4, &stBytes);

		if (addComma) 
			strcat(tmp,","); 
		else 
			addComma = true;

		sprintf(tmp,"%s%f",tmp,value[i]);
	};

	strcat(tmp,"]");
	QWrite(tmp, out);
}
break;








case C_MEM_SETWEATHER:
{ // Change weather values in the memory

	int valueINT[3];
	float valueFLT[24];
	float windSpeed[3];
	float gust[3];

	bool set[24];
	char argument[][20] =
	{
		"actualOvercast",
		"wantedOvercast",
		"speedOvercast",
		"actualFog",
		"wantedFog",
		"speedFog",
		"weatherTime",
		"nextWeatherChange",
		"cloudsPos",
		"cloudsAlpha",
		"cloudsBrightness",
		"cloudsSpeed",
		"skyThrough",
		"rainDensity",
		"rainDensityWanted",
		"rainDensitySpeed",
		"thunderBoltTime",		//16 - int
		"windSpeed",			//17 - float array
		"lastWindSpeedChange",	//18 - int
		"gust",					//19 - float array
		"gustUntil",			//20 - int
		"seaWaveSpeed",
		"maxTide",
		"maxWave"
	};
	int max_loops = sizeof(set) / sizeof(set[0]);

	memset(set, 0, sizeof(set));

	// Parse input
	for (int i=2; i<numP; i++)
	{
		char *arg = stripq(par[i]);
		char *pch = strchr(arg, ':');

		if (pch == NULL) 
			continue;

		int pos		= pch - arg;
		arg[pos]	= '\0';
		char *val	= Trim(arg+pos+1);

		// If name matches then queue for change
		for (int j=0; j<max_loops; j++)
		{
			if (strcmpi(arg,argument[j]) == 0)
			{
				set[j] = 1;

				// floats
				if (j<16  ||  j>20)
					valueFLT[j] = (float)atof(val);
				
				// integers
				if (j == 16)
					valueINT[0] = atoi(val);

				if (j == 18)
					valueINT[1] = atoi(val);

				if (j == 20)
					valueINT[2] = atoi(val);

				// float array
				if (j==17  ||  j==19)
				{
					int index	 = 0;
					int max		 = strlen(val);
					char tmp[64] = "";

					for (int s=0; s<max; s++)
					{
						if (val[s] == '[')
							continue;						

						if (val[s]==','  ||  val[s]==']')
						{
							if (j==17)
								windSpeed[index] = (float)atof(tmp);

							if (j==19)
								gust[index] = (float)atof(tmp);

							strcpy(tmp, "");
							index++;
						}
						else
							sprintf(tmp, "%s%c", tmp, val[s]);


					};
				};

				break;
			};
		};
	};


	int setIndex	= 0;
	int base		= !global.DedicatedServer ? (!global.CWA ? 0x79F8D0 : 0x78E9C8) : (!global.CWA ? 0x71F738 : 0x71F788);
	int pointer		= 0;
	
	// Find beginning of the weather values
	ReadProcessMemory(phandle, (LPVOID)base, &pointer,  4, &stBytes);
	pointer += 0x7C4;

	for (i=0;  i<=7;  i++, setIndex++)
	{
		if (set[setIndex])
			WriteProcessMemory(phandle, (LPVOID)pointer, &valueFLT[setIndex], 4, &stBytes);

		// skip latitude and longitude
		if (i == 5)
			pointer += 12;
		else
			pointer += 4;
	};


	// Get the other set of weather values
	float value	= 0;
	int jump	= 0;
	int value2	= 0;

	base = !global.DedicatedServer ? (!global.CWA ? 0x7B3ACC : 0x7A2C0C) : (!global.CWA ? 0x73392C : 0x7339C4);
	ReadProcessMemory(phandle, (LPVOID)base, &pointer, 4, &stBytes);

	for (int offset=0x2054C;  offset<=0x2059C;  offset+=4, setIndex++) {
		// skip sky
		if (offset == 0x20550)
			offset += 4;

		// skip waves
		if (offset == 0x20594)
			offset += 8;

		if (set[setIndex]) {
			// write floats
			if (setIndex<16  ||  setIndex>20) {
				if (setIndex == 21) {
					if (!global.restore_memory[RESTORE_WAVE_SPEED]) {
						global.restore_memory[RESTORE_WAVE_SPEED] = 1;
						ReadProcessMemory(phandle,(LPVOID)(pointer+offset), &global.restore_float[FLOAT_WAVE_SPEED], 4, &stBytes);
					}
				}

				WriteProcessMemory(phandle, (LPVOID)(pointer+offset), &valueFLT[setIndex], 4, &stBytes);
			}

			// write ints
			if (setIndex == 16)
				WriteProcessMemory(phandle, (LPVOID)(pointer+offset), &valueINT[0], 4, &stBytes);

			if (setIndex == 18)
				WriteProcessMemory(phandle, (LPVOID)(pointer+offset), &valueINT[1], 4, &stBytes);

			if (setIndex == 20)
				WriteProcessMemory(phandle, (LPVOID)(pointer+offset), &valueINT[2], 4, &stBytes);

			// write float arrays
			if (setIndex == 17) {
				WriteProcessMemory(phandle, (LPVOID)(pointer+offset),   &windSpeed[0], 4, &stBytes);
				WriteProcessMemory(phandle, (LPVOID)(pointer+offset+4), &windSpeed[2], 4, &stBytes);
				WriteProcessMemory(phandle, (LPVOID)(pointer+offset+8), &windSpeed[1], 4, &stBytes);
			}

			if (setIndex == 19) {
				WriteProcessMemory(phandle, (LPVOID)(pointer+offset),   &gust[0], 4, &stBytes);
				WriteProcessMemory(phandle, (LPVOID)(pointer+offset+4), &gust[2], 4, &stBytes);
				WriteProcessMemory(phandle, (LPVOID)(pointer+offset+8), &gust[1], 4, &stBytes);
			}
		}

		// skip wind vectors
		if (offset==0x20574  ||  offset==0x20584)
			offset +=8;
	}


	// Tide and wave
	DWORD old = 0;
	base	  = !global.DedicatedServer ? (!global.CWA ? 0x72F8E4 : 0x72295C) : (!global.CWA ? 0x6BE184 : 0x6BE144);

	if (set[22]) {
		if (!global.restore_memory[RESTORE_TIDE]) {
			global.restore_memory[RESTORE_TIDE] = 1;
			ReadProcessMemory(phandle,(LPVOID)base, &global.restore_float[FLOAT_TIDE], 4, &stBytes);
		}

		VirtualProtectEx(phandle, (LPVOID)base, 256, PAGE_EXECUTE_READWRITE, &old),
		WriteProcessMemory(phandle, (LPVOID)base, &valueFLT[22], 4, &stBytes);
	}

	if (set[23]) {
		if (!global.restore_memory[RESTORE_WAVE]) {
			global.restore_memory[RESTORE_WAVE] = 1;
			ReadProcessMemory(phandle,(LPVOID)(base+4), &global.restore_float[FLOAT_WAVE], 4, &stBytes);
		}

		VirtualProtectEx(phandle, (LPVOID)(base+4), 256, PAGE_EXECUTE_READWRITE, &old),
		WriteProcessMemory(phandle, (LPVOID)(base+4), &valueFLT[23], 4, &stBytes);
	}
}
break;








case C_MEM_SETCAM:
{ // Get external camera offset

	bool multiplayer = false;

	// Parse arguments
	for (int i=2; i<numP; i++)
	{
		char *arg = stripq(par[i]);
		char *pch = strchr(arg, ':');

		if (pch == NULL) 
			continue;

		int pos		= pch-arg;
		arg[pos]	= '\0';
		char *val	= Trim(arg+pos+1);

		if (strcmpi(arg,"mp") == 0) {
			multiplayer = String2Bool(val);
		}

		if (strcmpi(arg,"extCameraPosition") == 0)
		{
			// float array
			int index		= 0;
			int max			= strlen(val);
			char tmp[64]	= "";
			float extcam[]	= {0,0,0};

			for (int s=0; s<max; s++)
			{
				if (val[s] == '[')
					continue;			

				if (val[s]==','  ||  val[s]==']')
				{
					extcam[index] = (float)atof(tmp);
					strcpy(tmp, "");
					index++;
				}
				else
					sprintf(tmp, "%s%c", tmp, val[s]);
			};

			int pointer[]	= {0,0,0,0};
			int	modif[]		= {0,0,0};
			int	max_loops	= (sizeof(pointer) / sizeof(pointer[0])) - 1;

			if (global.CWA) {
				pointer[0] = multiplayer ? 0x38E93C : 0x7A3128;
				modif[0]   = multiplayer ? 0x94     : 0xAC;
				modif[1]   = multiplayer ? 0x4A8    : 0x31C;
				modif[2]   = 0x6A0;
			} else {
				pointer[0] = multiplayer ? 0x0 : 0x7B4030;
				modif[0]   = multiplayer ? 0x0 : 0x7C8;
				modif[1]   = multiplayer ? 0x0 : 0x4A8;
				modif[2]   = multiplayer ? 0x0 : 0x69C;
			}



			for (int i=0; i<max_loops; i++)
			{
				ReadProcessMemory(phandle, (LPVOID)pointer[i], &pointer[i+1], 4, &stBytes);
				pointer[i+1] = pointer[i+1] + modif[i];
			};

			if (!global.restore_memory[RESTORE_EXTCAMPOS])
			{
				global.restore_memory[RESTORE_EXTCAMPOS] = 1;
				ReadProcessMemory(phandle,(LPVOID)(pointer[max_loops]),	  &global.restore_float[FLOAT_EXTCAMX], 4, &stBytes);
				ReadProcessMemory(phandle,(LPVOID)(pointer[max_loops]+4), &global.restore_float[FLOAT_EXTCAMZ], 4, &stBytes);
				ReadProcessMemory(phandle,(LPVOID)(pointer[max_loops]+8), &global.restore_float[FLOAT_EXTCAMY], 4, &stBytes);
				global.extCamOffset = pointer[max_loops];
			};

			WriteProcessMemory(phandle, (LPVOID)(pointer[max_loops]),	&extcam[0], 4, &stBytes);
			WriteProcessMemory(phandle, (LPVOID)(pointer[max_loops]+4), &extcam[2], 4, &stBytes);
			WriteProcessMemory(phandle, (LPVOID)(pointer[max_loops]+8), &extcam[1], 4, &stBytes);
		};
	};
}
break;






case C_MEM_HUD:
{
	int	max_loops = sizeof(hud_offset) / sizeof(hud_offset[0]);
	int ui_base   = 0;
	int chat_base = 0;
	int currentINT[ARRAY_SIZE];
	float current[ARRAY_SIZE];
	bool is_custom[ARRAY_SIZE];

	memset(current    , 0, ARRAY_SIZE*4);
	memset(currentINT , 0, ARRAY_SIZE*4);
	memset(is_custom  , 0, ARRAY_SIZE*1);


	// Parse input
	for (int i=2; i<numP; i++) {
		char *arg = stripq(par[i]);
		char *pch = strchr(arg, ':');

		if (pch == NULL) 
			continue;

		int pos	  = pch - arg;
		arg[pos]  = '\0';
		char *val = Trim(arg+pos+1);

		// If name matches then queue for change
		for (int j=0; j<max_loops; j++) {
			if (strcmpi(arg,hud_names[j]) == 0) {
				is_custom[j] = 1;

				int is_int = IsNumberInArray(j,hud_int_list,sizeof(hud_int_list)/sizeof(hud_int_list[0]));
				if (is_int) {
					if (IsNumberInArray(j,hud_color_list,sizeof(hud_color_list)/sizeof(hud_color_list[0]))) {
						int index              = 0;
						char *number           = strtok(val, "[,];");
						unsigned char color[4] = {0,0,0,0};

						while (number != NULL  &&  index<4) {
							color[index++] = (unsigned char)(atof(number) * 255);
							number         = strtok(NULL, "[,];");
						}

						currentINT[j] = ((color[3] << 24) | (color[0] << 16) | (color[1] << 8) | color[2]);
					} else
						currentINT[j] = atoi(val);
				} else
					current[j] = (float)atof(val);

				break;
			}
		}
	}

	switch(game_version) {
		case VER_196 : ui_base=0x79F8D0; break;
		case VER_199 : ui_base=0x78E9C8; break;
		case VER_201 : ui_base=global.exe_address+0x6D8240; break;
	}

	ReadProcessMemory(phandle, (LPVOID)(ui_base+0x0), &ui_base, 4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)(ui_base+0x8), &ui_base, 4, &stBytes);

	if (ui_base == 0)
		break;

	// Chat has a different address
	switch(game_version) {
		case VER_196 : chat_base=0x7831B0; break;
		case VER_199 : chat_base=0x7722A0; break;
		case VER_201 : chat_base=global.exe_address+0x6FFCC0; break;
	}

	int pointer = 0;
	char tmp[256] = "";
	QWrite("[", out);

	for (i=0; i<max_loops; i++) {
		if (i >= CHAT_X)
			pointer = chat_base;
		else
			pointer = ui_base;

		int is_int = IsNumberInArray(i,hud_int_list,sizeof(hud_int_list)/sizeof(hud_int_list[0]));

		if (is_custom[i]) {	
			if (!global.restore_memory[RESTORE_HUD+i] && (i<CHAT_X || i>CHAT_H)) {
				global.restore_memory[RESTORE_HUD+i] = 1;
				if (is_int)
					ReadProcessMemory(phandle,(LPVOID)(pointer+hud_offset[i]), &global.restore_hud_int[i], 4, &stBytes);
				else
					ReadProcessMemory(phandle,(LPVOID)(pointer+hud_offset[i]), &global.restore_hud_float[i], 4, &stBytes);
			}

			if (is_int)
				WriteProcessMemory(phandle, (LPVOID)(pointer+hud_offset[i]), &currentINT[i], 4, &stBytes);
			else
				WriteProcessMemory(phandle, (LPVOID)(pointer+hud_offset[i]), &current[i], 4, &stBytes);
		}

		if (is_int) {
			ReadProcessMemory(phandle, (LPVOID)(pointer+hud_offset[i]), &currentINT[i], 4, &stBytes);

			if (IsNumberInArray(i,hud_color_list,sizeof(hud_color_list)/sizeof(hud_color_list[0]))) {
				unsigned char alpha = (currentINT[i] >> 24) & 0xFF;
				unsigned char red   = (currentINT[i] >> 16) & 0xFF;
				unsigned char green = (currentINT[i] >> 8)  & 0xFF;
				unsigned char blue  = currentINT[i] & 0xFF;
				sprintf(tmp, "]+[[%.6f,%.6f,%.6f,%.6f]", (float)red/255,(float)green/255,(float)blue/255,(float)alpha/255);
			} else {
				sprintf(tmp, "]+[%d", currentINT[i]);
			}
		} else {
			ReadProcessMemory(phandle, (LPVOID)(pointer+hud_offset[i]), &current[i], 4, &stBytes);		
			sprintf(tmp, "]+[%f", current[i]);
		}

		QWrite(tmp, out);
	}

	QWrite("]", out);
}
break;







// -----------------------------------------------------------------------------------
case C_MEM_MULTI:		// reuses the same cases
	QWrite("[",out);

case C_MEM_GETCAM:
{ // Get camera values from the memory
	
	char tmp[128]	= "";
	float posX		= 0;
	float posY		= 0;
	float posZ		= 0;
	float sin		= 0;
	float cos		= 0;
	float dir		= 0;
	float pitch		= 0;
	float fov		= 0;
	int plr			= 1;
	int off[]		= 
	{
		!global.CWA ? 0x788434 : 0x77751C,
		!global.CWA ? 0x788450 : 0x777538, 
		!global.CWA ? 0x788458 : 0x777540, 
		!global.CWA ? 0x78845C : 0x777544, 
		!global.CWA ? 0x788460 : 0x777548, 
		!global.CWA ? 0x7884F8 : 0x7775E0, 
		!global.CWA ? 0x78864C : 0x777614, 
		!global.CWA ? 0x79DFCC : 0x78D0C3		//if this one fails then C4
	};


	ReadProcessMemory(phandle, (LPVOID)off[0], &cos, 4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)off[1], &pitch, 4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)off[2], &posX, 4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)off[3], &posZ, 4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)off[4], &posY, 4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)off[5], &fov, 4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)off[6], &sin, 4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)off[7], &plr, 1, &stBytes);


	double result = rad2deg(acos(cos));	// arccosine dir; rad to deg
	if (sin < 0) 
		result = 360 - result;			// format value
	dir = float(result);				// from double to float

	result = rad2deg(asin(pitch));
	pitch = float(result);



	// get ext cam pos
	float extcam[3] = {0,0,0};
	int pointer[]	= {0x7894A0,0,0};
	int	modif[]		= {0x5C, 0x69C};
	int	max_loops	= (sizeof(pointer) / sizeof(pointer[0])) - 1;

	if (global.CWA) 
		pointer[0] = 0x778590,
		modif[1]   = 0x6A0;

	for (int i=0; i<max_loops; i++)
	{
		ReadProcessMemory(phandle, (LPVOID)pointer[i], &pointer[i+1], 4, &stBytes);
		pointer[i+1] = pointer[i+1] + modif[i];
	};

	ReadProcessMemory(phandle, (LPVOID)(pointer[max_loops]+0x0), &extcam[0], 4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)(pointer[max_loops]+0x4), &extcam[1], 4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)(pointer[max_loops]+0x8), &extcam[2], 4, &stBytes);



	sprintf(tmp, "[%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%s,[%.6f,%.6f,%.6f]]", posX, posY, posZ, dir, pitch, fov, getBool(!plr),extcam[0],extcam[2],extcam[1]);

	if (isMULTI) 
		strcat(tmp,",");

	QWrite(tmp, out);
}
if (!isMULTI) 
	break;








case C_MEM_GETMAP:
{ // Get 2D map state from memory

	int base	= !global.CWA ? 0x7B4028 : 0x7A3128;
	int	pointer = 0;
	int	isMapOn = 0;
	
	ReadProcessMemory(phandle, (LPVOID)base,			&pointer, 4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)(pointer+0x7CF), &isMapOn, 1, &stBytes);

	QWrite(getBool(isMapOn), out);
}
if (!isMULTI) 
	break;
else 
	QWrite(",",out);







case C_MEM_GETNV:
{ // Get nightvision goggles value from the memory

//[[[0x00786CA0]] + 0x8] + 0x6C6

	int pointer[5]	= {0x786CA0,0,0,0};
	int	modif[3]	= {0x0, 0x8, 0x6C6};
	int	BytesToRead = 4;
	int	max_loops	= (sizeof(pointer) / sizeof(pointer[0])) - 1;


	if (global.CWA) 
		pointer[0]	= 0x7A3128, 
		modif[0]	= 0x78C, 
		modif[2]	= 0x6D6;


	for (int i=0; i<max_loops; i++)
	{
		if (i == max_loops-1)		// in last iteration read just one byte
			BytesToRead = 1;

		ReadProcessMemory(phandle, (LPVOID)pointer[i], &pointer[i+1], BytesToRead, &stBytes);

		if (i < max_loops-1) 
			pointer[i+1] = pointer[i+1] + modif[i];
	};

	QWrite(getBool(pointer[max_loops]),out);
}
if (!isMULTI) 
	break;
else 
	QWrite(",",out);







case C_MEM_GETPLAYERVIEW:
{ // Get camera view type from the memory

	char tmp[32]	= "[";
	int pointer		= 0;
	int display		= 0;
	int toggle		= 0;
	int *a			= &display;
	int	base		= !global.CWA ? 0x7B4028 : 0x7A3128;


	ReadProcessMemory(phandle, (LPVOID)base,			&pointer, 4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)(pointer+0x860), &toggle,  1, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)(pointer+0x864), &display, 1, &stBytes);


	for (int i=0; i<2; i++)
	{
		if (i != 0) 
			strcat(tmp, ","), 
			a = &toggle;

		strcat(tmp,"\"");

		switch (*a)
		{
			case 0: strcat(tmp,"INTERNAL"); break;
			case 1: strcat(tmp,"GUNNER"); break;
			case 2: strcat(tmp,"EXTERNAL"); break;
			case 3: strcat(tmp,"GROUP"); break;
		};

		strcat(tmp,"\"");
	};

	strcat(tmp,"]");

	if (isMULTI) 
		strcat(tmp,",");

	QWrite(tmp, out);
}
if (!isMULTI) 
	break;







case C_MEM_GETPLAYERAIM:
{ // Get player's mouse target and gun direction and angle

/* 
	OFP
[[[0x7B3ACC] + 0x38] + 0x8] + 0x7C		mouse sin
[[[0x7B3ACC] + 0x38] + 0x8] + 0x80		mouse pitch
[[[0x7B3ACC] + 0x38] + 0x8] + 0x84		mouse cos
[[[0x7B4028] + 0x784] + 0x8] + 0x474	gun horiz offset
[[[0x7B4028] + 0x784] + 0x8] + 0x478	gun horiz goal offset
[[[0x7B4028] + 0x784] + 0x8] + 0x47C	gun pitch
[[[0x7B4028] + 0x784] + 0x8] + 0x480	gun pitch goal
[[[0x7B4028] + 0x784] + 0x8] + 0x484	gun velocity vertical
[[[0x7B4028] + 0x784] + 0x8] + 0x488	gun velocity horizontal
	CWA
[[0x78E9C8] + 0x8] + 0x7C				mouse sin
[[0x78E9C8] + 0x8] + 0x80				mouse pitch
[[0x78E9C8] + 0x8] + 0x84				mouse cos
[[[0x7A3128] + 0x784] + 0x8] + 0x484	gun horiz offset
[[[0x7A3128] + 0x784] + 0x8] + 0x488	gun horiz goal offset
[[[0x7A3128] + 0x784] + 0x8] + 0x48C	gun pitch
[[[0x7A3128] + 0x784] + 0x8] + 0x490	gun pitch goal
[[[0x7A3128] + 0x784] + 0x8] + 0x494	gun velocity vertical
[[[0x7A3128] + 0x784] + 0x8] + 0x498	gun velocity horizontal
*/

	// Find where values are stored
	int pointer[4]	= {0x7B3ACC,0,0,0};
	int modif[3]	= {0x38, 0x8, 0x7C};
	int pointer2[4] = {0x7B4028,0,0,0};
	int modif2[3]	= {0x784, 0x8, 0x474};
	int max_loops	= sizeof(pointer) / sizeof(pointer[0]) - 1;


	if (global.CWA) 
		pointer[0]	= 0x78E9C8, 
		pointer2[0] = 0x7A3128, 
		modif[0]	= 0x8, 
		modif[1]	= 0x7C, 
		modif2[2]	= 0x484;


	for (int i=0; i<max_loops; i++)
	{
		// There's one less loop in CWA version
		if (!global.CWA  ||  global.CWA  &&  i<max_loops-1)
		{
			ReadProcessMemory(phandle, (LPVOID)pointer[i], &pointer[i+1], 4, &stBytes);
			pointer[i+1] = pointer[i+1] +  modif[i];
		};		
		
		ReadProcessMemory(phandle, (LPVOID)pointer2[i], &pointer2[i+1], 4, &stBytes);
		pointer2[i+1] = pointer2[i+1] +  modif2[i];
	};


	// Read values
	float m_sin = 0;
	float m_cos = 0;
	float m_pit = 0;
	float m_dir = 0;
	float g_off = 0;
	float g_pit = 0;
	float g_vlV = 0;
	float g_vlH = 0;
	float g_pit2 = 0;

	if (global.CWA) 
		max_loops--;

	ReadProcessMemory(phandle, (LPVOID)pointer[max_loops],     &m_sin, 4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)(pointer[max_loops]+4), &m_pit, 4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)(pointer[max_loops]+8), &m_cos, 4, &stBytes);

	if (global.CWA) 
		max_loops++;

	ReadProcessMemory(phandle, (LPVOID)(pointer2[max_loops]),   &g_off, 4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)(pointer2[max_loops]+8), &g_pit, 4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)(pointer2[max_loops]+12), &g_pit2, 4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)(pointer2[max_loops]+16), &g_vlV, 4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)(pointer2[max_loops]+20), &g_vlH, 4, &stBytes);


	// Format values
	double result = rad2deg(acos(m_cos));

	if (m_sin < 0) 
		result = 360 - result;

	m_dir = float(result);

	result = rad2deg(asin(g_off));
	g_off = float(result*-1);

	if (g_pit != g_pit) 
		g_pit = 0;

	if (g_pit2 != g_pit2) 
		g_pit2 = 0;


	char tmp[128] = "";
	sprintf(tmp, "[%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f]", m_dir, g_off, m_pit, g_pit, g_vlH, g_vlV, g_pit2);

	if (isMULTI) 
		strcat(tmp,",");

	QWrite(tmp, out);
}
if (!isMULTI) 
	break;







case C_MEM_GETPLAYERANIM:
{ // Get player animation code from memory

/*Multi-step pointers:
  OFP
[[[0x7B4028] + 0x784] + 0x8] + 0x708	basic
[[[0x79F8D0] + 0x7A8] + 0x8] + 0x708
[[[0x7B4028] + 0x788] + 0x8] + 0x708	with vehs but won't work in intro
  CWA
[[[0x78E9C8] + 0x7A8] + 0x8] + 0x718	basic
[[[0x7A3128] + 0x78C] + 0x8] + 0x718	vehs
*/

	int pointer[5]	 = {0x7B4028,0,0,0,0};
	int	modif[3]	 = {0x788, 0x8, 0x708};
	int	max_loops	 = (sizeof(pointer) / sizeof(pointer[0])) - 1; // number of loops = items in the array - 1
	bool restartPath = false;

	if (global.CWA) 
		pointer[0]	= 0x7A3128, 
		modif[0]	= 0x78C, 
		modif[2]	= 0x718;
	

	// Loop reading memory
	for (int i=0; i<max_loops; i++)
	{
		// read 4 times
		ReadProcessMemory(phandle, (LPVOID)pointer[i], &pointer[i+1], 4, &stBytes);

		// alternate path if current failed	
		if (!restartPath  &&  pointer[i+1]==0)
		{
			i			= -1; 
			restartPath = true;

			if (!global.CWA) 
				modif[0]	= 0x784; 
			else 
				pointer[0]	= 0x78E9C8,
				modif[0]	= 0x7A8;

			continue;
		};

		// modify 3 times
		if (i < max_loops-1) 
			pointer[i+1] = pointer[i+1] + modif[i];
	};


	char tmp[16] = "";
	sprintf(tmp, "%d", pointer[max_loops]);		// last read is the value we want

	if (isMULTI) 
		strcat(tmp,",");

	QWrite(tmp, out);
}
if (!isMULTI) 
	break;







case C_MEM_ISDIALOG:
{ // Return number of dialogs

	int i	 = 0;
	int base = !global.CWA ? 0x79E9E0 : 0x78DAD8;

	ReadProcessMemory(phandle, (LPVOID)base, &i, 4, &stBytes);

	char tmp[16] = "";
	sprintf(tmp, "%d", i);
	QWrite(tmp, out);
}
if (!isMULTI) 
	break;
else 
	QWrite(",", out);








case C_MEM_GETRADIOBOX:
{ // Is radio options box displayed

	int base	 = !global.CWA ? 0x79F8D0 : 0x78E9C8;
	int	pointer  = 0;
	int pointer2 = 0;
	int value	 = 0;

	ReadProcessMemory(phandle, (LPVOID)base,			 &pointer,  4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)(pointer+8),		 &pointer2, 4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)(pointer2+0x2D4), &value,	1, &stBytes);

	QWrite(getBool(value), out);
}
if (!isMULTI) 
	break;
else 
	QWrite(",",out);








case C_MEM_GETWEATHER:
{ // Get weather values from the memory

// [0x78E9C8] + 0x7C4

	float weather[8];
	char tmp[128]	= "";
	int base		= !global.DedicatedServer ? (!global.CWA ? 0x79F8D0 : 0x78E9C8) : (!global.CWA ? 0x71F738 : 0x71F788);
	int pointer		= 0;
	int	max_loops	= (sizeof(weather) / sizeof(weather[0])) - 1; 
	
	// Find beginning of the weather values
	ReadProcessMemory(phandle, (LPVOID)base, &pointer,  4, &stBytes);
	pointer += 0x7C4;

	// Read eight of them one after another
	QWrite("[", out);

	for (int i=0; i<=max_loops; i++)
	{
		ReadProcessMemory(phandle, (LPVOID)pointer, &weather[i], 4, &stBytes);
		sprintf(tmp, "%.6f,", weather[i]);
		QWrite(tmp, out);

		// skip latitude and longitude
		if (i == 5)
			pointer += 12;
		else
			pointer += 4;
	};


	// Get mission time
	int missionTime = 0;
	base = !global.DedicatedServer ? (!global.CWA ? 0x7DD028 : 0x7CBFE8) : (!global.CWA ? 0x75A2E0 : 0x75A370);

	ReadProcessMemory(phandle, (LPVOID)base, &missionTime, 4, &stBytes);
	sprintf(tmp, "%d,", missionTime);
	QWrite(tmp, out);


	// Get the other set of weather values
	float value	= 0;
	int jump	= 0;
	int value2	= 0;

	base = !global.DedicatedServer ? (!global.CWA ? 0x7B3ACC : 0x7A2C0C) : (!global.CWA ? 0x73392C : 0x7339C4);
	ReadProcessMemory(phandle, (LPVOID)base, &pointer, 4, &stBytes);

	for (int offset=0x2054C;  offset<=0x2059C;  offset+=4)
	{
		// skip sky
		if (offset == 0x20550)
			offset += 4;

		// Wind vectors output in a different order
		if (jump==0  &&  (offset==0x20578 || offset==0x20588))
			offset += 4,
			jump    = 1;

		ReadProcessMemory(phandle, (LPVOID)(pointer+offset), &value, 4, &stBytes);
		ReadProcessMemory(phandle, (LPVOID)(pointer+offset), &value2, 4, &stBytes);

		// Wind vectors put in array
		if (offset==0x20574  ||  offset==0x20584)
			QWrite("[", out);

		// Time is int, everything else is float
		if (offset==0x20570  ||  offset==0x20580  ||  offset==0x20590)
			sprintf(tmp, "%d", value2);
		else
			sprintf(tmp, "%.6f", value);
		
		// Close array for wind vector
		if (jump == 2)
			strcat(tmp, "]");

		strcat(tmp, ",");
		QWrite(tmp, out);

		if (jump == 2)
			offset += 4,
			jump    = 0;

		if (jump == 1)
			offset -= 8,
			jump    = 2;
	};

	
	// Get tide and wave values
	base			= !global.DedicatedServer ? (!global.CWA ? 0x72F8E4 : 0x72295C) : (!global.CWA ? 0x6BE184 : 0x6BE144);
	float maxTide	= 0;
	float maxWave	= 0;

	ReadProcessMemory(phandle, (LPVOID)base,	 &maxTide, 4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)(base+4), &maxWave, 4, &stBytes);

	sprintf(tmp, "%.6f,%.6f]", maxTide, maxWave);
	QWrite(tmp, out);

	if (isMULTI) 
		QWrite("]",out);
}
break;
