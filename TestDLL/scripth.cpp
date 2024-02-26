/*
You may use this source code for personal entertainment purposes only. Any commercial-, education- or military use is strictly forbidden without permission from the author. Any programs compiled from this source and the source code itself should be made available free of charge. Any modified versions must have the source code available for free upon request.
*/

//
// by Kegetys <kegetys@dnainternet.net>
//

// Script handling functions

#define WIN32_LEAN_AND_MEAN

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <shellapi.h>
#include "testdll.h"
#include <ctype.h>
#include <Mmsystem.h>	// Joystick input
#include <math.h>		// Trigonometry functions
#include <tlhelp32.h>	// Traversing process modules
#include "errno.h"		// Error constants
#include "shlobj.h"		// Copying files to the clipboard
#include "..\common_functions.cpp"	// StringDynamic, String types
#include <time.h>		// Random number
#include <io.h>			// Converting HANDLE to FILE
#include <fcntl.h>		// Converting HANDLE to FILE
#include <climits>
#include <float.h>		// _is_nan

// task scheduler
#include <initguid.h>
#include <ole2.h>
#include <mstask.h>
#include <msterr.h>
#include <objidl.h>
#include <wchar.h>

extern GLOBAL_VARIABLES_TESTDLL global;

#define SCRIPT_VERSION	1.16		// Script engine version
#define MAX_PARAMS		64			// Maximum parameters for commands
#define WGET_MINWAIT	1000
#define PIPEBUFSIZE		4096

