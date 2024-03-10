// Function supplied with Fwatch v1.16
// Required to make MainMenu.sqs work

FUNCTION_FIND = {
	if (IS_CWA) then {(_this select 1) find (_this select 0)} else {
		private ["_i","_index"];
		_i=0;
		_index=-1;
		
		{
			if ((_this select 0) == _x) then {_index=_i};
			_i=_i+1
		} forEach (_this select 1);
		
		_index
	}
};



FUNCTION_MODS2STRING = {
	private ["_string", "_i", "_mods", "_separator", "_mod_id", "_mod_name", "_mod_version", "_index2", "_my_version"];
	_mods      = _this select 0;
	_separator = _this select 1;
	_string    = ""; 
	_i         = 0;

	{
		_mod_id      = _x select 0;
		_mod_name    = _x select 1;
		_mod_size    = _x select 4;
		_mod_version = _x select 2;
		_verTXT      = Format ["%1", _mod_version];
		
		if (_separator != ";") then {
			// Show version difference
			_index = [_mod_id, FWATCH_MODLISTID] call FUNCTION_FIND;
			if (_index >= 0) then {
				_my_version = (FWATCH_MODLISTCFG select _index) select 0;

				if (_my_version < _mod_version) then {
					_verTXT = Format [MAINMENU_STR select 50, _my_version, _mod_version]
				} else {
					_mod_size = "";
					_index   = -1;
				}
			};
			
			_string = Format ["%1%2", _string, _mod_name];
			
			// Don't display version if it's 1 unless there's an update
			if (_mod_version != 1  ||  _index >= 0) then {
				_string = Format ["%1 %2", _string, _verTXT]
			};

			if (_mod_size != "") then {
				_string = Format ["%1 - %2", _string, _mod_size]
			};
			
			_string = Format ["%1%2", _string, _separator]
		} else {
			_string = Format["%1%2%3", _string, _mod_name, (if (_i < count _mods-1) then {_separator} else {""})]
		};

		_i = _i + 1
	} 
	forEach _mods; 

	_string
};



FUNCTION_ARRAY2STRING = {
	private ["_string", "_i"]; 
	_string = ""; 
	_i      = 0;

	{
		_string = Format["%1%2%3", _string, _x, (if (_i<count (_this select 0)-1) then {_this select 1} else {""})];
		_i      = _i + 1
	} 
	forEach (_this select 0);

	_string
};



FUNCTION_CTRLSETTEXT = {
	{ctrlShow [_x,(_this select 1) != ""]} forEach [(_this select 0),(_this select 0)+1];
	ctrlSetText [(_this select 0)+1, _this select 1]
};



FUNCTION_SHOW_RED_TEXT = {
	// check if yellow is visible in the first place
	if (ctrlVisible ((_this select 0)-2)) then {
		// if so then hide yellow and show red if argument is true
		ctrlShow [(_this select 0)-2, !(_this select 1)]; 
		ctrlShow [_this select 0, _this select 1]
	} else {
		// if not then hide red
		ctrlShow [_this select 0, false]
	}
};



FUNCTION_IS_MOD_MISSING = {
	private ["_index", "_add_to_missing", "_assign", "_my_version", "_mod_name", "_mod_sizearray", "_index2", "_index3", "_j"];

	// _x is an array with arrays containing [id,name,ver,forcename,size,size array]
	
	_add_to_missing = true;
	_assign         = false;
	_my_version     = 0;
	_mod_name       = _x select 1;
	_mod_sizearray  = [0,0,0];

	// Look for the mod id in the user's mod list
	_index2 = [_x select 0, FWATCH_MODLISTID] call FUNCTION_FIND;
	if (_index2 >= 0) then {
		_mod_name       = FWATCH_MODLIST select _index2;
		_my_version     = (FWATCH_MODLISTCFG select _index2) select 0;
		_add_to_missing = _my_version < (_x select 2);				// If user has older version
	} else {
		_index3 = [_mod_name,FWATCH_MODLIST] call FUNCTION_FIND;
		if (_index3 >= 0) then {
			_assign = (FWATCH_MODLISTID select _index3) == "";		// If modfolder exists but not tagged
		}
	};
	
	if (_add_to_missing) then {
		_missing_mods_id        set [count _missing_mods_id       , _x select 0];
		_missing_mods_name      set [count _missing_mods_name     , _mod_name  ];
		_missing_mods_assign    set [count _missing_mods_assign   , _assign    ];
		_missing_mods_version   set [count _missing_mods_version  , _x select 2];
		_missing_mods_myversion set [count _missing_mods_myversion, _my_version];
		_missing_mods_forcename set [count _missing_mods_forcename, _x select 3];

		_j = -1; 
		while "_j=_j+1; _j<count _missing_mods_sizearray" do {
			_missing_mods_sizearray set [_j, (_missing_mods_sizearray select _j)+((_x select 5) select _j)]
		}
	}
};



FUNCTION_ADD_MOD_TO_LISTBOX = {
	_id = lbAdd [6657, _x]; 
	lbSetData [6657, _id, FWATCH_MODLISTID select _i]; 
	lbSetValue [6657, _id, 2]; 
	
	if (_x in _launch_queue) then {
		lbSetValue [6657, _id, 3];
		lbSetColor [6657, _id, _color_red];
	} else {
		if ([_x,FWATCH_CURRMOD] call FUNCTION_FIND>=0) then {
			lbSetColor [6657, _id, _color_pink]
		}
	};
	
	_i = _i + 1;
};



FUNCTION_ADD_MOD_FOR_SHORTCUT_TO_LISTBOX = {
	_id = lbAdd [6657, _x]; 
	lbSetData [6657, _id, FWATCH_MODLISTID select _i]; 
	lbSetValue [6657, _id, 12]; 
	
	if (_x in _mods_for_new_shortcut) then {
		lbSetValue [6657, _id, 13];
		lbSetColor [6657, _id, _color_red];
	};
	
	_i = _i + 1;
};



FUNCTION_DISPLAY_MOD_QUEUE = {
	_i      = 0;
	_string = "-mod=";
	"_string=_string+_x; if (_i<count _this-1) then {_string=_string+"";""}; _i=_i+1" forEach _this;
	titleText [_string, "PLAIN DOWN", 0.1];
};



