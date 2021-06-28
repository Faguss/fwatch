// -----------------------------------------------------------------
// INPUT READING
// -----------------------------------------------------------------

case C_INPUT_GETKEYS:				// v1.1 updateda
{ // Extended version of getkeys

	if(argument_num < 2) 
	{
		QWrite("ERROR: Not enough parameters");
		break;
	}

	bool side=false;
	if (argument_num>2) side=true;		// optional parameter

	QWrite("[");				
	bool f = false;
	if (checkActiveWindow())
	{
		for (int i=0;i < 256;i++)				//v1.1 changed 128 to 256
		{
			if ((GetKeyState(i) & 0x8000) >> 15)
			{
				if (!side && i>=0xA0 && i<=0xA5) continue;	//v1.1 skip side SHIFT,CTRL,ALT
				if (side  && i>=16   && i<=18)   continue;	//v1.1 skip SHIFT,CTRL,ALT
				if (f) QWrite(",");
				QWrite_formatKey(i);
				f = true;
			}
		}
	}
	QWrite("]");
}
break;






case C_INPUT_AGETKEYS:				// v1.1 updated
{ // Extended version of agetkeys

	if(argument_num < 2)
	{
		QWrite("ERROR: Not enough parameters");
		break;
	}

	bool side=false;
	if (argument_num>2) side=true;		// optional parameter

	QWrite("[");
	bool f = false;
	if (checkActiveWindow())
	{
		for (int i=0;i < 256;i++) 				//v1.1 changed 128 to 256
		{
			if (GetAsyncKeyState(i) & 0x0001)
			{
				if (!side && i>=0xA0 && i<=0xA5) continue;	//v1.1 skip side SHIFT,CTRL,ALT
				if (side  && i>=16   && i<=18)   continue;	//v1.1 skip SHIFT,CTRL,ALT
				if (f) QWrite(",");
				QWrite_formatKey(i);
				f = true;
			}
		}
	}
	QWrite("]");
}
break;






case C_INPUT_GETKEY:
{ // Return 1 if key pressed
	if(argument_num < 3) {
		QWrite("ERROR: Not enough parameters");
		break;
	}
	char idx = toupper(argument[2][0]);
				
	if((GetKeyState(idx) & 0x8000) >> 15 && checkActiveWindow())
		QWrite("1");
	else
		QWrite("-1");
}
break;






case C_INPUT_AGETKEY:
{ // Return 1 if key pressed, works asynchronously
	if(argument_num < 3) {
		QWrite("ERROR: Not enough parameters");
		break;
	}
	char idx = toupper(argument[2][0]);
				
	if(GetAsyncKeyState(idx) & 0x0001 && checkActiveWindow())
		QWrite("1");
	else
		QWrite("-1");
}
break;






case C_INPUT_GETMOUSE:
{ // Get mouse data [x, y, leftmouse, rightmouse, middlemouse]
	if(argument_num < 2) {
		QWrite("ERROR: Not enough parameters");
		break;
	}
				
	// Get mouse position etc.
	POINT mp;
	GetCursorPos(&mp);
	QWritef("[%d, %d, %s, %s, %s]", mp.x, mp.y, getBool(GetAsyncKeyState(VK_LBUTTON)), getBool(GetAsyncKeyState(VK_RBUTTON)), getBool(GetAsyncKeyState(VK_MBUTTON)));
}
break;






case C_INPUT_SETMOUSE:
{ // Set mouse cursor position
	if(argument_num < 4) {
		QWrite("ERROR: Not enough parameters");
		break;
	}

	unsigned int mx = atoi(argument[2]);
	unsigned int my = atoi(argument[3]);
				
	// Get mouse position etc.
	if(checkActiveWindow()) { // Do not set cursor pos is OFP window is not active
		if(SetCursorPos(mx, my))
			QWrite("1");
		else
			QWrite("-1");
		} else
			QWrite("-1");
	}
break;






case C_INPUT_LOCK:
{ // Return Num, Caps, Scroll status

	QWritef("[%s,%s,%s]", 
		getBool(GetKeyState(VK_NUMLOCK) & 1), 
		getBool(GetKeyState(VK_CAPITAL) & 1), 
		getBool(GetKeyState(VK_SCROLL)  & 1)
	);
}
break;






case C_INPUT_GETJOYSTICK:
{ // Read joystick input using windows api

	QWrite_Joystick(argument_num>2 ? atoi(argument[2]) : - 1);
}
break;






