// -----------------------------------------------------------------
// MEMORY OPERATIONS
// -----------------------------------------------------------------

case C_MEM_GETCURSOR:
{ // Get mouse cursor position from memory

	int base     = 0;
	float pos[2] = {-1, -1};

	switch(global_exe_version[global.exe_index]) {
		case VER_196 : base=0x79E94C; break;
		case VER_199 : base=0x78DA44; break;
		case VER_201 : base=global.exe_address+0x71611C; break;
	}

	if (base)
		ReadProcessMemory(phandle, (LPVOID)base, &pos, 8, &stBytes);

	QWritef("[%.6f,%.6f]", ++pos[0]/2, ++pos[1]/2);
}
break;






case C_MEM_SETCURSOR:
{ // Write mouse cursor position to memory

	if (argument_num < 4) {
		QWrite(":mem setcursor ERROR - not enough parameters");
		break;
	}

	int base     = 0;
	float pos[2] = {
		(float)atof(argument[2].text) * 2 - 1,
		(float)atof(argument[3].text) * 2 - 1
	};

	switch(global_exe_version[global.exe_index]) {
		case VER_196 : base=0x79E94C; break;
		case VER_199 : base=0x78DA44; break;
		case VER_201 : base=global.exe_address+0x71611C; break;
	}

	if (base)
		WriteProcessMemory(phandle, (LPVOID)base, &pos, 8, &stBytes);
}
break;






case C_MEM_GETWORLD:
{ // Read island shortcut from memory

	char island[80] = "";
	int base        = 0;
	
	switch(global_exe_version[global.exe_index]) {
		case VER_196        : base=0x7DD130; break;
		case VER_199        : base=0x7CC0F0; break;
		case VER_201        : base=global.exe_address+0x714BBC; break;
		case VER_196_SERVER : base=0x75A3E8; break;
		case VER_199_SERVER : base=0x75A478; break;
		case VER_201_SERVER : base=global.exe_address+0x60B00C; break;
	}

	if (base)
		ReadProcessMemory(phandle, (LPVOID)base, &island, 80, &stBytes);


	// Simple or extended info
	if (argument_num<=2  ||  strcmpi(argument[2].text,"extended")!=0)
		QWrite(island);
	else {
		// Get island size
		int pointer   = 0;
		int land_size = 0;

		switch(global_exe_version[global.exe_index]) {
			case VER_196 : base=0x7B3ACC; break;
			case VER_199 : base=0x7A2C0C; break;
			case VER_201 : base=global.exe_address+0x6D6B34; break;
			default      : base=0;
		}

		if (base) {
			ReadProcessMemory(phandle, (LPVOID)base,          &pointer,   4, &stBytes);
			ReadProcessMemory(phandle, (LPVOID)(pointer+0x8), &land_size, 4, &stBytes);
		}


		// Get geographical coordinates
		float coordinates[2] = {0};

		switch(global_exe_version[global.exe_index]) {
			case VER_196        : base=0x79F8D0; break;
			case VER_199        : base=0x78E9C8; break;
			case VER_201        : base=global.exe_address+0x6D8240; break;
			case VER_196_SERVER : base=0x71F738; break;
			case VER_199_SERVER : base=0x71F788; break;
			case VER_201_SERVER : base=global.exe_address+0x5CE6B8; break;
			default             : base=0;
		}

		if (base) {
			ReadProcessMemory(phandle, (LPVOID)base, &pointer, 4, &stBytes);
			pointer += 0x7DC;

			ReadProcessMemory(phandle, (LPVOID)pointer, &coordinates, 8, &stBytes);
		}

		QWritef("[\"%s\",%d,%.6f,%.6f]", 
			island, 
			land_size, 
			rad2deg(coordinates[0]), 
			rad2deg(coordinates[1])
		);
	}
}
break;






case C_MEM_SETMAP:
{ // Change map state in memory

	if (argument_num < 3) {
		QWrite(":mem setmap ERROR - not enough parameters");
		break;
	}

	int base    = 0;
	int	pointer = 0;
	bool is_map = String_bool(argument[2]);

	switch(global_exe_version[global.exe_index]) {
		case VER_196 : base=0x7B4028; break;
		case VER_199 : base=0x7A3128; break;
		case VER_201 : base=global.exe_address+0x6D7018; break;
	}

	if (base) {
		ReadProcessMemory (phandle, (LPVOID)base,			 &pointer, 4, &stBytes);     
		WriteProcessMemory(phandle, (LPVOID)(pointer+0x7CF), &is_map , 1, &stBytes);
	}
}
break;








case C_MEM_GETGRAPHICS:
{ // Get graphic options values from memory

	int base    = 0;
	int pointer = 0;


	// Resolution windowed
	int resolution_window[2] = {0};

	switch(global_exe_version[global.exe_index]) {
		case VER_196 : base=0x7DCFB4; break;
		case VER_199 : base=0x7CBF74; break;
		default      : base=0;
	}

	if (base)
		ReadProcessMemory(phandle, (LPVOID)base, &resolution_window, 8, &stBytes);



	// Resolution fullscreen, brightness, multitexturing, fov, refresh rate, gamma
	struct EngineRecord {
		float brightness;
		int unknown[2];
		bool nightvision;
		bool multitexturing;
		int unknown2;
		float fovLeft;
		float fovTop;
		float uiTopLeftX;
		float uiTopLeftY;
		float uiBottomRightX;
		float uiBottomRightY;
		int unknown3[52];
		int resolutionX;
		int resolutionY;
		int pixelsize;
		int depth;
		int refresh;
		int unknown4[392];
		float gamma;
	} engine;

	switch(global_exe_version[global.exe_index]) {
		case VER_196 : base=0x789D88; break;
		case VER_199 : base=0x778E80; break;
		default      : base=0;
	}

	if (base) {
		ReadProcessMemory(phandle, (LPVOID)base,            &pointer, 4, &stBytes);
		ReadProcessMemory(phandle, (LPVOID)(pointer+0x2C) , &engine,  sizeof(EngineRecord), &stBytes);
	}


	// Framerate, terrain detail, visual quality, shadows, cloudlets, terrain detail
	struct SceneRecord {
		float framerate;
		int unknown[5];
		float visual_quality;
		int unknown2[19]; 
		bool object_shadows;
		bool vehicle_shadows;
		bool cloudlets;
		float terrain_detail;
	} scene;

	switch(global_exe_version[global.exe_index]) {
		case VER_196 : base=0x7B4028; break;
		case VER_199 : base=0x7A3128; break;
		default      : base=0;
	}

	if (base) {
		ReadProcessMemory(phandle, (LPVOID)base,			&pointer, 4, &stBytes);
		ReadProcessMemory(phandle, (LPVOID)(pointer+0x524), &scene,	sizeof(SceneRecord), &stBytes);
	}


	// Visibility
	struct ConfigRecord {
		float visibility_fog;
		float visibility_objects;
		float visibility;
		float unused;
		float visibility_shadows;
		int max_objects;
		float tracks_duration;
		float tracks_disappear_rate;
		float benchmark;
		int max_lights;
		float lod;
		float limit_lod;
		float shadows_lod;
		int unknown[7];
		bool unknown2[3];
		bool blood;
	} config;

	switch(global_exe_version[global.exe_index]) {
		case VER_196 : base=0x7DD068; break;
		case VER_199 : base=0x7CC028; break;
		default      : base=0; break;
	}

	if (base)
		ReadProcessMemory(phandle, (LPVOID)base, &config, sizeof(ConfigRecord), &stBytes);



	// Current terrain detail
	float current_terrain_detail = 0;

	switch(global_exe_version[global.exe_index]) {
		case VER_196 : base=0x7B3ACC; break;
		case VER_199 : base=0x7A2C0C; break;
		default      : base=0;
	}

	if (base) {
		ReadProcessMemory(phandle, (LPVOID)base, &pointer, 4, &stBytes);
		ReadProcessMemory(phandle, (LPVOID)(pointer+0x2C), &current_terrain_detail, 4, &stBytes);
	}


	QWritef("[%d,%d,%d,%s,%s,%s,%s,%s,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%d,%.6f,%.6f,%d,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f]", 
			resolution_window[0]==0 ? resolution_window[0] : engine.resolutionX, 
			resolution_window[1]==0 ? resolution_window[1] : engine.resolutionY, 
			engine.refresh,
			getBool(engine.multitexturing), 
			getBool(scene.object_shadows), 
			getBool(scene.vehicle_shadows), 
			getBool(scene.cloudlets), 
			getBool(config.blood),
			engine.brightness, 
			engine.gamma, 
			scene.framerate, 
			scene.visual_quality, 
			config.visibility, 
			scene.terrain_detail,
			config.visibility_fog,
			config.visibility_objects,
			config.unused,
			config.visibility_shadows,
			config.max_objects,
			config.tracks_duration,
			config.tracks_disappear_rate,
			config.max_lights,
			engine.fovLeft,
			engine.fovTop,
			engine.uiTopLeftX,
			engine.uiTopLeftY,
			engine.uiBottomRightX,
			engine.uiBottomRightY,
			current_terrain_detail
	);
}
break;