FUNCTION_MOD_SHORTCUTS = {
	private ["_index", "_output","_i","_folder","_extra_params"];
	_output_type = _this select 0;
	_data_index  = _this select 1;
	
	_output = "";
	
	if (_output_type == "launch") then {
		_output = [[], ""];		//0 - mod line, 1 - extra startup parameters
	};
	
	if (_output_type == "preview") then {
		_output = Format ["%1:\n\n", _mod_shortcuts_names select _data_index];
	};

	_i = 0;
	_extra_params = "";
	{
		_folder = "";
		
		if ((_x select 0) == "id") then {
			_index = [(_x select 1),FWATCH_MODLISTID] call FUNCTION_FIND;
			if (_index >= 0) then {
				_folder = (FWATCH_MODLIST select _index)
			} else {
				_folder = ""
			}
		};

		if ((_x select 0) == "name") then {
			_folder = _x select 1
		};
		
		if ((_x select 0) == "launch") then {
			_extra_params = _x select 1;
		};

		if (_folder != "") then {
			if (_output_type == "launch") then {
				(_output select 0) set [count (_output select 0), _folder];
			};
			
			if (_output_type == "preview") then {
				_output = _output + " " + _folder + "\n";
			}
		}
	} forEach (_mod_shortcuts_data select _data_index);
	
	if (_extra_params != "") then {
		if (_output_type == "launch") then {
			_output set [1, _extra_params];
		};
	
		if (_output_type == "preview") then {
			_output = _output + "\n" + _extra_params;
		}
	};
	
	_output
};


FUNCTION_ADD_DOWNLOADABLE_MOD_TO_LISTBOX = {
	private ["_entry", "_to_add", "_positions"];
	
	if (count _x == 1) then {
		// label
		if ((_x select 0) == "update")  then {lbSetColor [6657, lbAdd [6657, "==="+(MAINMENU_STR select 83)+"==="], _color_fireenginered]};
		if ((_x select 0) == "missing") then {lbSetColor [6657, lbAdd [6657, "==="+(MAINMENU_STR select 84)+"==="], _color_sandybrown]};
	} else {
		_to_add = true;
		
		if (_mod_search_category >= 0) then {
			_to_add = (_x select 2) == _mod_search_category;
		} else {
			if (_mod_search_phrase != "") then {
				_to_add = count (call loadFile Format ["\:STRING FIND text:%1find:%2", _x select 0, _mod_search_phrase]) > 0;
			}
		};
		
		if (_to_add) then {
			_entry = lbAdd [6657, _x select 0]; //name
			lbSetData  [6657, _entry, _x select 1];	//public id
			lbSetValue [6657, _entry, 6];

			if ((_x select 1) in _missing_mods_id) then {lbSetColor [6657,_entry,[1,0,0,0.5]]};
		}
	};
};



FUNCTION_ADD_SERVER_TO_LISTBOX = {
	private ["_entry"];
	
	if (count _x == 1) then {
		// label
		if ((_x select 0) == "now")        then {lbSetColor [6657, lbAdd [6657, MAINMENU_STR select 59], _color_fireenginered]};
		if ((_x select 0) == "today")      then {lbSetColor [6657, lbAdd [6657, MAINMENU_STR select 60], _color_sandybrown]};
		if ((_x select 0) == "future")     then {lbSetColor [6657, lbAdd [6657, MAINMENU_STR select 61], _color_sand]};
		if ((_x select 0) == "persistent") then {lbSetColor [6657, lbAdd [6657, "=="+(MAINMENU_STR select 108)+"=="], _color_cottoncandy]};
	} else {
		// server
		_entry = lbAdd [6657, _x select 0]; //name
		lbSetData  [6657, _entry, _x select 1];	//public id
		lbSetValue [6657, _entry, 201];		

		// After installation display server options
		if (( _x select 1) == _jump_to_server) then {
			_data = _x select 1;
			goto "DisplayServer"
		};
	};
};



FUNCTION_GET_EXECUTE_PARAMS = {
	private ["_params", "_params_modid", "_ok", "_Input", "_add_param_name", "_modifier"];
	_params = "";
	
	if (_server_uniqueid != "mod") then {
		_modifier = if (_server_encrypted) then {"e"} else {""};
		_params   = Format ["-serveruniqueid=%1 -%2connect=", _server_uniqueid, _modifier] + _server_ip;
		
		if (_server_port != "")           then {_params=_params+Format[" -%1port=",_modifier]+_server_port};
		if (_server_password != "")       then {_params=_params+Format[" -%1password=",_modifier]+_server_password};
		if (_server_equalModReq)          then {_params=_params+" -serverequalmodreq=true"};
		if (_run_voice_program)           then {_params=_params+Format[" -%1voice=",_modifier]+(_server_voice select 1)};
		if (_server_maxcustombytes != "") then {_params=_params+Format[" -maxcustom=%1 ""-plrname=%2""",_server_maxcustombytes, FWATCH_USERNAME]};
		
		_add_param_name = true;
        _params_modid   = "";

		{
			if (_params_modid == "") then {
				_params_modid = _params_modid + " -modid=";
			} else {
				_params_modid = _params_modid + ";";
			};
			
			_params_modid = _params_modid + (_x select 0);
		} forEach _server_modfolders;
        
        _params = _params + _params_modid;
	} else {
		_params = "-mod=" + (_missing_mods_name select 0);
	};
	
	_params
};



FUNCTION_WRITE_INSTALL = {
	private ["_ids", "_names"];
	_ids   = "-installid=";
	_names = "-installdir=";
	
	_i = 0;
	{
		_ids   = _ids   + (if (_i>0) then {","} else {""}) + (_missing_mods_id   select _i);
		_names = _names + (if (_i>0) then {","} else {""}) + (_missing_mods_name select _i);
		_i = _i + 1;
	} 
	forEach _missing_mods_id;
	
	_ids + " " + _names
};



