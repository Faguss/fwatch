/* 
Function supplied with Fwatch v1.16
Converts any data type to a string

Usage:
	["FLIB_FORMAT"] call preProcessFile "..\fwatch\data\InitFLib.sqf"
	<data> call FLIB_FORMAT

Example:
	["1","2","3"] call FLIB_FORMAT
*/

private ["_string"];

if (_this in [_this]) then {	// if scalar
	if (_this in [Format["%1",_this]]) then {	// if string
		"{" + _this + "}"
	} else {
		Format ["%1",_this]	// if other
	}
} else {	// if array
	_string = "[";
	{_string = _string + "]+[" + (_x call FLIB_FORMAT)} forEach _this;
	_string + "]"
}