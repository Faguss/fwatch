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

extern GLOBAL_VARIABLES_TESTDLL global;

#define SCRIPT_VERSION	1.16		// Script engine version
#define MAX_PARAMS		32			// Maximum parameters for commands
#define WGET_MINWAIT	1000
#define PIPEBUFSIZE		4096

enum COMMAND_HASHES {
	C_CLASS_LIST          = 3894729449u,
	C_CLASS_TOKEN         = 1306216122u,
	C_CLASS_MODIFY        = 3457835787u,
	C_CLASS_MODTOK        = 2686266215u,
	C_CLASS_READ          = 1269173709u,
	C_CLASS_READ2         = 359139437u,
	C_CLASS_READSQM       = 1404966420u,
	C_CLASS_WRITE         = 2805421668u,
	C_CLIP_COPY           = 747579208u,
	C_CLIP_GET            = 2069825331u,
	C_CLIP_GETLINE        = 1221872377u,
	C_CLIP_SIZE           = 809306128u,
	C_CLIP_TOFILE         = 2539841174u,
	C_CLIP_FROMFILE       = 3730249919u,
	C_CLIP_COPYFILE       = 8450590u,
	C_CLIP_CUTFILE        = 1451318777u,
	C_CLIP_PASTEFILE      = 962169530u,
	C_SPIG_EXE            = 3206713566u,
	C_EXE_MAKEPBO         = 3754554022u,
	C_EXE_ADDONTEST       = 501781933u,
	C_EXE_ADDONINSTALL    = 367297688u,
	C_EXE_UNPBO           = 1304888561u,
	C_EXE_WGET            = 3861629986u,
	C_EXE_PREPROCESS      = 2891211499u,
	C_RESTART_CLIENT      = 1040407795u,
	C_RESTART_SERVER      = 1698669191u,
	C_EXE_WEBSITE         = 226367578u,
	C_FILE_EXISTS         = 1534773825u,
	C_FILE_READ           = 4131020217u,
	C_FILE_WRITE          = 11066784u,
	C_FILE_QWRITE         = 2784837127u,
	C_FILE_AWRITE         = 2781408183u,
	C_FILE_VARS           = 3245436723u,
	C_FILE_READVARS       = 2763768851u,
	C_FILE_REMOVE         = 2395420265u,
	C_FILE_DELETE         = 395478174u,
	C_FILE_WGET           = 3189353990u,
	C_FILE_DXDLL          = 615999755u,
	C_FILE_READ2          = 685851089u,
	C_FILE_MODLIST        = 2709572447u,
	C_IGSE_WRITE          = 1567449702u,
	C_IGSE_LIST           = 3690437211u,
	C_IGSE_LOAD           = 3760381619u,
	C_IGSE_NEW            = 3979929763u,
	C_IGSE_RENAME         = 731063145u,
	C_IGSE_COPY           = 1607149306u,
	C_IGSE_FIND           = 519206608u,
	C_IGSE_DB             = 1686459221u,
	C_INFO_VERSION        = 682326273u,
	C_INFO_DEBUGOUT       = 1250266690u,
	C_INFO_DATE           = 2828831323u,
	C_INFO_RESOLUTION     = 1512440909u,
	C_INFO_STARTTIME      = 3949992052u,
	C_INFO_TICK           = 3594399694u,
	C_INFO_ERRORLOG       = 2670383007u,
	C_INPUT_GETKEYS       = 529347995u,
	C_INPUT_AGETKEYS      = 1623224776u,
	C_INPUT_GETKEY        = 1580245162u,
	C_INPUT_AGETKEY       = 1889404011u,
	C_INPUT_GETMOUSE      = 4042371078u,
	C_INPUT_SETMOUSE      = 1710720178u,
	C_INPUT_LOCK          = 2534329742u,
	C_INPUT_GETJOYSTICK   = 341548345u,
	C_INPUT_MULTI         = 324077510u,
	C_MEM_GETCURSOR       = 1060535396u,
	C_MEM_SETCURSOR       = 4120146568u,
	C_MEM_GETWORLD        = 3684053402u,
	C_MEM_GETMAP          = 2119371472u,
	C_MEM_SETMAP          = 3859369924u,
	C_MEM_GETGRAPHICS     = 3949774627u,
	C_MEM_GETJOYSTICK     = 3474622136u,
	C_MEM_GETSCROLL       = 561072295u,
	C_MEM_GETPLAYERANIM   = 2961889542u,
	C_MEM_SETPLAYERANIM   = 856509018u,
	C_MEM_GETCAM          = 4058691877u,
	C_MEM_GETCINEMABORDER = 2974059547u,
	C_MEM_GETNV           = 1457539578u,
	C_MEM_GETRESPAWNTYPE  = 199500974u,
	C_MEM_SETRESPAWNTYPE  = 1059833330u,
	C_MEM_GETRESSIDE      = 1638557187u,
	C_MEM_GETDAYLIGHT     = 4098054280u,
	C_MEM_GETDATE         = 1428058178u,
	C_MEM_GETPLAYERVIEW   = 956477386u,
	C_MEM_SETPLAYERVIEW   = 4154429014u,
	C_MEM_ERROR           = 3304531164u,
	C_MEM_GETPLAYERAIM    = 526157708u,
	C_MEM_SETPLAYERAIM    = 1706376064u,
	C_MEM_MODLIST         = 1096286730u,
	C_MEM_GETDIFFICULTY   = 1176805931u,
	C_MEM_SETDIFFICULTY   = 1089543055u,
	C_MEM_ISDIALOG        = 3850265438u,
	C_MEM_GETRADIOBOX     = 2504840282u,
	C_MEM_SETRADIOBOX     = 790714550u,
	C_MEM_SETGRAPHICS     = 1044199951u,
	C_MEM_GETSPEEDKEY     = 3620598336u,
	C_MEM_SETSPEEDKEY     = 861867444u,
	C_MEM_GETPLAYERHATCH  = 1766714027u,
	C_MEM_SETPLAYERHATCH  = 4119745471u,
	C_MEM_GETPLAYERLADDER = 3090776391u,
	C_MEM_SETPLAYERLADDER = 2409418555u,
	C_MEM_MULTI           = 351704443u,
	C_MEM_MASTERSERVER    = 523283091u,
	C_MEM_MISSIONINFO     = 733488320u,
	C_MEM_BULLETS         = 1300677055u,
	C_MEM_GETWEATHER      = 598054334u,
	C_MEM_SETWEATHER      = 3155389850u,
	C_MEM_SETCAM          = 2822225721u,
	C_MEM_HUD             = 2366644167u,
	C_STRING_FIRST        = 3570665910u,
	C_STRING_LAST         = 1839646952u,
	C_STRING_LENGTH       = 3665905396u,
	C_STRING_RANGE        = 682937149u,
	C_STRING_TOLOWER      = 4292283326u,
	C_STRING_TOUPPER      = 1500743211u,
	C_STRING_TOARRAY      = 868016138u,
	C_STRING_INDEX        = 2627341444u,
	C_STRING_SIZE         = 658852761u,
	C_STRING_REPLACE      = 1348713056u,
	C_STRING_COMPARE      = 2284732455u,
	C_STRING_ISNUMBER     = 680534929u,
	C_STRING_ISVARIABLE   = 745200000u,
	C_STRING_ISEMPTY      = 3378770225u,
	C_STRING_CUT          = 702942952u,
	C_STRING_TOKENIZE     = 2000539349u,
	C_STRING_TRIM         = 4109809924u,
	C_STRING_FIND         = 3673872707u,
	C_STRING_SPLIT        = 1262498968u,
	C_STRING_DOMAIN       = 2829566612u,
	C_STRING_CASE         = 3135263444u,
	C_STRING_REPLACECHAR  = 3094755036u,
	C_STRING_JOIN         = 4131562040u,
	C_STRING_WORDPOS      = 1620765186u,
};

