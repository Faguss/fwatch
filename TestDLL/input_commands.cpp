// -----------------------------------------------------------------
// INPUT READING
// -----------------------------------------------------------------

case C_INPUT_GETKEYS:				// v1.1 updateda
{ // Extended version of getkeys

	if(numP < 2) 
	{
		QWrite("ERROR: Not enough parameters", out);
		break;
	}

	bool side=false;
	if (numP>2) side=true;		// optional parameter

	QWrite("[", out);				
	bool f = false;
	if (checkActiveWindow())
	{
		for (int i=0;i < 256;i++)				//v1.1 changed 128 to 256
		{
			if ((GetKeyState(i) & 0x8000) >> 15)
			{
				if (!side && i>=0xA0 && i<=0xA5) continue;	//v1.1 skip side SHIFT,CTRL,ALT
				if (side  && i>=16   && i<=18)   continue;	//v1.1 skip SHIFT,CTRL,ALT
				if (f) QWrite(",",out);
				QWrite(formatKey(i), out);
				f = true;
			}
		}
	}
	QWrite("]", out);
}
break;






case C_INPUT_AGETKEYS:				// v1.1 updated
{ // Extended version of agetkeys

	if(numP < 2)
	{
		QWrite("ERROR: Not enough parameters", out);
		break;
	}

	bool side=false;
	if (numP>2) side=true;		// optional parameter

	QWrite("[", out);
	bool f = false;
	if (checkActiveWindow())
	{
		for (int i=0;i < 256;i++) 				//v1.1 changed 128 to 256
		{
			if (GetAsyncKeyState(i) & 0x0001)
			{
				if (!side && i>=0xA0 && i<=0xA5) continue;	//v1.1 skip side SHIFT,CTRL,ALT
				if (side  && i>=16   && i<=18)   continue;	//v1.1 skip SHIFT,CTRL,ALT
				if (f) QWrite(",",out);
				QWrite(formatKey(i), out);
				f = true;
			}
		}
	}
	QWrite("]", out);
}
break;






case C_INPUT_GETKEY:
{ // Return 1 if key pressed
	if(numP < 3) {
		QWrite("ERROR: Not enough parameters", out);
		break;
	}
	char idx = toupper(par[2][0]);
				
	if((GetKeyState(idx) & 0x8000) >> 15 && checkActiveWindow())
		QWrite("1", out);
	else
		QWrite("-1", out);
}
break;






case C_INPUT_AGETKEY:
{ // Return 1 if key pressed, works asynchronously
	if(numP < 3) {
		QWrite("ERROR: Not enough parameters", out);
		break;
	}
	char idx = toupper(par[2][0]);
				
	if(GetAsyncKeyState(idx) & 0x0001 && checkActiveWindow())
		QWrite("1", out);
	else
		QWrite("-1", out);
}
break;






case C_INPUT_GETMOUSE:
{ // Get mouse data [x, y, leftmouse, rightmouse, middlemouse]
	if(numP < 2) {
		QWrite("ERROR: Not enough parameters", out);
		break;
	}
	char ret[64]="";	//v1.13 changed
				
	// Get mouse position etc.
	POINT mp;
	GetCursorPos(&mp);
	sprintf(ret, "[%d, %d, %s, %s, %s]", mp.x, mp.y, getBool(GetAsyncKeyState(VK_LBUTTON)), 
			 getBool(GetAsyncKeyState(VK_RBUTTON)), getBool(GetAsyncKeyState(VK_MBUTTON)));
	QWrite(ret, out);
}
break;






case C_INPUT_SETMOUSE:
{ // Set mouse cursor position
	if(numP < 4) {
		QWrite("ERROR: Not enough parameters", out);
		break;
	}

	unsigned int mx = atoi(par[2]);
	unsigned int my = atoi(par[3]);
				
	// Get mouse position etc.
	if(checkActiveWindow()) { // Do not set cursor pos is OFP window is not active
		if(SetCursorPos(mx, my))
			QWrite("1", out);
		else
			QWrite("-1", out);
		} else
			QWrite("-1", out);
	}
break;






case C_INPUT_LOCK:
{ // Return Num, Caps, Scroll status

	if ((GetKeyState(VK_NUMLOCK) & 1)!=0) QWrite("[true,",out); else QWrite("[false,",out);
	if ((GetKeyState(VK_CAPITAL) & 1)!=0) QWrite("true,", out); else QWrite("false,", out);
	if ((GetKeyState(VK_SCROLL)  & 1)!=0) QWrite("true]", out); else QWrite("false]", out);
}
break;






