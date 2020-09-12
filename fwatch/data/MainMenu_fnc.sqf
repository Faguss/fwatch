// Function supplied with Fwatch v1.16
// Required to make MainMenu.sqs work

FUNCTION_FIND = {
	private ["_i", "_break"];
	_i     = 0;
	_break = false; 

	while "!_break" do {
		if (_i >= count (_this select 1)) then {
			_i     = -1; 
			_break = true
		} else {
			if ((_this select 0) == (_this select 1) select _i) then {
				_break = true
			} else {
				_i = _i + 1
			}
		}
	};

	_i
};



FUNCTION_MODS2STRING = {
	private ["_string", "_i", "_array", "_separator", "_mod_name", "_mod_version", "_index2", "_my_version"];
	_array     = _this select 0;
	_separator = _this select 1;
	_string    = ""; 
	_i         = 0;

	{
		_index = [_x, _all_modsID] call FUNCTION_FIND;
		if (_index >= 0) then {
			_mod_name      = "";
			_mod_forcename = false;
			_mod_size      = "";
			_mod_version   = -1;
			call (_all_mods select _index);
			_verTXT        = Format ["%1", _mod_version];

			if (_separator != ";") then {
				// Show version difference
				_index2 = [_x, FWATCH_MODLISTID] call FUNCTION_FIND;
				if (_index2 >= 0) then {
					_my_version = FWATCH_MODLISTVER select _index2;

					if (_my_version < _mod_version) then {
						_verTXT = Format [MAINMENU_STR select 50, _my_version, _mod_version]
					} else {
						_mod_size = "";
						_index2   = -1;
					}
				};
				
				_string = Format["%1%2", _string, _mod_name];
				
				// Don't display version if it's 1 unless there's an update
				if (_mod_version != 1  ||  _index2 >= 0) then {
					_string = Format ["%1 %2", _string, _verTXT]
				};

				if (_mod_size != "") then {
					_string = Format ["%1 - %2", _string, _mod_size]
				};
				
				_string = Format ["%1%2", _string, _separator]
			} else {
				_string = Format["%1%2%3", _string, _mod_name, (if (_i < count _array-1) then {_separator} else {""})]
			}
		};

		_i = _i + 1
	} 
	forEach _array; 

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
	private ["_index", "_add_to_missing", "_assign", "_my_version", "_mod_name", "_mod_sizearray", "_mod_version", "_index2", "_index3", "_j"];

	// _x is unique id of the missing mod. I need to find its index in the mod database so I can get name and version
	_index = [_x, _all_modsID] call FUNCTION_FIND;
	if (_index >= 0) then {
		_add_to_missing = true;
		_assign         = false;
		_my_version     = 0;
		_mod_name       = "";
		_mod_sizearray  = [];
		_mod_version    = 0;
		call (_all_mods select _index);

		// Look for the mod id in the user's mod list
		_index2 = [_x, FWATCH_MODLISTID] call FUNCTION_FIND;
		if (_index2 >= 0) then {
			_mod_name       = FWATCH_MODLIST select _index2;
			_my_version     = FWATCH_MODLISTVER select _index2;
			_add_to_missing = _my_version < _mod_version;				// If user has older version
		} else {
			_index3 = [_mod_name,FWATCH_MODLIST] call FUNCTION_FIND;
			if (_index3 >= 0) then {
				_assign = (FWATCH_MODLISTID select _index3) == "";		// If modfolder exists but not tagged
			}
		};
		
		if (_add_to_missing) then {
			_missing_mods           set [count _missing_mods          , _index     ];
			_missing_mods_id        set [count _missing_mods_id       , _x         ];
			_missing_mods_name      set [count _missing_mods_name     , _mod_name  ];
			_missing_mods_assign    set [count _missing_mods_assign   , _assign    ];
			_missing_mods_myversion set [count _missing_mods_myversion, _my_version];

			_j = -1; 
			while "_j=_j+1; _j<count _missing_mods_sizearray" do {
				_missing_mods_sizearray set [_j, (_missing_mods_sizearray select _j)+(_mod_sizearray select _j)]
			}
		}
	}
};