case C_MEM_SETGRAPHICS:
{ // Change graphical options

	if (argument_num < 3) 
		break;

	// brightness, gamma, vquality, objectshadows, vehicleshadows, cloudlets, visibility
	// b, g, vq, os, vs, c, v

	// Read pointers
	int p1 = 0;
	int	p2 = 0;
	int	p3 = 0;
	int b1 = 0;
	int b2 = 0;
	int b3 = 0;
	int base = 0;

	switch(global_exe_version[global.exe_index]) {
		case VER_196 : b1=0x789D88; b2=0x7B4028; b3=0x79F8D0; break;
		case VER_199 : b1=0x778E80; b2=0x7A3128; b3=0x78E9C8; break;
	}

	if (!b1)
		break;

	ReadProcessMemory(phandle, (LPVOID)b1, &p1, 4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)b2, &p2, 4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)b3, &p3, 4, &stBytes);


	// Parse input
	for (size_t i=2; i<argument_num; i+=2) {
		switch (argument_hash[i]) {
			case NAMED_ARG_BRIGHTNESS : {
				if (!global.restore_memory[RESTORE_BRIGHTNESS]) {
					global.restore_memory[RESTORE_BRIGHTNESS] = 1;
					ReadProcessMemory(phandle,(LPVOID)(p1+0x2C), &global.restore_float[FLOAT_BRIGHTNESS], 4, &stBytes);
				}

				float val = (float)atof(argument[i+1].text);
				WriteProcessMemory(phandle,(LPVOID)(p1+0x2C), &val, 4, &stBytes);
			} break;

			case NAMED_ARG_VISUALQUALITY : {
				float val = (float)atof(argument[i+1].text);
				WriteProcessMemory(phandle,(LPVOID)(p2+0x53C), &val, 4, &stBytes);
			} break;

			case NAMED_ARG_OBJECTSHADOWS : {
				if (!global.restore_memory[RESTORE_OBJECT_SHADOWS]) {
					global.restore_memory[RESTORE_OBJECT_SHADOWS] = 1;
					ReadProcessMemory(phandle,(LPVOID)(p3+0x5B0), &global.restore_byte[BYTE_OBJECT_SHADOWS], 1, &stBytes);
				}

				int val = atoi(argument[i+1].text);
				if (val < 0) val=0;
				if (val > 1) val=1;
				WriteProcessMemory(phandle, (LPVOID)(p3+0x5B0), &val, 1, &stBytes);
			} break;

			case NAMED_ARG_VEHICLESHADOWS : {
				if (!global.restore_memory[RESTORE_VEHICLE_SHADOWS]) {
					global.restore_memory[RESTORE_VEHICLE_SHADOWS] = 1;
					ReadProcessMemory(phandle,(LPVOID)(p3+0x5B1), &global.restore_byte[BYTE_VEHICLE_SHADOWS], 1, &stBytes);
				}

				int val = atoi(argument[i+1].text);
				if (val < 0) val=0;
				if (val > 1) val=1;
				WriteProcessMemory(phandle, (LPVOID)(p3+0x5B1), &val, 1, &stBytes);
			} break;

			case NAMED_ARG_CLOUDLETS : {
				if (!global.restore_memory[RESTORE_CLOUDLETS]) {
					global.restore_memory[RESTORE_CLOUDLETS] = 1;
					ReadProcessMemory(phandle,(LPVOID)(p3+0x5B2), &global.restore_byte[BYTE_CLOUDLETS], 1, &stBytes);
				}

				int val = atoi(argument[i+1].text);
				if (val < 0) val=0;
				if (val > 1) val=1;
				WriteProcessMemory(phandle, (LPVOID)(p3+0x5B2), &val, 1, &stBytes);
			}

			case NAMED_ARG_LANDSCAPEDISTANCE : {
				float val = (float)atof(argument[i+1].text);
				switch(global_exe_version[global.exe_index]) {
					case VER_196 : base=0x7DD068; break;
					case VER_199 : base=0x7CC028; break;
					default      : base=0;
				}
				if (base)
					WriteProcessMemory(phandle, (LPVOID)base, &val, 4, &stBytes);
			} break;

			case NAMED_ARG_OBJECTSDISTANCE : {
				float val = (float)atof(argument[i+1].text);
				switch(global_exe_version[global.exe_index]) {
					case VER_196 : base=0x7DD06C; break;
					case VER_199 : base=0x7CC02C; break;
					default      : base=0;
				}
				if (base)
					WriteProcessMemory(phandle, (LPVOID)base, &val, 4, &stBytes);
			} break;

			case NAMED_ARG_VIEWDISTANCE : {
				float val = (float)atof(argument[i+1].text);
				switch(global_exe_version[global.exe_index]) {
					case VER_196 : base=0x7DD070; break;
					case VER_199 : base=0x7CC030; break;
					default      : base=0;
				}
				if (base)
					WriteProcessMemory(phandle, (LPVOID)base, &val, 4, &stBytes);
			} break;

			case NAMED_ARG_RADARDISTANCE : {
				switch(global_exe_version[global.exe_index]) {
					case VER_196 : base=0x7DD074; break;
					case VER_199 : base=0x7CC034; break;
					default      : base=0;
				}
				if (base) {
					if (!global.restore_memory[RESTORE_RADAR]) {
						global.restore_memory[RESTORE_RADAR] = 1;
						ReadProcessMemory(phandle,(LPVOID)base, &global.restore_float[FLOAT_RADAR], 4, &stBytes);
					}

					float val = (float)atof(argument[i+1].text);
					WriteProcessMemory(phandle, (LPVOID)base, &val, 4, &stBytes);
				}
			}

			case NAMED_ARG_SHADOWSDISTANCE : {
				float val = (float)atof(argument[i+1].text);
				switch(global_exe_version[global.exe_index]) {
					case VER_196 : base=0x7DD078; break;
					case VER_199 : base=0x7CC038; break;
					default      : base=0;
				}
				if (base)
					WriteProcessMemory(phandle, (LPVOID)base, &val, 4, &stBytes);
			} break;

			case NAMED_ARG_MAXOBJECTS : {
				switch(global_exe_version[global.exe_index]) {
					case VER_196 : base=0x7DD07C; break;
					case VER_199 : base=0x7CC03C; break;
					default      : base=0;
				}

				if (base) {
					if (!global.restore_memory[RESTORE_MAX_OBJECTS]) {
						global.restore_memory[RESTORE_MAX_OBJECTS] = 1;
						ReadProcessMemory(phandle,(LPVOID)base, &global.restore_int[INT_MAX_OBJECTS], 4, &stBytes);
					}

					int val = atoi(argument[i+1].text);
					WriteProcessMemory(phandle, (LPVOID)base, &val, 4, &stBytes);
				}
			} break;

			case NAMED_ARG_TRACKTIME : {
				switch(global_exe_version[global.exe_index]) {
					case VER_196 : base=0x7DD080; break;
					case VER_199 : base=0x7CC040; break;
					default      : base=0;
				}

				if (base) {
					if (!global.restore_memory[RESTORE_TRACK1]) {
						global.restore_memory[RESTORE_TRACK1] = 1;
						ReadProcessMemory(phandle,(LPVOID)base, &global.restore_float[FLOAT_TRACK1], 4, &stBytes);
					}

					float val = (float)atof(argument[i+1].text);
					WriteProcessMemory(phandle, (LPVOID)base, &val, 4, &stBytes);
				}
			} break;

			case NAMED_ARG_INVTRACKTIME : {
				switch(global_exe_version[global.exe_index]) {
					case VER_196 : base=0x7DD084; break;
					case VER_199 : base=0x7CC044; break;
					default      : base=0;
				}

				if (base) {
					if (!global.restore_memory[RESTORE_TRACK2]) {
						global.restore_memory[RESTORE_TRACK2] = 1;
						ReadProcessMemory(phandle,(LPVOID)base, &global.restore_float[FLOAT_TRACK2], 4, &stBytes);
					}

					float val = (float)atof(argument[i+1].text);
					WriteProcessMemory(phandle, (LPVOID)base, &val, 4, &stBytes);
				}
			} break;

			case NAMED_ARG_MAXLIGHTS : {
				switch(global_exe_version[global.exe_index]) {
					case VER_196 : base=0x7DD08C; break;
					case VER_199 : base=0x7CC04C; break;
					default      : base=0;
				}

				if (base) {
					if (!global.restore_memory[RESTORE_MAX_LIGHTS]) {
						global.restore_memory[RESTORE_MAX_LIGHTS] = 1;
						ReadProcessMemory(phandle,(LPVOID)base, &global.restore_int[INT_MAX_LIGHTS], 4, &stBytes);
					}

					int val = atoi(argument[i+1].text);
					WriteProcessMemory(phandle, (LPVOID)base, &val, 4, &stBytes);
				}
			} break;

			case NAMED_ARG_FOVLEFT : {
				if (!global.restore_memory[RESTORE_FOVLEFT]) {
					global.restore_memory[RESTORE_FOVLEFT] = 1;
					ReadProcessMemory(phandle,(LPVOID)(LPVOID)(p1+0x40), &global.restore_float[FLOAT_FOVLEFT], 4, &stBytes);
				}

				float val = (float)atof(argument[i+1].text);
				WriteProcessMemory(phandle, (LPVOID)(p1+0x40), &val, 4, &stBytes);
			} break;

			case NAMED_ARG_FOVTOP : {
				if (!global.restore_memory[RESTORE_FOVTOP]) {
					global.restore_memory[RESTORE_FOVTOP] = 1;
					ReadProcessMemory(phandle,(LPVOID)(LPVOID)(p1+0x40+4), &global.restore_float[FLOAT_FOVTOP], 4, &stBytes);
				}

				float val = (float)atof(argument[i+1].text);
				WriteProcessMemory(phandle, (LPVOID)(p1+0x40+4), &val, 4, &stBytes);
			} break;

			case NAMED_ARG_UITOPLEFTX : {
				if (!global.restore_memory[RESTORE_UITOPLEFTX]) {
					global.restore_memory[RESTORE_UITOPLEFTX] = 1;
					ReadProcessMemory(phandle,(LPVOID)(LPVOID)(p1+0x40+8), &global.restore_float[FLOAT_UITOPLEFTX], 4, &stBytes);
				}

				float val = (float)atof(argument[i+1].text);
				WriteProcessMemory(phandle, (LPVOID)(p1+0x40+8), &val, 4, &stBytes);
			} break;

			case NAMED_ARG_UITOPLEFTY : {
				if (!global.restore_memory[RESTORE_UITOPLEFTY]) {
					global.restore_memory[RESTORE_UITOPLEFTY] = 1;
					ReadProcessMemory(phandle,(LPVOID)(LPVOID)(p1+0x40+12), &global.restore_float[FLOAT_UITOPLEFTY], 4, &stBytes);
				}

				float val = (float)atof(argument[i+1].text);
				WriteProcessMemory(phandle, (LPVOID)(p1+0x40+12), &val, 4, &stBytes);
			} break;

			case NAMED_ARG_UIBOTTOMRIGHTX : {
				if (!global.restore_memory[RESTORE_UIBOTTOMRIGHTX]) {
					global.restore_memory[RESTORE_UIBOTTOMRIGHTX] = 1;
					ReadProcessMemory(phandle,(LPVOID)(LPVOID)(p1+0x40+16), &global.restore_float[FLOAT_UIBOTTOMRIGHTX], 4, &stBytes);
				}

				float val = (float)atof(argument[i+1].text);
				WriteProcessMemory(phandle, (LPVOID)(p1+0x40+16), &val, 4, &stBytes);
			} break;

			case NAMED_ARG_UIBOTTOMRIGHTY : {
				if (!global.restore_memory[RESTORE_UIBOTTOMRIGHTY]) {
					global.restore_memory[RESTORE_UIBOTTOMRIGHTY] = 1;
					ReadProcessMemory(phandle,(LPVOID)(LPVOID)(p1+0x40+20), &global.restore_float[FLOAT_UIBOTTOMRIGHTY], 4, &stBytes);
				}

				float val = (float)atof(argument[i+1].text);
				WriteProcessMemory(phandle, (LPVOID)(p1+0x40+20), &val, 4, &stBytes);
			} break;
		}
	}
}
break;






case C_MEM_GETJOYSTICK:
{ // Get joystick values from memory

	bool addComma = false;
	int i         = 0;
	int but       = 0;
	int pov       = 0;
	int povAngle  = 65535;
	int	base      = 0;

	switch(global_exe_version[global.exe_index]) {
		case VER_196 : base=0x79E994; break;
		case VER_199 : base=0x78DA8C; break;
	}


	// AXES --------------------------------------------------------
	float axisX  = 0;
	float axisY  = 0;
	float axisZ  = 0;
	float axisR1 = 0;
	float axisR2 = 0;

	if (base) {
		ReadProcessMemory(phandle, (LPVOID)base,	 &axisX, 4, &stBytes);
		ReadProcessMemory(phandle, (LPVOID)(base+4), &axisY, 4, &stBytes);
		ReadProcessMemory(phandle, (LPVOID)(base+8), &axisR2,4, &stBytes);
		ReadProcessMemory(phandle, (LPVOID)(base+20),&axisZ, 4, &stBytes);
		ReadProcessMemory(phandle, (LPVOID)(base+24),&axisR1,4, &stBytes);
	}
	
	if (axisR1 == 0) 
		axisR1 = axisR2;

	QWritef("[%.6f,%.6f,%.6f,%.6f,[", axisX,axisY,axisZ,axisR1);
	// -------------------------------------------------------------

	// BUTTONS -----------------------------------------------------
	switch(global_exe_version[global.exe_index]) {
		case VER_196 : base=0x79E96C; break;
		case VER_199 : base=0x78DA64; break;
	}

	for (i=0; i<8; i++) {	
		if (base)
			ReadProcessMemory(phandle, (LPVOID)base, &but, 1, &stBytes);

		if (but != 0) {
			if (addComma) 
				QWrite(","); 
			else 
				addComma = 1;

			QWritef("\"JOY%d\"", i+1);
		}

		base += 4;
	}
	// -------------------------------------------------------------

	// POV CHECK ---------------------------------------------------
	// one of the eight bytes indicates pov status
	switch(global_exe_version[global.exe_index]) {
		case VER_196 : base=0x79E95C; break;
		case VER_199 : base=0x78DA4C; break;
	}

	for (i=0; i<8; i++) {
		if (base)
			ReadProcessMemory(phandle,(LPVOID)(base+i),&pov,1,&stBytes);

		if (pov != 0) 
			break;
	}
	// -------------------------------------------------------------

	// POV PRINT ---------------------------------------------------
	if (i!=8  &&  addComma) 
		QWrite(",");

	switch(i) {
		case 0 : povAngle=0; QWrite("\"JOYPOVUP\"]"); break;
		case 1 : povAngle=45; QWrite("\"JOYPOVUPRIGHT\"]"); break;
		case 2 : povAngle=90; QWrite("\"JOYPOVRIGHT\"]"); break;
		case 3 : povAngle=135; QWrite("\"JOYPOVDOWNRIGHT\"]"); break;
		case 4 : povAngle=180; QWrite("\"JOYPOVDOWN\"]"); break;
		case 5 : povAngle=225; QWrite("\"JOYPOVDOWNLEFT\"]"); break;
		case 6 : povAngle=270; QWrite("\"JOYPOVLEFT\"]"); break;
		case 7 : povAngle=315; QWrite("\"JOYPOVUPLEFT\"]"); break;
		default : QWrite("]");
	}

	QWritef(",%d]", povAngle);
	// -------------------------------------------------------------
}
break;