// command hashes
#define C_CLASS_LIST           3894729449u
#define C_CLASS_TOKEN          1306216122u
#define C_CLASS_MODIFY         3457835787u
#define C_CLASS_MODTOK         2686266215u
#define C_CLASS_READ           1269173709u
#define C_CLASS_READ2          359139437u
#define C_CLASS_READSQM        1404966420u
#define C_CLASS_WRITE          2805421668u
#define C_CLIP_COPY            747579208u
#define C_CLIP_GET             2069825331u
#define C_CLIP_GETLINE         1221872377u
#define C_CLIP_SIZE            809306128u
#define C_CLIP_TOFILE          2539841174u
#define C_CLIP_FROMFILE        3730249919u
#define C_CLIP_COPYFILE        8450590u
#define C_CLIP_CUTFILE         1451318777u
#define C_CLIP_PASTEFILE       962169530u
#define C_SPIG_EXE             3206713566u
#define C_EXE_MAKEPBO          3754554022u
#define C_EXE_ADDONTEST        501781933u
#define C_EXE_ADDONINSTALL     367297688u
#define C_EXE_UNPBO            1304888561u
#define C_EXE_WGET             3861629986u
#define C_EXE_PREPROCESS       2891211499u
#define C_RESTART_CLIENT       1040407795u
#define C_RESTART_SERVER       1698669191u
#define C_RESTART_SCHEDULE     962537865u
#define C_EXE_WEBSITE          226367578u
#define C_FILE_EXISTS          1534773825u
#define C_FILE_READ            4131020217u
#define C_FILE_WRITE           11066784u
#define C_FILE_QWRITE          2784837127u
#define C_FILE_AWRITE          2781408183u
#define C_FILE_VARS            3245436723u
#define C_FILE_READVARS        2763768851u
#define C_FILE_REMOVE          2395420265u
#define C_FILE_DELETE          395478174u
#define C_FILE_WGET            3189353990u
#define C_FILE_DXDLL           615999755u
#define C_FILE_READ2           685851089u
#define C_FILE_MODLIST         2709572447u
#define C_FILE_PBO             4240196012u
#define C_FILE_CUSTOMCOUNTSIZE 1721154120u
#define C_IGSE_WRITE           1567449702u
#define C_IGSE_LIST            3690437211u
#define C_IGSE_LOAD            3760381619u
#define C_IGSE_NEW             3979929763u
#define C_IGSE_RENAME          731063145u
#define C_IGSE_COPY            1607149306u
#define C_IGSE_FIND            519206608u
#define C_IGSE_DB              1686459221u
#define C_INFO_VERSION         682326273u
#define C_INFO_DEBUGOUT        1250266690u
#define C_INFO_DATE            2828831323u
#define C_INFO_RESOLUTION      1512440909u
#define C_INFO_STARTTIME       3949992052u
#define C_INFO_TICK            3594399694u
#define C_INFO_ERRORLOG        2670383007u
#define C_INPUT_GETKEYS        529347995u
#define C_INPUT_AGETKEYS       1623224776u
#define C_INPUT_GETKEY         1580245162u
#define C_INPUT_AGETKEY        1889404011u
#define C_INPUT_GETMOUSE       4042371078u
#define C_INPUT_SETMOUSE       1710720178u
#define C_INPUT_LOCK           2534329742u
#define C_INPUT_GETJOYSTICK    341548345u
#define C_INPUT_MULTI          324077510u
#define C_MEM_GETCURSOR        1060535396u
#define C_MEM_SETCURSOR        4120146568u
#define C_MEM_GETWORLD         3684053402u
#define C_MEM_GETMAP           2119371472u
#define C_MEM_SETMAP           3859369924u
#define C_MEM_GETGRAPHICS      3949774627u
#define C_MEM_GETJOYSTICK      3474622136u
#define C_MEM_GETSCROLL        561072295u
#define C_MEM_GETPLAYERANIM    2961889542u
#define C_MEM_SETPLAYERANIM    856509018u
#define C_MEM_GETCAM           4058691877u
#define C_MEM_GETCINEMABORDER  2974059547u
#define C_MEM_GETNV            1457539578u
#define C_MEM_GETRESPAWNTYPE   199500974u
#define C_MEM_SETRESPAWNTYPE   1059833330u
#define C_MEM_GETRESSIDE       1638557187u
#define C_MEM_GETDAYLIGHT      4098054280u
#define C_MEM_GETDATE          1428058178u
#define C_MEM_GETPLAYERVIEW    956477386u
#define C_MEM_SETPLAYERVIEW    4154429014u
#define C_MEM_ERROR            3304531164u
#define C_MEM_GETPLAYERAIM     526157708u
#define C_MEM_SETPLAYERAIM     1706376064u
#define C_MEM_MODLIST          1096286730u
#define C_MEM_GETDIFFICULTY    1176805931u
#define C_MEM_SETDIFFICULTY    1089543055u
#define C_MEM_ISDIALOG         3850265438u
#define C_MEM_GETRADIOBOX      2504840282u
#define C_MEM_SETRADIOBOX      790714550u
#define C_MEM_SETGRAPHICS      1044199951u
#define C_MEM_GETSPEEDKEY      3620598336u
#define C_MEM_SETSPEEDKEY      861867444u
#define C_MEM_GETPLAYERHATCH   1766714027u
#define C_MEM_SETPLAYERHATCH   4119745471u
#define C_MEM_GETPLAYERLADDER  3090776391u
#define C_MEM_SETPLAYERLADDER  2409418555u
#define C_MEM_MULTI            351704443u
#define C_MEM_MASTERSERVER     523283091u
#define C_MEM_MISSIONINFO      733488320u
#define C_MEM_BULLETS          1300677055u
#define C_MEM_GETWEATHER       598054334u
#define C_MEM_SETWEATHER       3155389850u
#define C_MEM_SETCAM           2822225721u
#define C_MEM_HUD              2366644167u
#define C_STRING_FIRST         3570665910u
#define C_STRING_LAST          1839646952u
#define C_STRING_LENGTH        3665905396u
#define C_STRING_RANGE         682937149u
#define C_STRING_TOLOWER       4292283326u
#define C_STRING_TOUPPER       1500743211u
#define C_STRING_TOARRAY       868016138u
#define C_STRING_INDEX         2627341444u
#define C_STRING_SIZE          658852761u
#define C_STRING_REPLACE       1348713056u
#define C_STRING_COMPARE       2284732455u
#define C_STRING_ISNUMBER      680534929u
#define C_STRING_ISVARIABLE    745200000u
#define C_STRING_ISEMPTY       3378770225u
#define C_STRING_CUT           702942952u
#define C_STRING_TOKENIZE      2000539349u
#define C_STRING_TRIM          4109809924u
#define C_STRING_FIND          3673872707u
#define C_STRING_SPLIT         1262498968u
#define C_STRING_DOMAIN        2829566612u
#define C_STRING_CASE          3135263444u
#define C_STRING_REPLACECHAR   3094755036u
#define C_STRING_JOIN          4131562040u
#define C_STRING_WORDPOS       1620765186u