FUNCTION_QUICKSORTM = {
	private ["_array", "_arrays", "_currentARR", "_hi", "_i", "_j", "_k", "_lo", "_mid", "_temp"];

	if (Format ["%1",QUICKSORT_RECURRENCE] == "scalar bool array string 0xfcffffef") then {
		QUICKSORT_RECURRENCE = 1;
		_arrays              = _this;
		_array               = _arrays select 0;
		_lo                  = 0;
		_hi                  = count _array -1;
		_i                   = _lo;
		_j                   = _hi;
	} else {
		QUICKSORT_RECURRENCE = QUICKSORT_RECURRENCE + 1;	
		_arrays              = _this select 0;
		_array               = _arrays select 0;
		_lo                  = _this select 1;
		_hi                  = _this select 2;
		_i                   = _lo;
		_j                   = _hi;
	};

	_mid = (_lo + _hi) / 2;
	_mid = _mid - (_mid mod 1);
	_mid = _array select _mid;
		
	while "_i <= _j" do {
		while "(_array select _i) < _mid" do {_i=_i+1};
		while "(_array select _j) > _mid" do {_j=_j-1};
			
		if (_i <= _j) then {
			_k = 0;
			while "_k < count _arrays" do {
				_currentARR = _arrays select _k;
				_temp       = _currentARR select _i;
				_currentARR set [_i, (_currentARR select _j)];
				_currentARR set [_j, _temp];
				_arrays set [_k, _currentARR];
				_k = _k + 1;
			};
			
			_i = _i + 1;
			_j = _j - 1;
		};
	};

	if (_lo < _j) then {[_arrays, _lo, _j] call FUNCTION_QUICKSORTM};
	if (_i < _hi) then {[_arrays, _i, _hi] call FUNCTION_QUICKSORTM};
		
	QUICKSORT_RECURRENCE = QUICKSORT_RECURRENCE - 1;
	if (QUICKSORT_RECURRENCE == 0) then {QUICKSORT_RECURRENCE=nil};
};



FUNCTION_FORMAT_GAME_TIME = {
	private ["_i", "_eventType", "_length", "_eventChecked","_eventBegan","_startTime","_difference","_endTime","_DiffToStart","_DiffToEnd","_formatPattern"];
	
	_now          = call loadFile ":info date";
	_i            = -1;
	_eventChecked = [];	// hold "daily" and "weekly" events here so they don't get double-checked

	while "_i=_i+1; _i < count _this" do {
		_eventType = (_this select _i) select 0;
		_startTime = (_this select _i) select 1;
		_length    = (_this select _i) select 2;
		
		/*
		event types:
		0 - single
		1 - weekly
		2 - daily
		*/
		
		_eventBegan  = false;
		if (_eventType==2  &&  _i in _eventChecked) then {_eventBegan=true};
		
		// localize time
		["minutes", (_now select 8) - (_startTime select 8), _startTime] call FLIB_MODIFYDATE;
		_startTime set [8, _now select 8];
		
		// if it's a recurrent event
		if (_eventType>0  &&  !(_i in _eventChecked)) then {
			_DiffToStart = [_startTime,_now,"subtract"] call FLIB_DATEDIFF;
			
			// have we passed event start date?
			if ("_x<0" count _DiffToStart > 0) then {
				_eventBegan = true;
				
				// if so then find nearest occurence
				_difference = 
				if (_eventType == 1) then {
					if ((_startTime select 3) >= (_now select 3)) then {
						(_startTime select 3) - (_now select 3)
					} else {
						7 - (_now select 3) + (_startTime select 3)
					}
				};
			
				_startTime set [0, _now select 0];
				_startTime set [1, _now select 1];
				_startTime set [2, _now select 2];
			
				["day", _difference, _startTime] call FLIB_MODIFYDATE
			};
		};
		
		
		// Find when game will end
		_endTime =+ _startTime;
		["minutes", _length, _endTime] call FLIB_MODIFYDATE;
		
		
		// What's the difference between now and game time range
		_DiffToStart = [_startTime,_now,"subtract"] call FLIB_DATEDIFF;
		_DiffToEnd   = [_endTime,  _now,"subtract"] call FLIB_DATEDIFF;

		// if the event has not ended
		if ("_x<0" count _DiffToEnd == 0) then {
			// if the event has started
			if ("_x<0" count _DiffToStart > 0) then {_playingNow=true};
			
			// Format date description
			_formatPattern = "@d MM 0h:0i";
			
			if (_eventBegan && _eventType>0) then {
				//temporarily disabled
				//_formatPattern = Format ["@~%1~ 0h:0i", (if (_eventType==1) then {MAINMENU_STR select (51 + (_startTime select 3))} else {MAINMENU_STR select 58})]
			};
			
			// add event info to arrays
			_gameTimeDistance set [
				count _gameTimeDistance, 
				[_startTime, _now, "fraction"] call FLIB_DATEDIFFDAY
			];
			
			_gameTimeFormatted set [
				count _gameTimeFormatted, 
				Format ["%1 - %2", [_formatPattern,_startTime] call FLIB_FORMATDATE, ["@0h:0i",_endTime] call FLIB_FORMATDATE]
			];
			
			_gameTimeDates set [
				count _gameTimeDates,
				//["Y,m,d, ,h,i,s,l", _startTime] call FLIB_FORMATDATE
				_startTime
			];
			
			// if it's happening today
			if (([_startTime, _now, "ignorehour"] call FLIB_DATEDIFFDAY) < 1) then {
				_today_game_servers set [
					count _today_game_servers,
					_server_uniqueid
				]
			};
		} else {
			if (!(_i in _eventChecked)  &&  _eventType!=0) then {
				_eventChecked set [count _eventChecked, _i];
				
				// If this is a weekly event then try again next week
				// If this is a daily event then try again tomorrow
				["day", (if (_eventType==1) then {7} else {1}), _startTime] call FLIB_MODIFYDATE;
				
				_i = _i - 1
			};
		};
	};
	
	// If there's more than one valid game time then sort them
	if (count _gameTimeDistance > 1) then {
		[_gameTimeDistance, _gameTimeFormatted, _gameTimeDates] call FUNCTION_QUICKSORTM		
	}
};



