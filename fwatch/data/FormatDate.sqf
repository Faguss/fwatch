/* 
Function supplied with Fwatch v1.13
Returns a date string formatted according to the given syntax

Usage:
	["FLIB_FORMATDATE"] call preProcessFile "..\fwatch\data\InitFLib.sqf"
	_string = [<format>, <dateArray>] call FLIB_FORMATDATE

Input: 
	<format> - string with formatting options, max 123 characters
			   alternatively array with single characters (for longer formats than 123)
	<date array> - name of the array storing date and time (can be obtained with :info date or :il)		
		
Formatting options:
y	- two digit representation of a year (14)
Y	- four digit representation of a year (2014)
m	- numeric representation of a month (6)
M	- short textual representation of a month (Jun)
MM	- full textual representation of a month (June)
d	- day of the month (3)
D	- short textual representation of the day of the week (Wed)
DD	- full textual representation of the day of the week (Wednesday)
h	- 24-hour format of an hour (15)
H	- 12-hour format of an hour (3)
i	- minutes
s 	- seconds
l	- milliseconds

Switches:
~	- disable/enable parsing
@ 	- enable/disable date localization
0 	- add leading zero(s) to the next number
e	- set English language (cancel localization) for the next month/day name
c	- cancel capitalization (except for English and German) for the next month/day name
am	- changes to "pm" for afternoon hours; type it in capital for uppercase
th	- add English ordinal suffix based on last used number; type it in capital for uppercase
TZ	- enable/disable timezone mode

Timezone mode:
+	- timezone difference (changes to minus if east from Greenwich)
h	- timezone difference in hours
i	- timezone difference in minutes (multiples of 60 are subtracted)
I	- timezone difference in minutes (full number)

Examples:
	"0d.0m.Y 0h:0i:0s.l TZ+0h:0i"	- 03.06.2014 15:20:55.2 +01:00
	"0m/0d/0y"						- 06/03/14
	"dth eMM (eD) Y"				- 3rd June (Wed) 2014
	"~Täänän on~ cDD"				- Täänän on keskiviikko
	"H:0i AM"						- 3:20 PM
	"~UTC~TZ+h"						- UTC+1
	
Changelog:
1.16
- added switch "@"

1.15
- uses Fwatch 1.15

1.14
- added russian short month and day names
- can pass array as a <format> argument
- russian long month names changed to normal form
*/




private ["_array", "_date", "_string", "_i", "_leadingZero", "_onlyEnglish", "_lastNumber", "_comment", "_timeZone", "_cancelCap", "_localized", "_now", "_dateLocal", "_letter", "_letterNext", "_find", "_languages", "_months", "_langID", "_language", "_days", "_number", "_uppercase", "_oneDigit", "_set"]; 

// Arguments
_array = _this select 0;
_date  = _this select 1;

if (_array in [_array]) then
{
	_array = call loadFile Format ["\:STRING SPLIT  text:%1", _array];
};

// Variables
_i 			 = 0;
_string 	 = "";
_leadingZero = false;
_onlyEnglish = false;
_lastNumber  = 0;
_comment 	 = false;
_timeZone 	 = false;
_cancelCap 	 = false;
_localized	 = false;
_dateLocal	 = _date;