case C_MEM_GETSCROLL:	
{ // Get mouse scroll counter value from memory

	int scroll = 0;
	ReadProcessMemory(phandle, (LPVOID)global.exe_address_scroll, &scroll ,4, &stBytes);	
	QWritef("%d", scroll/120);		// one scroll movement changes value by 120
}
break;














case C_MEM_SETPLAYERANIM:		
{ // Set player animation code in memory

	if (argument_num < 3) {
		QWrite(":mem setplayeranim ERROR - not enough parameters");
		break;
	}

	int pointer[4] = {0};
	int	modif[3]   = {0};

	switch(global_exe_version[global.exe_index]) {
		case VER_196 : pointer[0]=0x7B4028; modif[0]=0x788; modif[1]=0x8; modif[2]=0x708; break;
		case VER_199 : pointer[0]=0x7A3128; modif[0]=0x78C; modif[1]=0x8; modif[2]=0x718; break;
	}


	bool restartPath = false;
	int animcode     = atoi(argument[2].text);

	if (animcode<0  ||  !pointer[0]) 
		break;


	// Unlike in getplayeranim we're reading 3 times instead of 4
	// because we don't need value under the last address; just the address
	for (int i=0, max=(sizeof(pointer) / sizeof(pointer[0]))-1;  i<max;  i++) {
		ReadProcessMemory(phandle, (LPVOID)pointer[i], &pointer[i+1], 4, &stBytes);

		if (!restartPath  &&  pointer[i+1]==0) {
			i			= -1;
			restartPath = true;

			switch(global_exe_version[global.exe_index]) {
				case VER_196 : modif[0]=0x784; break;
				case VER_199 : pointer[0]=0x78E9C8; modif[0]=0x7A8; break;
			}

			continue;
		}

		pointer[i+1] = pointer[i+1] +  modif[i];
	}

	WriteProcessMemory(phandle, (LPVOID)pointer[max], &animcode, 4, &stBytes);
}
break;






case C_MEM_GETCINEMABORDER:
{ // Get showCinemaBorder value from the memory

	int cin  = 0;
	int base = 0;

	switch(global_exe_version[global.exe_index]) {
		case VER_196 : base=0x76D1D0; break;
		case VER_199 : base=0x755678; break;
	}

	if (base)
		ReadProcessMemory(phandle, (LPVOID)base, &cin, 1, &stBytes);

	QWrite(getBool(cin));
}
break;






case C_MEM_GETRESPAWNTYPE:
{ // Get respawn value from the memory

	int	base    = 0;
	int pointer = 0;
	int respawn = 0;

	switch(global_exe_version[global.exe_index]) {
		case VER_196        : base=0x78337C; break;
		case VER_199        : base=0x77246C; break;
		case VER_196_SERVER : base=0x7031D8; break;
		case VER_199_SERVER : base=0x703228; break;
	}

	if (base) {
		ReadProcessMemory(phandle, (LPVOID)base,           &pointer, 4, &stBytes);
		ReadProcessMemory(phandle, (LPVOID)(pointer+0x6C), &respawn, 1, &stBytes);
	}

	QWritef("%d", respawn);
}
break;






case C_MEM_SETRESPAWNTYPE:
{ // Set respawn value in the memory

	if (argument_num < 3) {
		QWrite(":mem setrespawntype ERROR - not enough parameters");
		break;
	}

	int pointer = 0;
	int respawn = atoi(argument[2].text);
	int base	= 0;

	switch(global_exe_version[global.exe_index]) {
		case VER_196 : base=0x78337C; break;
		case VER_199 : base=0x77246C; break;
	}
	
	if (base) {
		ReadProcessMemory (phandle, (LPVOID)base,           &pointer, 4, &stBytes);
		WriteProcessMemory(phandle, (LPVOID)(pointer+0x6C), &respawn, 1, &stBytes);
	}
}
break;






case C_MEM_GETRESSIDE:
{ // Get resistance friendliness values from the memory

	int west = 0;
	int east = 0;
	int	base = 0;

	switch(global_exe_version[global.exe_index]) {
		case VER_196        : base=0x786850; break;
		case VER_199        : base=0x775938; break;
		case VER_196_SERVER : base=0x7066A8; break;
		case VER_199_SERVER : base=0x7066F8; break;
	}

	if (base) {
		ReadProcessMemory(phandle, (LPVOID)base,     &east, 4, &stBytes);
		ReadProcessMemory(phandle, (LPVOID)(base+4), &west, 4, &stBytes);
	}

	if (west>0  &&  east==0) 
		QWrite("west");
	else
		if (west==0  &&  east>0) 
			QWrite("east");
		else
			if (west>0  &&  east>0) 
				QWrite("everybody");
			else
				QWrite("nobody");
}
break;






case C_MEM_GETDAYLIGHT:
{ // Get brightness related values from the memory

	float daytime   = 0;
	float daybright = 0;
	int pointer     = 0;
	int pointer2    = 0;
	int	base        = 0;

	switch(global_exe_version[global.exe_index]) {
		case VER_196        : base=0x7B4028; break;
		case VER_199        : base=0x7A3128; break;
		case VER_196_SERVER : base=0x733E88; break;
		case VER_199_SERVER : base=0x733F20; break;
	}

	if (base) {
		ReadProcessMemory(phandle, (LPVOID)base,           &pointer,  4, &stBytes);
		ReadProcessMemory(phandle, (LPVOID)(pointer+0xB0), &pointer2, 4, &stBytes);

		ReadProcessMemory(phandle, (LPVOID)(pointer2+0x64), &daytime,  4, &stBytes);
		ReadProcessMemory(phandle, (LPVOID)(pointer2+0x8C), &daybright,4, &stBytes);
	}

	QWritef("[%.6f,%.6f]", daytime, daybright);
	
	// First value is G from RGB ground lighting colour but it can be used to 
	// check time of day
	
	// Second value is presumably brightness multiplier but it's read-only
}
break;






case C_MEM_GETDATE:
{ // Get mission date from the memory

	enum MEM_GETDATE_INDEX {
		INDEX_YEAR,
		INDEX_MONTH,
		INDEX_DAY,
		INDEX_DAYWEEK,
		INDEX_DAYYEAR
	};

	int value[5] = {0};
	int base[5]  = {0};

	switch(global_exe_version[global.exe_index]) {
		case VER_196 : 
			base[INDEX_YEAR]    = 0x780608;
			base[INDEX_MONTH]   = 0x7DD3FC;
			base[INDEX_DAY]     = 0x7DD400;
			base[INDEX_DAYWEEK] = 0x7DD408;
			base[INDEX_DAYYEAR] = 0x7DD40C;
			break;

		case VER_199 : 
			base[INDEX_YEAR]    = 0x76F7C8;
			base[INDEX_MONTH]   = 0x7CC3BC;
			base[INDEX_DAY]     = 0x7CC3C0;
			base[INDEX_DAYWEEK] = 0x7CC3C8;
			base[INDEX_DAYYEAR] = 0x7CC3CC;
			break;
	}

	for (int i=0, max=sizeof(base)/sizeof(base[0]);  i<max && base[0];  i++)
		ReadProcessMemory(phandle, (LPVOID)base[i], &value[i], 4, &stBytes);

	QWritef("[%d,%d,%d,%d,%d]", 
		base[INDEX_YEAR] + 1900, 
		base[INDEX_MONTH] + 1, 
		base[INDEX_DAY], 
		base[INDEX_DAYWEEK], 
		base[INDEX_DAYYEAR] + 1
	);
}
break;






case C_MEM_SETPLAYERVIEW:
{ // Change camera view type in the memory

	if (argument_num < 3) {
		QWrite(":mem setplayerview ERROR - not enough parameters");
		break;
	}

	String_trim_quotes(argument[2]);

	int pointer = 0;
	int mode    = -1;
	int	base    = 0;

	switch(global_exe_version[global.exe_index]) {
		case VER_196 : base=0x7B4028; break;
		case VER_199 : base=0x7A3128; break;
	}

	if (base) {
		// Convert string to number
		if (strcmpi(argument[2].text,"INTERNAL") == 0) 
			mode = 0; 
		else
			if (strcmpi(argument[2].text,"GUNNER") == 0) 
				mode = 1; 
			else
				if (strcmpi(argument[2].text,"EXTERNAL") == 0) 
					mode = 2; 
				else
					if (strcmpi(argument[2].text,"GROUP") == 0) 
					mode = 3; 
				else 
					break;

		ReadProcessMemory (phandle, (LPVOID)base,            &pointer, 4, &stBytes);
		WriteProcessMemory(phandle, (LPVOID)(pointer+0x860), &mode,    1, &stBytes);
	}
}
break;






case C_MEM_ERROR:
{ // Get error message from the memory
	
// [[[0x789D88] + 0x68] + 0x1C] + 0x00	OFP
// [[[0x778E80] + 0x68] + 0x1C] + 0x00	CWA
	
	// Base address and offsets
	int pointer[4] = {0, 0, 0, 0};
	int modif[3]   = {0x68, 0x1C, 0};
	int max_loops  = sizeof(pointer) / sizeof(pointer[0]) - 1;

	switch(global_exe_version[global.exe_index]) {
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
	}

	// if went nowhere
	if (pointer[max_loops] == 0) 
		break;

	char error_msg_buf[512] = "";
	ReadProcessMemory(phandle, (LPVOID)pointer[max_loops], &error_msg_buf, 512, &stBytes);

	String error_msg = {error_msg_buf, strlen(error_msg_buf)};

	// Default mode - just return the text
	if (argument_num == 2)
		QWritel(error_msg.text, error_msg.length);
	else 
		// Optional mode - copy text to clipboard
		if (argument_num>2  &&  strcmpi(argument[2].text,"clip")==0)
			if (CopyToClip(error_msg, false)) {
				global.option_error_output = OPTION_ERROR_ARRAY_CLOSE;
				QWrite_err(FWERROR_NONE, 0);
			}
}
break;






