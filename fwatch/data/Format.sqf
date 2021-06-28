/* 
Function supplied with Fwatch v1.16
Converts any data type to a string

Usage:
	["FLIB_FORMAT"] call preProcessFile "..\fwatch\data\InitFLib.sqf"
	<data> call FLIB_FORMAT

Example:
	["1","2","3"] call FLIB_FORMAT
*/

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
		_string = _string + (_item call FLIB_FORMAT);
	}
};

_string + "]"