FUNCTION_FIND_URL = {
	private ["_i", "_break", "_cut", "_results", "_array", "_find", "_array_item"];
	
	_i     = 0;
	_break = false; 

	while "!_break" do {
		if (_i >= count (_this select 1)) then {
			_i     = -1; 
			_break = true
		} else {
			// remove http://www.
			_array = call loadFile Format ["\:STRING DOMAIN %1", _this select 0];
			_find  = call loadFile Format ["\:STRING FIND text:%1find:%2", _this select 0, _array select 3];
			_cut   = loadFile Format ["\:STRING CUT start:%1  text:%2", _find select 0, _this select 0];
			
			// is domain inside other
			_array_item = (_this select 1) select _i;
			if (_array_item in [_array_item]) then {} else {_array_item = ((_this select 1) select _i) select 0};
			_results = call loadFile Format ["\:STRING FIND text:%1find:%2", _array_item, _cut];
			
			if (count _results > 0) then {
				_break = true
			} else {
				_i = _i + 1
			}
		}
	};

	_i
};



FUNCTION_GET_CLOSEST_GAME_TIME = {
	_server_uniqueid   = "";
	_server_game_times = []; 
	
	call _x; 
	
	_playingNow        = false; 
	_gameTimeDistance  = []; 
	_gameTimeFormatted = []; 

	_server_game_times call FUNCTION_FORMAT_GAME_TIME;

	// If there are valid game times then add first one to the list of closest games
	if (count _gameTimeDistance > 0) then {
		_closest_game_times set [
			count _closest_game_times, 
			(if (_playingNow) then {0} else {_gameTimeDistance select 0})
		]
	} else {
		// If all game times have elapsed then remove server from the list
		_all_servers set [_i, "<null>"];
		//_all_servers = _all_servers - ["<null>"];
		
		_all_serverNames set [_i, "<null>"];
		//_all_serverNames = _all_serverNames - ["<null>"]
		
		_all_serverID set [_i, "<null>"];
		_all_serverURL set [_i, "<null>"];
	};
	
	_i = _i + 1
};



FUNCTION_SERVERS_TO_LISTBOX = {	
	private ["_color1", "_color2", "_color3", "_color"];
	
	_color1 = [ 246, 40, 23 ];	// Fire Engine Red
	//_color2 = [ 255, 127, 80 ];	// Coral
	_color2 = [238, 154, 77  ]; // Sandy Brown
	_color3 = [ 201, 190, 98 ]; // Ginger Brown
	
	if ((_closest_game_times select _i) == 0) then {
		if (!_nowLabel) then {
			_nowLabel = true;
			_color    = []; 
			"_color=_color+[_x/255]" foreach _color1;
			lbSetColor [6657, lbAdd [6657, MAINMENU_STR select 59] ,_color + [1]]
		}
	} else {
		if (_x in _today_game_servers) then {
			if (!_todayLabel) then {
				_todayLabel = true;
				_color      = [];
				"_color=_color+[_x/255]" foreach _color2;
				lbSetColor [6657, lbAdd [6657, MAINMENU_STR select 60] ,_color + [1]]			
			}
		} else {
			if (!_upcomingLabel) then {
				_upcomingLabel = true;
				_color         = []; 
				"_color=_color+[_x/255]" foreach _color3;
				lbSetColor [6657, lbAdd [6657, MAINMENU_STR select 61] ,_color + [1]]
			}
		}
	};

	_entry = lbAdd [6657, _all_serverNames select _i]; 
	lbSetData  [6657, _entry, Format ["%1",_i]];
	lbSetValue [6657, _entry, 201]; 
	
	// After installation display that server options
	if (_x == _jump_to_server) then {
		_data = Format ["%1",_i];
		goto "DisplayServer"
	};
	
	_i = _i + 1
};



