/* 
Function supplied with Fwatch v1.13
Increments / decrements given time unit in a date array

Usage:
	["FLIB_MODIFYDATE"] call preProcessFile "..\fwatch\data\InitFLib.sqf"
	[<unit>, <modifier>, <dateArrayName>] call FLIB_MODIFYDATE

	<unit> can be a number or a string (plural allowed):
	0 - "year"
	1 - "month"
	2 - "day"
	4 - "hour"
	5 - "minute"
	6 - "second"
	7 - "millisecond"

Example:
	_date = call loadFile ":info date"
	["day", +1, _date] call FLIB_MODIFYDATE
	
Changelog:
1.16
- fixed a bug where it would show max unit number (like hour 11:60) after decreasing
- fixed a bug where days were changed when added hours
- no error message when passed incorrect unit string

1.14
- "month" and "year" adds/subtracts number of days in the current month/year instead of a fixed number
*/

private ["_id", "_idS", "_add", "_array", "_i", "_item", "_max", "_year", "_days", "_howMany", "_backupYear", "_backupMonth", "_backupDay", "_addMonths", "_addYears", "_previousItem", "_names"];

_id    = _this select 0;
_add   = _this select 1;
_array = _this select 2;


// If _id is a string
_idS = Format ["%1",_id];
if (_idS in [_id]) then
{
	_i     = -1;
	_id    = -1;
	_names = ["year","month","day","day","hour","minute","second","millisecond"];
	
	while "_i=_i+1; _i<count _names" do 
	{
		if (_idS==(_names select _i) || _idS==(_names select _i)+"s") then {_id=_i}
	}
};

// Verify argument
if (_id == 3)         then {_id=2};
if (_id<0  ||  _id>7) then {_add=0};


// Convert year and month to days
_addYears  = 0;
_addMonths = 0;
if (_id==0) then {_id=2; _addYears=_add};
if (_id==1) then {_id=2; _addMonths=_add};


if (_add != 0) then
{
	// Make a copy for later
	_backupYear  = _array select 0;
	_backupMonth = _array select 1;
	_backupDay   = _array select 2;
	
	// Index months/days from zero instead of one
	_array set [1, (_array select 1)-1];
	_array set [2, (_array select 2)-1];
	
	_i = _id;
	
	while "_i >= 0" do
	{	
		_item = (_array select _i);
	
		// Determine maximal value for each item
		_max = 0;
		
		if (_i == 7) then {_max=1000};
		if (_i == 6) then {_max=60};
		if (_i == 5) then {_max=60};
		if (_i == 4) then {_max=24};
		if (_i == 2) then 
		{
			_year = _array select 0;
			_days = [31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31];
			if (_year%4==0  &&  (_year%100!=0 || _year%400==0)) then {_days set [1, 29]};		//leap year
			_max = _days select (_array select 1);
			
			// If user wants to add years/months then we're really adding number of days in the current year/month
			if (_addYears != 0)  then 
			{
				_add = 0; 
				"_add=_add+_x" forEach _days;
				
				if (_addYears < 0) then 
				{
					_add      = -_add; 
					_addYears = _addYears + 1
				} 
				else 
				{
					_addYears = _addYears - 1
				}
			};
			
			if (_addMonths != 0) then {_add=_max; if (_addMonths<0) then {_add=-_add; _addMonths=_addMonths+1} else {_addMonths=_addMonths-1}};
		};
		if (_i == 1) then {_max=12};
		

		
		
		// Increase/decrease wanted item
		if (_i == _id) then
		{
			_item = _item + _add;
			_array set [_i, _item];
		};


		// If over the range then reset and increase previous in the array (higher order)
		if (_i > 0  &&  _item >= _max) then
		{
			if (_i != 2) then		// not days
			{
				_previousItem = if (_i==4) then {2} else {_i-1};	// avoid day name
				
				_howMany = _item / _max;
				_howMany = _howMany - (_howMany mod 1);
				
				_array set [_i, _item - _howMany*_max];		
				_array set [_previousItem, (_array select _previousItem) + _howMany];
			}
			else	// days
			{
				// maximal number of days is different for each month so we need to keep track
				while "_item >= _max" do
				{
					_item = _item - _max;
					
					// set to next month
					_array set [1, (_array select 1)+1];

					// if going to next year
					if ((_array select 1) > 11) then
					{
						_array set [1, 0];
						_array set [0, (_array select 0)+1];
					};

					// Find number of days for this month
					_year = _array select 0;
					_days = [31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31];
					if (_year%4==0  &&  (_year%100!=0 || _year%400==0)) then {_days set [1, 29]};		//leap year
					_max = _days select (_array select 1);
				};
				
				_array set [2, _item];			
			};
		};
	
	
		// If below the range then reset and decrease previous in the array (higher order)
		if (_i > 0  &&  _item < 0) then
		{	
			if (_i != 2) then		// not days
			{
				_previousItem = if (_i==4) then {2} else {_i-1};
				
				_array set [_i, _max + _item];
				_array set [_previousItem, (_array select _previousItem) - 1];

				if (abs _item != _max) then 
				{
					_howMany = abs (_item / _max);
					_howMany = _howMany - (_howMany mod 1);
				
					_array set [_i, _item + (_howMany*_max) + _max];
					_array set [_previousItem, (_array select _previousItem) - _howMany];
					
					if ((_array select _i) == _max) then {
						_array set [_i, 0];
						_array set [_previousItem, (_array select _previousItem) +1];
					}
				};
			}
			else		// days
			{		
				// maximal number of days is different for each month so we need to keep track
				while "_item < 0" do
				{
					// set to prev month
					_array set [1, (_array select 1)-1];

					// if going to prev year
					if ((_array select 1) < 0) then
					{
						_array set [1, 11];
						_array set [0, (_array select 0)-1];
					};

					// Find number of days for this month
					_year = _array select 0;
					_days = [31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31];
					if (_year%4==0  &&  (_year%100!=0 || _year%400==0)) then {_days set [1, 29]};		//leap year
					_max = _days select (_array select 1);

					// if our number is smaller than the number of days in the current month
					// then wrap it up
					if (abs _item <= _max) then
					{
						_item = _max + _item;
						_array set [2, _item]
					}
					else
					// if our number is still larger than the number of days in the current month
					// then reduce it and keep going
					{
						_item = _item + _max;
					};
				};
			};
		};

		_i = _i - 1;
		if (_i==3) then {_i=_i-1};	//skip day name
		
		// if user wants to add months/years then just stay with days
		if (_addYears != 0  ||  _addMonths != 0) then {_i=2};
	};

	
	// Convert months/days back to 1-XX
	_array set [1, (_array select 1)+1];
	_array set [2, (_array select 2)+1];

	
	// Find a new day of the week
	if (_backupYear!=_array select 0  ||  _backupMonth!=_array select 1  ||  _backupDay!=_array select 2) then
	{
		_array set [3, (call loadFile Format [":info date %1", _array]) select 3]
	}
};


_array