unsigned int commands_named_arguments[] = { // sorted
	C_CLASS_READ2,
	C_IGSE_FIND,
	C_STRING_SIZE,
	C_STRING_ISNUMBER,
	C_STRING_CUT,
	C_IGSE_RENAME,
	C_STRING_ISVARIABLE,
	C_CLIP_COPY,
	C_RESTART_SCHEDULE,
	C_MEM_SETGRAPHICS,
	C_MEM_MODLIST,
	C_CLIP_GETLINE,
	C_STRING_SPLIT,
	C_CLASS_READ,
	C_MEM_BULLETS,
	C_CLASS_TOKEN,
	C_STRING_REPLACE,
	C_CLASS_READSQM,
	C_IGSE_WRITE,
	C_IGSE_COPY,
	C_STRING_WORDPOS,
	C_IGSE_DB,
	C_STRING_TOKENIZE,
	C_STRING_COMPARE,
	C_MEM_HUD,
	C_CLIP_TOFILE,
	C_CLASS_MODTOK,
	C_CLASS_WRITE,
	C_MEM_SETCAM,
	C_STRING_DOMAIN,
	C_STRING_REPLACECHAR,
	C_STRING_CASE,
	C_MEM_SETWEATHER,
	C_CLASS_MODIFY,
	C_STRING_FIND,
	C_IGSE_LIST,
	C_IGSE_LOAD,
	C_CLASS_LIST,
	C_IGSE_NEW,
	C_STRING_TRIM,
	C_STRING_JOIN,
	C_FILE_PBO
};

unsigned int commands_memory[] = { // sorted
	C_MEM_GETRESPAWNTYPE,
	C_EXE_WEBSITE,
	C_INPUT_MULTI,
	C_MEM_MULTI,
	C_MEM_MASTERSERVER,
	C_MEM_GETPLAYERAIM,
	C_MEM_GETSCROLL,
	C_MEM_GETWEATHER,
	C_MEM_MISSIONINFO,
	C_MEM_SETRADIOBOX,
	C_MEM_SETPLAYERANIM,
	C_MEM_SETSPEEDKEY,
	C_MEM_GETPLAYERVIEW,
	C_RESTART_SCHEDULE,
	C_MEM_SETGRAPHICS,
	C_MEM_SETRESPAWNTYPE,
	C_MEM_GETCURSOR,
	C_MEM_SETDIFFICULTY,
	C_MEM_MODLIST,
	C_MEM_GETDIFFICULTY,
	C_MEM_BULLETS,
	C_MEM_GETDATE,
	C_MEM_GETNV,
	C_MEM_GETRESSIDE,
	C_MEM_SETPLAYERAIM,
	C_MEM_GETPLAYERHATCH,
	C_MEM_GETMAP,
	C_MEM_HUD,
	C_MEM_SETPLAYERLADDER,
	C_MEM_GETRADIOBOX,
	C_FILE_MODLIST,
	C_MEM_SETCAM,
	C_MEM_GETPLAYERANIM,
	C_MEM_GETCINEMABORDER,
	C_MEM_GETPLAYERLADDER,
	C_MEM_SETWEATHER,
	C_MEM_ERROR,
	C_MEM_GETJOYSTICK,
	C_MEM_GETSPEEDKEY,
	C_MEM_GETWORLD,
	C_MEM_ISDIALOG,
	C_MEM_SETMAP,
	C_MEM_GETGRAPHICS,
	C_INFO_STARTTIME,
	C_MEM_GETCAM,
	C_MEM_GETDAYLIGHT,
	C_MEM_SETPLAYERHATCH,
	C_MEM_SETCURSOR,
	C_MEM_SETPLAYERVIEW
};

