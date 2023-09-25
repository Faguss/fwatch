#include "stdafx.h"
#include "resource.h"
#include "common.h"
#include "functions.h"
#include "installer.h"

DWORD WINAPI addonInstallerWrapper(__in LPVOID lpParameter)
{
	UNREFERENCED_PARAMETER(lpParameter);
	DWORD threadID1 = 0;

	global.thread_installer = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)addonInstallerMain, 0, 0,&threadID1);
	if (global.thread_installer)
		WaitForSingleObject(global.thread_installer, INFINITE);
	DisableMenu();

	// If user wants to restart the game after installation
	if (global.restart_game) {
		DeleteFile(L"fwatch\\tmp\\schedule\\install_progress.sqf");
		
		if (global.run_voice_program)
			global.gamerestart_arguments += L" -evoice=" + global.arguments_table[L"evoice"];
		
		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		ZeroMemory(&si, sizeof(si));
		ZeroMemory(&pi, sizeof(pi));
		si.cb          = sizeof(si);
		si.dwFlags     = STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_SHOW;
		std::wstring param = WrapInQuotes(global.working_directory) + L" " + global.gamerestart_arguments;
	 
		if (CreateProcess(L"fwatch\\data\\gameRestart.exe", &param[0], NULL, NULL, TRUE, HIGH_PRIORITY_CLASS, NULL, NULL, &si, &pi))
			LogMessage(L"Executing gameRestart.exe  " + global.gamerestart_arguments);
		else
			LogMessage(L"Failed to launch gameRestart.exe " + FormatError(GetLastError()));
	 
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		
	}

	LogMessage(L"", OPTION_CLOSELOG);

	if (global.restart_game)
		PostQuitMessage(0);

	return 0;
}