FUNCTION_BUILD_QUERY_STRING = {
	private ["_output", "_i", "_mod", "_ver"];
	_output = ([_access_code, "password"] call FUNCTION_FORMAT_PASSWORD_STRING);
	_output = _output + ([_access_code_mod, "password_mods"] call FUNCTION_FORMAT_PASSWORD_STRING);
	
	if ((_this select 0) == "modcheck") then {
		_mod = "";
		_ver = "";
		_i   = -1;

		while "_i=_i+1; _i<count (_this select 1)" do {
			_index = [(_this select 1) select _i, FWATCH_MODLISTID] call FUNCTION_FIND;
			
			if (_index >= 0) then {
				_mod = _mod + (if (_mod=="") then {""} else {","}) + (FWATCH_MODLISTID select _index);
				_ver = _ver + (if (_ver=="") then {""} else {","}) + Format["%1",(FWATCH_MODLISTCFG select _index) select 0];
			}
		};
		
		if (_mod != "") then {
			_output = _output + "&mod=" + _mod + "&ver=" + _ver;
		}
	};

	if ((_this select 0) == "modinstall") then {
		_mod = "";
		_ver = "";
		_i   = -1;
		
		while "_i=_i+1; _i<count (_this select 1)" do {
			_mod = _mod + (if (_mod=="") then {""} else {","}) + ((_this select 1) select _i);
			_ver = _ver + (if (_ver=="") then {""} else {","}) + Format["%1",((_this select 2) select _i)];
		};
		
		_output = _output + "&mod=" + _mod + "&ver=" + _ver;
	};
	
	if ((_this select 0) == "allusermods") then {
		_mod    = "";
		_ver    = "";
		_output = _output + " --post-data=";
		
		{
			if (_x != "") then {
				_mod = _mod + (if (_mod=="") then {""} else {","}) + _x;
			};
		} forEach FWATCH_MODLISTID;
		
		{
			if ((_x select 0)!=0) then {
				_ver = _ver + (if (_ver=="") then {""} else {","}) + Format["%1",_x select 0];
			};
		} forEach FWATCH_MODLISTCFG;
		
		_output = _output + "&mod=" + _mod + "&ver=" + _ver;
	};

	_output
};



FUNCTION_FORMAT_FILESIZE = {
	private ["_roundToTenths", "_index", "_size"];
	
	_roundToTenths = "private [""_fraction""];_this=_this-(_this mod 0.001); _fraction=(_this mod 0.01);_this=_this-_fraction;if (_fraction>0.004999) then {_this=_this+0.01};_fraction=(_this mod 0.1);_this=_this-_fraction;if (_fraction>0.049999) then {_this=_this+0.1};_this";
	_index         = 2;

	if ((_this select 2) == 0) then {_index=1};
	if ((_this select 1) == 0) then {_index=0};
	
	_size = _this select _index; 
	if (_index > 0) then {_size=_size+(_this select (_index-1))/1024}; 
		
	Format ["%1 %2", _size call _roundToTenths, ["B","KB","MB"] select _index]
};



FUNCTION_CUSTOM_SIZE = {
	private ["_output_text", "_custom_file_info", "_biggest_file_name", "_biggest_file_size", "_server_limit"];
	_output_text = "";
	
	if (count _server_maxcustomfilesize > 0) then {	
		{
			if ((_x select 0) == _server_uniqueid) then {
				_output_text = _x select 1;
				_too_big     = _x select 2;
			}
		} forEach GS_CUSTOM_FILE_CACHE;
		
		if (_output_text == "") then {
			_custom_file_info  = call loadFile Format ["\:FILE CUSTOMCOUNTSIZE ""%1"" %2" , [_server_modfolders,";"] call FUNCTION_MODS2STRING, _server_maxcustombytes];
			_too_big           = !(_custom_file_info select 4);
			_biggest_file_name = _custom_file_info select 5;
			_biggest_file_size = _custom_file_info select 6;
			FWATCH_USERNAME    = _custom_file_info select 7;
		
			_server_limit =
			if ("_x>0" count _server_maxcustomfilesize > 0) then {
				_server_maxcustomfilesize call FUNCTION_FORMAT_FILESIZE
			} else {
				MAINMENU_STR select 62
			};
		
			_output_text = 	
			if (_too_big) then {
				Format ["%1 < %2 (%3)", _server_limit, _biggest_file_name, _biggest_file_size call FUNCTION_FORMAT_FILESIZE]
			} else {
				_server_limit
			};
			
			GS_CUSTOM_FILE_CACHE set [count GS_CUSTOM_FILE_CACHE, [_server_uniqueid, _output_text, _too_big]];
		}
	};
	
	_output_text
};



FUNCTION_MSG_LB = {
	lbAdd [6657, _this];
	if (lbSize 6657 >= 10) then {
		lbSetCurSel [6657, lbAdd [6657, ""]]
	}
};



FUNCTION_REFRESH_MODLIST = {
	_ok = call loadFile ":file modlist";
	if (_ok select 0) then {
		FWATCH_MODLIST      = _ok select 4; 
		FWATCH_MODLISTID    = _ok select 5;
		FWATCH_MODLISTCFG   = _ok select 6; 
		FWATCH_MODLISTHASH  = _ok select 7;
	} else {
		titleText [((MAINMENU_STR select 63)+(_ok select 3)),"PLAIN DOWN",0.1]
	}
};



FUNCTION_DOWNLOAD_INFO = {
	if (!_silent_mode) then {
		_title = _this select 0;
		_text  = _this select 1;

		lbSetValue [6657, lbAdd [6657,"["+_title+"]"], 0];
		lbSetCurSel [6657, (lbSize 6657)-1];

		"ctrlShow [_x,false]" forEach _info_window;
		"ctrlShow [_x,true ]" forEach [6460, 6463];
		
		if (_text != "compareversion") then {
			ctrlSetText [6463, _text];
		} else {
			ctrlSetText [6463, Format [MAINMENU_STR select 75, GS_VERSION, GS_MY_VERSION]];
		};
		
		lbAdd [6657, ""];
		lbSetValue [6657, lbAdd [6657, MAINMENU_STR select 85], 227]
	}
};