case C_MEM_SETPLAYERAIM:
{ // Set player's mouse target and gun direction and angle

	if (argument_num < 3) {
		QWrite(":mem setplayeraim ERROR - not enough parameters");
		break;
	}


	// Find where values are stored
	int pointer[4]	= {0};
	int modif[3]	= {0};
	int pointer2[4] = {0};
	int modif2[3]	= {0};
	int max_loops	= sizeof(pointer) / sizeof(pointer[0]) - 1;

	switch(global_exe_version[global.exe_index]) {
		case VER_196 : 
			pointer[0]  = 0x7B3ACC; 
			modif[0]    = 0x38;
			modif[1]    = 0x8;
			modif[2]    = 0x7C;
			pointer2[0] = 0x7B4028;
			modif2[0]   = 0x784;
			modif2[1]   = 0x8;
			modif2[2]   = 0x474;
			break;

		case VER_199 : 
			pointer[0]  = 0x78E9C8; 
			modif[0]    = 0x8;
			modif[1]    = 0x7C;
			pointer2[0] = 0x7A3128;
			modif2[0]   = 0x784;
			modif2[1]   = 0x8;
			modif2[2]   = 0x484;
			break;
	}

	if (pointer[0]) {
		for (int i=0; i<max_loops; i++) {
			// There's one less loop in CWA version
			if (global_exe_version[global.exe_index]!=VER_199  ||  (global_exe_version[global.exe_index]==VER_199  &&  i<max_loops-1)) {
				ReadProcessMemory(phandle, (LPVOID)pointer[i], &pointer[i+1], 4, &stBytes);
				pointer[i+1] = pointer[i+1] +  modif[i];
			}
			
			ReadProcessMemory(phandle, (LPVOID)pointer2[i], &pointer2[i+1], 4, &stBytes);
			pointer2[i+1] = pointer2[i+1] +  modif2[i];
		}


		// Parse input
		int index   = 0;
		int j       = global_exe_version[global.exe_index]!=VER_199 ? max_loops : max_loops-1;
		String item = {NULL, 0};
		size_t pos  = 0;

		while ((item = String_tokenize(argument[2], ",", pos, OPTION_TRIM_SQUARE_BRACKETS)).length > 0) {	
			if (strcmp(item.text,"-") != 0) {
				double num = atof(item.text);
				float fnum = (float)num;

				switch (index) {
					// Mouse target direction
					case 0 : {
						float m_sin = (float)sin(deg2rad(num));
						float m_cos = (float)cos(deg2rad(num));

						WriteProcessMemory(phandle, (LPVOID)pointer[j],     &m_sin, 4, &stBytes);
						WriteProcessMemory(phandle, (LPVOID)(pointer[j]+8), &m_cos, 4, &stBytes);
					} break;

					// Gun offset
					case 1 : {
						num  = sin(deg2rad(num));
						fnum = float(num*-1);

						WriteProcessMemory(phandle, (LPVOID)pointer2[max_loops],	 &fnum, 4, &stBytes);
						WriteProcessMemory(phandle, (LPVOID)(pointer2[max_loops]+4), &fnum, 4, &stBytes);
					} break;

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
				}
			}

			index++;
		}
	}
}
break;






case C_MEM_MODLIST:
{ // Get list of modfolders that user selected

//[[ifc22.dll + 0x2C154] + 0x0] + 0x0

	int base             = global.exe_address_ifc22 + 0x2C154;
	char parameters[512] = "";

	ReadProcessMemory(phandle, (LPVOID)base, &base, 4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)base, &base, 4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)base, &parameters, 512, &stBytes);


	// Parse game exe parameters (double null terminated)
	int word_begin     = -1;
	int word_begin_mod = -1;
	bool add_comma     = false;
	QWrite("[");
	
	for (int i=0; i<512; i++) {
		// Beginning of a parameter
		if (!isspace(parameters[i])  &&  parameters[i]!='\0'  &&  word_begin==-1)
			word_begin = i;
			
		// End of a parameter
		if ((isspace(parameters[i]) || parameters[i]=='\0')  &&  word_begin>=0)
			word_begin = -1;
		
		// Beginning of a -mod parameter
		if (word_begin>=0  &&  i-word_begin==5  &&  strncmpi(parameters+word_begin,"-mod=", 5)==0)
			word_begin_mod = i;
		
		// -mod sub-value
		if (word_begin_mod>=0 && (parameters[i]==';' || parameters[i]=='\0')) {
			if (i-word_begin_mod > 0) {
				if (add_comma)
					QWrite(",");
				else
					add_comma=true;
				
				QWrite("\"");
				QWritel(parameters+word_begin_mod, i-word_begin_mod);
				QWrite("\"");
			}
				
			word_begin_mod = i + 1;
			
			if (parameters[i] == '\0')
				word_begin_mod = -1;
		}
		
		if (parameters[i]=='\0' && parameters[i+1]=='\0')
			break;
	}
	
	QWrite("]");
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
	if (argument_hash[0]==C_MEM_GETDIFFICULTY  &&  argument_num<3) {
		QWrite("[]"); 
		break;
	}

	if (argument_hash[0]==C_MEM_SETDIFFICULTY  &&  argument_num<4) {
		QWrite("false"); 
		break;
	}


	bool single_player = strcmpi(argument[2].text,"sp")==0 || strcmpi(argument[2].text,"false")==0;
	bool multi_player  = strcmpi(argument[2].text,"mp")==0 || strcmpi(argument[2].text,"true")==0;
	bool veteran       = false;
	int base           = 0;
	int pointer        = 0;
	int offset         = 0;
	int	offsets[][4]   = {
		{0x7DD0C8, 0x7DD0D4, 0x75A380, 0x75A38C},	//ofp
		{0x7CC088, 0x7CC094, 0x75A410, 0x75A41C}	//cwa
	};


	// Select address from the array based on the arguments
	if (single_player  ||  global.is_server) {
		if (argument_hash[0]==C_MEM_GETDIFFICULTY  &&  argument_num<4) {		// not enough params
			QWrite("[]"); 
			break;
		}

		int i = global_exe_version[global.exe_index]!=VER_199 ? 0 : 1;	// which game
		int j = 0;				// which difficulty

		if (strcmpi(argument[3].text,"veteran")==0  ||  strcmpi(argument[3].text,"false")==0) {
			veteran = true;
			j++;
		}

		if (global.is_server) 
			j += 2;

		offset = offsets[i][j];
	} else 
		if (multi_player) {
			// player server
			switch(global_exe_version[global.exe_index]) {
				case VER_196 : base=0x783378; break;
				case VER_199 : base=0x772468; break;
				default : base=0; break;
			}

			if (base) {
				ReadProcessMemory(phandle, (LPVOID)base,           &pointer, 4, &stBytes);
				ReadProcessMemory(phandle, (LPVOID)(pointer+0x60), &offset,  4, &stBytes);

				// player client
				if (!offset) {
					switch(global_exe_version[global.exe_index]) {
						case VER_196 : base=0x78337C; break;
						case VER_199 : base=0x77246C; break;
						default : base=0; break;
					}

					if (base) {
						ReadProcessMemory(phandle, (LPVOID)base,			&pointer, 4, &stBytes);
						ReadProcessMemory(phandle, (LPVOID)(pointer+0x60),	&offset,  4, &stBytes);
					}
				}
			}

			if (!offset) {
				if (argument_hash[0] == C_MEM_GETDIFFICULTY)
					QWrite("[]"); 
				else
					QWrite("false");
				break;
			}
		} else {
			if (argument_hash[0] == C_MEM_GETDIFFICULTY)
				QWrite("[]"); 
			else
				QWrite("false");
			break;
		}


	// Now read memory
	if (argument_hash[0] == C_MEM_GETDIFFICULTY) {
		bool setting[12] = {0};
		ReadProcessMemory(phandle, (LPVOID)offset, &setting, 12, &stBytes);

		QWrite("[");

		for (int i=0; i<12; i++)
			QWritef("%s%s", i!=0 ? "," : "", getBool(setting[i]));

		QWrite("]");
	} else {
		// Change twelve bytes one by one
		int i      = 0;
		size_t pos = 0;
		String item;

		while ((item = String_tokenize(argument[4], ",", pos, OPTION_TRIM_SQUARE_BRACKETS)).length>0  &&  i<12) {
			int setting = -1;

			if (strcmpi(item.text,"false")==0  ||  strcmpi(item.text,"0")==0) 
				setting = 0;
			
			if (strcmpi(item.text,"true")==0  ||  strcmpi(item.text,"1")==0) 
				setting = 1;

			if (setting >= 0) {
				int pos  = veteran ? RESTORE_VETERAN : RESTORE_CADET;
				int pos2 = veteran ? BYTE_VETERAN    : BYTE_CADET;

				if ((single_player || global.is_server)  &&  !global.restore_memory[pos+i]) {
					global.restore_memory[pos+i] = 1;
					ReadProcessMemory(phandle,(LPVOID)(offset+i), &global.restore_byte[pos2+i], 1, &stBytes);
				}

				WriteProcessMemory(phandle, (LPVOID)(offset+i), &setting,  1, &stBytes);
			}

			i++;
		}
	}
}
break;






case C_MEM_SETRADIOBOX:
{ // Show hide radio options

	if (argument_num < 3) 
		break;

	int base     = 0;
	int	pointer  = 0;
	int pointer2 = 0;
	bool value   = String_bool(argument[2]);

	switch(global_exe_version[global.exe_index]) {
		case VER_196 : base=0x79F8D0; break;
		case VER_199 : base=0x78E9C8; break;
	}
	
	if (base) {
		ReadProcessMemory(phandle, (LPVOID)base,			&pointer,  4, &stBytes);
		ReadProcessMemory(phandle, (LPVOID)(pointer+8),		&pointer2, 4, &stBytes);
		WriteProcessMemory(phandle,(LPVOID)(pointer2+0x2D4),&value,	   1, &stBytes);
	}
}
break;






case C_MEM_GETSPEEDKEY:
{ // Get forward/backward movement

	bool rawMode  = argument_num>2 && strcmpi(argument[2].text,"raw")==0; // Optional mode activated by argument
	int offset[]  = {0x0, 0x8, 0x4, 0xC};	// In order: fast (E), forward (W), slow (Q), reverse (S)
	int	weight[]  = {3,2,1,-2};
	int	max_loops = sizeof(offset)/sizeof(offset[0]);
	int	speed     = 0;
	int quantity  = 0;
	int current   = 0;
	int	base      = 0;

	switch(global_exe_version[global.exe_index]) {
		case VER_196 : base=0x79E9C2; break;
		case VER_199 : base=0x78DABA; break;
	}
		
	// Read those four values
	for (int i=0; i<max_loops && base; i++) {
		ReadProcessMemory(phandle, (LPVOID)(base+offset[i]), &current, 2, &stBytes);

		switch(current) {
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
		}

		speed += weight[i] * quantity;		// Sum up

		// if optional mode then save values that we've read to array
		offset[i] = current;
	}


	// For optional mode return array
	if (rawMode) 
		QWrite("[\"");


	// Convert number to string
	if (speed >= 3) 
		QWrite("fast"); 
	else
		if (speed == 2) 
			QWrite("forward"); 
		else
			if (speed == 1) 
				QWrite("slow"); 
			else
				if (speed == 0) 
					QWrite("stop"); 
				else
					if (speed < 0) 
						QWrite("reverse");


	// For optional mode print out values (that we've saved) in array
	if (rawMode) {
		QWrite("\"");

		for (i=0; i<max_loops; i++)
			QWritef(",%d", offset[i]);

		QWrite("]");
	}
}
break;