case C_INPUT_MULTI:
{ // Read keyboard, mouse, joystick

	QWrite("[[");

	// INPUT GETKEYS --------------------------------------------------------------------------------		
	bool gameActive = checkActiveWindow();
	if (gameActive)
		for (int i=0; i<256; i++)
			if ((GetKeyState(i) & 0x8000) >> 15)
			{
				if (i>=16 && i<=18) continue;	// skip shift,alt,ctrl
				QWrite("]+[");
				QWrite_formatKey(i);
			};
	QWrite("],[");
	//-----------------------------------------------------------------------------------------------

	// INPUT AGETKEYS -------------------------------------------------------------------------------
	if (gameActive)
		for (int i=0; i<256; i++)
			if(GetAsyncKeyState(i) & 0x0001)
			{
				if (i>=16 && i<=18)  continue;	// skip shift,alt,ctrl
				QWrite("]+[");
				QWrite_formatKey(i);
			};
	QWrite("],[");
	//-----------------------------------------------------------------------------------------------

	// INPUT LOCK -----------------------------------------------------------------------------------
	QWritef("%s,%s,%s", 
		getBool(GetKeyState(VK_NUMLOCK) & 1),
		getBool(GetKeyState(VK_CAPITAL) & 1),
		getBool(GetKeyState(VK_SCROLL)  & 1)
	);
	//-----------------------------------------------------------------------------------------------

	// INPUT GETMOUSE -------------------------------------------------------------------------------
	POINT mp;
	GetCursorPos(&mp);
	QWritef("],[%d,%d", mp.x, mp.y);
	//-----------------------------------------------------------------------------------------------

	// MEM GETCURSOR --------------------------------------------------------------------------------
	float X=-1, Y=-1;
	int offset = 0;
	switch(global_exe_version[global.exe_index]) {
		case VER_196 : offset=0x79E94C; break;
		case VER_199 : offset=0x78DA44; break;
		case VER_201 : offset=global.exe_address+0x71611C; break;
	}
	if (offset != 0) {
		ReadProcessMemory(phandle, (LPVOID)offset,	   &X, 4, &stBytes);
		ReadProcessMemory(phandle, (LPVOID)(offset+4), &Y, 4, &stBytes);
	}
	QWritef(",%.6f,%.6f", ++X/2, ++Y/2);
	//-----------------------------------------------------------------------------------------------

	// MEM GETSCROLL --------------------------------------------------------------------------------
	int scroll = 0;
	ReadProcessMemory(phandle, (LPVOID)global.exe_address_scroll, &scroll ,4, &stBytes);	
	QWritef(",%d]", scroll/120);
	//-----------------------------------------------------------------------------------------------

	// MEM GETJOYSTICK ------------------------------------------------------------------------------
	if (global_exe_version[global.exe_index] != VER_201) {
		int i=0, but=0, pov=0, povAngle=65535, base = 0;

		switch(global_exe_version[global.exe_index]) {
			case VER_196 : base=0x79E994; break;
			case VER_199 : base=0x78DA8C; break;
		}

		float axisX=0, axisY=0, axisZ=0, axisR1=0, axisR2=0;
		ReadProcessMemory(phandle, (LPVOID)base,	 &axisX, 4, &stBytes);
		ReadProcessMemory(phandle, (LPVOID)(base+4), &axisY, 4, &stBytes);
		ReadProcessMemory(phandle, (LPVOID)(base+8), &axisR2,4, &stBytes);
		ReadProcessMemory(phandle, (LPVOID)(base+20),&axisZ, 4, &stBytes);
		ReadProcessMemory(phandle, (LPVOID)(base+24),&axisR1,4, &stBytes);
		
		if (axisR1==0) axisR1=axisR2;
		QWritef(",[%.6f,%.6f,%.6f,%.6f,[", axisX,axisY,axisZ,axisR1);

		switch(global_exe_version[global.exe_index]) {
			case VER_196 : base=0x79E96C; break;
			case VER_199 : base=0x78DA64; break;
		}

		for (i=0; i<8; i++)
		{	
			ReadProcessMemory(phandle, (LPVOID)base, &but, 1, &stBytes);
			if (but != 0) QWritef("]+[\"JOY%d\"", i+1);
			base += 4;
		};

		switch(global_exe_version[global.exe_index]) {
			case VER_196 : base=0x79E95C; break;
			case VER_199 : base=0x78DA4C; break;
		}

		for (i=0; i<8; i++)
		{
			ReadProcessMemory(phandle,(LPVOID)(base+i),&pov,1,&stBytes);
			if (pov != 0) break;
		};

		QWrite("]+[");
		switch(i)
		{
			case 0 : povAngle=0; QWrite("\"JOYPOVUP\""); break;
			case 1 : povAngle=45; QWrite("\"JOYPOVUPRIGHT\""); break;
			case 2 : povAngle=90; QWrite("\"JOYPOVRIGHT\""); break;
			case 3 : povAngle=135; QWrite("\"JOYPOVDOWNRIGHT\""); break;
			case 4 : povAngle=180; QWrite("\"JOYPOVDOWN\""); break;
			case 5 : povAngle=225; QWrite("\"JOYPOVDOWNLEFT\""); break;
			case 6 : povAngle=270; QWrite("\"JOYPOVLEFT\""); break;
			case 7 : povAngle=315; QWrite("\"JOYPOVUPLEFT\""); break;
			default: QWrite("\"\"");
		};
		QWritef("],%d],", povAngle);
	} else {
		QWritef(",[0,0,0,0,[],65535],");
	}
	//-----------------------------------------------------------------------------------------------

	// INPUT GETJOYSTICK ----------------------------------------------------------------------------
	if (argument_num >= 3) 
		QWrite_Joystick(atoi(argument[2]));
	else
		QWrite("[]");
	//-----------------------------------------------------------------------------------------------

	// MEM GETSPEEDKEY ------------------------------------------------------------------------------
	if (global_exe_version[global.exe_index] != VER_201) {
		int offset_[] = {0x0, 0x8, 0x4, 0xC}, 
			weight[] = {3, 2, 1, -2}, 
			max_loops = sizeof(offset_) / sizeof(offset_[0]),
			speed=0, quantity=0, current=0;
		
		int base = 0;
		switch(global_exe_version[global.exe_index]) {
			case VER_196 : base=0x79E9C2; break;
			case VER_199 : base=0x78DABA; break;
		}

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

		QWrite(",[\"");
		if (speed>=3) QWrite("fast"); else
		if (speed==2) QWrite("forward"); else
		if (speed==1) QWrite("slow"); else
		if (speed==0) QWrite("stop"); else
		if (speed<0) QWrite("reverse");
		QWrite("\"");

		for (i=0; i<max_loops; i++)
			QWritef(",%d", offset_[i]);

		QWrite("]]");
	} else
		QWrite(",[]]");
}
break;