FUNCTION_GET_EXECUTE_PARAMS = {
	private ["_string", "_string2", "_ok", "_Input", "_add_param_name", "_modifier"];
	_string  = "";
	_string2 = "";
	
	if (_server_uniqueid != "mod") then {
		_modifier = if (_server_encrypted) then {"e"} else {""};

		_string = Format ["-serveruniqueid=%1 -%2connect=", _server_uniqueid, _modifier] + _server_ip;
		
		if (_server_port != "") then {_string=_string+Format[" -%1port=",_modifier]+_server_port};
		if (_server_password != "") then {_string=_string+Format[" -%1password=",_modifier]+_server_password};
		if (_server_equalModReq) then {_string=_string+" -serverequalmodreq=true"};
		if (_run_voice_program) then {_string=_string+Format[" -%1voice=",_modifier]+(_server_voice select 1)};
		if (_server_maxcustombytes != "") then {_string=_string+Format[" -maxcustom=%1 ""-plrname=%2""",_server_maxcustombytes, FWATCH_USERNAME]};
		
		_add_param_name = true;
		_string2 = "";

		{
			if (_add_param_name) then {
				_add_param_name = false;
				_string         = _string  + " -mod=";
				_string2        = _string2 + " -modid=";
			} else {
				_string         = _string  + ";";
				_string2        = _string2 + ";";
			};
			
			_index = [_x, _all_modsID] call FUNCTION_FIND;
			if (_index >= 0) then {
				_mod_name = "";
				call (_all_mods select _index);
				
				_string  = _string  + _mod_name;
				_string2 = _string2 + _x;
			}
		}
		forEach _server_modfolders;
	} else {
		_string = "-mod=" + (_missing_mods_name select 0);
	};
	
	_string + _string2
};



FUNCTION_FORMAT_ARRAY = {
	private ["_i", "_string", "_item"];

	_i      = -1;
	_string = "[";

	while "_i=_i+1; _i < (count _this)" do {
		_item   = _this select _i;
		_string = _string + "]+[";	

		// Check if scalar
		if (_item in [_item]) then {
			// Check if string
			if (_item in [Format["%1",_item]]) then {
				_string = _string + "{" + _item + "}";
			} else {
				_string = _string + Format ["%1",_item];
			}
		} else {
			_string = _string + (_item call FUNCTION_FORMAT_ARRAY);
		}
	};

	_string + "]"
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
	forEach _missing_mods;
	
	_ids + " " + _names
};