FUNCTION_READ_DOWNLOADED_FILE = {
	private ["_continue", "_downloaded_file", "_error","_error_title","_error_message","_file_to_display","_ok"];
	
	_continue        = _this select 0;
	_downloaded_file = _this select 1;
	_error           = !(GS_DOWNLOAD_RESULT select 0);
	_error_title     = MAINMENU_STR select 15;
	_error_message   = "";
	_file_to_display = "downloadLog.txt";

	// Read file if download succeeded
	if (GS_DOWNLOAD_RESULT select 0) then {
		_ok = call loadFile Format ["\:IGSE DB file:..\fwatch\tmp\%1 read:general", _downloaded_file];

		// If reading failed
		if (!(_ok select 0) || (count (_ok select 5) > 0)) then {
			_error           = true;
			_error_title     = MAINMENU_STR select 18;
			_error_message   = if (!(_ok select 0)) then {_ok select 3} else {"Missing general key"};
			_file_to_display = "";
		} else {
			_all_ok = true;
		};
	};
	
	// Display error
	if (_error) then {
		if ((GS_DOWNLOAD_RESULT select 1) == 5) then {
			if ((GS_DOWNLOAD_RESULT select 2) in [5,186]) then {
				GS_DOWNLOAD_RESULT set [3, (GS_DOWNLOAD_RESULT select 3)+"\n\nCheck if firewall or antivirus aren't blocking\n\nGo to fwatch.exe properties and set it to run as an administrator"];
			};

			[_error_title, GS_DOWNLOAD_RESULT select 3] call FUNCTION_DOWNLOAD_INFO;
			goto "ResetLMB";
		} else {
			// Download from the next mirror
			if (_mirror<count (GS_DEFAULT_URL select _i)-1) then {
				if (!_silent_mode) then {lbSetValue [6657, lbAdd [6657,"["+_error_title+"]"], 0]};
				_mirror = _mirror + 1;
				goto _continue;
			} else {
				// No more mirrors
				if (_file_to_display != "") then {
					_error_message = loadFile Format ["\:IGSE LOAD  mode:execute  file:..\fwatch\tmp\schedule\%1",_file_to_display];
				};
				
				[_error_title,(GS_DOWNLOAD_RESULT select 3)+"\n\n"+_error_message] call FUNCTION_DOWNLOAD_INFO;
				goto "ResetLMB";
			};		
		};
	};
};



FUNCTION_VOICE_URL_FIND = {
	private ["_i", "_found"];

	if (count _server_voice > 1) then {
		_i     = -1;
		_found = false;
		
		while "_i=_i+1; _i<count GS_VOICE && !_found" do {
			_positions = call loadFile Format ["\:STRING FIND text:%1find:%2", _server_voice select , _voice_url select _i];
			
			if (count _positions > 0) then {
				_voice_index = _i;
				_found       = true;
			}
		}
	}
};



FUNCTION_BUILD_PREVIEW_LINK = {
	private ["_url", "_id_list", "_ver_list", "_id_string", "_ver_string", "_i", "_positions"];
	
	_url        = _this select 0;
	_id_list    = _this select 1;
	_ver_list   = _this select 2;
	_id_string  = "";
	_ver_string = "";
	_i          = -1;
	
	_positions = call loadFile ("\:STRING FIND find:/text:"+_url);
	if (count _positions > 0) then {
		_url = loadFile (Format ["\:STRING CUT end:%1 text:", _positions select (count _positions-1)] + _url);
	};
	
	_i=-1;
	while "_i=_i+1; _i<count _id_list" do {		
		_id_string  = _id_string  + (if (_id_string=="" ) then {""} else {","}) + (_id_list select _i);
		_ver_string = _ver_string + (if (_ver_string=="") then {""} else {","}) + (if (_i >= count _ver_list) then {"0"} else {Format ["%1",_ver_list select _i]});
	};
	
	_url + "/show.php?onlychangelog=1&mod=" + _id_string + "&ver=" + _ver_string
};