unsigned int commands_named_arguments[] = {
	C_CLASS_READ2,
	C_IGSE_FIND,
	C_STRING_ISNUMBER,
	C_STRING_CUT,
	C_IGSE_RENAME,
	C_STRING_ISVARIABLE,
	C_CLIP_COPY,
	C_MEM_SETGRAPHICS,
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
	C_STRING_JOIN
};

unsigned int commands_memory[] = {
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

enum NAMED_ARGUMENTS {
	NAMED_ARG_TEXT1               = 9232797u,
	NAMED_ARG_SKYTHROUGH          = 64651335u,
	NAMED_ARG_APPEND              = 110723809u,
	NAMED_ARG_RADIOMENU_H         = 118299068u,
	NAMED_ARG_INDEX               = 151693739u,
	NAMED_ARG_UPPER               = 176974407u,
	NAMED_ARG_LIST                = 217798785u,
	NAMED_ARG_ACTION_H            = 277796120u,
	NAMED_ARG_GUSTUNTIL           = 293809950u,
	NAMED_ARG_DIRECTORY           = 294729096u,
	NAMED_ARG_SHELL               = 300022785u,
	NAMED_ARG_CLOUDSPOS           = 303233623u,
	NAMED_ARG_LEFT                = 306900080u,
	NAMED_ARG_WINDSPEED           = 328211058u,
	NAMED_ARG_OFFSET              = 348705738u,
	NAMED_ARG_LINE                = 400234023u,
	NAMED_ARG_VIEWDISTANCE        = 405576979u,
	NAMED_ARG_MAXTIDE             = 437406701u,
	NAMED_ARG_TRACKTIME           = 459210109u,
	NAMED_ARG_SOURCE              = 466561496u,
	NAMED_ARG_ACTION_SIZE         = 467552881u,
	NAMED_ARG_ACTION_W            = 529460405u,
	NAMED_ARG_ACTION_X            = 546238024u,
	NAMED_ARG_ACTION_Y            = 563015643u,
	NAMED_ARG_SIZE                = 597743964u,
	NAMED_ARG_CHAT_ENABLE         = 621100091u,
	NAMED_ARG_FOVTOP              = 647594911u,
	NAMED_ARG_MAXLEVEL            = 650761905u,
	NAMED_ARG_CLOUDSSPEED         = 675179266u,
	NAMED_ARG_RENAMEPROPERTY      = 769516826u,
	NAMED_ARG_WORDDELIMITER       = 834005314u,
	NAMED_ARG_LIMIT               = 853203252u,
	NAMED_ARG_BRIGHTNESS          = 886071850u,
	NAMED_ARG_CHAT_FONT           = 902884201u,
	NAMED_ARG_ENDOFFSET           = 906008637u,
	NAMED_ARG_NEXTWEATHERCHANGE   = 930579092u,
	NAMED_ARG_ACTION_FONT         = 969538125u,
	NAMED_ARG_CHAT_X              = 976240236u,
	NAMED_ARG_CHAT_Y              = 993017855u,
	NAMED_ARG_ADD                 = 993596020u,
	NAMED_ARG_LANDSCAPEDISTANCE   = 1025569689u,
	NAMED_ARG_UNIQUE              = 1035877158u,
	NAMED_ARG_CHAT_W              = 1093683569u,
	NAMED_ARG_TO                  = 1111836708u,
	NAMED_ARG_SPEEDFOG            = 1127042784u,
	NAMED_ARG_DELETEPROPERTY      = 1140007295u,
	NAMED_ARG_MAXOBJECTS          = 1143128927u,
	NAMED_ARG_CHAT_H              = 1244682140u,
	NAMED_ARG_GUST                = 1356619534u,
	NAMED_ARG_WANTEDOVERCAST      = 1364610859u,
	NAMED_ARG_RAINDENSITY         = 1377814941u,
	NAMED_ARG_UIBOTTOMRIGHTX      = 1389361218u,
	NAMED_ARG_CLIP                = 1393659643u,
	NAMED_ARG_MP                  = 1395526040u,
	NAMED_ARG_UIBOTTOMRIGHTY      = 1406138837u,
	NAMED_ARG_ADDEMPTY            = 1409847989u,
	NAMED_ARG_CORRESPOND          = 1411193268u,
	NAMED_ARG_WANTEDFOG           = 1431950518u,
	NAMED_ARG_CLASSPATH           = 1480922488u,
	NAMED_ARG_SKIPMATH            = 1492774488u,
	NAMED_ARG_TANK_H              = 1501737790u,
	NAMED_ARG_CHAT_COLORGLOBAL    = 1546100952u,
	NAMED_ARG_WORDS               = 1555540618u,
	NAMED_ARG_SMOKE               = 1611018502u,
	NAMED_ARG_VISUALQUALITY       = 1629059530u,
	NAMED_ARG_CHAT_COLORSIDE      = 1661343704u,
	NAMED_ARG_TANK_W              = 1686291599u,
	NAMED_ARG_RAINDENSITYSPEED    = 1692261480u,
	NAMED_ARG_START               = 1697318111u,
	NAMED_ARG_CLOUDLETS           = 1698501532u,
	NAMED_ARG_EXCLUDE             = 1717269161u,
	NAMED_ARG_DELETE              = 1740784714u,
	NAMED_ARG_KEY                 = 1746258028u,
	NAMED_ARG_TANK_X              = 1770179694u,
	NAMED_ARG_EXTCAMERAPOSITION   = 1773534464u,
	NAMED_ARG_TANK_Y              = 1786957313u,
	NAMED_ARG_END                 = 1787721130u,
	NAMED_ARG_DELIMITER           = 1813362170u,
	NAMED_ARG_RADAR_H             = 1827875692u,
	NAMED_ARG_ACTION_COLORBACK    = 1832427264u,
	NAMED_ARG_CHAT_COLORBACK      = 1906966444u,
	NAMED_ARG_PIPEBOMB            = 1919016421u,
	NAMED_ARG_RADARDISTANCE       = 1936064660u,
	NAMED_ARG_THUNDERBOLTTIME     = 1963663307u,
	NAMED_ARG_UITOPLEFTY          = 2010304594u,
	NAMED_ARG_UITOPLEFTX          = 2027082213u,
	NAMED_ARG_RIGHT               = 2028154341u,
	NAMED_ARG_SUBDIRS             = 2041531509u,
	NAMED_ARG_GROUPDIR_W          = 2063515065u,
	NAMED_ARG_REVERSECASE         = 2081347313u,
	NAMED_ARG_RADAR_X             = 2096317596u,
	NAMED_ARG_RADAR_Y             = 2113095215u,
	NAMED_ARG_WRAP                = 2145121445u,
	NAMED_ARG_GRAVACC             = 2164471156u,
	NAMED_ARG_RENAME              = 2180167635u,
	NAMED_ARG_CHAT_COLORTEAM      = 2184555562u,
	NAMED_ARG_IGNORELEADSPACE     = 2190487949u,
	NAMED_ARG_SENTENCES           = 2195343877u,
	NAMED_ARG_LENGTH              = 2211460629u,
	NAMED_ARG_SPEEDOVERCAST       = 2213568761u,
	NAMED_ARG_RADAR_W             = 2213760929u,
	NAMED_ARG_GROUPDIR_X          = 2214513636u,
	NAMED_ARG_ACTUALFOG           = 2217812121u,
	NAMED_ARG_PATH                = 2223459638u,
	NAMED_ARG_GROUPDIR_Y          = 2231291255u,
	NAMED_ARG_INVTRACKTIME        = 2240043054u,
	NAMED_ARG_LASTWINDSPEEDCHANGE = 2274646378u,
	NAMED_ARG_SPLIT               = 2276994531u,
	NAMED_ARG_MAXLIGHTS           = 2281039248u,
	NAMED_ARG_WAIT                = 2301512864u,
	NAMED_ARG_CHAT_COLORDIRECT    = 2373422770u,
	NAMED_ARG_POSITION            = 2471448074u,
	NAMED_ARG_ACTUALOVERCAST      = 2480966134u,
	NAMED_ARG_GROUPDIR_H          = 2482955540u,
	NAMED_ARG_TOKEN               = 2491017778u,
	NAMED_ARG_FLARE               = 2525185847u,
	NAMED_ARG_STARTOFFSET         = 2535189924u,
	NAMED_ARG_RAINDENSITYWANTED   = 2561392054u,
	NAMED_ARG_ESCAPE              = 2652972038u,
	NAMED_ARG_REPLACE             = 2704835779u,
	NAMED_ARG_TIMEBOMB            = 2752571412u,
	NAMED_ARG_ENDFIND             = 2764741869u,
	NAMED_ARG_OBJECTSDISTANCE     = 2825239140u,
	NAMED_ARG_CLOUDSALPHA         = 2853247177u,
	NAMED_ARG_FILE                = 2867484483u,
	NAMED_ARG_WEATHERTIME         = 2922057354u,
	NAMED_ARG_CHAT_ROWS           = 2996056849u,
	NAMED_ARG_DESTINATION         = 3000919231u,
	NAMED_ARG_FLAREDURATION       = 3022446051u,
	NAMED_ARG_LOWER               = 3038577850u,
	NAMED_ARG_ACTION_ROWS         = 3073035893u,
	NAMED_ARG_MERGE               = 3111536167u,
	NAMED_ARG_FOVLEFT             = 3130206285u,
	NAMED_ARG_TEXT                = 3185987134u,
	NAMED_ARG_MIDDLESINGLE        = 3186492500u,
	NAMED_ARG_FIND                = 3186656602u,
	NAMED_ARG_WRITE               = 3190202204u,
	NAMED_ARG_CHAT_SIZE           = 3196690805u,
	NAMED_ARG_OBJECTSHADOWS       = 3236536979u,
	NAMED_ARG_CASESENSITIVE       = 3299045579u,
	NAMED_ARG_STARTFIND           = 3313319524u,
	NAMED_ARG_LEADER_H            = 3325274155u,
	NAMED_ARG_DELETECLASS         = 3365088058u,
	NAMED_ARG_MIDDLE              = 3380782872u,
	NAMED_ARG_BOMB                = 3399030161u,
	NAMED_ARG_SYSTEMTIME          = 3424565213u,
	NAMED_ARG_QUOTESKIP           = 3452202332u,
	NAMED_ARG_READ                = 3470762949u,
	NAMED_ARG_ACTION_COLORSEL     = 3522204709u,
	NAMED_ARG_VEHICLESHADOWS      = 3526923404u,
	NAMED_ARG_LEADER_W            = 3543383202u,
	NAMED_ARG_OPEN                = 3546203337u,
	NAMED_ARG_LEADER_Y            = 3576938440u,
	NAMED_ARG_LEADER_X            = 3593716059u,
	NAMED_ARG_ONLYNAME            = 3599284660u,
	NAMED_ARG_ACTION_COLORTEXT    = 3629826064u,
	NAMED_ARG_COMPASS_W           = 3631425893u,
	NAMED_ARG_COMPASS_X           = 3648203512u,
	NAMED_ARG_MAXWAVE             = 3661861568u,
	NAMED_ARG_COMPASS_Y           = 3664981131u,
	NAMED_ARG_REMOVE              = 3683784189u,
	NAMED_ARG_PATHPOS             = 3740902012u,
	NAMED_ARG_NATURAL             = 3753552150u,
	NAMED_ARG_VERIFY              = 3838716196u,
	NAMED_ARG_SEAWAVESPEED        = 3851094496u,
	NAMED_ARG_ROCKET              = 3866886229u,
	NAMED_ARG_SUFFIX              = 3901637708u,
	NAMED_ARG_BULLET              = 3902055289u,
	NAMED_ARG_CLOUDSBRIGHTNESS    = 3906663404u,
	NAMED_ARG_COMPASS_H           = 3916645416u,
	NAMED_ARG_OVERWRITE           = 3951415538u,
	NAMED_ARG_MODE                = 3966689298u,
	NAMED_ARG_HINT_H              = 4019348663u,
	NAMED_ARG_RENAMECLASS         = 4024152453u,
	NAMED_ARG_HINT_W              = 4103236758u,
	NAMED_ARG_COLUMN              = 4126724647u,
	NAMED_ARG_CUT                 = 4132842947u,
	NAMED_ARG_RADIOMENU_X         = 4144824460u,
	NAMED_ARG_RADIOMENU_Y         = 4161602079u,
	NAMED_ARG_MATCHWORD           = 4179106050u,
	NAMED_ARG_RANGE               = 4208725202u,
	NAMED_ARG_PREFIX              = 4232466889u,
	NAMED_ARG_CHAT_COLORVEHICLE   = 4242011889u,
	NAMED_ARG_SHADOWSDISTANCE     = 4245409149u,
	NAMED_ARG_TEXT2               = 4253867236u,
	NAMED_ARG_RADIOMENU_W         = 4262267793u,
	NAMED_ARG_HINT_Y              = 4271012948u,
	NAMED_ARG_HINT_X              = 4287790567u
};





// List of accessible locations
const int allowed_paths_num = 8;
static char allowed_paths[allowed_paths_num][32] = {
	"in-game-script-editor",
	"flashpointcutscenemaker",
	"missioneditor3d",
	"@addontest",
	"fwatch\\idb",
	"fwatch/idb",
	"fwatch\\tmp",
	"fwatch/tmp"
};














// Parse a script command
void ParseScript(char *com, FULLHANDLE file) 
{
	global.out = file.handle;

	// Tokenize and hash command line
	bool match_command         = true;
	bool expect_space          = false;
	bool named_arguments       = false;
	bool enable_unit_separator = false;
	bool in_quote              = false;
	bool named_arg_in_quote    = false;
	size_t argument_num        = 0;
	size_t argument_end        = 0;
	char *argument[MAX_PARAMS]             = {0};
	size_t argument_length[MAX_PARAMS]     = {0};
	unsigned int argument_hash[MAX_PARAMS] = {0};

	for (size_t i=0; argument_num<MAX_PARAMS && !argument_end; i++) {
		// End of the string
		if (com[i] == '\0') {
			argument_end = i;

			// If last argument is a named argument with an empty value
			if (named_arguments  &&  argument_hash[argument_num-1]!=0  &&  argument[argument_num]==NULL) {
				expect_space           = true;
				argument[argument_num] = com + i;
			}
		}
		
		// End of the word
		if ((isspace(com[i]) && !enable_unit_separator && !in_quote)  ||  argument_end  ||  (!enable_unit_separator && named_arguments && com[i]==':')  ||  (enable_unit_separator && com[i]==0x1F)  ||  (named_arg_in_quote && com[i]=='\"')) {

			// If named argument started with quotation mark
			if (!enable_unit_separator && named_arguments && com[i]==':' && in_quote) {
				argument[argument_num]++;
				named_arg_in_quote = true;
			}

			// When named argument ends with quotation mark then end on it
			if (named_arg_in_quote && com[i]=='\"') {
				named_arg_in_quote = false;
				in_quote           = false;
			}

			// Determine length of the argument
			if (expect_space) {
				expect_space                  = false;
				argument_length[argument_num] = com+i - argument[argument_num];
				argument_num++;
			}

			if (argument_num > 1) {
				char current_character = com[i];
				com[i]                 = '\0';

				// Hash command name
				if (argument_num==2  &&  match_command) {
					match_command       = false;
					argument_hash[0]    = fnv1a_hash(FNV_BASIS, argument[0], i, OPTION_LOWERCASE);
					named_arguments     = binary_search(argument_hash[0], commands_named_arguments, 0, sizeof(commands_named_arguments)/sizeof(commands_named_arguments[0])-1) >= 0;
					global.command_hash = argument_hash[0];
				}
				
				// Hash named arguments
				if (named_arguments && current_character==':') {
					argument_hash[argument_num-1] = fnv1a_hash(FNV_BASIS, argument[argument_num-1], com+i-argument[argument_num-1], OPTION_LOWERCASE);
					
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
							enable_unit_separator = argument_hash[argument_num-1]==NAMED_ARG_TEXT || argument_hash[argument_num-1]==NAMED_ARG_FIND;
							break;

						case C_STRING_JOIN : 
							enable_unit_separator = argument_hash[argument_num-1]==NAMED_ARG_TEXT || argument_hash[argument_num-1]==NAMED_ARG_MERGE;
							break;
							
						case C_STRING_CUT :
							enable_unit_separator = argument_hash[argument_num-1]==NAMED_ARG_TEXT || argument_hash[argument_num-1]==NAMED_ARG_STARTFIND || argument_hash[argument_num-1]==NAMED_ARG_ENDFIND; break;
					}
				}
				
				if (enable_unit_separator  &&  current_character==0x1F)
					enable_unit_separator = false;
			}
		} else {
			// Beginning of the word
			if (!expect_space) {
				expect_space           = true;
				argument[argument_num] = com + i;
			}
				
			if (com[i] == '\"' && !enable_unit_separator)
				in_quote = !in_quote;
		}

		if (com[i] == 0x1E)	// record separator back to unit separator (was converted by IGSE_LOAD)
			com[i] = 0x1F;
	}
	

	// If it's a memory command then open game process
	SIZE_T stBytes = 0;
	HANDLE phandle = NULL;

	if (binary_search(argument_hash[0], commands_memory, 0, sizeof(commands_memory) / sizeof(commands_memory[0]))-1) {
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


	global.option_error_output = OPTION_NONE;
	char empty_string[]        = "";

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
			QWritef("ERROR: Unknown command %s", com);
			break;
	}

	if (phandle) 
		CloseHandle(phandle);
} 

#include "_Functions.cpp"