DWORD WINAPI addonInstallerMain(__in LPVOID lpParameter)
{
	UNREFERENCED_PARAMETER(lpParameter);
	
	// Process arguments
	global.arguments_table.insert(std::pair<std::wstring,std::wstring>(L"gameversion",L"1.99"));
	global.arguments_table.insert(std::pair<std::wstring,std::wstring>(L"assignid",L""));
	global.arguments_table.insert(std::pair<std::wstring,std::wstring>(L"assignidpath",L""));
	global.arguments_table.insert(std::pair<std::wstring,std::wstring>(L"assignname",L""));
	global.arguments_table.insert(std::pair<std::wstring,std::wstring>(L"assignkeepname",L""));
	global.arguments_table.insert(std::pair<std::wstring,std::wstring>(L"testmod",L""));
	global.arguments_table.insert(std::pair<std::wstring,std::wstring>(L"testdir",L""));
	global.arguments_table.insert(std::pair<std::wstring,std::wstring>(L"installid",L""));
	global.arguments_table.insert(std::pair<std::wstring,std::wstring>(L"installdir",L""));
	global.arguments_table.insert(std::pair<std::wstring,std::wstring>(L"downloadscript",L""));
	global.arguments_table.insert(std::pair<std::wstring,std::wstring>(L"evoice",L""));
	global.arguments_table.insert(std::pair<std::wstring,std::wstring>(L"language",L"English"));

	// Separate arguments:
	// arguments for this program go to the table
	// arguments for gameRestart.exe go to a separate string
	{
		std::vector<std::wstring> argv;
		Tokenize(global.program_arguments, L" ", argv);
	
		for (size_t i=0; i<argv.size(); i++) {
			bool found = false;
		
			for (std::map<std::wstring,std::wstring>::iterator it=global.arguments_table.begin(); it!=global.arguments_table.end(); ++it) {
				std::wstring table_argument = L"-" + it->first + L"=";
			
				if (Equals(argv[i].substr(0, table_argument.length()), table_argument)) {
					it->second = argv[i].substr(table_argument.length());
					found      = true;
					break;
				}
			}
		
			if (!found)
				global.gamerestart_arguments += argv[i] + L" ";
		}
	
		global.test_mode = !(global.arguments_table[L"testmod"].empty());
	
		Tokenize(global.arguments_table[L"installid"] , L",", global.mod_id);
		Tokenize(global.arguments_table[L"installdir"], L",", global.mod_name);
	}

	// Load language
	std::wstring stringtable[][STR_MAX] = {
		{
			L"Initializing",
			L"Fetching installation script",
			L"Reading installation script",
			L"Connecting",
			L"Downloading",
			L"Downloaded",
			L"Extracting",
			L"Unpacking PBO",
			L"Packing PBO",
			L"Copying",
			L"Copying downloaded file to the fwatch\\tmp",
			L"Cleaning up",
			L"Preparing to install a mod",
			L"Deleting",
			L"Renaming",
			L"Editing",
			L"Installation aborted by user",
			L"Installation complete!",
			L"Done\r\nbut mods %MOD% are still missing\r\nOpen fwatch\\data\\addonInstallerLog.txt for details",
			L"Installation progress:",
			L"ALT+TAB to the desktop",
			L"ERROR",
			L"Can't create logfile",
			L"Can't read install script",
			L"Incorrect script version",
			L"In version",
			L"On line",
			L"Failed to launch",
			L"Not enough arguments",
			L"Failed to list files in ",
			L"Missing file name argument",
			L"Path is leaving current directory",
			L"Installation script is invalid",
			L"Invalid installator arguments",
			L"Failed to allocate buffer",
			L"left",
			L"total",
			L"Invalid download destination",
			L"Failed to download",
			L"Failed to find",
			L"remove this file and download again",
			L"Failed to extract",
			L"Failed to create directory",
			L"Failed to get attributes of",
			L"Source path is leaving current directory",
			L"Destination path is leaving current directory",
			L"Not allowed to move files out of the game directory",
			L"Failed to move",
			L"to",
			L"Failed to copy",
			L"Failed to rename",
			L"to",
			L"New file name contains slashes",
			L"Wildcards in the path",
			L"Missing new file name",
			L"Failed to delete",
			L"Failed to move to recycle bin",
			L"You must manually run",
			L"You must manually download",
			L"Select folder with the downloaded file",
			L"Missing version number",
			L"Not a PBO file",
			L"Failed to create PBO",
			L"Failed to unpack PBO",
			L"Failed to read file",
			L"Failed to create file",
			L"Reading mission.sqm",
			L"Select 'Retry' or 'Abort'"
		},
		{
			L"Запуск",
			L"Получение скрипта установки",
			L"Считывание файлов",
			L"Подключение",
			L"Загрузка",
			L"Загрузка завершена",
			L"Извлечение файлов",
			L"Распаковка архива PBO",
			L"Создание архива PBO",
			L"Копирование",
			L"Копирование загруженного файла в fwatch\\tmp",
			L"Удаление временных файлов",
			L"Начало установки мода",
			L"Удаление файлов мода",
			L"Переименование файлов мода",
			L"Редактирование файлов мода",
			L"Установка прервана пользователем",
			L"Установка завершена!",
			L"Установка завершена\r\nно отсутствуют моды %MOD%. Дополнительная информация в fwatch\\data\\addonInstallerLog.txt",
			L"Процесс установки:",
			L"Нажмите ALT+TAB, чтобы свернуть игру",
			L"ОШИБКА",
			L"Невозможно создать файл журнала установки",
			L"Невозможно считать скрипт установки",
			L"Неверная версия скрипта установки",
			L"В версии",
			L"В строке",
			L"Ошибка при запуске",
			L"Недостаточно аргументов функции",
			L"Ошибка при создании списка файлов в папке ",
			L"Отсутствует имя файла аргумента",
			L"Путь не соответствует текущей папке",
			L"Неверный скрипт установки",
			L"Неверные аргументы мастера установки",
			L"Ошибка при выделении понятий",
			L"осталось",
			L"всего",
			L"Неверный путь установки",
			L"Ошибка при загрузке",
			L"Совпадений не найдено",
			L"удалите этот файл и загрузите снова",
			L"Ошибка при извлечении",
			L"Ошибка при создании папки",
			L"Ошибка при считывании атрибутов файла",
			L"Исходный путь не соответствует текущей папке",
			L"Путь назначения не соответствует текущей папке",
			L"Невозможно переместить файлы из папки игры",
			L"Ошибка при перемещении",
			L"в",
			L"Ошибка при копировании",
			L"Ошибка при переименовании файла",
			L"в",
			L"Новое имя файла содержит слеши",
			L"Путь содержит символы подстановки",
			L"Отсутствует новое имя файла",
			L"Ошибка при удалении",
			L"Ошибка при перемещении в Корзину",
			L"Необходимо запустить вручную",
			L"Необходимо загрузить вручную",
			L"Выберите папку с загруженным файлом",
			L"Отсутствует номер версии",
			L"Не файл типа PBO",
			L"Ошибка при создании файла PBO",
			L"Ошибка при извлечении из архива PBO",
			L"Ошибка при считывании",
			L"Ошибка при создании файла",
			L"Считывание mission.sqm",
			L"Выберите 'Заново' или 'Отмена'"
		},
		{
			L"Przygotowywanie",
			L"Pobieraniu skryptu instalacyjnego",
			L"Przetwarzanie skryptu instalacyjnego",
			L"Łączenie",
			L"Pobieranie",
			L"Pobrano",
			L"Wypakowywanie",
			L"Wypakowywanie PBO",
			L"Pakowanie PBO",
			L"Kopiowanie plików",
			L"Kopiowanie plików do fwatch\\tmp",
			L"Porządkowanie",
			L"Przygotowywanie do instalacji modu",
			L"Usuwanie plików",
			L"Zmienianie nazwy plików",
			L"Edytowanie plików",
			L"Instalacja przerwana przez użytkownika",
			L"Instalacja zakończona!",
			L"Koniec instalacji\r\nale brakuje modów %MOD%\r\nSzczegóły w pliku fwatch\\data\\addonInstallerLog.txt",
			L"Postęp instalacji:",
			L"ALT+TAB żeby przejść do pulpitu",
			L"BŁĄD",
			L"Nie można utworzyć zapisu instalacji",
			L"Nie można odczytać skryptu instalacyjnego",
			L"Niepoprawna wersja skryptu instalacyjnego",
			L"W wersji",
			L"W linijce",
			L"Nie można uruchomić",
			L"Brakuje argumentów",
			L"Nie można utworzyć listy plików z ",
			L"Brakuje nazwy pliku",
			L"Ścieżka wychodzi poza obecny katalog",
			L"Skrypt instalacyjny jest błędny",
			L"Błędne argumenty instalatora",
			L"Nie można zarezerwować pamięci",
			L"zostało",
			L"w sumie",
			L"Nieprawidłowy katalog docelowy dla ściągniętego pliku",
			L"Nie można pobrać",
			L"Nie znaleziono",
			L"usuń ten plik i ściągnij ponownie",
			L"Nie można rozpakować",
			L"Nie można utworzyć katalogu",
			L"Nie można odczytać atrybutów",
			L"Ścieżka źródłowa wychodzi poza obecny katalog",
			L"Ścieżka docelowa wychodzi poza obecny katalog",
			L"Nie można przenosić plików poza katalog z grą",
			L"Nie można przenieść",
			L"do",
			L"Nie można skopiować",
			L"Nie można zmienić nazwy",
			L"na",
			L"Nowa nazwa pliku zawiera ukośniki",
			L"Ścieżka zawiera symbole zastępcze",
			L"Brakuje nowej nazwy pliku",
			L"Nie można skasować",
			L"Nie można przenieść do kosza",
			L"Musisz ręcznie uruchomić",
			L"Musisz ręcznie pobrać",
			L"Wybierz katalog z pobranym plikiem",
			L"Brakuje numeru wersji",
			L"Plik nie jest PBO",
			L"Nie można utworzyć PBO",
			L"Nie można rozpakować PBO",
			L"Nie można wczytać pliku",
			L"Nie można utworzyć pliku",
			L"Przetwarzanie mission.sqm",
			L"Wybierz 'Kontynuuj' lub 'Przerwij'"
		}
	};
	global.lang_eng = stringtable[0];
	global.lang     = stringtable[0];
	
	if (Equals(global.arguments_table[L"language"],L"Russian"))
		global.lang = stringtable[1];
	
	if (Equals(global.arguments_table[L"language"],L"Polish"))
		global.lang = stringtable[2];

	// Find current directory
	{
		wchar_t pwd[MAX_PATH];
		GetCurrentDirectory(MAX_PATH, pwd);
		global.working_directory = (std::wstring)pwd;

		// When testing outside of the game change path to the game root dir
		if (global.test_mode) {
			global.working_directory = ReplaceAll(global.working_directory, L"\\fwatch\\data", L"");
			SetCurrentDirectory(global.working_directory.c_str());
		}
	}

	// If ordered to create id file for a mod
	if (!global.arguments_table[L"assignidpath"].empty() && !global.arguments_table[L"assignid"].empty()) {
		global.current_mod = global.arguments_table[L"assignname"];
		WriteModID(global.arguments_table[L"assignidpath"], global.arguments_table[L"assignid"], global.arguments_table[L"assignkeepname"]);
		PostQuitMessage(0);
		return 0;
	}

	WriteProgressFile(INSTALL_PROGRESS, global.lang[STR_ACTION_INIT]);

	// Start listen thread
	DWORD threadID1 = 0;
	global.thread_receiver = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ReceiveInstructions, 0, 0,&threadID1);

	// Create a log file
	{
		global.logfile.open("fwatch\\data\\addonInstallerLog.txt", std::ios::out | std::ios::app | std::ios::binary);

		if (!global.logfile.is_open()) {
			WriteProgressFile(INSTALL_ERROR, (global.lang[STR_ERROR]+L"\r\n"+global.lang[STR_ERROR_LOGFILE]));
			return ERROR_LOGFILE;
		}

		SYSTEMTIME st;
		GetLocalTime(&st);
		LogMessage(L"\r\n--------------\r\n\r\n" + 
			Int2StrW(st.wYear) + L"." + 
			Int2StrW(st.wMonth, OPTION_LEADINGZERO) + L"." + 
			Int2StrW(st.wDay, OPTION_LEADINGZERO) + L"  " + 
			Int2StrW(st.wHour, OPTION_LEADINGZERO) + L":" + 
			Int2StrW(st.wMinute, OPTION_LEADINGZERO) + L":" + 
			Int2StrW(st.wSecond, OPTION_LEADINGZERO)
		);
	}
			
	// Set up global variables for testing mode
	if (global.test_mode) {
		global.current_mod              = global.arguments_table[L"testmod"];
		global.current_mod_version      = L"1";
		global.current_mod_version_date = time(0);
		
		if (global.arguments_table[L"testdir"].empty())
			global.current_mod_new_name = global.arguments_table[L"testmod"];
		else
			global.current_mod_new_name = global.arguments_table[L"testdir"];
			
		LogMessage(L"Test Mode - " + global.current_mod);
		
		if (!Equals(global.current_mod,global.current_mod_new_name))
			LogMessage(L" as " + global.current_mod_new_name);
	}

	// Define allowed commands
	enum SCRIPTING_COMMANDS_ID {
		COMMAND_AUTO_INSTALL,
		COMMAND_DOWNLOAD,
		COMMAND_UNPACK,
		COMMAND_MOVE,
		COMMAND_COPY,
		COMMAND_MAKEDIR,
		COMMAND_ASK_RUN,
		COMMAND_BEGIN_MOD,
		COMMAND_DELETE,
		COMMAND_RENAME,
		COMMAND_ASK_DOWNLOAD,
		COMMAND_IF_VERSION,
		COMMAND_ELSE,
		COMMAND_ENDIF,
		COMMAND_MAKEPBO,
		COMMAND_EXTRACTPBO,
		COMMAND_EDIT,
		COMMAND_BEGIN_VERSION,
		COMMAND_ALIAS,
		COMMAND_FILEDATE,
		COMMAND_INSTALL_VERSION,
		COMMAND_EXIT
	};
	
	std::wstring command_names[] = {
		L"auto_install",	
		L"download",		
		L"get", 			
		L"unpack", 		
		L"extract", 		
		L"move", 		
		L"copy", 		
		L"makedir",		
		L"newfolder", 	
		L"ask_run", 		
		L"ask_execute", 	
		L"begin_mod",	
		L"delete",		
		L"remove",		
		L"rename",		
		L"ask_download",	
		L"ask_get",		
		L"if_version",	
		L"else",			
		L"endif",		
		L"makepbo",		
		L"extractpbo",	
		L"unpackpbo",	
		L"unpbo",		
		L"edit",			
		L"begin_ver",	
		L"alias",
		L"merge_with",
		L"filedate",
		L"install_version",
		L"exit",
		L"quit"
	};

	int match_command_name_to_id[] = {
		COMMAND_AUTO_INSTALL,
		COMMAND_DOWNLOAD,
		COMMAND_DOWNLOAD,
		COMMAND_UNPACK,
		COMMAND_UNPACK,
		COMMAND_MOVE,
		COMMAND_COPY,
		COMMAND_MAKEDIR,
		COMMAND_MAKEDIR,
		COMMAND_ASK_RUN,
		COMMAND_ASK_RUN,
		COMMAND_BEGIN_MOD,
		COMMAND_DELETE,
		COMMAND_DELETE,
		COMMAND_RENAME,
		COMMAND_ASK_DOWNLOAD,
		COMMAND_ASK_DOWNLOAD,
		COMMAND_IF_VERSION,
		COMMAND_ELSE,
		COMMAND_ENDIF,
		COMMAND_MAKEPBO,
		COMMAND_EXTRACTPBO,
		COMMAND_EXTRACTPBO,
		COMMAND_EXTRACTPBO,
		COMMAND_EDIT,
		COMMAND_BEGIN_VERSION,
		COMMAND_ALIAS,
		COMMAND_ALIAS,
		COMMAND_FILEDATE,
		COMMAND_INSTALL_VERSION,
		COMMAND_EXIT,
		COMMAND_EXIT
	};
	
	int control_flow_commands[] = {
		COMMAND_BEGIN_MOD,
		COMMAND_BEGIN_VERSION,
		COMMAND_IF_VERSION,
		COMMAND_ELSE,
		COMMAND_ENDIF,
		COMMAND_INSTALL_VERSION,
		COMMAND_EXIT
	};
	
	enum COMMAND_SWITCHES {
		SWITCH_NONE,
		SWITCH_PASSWORD       = 0x1,
		SWITCH_NO_OVERWRITE   = 0x2,
		SWITCH_MATCH_DIR      = 0x4,
		SWITCH_KEEP_SOURCE    = 0x8,
		SWITCH_INSERT         = 0x10,
		SWITCH_NEWFILE        = 0x20,
		SWITCH_APPEND         = 0x40,
		SWITCH_MATCH_DIR_ONLY = 0x80,
		SWITCH_TIMESTAMP      = 0x100,
		SWITCH_MAX            = 0x200
	};
	
	std::wstring command_switches_names[] = {
		L"",
		L"/password:",
		L"/no_overwrite",
		L"/match_dir",
		L"/keep_source",
		L"/insert",
		L"/newfile",
		L"/append",
		L"/match_dir_only",
		L"/timestamp:"
	};
	
	// Automatic filling with empty strings for a command when not enough arguments were passed
	// This is for the commands that can be used with or without arguments
	int command_minimal_arg[] = {
		0, // auto_install
		0, // download
		1, // unpack
		3, // move
		3, // copy
		1, // makedir
		1, // ask_run
		0, // begin_mod
		1, // delete
		2, // rename
		0, // ask_download
		0, // if_version
		0, // else
		0, // endif
		1, // makepbo
		2, // extractpbo
		0, // edit
		0, // begin_version
		0, // alias
		0, // filedate
		1, // install_version
		0  // exit
	};

	int number_of_commands = sizeof(command_names) / sizeof(command_names[0]);
	int number_of_switches = sizeof(command_switches_names) / sizeof(command_switches_names[0]);
	int number_of_ctrlflow = sizeof(control_flow_commands) / sizeof(control_flow_commands[0]);

	// Download installation script
	if (!global.arguments_table[L"downloadscript"].empty()) {
		WriteProgressFile(INSTALL_PROGRESS, global.lang[STR_ACTION_GETSCRIPT]);

		std::wstring url = GetFileContents(global.arguments_table[L"downloadscript"]) + L" --verbose \"--output-document=fwatch\\tmp\\installation script\"";
		int result       = Download(url, FLAG_OVERWRITE | FLAG_SILENT_MODE);

		if (result > 0) {
			LogMessage(L"", OPTION_CLOSELOG);
			return ERROR_NO_SCRIPT;
		}
	}
	
	// Open installation script
	std::wstring script_file_content;
	{
		WriteProgressFile(INSTALL_PROGRESS, global.lang[STR_ACTION_READSCRIPT]);
		std::wstring script_file_name = global.test_mode ? L"fwatch\\data\\addonInstaller_test.txt" : L"fwatch\\tmp\\installation script";
		script_file_content           = GetFileContents(script_file_name);
	
		if (script_file_content.empty()) {
			LogMessage(L"Failed to open " + script_file_name, OPTION_CLOSELOG);
			WriteProgressFile(INSTALL_ERROR, (global.lang[STR_ERROR]+L"\r\n"+global.lang[STR_ERROR_READSCRIPT]));
			return ERROR_NO_SCRIPT;
		}
	}
	
	// Table storing commands from the script
	struct {
		std::vector<int>          id;        //command enum
		std::vector<bool>         ctrl_flow; //is it a control flow command
		std::vector<int>          line_num;  //position in the text file
		std::vector<int>          step_num;  //progress number
		std::vector<int>          switches;  //bit mask
		std::vector<size_t>       arg_start; //arguments starting index in the other table
		std::vector<int>          arg_num;   //number of arguments passed to this command
		std::vector<size_t>       url_start; //urls starting index in the other table
		std::vector<int>          url_num;   //number of urls passed to this command
		std::vector<std::wstring> password;  //password switch passed to this command
		std::vector<std::wstring> arguments; //table storing arguments associated with commands (not parallel)
		std::vector<std::wstring> timestamp; //timestamp switch passed to this command
	} current_script_command;

	// Table storing download urls associated with the commands	
	struct {
		std::vector<size_t>       arg_start; //arguments starting index in the other table
		std::vector<int>          arg_num;   //number of arguments passed to this url
		std::vector<int>          line_num;  //position in the text file
		std::vector<std::wstring> link;      //actual url
		std::vector<std::wstring> arguments; //table storing url arguments associated with the urls (not parallel)
	} current_script_command_urls;

	// Parse installation script and store instructions in vectors
	{
		size_t word_begin          = 0;  //number of column where a phrase begins
		int word_count             = 1;  //number of found phrases in the current line
		int word_line_num          = 1;	 //line count for the entire script
		int word_line_num_local    = 1;  //line count for single version of the mod
		int command_id             = -1;
		int last_command_line_num  = -1;
		size_t last_url_list_id    = 0;
		bool in_quote              = false;
		bool remove_quotes         = true;
		bool url_block             = false;
		bool url_line              = false;
		bool word_started          = false;
		bool last_url_list_started = false;

		for (size_t i=0; i<=script_file_content.length(); i++) {
			bool end_of_word = i == script_file_content.length() || iswspace(script_file_content[i]);
		
			// When quote
			if (script_file_content[i] == L'"')
				in_quote = !in_quote;
		
			// If beginning of an url block
			if (script_file_content[i] == L'{' && !word_started) {
				url_block = true;
	
				// if bracket is the first thing in the line then it's auto installation
				if (word_count == 1) {
					last_command_line_num = word_line_num;
					current_script_command.id.push_back(COMMAND_AUTO_INSTALL);
					current_script_command.ctrl_flow.push_back(false);
					current_script_command.line_num.push_back(word_line_num_local);
					current_script_command.step_num.push_back(0);
					current_script_command.switches.push_back(SWITCH_NONE);
					size_t temp = current_script_command.arguments.size();  //dealing with std::_Vbase
					current_script_command.arg_start.push_back(temp);
					current_script_command.arg_num.push_back(0);				
					temp = current_script_command_urls.link.size();
					current_script_command.url_start.push_back(temp);				
					current_script_command.url_num.push_back(0);
					current_script_command.password.push_back(L"");
					current_script_command.timestamp.push_back(L"");
				} else
					// if bracket is an argument for the command
					if (command_id != -1) {
						current_script_command.arguments.push_back(L"<dl>");
						current_script_command.arg_num[current_script_command.arg_num.size()-1]++;
					}
			
				continue;
			}
		
			// If ending of an url block
			if (script_file_content[i] == L'}' && url_block) {
				end_of_word = true;
			
				// If there's space between last word and the closing bracket
				if (!word_started) {	
					url_block = false;
					url_line  = false;
					word_count++;
					continue;
				}
			}
		
			// Remember beginning of the word
			if (!end_of_word && !word_started) {
				word_begin   = i;
				word_started = true;
			
				// If custom delimeter - jump to the end of the argument
				if (script_file_content.substr(word_begin,2) == L">>") {
					word_begin   += 3;
					size_t end    = script_file_content.find(script_file_content[i+2], i+3);
					end_of_word   = true;
					i             = end==std::wstring::npos ? script_file_content.length() : end;
					remove_quotes = false;
				}
			}
		
			// When hit end of the word
			if (end_of_word && word_started && !in_quote) {
				std::wstring word = script_file_content.substr(word_begin, i-word_begin);

				if (remove_quotes)
					word = UnQuote(word);
				else
					remove_quotes = true;
				
				// If first word in the line
				if (word_count == 1 && !url_block) {
					command_id = -1;
				
					// Check if it's a valid command
					if (IsURL(word))
						command_id = COMMAND_AUTO_INSTALL;
					else
						for (int j=0; j<number_of_commands && command_id==-1; j++)
							if (Equals(word, command_names[j]))
								command_id = match_command_name_to_id[j];
				
					// If so then add it to database, otherwise skip this line
					if (command_id != -1) {
						last_command_line_num = word_line_num;
						current_script_command.id.push_back(command_id);
						current_script_command.ctrl_flow.push_back(false);
						current_script_command.line_num.push_back(word_line_num_local);
						current_script_command.step_num.push_back(0);
						current_script_command.switches.push_back(SWITCH_NONE);
						size_t temp = current_script_command.arguments.size();  //dealing with std::_Vbase
						current_script_command.arg_start.push_back(temp);
						current_script_command.arg_num.push_back(0);
						temp = current_script_command_urls.link.size();
						current_script_command.url_start.push_back(temp);
						current_script_command.url_num.push_back(0);
						current_script_command.password.push_back(L"");
						current_script_command.timestamp.push_back(L"");
					
						// Check if it's a control flow type of command
						for (int j=0; j<number_of_ctrlflow; j++)
							if (command_id == control_flow_commands[j])
								current_script_command.ctrl_flow[current_script_command.ctrl_flow.size()-1] = true;
					
						// If command is an URL then add it to the url database
						if (IsURL(word)) {
							url_line              = true;
							last_url_list_id      = current_script_command_urls.link.size();
							last_url_list_started = true;
							size_t temp2          = current_script_command_urls.arguments.size(); //dealing with std::_Vbase
							current_script_command_urls.arg_start.push_back(temp2);
							current_script_command_urls.arg_num.push_back(0);
							current_script_command_urls.line_num.push_back(word_line_num);
							current_script_command_urls.link.push_back(word);
							current_script_command.url_num[current_script_command.url_num.size()-1]++;
						}
					
						if (command_id == COMMAND_BEGIN_MOD || command_id == COMMAND_BEGIN_VERSION)
							word_line_num_local = 0;
					} else {
						size_t end = script_file_content.find(L"\n", i);
						i          = (end==std::wstring::npos ? script_file_content.length() : end) - 1;
					}
				} else {
					// Check if URL starts here
					if (!url_line && command_id != COMMAND_ASK_DOWNLOAD)
						url_line = IsURL(word);
					
					// Check if it's a valid command switch
					bool is_switch           = false;
					size_t colon             = word.find(L":");
					std::wstring switch_name = word;
					std::wstring switch_arg  = L"";
				
					if (colon != std::wstring::npos) {
						switch_name = word.substr(0,colon+1);
						switch_arg  = word.substr(colon+1);
					}
				
					for (int switch_index=1, switch_enum=1; switch_index<number_of_switches && !is_switch; switch_enum*=2, switch_index++)
						if (Equals(switch_name, command_switches_names[switch_index])) {
							is_switch   = true;
							size_t last = current_script_command.switches.size() - 1;
							current_script_command.switches[last] |= switch_enum;
						
							if (switch_enum == SWITCH_PASSWORD)
								current_script_command.password[last] = switch_arg;
							
							if (switch_enum == SWITCH_TIMESTAMP)
								current_script_command.timestamp[last] = switch_arg;
						}

					// Add word to the URL database or the arguments database
					if (!is_switch) {
						if (url_line) {
							if (!last_url_list_started) {
								last_url_list_id      = word_line_num;
								last_url_list_started = true;
								size_t temp           = current_script_command_urls.arguments.size();

								current_script_command_urls.arg_start.push_back(temp); //dealing with std::_Vbase
								current_script_command_urls.arg_num.push_back(0);
								current_script_command_urls.line_num.push_back(word_line_num);
								current_script_command_urls.link.push_back(word);
								current_script_command.url_num[current_script_command.url_num.size()-1]++;
							} else {							
								current_script_command_urls.arguments.push_back(word);
								current_script_command_urls.arg_num[current_script_command_urls.arg_num.size()-1]++;
							}
						} else {
							current_script_command.arguments.push_back(word);
							current_script_command.arg_num[current_script_command.arg_num.size()-1]++;
						}
					}
				}
			
				// If ending of an url block
				if (script_file_content[i] == L'}' && url_block) {
					url_block = false;
					url_line  = false;
				}

				word_started = false;
				word_count++;
			}
		
			// When new line
			if (!in_quote && (script_file_content[i] == L'\n' || script_file_content[i] == L'\0')) {
				size_t last = current_script_command.id.size() - 1;
			
				// Maintain minimal number of arguments
				while (!url_block && command_id != -1 && current_script_command.arg_num[last] < command_minimal_arg[current_script_command.id[last]]) {
					current_script_command.arguments.push_back(L"");
					current_script_command.arg_num[last]++;
				}
			
				word_count            = 1;
				url_line              = false;
				last_url_list_started = false;
				word_line_num++;
				word_line_num_local++;
			}
		}
	}

	// Pretend to Install
	// Figure out how many steps this installation script has so later we can display progress for the user
	for (size_t i=0;  i<current_script_command.id.size(); i++) {
		if (
			// if modfolder wasn't formally started OR skipping this mod
			((global.current_mod.empty() || global.skip_modfolder) && current_script_command.id[i] != COMMAND_BEGIN_MOD && current_script_command.id[i] != COMMAND_INSTALL_VERSION)
			||
			// if version wasn't formally started
			(!global.current_mod.empty() && global.current_mod_version.empty() && current_script_command.id[i] != COMMAND_BEGIN_VERSION && current_script_command.id[i] != COMMAND_INSTALL_VERSION)
			||
			// if inside condition block
			(global.condition_index >= 0 && !global.condition[global.condition_index] && !current_script_command.ctrl_flow[i])
		)
			continue;

		// Execute only control flow instructions
		switch(current_script_command.id[i]) {
			case COMMAND_INSTALL_VERSION : global.script_version=_wtof(current_script_command.arguments[current_script_command.arg_start[i]].c_str()); break;
			case COMMAND_BEGIN_MOD       : global.current_mod=L"?pretendtoinstall"; break;
			case COMMAND_BEGIN_VERSION   : global.current_mod_version=L"-1"; break;
			case COMMAND_IF_VERSION      : Condition_If_version(current_script_command.arguments, current_script_command.arg_start[i], current_script_command.arg_num[i]); break;
			case COMMAND_ELSE            : Condition_Else(); break;
			case COMMAND_ENDIF           : Condition_Endif(); break;
			default                      : global.installation_steps_max++;
		}

		//if (i < current_script_command.id.size())
		current_script_command.step_num[i] = global.installation_steps_max;

		if (current_script_command.id[i] == COMMAND_EXIT)
			break;
	}
	
	// Reset variables
	{
		global.current_mod         = L"";
		global.current_mod_version = L"";
		global.condition_index     = -1;
		global.condition.clear();
		global.current_mod_alias.clear();
	
		if (global.test_mode) {
			global.current_mod              = global.arguments_table[L"testmod"];
			global.current_mod_version      = L"1";
			global.current_mod_version_date = time(0);
		
			if (global.arguments_table[L"testdir"].empty())
				global.current_mod_new_name = global.arguments_table[L"testmod"];
			else
				global.current_mod_new_name = global.arguments_table[L"testdir"];
		} else
			// If wrong version
			if (global.script_version == 0 || global.installer_version < global.script_version) {
				LogMessage(L"Version mismatch. Script version: " + Float2StrW(global.script_version) + L"  Program version: " + Float2StrW(global.installer_version), OPTION_CLOSELOG);
				WriteProgressFile(INSTALL_ERROR, (global.lang[STR_ERROR_WRONG_VERSION] + L"\r\n" + Float2StrW(global.script_version) + L" vs " + Float2StrW(global.installer_version)));
				return ERROR_WRONG_SCRIPT;
			}
	}
	
	// Install for Real
	size_t instruction_index = 0;
	while (instruction_index < current_script_command.id.size()) {
		global.installation_steps_current = current_script_command.step_num[instruction_index];

		if (global.pause_installer && !global.abort_installer) {
			WriteProgressFile(INSTALL_PAUSED, L"Installation paused");

			while(global.pause_installer && !global.abort_installer)
				Sleep(200);
		}

		if (isAborted())
			return ERROR_USER_ABORTED;

		// Update global variables
		for (int j=0; j<number_of_commands; j++)
			if (current_script_command.id[instruction_index] == match_command_name_to_id[j])
				global.command_line = command_names[j];

		global.command_line_num      = current_script_command.line_num[instruction_index];
		global.download_iterator     = current_script_command.url_start[instruction_index];
		global.last_download_attempt = true;

		if (
			// if modfolder wasn't formally started OR skipping this mod
			((global.current_mod.empty() || global.skip_modfolder) && current_script_command.id[instruction_index] != COMMAND_BEGIN_MOD)
			||
			// if version wasn't formally started
			(!global.current_mod.empty() && global.current_mod_version.empty() && current_script_command.id[instruction_index] != COMMAND_BEGIN_VERSION)
			||
			// if inside condition block
			(global.condition_index >= 0 && !global.condition[global.condition_index] && !current_script_command.ctrl_flow[instruction_index])
		) {
			instruction_index++;
			continue;
		}
		
		int command_result   = ERROR_NONE;
		int failed_downloads = 0;

		// Check if there's an URL list for this command
		if (current_script_command.url_num[instruction_index] > 0) {
			Download_Phase:
			global.download_phase = true;
			int last_url          = current_script_command.url_start[instruction_index] + current_script_command.url_num[instruction_index] - 1;
			
			// For each url
			for (;  global.download_iterator<=last_url;  global.download_iterator++) {
				int j                        = global.download_iterator;
				int download_flags           = FLAG_CONTINUE | (current_script_command.id[instruction_index] == COMMAND_DOWNLOAD ? FLAG_CLEAN_DL_LATER : FLAG_CLEAN_DL_NOW);
				global.last_download_attempt = j == last_url;
				global.command_line_num      = current_script_command_urls.line_num[j];
				
				// Check how many url arguments
				if (current_script_command_urls.arg_num[j] == 0)
					command_result = Download(current_script_command_urls.link[j], download_flags);
				else 
				if (current_script_command_urls.arg_num[j] == 1)
					command_result = Download(current_script_command_urls.link[j] + L" \"--output-document=" + current_script_command_urls.arguments[current_script_command_urls.arg_start[j]] + L"\"", download_flags);
				else {
					std::wstring original_url     = current_script_command_urls.link[j];
					std::wstring cookie_file_name = L"fwatch\\tmp\\__cookies.txt";
					std::wstring token_file_name  = L"fwatch\\tmp\\__downloadtoken";
					std::wstring wget_arguments   = L"";
					std::wstring new_url          = original_url;
					std::wstring POST             = L"";
					size_t last_url_arg           = current_script_command_urls.arg_start[j] + current_script_command_urls.arg_num[j] - 1;
					bool found_phrase             = false;
				
					DeleteFile(cookie_file_name.c_str());
					DeleteFile(token_file_name.c_str());
				
					for (size_t k=current_script_command_urls.arg_start[j]; k<last_url_arg; k++) {
						wget_arguments = L"";
						
						if (!POST.empty()) {
							wget_arguments += L"--post-data=" + POST + L" ";
							POST            = L"";
						}
					
						wget_arguments += (k==current_script_command_urls.arg_start[j] ? L"--keep-session-cookies --save-cookies " : L"--load-cookies ") + cookie_file_name + L" --output-document=" + token_file_name + L" " + new_url;
						command_result  = Download(wget_arguments, FLAG_OVERWRITE, new_url);
				
						if (command_result != ERROR_NONE)
							goto Finished_downloading;
				
						// Parse downloaded file and find link
						std::wstring token_file_buffer = GetFileContents(token_file_name);
					    bool is_href                   = Equals(current_script_command_urls.arguments[k].substr(0,6),L"href=\"") || Equals(current_script_command_urls.arguments[k].substr(0,6),L"href=''");
						size_t find                    = token_file_buffer.find(current_script_command_urls.arguments[k]);
			
						if (find != std::wstring::npos) {
							size_t left_quote  = std::wstring::npos;
							size_t right_quote = std::wstring::npos;
							
							if (is_href)
								left_quote = find+5;
							else
								for (size_t m=find; m>=0 && left_quote==std::wstring::npos; m--)
									if (token_file_buffer[m]==L'\"' || token_file_buffer[m]==L'\'')
										left_quote = m;
									
							for (size_t n=find+(is_href ? 6u : 0u); n<token_file_buffer.length() && right_quote==std::wstring::npos; n++)
								if (token_file_buffer[n]==L'\"' || token_file_buffer[n]==L'\'')
									right_quote = n;
							
							if (left_quote!=std::wstring::npos && right_quote!=std::wstring::npos) {
								left_quote++;
								found_phrase           = true;
								std::wstring found_url = ReplaceAll(token_file_buffer.substr(left_quote, right_quote - left_quote), L"&amp;", L"&");
								
								// if relative address
								if (found_url[0] == L'/') {
									size_t offset      = 0;
									size_t doubleslash = original_url.find(L"//");
									
									if (doubleslash != std::wstring::npos)
										offset = doubleslash + 2;
									
									size_t slash = original_url.find_first_of(L"/", offset);
									
									if (slash != std::wstring::npos)
										original_url = original_url.substr(0, slash);
									
									found_url = original_url + found_url;
								} else
									if (!IsURL(found_url)) {
										size_t last_slash = new_url.find_last_of(L"/");
										
										if (last_slash != std::wstring::npos)
											new_url = new_url.substr(0, last_slash+1);
										
										found_url = new_url + found_url;
									}
									
								// Check if it's a form
								if (left_quote > 8 && token_file_buffer.substr(left_quote-8, 7) == L"action=") {
									size_t offset      = 0;
									std::wstring form  = GetTextBetween(token_file_buffer, L"</form>", L"<form", left_quote, true);
									std::wstring input = GetTextBetween(form, L"<input", L">", offset);
									
									while (!input.empty()) {
										std::vector<std::wstring> attributes;
										Tokenize(input, L" ", attributes);
										std::wstring name  = L"";
										std::wstring value = L"";
										
										for (size_t z=0; z<attributes.size(); z++) {
											if (attributes[z].substr(0,5) == L"name=")
												name = ReplaceAll(attributes[z].substr(5), L"\"", L"");
												
											if (attributes[z].substr(0,6) == L"value=")
												value = ReplaceAll(attributes[z].substr(6), L"\"", L"");
										}
										
										if (!name.empty()) {
											size_t replacement = token_file_buffer.find(L"input[name="+name);
											
											if (replacement != std::wstring::npos) {
												size_t new_value = token_file_buffer.find(L"'", replacement+13+name.length());
												
												if (new_value != std::wstring::npos) {
													new_value++;
													size_t end_value = token_file_buffer.find(L"'", new_value);
													
													if (end_value != std::wstring::npos)
														value = token_file_buffer.substr(new_value, end_value-new_value);
												}
											}
											
											POST += (POST.empty() ? L"" : L"&") + url_encode(name) + L"=" + url_encode(value);
										}
										
										input = GetTextBetween(form, L"<input", L">", offset);
									}
								}
								
								new_url = found_url;
							}
						}

						if (!found_phrase) {
							command_result = ErrorMessage(STR_DOWNLOAD_FIND_ERROR, L"%STR% " + current_script_command_urls.arguments[k]);
							goto Finished_downloading;
						}
							
						token_file_name += UInt2StrW(k - current_script_command_urls.arg_start[j]);
					}

					wget_arguments = L"--load-cookies " + cookie_file_name;
					
					if (!POST.empty())
						wget_arguments += L" --post-data=\"" + POST + L"\" ";
					
					wget_arguments +=  L" \"--output-document=" + current_script_command_urls.arguments[last_url_arg] + L"\" " + new_url;
					command_result  = Download(wget_arguments, download_flags, new_url);
				
					if (!global.test_mode) {
						DeleteFile(cookie_file_name.c_str());
						DeleteFile(token_file_name.c_str());
					}
				}
		
				Finished_downloading:
				if (global.pause_installer || global.abort_installer)
					break;
				else
					if (command_result == ERROR_NONE)
						break;
					else
						failed_downloads++;
			}
		}
		
		
		
		// If download was successful then execute command
		if ((
				command_result != ERROR_USER_ABORTED && 
				command_result != ERROR_PAUSED
			) && 
			(
				current_script_command.url_num[instruction_index] == 0 || 
				(
					current_script_command.url_num[instruction_index] > 0 && 
					failed_downloads < current_script_command.url_num[instruction_index]
				)
			)
		) {
			int first             = current_script_command.arg_num[instruction_index]>0 ? current_script_command.arg_start[instruction_index] : -1;
			global.download_phase = false;
			
			switch(current_script_command.id[instruction_index]) {
				case COMMAND_BEGIN_MOD : {
					if (current_script_command.arg_num[instruction_index] < 4) {
						command_result = ErrorMessage(STR_ERROR_INVALID_SCRIPT, L"%STR%", ERROR_WRONG_SCRIPT);
						break;
					}
					
					if (!global.current_mod.empty())
						EndMod();
						
					WriteProgressFile(INSTALL_PROGRESS, global.lang[STR_ACTION_PREPARING]);
				
					global.current_mod          = current_script_command.arguments[first];
					global.current_mod_id       = current_script_command.arguments[first + 1];
					global.current_mod_keepname = current_script_command.arguments[first + 2];
					global.command_line_num     = 0;
					global.current_mod_version  = L"";
					
					// Make a list of mod aliases for the entire installation
					std::vector<std::wstring> aliases;
					Tokenize(current_script_command.arguments[first + 3], L" ", aliases);
					
					for (size_t j=0; j<aliases.size(); j++)
						global.current_mod_alias.push_back(aliases[j]);
						
					global.saved_alias_array_size = global.current_mod_alias.size();
					
					
					// Find to which folder we should install the mod
					for (size_t j=0;  j<global.mod_id.size(); j++)
						if (Equals(global.current_mod_id,global.mod_id[j]))
							global.current_mod_new_name = global.mod_name[j];
				
					if (global.current_mod_new_name.empty()) {
						command_result = ErrorMessage(STR_ERROR_INVALID_ARG, L"%STR%", ERROR_WRONG_SCRIPT);
						break;
					}
				
					
					bool activate_rename = false;
					
					// Check if modfolder already exists
					DWORD dir = GetFileAttributes(global.current_mod_new_name.c_str());
				
					if (dir != INVALID_FILE_ATTRIBUTES) {
						activate_rename = true;
						
						if (dir & FILE_ATTRIBUTE_DIRECTORY) {						
							std::wstring mod_id_filename = global.current_mod_new_name + L"\\__gs_id";
							std::wstring mod_id_contents = GetFileContents(mod_id_filename);
							
							if (!mod_id_contents.empty()) {
								std::vector<std::wstring> mod_id_items;
								Tokenize(mod_id_contents, L";", mod_id_items);
								
								if (mod_id_items.size() > 0)
									activate_rename = mod_id_items[0] != global.current_mod_id;
							}
						}
					}
						
					// Rename current modfolder to make space for a new one
					if (activate_rename) {
						std::wstring rename_src = global.current_mod_new_name;
						std::wstring rename_dst = L"";
						int tries               = 1;
						int last_error          = ERROR_SUCCESS;
						
						do {
							rename_dst = global.current_mod_new_name + L"_old" + (tries>1 ? Int2StrW(tries) : L"");
							
							if (MoveFileEx(rename_src.c_str(), rename_dst.c_str(), 0))
								last_error = ERROR_SUCCESS;
							else {
								tries++;
								last_error = GetLastError();
								
								if (last_error != ERROR_ALREADY_EXISTS) {
									command_result = ErrorMessage(STR_MOVE_RENAME_ERROR, L"%STR% " + rename_src + L" " + global.lang[STR_MOVE_RENAME_TO_ERROR] + L" " + rename_dst + L" - " + Int2StrW(last_error) + L" " + FormatError(last_error));
									goto End_command_execution;
								}
							}
						} while (last_error == ERROR_ALREADY_EXISTS);
						
						LogMessage(L"Renaming existing " + rename_src + L" to " + rename_dst);
					}
				
					break;
				}

				case COMMAND_BEGIN_VERSION : {
					if (current_script_command.arg_num[instruction_index] >= 2) {
						if (!global.current_mod_version.empty())
							EndModVersion();
						
						global.current_mod_version      = current_script_command.arguments[first];
						global.current_mod_version_date = _wtoi(current_script_command.arguments[first+1].c_str());
						global.command_line_num         = 0;
					} else
						command_result = ErrorMessage(STR_ERROR_INVALID_SCRIPT, L"%STR%", ERROR_WRONG_SCRIPT);

					break;
				}
				
				case COMMAND_ALIAS : {
					if (current_script_command.arg_num[instruction_index] == 0)
						global.current_mod_alias.clear();
					else 
						for (size_t j=current_script_command.arg_start[instruction_index]; j<current_script_command.arg_start[instruction_index]+current_script_command.arg_num[instruction_index]; j++)
							global.current_mod_alias.push_back(current_script_command.arguments[j]);
					break;
				}

				case COMMAND_IF_VERSION : command_result=Condition_If_version(current_script_command.arguments, current_script_command.arg_start[instruction_index], current_script_command.arg_num[instruction_index]); break;
				case COMMAND_ELSE       : command_result=Condition_Else(); break;
				case COMMAND_ENDIF      : command_result=Condition_Endif(); break;
				case COMMAND_EXIT       : break;

				case COMMAND_AUTO_INSTALL :  {
					LogMessage(L"Auto installation"); 
					std::wstring file = global.downloaded_filename;
					
					if (current_script_command.arg_num[instruction_index] > 0)
						file = current_script_command.arguments[first];
					
					std::wstring file_with_path = L"fwatch\\tmp\\" + file;
					DWORD attributes            = GetFileAttributes(file_with_path.c_str());
					
					if (attributes != INVALID_FILE_ATTRIBUTES) {
						command_result = Auto_Install(file, attributes, FLAG_RUN_EXE, current_script_command.password[instruction_index]);
						
						// If not an archive but there are still backup links then go back to download
						if (!global.last_download_attempt && command_result == ERROR_WRONG_ARCHIVE) {
							file = L"fwatch\\tmp\\" + file;
							DeleteFile(file.c_str());
							file = L"<dl>";
							global.downloads.pop_back();
							global.download_iterator++;
							goto Download_Phase;
						}
					} else {
						DWORD error_code = GetLastError();
						command_result   = ErrorMessage(STR_AUTO_READ_ATTRI, L"%STR% " + file + L" - " + UInt2StrW(error_code) + L" " + FormatError(error_code));
					}
					
					break;
				}

				case COMMAND_UNPACK : {
					std::wstring file_name = current_script_command.arguments[first];

					if (Equals(file_name,L"<download>") || Equals(file_name,L"<dl>") || (file_name).empty())
						file_name = global.downloaded_filename;

					if (!(file_name).empty()) {
						command_result = Unpack(file_name, current_script_command.password[instruction_index]);
						
						// If not an archive but there are still backup links then go back to download
						if (!global.last_download_attempt && command_result == ERROR_WRONG_ARCHIVE) {
							file_name = L"fwatch\\tmp\\" + file_name;
							DeleteFile(file_name.c_str());
							file_name = L"<dl>";
							global.downloads.pop_back();
							global.download_iterator++;
							goto Download_Phase;
						}
					} else
						command_result = ErrorMessage(STR_ERROR_NO_FILE);
						
					break;
				}
				
				case COMMAND_MOVE :  
				case COMMAND_COPY : {
					std::wstring source      = current_script_command.arguments[first];
					std::wstring destination = current_script_command.arguments[first + 1];
					std::wstring new_name    = current_script_command.arguments[first + 2];

					if (source.empty()) {
						command_result = ErrorMessage(STR_ERROR_NO_FILE);
						break;
					}

					bool is_download_dir = true;
					int options          = FLAG_OVERWRITE | (current_script_command.id[instruction_index]==COMMAND_MOVE ? FLAG_MOVE_FILES : FLAG_NONE);

					if (current_script_command.switches[instruction_index] & SWITCH_NO_OVERWRITE)
						options &= ~FLAG_OVERWRITE;

					if (current_script_command.switches[instruction_index] & SWITCH_MATCH_DIR)
						options |= FLAG_MATCH_DIRS;
						
					if (current_script_command.switches[instruction_index] & SWITCH_MATCH_DIR_ONLY)
						options |= FLAG_MATCH_DIRS | FLAG_MATCH_DIRS_ONLY;

					for (int j=first; j<=first+2; j++)
						if (!VerifyPath(current_script_command.arguments[j])) {
							command_result = ErrorMessage(STR_ERROR_PATH);
							goto End_command_execution;
						}


					// Format source path
					if (Equals(source,L"<download>")  ||  Equals(source,L"<dl>")) {
						source = L"fwatch\\tmp\\" + global.downloaded_filename;
						
						if (options & FLAG_MOVE_FILES)
							global.downloads.pop_back();
					} else 
						if (Equals(source.substr(0,5),L"<mod>")) {
							source          = global.current_mod_new_name + source.substr(5);
							is_download_dir = false;
						} else
							if (Equals(source.substr(0,7),L"<game>\\")) {
								is_download_dir = false;
								
								if (~options & FLAG_MOVE_FILES)
									source = source.substr(7);
								else {
									command_result = ErrorMessage(STR_MOVE_DST_PATH_ERROR);
									break;
								}
							} else
								source = L"fwatch\\tmp\\_extracted\\" + source;
				
					// If user selected directory then move it along with its sub-folders
					bool source_is_dir = false;
					
					if (
						source.find(L"*") == std::wstring::npos && 
						source.find(L"?") == std::wstring::npos && 
						GetFileAttributes(source.c_str()) & FILE_ATTRIBUTE_DIRECTORY
					) {
						options      |= FLAG_MATCH_DIRS;
						source_is_dir = true;
					}
				
				
					// Format destination path
					bool destination_passed = !destination.empty();
					
					if (destination == L".")
						destination = L"";
					
					destination = global.current_mod_new_name + L"\\" + destination;
					
					if (destination.substr(destination.length()-1) != L"\\")
						destination += L"\\";
				
					// If user wants to move modfolder then change destination to the game directory
					if (is_download_dir && IsModName(PathLastItem(source)) && !destination_passed) {
						destination = L"";
						new_name    = Equals(global.current_mod,global.current_mod_new_name) ? L"" : global.current_mod_new_name;
						options     |= FLAG_MATCH_DIRS;
					} else {
						// Otherwise create missing directories in the destination path
						
						// if user wants to copy directory and give it a new name then first create a new directory with wanted name in the destination location
						if (~options & FLAG_MOVE_FILES && source_is_dir && !new_name.empty())
							command_result = MakeDir(destination + new_name);
						else
							command_result = MakeDir(PathNoLastItem(destination));
							
						if (command_result != ERROR_NONE)
							break;
					}
						
				
					// Format new name 
					// 3rd argument - new name
					if (new_name.find(L"\\") != std::wstring::npos || new_name.find(L"/") != std::wstring::npos) {
						command_result = ErrorMessage(STR_RENAME_DST_PATH_ERROR);
						break;
					}

					command_result = MoveFiles(source, destination, new_name, options);
					break;
				}
				
				case COMMAND_MAKEDIR : {
					std::wstring *path = &current_script_command.arguments[first];
					
					if (VerifyPath(*path))
						command_result = MakeDir(global.current_mod_new_name + L"\\" + *path);
					else
						command_result = ErrorMessage(STR_ERROR_PATH);
				
					break;
				}
				
				case COMMAND_DELETE : {
					WriteProgressFile(INSTALL_PROGRESS, global.lang[STR_ACTION_DELETING]+L"...");
					
					std::wstring file_name = current_script_command.arguments[first];
					int options            = FLAG_MOVE_FILES | FLAG_ALLOW_ERROR;
					
					if (current_script_command.switches[instruction_index] & SWITCH_MATCH_DIR)
						options |= FLAG_MATCH_DIRS;
						
					if (!VerifyPath(file_name)) {
						command_result = ErrorMessage(STR_ERROR_PATH);
						break;
					}
				
					// Format source path
					bool trash = false;
				
					if (Equals(file_name,L"<download>") || Equals(file_name,L"<dl>") || file_name.empty()) {
						if (global.downloaded_filename.empty()) {
							command_result = ErrorMessage(STR_ERROR_NO_FILE);
							break;
						}
					
						file_name = L"fwatch\\tmp\\" + global.downloaded_filename;
					} else {
						file_name = global.current_mod_new_name + L"\\" + file_name;
						trash     = true;
					}
				
				
					// Find files and save them to a list
					std::vector<std::wstring> source_list;
					std::vector<std::wstring> destination_list;
					std::vector<bool>         is_dir_list;
					std::vector<std::wstring> empty_dirs;
					size_t buffer_size = 1;
					int recursion      = -1;
					std::wstring destination = L"fwatch\\tmp\\_backup";
				
					command_result = CreateFileList(file_name, destination, source_list, destination_list, is_dir_list, options, empty_dirs, buffer_size, recursion);
				
					if (command_result != ERROR_NONE)
						break;
								  
					// For each file in the list
					for (size_t j=0; j<destination_list.size(); j++) {
						if (isAborted()) {
							command_result = ERROR_USER_ABORTED;
							goto End_command_execution;
						}

						LogMessage(L"Deleting  " + source_list[j]);
						
						if (trash) {
							std::wstring backup_path = destination + L"\\" + source_list[j];
							MakeDir(PathNoLastItem(backup_path), FLAG_SILENT_MODE);

							DWORD backup_attr = GetFileAttributes(backup_path.c_str());
							int backup_num    = 1;

							while (backup_attr != INVALID_FILE_ATTRIBUTES) {
								backup_path = destination + L"\\" + Int2StrW(++backup_num);
								backup_attr = GetFileAttributes(backup_path.c_str());
							}

							if (MoveFileEx(source_list[j].c_str(), backup_path.c_str(), MOVEFILE_REPLACE_EXISTING)) {
								OPERATION_LOG backup;
								backup.operation_type = OPERATION_MOVE;
								backup.source         = backup_path;
								backup.destination    = source_list[j];
								global.rollback.push_back(backup);
							} else {
								DWORD error_code = GetLastError();
								LogMessage(L"Failed to backup " + source_list[j]);
								command_result = ErrorMessage(STR_MOVE_ERROR,
									L"%STR% " + source_list[j] + L" " + global.lang[STR_MOVE_TO_ERROR] + L" " + backup_path + L" - " + Int2StrW(error_code) + L" " + FormatError(error_code)
								);
								break;
							}

						} else {
							DWORD error_code = 0;
							
							if (is_dir_list[j])
								error_code = DeleteDirectory(source_list[j]);
							else
								if (!DeleteFile(source_list[j].c_str()))
									error_code = GetLastError();

							if (error_code != 0) {
								command_result = ErrorMessage(STR_DELETE_PERMANENT_ERROR, L"%STR% " + source_list[j] + L"  - " + UInt2StrW(error_code) + L" " + FormatError(error_code));
								break;
							}
						}
					}

					break;
				}
				
				case COMMAND_RENAME : {
					WriteProgressFile(INSTALL_PROGRESS, global.lang[STR_ACTION_RENAMING]+L"...");
				
					std::wstring source      = current_script_command.arguments[first];
					std::wstring destination = current_script_command.arguments[first + 1];
					int options              = FLAG_MOVE_FILES;
					
					if (current_script_command.switches[instruction_index] & SWITCH_MATCH_DIR)
						options |= FLAG_MATCH_DIRS;
						
					for (int j=first; j<=first+1; j++)
						if (!VerifyPath(current_script_command.arguments[j])) {
							command_result = ErrorMessage(STR_ERROR_PATH);
							goto End_command_execution;
						}

				
					// Format source path
					if (source.empty()) {
						command_result = ErrorMessage(STR_RENAME_NO_NAME_ERROR, L"%STR%");
						break;
					}
					
					if (Equals(source,L"<download>")  ||  Equals(source,L"<dl>"))	{
						if (global.downloaded_filename.empty()) {
							command_result = ErrorMessage(STR_ERROR_NO_FILE);
							break;
						}
						
						source = L"fwatch\\tmp\\_extracted\\" + global.downloaded_filename;
					} else
						source = global.current_mod_new_name + L"\\" + source;
				
					std::wstring relative_path = PathNoLastItem(source);
				
					if (relative_path.find(L"*") != std::wstring::npos || relative_path.find(L"?") != std::wstring::npos) {
						command_result = ErrorMessage(STR_RENAME_WILDCARD_ERROR, L"%STR%");
						break;
					}
					
				
					// Format new name
					if (destination.empty()) {
						command_result = ErrorMessage(STR_RENAME_NO_NAME_ERROR);
						break;
					}
					
					if (destination.find(L"\\") != std::wstring::npos || destination.find(L"/") != std::wstring::npos) {
						command_result = ErrorMessage(STR_RENAME_DST_PATH_ERROR);
						break;
					}
						
							
					// Find files and save them to a list
					std::vector<std::wstring> source_list;
					std::vector<std::wstring> destination_list;
					std::vector<bool>         is_dir_list;
					std::vector<std::wstring> empty_dirs;
					size_t buffer_size = 0;
					int recursion      = -1;

					command_result = CreateFileList(source, relative_path+destination, source_list, destination_list, is_dir_list, options, empty_dirs, buffer_size, recursion);

					if (command_result != 0)
						break;

					std::wstring source_wide;
					std::wstring destination_wide;

					// For each file on the list
					for (size_t j=0;  j<source_list.size(); j++) {
						if (isAborted()) {
							command_result = ERROR_USER_ABORTED;
							break;
						}

						// Format path for logging
						LogMessage(L"Renaming  " + ReplaceAll(source_list[j], L"fwatch\\tmp\\_extracted\\", L"") + L"  to  " + PathLastItem(destination_list[j]));

						// Rename
					    if (MoveFileEx(source_list[j].c_str(), destination_list[j].c_str(), 0)) {
							OPERATION_LOG backup;
							backup.operation_type = OPERATION_MOVE;
							backup.source         = destination_list[j];
							backup.destination    = source_list[j];
							global.rollback.push_back(backup);
						} else {
							DWORD error_code = GetLastError();
							command_result = ErrorMessage(STR_MOVE_RENAME_ERROR, L"%STR% " + source_list[j] + L" " + global.lang[STR_MOVE_RENAME_TO_ERROR] + L" " + destination_list[j] + L" - " + UInt2StrW(error_code) + L" " + FormatError(error_code));
							break;
					    }
					}

					break;
				}
				
				case COMMAND_ASK_RUN : {
					std::wstring file_name = current_script_command.arguments[first];
					
					if (Equals(file_name,L"<download>") || Equals(file_name,L"<dl>") || file_name.empty())
						file_name = global.downloaded_filename;
				
					if (file_name.empty()) {
						command_result = ErrorMessage(STR_ERROR_NO_FILE);
						break;
					}
				
					if (!VerifyPath(file_name)) {
						command_result = ErrorMessage(STR_ERROR_PATH);
						break;
					}
				
					std::wstring path_to_dir = global.working_directory;
					
					if (file_name.substr(0,6) == L"<mod>\\") {
						file_name    = file_name.substr(6);
						path_to_dir += L"\\" + global.current_mod_new_name + L"\\";
					} else
						path_to_dir += L"\\fwatch\\tmp\\";
				
					command_result = RequestExecution(path_to_dir, file_name);
					break;
				}

				case COMMAND_ASK_DOWNLOAD : {
					if (current_script_command.arg_num[instruction_index] < 2) {
						command_result = ErrorMessage(STR_ERROR_ARG_COUNT);
						break;
					}
					
					std::wstring *file_name   = &current_script_command.arguments[first];
					std::wstring *url         = &current_script_command.arguments[first + 1];
					std::wstring tmp          = L"fwatch\\tmp\\schedule\\DownloadDir.txt";
					std::wstring download_dir = GetFileContents(tmp);
					bool move                 = false;
					std::fstream config;
					
					// Check if file already exists
					std::wstring path1 = download_dir + L"\\" + *file_name;
					std::wstring path2 = L"fwatch\\tmp\\" + *file_name;
					
					if (GetFileAttributes(path1.c_str()) != INVALID_FILE_ATTRIBUTES) {
						LogMessage(L"Found " + path1);
						move = true;
					} else
						if (GetFileAttributes(path2.c_str()) != INVALID_FILE_ATTRIBUTES) {
							global.downloaded_filename = *file_name;
							LogMessage(L"Found " + path2);
							global.downloads.push_back(global.downloaded_filename);
						} else {
							std::wstring message = global.lang[STR_ASK_DLOAD] + L":\r\n" + *file_name + L"\r\n\r\n" + global.lang[STR_ALTTAB];
							WriteProgressFile(INSTALL_WAITINGFORUSER, message);
							
							ShellExecute(0, 0, (*url).c_str(), 0, 0 , SW_SHOW);
							LogMessage(L"Opened " + (*url));
							
							message      = L"You must manually download\n" + *file_name + L"\n\nPress OK once download has finished\nPress CANCEL to skip installing this modfolder";
							int msgboxID = MessageBox(NULL, message.c_str(), L"Addon Installer", MB_ICONQUESTION | MB_OKCANCEL | MB_DEFBUTTON1);
							
							if (msgboxID == IDCANCEL)
								global.skip_modfolder = true;
							else {
								if (download_dir.empty()  ||  GetFileAttributes(path1.c_str()) == INVALID_FILE_ATTRIBUTES) {
									WriteProgressFile(INSTALL_WAITINGFORUSER, global.lang[STR_ASK_DLOAD_SELECT] + L"\r\n\r\n" + global.lang[STR_ALTTAB]);
									
									download_dir = BrowseFolder(L"");
									wprintf(L"%s", download_dir.c_str());
									
									std::ofstream config_out("fwatch\\tmp\\schedule\\DownloadDir.txt", std::ios::out | std::ios::trunc);
					
									if (config_out.is_open()) {
										config_out << utf8(download_dir);
										config_out.close();
									}
								}
				
								move = true;
							}
						}
						
					if (move) {
						WriteProgressFile(INSTALL_PROGRESS, global.lang[STR_ACTION_COPYINGDOWNLOAD]);
						
						std::wstring source      = download_dir + L"\\" + *file_name;
						std::wstring destination = global.working_directory + L"\\fwatch\\tmp\\" + *file_name;
						BOOL result              = MoveFileEx(source.c_str(), destination.c_str(), MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING);
						
						LogMessage(L"Moving " + source + L"  to  " + destination);
						
						if (!result) {
							DWORD error_code = GetLastError();
							command_result   = ErrorMessage(STR_MOVE_ERROR, L"%STR% " + source + L" " + global.lang[STR_MOVE_TO_ERROR] + L" " + destination + L" - " + UInt2StrW(error_code) + L" " + FormatError(error_code));
						} else {
							global.downloaded_filename = *file_name;
							global.downloads.push_back(global.downloaded_filename);
						}
					}
					
					break;
				}
										
				case COMMAND_MAKEPBO : {
					WriteProgressFile(INSTALL_PROGRESS, global.lang[STR_ACTION_PACKINGPBO]+L"...");
					
					std::wstring file_name = current_script_command.arguments[first];
										
					if (!VerifyPath(file_name)) {
						command_result = ErrorMessage(STR_ERROR_PATH);
						break;
					}			
				
					// Format source path
					if (file_name.empty()) {
						if (global.last_pbo_file.empty()) {
							command_result = ErrorMessage(STR_ERROR_NO_FILE);
							break;
						}
					
						file_name = global.last_pbo_file;
					} else
						file_name = global.current_mod_new_name + L"\\" + file_name;
				
					std::wstring pbo_name = file_name + L".pbo";

					// Make backup
					OPERATION_LOG backup;

					if (GetFileAttributes(pbo_name.c_str()) != INVALID_FILE_ATTRIBUTES) {
						std::wstring backup_path = L"fwatch\\tmp\\_backup\\" + pbo_name;
						MakeDir(PathNoLastItem(backup_path), FLAG_SILENT_MODE);

						DWORD backup_attr = GetFileAttributes(backup_path.c_str());
						int backup_num    = 1;

						while (backup_attr != INVALID_FILE_ATTRIBUTES) {
							backup_path = L"fwatch\\tmp\\_backup\\" + pbo_name + Int2StrW(++backup_num);
							backup_attr = GetFileAttributes(backup_path.c_str());
						}

						if (MoveFileEx(pbo_name.c_str(), backup_path.c_str(), MOVEFILE_REPLACE_EXISTING)) {
							backup.operation_type = OPERATION_MOVE;
							backup.source         = backup_path;
							backup.destination    = pbo_name;
							global.rollback.push_back(backup);
							backup.operation_type = OPERATION_NONE;
						} else {
							DWORD error_code = GetLastError();
							LogMessage(L"Failed to backup " + pbo_name);
							command_result = ErrorMessage(STR_MOVE_ERROR,
								L"%STR% " + pbo_name + L" " + global.lang[STR_MOVE_TO_ERROR] + L" " + backup_path + L" - " + Int2StrW(error_code) + L" " + FormatError(error_code)
							);
							break;
						}
					} else {
						backup.operation_type = OPERATION_DELETE;
						backup.source         = pbo_name;
					}

					// Create log file
					SECURITY_ATTRIBUTES sa  = {0};
				    sa.nLength              = sizeof(sa);
				    sa.lpSecurityDescriptor = NULL;
				    sa.bInheritHandle       = TRUE;       
				
				    HANDLE logFile = CreateFile(L"fwatch\\tmp\\schedule\\PBOLog.txt",
				        FILE_APPEND_DATA,
				        FILE_SHARE_WRITE | FILE_SHARE_READ,
				        &sa,
				        CREATE_ALWAYS,
				        FILE_ATTRIBUTE_NORMAL,
				        NULL );
				        
					// Execute program
					PROCESS_INFORMATION pi;
				    STARTUPINFO si; 
					ZeroMemory( &si, sizeof(si) );
					ZeroMemory( &pi, sizeof(pi) );
					si.cb 		   = sizeof(si);
					si.dwFlags 	   = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
					si.wShowWindow = SW_HIDE;
					si.hStdInput   = NULL;
					si.hStdOutput  = logFile;
					si.hStdError   = logFile;
					
					std::wstring exename    = L"MakePbo.exe";
					std::wstring executable = L"fwatch\\data\\" + exename;
					std::wstring arguments  = L" -NRK \"" + file_name + L"\"";
				
					if (CreateProcess(&executable[0], &arguments[0], NULL, NULL, true, 0, NULL, NULL, &si, &pi)) {
						if (backup.operation_type != OPERATION_NONE)
							global.rollback.push_back(backup);
						
						LogMessage(L"Creating a PBO file out of " + file_name);
						Sleep(10);

						// Wait for the program to finish its job
						DWORD exit_code = STILL_ACTIVE;
						std::string message = "";
					
						do {					
							if (isAborted()) {
								TerminateProcess(pi.hProcess, 0);
								CloseHandle(pi.hProcess);
								CloseHandle(pi.hThread);
								CloseHandle(logFile);
								command_result = ERROR_USER_ABORTED;
								goto End_command_execution;
							}
						
							ParsePBOLog(message, exename, file_name);
							GetExitCodeProcess(pi.hProcess, &exit_code);
							Sleep(100);
						} while(exit_code == STILL_ACTIVE);
					
						ParsePBOLog(message, exename, file_name);
				
						CloseHandle(pi.hProcess);
						CloseHandle(pi.hThread);
						CloseHandle(logFile);
						Sleep(1000);

						// Need to fix the pbo timestamps after makepbo
						if (exit_code == ERROR_SUCCESS) {
							std::vector<std::wstring> sourcedir_name;
							std::vector<time_t> sourcedir_time;
							command_result = CreateTimestampList(file_name, file_name.length()+1, sourcedir_name, sourcedir_time);
				
							if (command_result == ERROR_NONE) {
								FILE *f;
								errno_t f_error = _wfopen_s(&f, pbo_name.c_str(), L"rb");
								if (f_error == 0) {
									fseek(f, 0, SEEK_END);
									size_t file_size = ftell(f);
									fseek(f, 0, SEEK_SET);
								
									char *buffer = (char*) malloc(file_size+1);
								
									if (buffer != NULL) {
										memset(buffer, 0, file_size+1);
										fread(buffer, 1, file_size, f);
									
										const int name_max  = 512;
										char name[name_max] = "";
										int name_len        = 0;
										int file_count      = 0;
										size_t file_pos     = 0;
									 
										while (file_pos < file_size) {
											memset(name, 0, name_max);
											name_len = 0;
								
											for (int i=0; i<name_max-1; i++) {
												char c = buffer[file_pos++];
								
												if (c != '\0')
													name[name_len++] = c;
												else
													break;
											}
								
											unsigned long MimeType  = *((unsigned long*)&buffer[file_pos]);
											unsigned long TimeStamp = *((unsigned long*)&buffer[file_pos+12]);
											unsigned long Datasize  = *((unsigned long*)&buffer[file_pos+16]);
								
											file_pos += 20;
								
											if (name_len == 0) {
												if (file_count==0 && MimeType==0x56657273 && TimeStamp==0 && Datasize==0) {
													int value_len = 0;
													bool is_name  = true;
												
													while (file_pos < file_size) {
														if (buffer[file_pos++] != '\0')
															value_len++;
														else {
															if (is_name && value_len==0)
																break;
															else {
																is_name   = !is_name;
																value_len = 0;
															}
														}
													}
												} else
													break;
											} else {
												for (size_t i=0; i<sourcedir_name.size(); i++) {
													std::string nameA  = (std::string)name;
													std::wstring nameW = utf16(nameA);
													if (wcscmp(sourcedir_name[i].c_str(), nameW.c_str()) == 0) {
														if (sourcedir_time[i] != TimeStamp)
															memcpy(buffer+file_pos-8, &sourcedir_time[i], 4);
													
														break;
													}
												}
											}
											
											file_count++;
										}
									
										fclose(f);
										errno_t reopen       = _wfopen_s(&f, pbo_name.c_str(), L"wb");
										size_t bytes_written = 0;
									
										if (reopen == 0) {
											bytes_written = fwrite(buffer, 1, file_size, f);
											fclose(f);
										}
									
										free(buffer);
										
										if (bytes_written != file_size) {										
											command_result = ErrorMessage(STR_EDIT_WRITE_ERROR, L"%STR% " + UInt2StrW(bytes_written) + L"/" + UInt2StrW(file_size));
											break;
										}
									}
								} else {									
									const int message_size              = 128;
									wchar_t error_message[message_size] = L"";
									_wcserror_s(error_message, message_size, f_error);
									command_result = ErrorMessage(STR_EDIT_READ_ERROR, L"%STR% " + Int2StrW(f_error) + L" - " + error_message);
									break;
								}
							} else
								break;
				
							if (current_script_command.switches[instruction_index] & SWITCH_TIMESTAMP)
								command_result = ChangeFileDate(pbo_name, current_script_command.timestamp[instruction_index]);
							else
								command_result = ChangeFileDate(pbo_name, global.current_mod_version_date);

							if (command_result == ERROR_NONE && ~current_script_command.switches[instruction_index] & SWITCH_KEEP_SOURCE) {
								WriteProgressFile(INSTALL_PROGRESS, global.lang[STR_ACTION_DELETING]+L"...");
								LogMessage(L"Removing " + file_name + L" directory");
								global.last_pbo_file = L"";

								// Make backup
								{
									std::wstring backup_path = L"fwatch\\tmp\\_backup\\" + file_name;
									MakeDir(PathNoLastItem(backup_path), FLAG_SILENT_MODE);

									DWORD backup_attr = GetFileAttributes(backup_path.c_str());
									int backup_num    = 1;

									while (backup_attr != INVALID_FILE_ATTRIBUTES) {
										backup_path = L"fwatch\\tmp\\_backup\\" + file_name + Int2StrW(++backup_num);
										backup_attr = GetFileAttributes(backup_path.c_str());
									}

									if (MoveFileEx(file_name.c_str(), backup_path.c_str(), MOVEFILE_REPLACE_EXISTING)) {
										backup.operation_type = OPERATION_MOVE;
										backup.source         = backup_path;
										backup.destination    = file_name;
										global.rollback.push_back(backup);
									} else {
										DWORD error_code = GetLastError();
										LogMessage(L"Failed to backup " + file_name);
										command_result = ErrorMessage(STR_MOVE_ERROR,
											L"%STR% " + file_name + L" " + global.lang[STR_MOVE_TO_ERROR] + L" " + backup_path + L" - " + Int2StrW(error_code) + L" " + FormatError(error_code)
										);
									}
				
								}
							}
						} else						
							command_result = ErrorMessage(STR_PBO_MAKE_ERROR, L"%STR% " + UInt2StrW(exit_code) + L" - " + utf16(message));
					} else {
						DWORD error_code = GetLastError();
						command_result   = ErrorMessage(STR_ERROR_EXE, L"%STR% " + exename + L" - " + UInt2StrW(error_code) + L" " + FormatError(error_code));
					}

					break;
				}
				
				case COMMAND_EXTRACTPBO : {
					std::wstring source      = current_script_command.arguments[first];
					std::wstring destination = current_script_command.arguments[first + 1];
					
					// Verify source argument
					if (source.empty()) {
						command_result = ErrorMessage(STR_ERROR_NO_FILE);
						break;
					}
						
					if (!VerifyPath(source)) {
						command_result = ErrorMessage(STR_UNPACKPBO_SRC_PATH_ERROR);
						break;
					}
					
					if (GetFileExtension(source) != L"pbo") {
						command_result = ErrorMessage(STR_PBO_NAME_ERROR);
						break;
					}
					
					bool is_game_dir = false;
					
					if (Equals(source.substr(0,7),L"<game>\\")) {
						global.last_pbo_file = L"";
						source               = source.substr(7);
						is_game_dir          = true;
					} else {
						global.last_pbo_file = global.current_mod_new_name + L"\\" + source.substr(0, source.length()-4);
						source               = global.current_mod_new_name + L"\\" + source;
					}
				
				
					// Verify destination argument				
					if (!VerifyPath(destination)) {
						command_result = ErrorMessage(STR_UNPACKPBO_DST_PATH_ERROR);
						break;
					}
					
					// Process optional 2nd argument: extraction destination
					if (!(destination).empty() || is_game_dir) {
						if (destination == L".")
							destination = L"";
				
						command_result = MakeDir(global.current_mod_new_name + L"\\" + destination);
				
						if (command_result != ERROR_NONE)
							break;
				
						// Create path to the extracted directory for use with MakePbo function
						global.last_pbo_file = global.current_mod_new_name + L"\\" + destination + L"\\" + PathLastItem(source.substr(0, source.length()-4));
						destination          = global.working_directory    + L"\\" + global.current_mod_new_name + L"\\" + destination;
						
						if (destination.substr(destination.length()-1) != L"\\")
							destination += L"\\";
					}
				
					command_result = ExtractPBO(source, destination);
					break;
				}
				
				case COMMAND_EDIT : {
					WriteProgressFile(INSTALL_PROGRESS, global.lang[STR_ACTION_EDITING]+L"...");
				
					if (current_script_command.arg_num[instruction_index] < 3) {
						command_result = ErrorMessage(STR_ERROR_ARG_COUNT);
						break;
					}
					
					std::wstring file_name  = current_script_command.arguments[first];
					std::string wanted_text = utf8(current_script_command.arguments[first + 2]);
					size_t wanted_line      = wcstoul(current_script_command.arguments[first+1].c_str(), NULL, 10);
				
					if (file_name.empty()) {
						command_result = ErrorMessage(STR_ERROR_NO_FILE);
						break;
					}
					
					if (Equals(file_name,L"<download>") || Equals(file_name,L"<dl>")) {
						if (global.downloaded_filename.empty()) {
							command_result = ErrorMessage(STR_ERROR_NO_FILE);
							break;
						}
				
						file_name = L"fwatch\\tmp\\" + global.downloaded_filename;
					} else 
						file_name = global.current_mod_new_name + L"\\" + file_name;
				
					if (!VerifyPath(file_name)) {
						command_result = ErrorMessage(STR_ERROR_PATH);
						break;
					}

					// Make bacup
					OPERATION_LOG backup;
					backup.operation_type = OPERATION_NONE;

					{
						DWORD dest_attr          = GetFileAttributes(file_name.c_str());
						std::wstring backup_path = L"fwatch\\tmp\\_backup\\" + file_name;

						if (dest_attr != INVALID_FILE_ATTRIBUTES) {
							MakeDir(PathNoLastItem(backup_path), FLAG_SILENT_MODE);

							DWORD backup_attr = GetFileAttributes(backup_path.c_str());
							int backup_num    = 1;

							while (backup_attr != INVALID_FILE_ATTRIBUTES) {
								backup_path = L"fwatch\\tmp\\_backup\\" + file_name + Int2StrW(++backup_num);
								backup_attr = GetFileAttributes(backup_path.c_str());
							}

							if (MoveFileEx(file_name.c_str(), backup_path.c_str(), MOVEFILE_REPLACE_EXISTING)) {
								backup.operation_type = OPERATION_MOVE;
								backup.source         = backup_path;
								backup.destination    = file_name;
								global.rollback.push_back(backup);
								backup.operation_type = OPERATION_NONE;
							} else {
								DWORD error_code = GetLastError();
								LogMessage(L"Failed to backup " + file_name);
								command_result = ErrorMessage(STR_MOVE_ERROR,
									L"%STR% " + file_name + L" " + global.lang[STR_MOVE_TO_ERROR] + L" " + backup_path + L" - " + Int2StrW(error_code) + L" " + FormatError(error_code)
								);
								break;
							}
						} else {
							backup.operation_type = OPERATION_DELETE;
							backup.source         = file_name;
						}
					}
				
					std::vector<std::string> contents;
					std::fstream file;
					size_t line_number     = 0;
					bool ends_with_newline = true;
				    
				    if (~current_script_command.switches[instruction_index] & SWITCH_NEWFILE) {
				    	LogMessage(L"Editing line " + UInt2StrW(wanted_line) + L" in " + file_name);
				    	
				    	file.open(file_name.c_str(), std::ios::in);
				    	
						if (file.is_open()) {
							std::string line;
						
							while (getline(file, line)) {
								line_number++;
								
								if (file.eof())
									ends_with_newline = false;
								
								if (line_number == wanted_line) {
									std::string new_line = current_script_command.switches[instruction_index] & SWITCH_APPEND ? line+wanted_text : wanted_text;
								
									contents.push_back(new_line);
									
									if (current_script_command.switches[instruction_index] & SWITCH_INSERT) {
										contents.push_back(line);
										line_number++;
									}
								} else 
									contents.push_back(line);
							}
							
							if (current_script_command.switches[instruction_index] & SWITCH_INSERT  &&  (wanted_line==0 || wanted_line > line_number)) {
								contents.push_back(wanted_text);
								line_number++;
							}
							
							file.close();
						} else {
							command_result = ErrorMessage(STR_EDIT_READ_ERROR);
							break;
						}
					} else {
						LogMessage(L"Creating new file " + file_name);
						contents.push_back(wanted_text);
					}
				    	
				    	
				    // Write file
					std::ofstream file_new;
					file_new.open(file_name.c_str(), std::ios::out | std::ios::trunc);
					
					if (file_new.is_open()) {
						for (size_t j=0; j<contents.size(); j++) {
							file_new << contents[j];
							
							if (j+1 < line_number  ||  (j+1==line_number && ends_with_newline))
								file_new << std::endl;
						}
				
						file_new.close();
						
						if (current_script_command.switches[instruction_index] & SWITCH_TIMESTAMP)
							command_result = ChangeFileDate(file_name, current_script_command.timestamp[instruction_index]);
						else
				    		command_result = ChangeFileDate(file_name, global.current_mod_version_date);

						if (backup.operation_type != OPERATION_NONE)
							global.rollback.push_back(backup);
					} else {
						command_result = ErrorMessage(STR_EDIT_WRITE_ERROR);
						break;
					}
				
					break;
				}
				
				case COMMAND_FILEDATE : {
					if (current_script_command.arg_num[instruction_index] < 2) {
						command_result = ErrorMessage(STR_ERROR_ARG_COUNT);
						break;
					}
					
					std::wstring file_name = current_script_command.arguments[first];
					std::wstring date_text = current_script_command.arguments[first + 1];
					
					if (!VerifyPath(file_name)) {
						command_result = ErrorMessage(STR_ERROR_PATH);
						break;
					}

					file_name = global.current_mod_new_name + L"\\" + file_name;

					HANDLE file_handle;
					DWORD error_code = 0;
					file_handle      = CreateFile(file_name.c_str(), GENERIC_READ, FILE_SHARE_READ,  NULL,  OPEN_EXISTING,  FILE_ATTRIBUTE_NORMAL, NULL);

					if (file_handle != INVALID_HANDLE_VALUE) {
						FILETIME modification_time;

						if (GetFileTime(file_handle, NULL, NULL, &modification_time)) {
							command_result = ChangeFileDate(file_name, date_text);

							if (command_result == ERROR_NONE) {
								OPERATION_LOG backup;
								backup.operation_type = OPERATION_FILEDATE;
								backup.source         = file_name;
								backup.modif_time     = modification_time;
								global.rollback.push_back(backup);
							}
						} else
							error_code = GetLastError();

						CloseHandle(file_handle);
					} else
						error_code = GetLastError();

					if (error_code != 0)
						command_result = ErrorMessage(STR_EDIT_WRITE_ERROR, L"%STR% " + file_name + L" - " + UInt2StrW(error_code) + L" " + FormatError(error_code));

					break;
				}
			}
		}

		End_command_execution:

		// If command failed then ask to retry/abort
		if (command_result != ERROR_NONE && command_result != ERROR_USER_ABORTED && command_result != ERROR_PAUSED) {
			WriteProgressFile(INSTALL_RETRYORABORT, global.last_log_message + L"\r\n\r\n" + global.lang[STR_ASK_RETRYORABORT]);
			global.retry_installer = false;
			EnableMenuItem(global.window_menu, ID_PROCESS_RETRY, MF_BYCOMMAND);

			while (!global.abort_installer && !global.retry_installer)
				Sleep(200);

			EnableMenuItem(global.window_menu, ID_PROCESS_RETRY, MF_BYCOMMAND | MF_GRAYED);

			if (isAborted()) {
				command_result = ERROR_USER_ABORTED;
			} else
				if (global.retry_installer) {
					if (command_result == ERROR_WRONG_ARCHIVE) {
						std::wstring file_name = L"fwatch\\tmp\\" + global.downloaded_filename;
						DeleteFile(file_name.c_str());
						global.downloads.pop_back();
					}

					global.retry_installer = false;
				}
		}

		if (current_script_command.id[instruction_index] == COMMAND_EXIT)
			break;

		// Rollback changes
		if (command_result == ERROR_USER_ABORTED) {
			for (std::vector<OPERATION_LOG>::reverse_iterator current_op=global.rollback.rbegin(); current_op!=global.rollback.rend(); ++current_op) {
				std::wstring a = current_op->source.c_str();
				std::wstring b = current_op->destination.c_str();

				switch(current_op->operation_type) {
					case OPERATION_MOVE: {
						MakeDir(PathNoLastItem(current_op->destination).c_str(), FLAG_SILENT_MODE);
						MoveFileEx(current_op->source.c_str(), current_op->destination.c_str(), MOVEFILE_REPLACE_EXISTING);
					} break;

					case OPERATION_DELETE: {
						DeleteFile(current_op->source.c_str());
					} break;

					case OPERATION_DELETE_DIR : {
						DeleteDirectory(current_op->source.c_str());
					} break;

					case OPERATION_FILEDATE : {
						ChangeFileDate(current_op->source.c_str(), &(current_op->modif_time));
					} break;
				}
			}

			return command_result;
		}

		if (command_result == ERROR_NONE)
			instruction_index++;
	}

    // Clean up after the last mod
	if (!global.current_mod.empty())
		EndMod();

	// Finish log file
	if (global.missing_modfolders.empty()) {
		WriteProgressFile(INSTALL_DONE, global.lang[STR_ACTION_DONE]);
		SYSTEMTIME st;
		GetLocalTime(&st);
		LogMessage(
			L"All done  " + 
			Int2StrW(st.wHour, OPTION_LEADINGZERO) + L":" + 
			Int2StrW(st.wMinute, OPTION_LEADINGZERO) + L":" + 
			Int2StrW(st.wSecond, OPTION_LEADINGZERO)
		);
	} else {
		std::wstring message = ReplaceAll(global.lang[STR_ACTION_DONEWARNING], L"%MOD%", global.missing_modfolders);
		WriteProgressFile(INSTALL_WARNING, message);
		LogMessage(L"WARNING: Installation completed but modfolders " + global.missing_modfolders + L" are still missing");
		global.restart_game = false;
	}
	
	// Close listen thread
	global.end_thread = true;
	WaitForSingleObject(global.thread_receiver, INFINITE);

    return ERROR_NONE;
}