FUNCTION_STRINGTABLE = {
	call FLIB_CURRENTLANG;
	
	if (CURRENT_LANGUAGE == "Polish") then {
		MAINMENU_STR = [
			"Interfejs niedost�pny. Sprawd� sw�j bin\resource.cpp.",		//0
			"Plik bin\resource.cpp jest przestarza�y. �ci�gnij now� wersj� OFP Aspect Ratio",		//1
			"ROZK�AD ROZGRYWEK",		//2
			"SERWER PRZEGL�DARKI GIER",		//3
			"Jest ju� w��czony!",		//4
			"Napisz has�a do prywatnych serwer�w",		//5
			"[Uruchom bez mod�w]",		//6
			"[Dodaj nowy]",		//7
			"Nie ma nic do zapisania!",		//8
			"Jest ju� na li�cie!",		//9
			"[Prosz� czeka�]",		//10
			"[�ci�gnij %1]",		//11
			"[Otw�rz zaproszenie na %1]",		//12
			"Nie uda�o si� utworzy� fwatch\tmp\schedule\n%1\n\nUruchom gr� jako administrator",		//13
			"[Sprawdzanie aktualizacji]",		//14
			"Nie uda�o si� pobra�",		//15
			"[Pobieranie planu]",		//16
			"[Przetwarzanie danych]",		//17
			"Niepoprawne dane",		//18
			"[Brak serwer�w]",		//19
			"Niepoprawna wersja planu",		//20
			"[�ci�ganie loga %1]",		//21
			"Nie uda�o si� pobra� loga\n%1",		//22
			"[Uk�adanie serwer�w]",		//23
			"[Odwo�aj zaplanowane pod��czenie]",		//24
			"[Poka� prywatne gry]",		//25
			"[Do��cz o czasie]",		//26
			"[Do��cz]",		//27
			"[�ci�gnij wymagane mody %1]",		//28
			"[Poka� zmiany w modach]",		//29
			"[Odwied� stron�]",		//30
			"[Dodatkowe opcje uruchamiania]",		//31
			"[Cofnij]",		//32
			"[Uruchom %1]",		//33
			"[Bez]",		//34
			"",		//35
			"[Przerwij]",		//36
			"Ju� masz folder %1. Czy wolisz:\n\n- zainstalowa� now� kopi� (bezpieczna opcja; nazwa obecnego modu zostanie zmieniona)\n\nLUB\n\n- Oznaczy� obecny mod jako wersj� %2 (szybsza opcja; dane moda b�d� podlega� aktualizacjom)",		//37
			"[�ci�gnij now� kopi�]",		//38
			"[Oznacz aktualn�]",		//39
			"Nie uda�o si� uruchomi� fwatch\data\addonInstaller.exe\n%1",		//40
			"Nie uda�o si� utworzy� pliku\n%1",		//41
			"[Uruchom ponownie po sko�czeniu: %1]",		//42
			"TAK",		//43
			"NIE",		//44
			"\n\n\nFwatch nie widzi instalatora.\nZignoruj ten komunikat je�li wida� post�p instalacji.\n\nW innym przypadku sprawd� w menedrze�e zada� czy jest addonInstaller.exe; przerwij instalacj�; zobacz fwatch\data\addonInstallerLog.txt",		//45
			"Serwer wymaga dok�adnie tych samych mod�w wi�c argument -mod zostanie pomini�ty",		//46
			"Gra zostanie uruchomiona o\n%1\n\ni pod�aczy si� automatycznie do\n%2",		//47
			"[Aktualizuj %1]",		//48
			"Jest dostp�pna aktualizacja %1. Musisz uaktualni�.\n\n\n       Nowa wersja:\n           %2\n\n       Twoja wersja:\n           %3\n\n\nDwuklik na opcj� �eby zacz�� proces. Instalator zamknie gr�, �ci�gnie now� wersj�, zamieni dane i uruchomi gr� ponownie.",		//49
			"%1 do %2",		//50
			"Co niedziel�",		//51
			"Co poniedzia�ek",		//52
			"Co wtorek",		//53
			"Co �rod�",		//54
			"Co czwartek",		//55
			"Co pi�tek",		//56
			"Co sobot�",		//57
			"Codziennie",		//58
			"==Teraz==",		//59
			"==Dzisiaj==",		//60
			"==Wkr�tce==",		//61
			"niedozwolone",		//62
			"B��D:\n",		//63
			"[Kontynuuj]",		//64
			"Wersja:",		//65
			"Mody:",		//66
			"W�asne pliki:",		//67
			"Rozk�ad:",		//68
			"Rozmowy:",		//69
			"J�zyki:",		//70
			"Po�o�enie:",		//71
			"Strona:",		//72
			"Uwagi:",		//73
			"Wpisz tekst:",		//74
			"�ci�gnij wersj�:\n           %1\n\nTwoja wersja:\n           %2",		//75
			"Nie mo�esz zaktualizowa� %1 bo jest w��czony.\n\nCzy chcia�by� uruchomi� gr� bez mod�w?",		//76
			"Nie mo�esz zaktualizowa� %1 bo s� w��czone.\n\nCzy chcia�by� uruchomi� gr� bez mod�w?",		//77
			"Plan rozgrywek",		//78
			"Serwer przegl�darki gier",		//79
			"nowej wersji testowej Fwatch 1.16",		//80
			"nowej wersji Resource.cpp",		//81
			"[�ci�gnij mody]",		//82
			"Dost�pne aktualizacje",		//83
			"Pobierz nowy mod",		//84
			"[Zamknij Okno]",		//85
			"Jest dostp�pna aktualizacja nowej wersji testowej Fwatch 1.16",		//86
			"Dostp�pne aktualizacje mod�w: %1",		//87
			"[Do��cz po sko�czeniu: %1]",		//88
			"[Poka� prywatne mody]",		//89
			"Napisz has�a do prywatnych mod�w",		//90
			"Wyszukaj",		//91
			"Wpisz nazw� moda lub kategorii (rozszerzenie; zbi�r addon�w; uzupe�nienie; zbi�r misji; narz�dzia)",		//92
			"Typ:",		//93
			"Do pobrania:",		//94
			"Doda�:",		//95
			"Opis:",		//96
			"wymusza oryginaln� nazw�",		//97
			"niezgodny z gr� sieciow�",		//98
			"rozszerzenie",		//99
			"zbi�r addon�w",		//100
			"uzupe�nienie",		//101
			"zbi�rmisji",		//102
			"narz�dzia",		//103
			"Opcje pod prawym przyciskiem lub spacj�",		//104
			"[Dodaj do kolejki]",		//105
			"Brak po��czenia",		//106
			"Status pod prawym przyciskiem lub spacj�",		//107
			"Sta�e",		//108
			"[Utw�rz Skr�t]",		//109
			"Nazwa skr�tu",		//110
			"Wybierz mody dla skr�tu",		//111
			"[Wstrzymaj]",		//112
			"Opcjonalnie wpisz dodatkowe parametry uruchamiania gry",		//113
			"Wersja gry:" 		//114
		];
	};

	if (CURRENT_LANGUAGE == "Russian") then {
		MAINMENU_STR = [
			"������ ��� �������� ����������� ����. ��������� bin\resource.cpp",		//0
			"���� bin\resource.cpp �������. ��������� ����� ������ ���� OFP Aspect Ratio",		//1
			"���������� ���",		//2
			"������ ������",		//3
			"������� �����!",		//4
			"������� ������, ����� ����������� ����",		//5
			"[������ ��� �����]",		//6
			"[�������� �����]",		//7
			"��� ������ ��� ����������!",		//8
			"��� ���� � ������!",		//9
			"[����������, ���������]",		//10
			"[������� %1]",		//11
			"[��������������� ������ %1]",		//12
			"������ ��� �������� ����� fwatch\tmp\schedule\n%1\n\n��������� ���� �� ����� ��������������",		//13
			"[����� ����������]",		//14
			"������ ��� ��������",		//15
			"[�������� ����������]",		//16
			"[���������� ����������]",		//17
			"������ ��� ����������",		//18
			"[��� ��������]",		//19
			"[�������� ������ ����������]",		//20
			"[�������� ������� %1]",		//21
			"������ ��� ��������\n%1",		//22
			"[���������� ����������]",		//23
			"[�������� �������������� �����������]",		//24
			"[�������� ������� �������]",		//25
			"[������������ �������������]",		//26
			"[������������]",		//27
			"[������� ���� %1]",		//28
			"[����������� ������� ���������]",		//29
			"[�������� ���-����]",		//30
			"[�������������� ��������� �������]",		//31
			"[�����]",		//32
			"[� %1]",		//33
			"[��� %1]",		//34
			"",		//35
			"[������]",		//36
			"��� ������� ����� � ����� %1. \n\n- ��������� ����� ����� (���������� �����; ������� ����� ����� �������������)\n\n���\n\n- �������� ������������� ������ %2 � ������� ����� (������� �����; ��� ����� ���� ������� � ����������)?",		//37
			"[��������� ����� �����]",		//38
			"[�������� �������������]",		//39
			"������ ��� ������� fwatch\data\addonInstaller.exe\n%1",		//40
			"������ ��� �������� �����\n%1",		//41
			"[���������� ����� ���������: %1]",		//42
			"���",		//43
			"����",		//44
			"\n\n\nFwatch �� ��������� ������� ���������.\nIf the progress is still going then ignore this message.\n\nOtherwise open Task Manager and check if addonInstaller.exe is running; abort installation; see addonInstallerLog.txt",		//45
			"���������� ���������� ���� ��� ����������� (�������� -mod ��� ������� �� �����������)",		//46
			"���� ����� ������������ �\n%1\n\n����� ������������ � �������\n%2",		//47
			"[�������� %1]",		//48
			"�������� %1. ��������, ����� ����������.\n\n\n       ����� ������:\n           %2\n\n       ������� ������:\n           %3\n\n\n������� ��� ����, ����� ��������� ����������. ��� ���������� ���� ��������������.",		//49
			"%1 � %2",		//50
			"������ �����������",		//51
			"������ �����������",		//52
			"������ �������",		//53
			"������ �����",		//54
			"������ �������",		//55
			"������ �������",		//56
			"������ �������",		//57
			"������ ����",		//58
			"==�������==",		//59
			"==C������==",		//60
			"==�������==",		//61
			"���������",		//62
			"������:\n",		//63
			"[����������]",		//64
			"������:",		//65
			"����:",		//66
			"����. ����:",		//67
			"����������:",		//68
			"�����. ���:",		//69
			"�����:",		//70
			"������:",		//71
			"���-����:",		//72
			"����������:",		//73
			"������� �����:",		//74
			"��������� ������:\n           %1\n\n���� ������:\n           %2",		//75
			"���������� �������� %1, ���� �� �������.\n\n������������� ����?",		//76
			"���������� �������� %2, ���� ��� ��������.\n\n������������� ����?",		//77
			"���������� ���",		//78
			"������ ������",		//79
			"����� �������� ������ Fwatch 1.16",		//80
			"����� ������ Resource.cpp",		//81
			"[������� ����]",		//82
			"��������",		//83
			"�������� �����",		//84
			"[������� ����]",		//85
			"�������� ����� �������� ������ Fwatch 1.16",		//86
			"�������� ����: %1",		//87
			"[������������ ����� ���������: %1]",		//88
			"[�������� ������� ����]",		//89
			"������� ������, ����� ����������� ����",		//90
			"�����",		//91
			"������� �������� ���� ��� �������� ��������� (������; ������; ����������; ������; �����������)",		//92
			"���:",		//93
			"�������:",		//94
			"��������:",		//95
			"��������:",		//96
			"�������� ����������� ��������",		//97
			"������������ � ������� �����",		//98
			"������",		//99
			"������",		//100
			"����������",		//101
			"������",		//102
			"�����������",		//103
			"������ ������� ��� ������ ��� ����������",		//104
			"[�������� � �������]",		//105
			"�� � ����",		//106
			"������ ������� ��� ������ ��� �������",		//107
			"����������",		//108
			"[������� �����]",		//109
			"����������� ���",		//110
			"������� ���� ��� ������",		//111
			"[�����]",		//112
			"������� �������������� ��������� �������",		//113
			"������ ����:" 		//114
		];
	};

	if (Format["%1", count MAINMENU_STR] == "scalar") then {
		MAINMENU_STR = [
			"Failed to create dialog. Verify your bin\resource.cpp.",		//0
			"File bin\resource.cpp is outdated. Download new OFP Aspect Ratio pack version",		//1
			"GAME SCHEDULE",		//2
			"MASTER SERVER",		//3
			"This is the current one!",		//4
			"Type in password(s) to show private game(s)",		//5
			"[Start Without Mods]",		//6
			"[Add New]",		//7
			"There's nothing to save!",		//8
			"It's already on the list!",		//9
			"[Please wait]",		//10
			"[Download %1]",		//11
			"[Open %1 Invite]",		//12
			"Failed to create directory fwatch\tmp\schedule\n%1\n\nSet the game to run as admin",		//13
			"[Checking for updates]",		//14
			"Download Failed",		//15
			"[Downloading schedule]",		//16
			"[Reading data]",		//17
			"Invalid Data",		//18
			"[No servers]",		//19
			"Incorrect schedule version",		//20
			"[Downloading logo %1]",		//21
			"Failed to download logo\n%1",		//22
			"[Sorting game times]",		//23
			"[Cancel Auto Connect]",		//24
			"[Show Private Servers]",		//25
			"[Auto-Connect on Time]",		//26
			"[Connect]",		//27
			"[Download Mods %1]",		//28
			"[View Changelog]",		//29
			"[Visit Website]",		//30
			"[Extra Startup Parameters]",		//31
			"[Back]",		//32
			"[With %1]",		//33
			"[Without]",		//34
			"",		//35
			"[Abort]",		//36
			"You already have %1 modfolder. Would you like to:\n\n- Install a new copy (safer option; current one will be renamed)\n\nOR\n\n- Tag the existing one as version %2 (faster option; mod will be a subject to future modifications)",		//37
			"[Download New]",		//38
			"[Assign ID]",		//39
			"Failed to launch fwatch\data\addonInstaller.exe\n%1",		//40
			"Failed to generate a file\n%1",		//41
			"[Restart when done: %1]",		//42
			"ON",		//43
			"OFF",		//44
			"\n\n\nFwatch can't detect installator.\nIf the progress is still going then ignore this message.\n\nOtherwise open Task Manager and check if addonInstaller.exe is running; abort installation; see addonInstallerLog.txt",		//45
			"Server requires exact mods so the -mod parameter will be ignored",		//46
			"Game will be restarted at\n%1\n\nin order to connect to\n%2",		//47
			"[Update %1]",		//48
			"There is a %1 available. You must update in order to continue.\n\n\n       New version:\n           %2\n\n       Your version:\n           %3\n\n\nDouble-click on the option to start the process. Update program will close the game, download the patch, replace files and then start the game again.",		//49
			"%1 to %2",		//50
			"Every Sunday",		//51
			"Every Monday",		//52
			"Every Tuesday",		//53
			"Every Wednesday",		//54
			"Every Thursday",		//55
			"Every Friday",		//56
			"Every Saturday",		//57
			"Daily",		//58
			"==Now==",		//59
			"==Today==",		//60
			"==Upcoming==",		//61
			"none allowed",		//62
			"ERROR:\n",		//63
			"[Continue]",		//64
			"Version:",		//65
			"Modfolders:",		//66
			"Custom File:",		//67
			"Game time:",		//68
			"VOIP:",		//69
			"Languages:",		//70
			"Location:",		//71
			"Website:",		//72
			"Message:",		//73
			"Enter Text:",		//74
			"Downloaded version:\n           %1\n\nYour version:\n           %2",		//75
			"You cannot update %1 while you have it loaded.\n\nWould you like to restart the game?",		//76
			"You cannot update %1 while you have them loaded.\n\nWould you like to restart the game?",		//77
			"Game Schedule",		//78
			"Master Server",		//79
			"new test version of Fwatch 1.16",		//80
			"new Resource.cpp version",		//81
			"[Download Mods]",		//82
			"Available Updates",		//83
			"Install New",		//84
			"[Close Window]",		//85
			"There is a new test version of Fwatch 1.16 available",		//86
			"Available mod updates: %1",		//87
			"[Connect when done: %1]",		//88
			"[Show Private Mods]",		//89
			"Type in password(s) to show private mod(s)",		//90
			"Search",		//91
			"Type mod or category name (replacement; addon pack; supplement; mission pack; tools)",		//92
			"Type:",		//93
			"Download:",		//94
			"Added by:",		//95
			"Description:",		//96
			"force original name",		//97
			"multiplayer incompatible",		//98
			"replacement",		//99
			"addonpack",		//100
			"supplement",		//101
			"missionpack",		//102
			"tools",		//103
			"Right-click or space for options",		//104
			"[Add to Queue]",		//105
			"Offline",		//106
			"Right-click or space for status",		//107
			"Persistent",		//108
			"[Create Shortcut]",		//109
			"Shortcut name",		//110
			"Select mods for the shortcut",		//111
			"[Pause]",		//112
			"Optionally type in extra game startup parameters",		//113
			"Game ver:" 		//114
		];
	};
	
	MAINMENU_STR_MODCAT = [
		["replacement","rozszerzenie","������"],
		["addon pack","zbi�r addon�w","������"],
		["supplement","uzupe�nienie","����������"],
		["mission pack","zbi�r misji","������"],
		["tools","narz�dzia","�����������"]
	];
};



