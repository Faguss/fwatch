/* 
Function supplied with Fwatch v1.13
Initalizes other functions in the fwatch\data directory

Usage:
	<string or array with strings> call preProcessFile "..\fwatch\data\InitFLib.sqf"
	
Optional argument is function name/global variable name or an array with them.
If no arguments were passed then all functions are initialized

Examples:
	call preProcessFile "..\fwatch\data\InitFLib.sqf"
	"convertkeystring" call preProcessFile "..\fwatch\data\InitFLib.sqf"
	["flib_exec","inkeys.sqf"] call preProcessFile "..\fwatch\data\InitFLib.sqf"
	
Changelog:
1.15
- removed FLIB_EXEC
1.14
- added FLIB_GETAR, FLIB_CONVERTKEY
- can pass argument to initialize only selected functions instead of all of them
- fixed crashing
*/



// Format input
if (Format ["%1",_this in [_this]]=="bool") then {_this=[]};
if (_this in [_this]) then {_this=[_this]};


// Macro for function initialization
#define QUOT "
#define CONDITION(GLOBAL,FILE) \
	if ((_this select _i)==##QUOT##GLOBAL##QUOT## || (_this select _i)==##QUOT##FILE##QUOT## || (_this select _i)==##QUOT##FILE##.sqf##QUOT## || count _this==0) then \
	{ \
		if (Format["%1",GLOBAL in []]=="bool") then \
		{ \
			GLOBAL = preProcessFile ##QUOT##..\fwatch\data\##FILE##.sqf##QUOT##; \
		}; \
	};
	

// Loop through conditions
private ["_i"];
_i = -1;
while "_i=_i+1;  _i<count _this  ||  count _this==0 && _i==0" do
{
	CONDITION(FLIB_CONVERTKEY,ConvertKeyString);
	CONDITION(FLIB_CURRENTLANG,CurrentLanguage);
	CONDITION(FLIB_DATEDIFF,DateDifference);
	CONDITION(FLIB_DATEDIFFDAY,DateDifferenceInDays);
	CONDITION(FLIB_FORMATDATE,FormatDate);
	CONDITION(FLIB_GETAR,getAspectRatio);
	CONDITION(FLIB_INKEYS,inKeys);
	CONDITION(FLIB_MEASURETIME,MeasureTime);
	CONDITION(FLIB_MODIFYDATE,ModifyDate);
};

true