case C_MEM_SETSPEEDKEY:
{ // Set forward/backward movement

	if (argument_num < 3) 
		break;

	int offset[]  = {0x0, 0x8, 0x4, 0xC};
	int	max_loops = sizeof(offset)/sizeof(offset[0]);
	int	base      = 0;
	int	val       = 16256;
	int	i         = -1;

	switch(global_exe_version[global.exe_index]) {
		case VER_196 : base=0x79E9C2; break;
		case VER_199 : base=0x78DABA; break;
	}
	
	if (base) {
		// String to number
		if (strcmpi(argument[2].text,"fast") == 0) 
			i = 0;
		else 
			if (strcmpi(argument[2].text,"slow") == 0) 
				i = 2;
			else 
				if (strcmpi(argument[2].text,"forward") == 0) 
					i = 1;
				else 
					if (strcmpi(argument[2].text,"reverse") == 0) 
						i = 3;


		// For a particular speed change one of the offsets
		if (i >= 0)
			WriteProcessMemory(phandle, (LPVOID)(base+offset[i]), &val, 2, &stBytes);


		// For "stop" set all of them to zero
		if (strcmpi(argument[2].text,"stop") == 0)
			for (i=0; i<max_loops; i++) {
				val = 0;
				WriteProcessMemory(phandle, (LPVOID)(base+offset[i]), &val, 2, &stBytes);
			}
	}
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

	int pointer[4]      = {0};
	int	modif[3]        = {0x784, 0x8, 0x4CE};
	short int values[3] = {0,0,0};
	int	max_loops       = sizeof(pointer) / sizeof(pointer[0]) - 1;

	switch(global_exe_version[global.exe_index]) {
		case VER_196 : pointer[0]=0x7B4028; break;
		case VER_199 : pointer[0]=0x7A3128; modif[2]+=10; break;
	}


	if (pointer[0]) {
		// Path to the offset
		for (int i=0; i<max_loops; i++) {
			ReadProcessMemory(phandle, (LPVOID)pointer[i], &pointer[i+1], 4, &stBytes);
			pointer[i+1] = pointer[i+1] +  modif[i];
		}

		// Read seats
		for (int j=0; j<3; j++) {
			ReadProcessMemory(phandle, (LPVOID)(pointer[i]+4*j), &values[j], 2, &stBytes);

			if (values[j]!=0  &&  values[j]!=16256) 
				values[j] = 0;
			else 
				if (values[j] == 16256) 
					values[j] = 1;
				else 
					if (values[j]==0) 
						values[j] = 2;
		}
	}


	// Parameter specified - return specific seat
	if (argument_num >= 3) {
		int j = 0;
		if (strcmpi(argument[2].text,"commander") == 0) j = 0;
		if (strcmpi(argument[2].text,"driver") == 0)    j = 1;
		if (strcmpi(argument[2].text,"gunner") == 0)    j = 2;

		QWritef("%d", values[j]);
	} else
		// No parameters - return array with all seats
		QWritef("[%d,%d,%d]", values[1],values[2],values[0]);
}
break;






case C_MEM_SETPLAYERHATCH:
{ // Turn in / out player

	if (argument_num < 3) 
		break;

	int pointer[4] = {0};
	int modif[3]   = {0x784, 0x8, 0x0};
	int max_loops  = sizeof(pointer) / sizeof(pointer[0]) - 1;

	if (strcmpi(argument[2].text,"driver") == 0) 
		modif[2] = 0x4D2;

	if (strcmpi(argument[2].text,"gunner") == 0) 
		modif[2] = 0x4D6;

	if (strcmpi(argument[2].text,"commander") == 0) 
		modif[2] = 0x4CE;

	if (modif[2] == 0) 
		break;


	switch(global_exe_version[global.exe_index]) {
		case VER_196 : pointer[0]=0x7B4028; break;
		case VER_199 : pointer[0]=0x7A3128; modif[2]+=10; break;
	}

	if (pointer[0]) {
		for (int i=0; i<max_loops; i++) {
			ReadProcessMemory(phandle, (LPVOID)pointer[i], &pointer[i+1], 4, &stBytes);
			pointer[i+1] = pointer[i+1] +  modif[i];
		}

		short int value = -1;
		ReadProcessMemory(phandle, (LPVOID)pointer[i], &value, 2, &stBytes);

		if (value!=0  &&  value!=16256) // if player isn't in a vehicle
			break;	


		// If passed an extra argument - change to specific value
		if (argument_num > 3) {
			if (strcmpi(argument[3].text,"true") == 0) 
				value = 0; 
			else 
				value = 16256;
		// Default - just reverse
		} else
			if (value == 0) 
				value = 16256; 
			else 
				value = 0;


		WriteProcessMemory(phandle, (LPVOID)pointer[i], &value, 2, &stBytes);
	}
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

	int pointer[4] = {0};
	int	modif[3]   = {0x0, 0x8, 0x740};
	int	max_loops  = sizeof(pointer) / sizeof(pointer[0]) - 1;
	int	id         = -1;
	float pos      = 0;

	switch(global_exe_version[global.exe_index]) {
		case VER_196 : pointer[0]=0x786CA0; break;
		case VER_199 : pointer[0]=0x775D88; modif[2]=0x750; break;
	}

	if (pointer[0]) {
		for (int i=0; i<max_loops; i++) {
			ReadProcessMemory(phandle, (LPVOID)pointer[i], &pointer[i+1], 4, &stBytes);
			pointer[i+1] = pointer[i+1] +  modif[i];		
		}

		ReadProcessMemory(phandle, (LPVOID)pointer[max_loops], &id, 4, &stBytes);
		ReadProcessMemory(phandle, (LPVOID)(pointer[max_loops]+4), &pos, 4, &stBytes);
	}

	QWritef("[%.6f,%d]", pos,id);
}
break;






case C_MEM_SETPLAYERLADDER:
{ // Change ladder and/or position on it

	if (argument_num < 3) {
		QWrite("ERROR: Not enough parameters"); 
		break;
	}

	int pointer[4] = {0};
	int	modif[3]   = {0x0, 0x8, 0x740};
	int	max_loops  = sizeof(pointer) / sizeof(pointer[0]) - 1;

	switch(global_exe_version[global.exe_index]) {
		case VER_196 : pointer[0]=0x786CA0; break;
		case VER_199 : pointer[0]=0x775D88; modif[2]=0x750; break;
	}

	for (int i=0; i<max_loops; i++) {
		ReadProcessMemory(phandle, (LPVOID)pointer[i], &pointer[i+1], 4, &stBytes);
		pointer[i+1] = pointer[i+1] +  modif[i];
	}


	if (pointer[0]) {
		// Change position on the ladder
		float pos = (float)atof(argument[2].text);

		if (strcmp(argument[2].text,"-") != 0)
			WriteProcessMemory(phandle, (LPVOID)(pointer[max_loops]+4), &pos, 4, &stBytes);


		// Change ladder
		if (argument_num > 3) {
			if (strcmp(argument[3].text,"-") == 0) 
				break;

			int currID	= -1;
			int ID		= atoi(argument[3].text);

			// Only if player is already on a ladder
			ReadProcessMemory(phandle, (LPVOID)pointer[max_loops], &currID, 4, &stBytes);

			if (currID>=0  &&  ID>=0) 
				WriteProcessMemory(phandle, (LPVOID)pointer[max_loops], &ID, 4, &stBytes);
		}
	}
}
break;






case C_MEM_MASTERSERVER:
{ // Get or set address for the browser

	if (argument_num < 3) {
		QWrite("ERROR: Not enough parameters"); 
		break;
	}

	char serv1[64]   = "";
	char serv2[19]   = "";
	int master_base1 = 0;
	int	master_base2 = 0;
	int	inMenuOff    = 0;
	int inMenu       = 0;

	switch(global_exe_version[global.exe_index]) {
		case VER_196 : master_base1=0x76EBC0; master_base2=0x775F58; inMenuOff=0x75E254; break;
		case VER_199 : master_base1=0x756530; master_base2=0x75D7F0; inMenuOff=0x74B58C; break;
		case VER_201 : master_base1=global.exe_address+0x6C9298; master_base2=global.exe_address+0x6C9560; break;
	}

	if (master_base1 == 0) {
		QWrite("false");
		break;
	}

	// Optional mode - read instead of writing
	if (argument_num==3  &&  strcmpi(argument[2].text,"get")==0) {
		ReadProcessMemory(phandle, (LPVOID)master_base1, &serv1, 64, &stBytes);
		ReadProcessMemory(phandle, (LPVOID)master_base2, &serv2, 19, &stBytes);
		
		QWritef("[\"%s\",\"%s\"]", serv1, serv2);
		break;
	}

	// Is it main menu
	if (inMenu != 0) {
		ReadProcessMemory(phandle, (LPVOID)inMenuOff, &inMenu, 4, &stBytes);

		if (!inMenu) {
			QWrite("false"); 
			break;
		}
	}

	// Change addresses
	strncpy(serv1, String_trim_quotes(argument[3]), 63);

	if (argument_num > 4) 
		strncpy(serv2, String_trim_quotes(argument[4]), 18);

	if (strcmp(serv1,"") != 0)
		WriteProcessMemory(phandle, (LPVOID)master_base1, &serv1, 64, &stBytes);

	if (strcmp(serv2,"") != 0)
		WriteProcessMemory(phandle, (LPVOID)master_base2, &serv2, 19, &stBytes);	

	QWrite("true");
}
break;






case C_MEM_MISSIONINFO:
{ // Get mission name

	bool add_comma      = false;
	char buffer[256]    = "";
	char *buffer_ptr    = buffer;
	const int base_size = 7;
	int base[base_size] = {0};
	int	pointer         = 0;

	//0x7DD0E0				mission name
	//0x7DD130				world name
	//[0x7DD180] + 0x8		pbo name
	//[0x78324C] + 0x114	briefing name 1
	//[0x786880] + 0x8		briefing name 2
	//[0x786884] + 0x8		briefing desc
	//[0x786C30] + 0x8		campaign name

	switch(global_exe_version[global.exe_index]) {
		case VER_196 : 
			base[0] = 0x7DD0E0;
			base[1] = 0x7DD130;
			base[2] = 0x7DD180;
			base[3] = 0x78324C;
			base[4] = 0x786880;
			base[5] = 0x786884;
			base[6] = 0x786C30;
			break;

		case VER_199 : 
			base[0] = 0x7CC0A0;
			base[1] = 0x7CC0F0;
			base[2] = 0x7CC140;
			base[3] = 0x77233C;
			base[4] = 0x775968;
			base[5] = 0x77596C;
			base[6] = 0x775D18;
			break;

		case VER_196_SERVER : 
			base[0] = 0x75A398;
			base[1] = 0x75A3E8;
			base[2] = 0x75A438;
			base[3] = 0x7030AC;
			base[4] = 0x7066D8;
			base[5] = 0x7066DC;
			break;

		case VER_199_SERVER : 
			base[0] = 0x75A428;
			base[1] = 0x75A478;
			base[2] = 0x75A4C8;
			base[3] = 0x7030FC;
			base[4] = 0x706728;
			base[5] = 0x70672C;
			break;
	}

	int	modif[]= {
		0,
		0,
		0x8,
		0x114,
		0x8,
		0x8,
		0x8
	};



	QWrite("[");

	// Read from all the addresses
	for (int i=0; i<base_size; i++) {
		if (base[i]) {
			// First two - just read string
			if (i <= 1)
				ReadProcessMemory(phandle, (LPVOID)base[i], &buffer, 255, &stBytes);
			else
				// Otherwise read pointer
				if (!global.is_server  ||  (global.is_server && i<base_size-1)) {
					ReadProcessMemory(phandle, (LPVOID)base[i], &pointer, 4, &stBytes);	

					if (pointer != 0) 
						ReadProcessMemory(phandle, (LPVOID)(pointer+modif[i]), &buffer, 255, &stBytes);

					// if briefing 1 is available then skip briefing 2 (next value)
					if (i == 3) {
						if (pointer == 0)
							continue;
						else
							i++;
					}
				}
		}

		if (add_comma)
			QWrite(",");
		else
			add_comma = true;

		// If a name from the stringtable
		if (buffer[0]=='@'  ||  strncmpi(buffer,"$STR",4)==0) {
			buffer_ptr = buffer + 1;
			QWrite("localize ");
		} else
			buffer_ptr = buffer;

		QWrite("\"");
		QWriteq(buffer_ptr);
		QWrite("\"");
	}

	QWrite("]");
}
break;