FUNCTION_FORMAT_PASSWORD_STRING = {
	private ["_output", "_values", "_array", "_i"];
	_output = "";
	_values = "";
	_i      = -1;
	_array  = call loadFile Format ["\:STRING TOKENIZE  text:%1delimiter: ", _this select 0];
	
	while "_i=_i+1; _i<count _array" do {
		_values = _values + (if (_i==0) then {""} else {","}) + (_array select _i);
	};
	
	if (_values != "") then {
		_output = _output + "&"+(_this select 1)+"=" + _values;
	};
	
	_output
};



FUNCTION_LBADD = {
	private ["_entry"];
	if (!_silent_mode) then {		
		_entry = lbAdd [6657, _this select 0];
		lbSetValue [6657, _entry, _this select 1];
		lbSetColor [6657, _entry, _this select 2];
	};
	_entry
};



FUNCTION_SHOW_MOD_INFO = {
	private ["_ok", "_mod_id", "_source", "_my_game_version", "_description", "_index_local"];
	_mod_id          = _this select 0;
	_source          = _this select 1;
	_my_game_version = _this select 2;
	_mod_name        = "";
	_mod_type        = 0;
	_mod_version     = 0;
	_mod_req_version = 0;
	_mod_forcename   = false;
	_mod_size        = "";
	_mod_sizearray   = [0,0,0];
	_mod_is_mp       = true;
	_mod_addedby     = "";
	_mod_description = "";
	_mod_website     = "";
	_mod_logo        = "";
	_mod_logohash    = "";
	_ok              = [false,0,0,"",[],[]];
	
	if (_mod_id != "") then {
		_ok = call loadFile Format ["\:IGSE DB file:..\fwatch\tmp\schedule\dl_mods.db read:%1", _mod_id];
	};
	
	if ((_ok select 0) && count (_ok select 5)==0) then {
		"ctrlShow [_x,false]" forEach _info_window;
		"ctrlShow [_x,true ]" forEach [6460, 6461, 6462];
	
		ctrlSetText [6462, _mod_name];
		
		_description = MAINMENU_STR select (99 + _mod_type);
		if (_mod_forcename) then {_description=Format["%1\n%2",_description, MAINMENU_STR select 97]};
		if (!_mod_is_mp) then {_description=Format["%1\n%2",_description, MAINMENU_STR select 98]};
		
		[6480, _description] call FUNCTION_CTRLSETTEXT;
		ctrlSetText [6480, MAINMENU_STR select 93];
		
		[6530, _mod_addedby] call FUNCTION_CTRLSETTEXT;
		ctrlSetText [6530, MAINMENU_STR select 95];
		
		[6550, _mod_description] call FUNCTION_CTRLSETTEXT;
		ctrlSetText [6550, MAINMENU_STR select 96];
		
		{ctrlSetText [_x, MAINMENU_STR select 114]} forEach [6500, 6502];
		[6500, Format ["%1",_mod_req_version]] call FUNCTION_CTRLSETTEXT;
		[6502, _my_game_version < _mod_req_version] call FUNCTION_SHOW_RED_TEXT;
		
		[6540, (call loadFile Format ["\:STRING DOMAIN url:%1", _mod_website]) select 3] call FUNCTION_CTRLSETTEXT;
		
		ctrlShow [6464, true];
		ctrlSetText [6464, MAINMENU_STR select 104];
			
		_index_local = [_mod_id, FWATCH_MODLISTID] call FUNCTION_FIND;
		
		ctrlSetText [6470, MAINMENU_STR select 65];
		ctrlSetText [6540, MAINMENU_STR select 72];
		
		// Showing info for users mods
		if (_source == "local") then {
			if (_index_local >= 0) then {
				[6470, Format["%1",(FWATCH_MODLISTCFG select _index_local) select 0]] call FUNCTION_CTRLSETTEXT;
				
				if (((FWATCH_MODLISTCFG select _index_local) select 0) < _mod_version) then {
					[6472, true] call FUNCTION_SHOW_RED_TEXT;
					[6490, _mod_size] call FUNCTION_CTRLSETTEXT;
					ctrlSetText [6490, MAINMENU_STR select 94];
				}
			};
		// Showing info for mods from the db
		} else {
			_description = Format ["%1",_mod_version];
			
			if (_index_local >= 0) then {
				if (((FWATCH_MODLISTCFG select _index_local) select 0) != _mod_version) then {
					_description = Format ["%1 --> %2", (FWATCH_MODLISTCFG select _index_local) select 0, _mod_version];
				}
			};
			
			[6470, _description] call FUNCTION_CTRLSETTEXT;
			[6490, _mod_size] call FUNCTION_CTRLSETTEXT;
			ctrlSetText [6490, MAINMENU_STR select 94];
		};
		
		// Logo
		["logo_mod", "schedulemodlogo.bin", _mod_logo, _mod_id, _mod_logohash, _mod_name, lbCurSel 6657] call FUNCTION_DISPLAY_LOGO;
	} else {
		"ctrlShow [_x,false]" forEach _info_window;
	}
};