FUNCTION_BUILD_QUERY_STRING = {
	private ["_output", "_values", "_array", "_i", "_mod", "_ver"];
	_output = "";
	_values = "";
	_i      = -1;
	_array  = call loadFile Format ["\:STRING TOKENIZE  text:%1delimiter: ", _access_code];
	
	while "_i=_i+1; _i<count _array" do {
		_values = _values + (if (_i==0) then {""} else {","}) + (_array select _i);
	};
	
	if (_values != "") then {
		_output = _output + "&password=" + _values;
	};
	

	if ((_this select 0) == "modcheck") then {
		_mod = "";
		_ver = "";
		_i   = -1;

		while "_i=_i+1; _i<count (_this select 1)" do {
			_index = [(_this select 1) select _i, FWATCH_MODLISTID] call FUNCTION_FIND;
			
			if (_index >= 0) then {
				_mod = _mod + (if (_mod=="") then {""} else {","}) + (FWATCH_MODLISTID select _index);
				_ver = _ver + (if (_ver=="") then {""} else {","}) + Format["%1",(FWATCH_MODLISTVER select _index)];
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
	private ["_text", "_server_limit"];
	_text = "";
	
	if (count _server_maxcustomfilesize > 0) then {
		_too_big = (((FWATCH_CUSTOMSIZE select 2) * 1024 * 1024) + ((FWATCH_CUSTOMSIZE select 1) * 1024) + (FWATCH_CUSTOMSIZE select 0)) > (((_server_maxcustomfilesize select 2) * 1024 * 1024) + ((_server_maxcustomfilesize select 1) * 1024) + (_server_maxcustomfilesize select 0));
		
		_server_limit =
		if ("_x>0" count _server_maxcustomfilesize > 0) then {
			_server_maxcustomfilesize call FUNCTION_FORMAT_FILESIZE
		} else {
			MAINMENU_STR select 62
		};
	
		_text = 	
		if (_too_big) then {
			Format ["%1 - %2 (%3)", _server_limit, FWATCH_CUSTOMFILE, FWATCH_CUSTOMSIZE call FUNCTION_FORMAT_FILESIZE]
		} else {
			_server_limit
		};
	};
	
	_text
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
		FWATCH_MODLIST     = _ok select 4; 
		FWATCH_MODLISTID   = _ok select 5; 
		FWATCH_MODLISTVER  = _ok select 6; 
		FWATCH_MODLISTDATE = _ok select 7; 
		FWATCH_CUSTOMFILE  = _ok select 8; 
		FWATCH_CUSTOMSIZE  = _ok select 9; 
		FWATCH_MODLISTHASH = _ok select 10;
		FWATCH_USERNAME    = _ok select 11;
	} else {
		titleText [((MAINMENU_STR select 63)+(_ok select 3)),"PLAIN DOWN",0.1]
	}
};



FUNCTION_DOWNLOAD_INFO = {
	_title = _this select 0;
	_text  = _this select 1;

	lbSetValue [6657, lbAdd [6657,"["+_title+"]"], 0];
	lbSetCurSel [6657, (lbSize 6657)-1];

	"ctrlShow [_x,false]" forEach _serverDialog;
	"ctrlShow [_x,true ]" forEach [6460, 6463];
	
	if (_text != "compareversion") then {
		ctrlSetText [6463, _text];
	} else {
		ctrlSetText [6463, Format [MAINMENU_STR select 75, GS_VERSION, GS_MY_VERSION]];
	};
	
	lbSetValue [6657, lbAdd [6657,MAINMENU_STR select 64], 223];
};



FUNCTION_READ_DOWNLOADED_FILE = {
	private ["_error", "_fileContent","_fileIntegrity","_error_message","_file_name"];
	_error         = !(GS_DOWNLOAD_RESULT select 0);
	_error_message = MAINMENU_STR select 15;
	_file_name     = "downloadLog.txt";

	if (GS_DOWNLOAD_RESULT select 0) then {
		_fileContent   = loadFile "\:IGSE LOAD  mode:execute  file:..\fwatch\tmp\schedule\schedule.sqf";
		_fileIntegrity = loadFile ("\:STRING CUT start:-5 text:" + _fileContent);

		if (_fileIntegrity == ";true") then {
			call _fileContent;
		} else {
			_error = true;
			
			if (_fileContent != "") then {
				_error_message = MAINMENU_STR select 18;
				_file_name     = "schedule.sqf";
			};
		};
	};
	
	if (_error) then {
		if ((GS_DOWNLOAD_RESULT select 1) == 5) then {
			if ((GS_DOWNLOAD_RESULT select 2) == 186) then {
				GS_DOWNLOAD_RESULT set [3, (GS_DOWNLOAD_RESULT select 3)+"\n\nGo to fwatch.exe properties and set it to run as an administrator"];
			};
			
			[_error_message, GS_DOWNLOAD_RESULT select 3] call FUNCTION_DOWNLOAD_INFO;
		} else {
			if (_mirror<count (_all_url select _i)-1) then {
				lbSetValue [6657, lbAdd [6657,"["+_error_message+"]"], 0];
				_mirror = _mirror + 1;
				goto _this;
			} else {
				[_error_message,(GS_DOWNLOAD_RESULT select 3)+"\n\n"+(loadFile Format ["\:IGSE LOAD  mode:execute  file:..\fwatch\tmp\schedule\%1",_file_name])] call FUNCTION_DOWNLOAD_INFO;
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
	
	_url          = _this select 0;
	_id_list      = _this select 1;
	_ver_list     = _this select 2;
	_id_string    = "";
	_ver_string   = "";
	_i            = -1;
	
	_positions = call loadFile ("\:STRING FIND find:/text:"+_url);
	if (count _positions > 0) then {
		_url = loadFile (Format ["\:STRING CUT end:%1text:", _positions select (count _positions-1)] + _url);
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
			"Interfejs niedostкpny. Sprawdџ swуj bin\resource.cpp.",	//0
			"Plik bin\resource.cpp jest przestarzaіy. Њci№gnij now№ wersjк OFP Aspect Ratio",	//1
			"PLAN ROZGRYWEK",	//2
			"SERWER PRZEGLҐDARKI GIER",	//3
			"Jest juї wі№czony!",		//4
			"Napisz hasіa do prywatnych serwerуw",	//5
			"[Uruchom bez modуw]",		//6
			"[Dodaj nowy]",				//7
			"Nie ma nic do zapisania!",	//8
			"Jest juї na liњcie!", //9
			"[Proszк czekaж]",	//10
			"[Њci№gnij %1]",	//11
			"[Otwуrz zaproszenie do %1]",	//12
			"Nie udaіo siк utworzyж fwatch\tmp\schedule\n%1\n\nUruchom grк jako administrator",	//13
			"[Sprawdzanie aktualizacji]",	//14
			"Nie udaіo siк pobraж",			//15
			"[Pobieranie planu]",	//16
			"[Przetwarzanie danych]",	//17
			"Niepoprawne dane",		//18
			"[Brak serwerуw]",		//19
			"Niepoprawna wersja planu",	//20
			"[Њci№ganie loga %1]",	//21
			"Nie udaіo siк pobraж loga\n%1",	//22
			"[Ukіadanie serwerуw]",	//23
			"[Odwoіaj zaplanowane podі№czenie]",	//24
			"[Pokaї prywatne gry]",	//25
			"[Doі№cz o czasie]",	//26
			"[Doі№cz]",	//27
			"[Њci№gnij wymagane mody %1]",	//28
			"[Pokaї zmiany w modach]",		//29
			"[Wejdџ na stronк serwera]",		//30
			"[Dodatkowe opcje uruchamiania]",	//31
			"[Cofnij]",	//32
			"[Uruchom %1]",	//33
			"[Bez]",	//34
			"",	//35
			"[Przerwij]",	//36
			"Juї masz folder %1. Czy wolisz:\n\n- zainstalowaж now№ kopiк (bezpieczna opcja; nazwa obecnego modu zostanie zmieniona)\n\nLUB\n\n- Oznaczyж obecny mod jako wersjк %2 (szybsza opcja; dane moda bкd№ podlegaж aktualizacjom)",	//37
			"[Њci№gnij now№ kopiк]",	//38
			"[Oznacz aktualn№]",		//39
			"Nie udaіo siк uruchomiж fwatch\data\addonInstaller.exe\n%1",	//40
			"Nie udaіo siк utworzyж pliku\n%1",	//41
			"[Uruchom ponownie po skoсczeniu: %1]",		//42
			"TAK",	//43
			"NIE",	//44
			"\n\n\nFwatch nie widzi instalatora.\nZignoruj ten komunikat jeњli widaж postкp instalacji.\n\nW innym przypadku sprawdџ w menedrzeїe zadaс czy jest addonInstaller.exe; przerwij instalacjк; zobacz fwatch\data\addonInstallerLog.txt",	//45
			"Serwer wymaga dokіadnie tych samych modуw wiкc argument -mod zostanie pominiкty",	//46
			"Gra zostanie uruchomiona o\n%1\n\ni podіaczy siк automatycznie do\n%2",	//47
			"[Aktualizuj %1]",	//48
			"Jest dostpкpna aktualizacja %1. Musisz uaktualniж.\n\n\n       Nowa wersja:\n           %2\n\n       Twoja wersja:\n           %3\n\n\nDwuklik na opcjк їeby zacz№ж proces. Instalator zamknie grк, њci№gnie now№ wersjк, zamieni dane i uruchomi grк ponownie.", //49
			"%1 do %2",	//50
			"Co niedzielк",		//51		
			"Co poniedziaіek",		//52	
			"Co wtorek",	//53		
			"Co њrodк",	//54	
			"Co czwartek",	//55
			"Co pi№tek",		//56
			"Co sobotк",	//57	
			"Codziennie",	//58
			"==Teraz==",	//59
			"==Dzisiaj==",	//60
			"==Wkrуtce==",	//61
			"niedozwolone",	//62
			"BЈҐD:\n",	//63
			"[Kontynuuj]",	//64
			"Wersja:",	//65
			"Mody:",	//66
			"Wіasne pliki:",	//67
			"Rozkіad:",	//68
			"Rozmowy:",		//69
			"Jкzyki:",	//70
			"Poіoїenie:",	//71
			"Strona:",		//72
			"Uwagi:",		//73
			"Wpisz tekst:",	//74
			"Њci№gnij wersjк:\n           %1\n\nTwoja wersja:\n           %2",	//75
			"Nie moїesz zaktualizowaж %1 bo jest wі№czony.\n\nCzy chciaіbyњ uruchomiж grк bez modуw?",	//76
			"Nie moїesz zaktualizowaж %1 bo s№ wі№czone.\n\nCzy chciaіbyњ uruchomiж grк bez modуw?",	//77
			"Plan rozgrywek",	//78
			"Serwer przegl№darki gier",	//79		
			"nowa wersja testowa Fwatch 1.16",	//80
			"nowa wersja Resource.cpp",		//81
			"[Њci№gnij mody]"				//82
		];
	};
	
	if (CURRENT_LANGUAGE == "Russian") then {
		MAINMENU_STR = [
			"Ошибка при создании диалогового окна. Проверьте bin\resource.cpp",		//0
			"Файл bin\resource.cpp устарел. Загрузите новую версию пака OFP Aspect Ratio",		//1
			"РАСПИСАНИЕ ИГР",		//2
			"МАСТЕР СЕРВЕР",		//3
			"Текущий адрес!",		//4
			"Введите пароль, чтобы просмотреть игры",		//5
			"[Запуск без модов]",		//6
			"[Добавить Новый]",		//7
			"Нет данных для сохранения!",		//8
			"Уже есть в списке!",		//9
			"[Пожалуйста, подождите]",		//10
			"[Скачать %1]",		//11
			"[Пригласительная ссылка %1]",		//12
			"Ошибка при создании папки fwatch\tmp\schedule\n%1\n\nЗапустите игру от имени администратора",		//13
			"[Поиск обновлений]",		//14
			"Ошибка при загрузке",		//15
			"[Загрузка расписания]",		//16
			"[Считывание информации]",		//17
			"Ошибка при считывании",		//18
			"[Нет серверов]",		//19
			"[Неверная версия расписания]",		//20
			"[Загрузка аватара %1]",		//21
			"Ошибка при загрузке\n%1",		//22
			"[Сортировка расписания]",		//23
			"[Отменить автоматическое подключение]",		//24
			"[Показать частные серверы]",		//25
			"[Подключиться автоматически]",		//26
			"[Подключиться]",		//27
			"[Скачать моды %1]",		//28
			"[Просмотреть историю изменений]",		//29
			"[Посетить веб-сайт]",		//30
			"[Дополнительные параметры запуска]",		//31
			"[Назад]",		//32
			"[С %1]",		//33
			"[Без %1]",		//34
			"",		//35
			"[Отмена]",		//36
			"Уже имеется папка с модом %1. \n\n- Загрузить новую папку (безопасный режим; текущая папка будет переименована)\n\nИЛИ\n\n- Добавить идентификатор версии %2 к текущей папке (быстрый режим; мод может быть изменен в дальнейшем)?",		//37
			"[Загрузить новую папку]",		//38
			"[Добавить идентификатор]",		//39
			"Ошибка при запуске fwatch\data\addonInstaller.exe\n%1",		//40
			"Ошибка при создании файла\n%1",		//41
			"[Перезапуск после окончания: %1]",		//42
			"ВКЛ",		//43
			"ВЫКЛ",		//44
			"\n\n\nFwatch не обнаружил мастера установки.\nIf the progress is still going then ignore this message.\n\nOtherwise open Task Manager and check if addonInstaller.exe is running; abort installation; see addonInstallerLog.txt",		//45
			"Необходимы одинаковые моды для подключения (параметр -mod при запуске не учитывается)",		//46
			"Игра будет перезапущена в\n%1\n\nчтобы подключиться к серверу\n%2",		//47
			"[Обновить %1]",		//48
			"Доступна %1. Обновите, чтобы продолжить.\n\n\n       Новая версия:\n           %2\n\n       Текущая версия:\n           %3\n\n\nНажмите два раза, чтобы запустить обновление. При обновлении игра перезапустится.",		//49
			"%1 к %2",		//50
			"Каждое воскресенье",		//51
			"Каждый понедельник",		//52
			"Каждый вторник",		//53
			"Каждую среду",		//54
			"Каждый четверг",		//55
			"Каждую пятницу",		//56
			"Каждую субботу",		//57
			"Каждый день",		//58
			"==Текущая==",		//59
			"==Cегодня==",		//60
			"==Будущие==",		//61
			"запрещены",		//62
			"ОШИБКА:\n",		//63
			"[Продолжить]",		//64
			"Версия:",		//65
			"Моды:",		//66
			"Макс. файл:",		//67
			"Расписание:",		//68
			"Голос. чат:",		//69
			"Языки:",		//70
			"Регион:",		//71
			"Веб-сайт:",		//72
			"Примечание:",		//73
			"Введите текст:",		//74
			"Загружена версия:\n           %1\n\nВаша версия:\n           %2",		//75
			"Невозможно обновить %1, пока он включен.\n\nПерезапустить игру?",		//76
			"Невозможно обновить %2, пока они включены.\n\nПерезапустить игру?",	//77
			"Расписание Игр",		//78
			"Мастер Сервер",	//79		
			"новая тестовая версия Fwatch 1.16",	//80
			"новая версия Resource.cpp",		//81
			"[Скачать моды]"		//82
		];
	};
	
	if (Format["%1", count MAINMENU_STR] == "scalar") then {
		MAINMENU_STR = [
			"Failed to create dialog. Verify your bin\resource.cpp.",	//0
			"File bin\resource.cpp is outdated. Download new OFP Aspect Ratio pack version",	//1
			"GAME SCHEDULE",	//2
			"MASTER SERVER",	//3
			"This is the current one!",		//4
			"Type in password(s) to show private game(s)",	//5
			"[Start Without Mods]",		//6
			"[Add New]",				//7
			"There's nothing to save!",	//8
			"It's already on the list!", //9
			"[Please wait]",	//10
			"[Download %1]",	//11
			"[Open %1 Invite]",	//12
			"Failed to create directory fwatch\tmp\schedule\n%1\n\nSet the game to run as admin",	//13
			"[Checking for updates]",	//14
			"Download Failed",			//15
			"[Downloading schedule]",	//16
			"[Reading data]",	//17
			"Invalid Data",		//18
			"[No servers]",		//19
			"Incorrect schedule version",	//20
			"[Downloading logo %1]",	//21
			"Failed to download logo\n%1",	//22
			"[Sorting game times]",	//23
			"[Cancel Auto Connect]",	//24
			"[Show Private Servers]",	//25
			"[Auto-Connect on Time]",	//26
			"[Connect]",	//27
			"[Download Mods %1]",	//28
			"[View Changelog]",		//29
			"[Visit Website]",		//30
			"[Extra Startup Parameters]",	//31
			"[Back]",	//32
			"[With %1]",	//33
			"[Without]",	//34
			"",	//35
			"[Abort]",	//36
			"You already have %1 modfolder. Would you like to:\n\n- Install a new copy (safer option; current one will be renamed)\n\nOR\n\n- Tag the existing one as version %2 (faster option; mod will be a subject to future modifications)",	//37
			"[Download New]",	//38
			"[Assign ID]",		//39
			"Failed to launch fwatch\data\addonInstaller.exe\n%1",	//40
			"Failed to generate a file\n%1",	//41
			"[Restart when done: %1]",		//42
			"ON",	//43
			"OFF",	//44
			"\n\n\nFwatch can't detect installator.\nIf the progress is still going then ignore this message.\n\nOtherwise open Task Manager and check if addonInstaller.exe is running; abort installation; see addonInstallerLog.txt",	//45
			"Server requires exact mods so the -mod parameter will be ignored",	//46
			"Game will be restarted at\n%1\n\nin order to connect to\n%2",	//47
			"[Update %1]",	//48
			"There is a %1 available. You must update in order to continue.\n\n\n       New version:\n           %2\n\n       Your version:\n           %3\n\n\nDouble-click on the option to start the process. Update program will close the game, download the patch, replace files and then start the game again.", //49
			"%1 to %2",	//50
			"Every Sunday",		//51		
			"Every Monday",		//52	
			"Every Tuesday",	//53		
			"Every Wednesday",	//54	
			"Every Thursday",	//55
			"Every Friday",		//56
			"Every Saturday",	//57	
			"Daily",	//58
			"==Now==",	//59
			"==Today==",	//60
			"==Upcoming==",	//61
			"none allowed",	//62
			"ERROR:\n",	//63
			"[Continue]",	//64
			"Version:",	//65
			"Modfolders:",	//66
			"Custom File:",	//67
			"Game time:",	//68
			"VOIP:",		//69
			"Languages:",	//70
			"Location:",	//71
			"Website:",		//72
			"Message:",		//73
			"Enter Text:",	//74
			"Downloaded version:\n           %1\n\nYour version:\n           %2",	//75
			"You cannot update %1 while you have it loaded.\n\nWould you like to restart the game?",	//76
			"You cannot update %1 while you have them loaded.\n\nWould you like to restart the game?",	//77
			"Game Schedule",	//78
			"Master Server",	//79		
			"new test version of Fwatch 1.16",	//80
			"new Resource.cpp version",		//81
			"[Download Mods]"				//82
		];
	};
};
