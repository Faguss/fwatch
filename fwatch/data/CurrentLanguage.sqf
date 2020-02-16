/* 
Function supplied with Fwatch v1.13
Reads current game language and stores it in the CURRENT_LANGUAGE global variable
Default is "English"

Usage:
	["FLIB_CURRENTLANG"] call preProcessFile "..\fwatch\data\InitFLib.sqf"
	call FLIB_CURRENTLANG
*/


if (Format ["%1",CURRENT_LANGUAGE] == "scalar bool array string 0xfcffffef") then
{
	private ["_file", "_fwatch_error"];
	
	_file = "flashpoint.cfg";
	if ((call loadFile ":info version extended") select 1) then {_file="ColdWarAssault.cfg"};
	
	_fwatch_error = [true,0,0,""];
	CURRENT_LANGUAGE = call loadFile Format [":file read2 %1 language", _file];
	if (!(_fwatch_error select 0)) then {CURRENT_LANGUAGE="English"};
};

CURRENT_LANGUAGE