FUNCTION_DISPLAY_LOGO = {
	private ["_img_folder", "_database", "_url", "_record_id", "_global_hash", "_record_title", "_cursel", "_extension", "_logohash", "_download"];
	_img_folder   = _this select 0;
	_database     = _this select 1;
	_url          = _this select 2;
	_record_id    = _this select 3;
	_global_hash  = _this select 4;
	_record_title = _this select 5;
	_cursel       = _this select 6;
	
	ctrlSetText [6461, ""];
	ctrlSetText [6463, ""];//hide dots animation

	if (!(_record_id in GS_QUEUED_IMAGES)) then {
		// Read local record
		_extension = "";
		_logohash  = "";
		call loadFile Format ["\:IGSE DB  file:..\fwatch\tmp\schedule\%1  read:%2", _database, _record_id];
		
		// If logo exists on the website
		if (_global_hash!="" && _url!="") then {
			_download = true;
			
			// does the file exist locally
			if (_global_hash == _logohash) then {
				_ok = call loadFile Format ["\:IGSE NEW  mode:check  file:..\fwatch\tmp\schedule\%1\%2.%3", _img_folder, _record_id, _extension];
				if (_ok select 0) then {
					ctrlSetText [6461, Format ["..\fwatch\tmp\schedule\%1\%2.%3", _img_folder, _record_id, _extension]];
					_download = false;
				}
			} else {
				loadFile Format ["\:IGSE NEW  file:..\fwatch\tmp\schedule\%1\%2.%3  mode:delete", _img_folder, _record_id, _extension];
			};
			
			// if not then download it
			if (_download) then {
				GS_QUEUED_IMAGES set [count GS_QUEUED_IMAGES, _record_id];
				["download", _img_folder, _database, _url, _record_id, _global_hash, _record_title, _cursel] exec "..\fwatch\data\MainMenu.sqs";
			}
		} else {
			// remove it locally
			if (_logohash != "") then {
				loadFile Format ["\:IGSE DB  file:..\fwatch\tmp\schedule\%1  key:%2write:_extension="""";_logohash="""";", _database, _record_id];
			}
		}
	}
};