case C_MEM_BULLETS:
{ // Get/Set bullets properties

	DWORD old               = 0;
	bool add_comma          = false;
	const int array_size    = 10;
	bool set[array_size]    = {0};
	float value[array_size] = {0};
	int offset[array_size]  = {0};
	unsigned int valid_arguments[array_size] = {
		NAMED_ARG_GRAVACC,
		NAMED_ARG_BULLET,
		NAMED_ARG_SHELL,
		NAMED_ARG_ROCKET,
		NAMED_ARG_BOMB,
		NAMED_ARG_SMOKE,
		NAMED_ARG_FLARE,
		NAMED_ARG_FLAREDURATION,
		NAMED_ARG_PIPEBOMB,
		NAMED_ARG_TIMEBOMB
	};
	//gravity acceleration	9.8065996170043945
	//bullet lifetime		3
	//shell  lifetime		20
	//rocket lifetime		10
	//bomb lifetime			120
	//smoke lifetime		60
	//flare lifetime		17
	//flare duration		15
	//pipebomb lifetime		3.402823466E38 (7F7FFFFF)
	//timebomb lifetime		20

	switch(global_exe_version[global.exe_index]) {
		case VER_196 : 
			offset[0] = 0x71D518;
			offset[1] = 0x5F1570;
			offset[2] = 0x5F16D2;
			offset[3] = 0x5F1527;
			offset[4] = 0x5F147B;
			offset[5] = 0x5F178F;
			offset[6] = 0x5F12AD;
			offset[7] = 0x7137A0;
			offset[8] = 0x5F14BF;
			offset[9] = 0x5F1818;
			break;

		case VER_199 : 
			offset[0] = 0x710520;
			offset[1] = 0x5F818C;
			offset[2] = 0x5F7ACB;
			offset[3] = 0x4857B3;
			offset[4] = 0x487867;
			offset[5] = 0x487986;
			offset[6] = 0x5F7D5D;
			offset[7] = 0x7067A0;
			offset[8] = 0x485820;
			offset[9] = 0x5F789B;
			break;

		case VER_196_SERVER : 
			offset[0] = 0x6ABDE8;
			offset[1] = 0x5374B8;
			offset[2] = 0x534F9C;
			offset[3] = 0x533A91;
			offset[4] = 0x5357C7;
			offset[5] = 0x5371A1;
			offset[6] = 0x536D09;
			offset[7] = 0x6A66C0;
			offset[8] = 0x5345F9;
			offset[9] = 0x5347F7;
			break;

		case VER_199_SERVER : 
			offset[0] = 0x6ABDA8;
			offset[1] = 0x537671;
			offset[2] = 0x53517D;
			offset[3] = 0x533C4A;
			offset[4] = 0x5359A8;
			offset[5] = 0x53735A;
			offset[6] = 0x536EEA;
			offset[7] = 0x6A66C0;
			offset[8] = 0x5347DA;
			offset[9] = 0x5349D8;
			break;
	}


	// If name matches then queue for change
	for (size_t i=2; i<argument_num; i+=2)
		for (int j=0; j<array_size; j++)
			if (argument_hash[i] == valid_arguments[j]) {
				set[j]   = 1;
				value[j] = (float)atof(argument[i+1].text);
				break;
			}


	// Write / Read values
	QWrite("[");

	for (i=0; i<array_size; i++) {
		if ((i==0 || i==7)  &&  set[i])
			VirtualProtectEx(phandle, (LPVOID)offset[i], 4, PAGE_EXECUTE_READWRITE, &old);
		
		if (set[i]) {
			if (!global.restore_memory[RESTORE_BULLETS+i]) {
				global.restore_memory[RESTORE_BULLETS+i] = 1;
				ReadProcessMemory(phandle,(LPVOID)offset[i], &global.restore_float[FLOAT_BULLETS+i], 4, &stBytes);
			}
			
			WriteProcessMemory(phandle, (LPVOID)offset[i], &value[i], 4, &stBytes);
		}

		ReadProcessMemory(phandle, (LPVOID)offset[i], &value[i], 4, &stBytes);

		if (add_comma) 
			QWrite(","); 
		else 
			add_comma = true;

		QWritef("%f", value[i]);
	}

	QWrite("]");
}
break;








case C_MEM_SETWEATHER:
{ // Change weather values in the memory

	int valueINT[3]    = {0};
	float windSpeed[3] = {0};
	float gust[3]      = {0};

	const int array_size       = 24;
	float valueFLT[array_size] = {0};
	bool set[array_size]       = {0};
	unsigned int valid_arguments[array_size] = {
		NAMED_ARG_ACTUALOVERCAST,
		NAMED_ARG_WANTEDOVERCAST,
		NAMED_ARG_SPEEDOVERCAST,
		NAMED_ARG_ACTUALFOG,
		NAMED_ARG_WANTEDFOG,
		NAMED_ARG_SPEEDFOG,
		NAMED_ARG_WEATHERTIME,
		NAMED_ARG_NEXTWEATHERCHANGE,
		NAMED_ARG_CLOUDSPOS,
		NAMED_ARG_CLOUDSALPHA,
		NAMED_ARG_CLOUDSBRIGHTNESS,
		NAMED_ARG_CLOUDSSPEED,
		NAMED_ARG_SKYTHROUGH,
		NAMED_ARG_RAINDENSITY,
		NAMED_ARG_RAINDENSITYWANTED,
		NAMED_ARG_RAINDENSITYSPEED,
		NAMED_ARG_THUNDERBOLTTIME,		//16 - int
		NAMED_ARG_WINDSPEED,			//17 - float array
		NAMED_ARG_LASTWINDSPEEDCHANGE,	//18 - int
		NAMED_ARG_GUST,					//19 - float array
		NAMED_ARG_GUSTUNTIL,			//20 - int
		NAMED_ARG_SEAWAVESPEED,
		NAMED_ARG_MAXTIDE,
		NAMED_ARG_MAXWAVE
	};


	// Parse input
	for (size_t i=2; i<argument_num; i+=2) {
		for (int j=0; j<array_size; j++) {
			if (argument_hash[i] == valid_arguments[j]) {
				set[j] = 1;

				// floats
				if (j<16  ||  j>20)
					valueFLT[j] = (float)atof(argument[i+1].text);
				
				// integers
				if (j == 16)
					valueINT[0] = atoi(argument[i+1].text);

				if (j == 18)
					valueINT[1] = atoi(argument[i+1].text);

				if (j == 20)
					valueINT[2] = atoi(argument[i+1].text);

				// float array
				if (j==17  ||  j==19) {
					int index    = 0;
					char tmp[64] = "";
					int tmp_len  = 0;

					for (size_t k=0; k<argument[i+1].length; k++) {
						if (argument[i+1].text[k] == '[')
							continue;						

						if (argument[i+1].text[k]==','  ||  argument[i+1].text[k]==']') {
							tmp[tmp_len] = '\0';

							if (j == 17)
								windSpeed[index] = (float)atof(tmp);

							if (j == 19)
								gust[index] = (float)atof(tmp);

							tmp_len = 0;
							index++;
						} else
							tmp[tmp_len++] = argument[i+1].text[k];
					}
				}

				break;
			}
		}
	}


	int setIndex = 0;
	int base     = 0;
	int pointer  = 0;

	switch(global_exe_version[global.exe_index]) {
		case VER_196        : base=0x79F8D0; break;
		case VER_199        : base=0x78E9C8; break;
		case VER_196_SERVER : base=0x71F738; break;
		case VER_199_SERVER : base=0x71F788; break;
	}
	

	// Find beginning of the weather values
	if (base) {
		ReadProcessMemory(phandle, (LPVOID)base, &pointer,  4, &stBytes);
		pointer += 0x7C4;

		for (i=0;  i<=7;  i++, setIndex++) {
			if (set[setIndex])
				WriteProcessMemory(phandle, (LPVOID)pointer, &valueFLT[setIndex], 4, &stBytes);

			// skip latitude and longitude
			if (i == 5)
				pointer += 12;
			else
				pointer += 4;
		}
	}


	// Get the other set of weather values
	switch(global_exe_version[global.exe_index]) {
		case VER_196        : base=0x7B3ACC; break;
		case VER_199        : base=0x7A2C0C; break;
		case VER_196_SERVER : base=0x73392C; break;
		case VER_199_SERVER : base=0x7339C4; break;
		default             : base=0;
	}

	if (base) {
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
	}


	// Tide and wave
	DWORD old = 0;

	switch(global_exe_version[global.exe_index]) {
		case VER_196        : base=0x72F8E4; break;
		case VER_199        : base=0x72295C; break;
		case VER_196_SERVER : base=0x6BE184; break;
		case VER_199_SERVER : base=0x6BE144; break;
		default             : base=0;
	}

	if (base) {
		if (set[22]) {
			if (!global.restore_memory[RESTORE_TIDE]) {
				global.restore_memory[RESTORE_TIDE] = 1;
				ReadProcessMemory(phandle,(LPVOID)base, &global.restore_float[FLOAT_TIDE], 4, &stBytes);
			}

			VirtualProtectEx(phandle, (LPVOID)base, 4, PAGE_EXECUTE_READWRITE, &old),
			WriteProcessMemory(phandle, (LPVOID)base, &valueFLT[22], 4, &stBytes);
		}

		if (set[23]) {
			if (!global.restore_memory[RESTORE_WAVE]) {
				global.restore_memory[RESTORE_WAVE] = 1;
				ReadProcessMemory(phandle,(LPVOID)(base+4), &global.restore_float[FLOAT_WAVE], 4, &stBytes);
			}

			VirtualProtectEx(phandle, (LPVOID)(base+4), 4, PAGE_EXECUTE_READWRITE, &old),
			WriteProcessMemory(phandle, (LPVOID)(base+4), &valueFLT[23], 4, &stBytes);
		}
	}
}
break;








case C_MEM_SETCAM:
{ // Get external camera offset

	bool multiplayer = false;

	// Parse arguments
	for (size_t i=2; i<argument_num; i+=2) {
		switch (argument_hash[i]) {
			case NAMED_ARG_MP : 
				multiplayer = String_bool(argument[i+1]);
				break;

			case NAMED_ARG_EXTCAMERAPOSITION : {
				// float array
				int index      = 0;
				int tmp_len    = 0;
				char tmp[64]   = "";
				float extcam[] = {0,0,0};

				for (size_t j=0; j<argument[i+1].length; j++) {
					if (argument[i+1].text[j] == '[')
						continue;			

					if (argument[i+1].text[j]==','  ||  argument[i+1].text[j]==']') {
						tmp[tmp_len]  = '\0';
						extcam[index] = (float)atof(tmp);
						tmp_len       = 0;
						index++;
					} else
						tmp[tmp_len++] = argument[i+1].text[j];
				}

				int pointer[]    = {0,0,0,0};
				int	modif[]      = {0,0,0};
				size_t max_loops = (sizeof(pointer) / sizeof(pointer[0])) - 1;
				
				switch(global_exe_version[global.exe_index]) {
					case VER_196 : 
						pointer[0] = multiplayer ? 0x0 : 0x7B4030;
						modif[0]   = multiplayer ? 0x0 : 0x7C8;
						modif[1]   = multiplayer ? 0x0 : 0x4A8;
						modif[2]   = multiplayer ? 0x0 : 0x69C;
						break;

					case VER_199 : 
						pointer[0] = multiplayer ? 0x38E93C : 0x7A3128;
						modif[0]   = multiplayer ? 0x94     : 0xAC;
						modif[1]   = multiplayer ? 0x4A8    : 0x31C;
						modif[2]   = 0x6A0;
						break;
				}

				if (pointer[0]) {
					for (j=0; j<max_loops; j++) {
						ReadProcessMemory(phandle, (LPVOID)pointer[j], &pointer[j+1], 4, &stBytes);
						pointer[j+1] = pointer[j+1] + modif[j];
					}

					if (!global.restore_memory[RESTORE_EXTCAMPOS]) {
						global.restore_memory[RESTORE_EXTCAMPOS] = 1;
						ReadProcessMemory(phandle,(LPVOID)(pointer[max_loops]),	  &global.restore_float[FLOAT_EXTCAMX], 4, &stBytes);
						ReadProcessMemory(phandle,(LPVOID)(pointer[max_loops]+4), &global.restore_float[FLOAT_EXTCAMZ], 4, &stBytes);
						ReadProcessMemory(phandle,(LPVOID)(pointer[max_loops]+8), &global.restore_float[FLOAT_EXTCAMY], 4, &stBytes);
						global.extCamOffset = pointer[max_loops];
					}

					WriteProcessMemory(phandle, (LPVOID)(pointer[max_loops]),	&extcam[0], 4, &stBytes);
					WriteProcessMemory(phandle, (LPVOID)(pointer[max_loops]+4), &extcam[2], 4, &stBytes);
					WriteProcessMemory(phandle, (LPVOID)(pointer[max_loops]+8), &extcam[1], 4, &stBytes);
				}
			} break;
		}
	}
}
break;






