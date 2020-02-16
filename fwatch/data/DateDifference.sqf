/* 
Function supplied with Fwatch v1.13
Calculates difference between two dates and returns array with differences for each time unit

Usage:
	["FLIB_DATEDIFF"] call preProcessFile "..\fwatch\data\InitFLib.sqf"
	_difference = [<datearray>,<datearray>] call FLIB_DATEDIFF

Alternatively use
	[<datearray>,<datearray>,"subtract"]
for a negative result when first date is older

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
	
Output: difference array
	0 - years
	1 - months
	2 - days
	3 - hours
	4 - minutes
	5 - seconds
	6 - milliseconds
	
Changelog:
1.16
- variable _j wasn't local and would interfere with scripts - fixed

1.15
- added "subtract" option
- initializes FLIB_MODIFYDATE
*/

private ["_array1", "_array2", "_i", "_j", "_same", "_difference", "_DateThen", "_DateNow", "_then", "_now", "_max", "_year", "_days", "_subtract"];


if (Format["%1",FLIB_MODIFYDATE in []]=="bool") then
{
	FLIB_MODIFYDATE = preProcessFile "..\fwatch\data\ModifyDate.sqf"
};

// Optional argument
_subtract = false;
if (count _this > 2) then
{
	if (_this select 2 == "subtract") then {_subtract=true}
};


// Check time zone difference
_array1 =+ (_this select 0);
_array2 =+ (_this select 1);
_then = _array1 select 8;
_now = _array2 select 8;

// If there is difference then compensate
if (_then != _now) then
{
	// If 1st date is western then make 2nd date approach it
	if (_then > _now) then {[5, _then-_now, _array2] call FLIB_MODIFYDATE};
	
	// If 2nd date is western then make 1st date approach it
	if (_now > _then) then {[5, _now-_then, _array1] call FLIB_MODIFYDATE};
};



// Check if there is difference between dates
_i = 0;
_same = true;
_difference = [0,0,0,0,0,0,0];
_DateThen = _array1;
_DateNow = _array2;

while "_i<=7" do
{
	_then = _array1 select _i;
	_now = _array2 select _i;
	
	// If first date is older
	if (_then < _now) then {_same=false};
	
	// If first date is more recent
	if (_then > _now) then
	{
		_DateThen = _array2;
		_DateNow = _array1;
		_subtract = false; 
		_same = false
	};
	
	// Continue loop if both elements are the same
	if (_then == _now) then 
	{
		_i=_i+1; 
		if (_i==3) then {_i=4}
	} 
	else {_i=8};
};




// Find differences between two arrays
_i = 7;
while "!_same && _i>=0" do
{
	_then = _DateThen select _i;
	_now = _DateNow select _i;
	
	
	// Determine maximal value for each item
	_max = 0;
	if (_i==7) then {_max=1000};
	if (_i==6) then {_max=60};
	if (_i==5) then {_max=60};
	if (_i==4) then {_max=24};
	if (_i==2) then 
	{
		_now = _now - 1;
		_then = _then - 1;		//convert days to 0-XX

		_year = _DateThen select 0;
		_days = [31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31];
		if (_year%4==0  &&  (_year%100!=0 || _year%400==0)) then {_days set [1, 29]};		//leap year
		_max = _days select ((_DateThen select 1)-1)
	};
	if (_i==1) then 
	{
		_now = _now - 1;
		_then = _then - 1; 	//convert month to 0-11
		_max = 12
	};
	

	// _j is position in the _difference array
	_j = _i-1;
	if (_i<=2) then {_j=_i};

	// If value in the older date is smaller then just subtract
	if (_then < _now) then {_difference set [_j, _now-_then]};
	
	// If value in the older date is larger then increment date
	if (_then > _now && _i>0) then
	{
		_difference set [_j, _max-_then+_now];
		[_i-1, 1, _DateThen] call FLIB_MODIFYDATE
	};
	
	_i = _i - 1;
	if (_i==3) then {_i=_i-1};	//skip day name
};

_array1 = nil;
_array2 = nil;

if (_subtract) then {_i=0; "_difference set [_i, -_x]; _i=_i+1" forEach _difference};

_difference