FUNCTION_GET_SERVER_STATUS_NAME = {
	private ["_output"];
	_output = MAINMENU_STR select 106;
	if (_this >= 1 && _this <= 14) then {
		_output = localize ([
			"",
			"STR_SESSION_CREATE",
			"STR_SESSION_CREATE",
			"STR_SESSION_CREATE",
			"STR_SESSION_EDIT",
			"STR_SESSION_WAIT",
			"STR_SESSION_WAIT",
			"STR_SESSION_SETUP",
			"STR_SESSION_SETUP",
			"STR_SESSION_DEBRIEFING",
			"STR_SESSION_DEBRIEFING",
			"STR_SESSION_SETUP",
			"STR_SESSION_SETUP",
			"STR_SESSION_BRIEFING",
			"STR_SESSION_PLAY"
		] select _this);
	};
	_output
};



FUNCTION_LIST_EVENTS_FOR_AUTO_CONNECTION = {
	_event_text = _x select 0;
	_event_id   = _x select 1;

	_description = Format ["[%1]",_event_text];
	_option_value = 231;
	
	if (_event_id in _scheduled_events) then {
		_description  = Format ["[%1 %2]",localize "STR_DISP_DELETE", _event_text];
		_option_value = 211;
	};
	
	_index = [_description, _option_value] call FUNCTION_LBADD;
	lbSetData [6657, _index, Format["%1",_i]];
	
	if (_event_id in _scheduled_events) then {
		lbSetColor [6657, _index, _color_red];
	};
	
	_i = _i + 1;
};



FUNCTION_EXIT_DOWNLOAD_THREAD = {
	if (_in_download_thread) then {
		_in_download_thread = false;
		if (_silent_mode) then {GS_SILENT_MODE_THREAD=false};
		GS_DOWNLOAD_THREADS = GS_DOWNLOAD_THREADS - 1;
	}
};