case C_MEM_HUD:
{
	int currentINT[ARRAY_SIZE] = {0};
	float current[ARRAY_SIZE]  = {0};
	bool is_custom[ARRAY_SIZE] = {0};
	unsigned int valid_arguments[ARRAY_SIZE] = {
		NAMED_ARG_ACTION_X,
		NAMED_ARG_ACTION_Y,
		NAMED_ARG_ACTION_W,
		NAMED_ARG_ACTION_H,
		NAMED_ARG_ACTION_ROWS,
		NAMED_ARG_ACTION_COLORBACK,
		NAMED_ARG_ACTION_COLORTEXT,
		NAMED_ARG_ACTION_COLORSEL,
		NAMED_ARG_ACTION_FONT,
		NAMED_ARG_ACTION_SIZE,
		NAMED_ARG_RADIOMENU_X,
		NAMED_ARG_RADIOMENU_Y,
		NAMED_ARG_RADIOMENU_W,
		NAMED_ARG_RADIOMENU_H,
		NAMED_ARG_TANK_X,
		NAMED_ARG_TANK_Y,
		NAMED_ARG_TANK_W,
		NAMED_ARG_TANK_H,
		NAMED_ARG_RADAR_X,
		NAMED_ARG_RADAR_Y,
		NAMED_ARG_RADAR_W,
		NAMED_ARG_RADAR_H,
		NAMED_ARG_COMPASS_X,
		NAMED_ARG_COMPASS_Y,
		NAMED_ARG_COMPASS_W,
		NAMED_ARG_COMPASS_H,
		NAMED_ARG_HINT_X,
		NAMED_ARG_HINT_Y,
		NAMED_ARG_HINT_W,
		NAMED_ARG_HINT_H,
		NAMED_ARG_LEADER_X,
		NAMED_ARG_LEADER_Y,
		NAMED_ARG_LEADER_W,
		NAMED_ARG_LEADER_H,
		NAMED_ARG_GROUPDIR_X,
		NAMED_ARG_GROUPDIR_Y,
		NAMED_ARG_GROUPDIR_W,
		NAMED_ARG_GROUPDIR_H,
		NAMED_ARG_CHAT_X,
		NAMED_ARG_CHAT_Y,
		NAMED_ARG_CHAT_W,
		NAMED_ARG_CHAT_H,
		NAMED_ARG_CHAT_ROWS,
		NAMED_ARG_CHAT_COLORGLOBAL,
		NAMED_ARG_CHAT_COLORSIDE,
		NAMED_ARG_CHAT_COLORTEAM,
		NAMED_ARG_CHAT_COLORVEHICLE,
		NAMED_ARG_CHAT_COLORDIRECT,
		NAMED_ARG_CHAT_COLORBACK,
		NAMED_ARG_CHAT_FONT,
		NAMED_ARG_CHAT_SIZE,
		NAMED_ARG_CHAT_ENABLE
	};

	// If name matches then queue for change
	for (size_t i=2; i<argument_num; i+=2) {
		for (int j=0; j<ARRAY_SIZE; j++) {
			if (argument_hash[i] == valid_arguments[j]) {
				is_custom[j] = 1;

				if (IsNumberInArray(j, hud_int_list, sizeof(hud_int_list)/sizeof(hud_int_list[0]))) {
					if (IsNumberInArray(j, hud_color_list, sizeof(hud_color_list)/sizeof(hud_color_list[0]))) {
						int index              = 0;
						unsigned char color[4] = {0,0,0,0};
						String number          = {NULL, 0};
						size_t subvalue_pos    = 0;

						while ((number = String_tokenize(argument[i+1], ",;", subvalue_pos, OPTION_TRIM_SQUARE_BRACKETS)).length>0  &&  index<4) {
							String_trim_space(number);
							color[index++] = (unsigned char)(atof(number.text) * 255);
						}

						currentINT[j] = ((color[3] << 24) | (color[0] << 16) | (color[1] << 8) | color[2]);
					} else
						currentINT[j] = atoi(argument[i+1].text);
				} else
					current[j] = (float)atof(argument[i+1].text);

				break;
			}
		}
	}


	// Determine pointer address
	int ui_base   = 0;
	int chat_base = 0;
	int pointer   = 0;

	switch(global_exe_version[global.exe_index]) {
		case VER_196 : ui_base=0x79F8D0; break;
		case VER_199 : ui_base=0x78E9C8; break;
		case VER_201 : ui_base=global.exe_address+0x6D8240; break;
	}

	ReadProcessMemory(phandle, (LPVOID)(ui_base+0x0), &ui_base, 4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)(ui_base+0x8), &ui_base, 4, &stBytes);

	if (ui_base == 0)
		break;

	// Chat has a different address
	switch(global_exe_version[global.exe_index]) {
		case VER_196 : chat_base=0x7831B0; break;
		case VER_199 : chat_base=0x7722A0; break;
		case VER_201 : chat_base=global.exe_address+0x6FFCC0; break;
	}

	
	// Read all values from memory and change the ones user selected
	QWrite("[");

	for (i=0;  i<sizeof(hud_offset)/sizeof(hud_offset[0]);  i++) {
		if (i >= CHAT_X)
			pointer = chat_base;
		else
			pointer = ui_base;

		int is_int = IsNumberInArray(i, hud_int_list, sizeof(hud_int_list)/sizeof(hud_int_list[0]));

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

			if (IsNumberInArray(i, hud_color_list, sizeof(hud_color_list)/sizeof(hud_color_list[0]))) {
				unsigned char alpha = (currentINT[i] >> 24) & 0xFF;
				unsigned char red   = (currentINT[i] >> 16) & 0xFF;
				unsigned char green = (currentINT[i] >> 8)  & 0xFF;
				unsigned char blue  = currentINT[i] & 0xFF;
				QWritef("]+[[%.6f,%.6f,%.6f,%.6f]", (float)red/255,(float)green/255,(float)blue/255,(float)alpha/255);
			} else
				QWritef("]+[%d", currentINT[i]);
		} else {
			ReadProcessMemory(phandle, (LPVOID)(pointer+hud_offset[i]), &current[i], 4, &stBytes);		
			QWritef("]+[%f", current[i]);
		}
	}

	QWrite("]");
}
break;







// -----------------------------------------------------------------------------------
case C_MEM_MULTI:		// reuses the same cases
	QWrite("[");

case C_MEM_GETCAM:
{ // Get camera values from the memory
	
	float sin              = 0;
	float cos              = 0;
	float dir              = 0;
	float pitch            = 0;
	int plr                = 1;
	int base_plr           = 0;
	const int base_size    = 7;
	int base[base_size]    = {0};
	float value[base_size] = {0};

	enum {
		COS,
		PITCH,
		POSX,
		POSZ,
		POSY,
		FOV,
		SIN,
		PLR
	};

	switch(global_exe_version[global.exe_index]) {
		case VER_196 : 
			base[0]  = 0x788434;
			base[1]  = 0x788450;
			base[2]  = 0x788458;
			base[3]  = 0x78845C;
			base[4]  = 0x788460;
			base[5]  = 0x7884F8;
			base[6]  = 0x78864C;
			base_plr = 0x79DFCC;
			break;

		case VER_199 : 
			base[0]  = 0x77751C;
			base[1]  = 0x777538;
			base[2]  = 0x777540;
			base[3]  = 0x777544;
			base[4]  = 0x777548;
			base[5]  = 0x7775E0;
			base[6]  = 0x777614;
			base_plr = 0x78D0C3;	// if this one fails then C4
			break;
	}

	for (int i=0; i<base_size; i++)
		if (base[i])
			ReadProcessMemory(phandle, (LPVOID)base[i], &value[i], 4, &stBytes);

	if (base_plr)
		ReadProcessMemory(phandle, (LPVOID)base_plr, &plr, 1, &stBytes);


	double result = rad2deg(acos(cos));	// arccosine dir; rad to deg
	if (sin < 0) 
		result = 360 - result;			// format value
	dir = float(result);				// from double to float

	result = rad2deg(asin(pitch));
	pitch  = float(result);


	// get ext cam pos
	float extcam[3] = {0,0,0};
	int pointer[]   = {0};
	int	modif[]	    = {0x5C, 0x69C};
	int	max_loops   = (sizeof(pointer) / sizeof(pointer[0])) - 1;

	switch(global_exe_version[global.exe_index]) {
		case VER_196 : pointer[0]=0x7894A0; break;
		case VER_199 : pointer[0]=0x778590; modif[1]=0x6A0; break;
	}

	if (pointer[0]) {
		for (int i=0; i<max_loops; i++) {
			ReadProcessMemory(phandle, (LPVOID)pointer[i], &pointer[i+1], 4, &stBytes);
			pointer[i+1] = pointer[i+1] + modif[i];
		}

		ReadProcessMemory(phandle, (LPVOID)(pointer[max_loops]+0x0), &extcam[0], 4, &stBytes);
		ReadProcessMemory(phandle, (LPVOID)(pointer[max_loops]+0x4), &extcam[1], 4, &stBytes);
		ReadProcessMemory(phandle, (LPVOID)(pointer[max_loops]+0x8), &extcam[2], 4, &stBytes);
	}


	QWritef("[%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%s,[%.6f,%.6f,%.6f]]", 
		value[POSX], 
		value[POSY], 
		value[POSZ], 
		dir, 
		value[PITCH], 
		value[FOV], 
		getBool(!plr),
		extcam[0],
		extcam[2],
		extcam[1]
	);		
}
if (argument_hash[0] != C_MEM_MULTI) 
	break;
else
	QWrite(",");








case C_MEM_GETMAP:
{ // Get 2D map state from memory

	int base	= 0;
	int	pointer = 0;
	bool is_map = 0;

	switch(global_exe_version[global.exe_index]) {
		case VER_196 : base=0x7B4028; break;
		case VER_199 : base=0x7A3128; break;
		case VER_201 : base=global.exe_address+0x6D7018; break;
	}
	
	if (base) {
		ReadProcessMemory(phandle, (LPVOID)base,			&pointer, 4, &stBytes);
		ReadProcessMemory(phandle, (LPVOID)(pointer+0x7CF), &is_map,  1, &stBytes);
	}

	QWrite(getBool(is_map));
}
if (argument_hash[0] != C_MEM_MULTI) 
	break;
else
	QWrite(",");







