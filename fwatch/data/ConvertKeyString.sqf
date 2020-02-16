/* 
Function supplied with Fwatch v1.13
Converts:
- input codes to Fwatch strings
- Fwatch strings to localized strings

Usage:
	["FLIB_CONVERTKEY"] call preProcessFile "..\fwatch\data\InitFLib.sqf"
	<string> call FLIB_CONVERTKEY
	<arrayName> call FLIB_CONVERTKEY
	
Output data type is the same as input.
	
Function will modify given array. If you want to keep the original 
then make a copy before using this function. Example:
	_arrayOLD = ["TAB", "1"]
	_arrayNEW =+ _arrayOLD;
	_arrayNEW call FLIB_CONVERTKEY
	
You can add string "localize" to input array to convert input codes directly to localized strings. Example:
	[20,"localize"] call FLIB_CONVERTKEY
	
Changelog:
1.14
- can add string "localize"
*/


private ["_dikCodes", "_fwatchStrings", "_localStrings", "_i", "_index", "_find", "_localize"];

// Database
_dikCodes = [1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,86,87,88,100,101,102,112,115,121,123,125,126,141,144,145,146,147,148,149,150,151,153,156,157,160,161,162,164,174,176,178,179,181,183,184,197,199,200,201,203,205,207,208,209,210,211,219,220,221,222,223,227,229,230,231,232,233,234,235,236,237,65536,65537,65538,65539,65540,65541,65542,65543,131072,131073,131074,131075,131076,131077,131078,131079,196608,196609,196610,196611,196612,196613,196614,196615,262144,262145,262146,262147,262148,262149,262150,262151];
_fwatchStrings = ["ESC","1","2","3","4","5","6","7","8","9","0","-","=","BACKSPACE","TAB","Q","W","E","R","T","Y","U","I","O","P","[","]","ENTER","LCTRL","A","S","D","F","G","H","J","K","L",";","'","~","LSHIFT","\","Z","X","C","V","B","N","M",",",".","/","RSHIFT","MULTIPLY","LALT","SPACE","CAPSLOCK","F1","F2","F3","F4","F5","F6","F7","F8","F9","F10","NUMLOCK","SCROLLLOCK","NUMPAD7","NUMPAD8","NUMPAD9","SUBTRACT","NUMPAD4","NUMPAD5","NUMPAD6","ADD","NUMPAD1","NUMPAD2","NUMPAD3","NUMPAD0","DECIMAL","OEM102","F11","F12","F13","F14","F15","KANA","ABNT_C1","CONVERT","NONCONVERT","YEN","ABNT_C2","NUMPAD=","MEDIAPREV","&",":","_","KANJI","STOP","AX","UNLABELED","MEDIANEXT","ENTER","RCTRL","VOLUMEMUTE","CALCULATOR","MEDIAPLAY","MEDIASTOP","VOLUMEDOWN","VOLUMEUP","WEBHOME","DECIMAL","DIVIDE","PRINTSCREEN","RALT","PAUSE","HOME","UP","PAGEUP","LEFT","RIGHT","END","DOWN","PAGEDOWN","INSERT","DELETE","LWIN","RWIN","APPS","POWER","SLEEP","WAKE","WEBSEARCH","WEBFAVORITES","WEBREFRESH","WEBSTOP","WEBFORWARD","WEBBACK","MYCOMPUTER","MAIL","MEDIASELECT","LBUTTON","RBUTTON","MBUTTON","4BUTTON","5BUTTON","6BUTTON","7BUTTON","8BUTTON","JOY1","JOY2","JOY3","JOY4","JOY5","JOY6","JOY7","JOY8","AXISX","AXISY","AXISZ","AXISXROT","AXISYROT","AXISZROT","SLIDER1","SLIDER2","JOYPOVUP","JOYPOVUPRIGHT","JOYPOVRIGHT","JOYPOVDOWNRIGHT","JOYPOVDOWN","JOYPOVDOWNLEFT","JOYPOVLEFT","JOYPOVUPLEFT"];
_localStrings = ["STR_DIK_ESCAPE","STR_DIK_1","STR_DIK_2","STR_DIK_3","STR_DIK_4","STR_DIK_5","STR_DIK_6","STR_DIK_7","STR_DIK_8","STR_DIK_9","STR_DIK_0","STR_DIK_MINUS","STR_DIK_EQUALS","STR_DIK_BACK","STR_DIK_TAB","STR_DIK_Q","STR_DIK_W","STR_DIK_E","STR_DIK_R","STR_DIK_T","STR_DIK_Y","STR_DIK_U","STR_DIK_I","STR_DIK_O","STR_DIK_P","STR_DIK_LBRACKET","STR_DIK_RBRACKET","STR_DIK_RETURN","STR_DIK_LCONTROL","STR_DIK_A","STR_DIK_S","STR_DIK_D","STR_DIK_F","STR_DIK_G","STR_DIK_H","STR_DIK_J","STR_DIK_K","STR_DIK_L","STR_DIK_SEMICOLON","STR_DIK_APOSTROPHE","STR_DIK_GRAVE","STR_DIK_LSHIFT","STR_DIK_BACKSLASH","STR_DIK_Z","STR_DIK_X","STR_DIK_C","STR_DIK_V","STR_DIK_B","STR_DIK_N","STR_DIK_M","STR_DIK_COMMA","STR_DIK_PERIOD","STR_DIK_SLASH","STR_DIK_RSHIFT","STR_DIK_MULTIPLY","STR_DIK_LMENU","STR_DIK_SPACE","STR_DIK_CAPITAL","STR_DIK_F1","STR_DIK_F2","STR_DIK_F3","STR_DIK_F4","STR_DIK_F5","STR_DIK_F6","STR_DIK_F7","STR_DIK_F8","STR_DIK_F9","STR_DIK_F10","STR_DIK_NUMLOCK","STR_DIK_SCROLL","STR_DIK_NUMPAD7","STR_DIK_NUMPAD8","STR_DIK_NUMPAD9","STR_DIK_SUBTRACT","STR_DIK_NUMPAD4","STR_DIK_NUMPAD5","STR_DIK_NUMPAD6","STR_DIK_ADD","STR_DIK_NUMPAD1","STR_DIK_NUMPAD2","STR_DIK_NUMPAD3","STR_DIK_NUMPAD0","STR_DIK_DECIMAL","STR_DIK_OEM_102","STR_DIK_F11","STR_DIK_F12","STR_DIK_F13","STR_DIK_F14","STR_DIK_F15","STR_DIK_KANA","STR_DIK_ABNT_C1","STR_DIK_CONVERT","STR_DIK_NOCONVERT","STR_DIK_YEN","STR_DIK_ABNT_C2","STR_DIK_NUMPADEQUALS","STR_DIK_PREVTRACK","STR_DIK_AT","STR_DIK_COLON","STR_DIK_UNDERLINE","STR_DIK_KANJI","STR_DIK_STOP","STR_DIK_AX","STR_DIK_UNLABELED","STR_DIK_NEXTTRACK","STR_DIK_NUMPADENTER","STR_DIK_RCONTROL","STR_DIK_MUTE","STR_DIK_CALCULATOR","STR_DIK_PLAYPAUSE","STR_DIK_MEDIASTOP","STR_DIK_VOLUMEDOWN","STR_DIK_VOLUMEUP","STR_DIK_WEBHOME","STR_DIK_NUMPADCOMMA","STR_DIK_DIVIDE","STR_DIK_SYSRQ","STR_DIK_RMENU","STR_DIK_PAUSE","STR_DIK_HOME","STR_DIK_UP","STR_DIK_PRIOR","STR_DIK_LEFT","STR_DIK_RIGHT","STR_DIK_END","STR_DIK_DOWN","STR_DIK_NEXT","STR_DIK_INSERT","STR_DIK_DELETE","STR_DIK_LWIN","STR_DIK_RWIN","STR_DIK_APPS","STR_DIK_POWER","STR_DIK_SLEEP","STR_DIK_WAKE","STR_DIK_WEBSEARCH","STR_DIK_WEBFAVORITES","STR_DIK_WEBREFRESH","STR_DIK_WEBSTOP","STR_DIK_WEBFORWARD","STR_DIK_WEBBACK","STR_DIK_MYCOMPUTER","STR_DIK_MAIL","STR_DIK_MEDIASELECT","STR_INPUT_DEVICE_MOUSE_0","STR_INPUT_DEVICE_MOUSE_1","STR_INPUT_DEVICE_MOUSE_2","STR_INPUT_DEVICE_MOUSE_3","STR_INPUT_DEVICE_MOUSE_4","STR_INPUT_DEVICE_MOUSE_5","STR_INPUT_DEVICE_MOUSE_6","STR_INPUT_DEVICE_MOUSE_7","STR_INPUT_DEVICE_STICK_0","STR_INPUT_DEVICE_STICK_1","STR_INPUT_DEVICE_STICK_2","STR_INPUT_DEVICE_STICK_3","STR_INPUT_DEVICE_STICK_4","STR_INPUT_DEVICE_STICK_5","STR_INPUT_DEVICE_STICK_6","STR_INPUT_DEVICE_STICK_7","STR_INPUT_DEVICE_STICK_AXIS_X","STR_INPUT_DEVICE_STICK_AXIS_Y","STR_INPUT_DEVICE_STICK_AXIS_Z","STR_INPUT_DEVICE_STICK_ROT_X","STR_INPUT_DEVICE_STICK_ROT_Y","STR_INPUT_DEVICE_STICK_ROT_Z","STR_INPUT_DEVICE_STICK_SLIDER_1","STR_INPUT_DEVICE_STICK_SLIDER_2","STR_INPUT_DEVICE_POV_N","STR_INPUT_DEVICE_POV_NE","STR_INPUT_DEVICE_POV_E","STR_INPUT_DEVICE_POV_SE","STR_INPUT_DEVICE_POV_S","STR_INPUT_DEVICE_POV_SW","STR_INPUT_DEVICE_POV_W","STR_INPUT_DEVICE_POV_NW"];

_i = 0;
_index = -1;
_find = "private [""_i"", ""_break""]; _i=0; _break=false; while ""!_break"" do {if (_i >= count (_this select 1)) then {_i=-1; _break=true} else {if((_this select 0)==(_this select 1) select _i)then{_break=true}else{_i=_i+1}}};_i";


// Passed scalar
if (_this in [_this]) then
{
	// Convert direct input code to fwatch input string
	if (_this in _dikCodes) then 
	{
		_index = [_this,_dikCodes] call _find;
		_this = _fwatchStrings select _index;
	}
	// Convert fwatch input string to a descriptive localized string
	else
	{
		if (_this in _fwatchStrings) then
		{
			_index = [_this,_fwatchStrings] call _find;
			_this = localize (_localStrings select _index);
		}
	}
}


// Passed array
else
{
	_localize = if ("localize" in _this) then {_this=_this-["localize"]; true} else {false};
	
	while "_i < count _this" do
	{
		// Convert direct input code to fwatch input string
		if ((_this select _i) in _dikCodes) then 
		{
			_index = [_this select _i, _dikCodes] call _find;
			if (!_localize) then
			{
				_this set [_i, _fwatchStrings select _index]
			}
			else
			{
				_this set [_i, localize (_localStrings select _index)];
			}
		}
		// Convert fwatch input string to a descriptive localized string
		else
		{
			if ((_this select _i) in _fwatchStrings) then
			{
				_index = [_this select _i, _fwatchStrings] call _find;
				_this set [_i, localize (_localStrings select _index)];
			}
		};
	
		_i = _i + 1
	};
};

_this