case C_INPUT_GETJOYSTICK:
{ // Read joystick input using windows api

	char data[256] = "";
	int joyID = -1;
	if (numP > 2) 
		joyID=atoi(par[2]);

	ReadJoystick(data, joyID);
	QWrite(data, out);
}
break;






case C_INPUT_MULTI:
{ // Read keyboard, mouse, joystick

	char tmp[1024]="[[";

	// INPUT GETKEYS --------------------------------------------------------------------------------		
	bool gameActive=checkActiveWindow();
	int cond = 0;
	if (gameActive)
		for (int i=0; i<256; i++)
			if ((GetKeyState(i) & 0x8000) >> 15)
			{
				if (i>=16 && i<=18) continue;	// skip shift,alt,ctrl
				strcat(tmp,"]+[");
				strcat(tmp, formatKey(i));
			};
	strcat(tmp, "],[");
	//-----------------------------------------------------------------------------------------------

	// INPUT AGETKEYS -------------------------------------------------------------------------------
	if (gameActive)
		for (int i=0; i<256; i++)
			if(GetAsyncKeyState(i) & 0x0001)
			{
				if (i>=16 && i<=18)  continue;	// skip shift,alt,ctrl
				strcat(tmp, "]+[");
				strcat(tmp, formatKey(i));
			};
	strcat(tmp, "],[");
	//-----------------------------------------------------------------------------------------------

	// INPUT LOCK -----------------------------------------------------------------------------------
	strcat(tmp, getBool((GetKeyState(VK_NUMLOCK) & 1)!=0));	strcat(tmp,",");
	strcat(tmp, getBool((GetKeyState(VK_CAPITAL) & 1)!=0)); strcat(tmp,",");
	strcat(tmp, getBool((GetKeyState(VK_SCROLL)  & 1)!=0));
	//-----------------------------------------------------------------------------------------------

	// INPUT GETMOUSE -------------------------------------------------------------------------------
	POINT mp;
	GetCursorPos(&mp);
	sprintf(tmp, "%s],[%d,%d", tmp, mp.x, mp.y);
	//-----------------------------------------------------------------------------------------------

	// MEM GETCURSOR --------------------------------------------------------------------------------
	float X=-1, Y=-1;
	int offset = 0;
	switch(game_version) {
		case VER_196 : offset=0x79E94C; break;
		case VER_199 : offset=0x78DA44; break;
		case VER_201 : offset=global.exe_address+0x71611C; break;
	}
	if (offset != 0) {
		ReadProcessMemory(phandle, (LPVOID)offset,	   &X, 4, &stBytes);
		ReadProcessMemory(phandle, (LPVOID)(offset+4), &Y, 4, &stBytes);
	}
	sprintf(tmp, "%s,%.6f,%.6f", tmp, ++X/2, ++Y/2);
	//-----------------------------------------------------------------------------------------------

	// MEM GETSCROLL --------------------------------------------------------------------------------
	offset = 0; 
	int scroll=0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, global.pid);
	MODULEENTRY32 xModule;
 
	if (hSnap != INVALID_HANDLE_VALUE)
	{
		xModule.dwSize = sizeof(MODULEENTRY32);
		if (Module32First(hSnap, &xModule) != 0)
		{
			do
			{
				if (lstrcmpi(xModule.szModule, (LPCTSTR)"dinput8.dll") == 0)
				{
					offset = (int)xModule.modBaseAddr;
					if (xModule.modBaseSize == 233472) offset+=0x2D848; else offset+=0x2C1C8;
					ReadProcessMemory(phandle, (LPVOID)offset, &scroll ,4, &stBytes);
					break;
				}
			}
			while (Module32Next(hSnap, &xModule));
		};
		CloseHandle(hSnap);
	};
	sprintf(tmp, "%s,%d]", tmp,scroll/120);
	//-----------------------------------------------------------------------------------------------

	// MEM GETJOYSTICK ------------------------------------------------------------------------------
	if (game_version != VER_201) {
		int i=0, but=0, pov=0, povAngle=65535,
			base = !global.CWA ? 0x79E994 : 0x78DA8C;

		float axisX=0, axisY=0, axisZ=0, axisR1=0, axisR2=0;
		ReadProcessMemory(phandle, (LPVOID)base,	 &axisX, 4, &stBytes);
		ReadProcessMemory(phandle, (LPVOID)(base+4), &axisY, 4, &stBytes);
		ReadProcessMemory(phandle, (LPVOID)(base+8), &axisR2,4, &stBytes);
		ReadProcessMemory(phandle, (LPVOID)(base+20),&axisZ, 4, &stBytes);
		ReadProcessMemory(phandle, (LPVOID)(base+24),&axisR1,4, &stBytes);
		
		if (axisR1==0) axisR1=axisR2;
		sprintf(tmp, "%s,[%.6f,%.6f,%.6f,%.6f,[", tmp,axisX,axisY,axisZ,axisR1);

		base = !global.CWA ? 0x79E96C : 0x78DA64;
		for (i=0; i<8; i++)
		{	
			ReadProcessMemory(phandle, (LPVOID)base, &but, 1, &stBytes);
			if (but != 0) sprintf(tmp, "%s]+[\"JOY%d\"", tmp, i+1);
			base += 4;
		};

		base = !global.CWA ? 0x79E95C : 0x78DA4C;
		for (i=0; i<8; i++)
		{
			ReadProcessMemory(phandle,(LPVOID)(base+i),&pov,1,&stBytes);
			if (pov != 0) break;
		};

		strcat(tmp,"]+[");
		switch(i)
		{
			case 0 : povAngle=0; strcat(tmp,"\"JOYPOVUP\""); break;
			case 1 : povAngle=45; strcat(tmp,"\"JOYPOVUPRIGHT\""); break;
			case 2 : povAngle=90; strcat(tmp,"\"JOYPOVRIGHT\""); break;
			case 3 : povAngle=135; strcat(tmp,"\"JOYPOVDOWNRIGHT\""); break;
			case 4 : povAngle=180; strcat(tmp,"\"JOYPOVDOWN\""); break;
			case 5 : povAngle=225; strcat(tmp,"\"JOYPOVDOWNLEFT\""); break;
			case 6 : povAngle=270; strcat(tmp,"\"JOYPOVLEFT\""); break;
			case 7 : povAngle=315; strcat(tmp,"\"JOYPOVUPLEFT\""); break;
		};
		sprintf(tmp, "%s],%d],", tmp, povAngle);
	} else {
		sprintf(tmp, "%s,[0,0,0,0,[],65535],", tmp);
	}
	//-----------------------------------------------------------------------------------------------

	// INPUT GETJOYSTICK ----------------------------------------------------------------------------
	if (numP >= 3) 
		ReadJoystick(tmp, atoi(par[2]));
	else
		strcat(tmp, "[]");
	//-----------------------------------------------------------------------------------------------

	// MEM GETSPEEDKEY ------------------------------------------------------------------------------
	if (game_version != VER_201) {
		int offset_[] = {0x0, 0x8, 0x4, 0xC}, 
			weight[] = {3, 2, 1, -2}, 
			max_loops = sizeof(offset_) / sizeof(offset_[0]),
			speed=0, quantity=0, current=0;
		
		int base = !global.CWA ? 0x79E9C2 : 0x78DABA;

		for (int i=0; i<max_loops; i++)
		{
			ReadProcessMemory(phandle, (LPVOID)(base+offset_[i]), &current, 2, &stBytes);
			switch(current)
			{
				case 16256 : quantity=1; break;
				case 16384 : quantity=2; break;
				case 16448 : quantity=3; break;
				case 16512 : quantity=4; break;
				case 16544 : quantity=5; break;
				case 16576 : quantity=6; break;
				default : if (current>16576) quantity=7; else quantity=0;
			};
			speed = speed + weight[i] * quantity;
			offset_[i] = current;
		};

		strcat(tmp, ",[\"");
		if (speed>=3) strcat(tmp, "fast"); else
		if (speed==2) strcat(tmp, "forward"); else
		if (speed==1) strcat(tmp, "slow"); else
		if (speed==0) strcat(tmp, "stop"); else
		if (speed<0) strcat(tmp, "reverse");
		strcat(tmp, "\"");

		for (i=0; i<max_loops; i++)
			sprintf(tmp, "%s,%d", tmp,offset_[i]);

		strcat(tmp, "]]");
	} else {
		sprintf(tmp, "%s,[]]", tmp);
	}
	//-----------------------------------------------------------------------------------------------

	QWrite(tmp, out);
}
break;
