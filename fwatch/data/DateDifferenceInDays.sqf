/* 
Function supplied with Fwatch v1.13
Calculates difference between two dates and returns result in days

Usage:
	["FLIB_DATEDIFFDAY"] call preProcessFile "..\fwatch\data\InitFLib.sqf"
	_days = [<datearray>,<datearray>] call FLIB_DATEDIFFDAY

Activate options by adding string to the argument array:

	"subtract"
	for a negative result when first date is older
	
	"ignorehour"
	only look at the date to count difference
	
	"fraction"
	convert difference in hours into a fraction of a day
	
Input: date array
	0 - year
	1 - month
	2 - day
	3 - day name
	4 - hours
	5 - minutes
	6 - seconds
	7 - miliseconds
	8 - timezone
	9 - is systime
	
Changelog:
1.16
- added "ignorehour" and "fraction" options
- fixed a bug where it would loop infinitely when comparing from 31st day of the month

1.15
- added "subtract" option
- initializes FLIB_MODIFYDATE
*/

private ["_array1", "_array2", "_i", "_same", "_TotalDays", "_addedExtraDay", "_DateThen", "_DateNow", "_then", "_now", "_max", "_days", "_year", "_subtract", "_fraction", "_ignoreHour", "_divider"];


if (Format["%1",FLIB_MODIFYDATE in []]=="bool") then
{
	FLIB_MODIFYDATE = preProcessFile "..\fwatch\data\ModifyDate.sqf"
};

// Options
_subtract   = false;
_fraction   = false;
_ignoreHour = false;

if (count _this > 2) then
{
	if ("subtract" in _this) then {_subtract=true};
	if ("fraction" in _this) then {_fraction=true};
	if ("ignorehour" in _this) then {_ignoreHour=true};
};


// Check time zone difference
_array1 =+ (_this select 0);
_array2 =+ (_this select 1);
_then   = _array1 select 8;
_now    = _array2 select 8;

// If there is difference then compensate
if (_then != _now) then
{
	// If 1st date is western then make 2nd date approach it
	if (_then > _now) then {[5, _then-_now, _array2] call FLIB_MODIFYDATE};
	
	// If 2nd date is western then make 1st date approach it
	if (_now > _then) then {[5, _now-_then, _array1] call FLIB_MODIFYDATE};
};




// Check if there is difference between dates
_i             = 0;
_same          = true;
_TotalDays     = 0;
_addedExtraDay = false;
_DateThen      = _array1;
_DateNow       = _array2;

while "_i<=7" do
{
	_then = (_this select 0) select _i;
	_now  = (_this select 1) select _i;
	
	// If first date is older
	if (_then < _now) then {_same=false};
	
	// If first date is more recent
	if (_then > _now) then
	{
		_DateThen =+ (_this select 1);
		_DateNow  = (_this select 0);
		_subtract = false;
		_same     = false
	};
	
	// Continue loop if both elements are the same
	if (_then == _now) then 
	{
		_i = _i + 1; 
		if (_i==3) then {_i=4}
	} 
	else {_i=8};
};




// Convert time difference to a fraction of a day
if (_fraction  &&  !_same) then
{
	_i = 8;
	while "_i=_i-1; _i>=4" do
	{
		_then = _DateThen select _i;
		_now  = _DateNow select _i;
		
		if (_now != _then) then
		{
			_max     = 24;
			_divider = 24;
			if (_i == 5) then {_max=60; _divider=1440};
			if (_i == 6) then {_max=60; _divider=86400};
			if (_i == 7) then {_max=1000; _divider=86400000};
			
			_TotalDays = _TotalDays + 
			(
				(
					(
						if (_now>_then) then {_now} else {_max}
					)
					- _then
				)
				/ _divider
			);
		};
	};
};




// Find differences between two arrays
_i = 7;
while "!_same  &&  _i>=0" do
{
	_then = _DateThen select _i;
	_now  = _DateNow select _i;
	
	// Determine maximal days/months value
	_max = 0;
	if (_i == 2) then 
	{
		_now  = _now - 1;
		_then = _then - 1;		//convert to 0-XX
		
		_days = [31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31];
		_year = _DateThen select 0;
		
		if (_year%4==0  &&  (_year%100!=0 || _year%400==0)) then {_days set [1, 29]};		//leap year
		
		_max = _days select ((_DateThen select 1)-1);
	};
	
	if (_i == 1) then 
	{
		_now  = _now - 1;
		_then = _then - 1; 	//convert to 0-11
		_max  = 12
	};


	// If milliseconds, seconds, minutes and hours make any difference
	if (_i>2  &&  _then>_now  &&  !_ignoreHour) then {[_i-1, 1, _DateThen] call FLIB_MODIFYDATE};
	
	
	// Add days
	if (_i == 2) then
	{
		if (_then < _now) then {_TotalDays=_TotalDays+(_now-_then)};
	
		if (_then > _now   &&   _i>0) then
		{
			_TotalDays = _TotalDays + (_max-_then+_now);
			[_i, (_max-_then+_now), _DateThen] call FLIB_MODIFYDATE
		};
	};
	

	// Extract days from months
	if (_i == 1) then
	{	
		while "_then != _now" do
		{
			_days = [31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31];
			_year = _DateThen select 0;
			
			if (_year%4==0  &&  (_year%100!=0 || _year%400==0)) then {_addedExtraDay=true; _days set [1, 29]};		//leap year
			
			_TotalDays = _TotalDays + (_days select _then);
			[_i, 1, _DateThen] call FLIB_MODIFYDATE;
			_then = (_DateThen select _i) - 1
		};
	};
	
	
	// Extract days from years
	if (_i == 0) then
	{
		while "_then != _now" do
		{
			_days = 365;
			if (_then%4==0  &&  (_then%100!=0 || _then%400==0)) then {_days=366};
			if (_days==366 && _addedExtraDay) then {_addedExtraDay=false; _days=365};
			
			_TotalDays = _TotalDays + _days;
			[_i, 1, _DateThen] call FLIB_MODIFYDATE;
			_then = _DateThen select _i
		};
	};
	

	_i = _i - 1;
	
	if (_i == 3) then {_i=_i-1};	//skip day name
};


_array1 = nil;
_array2 = nil;

if (_subtract) then {-_TotalDays} else {_TotalDays}