case C_MEM_GETNV:
{ // Get nightvision goggles value from the memory

//[[[0x00786CA0]] + 0x8] + 0x6C6

	int pointer[5]	  = {0};
	int	modif[3]	  = {0x0, 0x8, 0x6C6};
	int	bytes_to_read = 4;
	int	max_loops	  = (sizeof(pointer) / sizeof(pointer[0])) - 1;

	switch(global_exe_version[global.exe_index]) {
		case VER_196 : pointer[0]=0x786CA0; break;
		case VER_199 : pointer[0]=0x7A3128; modif[0]=0x78C; modif[2]=0x6D6; break;
	}


	if (pointer[0]) {
		for (int i=0; i<max_loops; i++) {
			if (i == max_loops-1)		// in last iteration read just one byte
				bytes_to_read = 1;

			ReadProcessMemory(phandle, (LPVOID)pointer[i], &pointer[i+1], bytes_to_read, &stBytes);

			if (i < max_loops-1) 
				pointer[i+1] = pointer[i+1] + modif[i];
		}
	}


	QWrite(getBool(pointer[max_loops]));
}
if (argument_hash[0] != C_MEM_MULTI) 
	break;
else
	QWrite(",");







case C_MEM_GETPLAYERVIEW:
{ // Get camera view type from the memory

	int pointer = 0;
	int display = 0;
	int toggle  = 0;
	int *ptr    = &display;
	int	base    = 0;

	switch(global_exe_version[global.exe_index]) {
		case VER_196 : base=0x7B4028; break;
		case VER_199 : base=0x7A3128; break;
	}


	if (base) {
		ReadProcessMemory(phandle, (LPVOID)base,			&pointer, 4, &stBytes);
		ReadProcessMemory(phandle, (LPVOID)(pointer+0x860), &toggle,  1, &stBytes);
		ReadProcessMemory(phandle, (LPVOID)(pointer+0x864), &display, 1, &stBytes);
	}


	QWrite("[");

	for (int i=0; i<2; i++) {
		if (i != 0) {
			QWrite(",");
			ptr = &toggle;
		}

		QWrite("\"");

		switch (*ptr) {
			case 0: QWrite("INTERNAL"); break;
			case 1: QWrite("GUNNER"); break;
			case 2: QWrite("EXTERNAL"); break;
			case 3: QWrite("GROUP"); break;
		}

		QWrite("\"");
	}

	QWrite("]");
}
if (argument_hash[0] != C_MEM_MULTI) 
	break;
else
	QWrite(",");







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
	int pointer[4]  = {0};
	int modif[3]    = {0x38, 0x8, 0x7C};
	int pointer2[4] = {0};
	int modif2[3]   = {0x784, 0x8, 0x474};
	int max_loops   = sizeof(pointer) / sizeof(pointer[0]) - 1;

	switch(global_exe_version[global.exe_index]) {
		case VER_196 : 
			pointer[0]  = 0x7B3ACC;
			pointer2[0] = 0x7B4028;
			break;

		case VER_199 : 
			pointer[0]  = 0x78E9C8;
			pointer2[0] = 0x7A3128; 
			modif[0]    = 0x8;
			modif[1]    = 0x7C;
			modif2[2]   = 0x484;
			break;
	}


	for (int i=0;  i<max_loops && pointer[0];  i++) {
		// There's one less loop in CWA version
		if (global_exe_version[global.exe_index]!=VER_199  ||  (global_exe_version[global.exe_index]==VER_199  &&  i<max_loops-1))	{
			ReadProcessMemory(phandle, (LPVOID)pointer[i], &pointer[i+1], 4, &stBytes);
			pointer[i+1] = pointer[i+1] +  modif[i];
		}
		
		ReadProcessMemory(phandle, (LPVOID)pointer2[i], &pointer2[i+1], 4, &stBytes);
		pointer2[i+1] = pointer2[i+1] +  modif2[i];
	}


	// Read values
	float m_sin  = 0;
	float m_cos  = 0;
	float m_pit  = 0;
	float m_dir  = 0;
	float g_off  = 0;
	float g_pit  = 0;
	float g_vlV  = 0;
	float g_vlH  = 0;
	float g_pit2 = 0;

	if (global_exe_version[global.exe_index] == VER_199) 
		max_loops--;

	ReadProcessMemory(phandle, (LPVOID)pointer[max_loops],     &m_sin, 4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)(pointer[max_loops]+4), &m_pit, 4, &stBytes);
	ReadProcessMemory(phandle, (LPVOID)(pointer[max_loops]+8), &m_cos, 4, &stBytes);

	if (global_exe_version[global.exe_index] == VER_199) 
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

	m_dir  = float(result);
	result = rad2deg(asin(g_off));
	g_off  = float(result*-1);

	if (g_pit != g_pit) 
		g_pit = 0;

	if (g_pit2 != g_pit2) 
		g_pit2 = 0;

	QWritef("[%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f]", m_dir, g_off, m_pit, g_pit, g_vlH, g_vlV, g_pit2);
}
if (argument_hash[0] != C_MEM_MULTI) 
	break;
else
	QWrite(",");







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

	int pointer[5]   = {0};
	int	modif[3]     = {0x788, 0x8, 0x708};
	int	max_loops    = (sizeof(pointer) / sizeof(pointer[0])) - 1; // number of loops = items in the array - 1
	bool restartPath = false;

	switch(global_exe_version[global.exe_index]) {
		case VER_196 : pointer[0]=0x7B4028; break;
		case VER_199 : pointer[0]=0x7A3128; modif[0]=0x78C; modif[2]=0x718; break;
	}
	

	// Loop reading memory
	for (int i=0; i<max_loops; i++) {
		// read 4 times
		ReadProcessMemory(phandle, (LPVOID)pointer[i], &pointer[i+1], 4, &stBytes);

		// alternate path if current failed	
		if (!restartPath  &&  pointer[i+1]==0) {
			i			= -1; 
			restartPath = true;

			switch(global_exe_version[global.exe_index]) {
				case VER_196 : modif[0]=0x784; break;
				case VER_199 : pointer[0]=0x78E9C8; modif[0]=0x7A8; break;
			}

			continue;
		}

		// modify 3 times
		if (i < max_loops-1) 
			pointer[i+1] = pointer[i+1] + modif[i];
	}


	QWritef("%d", pointer[max_loops]);		// last read is the value we want
}
if (argument_hash[0] != C_MEM_MULTI) 
	break;
else
	QWrite(",");







case C_MEM_ISDIALOG:
{ // Return number of dialogs

	int i	 = 0;
	int base = 0;

	switch(global_exe_version[global.exe_index]) {
		case VER_196 : base=0x79E9E0; break;
		case VER_199 : base=0x78DAD8; break;
	}

	if (base)
		ReadProcessMemory(phandle, (LPVOID)base, &i, 4, &stBytes);

	QWritef("%d", i);
}
if (argument_hash[0] != C_MEM_MULTI) 
	break;
else
	QWrite(",");








case C_MEM_GETRADIOBOX:
{ // Is radio options box displayed

	int base	 = 0;
	int	pointer  = 0;
	int pointer2 = 0;
	int value	 = 0;

	switch(global_exe_version[global.exe_index]) {
		case VER_196 : base=0x79F8D0; break;
		case VER_199 : base=0x78E9C8; break;
	}

	if (base) {
		ReadProcessMemory(phandle, (LPVOID)base,			 &pointer,  4, &stBytes);
		ReadProcessMemory(phandle, (LPVOID)(pointer+8),		 &pointer2, 4, &stBytes);
		ReadProcessMemory(phandle, (LPVOID)(pointer2+0x2D4), &value,	1, &stBytes);
	}

	QWrite(getBool(value));
}
if (argument_hash[0] != C_MEM_MULTI) 
	break;
else
	QWrite(",");







case C_MEM_GETWEATHER:
{ // Get weather values from the memory

// [0x78E9C8] + 0x7C4

	float weather[8] = {0};
	int base         = 0;
	int pointer      = 0;
	int	max_loops    = (sizeof(weather) / sizeof(weather[0])) - 1; 

	switch(global_exe_version[global.exe_index]) {
		case VER_196        : base=0x79F8D0; break;
		case VER_199        : base=0x78E9C8; break;
		case VER_196_SERVER : base=0x71F738; break;
		case VER_199_SERVER : base=0x71F788; break;
	}
	
	// Find beginning of the weather values
	if (base) {
		ReadProcessMemory(phandle, (LPVOID)base, &pointer,  4, &stBytes);
		pointer += 0x7C4;
	}

	// Read eight of them one after another
	QWrite("[");

	for (int i=0; i<=max_loops; i++) {
		if (base)
			ReadProcessMemory(phandle, (LPVOID)pointer, &weather[i], 4, &stBytes);

		QWritef("%.6f,", weather[i]);

		// skip latitude and longitude
		if (i == 5)
			pointer += 12;
		else
			pointer += 4;
	}


	// Get mission time
	int missionTime = 0;

	switch(global_exe_version[global.exe_index]) {
		case VER_196        : base=0x7DD028; break;
		case VER_199        : base=0x7CBFE8; break;
		case VER_196_SERVER : base=0x75A2E0; break;
		case VER_199_SERVER : base=0x75A370; break;
		default             : base=0;
	}

	if (base)
		ReadProcessMemory(phandle, (LPVOID)base, &missionTime, 4, &stBytes);

	QWritef("%d,", missionTime);


	// Get the other set of weather values
	float value = 0;
	int jump    = 0;
	int value2  = 0;

	switch(global_exe_version[global.exe_index]) {
		case VER_196        : base=0x7B3ACC; break;
		case VER_199        : base=0x7A2C0C; break;
		case VER_196_SERVER : base=0x73392C; break;
		case VER_199_SERVER : base=0x7339C4; break;
		default             : base=0;
	}

	if (base)
		ReadProcessMemory(phandle, (LPVOID)base, &pointer, 4, &stBytes);

	for (int offset=0x2054C;  offset<=0x2059C;  offset+=4) {
		// skip sky
		if (offset == 0x20550)
			offset += 4;

		// Wind vectors output in a different order
		if (jump==0  &&  (offset==0x20578 || offset==0x20588)) {
			offset += 4;
			jump    = 1;
		}

		if (base) {
			ReadProcessMemory(phandle, (LPVOID)(pointer+offset), &value, 4, &stBytes);
			ReadProcessMemory(phandle, (LPVOID)(pointer+offset), &value2, 4, &stBytes);
		}

		// Wind vectors put in array
		if (offset==0x20574  ||  offset==0x20584)
			QWrite("[");

		// Time is int, everything else is float
		if (offset==0x20570  ||  offset==0x20580  ||  offset==0x20590)
			QWritef("%d", value2);
		else
			QWritef("%.6f", value);
		
		// Close array for wind vector
		if (jump == 2)
			QWrite("]");

		QWrite(",");

		if (jump == 2) {
			offset += 4;
			jump    = 0;
		}

		if (jump == 1) {
			offset -= 8;
			jump    = 2;
		}
	}

	
	// Get tide and wave values
	float maxTide = 0;
	float maxWave = 0;

	switch(global_exe_version[global.exe_index]) {
		case VER_196        : base=0x72F8E4; break;
		case VER_199        : base=0x72295C; break;
		case VER_196_SERVER : base=0x6BE184; break;
		case VER_199_SERVER : base=0x6BE144; break;
		default             : base=0;
	}

	if (base) {
		ReadProcessMemory(phandle, (LPVOID)base,	 &maxTide, 4, &stBytes);
		ReadProcessMemory(phandle, (LPVOID)(base+4), &maxWave, 4, &stBytes);
	}

	QWritef("%.6f,%.6f]", maxTide, maxWave);
}
if (argument_hash[0] == C_MEM_MULTI) 
	QWrite("]");
break;
