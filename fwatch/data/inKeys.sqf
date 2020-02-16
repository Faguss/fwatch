/* 
Function supplied with Fwatch v1.13
Checks if given key combination is in the _keys array and returns bool

Usage:
	_keys = []
	_pressed = []
	["FLIB_INKEYS"] call preProcessFile "..\fwatch\data\InitFLib.sqf"
	_isHotkey = <input> call FLIB_INKEYS

Where <input> can be string:
	"A" 			- if "A" is in the _keys
or array with strings:
	["A"]			- if "A" is in the _keys
	["A", "B"]		- if "A" or "B" are in the _keys
	[["A", "B"]]	- if "A" and "B" are in the _keys
	[["A","B"],"C"]	- if "A" and "B" or "C" are in the _keys
	
String is added to the _pressed array to prevent continuous activation.

For a combination only last item in the array is added to the
_pressed. For example: [["SHIFT","A"]] - "A" will be added to the
array and "SHIFT" will not.

Changelog:
1.15
- changed input format, now sub-array is a combination instead of alternative and vice versa
- empty sub-array can now be safely passed
*/




private ["_item", "_items", "_return", "_last", "_i"];
_return = false;

// For a string
if (_this in [_this]) then
{
	// click
	if (_this in _keys && !(_this in _pressed)) then
	{
		_pressed = _pressed + [_this];
		_return = true
	};

	// release
	if (!(_this in _keys) && _this in _pressed) then
	{
		_pressed = _pressed - [_this]
	}
}


else

// For an array
{
	_items =+ _this;
	
	// Sub-Arrays - alternative keys
	_i = -1;
	while "_i=_i+1; _i<count _items && !_return" do
	{
		_item = _items select _i;
		
		// If a string
		if (_item in [_item]) then
		{
			// click
			if (_item in _keys && !(_item in _pressed)) then
			{
				_pressed = _pressed + [_item];
				_return = true
			};
			
			// release
			if (!(_item in _keys) && _item in _pressed) then
			{
				_pressed = _pressed - [_item]
			}
		}
		// If an array
		else
		{
			if (count _item > 0) then
			{
				_last = _item select (count _item -1);

				// if all pressed
				if ("_x in _keys" count _item == count _item  &&  !(_last in _pressed)) then
				{
					// click
					_pressed = _pressed + [_last];
					_return = true
				};

				// release
				if (!(_last in _keys) && _last in _pressed) then
				{
					_pressed = _pressed - [_last]
				}
			}
		}
	}
};

_return