// named arguments hashes
#define NAMED_ARG_TEXT1               9232797u
#define NAMED_ARG_SETPOS              14519347u
#define NAMED_ARG_SKYTHROUGH          64651335u
#define NAMED_ARG_APPEND              110723809u
#define NAMED_ARG_RADIOMENU_H         118299068u
#define NAMED_ARG_INDEX               151693739u
#define NAMED_ARG_UPPER               176974407u
#define NAMED_ARG_LIST                217798785u
#define NAMED_ARG_TRIMDOLLAR          231504115u
#define NAMED_ARG_ACTION_H            277796120u
#define NAMED_ARG_GUSTUNTIL           293809950u
#define NAMED_ARG_DIRECTORY           294729096u
#define NAMED_ARG_SHELL               300022785u
#define NAMED_ARG_CLOUDSPOS           303233623u
#define NAMED_ARG_LEFT                306900080u
#define NAMED_ARG_WINDSPEED           328211058u
#define NAMED_ARG_OFFSET              348705738u
#define NAMED_ARG_LINE                400234023u
#define NAMED_ARG_VIEWDISTANCE        405576979u
#define NAMED_ARG_MAXTIDE             437406701u
#define NAMED_ARG_TRACKTIME           459210109u
#define NAMED_ARG_SOURCE              466561496u
#define NAMED_ARG_NUMBER              467038368u
#define NAMED_ARG_ACTION_SIZE         467552881u
#define NAMED_ARG_ACTION_W            529460405u
#define NAMED_ARG_ACTION_X            546238024u
#define NAMED_ARG_REVERSE             558918661u
#define NAMED_ARG_ACTION_Y            563015643u
#define NAMED_ARG_SIZE                597743964u
#define NAMED_ARG_CHAT_ENABLE         621100091u
#define NAMED_ARG_FOVTOP              647594911u
#define NAMED_ARG_MAXLEVEL            650761905u
#define NAMED_ARG_CLOUDSSPEED         675179266u
#define NAMED_ARG_RECURRENCE          720132575u
#define NAMED_ARG_RENAMEPROPERTY      769516826u
#define NAMED_ARG_WORDDELIMITER       834005314u
#define NAMED_ARG_URL                 848251934u
#define NAMED_ARG_LIMIT               853203252u
#define NAMED_ARG_BRIGHTNESS          886071850u
#define NAMED_ARG_CHAT_FONT           902884201u
#define NAMED_ARG_ENDOFFSET           906008637u
#define NAMED_ARG_NEXTWEATHERCHANGE   930579092u
#define NAMED_ARG_ACTION_FONT         969538125u
#define NAMED_ARG_CHAT_X              976240236u
#define NAMED_ARG_CHAT_Y              993017855u
#define NAMED_ARG_ADD                 993596020u
#define NAMED_ARG_LANDSCAPEDISTANCE   1025569689u
#define NAMED_ARG_UNIQUE              1035877158u
#define NAMED_ARG_CHAT_W              1093683569u
#define NAMED_ARG_TO                  1111836708u
#define NAMED_ARG_SPEEDFOG            1127042784u
#define NAMED_ARG_DELETEPROPERTY      1140007295u
#define NAMED_ARG_MAXOBJECTS          1143128927u
#define NAMED_ARG_PARAMETERS          1218784985u
#define NAMED_ARG_LIMITTO             1221296253u
#define NAMED_ARG_CHAT_H              1244682140u
#define NAMED_ARG_GUST                1356619534u
#define NAMED_ARG_WANTEDOVERCAST      1364610859u
#define NAMED_ARG_RAINDENSITY         1377814941u
#define NAMED_ARG_UIBOTTOMRIGHTX      1389361218u
#define NAMED_ARG_CLIP                1393659643u
#define NAMED_ARG_MP                  1395526040u
#define NAMED_ARG_UIBOTTOMRIGHTY      1406138837u
#define NAMED_ARG_ADDEMPTY            1409847989u
#define NAMED_ARG_CORRESPOND          1411193268u
#define NAMED_ARG_WANTEDFOG           1431950518u
#define NAMED_ARG_CLASSPATH           1480922488u
#define NAMED_ARG_SKIPMATH            1492774488u
#define NAMED_ARG_TANK_H              1501737790u
#define NAMED_ARG_CHAT_COLORGLOBAL    1546100952u
#define NAMED_ARG_WORDS               1555540618u
#define NAMED_ARG_SMOKE               1611018502u
#define NAMED_ARG_VISUALQUALITY       1629059530u
#define NAMED_ARG_CHAT_COLORSIDE      1661343704u
#define NAMED_ARG_TANK_W              1686291599u
#define NAMED_ARG_RAINDENSITYSPEED    1692261480u
#define NAMED_ARG_START               1697318111u
#define NAMED_ARG_CLOUDLETS           1698501532u
#define NAMED_ARG_EXCLUDE             1717269161u
#define NAMED_ARG_SETPOSASL           1718914665u
#define NAMED_ARG_COMMENT             1738982494u
#define NAMED_ARG_DELETE              1740784714u
#define NAMED_ARG_KEY                 1746258028u
#define NAMED_ARG_TANK_X              1770179694u
#define NAMED_ARG_EXTCAMERAPOSITION   1773534464u
#define NAMED_ARG_TANK_Y              1786957313u
#define NAMED_ARG_END                 1787721130u
#define NAMED_ARG_DELIMITER           1813362170u
#define NAMED_ARG_RADAR_H             1827875692u
#define NAMED_ARG_ACTION_COLORBACK    1832427264u
#define NAMED_ARG_CHAT_COLORBACK      1906966444u
#define NAMED_ARG_PIPEBOMB            1919016421u
#define NAMED_ARG_RADARDISTANCE       1936064660u
#define NAMED_ARG_THUNDERBOLTTIME     1963663307u
#define NAMED_ARG_UITOPLEFTY          2010304594u
#define NAMED_ARG_UITOPLEFTX          2027082213u
#define NAMED_ARG_RIGHT               2028154341u
#define NAMED_ARG_OUTPUT              2041138948u
#define NAMED_ARG_SUBDIRS             2041531509u
#define NAMED_ARG_GROUPDIR_W          2063515065u
#define NAMED_ARG_REVERSECASE         2081347313u
#define NAMED_ARG_RADAR_X             2096317596u
#define NAMED_ARG_RADAR_Y             2113095215u
#define NAMED_ARG_LOWERCASE           2115640198u
#define NAMED_ARG_WRAP                2145121445u
#define NAMED_ARG_GRAVACC             2164471156u
#define NAMED_ARG_RENAME              2180167635u
#define NAMED_ARG_CHAT_COLORTEAM      2184555562u
#define NAMED_ARG_IGNORELEADSPACE     2190487949u
#define NAMED_ARG_SENTENCES           2195343877u
#define NAMED_ARG_LENGTH              2211460629u
#define NAMED_ARG_SPEEDOVERCAST       2213568761u
#define NAMED_ARG_RADAR_W             2213760929u
#define NAMED_ARG_GROUPDIR_X          2214513636u
#define NAMED_ARG_ACTUALFOG           2217812121u
#define NAMED_ARG_PATH                2223459638u
#define NAMED_ARG_GROUPDIR_Y          2231291255u
#define NAMED_ARG_INVTRACKTIME        2240043054u
#define NAMED_ARG_LASTWINDSPEEDCHANGE 2274646378u
#define NAMED_ARG_SPLIT               2276994531u
#define NAMED_ARG_MAXLIGHTS           2281039248u
#define NAMED_ARG_WAIT                2301512864u
#define NAMED_ARG_CHAT_COLORDIRECT    2373422770u
#define NAMED_ARG_EVENTID             2407974098u
#define NAMED_ARG_KEEPWWW             2464564123u
#define NAMED_ARG_POSITION            2471448074u
#define NAMED_ARG_ACTUALOVERCAST      2480966134u
#define NAMED_ARG_GROUPDIR_H          2482955540u
#define NAMED_ARG_TOKEN               2491017778u
#define NAMED_ARG_FLARE               2525185847u
#define NAMED_ARG_STARTOFFSET         2535189924u
#define NAMED_ARG_RAINDENSITYWANTED   2561392054u
#define NAMED_ARG_ESCAPE              2652972038u
#define NAMED_ARG_REPLACE             2704835779u
#define NAMED_ARG_TIMEBOMB            2752571412u
#define NAMED_ARG_ENDFIND             2764741869u
#define NAMED_ARG_OBJECTSDISTANCE     2825239140u
#define NAMED_ARG_CLOUDSALPHA         2853247177u
#define NAMED_ARG_FILE                2867484483u
#define NAMED_ARG_WEATHERTIME         2922057354u
#define NAMED_ARG_CHAT_ROWS           2996056849u
#define NAMED_ARG_DESTINATION         3000919231u
#define NAMED_ARG_FLAREDURATION       3022446051u
#define NAMED_ARG_LOWER               3038577850u
#define NAMED_ARG_ACTION_ROWS         3073035893u
#define NAMED_ARG_OBJECT              3099987130u
#define NAMED_ARG_MERGE               3111536167u
#define NAMED_ARG_SHORT               3122818005u
#define NAMED_ARG_FOVLEFT             3130206285u
#define NAMED_ARG_TEXT                3185987134u
#define NAMED_ARG_MIDDLESINGLE        3186492500u
#define NAMED_ARG_FIND                3186656602u
#define NAMED_ARG_WRITE               3190202204u
#define NAMED_ARG_CHAT_SIZE           3196690805u
#define NAMED_ARG_OBJECTSHADOWS       3236536979u
#define NAMED_ARG_CASESENSITIVE       3299045579u
#define NAMED_ARG_STARTFIND           3313319524u
#define NAMED_ARG_LEADER_H            3325274155u
#define NAMED_ARG_DELETECLASS         3365088058u
#define NAMED_ARG_MIDDLE              3380782872u
#define NAMED_ARG_BOMB                3399030161u
#define NAMED_ARG_SYSTEMTIME          3424565213u
#define NAMED_ARG_READSETPOS          3428344371u
#define NAMED_ARG_QUOTESKIP           3452202332u
#define NAMED_ARG_READ                3470762949u
#define NAMED_ARG_ACTION_COLORSEL     3522204709u
#define NAMED_ARG_VEHICLESHADOWS      3526923404u
#define NAMED_ARG_LEADER_W            3543383202u
#define NAMED_ARG_OPEN                3546203337u
#define NAMED_ARG_DATE                3564297305u
#define NAMED_ARG_LEADER_Y            3576938440u
#define NAMED_ARG_FINDCHAR            3581622558u
#define NAMED_ARG_LEADER_X            3593716059u
#define NAMED_ARG_ONLYNAME            3599284660u
#define NAMED_ARG_ACTION_COLORTEXT    3629826064u
#define NAMED_ARG_COMPASS_W           3631425893u
#define NAMED_ARG_COMPASS_X           3648203512u
#define NAMED_ARG_MAXWAVE             3661861568u
#define NAMED_ARG_COMPASS_Y           3664981131u
#define NAMED_ARG_REMOVE              3683784189u
#define NAMED_ARG_PATHPOS             3740902012u
#define NAMED_ARG_NATURAL             3753552150u
#define NAMED_ARG_VERIFY              3838716196u
#define NAMED_ARG_SEAWAVESPEED        3851094496u
#define NAMED_ARG_ROCKET              3866886229u
#define NAMED_ARG_SUFFIX              3901637708u
#define NAMED_ARG_BULLET              3902055289u
#define NAMED_ARG_CLOUDSBRIGHTNESS    3906663404u
#define NAMED_ARG_COMPASS_H           3916645416u
#define NAMED_ARG_OVERWRITE           3951415538u
#define NAMED_ARG_MODE                3966689298u
#define NAMED_ARG_HINT_H              4019348663u
#define NAMED_ARG_RENAMECLASS         4024152453u
#define NAMED_ARG_HINT_W              4103236758u
#define NAMED_ARG_COLUMN              4126724647u
#define NAMED_ARG_CUT                 4132842947u
#define NAMED_ARG_RADIOMENU_X         4144824460u
#define NAMED_ARG_RADIOMENU_Y         4161602079u
#define NAMED_ARG_MATCHWORD           4179106050u
#define NAMED_ARG_PICK                4198624760u
#define NAMED_ARG_RANGE               4208725202u
#define NAMED_ARG_PREFIX              4232466889u
#define NAMED_ARG_CHAT_COLORVEHICLE   4242011889u
#define NAMED_ARG_SHADOWSDISTANCE     4245409149u
#define NAMED_ARG_TEXT2               4253867236u
#define NAMED_ARG_RADIOMENU_W         4262267793u
#define NAMED_ARG_HINT_Y              4271012948u
#define NAMED_ARG_HINT_X              4287790567u














