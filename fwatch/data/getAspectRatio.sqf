/* 
Function supplied with Fwatch v1.14
Returns dialog space format

Usage:
	_string = call preProcessFile "..\fwatch\data\getAspectRatio.sqf"

Also it executes "Aspect_Ratio.sqf" in order to create global variables AR_modifX and AR_modifY
If the file doesn't exist then it calculates values for these variables based on settings from memory
	
Optionally you may pass arguments:
"refresh" - update global variables AR_modifX, AR_modifY, use it after :mem setgraphics
"nochange" - do not update global variables, just return string

Changelog:
1.15
- requires 1.15
- calculates ratio instead of using predefined values
- initializes AR_modifX and AR_modifY
- returns e.g. "4:3" instead of "4_3"
*/



private ["_graphics", "_uiTopLeftX", "_uiTopLeftY", "_uiBottomRightX", "_uiBottomRightY", "_i", "_defaultRes", "_results", "_x", "_y", "_remainder", "_a", "_b", "_calculate", "_toFind"];



// Get resolution, fov and ui settings

	_graphics   	= call loadFile "\:MEM GETGRAPHICS";
	_uiTopLeftX 	= _graphics select 24;
	_uiTopLeftY 	= _graphics select 25;
	_uiBottomRightX = _graphics select 26;
	_uiBottomRightY = _graphics select 27;



// Find aspect ratio

	_i 			= -1;
	_defaultRes = [[800,600], [960,720], [1152,864]];
	_results 	= [0,0,0];
	_x  		= 0;
	_y 			= 0;

	// Repeat math couple of times to find greatest common divisor
	while "_i=_i+1;  _i < count _defaultRes" do
	{
		// Find resolution
		_x = ((_defaultRes select _i) select 0) / (_uiBottomRightX - _uiTopLeftX);
		_y = ((_defaultRes select _i) select 1) / (_uiBottomRightY - _uiTopLeftY);

		// Round result
		_remainder = (_x mod 1);
		_x 		   = _x - _remainder;
		if (_remainder >= 0.5) then {_x=_x+1};

		_remainder = (_y mod 1);
		_y 		   = _y - _remainder;
		if (_remainder >= 0.5) then {_y=_y+1};

		// Greatest common divisor
		_a = _x;
		_b = _y;
		
		while "_a != _b" do
		{
			if (_a > _b) then {_a=_a-_b} else {_b=_b-_a};
		};

		// If this is the largest gdc we found so far then save it
		if (_a > (_results select 0)) then
		{
			_results set [0, _a];
			_results set [1, _x];
			_results set [2, _y];
		};
	};

	

// Get resolution that matched with highest gdc

	_x = _results select 1;
	_y = _results select 2;
		
		
		
// If user doesn't have OFP Aspect Ratio package then calculate new modif values

	if (Format ["%1",_this] != "nochange") then
	{
		_calculate = Format ["%1",_this] == "refresh";
		
		if (!_calculate) then
		{
			_calculate = Format ["%1",call loadFile "\:IGSE LOAD  mode:execute  file:..\Aspect_Ratio.sqf"] != "true"
		};
		
		if (_calculate) then
		{
			AR_modifX = (_x * _uiTopLeftX) / (_x - 2 * (_x * _uiTopLeftX));
			AR_modifY = (_y * _uiTopLeftY) / (_y - 2 * (_y * _uiTopLeftY));
		};
	};
	
	

		
// Divide resolution by greatest common divisor

	_x = _x / (_results select 0);
	_y = _y / (_results select 0);
	
	
	
// Replace result in order to follow ratio naming convention

	_toFind	= 
	[
		[8,5,    16,10], 
		[5,3,    15,9], 
		[64,27,  21,9], 
		[4,1,    12,3], 
		[24,5,   48,10], 
		[5,1,    48,9],
		[15,8,   30,16], 
		[9,5,    27,15],
		[81,64,  27,21]
	];
	
	_i = -1;
	while "_i=_i+1;  _i < count _toFind" do
	{
		if (_x==((_toFind select _i) select 0)  &&  _y==((_toFind select _i) select 1)) then
		{
			_x = (_toFind select _i) select 2;
			_y = (_toFind select _i) select 3;
		};
		
		if (_y==((_toFind select _i) select 0)  &&  _x==((_toFind select _i) select 1)) then
		{
			_y = (_toFind select _i) select 2;
			_x = (_toFind select _i) select 3;
		};
	};

	
	
// Return string

	Format ["%1:%2", _x, _y]
