/* 
Function supplied with Fwatch v1.13
Returns difference in seconds between two calls of this function
Max result up to days

Usage:
	["FLIB_MEASURETIME"] call preProcessFile "..\fwatch\data\InitFLib.sqf"
	_seconds = <string> call FLIB_MEASURETIME

Argument <string> is optional. Pass "start" to reset stored date

Example:
	call FLIB_MEASURETIME
	~0.1
	_s = call FLIB_MEASURETIME
	
Changelog:
1.15
- initializes FLIB_MODIFYDATE
1.14
- fixed syntax error due to which function wasn't working at all
*/



private ["_return", "_DateThen", "_DateNow", "_i", "_then", "_now", "_max", "_multiplier", "_year", "_days"];
_return = 0;



// Reset time
if (Format ["%1",_this]=="start"  ||  Format ["%1",MEASURE_TIME_IN_SECONDS]=="scalar bool array string 0xfcffffef") then
{
	MEASURE_TIME_IN_SECONDS = call loadFile ":info date";
}


// Subtract saved time from current time
else
{
	if (Format["%1",FLIB_MODIFYDATE in []]=="bool") then
	{
		FLIB_MODIFYDATE = preProcessFile "..\fwatch\data\ModifyDate.sqf"
	};
			
	_DateThen =+ MEASURE_TIME_IN_SECONDS;
	_DateNow = call loadFile ":info date";
	_i = 7;
	
	while "_i >= 2" do
	{
		_then = _DateThen select _i;
		_now = _DateNow select _i;
		
		_max = 0;
		_multiplier = 1;
		if (_i==7) then {_max=1000; _multiplier=0.001};
		if (_i==6) then {_max=60};
		if (_i==5) then {_max=60; _multiplier=60};
		if (_i==4) then {_max=24; _multiplier=3600};
		if (_i==2) then 
		{
			_year = _array select 0;
			_days = [0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31];
			if (_year%4==0  &&  (_year%100!=0 || _year%400==0)) then {_days set [1, 29]};		//leap year
			_max = _days select (_DateThen select 1);
			_multiplier = 86400
		};
		
		// If value in the older date is smaller then just subtract
		if (_then < _now) then {_return=_return+(_now-_then)*_multiplier};
	
		// If value in the older date is larger then increment date
		if (_then > _now && _i>0) then
		{
			_return = _return + (_max-_then+_now) * _multiplier;		
			[_i-1, 1, _DateThen] call FLIB_MODIFYDATE
		};
	
		_i = _i - 1;
		if (_i==3) then {_i=_i-1};
	};
	
	_DateThen = nil;
	_DateNow = nil;
};


_return