// Parse and execute Fwatch command
void ParseScript(String &command, FULLHANDLE file) 
{
	global.out                 = file.handle;
	global.option_error_output = OPTION_NONE;

	// Tokenize and hash command line
	bool match_command                     = true;
	bool expect_space                      = false;
	bool named_arguments                   = false;
	bool enable_unit_separator             = false;
	bool in_quote                          = false;
	bool named_arg_in_quote                = false;
	bool is_name                           = true;
	size_t argument_num                    = 0;
	String argument[MAX_PARAMS]            = {0};
	unsigned int argument_hash[MAX_PARAMS] = {0};

	for (size_t i=0;  i<=command.length && argument_num<MAX_PARAMS-2;  i++) {		
		// End of the word
		if (
			(isspace(command.text[i]) && !enable_unit_separator && !in_quote)  ||              // whitespace not in quote ends an argument
			command.text[i] == '\0'  ||                                                        // zero terminator ends all types of arguments
			(!enable_unit_separator && named_arguments && command.text[i]==':' && is_name)  || // colon ends name part of the named argument
			(enable_unit_separator && command.text[i]==0x1F)  ||                               // unit separator ends special named arguments
			(named_arg_in_quote && command.text[i]=='\"')                                      // quote ends named argument that started with a quote
		) {
			// If named argument started with quotation mark
			if (!enable_unit_separator  &&  named_arguments  &&  command.text[i]==':'  &&  in_quote) {
				argument[argument_num].text++;
				named_arg_in_quote = true;
			}

			// When named argument ends with quotation mark then end on it
			if (named_arg_in_quote  &&  command.text[i]=='\"') {
				named_arg_in_quote = false;
				in_quote           = false;
			}

			// Determine length of the argument
			if (expect_space) {
				expect_space                  = false;
				argument[argument_num].length = command.text+i - argument[argument_num].text;
				argument_num++;
			}

			if (argument_num > 1) {
				char current_character = command.text[i];
				command.text[i]        = '\0';

				// Hash command name
				if (argument_num==2  &&  match_command) {
					match_command       = false;
					argument_hash[0]    = fnv1a_hash(FNV_BASIS, argument[0].text, i, OPTION_LOWERCASE);
					named_arguments     = binary_search(argument_hash[0], commands_named_arguments, 0, sizeof(commands_named_arguments)/sizeof(commands_named_arguments[0])-1) >= 0;
				}
				
				// Hash named arguments
				if (named_arguments && current_character==':') {
					argument_hash[argument_num-1] = fnv1a_hash(FNV_BASIS, argument[argument_num-1].text, command.text+i-argument[argument_num-1].text, OPTION_LOWERCASE);
					is_name                       = false;
					
					// Mark beginning of the next argument (in case it's empty)
					expect_space                = true;
					argument[argument_num].text = command.text + i + 1;
					
					// Determine if argument is terminated by unit separator (0x1F)
					switch(argument_hash[0]) {
						case C_CLASS_MODTOK : 
							enable_unit_separator = argument_hash[argument_num-1]==NAMED_ARG_ADD || argument_hash[argument_num-1]==NAMED_ARG_APPEND; 
							break;

						case C_CLASS_WRITE : 
							enable_unit_separator = argument_hash[argument_num-1] == NAMED_ARG_MERGE; 
							break;

						case C_IGSE_FIND : 
							enable_unit_separator = argument_hash[argument_num-1]==NAMED_ARG_TEXT || argument_hash[argument_num-1]==NAMED_ARG_REPLACE; 
							break;

						case C_IGSE_DB : {
							unsigned int named_arguments_igse_db[] = {
								NAMED_ARG_APPEND,
								NAMED_ARG_KEY,
								NAMED_ARG_RENAME,
								NAMED_ARG_WRITE,
								NAMED_ARG_READ,
								NAMED_ARG_REMOVE
							};

							enable_unit_separator = binary_search(argument_hash[argument_num-1], named_arguments_igse_db, 0, sizeof(named_arguments_igse_db)/sizeof(named_arguments_igse_db[0])-1) >= 0;
						} break;

						case C_STRING_REPLACE : 
						case C_STRING_REPLACECHAR :
							enable_unit_separator = argument_hash[argument_num-1]==NAMED_ARG_TEXT || argument_hash[argument_num-1]==NAMED_ARG_FIND || argument_hash[argument_num-1]==NAMED_ARG_REPLACE; 
							break;

						case C_STRING_COMPARE : 
							enable_unit_separator = argument_hash[argument_num-1]==NAMED_ARG_TEXT1 || argument_hash[argument_num-1]==NAMED_ARG_TEXT2; 
							break;

						case C_CLIP_COPY : 
						case C_IGSE_WRITE : 
						case C_STRING_SIZE :
						case C_STRING_ISNUMBER : 
						case C_STRING_ISVARIABLE :
						case C_STRING_TRIM : 
						case C_STRING_SPLIT : 
						case C_STRING_CASE : 
						case C_STRING_WORDPOS : 
							enable_unit_separator = argument_hash[argument_num-1] == NAMED_ARG_TEXT; break;

						case C_STRING_TOKENIZE : 
							enable_unit_separator = argument_hash[argument_num-1]==NAMED_ARG_TEXT || argument_hash[argument_num-1]==NAMED_ARG_DELIMITER;
							break;

						case C_STRING_FIND : 
							enable_unit_separator = argument_hash[argument_num-1]==NAMED_ARG_TEXT || argument_hash[argument_num-1]==NAMED_ARG_FIND || argument_hash[argument_num-1]==NAMED_ARG_FINDCHAR;
							break;

						case C_STRING_JOIN : 
							enable_unit_separator = argument_hash[argument_num-1]==NAMED_ARG_TEXT || argument_hash[argument_num-1]==NAMED_ARG_MERGE;
							break;
							
						case C_STRING_CUT :
							enable_unit_separator = argument_hash[argument_num-1]==NAMED_ARG_TEXT || argument_hash[argument_num-1]==NAMED_ARG_STARTFIND || argument_hash[argument_num-1]==NAMED_ARG_ENDFIND; break;

						case C_RESTART_SCHEDULE :
							enable_unit_separator = argument_hash[argument_num-1]==NAMED_ARG_PARAMETERS || argument_hash[argument_num-1]==NAMED_ARG_COMMENT; break;
					}
				} else
					is_name = true;
				
				if (enable_unit_separator  &&  current_character==0x1F)
					enable_unit_separator = false;
			}
		} else {
			// Beginning of the word
			if (!expect_space) {
				expect_space                = true;
				argument[argument_num].text = command.text + i;
			}
				
			if (command.text[i] == '\"' && !enable_unit_separator)
				in_quote = !in_quote;
		}

		// turn record separator back to the unit separator (IGSE_LOAD converts it)
		if (command.text[i] == 0x1E)	
			command.text[i] = 0x1F;
	}
	

	// If it's a memory command then open game process
	SIZE_T stBytes = 0;
	HANDLE phandle = NULL;

	if (binary_search(argument_hash[0], commands_memory, 0, sizeof(commands_memory) / sizeof(commands_memory[0])-1) >= 0) {
		phandle = OpenProcess(PROCESS_ALL_ACCESS, 0, global.pid);

		if (!phandle) {
			if (argument_hash[0] == C_INFO_STARTTIME) {
				global.option_error_output = OPTION_ERROR_ARRAY_CLOSE;
				QWrite_err(FWERROR_WINAPI, 1, GetLastError());
			} else
				QWrite("ERROR: Couldn't get process handle");

			return;
		}
	}


	// Default empty value for text arguments
	char empty_char[]               = "";
	const size_t empty_char_index   = MAX_PARAMS - 1;
	argument[empty_char_index].text = empty_char;
	String empty_string             = {empty_char, 0};

	// Execute command
	switch (argument_hash[0]) {
		#include "info_commands.cpp"
		#include "string_commands.cpp"
		#include "file_commands.cpp"
		#include "input_commands.cpp"
		#include "mem_commands.cpp"
		#include "exe_commands.cpp"
		#include "clip_commands.cpp"
		#include "class_commands.cpp"
		#include "igse_commands.cpp"

		default:
			QWritef("ERROR: Unknown command %s", command.text);
			break;
	}

	if (phandle) 
		CloseHandle(phandle);
} 

#include "_Functions.cpp"