// For each character in date pattern
while "_i < count _array" do
{
	_letter 	= _array select _i;
	_letterNext = if (_i < count _array - 1) then {_array select (_i+1)} else {""};
	
	
	// Comment
	if (_letter == "~") then {_comment=!_comment; _letter=""};
	
	
	// Localize date
	if (_letter == "@") then 
	{
		_letter = "";
		
		if (Format ["%1",count _now] == "scalar") then
		{
			_now = call loadFile ":info date";
			
			if ((_now select 8) != (_date select 8)) then
			{
				_dateLocal = +_date;
				
				if (Format["%1",FLIB_MODIFYDATE in []]=="bool") then
				{
					FLIB_MODIFYDATE = preProcessFile "..\fwatch\data\ModifyDate.sqf"
				};
				
				["minutes", (_now select 8) - (_date select 8), _dateLocal] call FLIB_MODIFYDATE;
				_dateLocal set [8, _now select 8];
				
				_date      = _dateLocal;
				_localized = true
			}
		}
		else
		{
			if (_localized) then {_date=_this select 1} else {_date=_dateLocal};
			_localized = !_localized
		}
	};
	
	
	if (!_comment) then
	{	
		// Switches
		if (_letter == "0") then {_leadingZero=true; _letter=""};
		if (_letter in ["e"]) then {_onlyEnglish=true; _letter=""};
		if (_letter in ["c"]) then {_cancelCap=true; _letter=""};
		if (_letter in ["T"] && _letterNext in ["Z"]) then {_timeZone=!_timeZone; _i=_i+1; _letter=""};
	
	
		// Year
		if (_letter in ["Y"]) then 
		{
			_letter = Format ["%1", _date select 0];
			if (_leadingZero) then 
			{
				_leadingZero = false;
				if (abs (_date select 0) < 1000) then {_letter=Format ["0%1",_letter]};
				if (abs (_date select 0) < 100) then {_letter=Format ["0%1",_letter]};
				if (abs (_date select 0) < 10) then {_letter=Format ["0%1",_letter]};
			};		
		};
		if (_letter in ["y"]) then 
		{
			_letter = Format ["%1", (_date select 0) mod 100];
			if (_leadingZero) then 
			{
				_leadingZero = false;
				if (abs ((_date select 0) mod 100) < 10) then {_letter=Format ["0%1",_letter]};
			};
		};
		if (_letter == "y") then {_lastNumber=(_date select 0)};
	
	
		// Month number
		if (_letter in ["m"]) then 
		{
			_letter = Format ["%1",_date select 1];
			_lastNumber = (_date select 1);
			if (_leadingZero) then 
			{
				_leadingZero = false;
				if ((_date select 1) < 10) then {_letter=Format ["0%1",_letter]};
			};
		};
	
	
		// Month literal
		if (_letter in ["M"]) then
		{	
			// Long version
			if (_letterNext in ["M"]) then
			{
				_find = "private [""_i"", ""_break""]; _i=0; _break=false; while ""!_break"" do {if (_i >= count (_this select 1)) then {_i=-1; _break=true} else {if((_this select 0)==(_this select 1) select _i)then{_break=true}else{_i=_i+1}}};_i";
				_languages = ["English","French","Italian","Spanish","German","Czech","Polish","Finnish","Russian"];
				_months = [
					["", "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"],
					["", "Janvier", "Février", "Mars", "Avril", "Mai", "Juin", "Juillet", "Août", "Septembre", "Octobre", "Novembre", "Décembre"],
					["", "Gennaio", "Febbraio", "Marzo", "Aprile", "Maggio", "Giugno", "Luglio", "Agosto", "Settembre", "Ottobre", "Novembre", "Dicembre"],
					["", "Enero", "Febrero", "Marzo", "Abril", "Mayo", "Junio", "Julio", "Agosto", "Septiembre", "Octubre", "Noviembre", "Diciembre"],
					["", "Januar", "Februar", "März", "April", "Mai", "Juni", "Juli", "August",  "September", "Oktober", "November", "Dezember"],
					["", "Leden", "Únor", "Bøezen", "Duben", "Kvìten", "Èerven", "Èervenec", "Srpen", "Záøí", "Øíjen", "Listopad", "Prosinec"],
					["", "Styczeñ", "Luty", "Marzec", "Kwiecieñ", "Maj", "Czerwiec", "Lipiec", "Sierpieñ", "Wrzesieñ", "PaŸdziernik", "Listopad", "Grudzieñ"],
					["", "Tammikuu", "Helmikuu", "Maaliskuu", "Huhtikuu", "Toukokuu", "Kesäkuu", "Heinäkuu", "Elokuu", "Syyskuu", "Lokakuu", "Marraskuu", "Joulukuu"],
					["", "ßíâàðü",  "Ôåâðàëü", "Ìàðò", "Àïðåëü", "Ìàé", "Èþíü", "Èþëü", "Àâãóñò", "Ñåíòÿáðü", "Îêòÿáðü", "Íîÿáðü", "Äåêàáðü"]];
				
				_language = call preProcessFile "..\fwatch\data\CurrentLanguage.sqf";
				_langID = [_language, _languages] call _find;
				if (_langID == -1) then {_language="English"; _langID=0};
				if (_onlyEnglish) then {_onlyEnglish=false; _language="English"; _langID=0};
				_letter = (_months select _langID) select (_date select 1);
				if (_cancelCap && _language!="English" && _language!="German") then 
				{
					_cancelCap = false;
					_letter = call loadFile Format ["\:STRING CASE  lower:1  text:%1", _letter];
				};
				_i = _i + 1;
			}
			// Short version
			else
			{
				_months = ["", localize "STR_JANUARY", localize "STR_FEBRUARY", localize "STR_MARCH", localize "STR_APRIL", localize "STR_MAY", localize "STR_JUNE", localize "STR_JULY", localize "STR_AUGUST", localize "STR_SEPTEMBER", localize "STR_OCTOBER", localize "STR_NOVEMBER", localize "STR_DECEMBER"];
				_language = call preProcessFile "..\fwatch\data\CurrentLanguage.sqf";
				if (_language=="Russian") then {_months=["","ßíâ", "Ôåâ", "Ìàð", "Àïð", "Ìàé", "Èþí", "Èþë", "Àâã", "Ñåí", "Îêò", "Íîÿ", "Äåê"]};
				if (_onlyEnglish) then {_onlyEnglish=false; _months=["","Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"]};
				_letter = _months select (_date select 1);
			};
		};
	
	
		// Day number
		if (_letter in ["d"]) then
		{
			_letter = Format ["%1", _date select 2];
			_lastNumber = (_date select 2);
			if (_leadingZero) then 
			{
				_leadingZero = false;
				if ((_date select 2) < 10) then {_letter=Format ["0%1",_letter]};
			};
		};
	
	
		// Day literal
		if (_letter in ["D"]) then
		{
			// Long version
			if (_letterNext in ["D"]) then
			{
				_find = "private [""_i"", ""_break""]; _i=0; _break=false; while ""!_break"" do {if (_i >= count (_this select 1)) then {_i=-1; _break=true} else {if((_this select 0)==(_this select 1) select _i)then{_break=true}else{_i=_i+1}}};_i";
				_languages = ["English","French","Italian","Spanish","German","Czech","Polish","Finnish","Russian"];
				_days = [
					["Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"],
					["Dimanche", "Lundi", "Mardi", "Mercredi", "Jeudi", "Vendredi", "Samedi"],
					["Domenica", "Lunedi", "Martedi", "Mercoledi", "Giovedi", "Venerdi", "Sabato"],
					["Domingo", "Lunes", "Martes", "Miércoles", "Jueves", "Viernes", "Sábado"],
					["Sonntag", "Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag", "Samstag"],
					["Nedìle", "Pondìlí", "Úterý", "Støeda", "Ètvrtek", "Pátek", "Sobota"],
					["Niedziela", "Poniedzia³ek", "Wtorek", "Œroda", "Czwartek", "Pi¹tek", "Sobota"],
					["Sunnuntai", "Maanantai", "Tiistai", "Keskiviikko", "Torstai", "Perjantai", "Lauantai"],
					["Âîñêðåñåíüå", "Ïîíåäåëüíèê", "Âòîðíèê", "Ñðåäà", "×åòâåðã", "Ïÿòíèöà", "Ñóááîòà"]];
			
				_language = call preProcessFile "..\fwatch\data\CurrentLanguage.sqf";
				_langID = [_language, _languages] call _find;
				if (_langID == -1) then {_language="English"; _langID=0};
				if (_onlyEnglish) then {_onlyEnglish=false; _language="English"; _langID=0};
				_letter = (_days select _langID) select (_date select 3);
				if (_cancelCap && _language!="English" && _language!="German") then 
				{
					_cancelCap = false;
					_letter = call loadFile Format ["\:STRING CASE  lower:1  text:%1", _letter];
				};
				_i = _i + 1;
			}
			// Short version
			else
			{
				_days = [localize "STR_SUNDAY", localize "STR_MONDAY", localize "STR_TUESDAY", localize "STR_WEDNESDAY", localize "STR_THURSDAY", localize "STR_FRIDAY", localize "STR_SATURDAY"];
				_language = call preProcessFile "..\fwatch\data\CurrentLanguage.sqf";
				if (_language=="Russian") then {_days=["Âñ", "Ïí", "Âò", "Ñð", "×ò", "Ïò", "Ñá"]};
				if (_onlyEnglish) then {_onlyEnglish=false; _days=["Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"]};
				_letter = _days select (_date select 3);
			};
		};
	
	
		// Hours
		if (!_timeZone && _letter == "h") then
		{
			_number = _date select 4;			
			
			//12-hour format
			if (_letter in ["H"]  &&  _number>12) then {_number=_number-12};
			
			_letter = Format ["%1", _date select 4];
		
			if (_leadingZero) then 
			{
				_leadingZero = false;
				if ((call _letter) < 10) then {_letter=Format ["0%1",_letter]};
			};
			_lastNumber = call _letter;
		};


		// Minutes
		if (!_timeZone && _letter == "i") then
		{
			_letter = Format ["%1", _date select 5];
			_lastNumber = _date select 5;
			if (_leadingZero) then
			{
				_leadingZero = false;
				if (_date select 5 < 10) then {_letter=Format ["0%1",_letter]};
			};
		};


		// Seconds
		if (_letter == "s") then
		{
			_letter = Format ["%1", _date select 6];
			_lastNumber = _date select 6;
			if (_leadingZero) then
			{
				_leadingZero = false;
				if (_date select 6 < 10) then {_letter=Format ["0%1",_letter]};
			};
		};


		// Milliseconds
		if (_letter == "l") then
		{
			_letter = Format ["%1", _date select 7];
			_lastNumber = _date select 7;
			if (_leadingZero) then
			{
				_leadingZero = false;
				if (_date select 7 < 100) then {_letter=Format ["0%1",_letter]};
				if (_date select 7 < 10) then {_letter=Format ["0%1",_letter]};
			};
		};
	
	
		// Timezone Hours
		if (_timeZone && _letter == "h") then
		{
			_number = abs (_date select 8) / 60;
			_number = _number - (_number mod 1);
			_letter = Format ["%1", _number];
			
			if (_leadingZero) then 
			{
				_leadingZero = false;
				if (_number < 10) then {_letter=Format ["0%1",_letter]};
			};
			_lastNumber = _number;
		};


		// Timezone Minutes
		if (_timeZone && _letter == "i") then
		{
			_number = 0;
			if (_letter in ["i"]) then		// 0-59
			{
				_number = abs (_date select 8) / 60;
				_number = _number - (_number mod 1);
				_number = abs (_date select 8) - _number*60;
				_letter = Format ["%1", _number];
				if (_leadingZero) then
				{
					_leadingZero = false;
					if (_number < 10) then {_letter=Format ["0%1",_letter]};
				};
			}
			else	// Full number
			{
				_letter = Format ["%1", abs (_date select 8)];
			};
			_lastNumber = _number;
		};


		// Timezone sign
		if (_timeZone && _letter=="+") then
		{
			if ((_date select 8) < 0) then {_letter="-"};
		};
		
		
		// Ante meridiem / Post meridiem
		if (_letter=="a"  &&  _letterNext=="m") then
		{
			_uppercase = _letter in ["A"];
			
			if ((_date select 4) > 12) then 
			{
				_letter = if (_uppercase) then {"PM"} else {"pm"};
			}
			else
			{
				_letter = if (_uppercase) then {"AM"} else {"am"};
			};
			
			_i = _i + 1;
		};


		// Add english suffix to the last number
		if (_letter=="t"  &&  _letterNext=="h") then 
		{
			_oneDigit = abs _lastNumber mod 10;
			_set = false;
			
			if (_oneDigit==1  &&  _lastNumber!=11) then 
			{
				_letter = if (_letter in ["t"]) then {"s"} else {"S"};
				if (_letterNext in ["h"]) then {_letter=Format["%1t",_letter]} else {_letter=Format["%1T",_letter]};
				_set = true;
			};
			
			if (_oneDigit==2  &&  _lastNumber!=12) then 
			{
				_letter = if (_letter in ["t"]) then {"n"} else {"N"};
				if (_letterNext in ["h"]) then {_letter=Format["%1d",_letter]} else {_letter=Format["%1D",_letter]};
				_set = true;
			};
			
			if (_oneDigit==3  &&  _lastNumber!=13) then 
			{
				_letter = if (_letter in ["t"]) then {"r"} else {"R"};
				if (_letterNext in ["h"]) then {_letter=Format["%1d",_letter]} else {_letter=Format["%1D",_letter]};
				_set = true;
			};
			
			if (!_set) then 
			{
				_letter = if (_letter in ["t"]) then {"t"} else {"T"};
				if (_letterNext in ["h"]) then {_letter=Format["%1h",_letter]} else {_letter=Format["%1H",_letter]};
			};
			
			_i = _i + 1;
		};
	};
	
	_string = Format ["%1%2",_string,_letter];
	_i = _i + 1
};

_string	
	
