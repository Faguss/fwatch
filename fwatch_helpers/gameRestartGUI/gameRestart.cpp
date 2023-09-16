// gameRestart by Faguss (ofp-faguss.com) for Fwatch v1.16
// Program restarting Operation Flashpoint game with given arguments

#include "stdafx.h"
#include "common.h"
#include "gameRestart.h"
#include "functions.h"

#include <mstask.h>

DWORD WINAPI gameRestartMain(__in LPVOID lpParameter)
{
	UNREFERENCED_PARAMETER(lpParameter);

	enum GAME_VERSION {
		VER_UNKNOWN,
		VER_196,
		VER_199,
		VER_201
	};

	std::wstring exe_name_list[] = {
		L"armaresistance.exe",
		L"coldwarassault.exe",
		L"flashpointresistance.exe",
		L"ofp.exe",
		L"flashpointbeta.exe",
		L"operationflashpoint.exe",
		L"operationflashpointbeta.exe",
		L"armaresistance_server.exe",
		L"coldwarassault_server.exe",
		L"ofpr_server.exe"
	};
	
	std::wstring window_name_list[] = {
		L"ArmA Resistance",
		L"Cold War Assault",
		L"Operation Flashpoint",
		L"Operation Flashpoint",
		L"Operation Flashpoint",
		L"Operation Flashpoint",
		L"Operation Flashpoint",
		L"ArmA Resistance Console",
		L"Cold War Assault Console",
		L"Operation Flashpoint Console"
	};
	
	int exe_version_list[] = {
		VER_201,
		VER_199,
		VER_196,
		VER_196,
		VER_196,
		VER_196,
		VER_196,
		VER_201,
		VER_199,
		VER_196
	};
	
	int exe_num = sizeof(exe_version_list) / sizeof(exe_version_list[0]);

	INPUT_ARGUMENTS input = {
		L" ",
		L" ",
		L"",
		L"",
		L"",
		L"",
		L"",
		L"",
		L"",
		L"",
		L"",
		L"",
		L"",
		"",
		false,
		SELF_UPDATE_DISABLED,
		0,
		0
	};

	std::vector<MODLIST> local_mods;

	// Process arguments
	ProcessArguments(global.program_arguments, input);

	// Open discord/steam url
	if (input.voice_server.substr(0,19) == L"https://discord.gg/" || input.voice_server.substr(0,20) == L"https://s.team/chat/") {
		ShellExecute(NULL, L"open", input.voice_server.c_str(), NULL, NULL, SW_SHOWNORMAL);
		PostMessage(global.window, WM_CLOSE, 0, 0);
		return 0;
	}

	// Set working directory to the game root folder
	{
		wchar_t pwd[MAX_PATH];
		GetCurrentDirectory(MAX_PATH,pwd);
		global.working_directory = ReplaceAll((std::wstring)pwd, L"\\fwatch\\data", L"");
		SetCurrentDirectory(global.working_directory.c_str());
	}

	// Start logging
	{
		global.logfile.open("fwatch\\data\\gameRestartLog.txt", std::ios::out | std::ios::app | std::ios::binary);

		SYSTEMTIME st;
		GetLocalTime(&st);
		LogMessage(
			L"\r\n--------------\r\n\r\n" + 
			FormatSystemTime(st) + L"\r\n" + 
			L"Arguments: " + global.working_directory + input.user_arguments_log
		);
	}

	// If scheduled to restart at a specific time
	if (!input.event_url.empty()) {
		WINDOWS_TASK_SCHEDULER local;
		if (!WTS_OpenTask(local, input.event_task_name)) {
			PostMessage(global.window, WM_CLOSE, 0, 0);
			return 1;
		}

		std::vector<std::wstring> messages;
		messages.push_back(L"Server: " + (std::wstring)local.comment);

		SYSTEMTIME time_scheduled = {0};
		time_scheduled.wYear   = local.trigger.wBeginYear;
		time_scheduled.wMonth  = local.trigger.wBeginMonth;
		time_scheduled.wDay    = local.trigger.wBeginDay;
		time_scheduled.wHour   = local.trigger.wStartHour;
		time_scheduled.wMinute = local.trigger.wStartMinute;

		switch(local.trigger.TriggerType) {
			case TASK_TIME_TRIGGER_ONCE : {
				messages[0] += L"\r\nGame time: " + FormatSystemTime(time_scheduled, OPTION_MESSAGEBOX);
			} break;

			case TASK_TIME_TRIGGER_DAILY : {
				messages[0] += L"\r\nGame time: daily " + 
					Int2StrW(local.trigger.wStartHour, OPTION_LEADINGZERO) + 
					L":" + 
					Int2StrW(local.trigger.wStartMinute, OPTION_LEADINGZERO);
			} break;

			case TASK_TIME_TRIGGER_WEEKLY : {
				int index = -1;
				std::wstring days[] = {
					L"Sunday",
					L"Monday",
					L"Thursday",
					L"Wednesday",
					L"Thursday",
					L"Friday",
					L"Saturday"
				};

				for(int i=0, j=1; i<(sizeof(days)/sizeof(days[0])) && index<0; i++,j<<=1)
					if (local.trigger.Type.Weekly.rgfDaysOfTheWeek & j)
						index = i;

				messages[0] += L"\r\nGame time: " + days[index] + L" " +
					Int2StrW(local.trigger.wStartHour, OPTION_LEADINGZERO) + L":" + 
					Int2StrW(local.trigger.wStartMinute, OPTION_LEADINGZERO);
			} break;
		}

		TIME_ZONE_INFORMATION TimeZoneInfo;
		DWORD result = GetTimeZoneInformation(&TimeZoneInfo);
		if (result == TIME_ZONE_ID_INVALID) {
			messages.push_back(L"Failed to get time zone information: " + FormatError(GetLastError()));
			LogMessage(FormatMessageArray(messages), OPTION_CLOSELOG);
			MessageBox(NULL, FormatMessageArray(messages, OPTION_MESSAGEBOX).c_str(), L"Scheduled OFP Launch", MB_OK | MB_ICONSTOP);
			WTS_CloseTask(local);
			PostMessage(global.window, WM_CLOSE, 0, 0);
			return 1;
		}

		LONG dst = result==TIME_ZONE_ID_DAYLIGHT ? TimeZoneInfo.DaylightBias : 0;
		std::wstring download_file_name = L"fwatch\\tmp\\schedule\\" + input.event_task_name + L".bin";

		result = Download(
			L"--output-document=" + download_file_name + L" " + 
			input.event_url +
			L"&timeoffset=" + Int2StrW((TimeZoneInfo.Bias + dst) * -1)
		);
		
		if (result != ERROR_SUCCESS) {
			messages.push_back(L"Failed to download event information");
			LogMessage(FormatMessageArray(messages), OPTION_CLOSELOG);
			MessageBox(NULL, FormatMessageArray(messages, OPTION_MESSAGEBOX).c_str(), L"Scheduled OFP Launch", MB_OK | MB_ICONSTOP);
			WTS_CloseTask(local);
			PostMessage(global.window, WM_CLOSE, 0, 0);
			return 1;
		}

		enum GR_EVENT_STATUS {
			GR_OK,
			GR_INCORRECT_PARAM,
			GR_SERVER_GONE,
			GR_EVENT_GONE,
			GR_NO_DATA
		};

		char fwatch_last_update[fwatch_build_date_size] = "";
		unsigned int event_status  = GR_NO_DATA;
		int server_version         = VER_UNKNOWN;
		bool vacation_mode         = false;
		std::wstring exe_arguments = L"";
		std::vector<MODLIST> event_mods;

		SYSTEMTIME downloaded_event_start = {0};
		SYSTEMTIME downloaded_event_end   = {0};

		TASK_TRIGGER trigger_downloaded;
		ZeroMemory(&trigger_downloaded, sizeof(TASK_TRIGGER));
		trigger_downloaded.cbTriggerSize = sizeof(TASK_TRIGGER);

		FILE *f;
		errno_t open_result = _wfopen_s(&f, download_file_name.c_str(), L"rb");
		if (open_result == 0) {
			const __int64 signature_valid = 0x534750464F;
			__int64 signature_read        = 0;
			fread(&signature_read, sizeof(signature_read), 1, f);

			enum GR_TAG_NAME {
				GR_TAG_ERROR,
				GR_TAG_STATUS,
				GR_TAG_SERVER_VERSION,
				GR_TAG_EVENT_DATE_START,
				GR_TAG_EVENT_DATE_END,
				GR_TAG_EVENT_TYPE,
				GR_TAG_FWATCH_DATE,
				GR_TAG_EXE_ARGUMENTS,
				GR_TAG_MOD_INFO,
				GR_TAG_EVENT_VACATION
			};

			if (signature_read == signature_valid) {
				while(!feof(f)) {
					unsigned int tag = GR_TAG_ERROR;
					fread(&tag, sizeof(tag), 1, f);

					switch(tag) {
						case GR_TAG_STATUS        : fread(&event_status, sizeof(event_status), 1, f); break;
						case GR_TAG_SERVER_VERSION: fread(&server_version, sizeof(server_version), 1, f); break;
						case GR_TAG_EVENT_DATE_END: fread(&downloaded_event_end, sizeof(downloaded_event_end), 1, f); break;
						case GR_TAG_EXE_ARGUMENTS : readStringFromBinaryFile(f, exe_arguments); break;
						case GR_TAG_EVENT_VACATION: fread(&vacation_mode, sizeof(vacation_mode), 1, f); break;

						case GR_TAG_EVENT_DATE_START: {
							fread(&downloaded_event_start, sizeof(downloaded_event_start), 1, f);
							trigger_downloaded.wBeginYear                   = downloaded_event_start.wYear;
							trigger_downloaded.wBeginMonth                  = downloaded_event_start.wMonth;
							trigger_downloaded.wBeginDay                    = downloaded_event_start.wDay;
							trigger_downloaded.wStartHour                   = downloaded_event_start.wHour;
							trigger_downloaded.wStartMinute                 = downloaded_event_start.wMinute;
							trigger_downloaded.Type.Weekly.rgfDaysOfTheWeek = 1 << downloaded_event_start.wDayOfWeek;
						} break;

						case GR_TAG_EVENT_TYPE: {
							fread(&trigger_downloaded.TriggerType, sizeof(int), 1, f);
							switch(trigger_downloaded.TriggerType) {
								case TASK_TIME_TRIGGER_WEEKLY: trigger_downloaded.Type.Weekly.WeeksInterval = 1;break;
								case TASK_TIME_TRIGGER_DAILY: trigger_downloaded.Type.Daily.DaysInterval = 1; break;
							}
						} break;

						case GR_TAG_FWATCH_DATE: {
							size_t length = 0;
							fread(&length, sizeof(length), 1, f);
							if (length <= fwatch_build_date_size) {
								fread(&fwatch_last_update, sizeof(char), length, f);
							} else {
								fread(&fwatch_last_update, sizeof(char), (fwatch_build_date_size-1), f);
								length -= (fwatch_build_date_size-1);
								if (length <= INT_MAX) 
									fseek(f, (int)length, SEEK_CUR);
								else {
									fseek(f, INT_MAX, SEEK_CUR);
									fseek(f, (int)(length-INT_MAX), SEEK_CUR);
								}
							}
							
						} break;

						case GR_TAG_MOD_INFO: {
							size_t length = 0;
							fread(&length, sizeof(length), 1, f);
							for (size_t i=0; i<length; i++) {
								MODLIST item;
								readStringFromBinaryFile(f, item.real_name);
								readStringFromBinaryFile(f, item.id);
								readStringFromBinaryFile(f, item.version);
								fread(&item.force_name, sizeof(item.force_name), 1, f);
								event_mods.push_back(item);
							}
						} break;
					}
				}
			}

			fclose(f);
		}

		bool launch_game  = false;
		bool task_delete  = false;
		bool task_update  = false;
		bool missing_mods = false;

		if (strcmp(global.fwatch_build_date, fwatch_last_update) == 0) {
			if (event_status == GR_OK) {
				int game_version = VER_UNKNOWN;

				for (int i=0; i<exe_num; i++)
					if (Equals(exe_name_list[i],input.game_exe)) {
						game_version = exe_version_list[i];
						break;
					}

				if (server_version == game_version) {
					FILETIME time_scheduled_ft = {0};
					SystemTimeToFileTime(&time_scheduled,&time_scheduled_ft);

					FILETIME downloaded_event_start_ft = {0};
					SystemTimeToFileTime(&downloaded_event_start,&downloaded_event_start_ft);
					LONG event_start = CompareFileTime(&downloaded_event_start_ft, &time_scheduled_ft);

					enum COMPARE_FILE_TIME_RESULT {
						PAST   = -1,
						SAME   = 0,
						FUTURE = 1
					};

					if (event_start == SAME) {
						launch_game = true;
					} else {
						task_update = 
								local.trigger.TriggerType != TASK_TIME_TRIGGER_ONCE || 
								(local.trigger.TriggerType == TASK_TIME_TRIGGER_ONCE && event_start == FUTURE);

						if (event_start == PAST) {
							FILETIME downloaded_event_end_ft = {0};
							SystemTimeToFileTime(&downloaded_event_end,&downloaded_event_end_ft);
							launch_game = CompareFileTime(&downloaded_event_end_ft, &time_scheduled_ft) == FUTURE;
						}

						std::wstring description = vacation_mode ? L"Event is on pause until " : L"Event date has changed to ";
						messages.push_back(description + FormatSystemTime(downloaded_event_start, OPTION_MESSAGEBOX));
					}

					if (trigger_downloaded.TriggerType != local.trigger.TriggerType) {
						std::wstring recurrence_string[] = {
							L"single",
							L"daily",
							L"weekly"
						};

						messages.push_back(L"Event repetition has changed to " + recurrence_string[trigger_downloaded.TriggerType]);
					
						if (local.trigger.TriggerType == TASK_TIME_TRIGGER_ONCE)
							messages.push_back(L"You can renew automatic connection in the main menu");
						else
							task_update = true;
					}
				} else {
					std::wstring game_version_string[] = {
						L"?",
						L"1.96",
						L"1.99",
						L"2.01"
					};

					messages.push_back(
						L"Server has changed version (from " + 
						game_version_string[game_version] + 
						L" to " + 
						game_version_string[server_version] + L")"
					);

					task_delete = true;
				}
			} else {
				switch(event_status) {
					case GR_INCORRECT_PARAM: messages.push_back(L"Web server data is incompatible with this program"); break;
					case GR_NO_DATA        : messages.push_back(L"No valid data downloaded"); break;
					case GR_SERVER_GONE    : messages.push_back(L"Server was removed"); task_delete=true; break;
					case GR_EVENT_GONE     : messages.push_back(L"Event was removed"); task_delete=true; break;
				}
			}

			if (task_update) {
				bool saved = WTS_SaveTask(local, trigger_downloaded);
				messages.push_back(saved ? L"Task was automatically updated" : L"Failed to automatically update the task");
			}

			if (task_delete) {
				local.result = local.scheduler->Delete(local.task_name.c_str());
				messages.push_back(SUCCEEDED(local.result) ? L"Task was automatically removed" : L"Failed to automatically remove the task");
			}

			if (!task_delete && event_mods.size() > 0) {
				DWORD readmods = ReadLocalMods(local_mods);

				if (readmods == ERROR_SUCCESS) {
					std::wstring list_all      = L"";
					std::wstring list_missing  = L"";
					std::wstring list_outdated = L"";

					for(size_t i=0; i<event_mods.size(); i++) {
						if (!list_all.empty())
							list_all += L";";

						list_all += event_mods[i].real_name;

						double global_version = wcstod(event_mods[i].version.c_str(), NULL);
						double local_version = 0;
			
						for(size_t j=0; j<local_mods.size() && local_version==0; j++)
							if (event_mods[i].id == local_mods[j].id)
								local_version = wcstod(local_mods[j].version.c_str(), NULL);

						if (local_version != 0) {
							if (local_version < global_version) {
								if (!list_outdated.empty()) 
									list_outdated += L";";

								list_outdated += event_mods[i].real_name;
							}
						} else {
							if (!list_missing.empty()) 
								list_missing += L";";

							list_missing += event_mods[i].real_name;
						}
					}

					messages[0] += L"\r\nMods: " + list_all;

					if (!list_missing.empty() || !list_outdated.empty()) {
						missing_mods = true;

						if (!list_missing.empty())
							messages.push_back(L"You don't have: " + list_missing);

						if (!list_outdated.empty())
							messages.push_back(L"Outdated: " + list_outdated);
					}
				} else {
					messages.push_back(L"Failed to read modfolders " + FormatError(readmods));
					launch_game = false;
				}
			}

			LogMessage(FormatMessageArray(messages));

			UINT flags = MB_OK;

			if (launch_game) {
				flags |= MB_ICONQUESTION | MB_YESNO;
				messages.push_back(missing_mods ? L"They can be downloaded from the main menu. Launch game?" : L"Connect?");
			} else
				if (task_update)
					flags |= MB_ICONEXCLAMATION;
				else
					flags |= MB_ICONSTOP;

			int pressed = MessageBox(NULL, FormatMessageArray(messages, OPTION_MESSAGEBOX).c_str(), L"Scheduled OFP Launch", flags);

			if (pressed == IDCANCEL || pressed == IDNO)
				launch_game = false;

			WTS_CloseTask(local);

			if (!launch_game) {
				LogMessage(L"User decided not to launch", OPTION_CLOSELOG);
				PostMessage(global.window, WM_CLOSE, 0, 0);
				return 1;
			}

			if (missing_mods)
				exe_arguments = L"-nosplash";
		
			ProcessArguments(&exe_arguments[0], input);

			if (!missing_mods && !input.event_voice)
				input.voice_server.clear();
		} else {
			int pressed = MessageBox(NULL, L"New Fwatch version is required. Do you want to update? The game will be closed.", L"Scheduled OFP Launch", MB_ICONEXCLAMATION | MB_YESNO);
			if (pressed == IDNO) {
				LogMessage(L"User decided not to update", OPTION_CLOSELOG);
				PostMessage(global.window, WM_CLOSE, 0, 0);
				return 0;
			}

			input.self_update = SELF_UPDATE_AND_START_ITSELF;
		}
	}

    // Get arguments passed to Fwatch
	DWORD fwatch_pid = 0;
	bool nolaunch    = false;
	bool steam       = false;
	std::wstring fwatch_arguments(L" ");

	{
		std::ifstream fwatch_info("fwatch_info.sqf", std::ios::in | std::ios::binary);

		if (fwatch_info.is_open()) {
			std::string data_line((std::istreambuf_iterator<char>(fwatch_info)), std::istreambuf_iterator<char>());
			std::vector<std::string> data_array;
			Tokenize(data_line, "[,]\" ", data_array);
		
				for (size_t i=0; i<data_array.size(); i++) {
					if (i==0)
						fwatch_pid = atoi(data_array[i].c_str());
					else {
						std::vector<std::string> param_array;
						Tokenize(data_array[i], " ", param_array);
					
						for (size_t j=0; j<param_array.size(); j++) {
							if (Equals(param_array[j],"-nolaunch"))
								nolaunch = true;
						
							if (Equals(param_array[j],"-steam")) {
								nolaunch = true;
								steam    = true;
							}
						
							if (
								Equals(param_array[j].substr(0,9),"-gamespy=") ||
								Equals(param_array[j].substr(0,9),"-udpsoft=") ||
								Equals(param_array[j].substr(0,5),"-run=") ||
								Equals(param_array[j],"-reporthost") ||
								Equals(param_array[j],"-removenomap")
							)
								fwatch_arguments += utf16(param_array[j]) + L" ";
						}
					}
				}		
		
			fwatch_info.close();
		}
	}

	// Detect which game and executable
	std::wstring game_exe    = L"";
	std::wstring game_window = L"";
	int game_version         = VER_196;
	bool dedicated_server    = false;
	GameInfo game;
	game.handle              = 0;
	game.pid                 = 0;
	
	// Detect game
	{
		bool got_handle = false;

		for (int i=0; i<exe_num; i++)
			if (!input.game_exe.empty()) {		// if executable name is known then just get window name
				if (Equals(exe_name_list[i],input.game_exe)) {
					game_window      = window_name_list[i];
					game_version     = exe_version_list[i];
					dedicated_server = i>=7;
					break;
				}
			} else if (i<exe_num-3) {	// if executable name is not known then search running processes by window name
				game = FindProcess(input.game_pid, window_name_list[i]);
			
				if (game.pid) {
					game_exe     = exe_name_list[i];
					game_window  = window_name_list[i];
					game_version = exe_version_list[i];
					got_handle   = true;
					break;
				}
			}

		// If exe is not known or if the game's not running then check executables in the game directory
		if (game_window.empty())
			for (int i=0; i<exe_num-3; i++)
				if (GetFileAttributes(exe_name_list[i].c_str()) != INVALID_FILE_ATTRIBUTES) {
					game_exe     = exe_name_list[i];
					game_window  = window_name_list[i];
					game_version = exe_version_list[i];
					break;
				}

		if (game_window.empty()) {
			LogMessage(L"Can't find game executable", OPTION_CLOSELOG);
			return 2;
		}

		if (!got_handle)
			game = FindProcess(input.game_pid, game_window);
	}
		
	// Access game process
	std::wstring filtered_game_arguments = L" ";

	if (game.handle) {
		HANDLE game_handle = OpenProcess(PROCESS_ALL_ACCESS, 0, game.pid);
		
		// Read game parameters
		if (game.handle != 0) {
			std::vector<std::wstring> all_game_arguments;
			std::wstring module_name(L"ifc22.dll");
			
			if (dedicated_server)
				module_name = L"ijl15.dll";
			
			DWORD result = GetOFPArguments(game.pid, &game_handle, module_name, dedicated_server ? 0x4FF20 : 0x2C154, all_game_arguments);
			if (result == 0) {
				std::wstring log_arguments = L"";
				
				for (size_t i=0; i<all_game_arguments.size(); i++) {
					if (
						!Equals(all_game_arguments[i].substr(0,9),L"-connect=") &&
						!Equals(all_game_arguments[i].substr(0,6),L"-port=") &&
						!Equals(all_game_arguments[i].substr(0,10),L"-password=")
					)
						log_arguments = log_arguments + all_game_arguments[i] + L" ";	// log params except for the ones that should be hidden
					
					if (
						!Equals(all_game_arguments[i].substr(0,5),L"-mod=") &&
						!Equals(all_game_arguments[i].substr(0,9),L"-connect=") &&
						!Equals(all_game_arguments[i].substr(0,6),L"-port=") &&
						!Equals(all_game_arguments[i].substr(0,10),L"-password=")
					)
						filtered_game_arguments += all_game_arguments[i] + L" ";
				}
				
				LogMessage(L"Game arguments: " + log_arguments);
			} else {
				if (result == ERROR_MOD_NOT_FOUND)
					LogMessage(L"Couldn't find " + module_name);
				else
					LogMessage(L"Can't get module list" + FormatError(GetLastError()));
			}
			
			CloseHandle(game_handle);
		} else {
			LogMessage(L"Can't access game process" + FormatError(GetLastError()), OPTION_CLOSELOG);
			return 3;
		}
				
		// Shutdown the game
		BOOL close = PostMessage(game.handle, WM_CLOSE, 0, 0);
		int tries  = 0;
			
		do {
			game = FindProcess(game.pid, game_window);
			Sleep(500);
		} while (close && game.pid!=0 && ++tries<7);
			
		if (game.pid) {
			game_handle = OpenProcess(PROCESS_ALL_ACCESS, 0, game.pid);
			
			if (TerminateProcess(game_handle, 0)) 
				LogMessage(L"Game terminated");
			else {
				LogMessage(L"Couldn't terminate the game" + FormatError(GetLastError()), OPTION_CLOSELOG);
				CloseHandle(game_handle);
				return 4;
			}

			CloseHandle(game_handle);
		} else
			LogMessage(L"Game closed");
	} else
		LogMessage(L"Can't find the game window");

	// Wait for Fwatch termination
	{
		int tries = 0;
		bool fwatch_was_launched = false;
		while (!nolaunch && GetProcessID(L"fwatch.exe")) {
			fwatch_was_launched = true;
			Sleep(100);

			if (++tries > 50) {
				LogMessage(L"Fwatch didn't quit", OPTION_CLOSELOG);
				return 5;
			}
		}
	
		if (nolaunch) {
			if (steam)
				LogMessage(L"Fwatch -steam");
			else
				LogMessage(L"Fwatch -nolaunch");
		} else 
			if (fwatch_was_launched)
				LogMessage(L"Waited for Fwatch to quit");
	}

	// Create a pbo file if ordered
	if (input.PBOaddon!=L"" && input.PBOaddon.find(L"..\\")==std::wstring::npos) {
        std::wstring PBOexec = global.working_directory + L"\\fwatch\\data\\MakePbo.exe";
		std::vector<std::wstring> PBOarg;
		
		PBOarg.push_back(global.working_directory + L"\\@AddonTest\\ -NRK @AddonTest\\addons\\" + input.PBOaddon);
		PBOarg.push_back(global.working_directory + L"\\@AddonTest\\ -NRK @AddonTest\\Campaigns\\AddonTest");
		
        for (size_t i=0; i<PBOarg.size(); i++) {
			LogMessage(PBOexec + L"\r\n" + PBOarg[i]);

			PROCESS_INFORMATION pi;
			STARTUPINFO si; 
			ZeroMemory(&si, sizeof(si));
			ZeroMemory(&pi, sizeof(pi));
			si.cb 			= sizeof(si);
			si.dwFlags 	= STARTF_USESHOWWINDOW;
			si.wShowWindow = SW_HIDE;
			
			if (!CreateProcess(&PBOexec[0], &PBOarg[i][0], NULL, NULL, false, 0, NULL, NULL, &si, &pi))
				LogMessage(L"MakePBO failure" + FormatError(GetLastError()));

			DWORD st;
			do {					
				GetExitCodeProcess(pi.hProcess, &st);
				Sleep(100);
			} while (st == STILL_ACTIVE);

			LogMessage(L"MakePBO result: " + Int2StrW(st));
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
		}
	}

	// Rename modfolders if necessary
	if (input.mods.size() > 0) {

		// If mod identification numbers were passed then find which folders have these signatures
		if (!input.mods[0].id.empty()) {
			DWORD readmods = ReadLocalMods(local_mods);

			if (readmods != ERROR_SUCCESS) {
				LogMessage(L"Failed to read modfolders " + FormatError(readmods), OPTION_CLOSELOG);
				return 6;
			}

			bool add_param_name = true;

			for (size_t i=0; i<input.mods.size(); i++) {        // for each required mod
				for (size_t j=0; j<local_mods.size(); j++) {    // find matching id within a list of existing modfolders
					if (input.mods[i].id == local_mods[j].id) {
						if ((input.server_equalmodreq || local_mods[j].force_name)  &&  !Equals(local_mods[j].folder_name,local_mods[j].real_name)) {    // if there's a name conflict...
							if (RenameWithBackup(local_mods[j].folder_name, local_mods[j].real_name)) {    // ...then rename to something else
								local_mods[j].folder_name = local_mods[j].real_name;
							} else {
								global.logfile.close();
								return 7;
							}
						}

						if (add_param_name) {
							input.user_arguments     += L"-mod=";
							input.user_arguments_log += L"-mod=";
							add_param_name            = false;
						} else {
							input.user_arguments     += L";";
							input.user_arguments_log += L";";
						}
					
						input.user_arguments     += local_mods[j].folder_name;
						input.user_arguments_log += local_mods[j].folder_name;
					}
				}
			}
		} else {
			// Check if signatures within selected folders require forcing mod name
			for (size_t i=0; i<input.mods.size(); i++) {
				std::vector<std::wstring> id_file_array = ReadModID(input.mods[i].folder_name);

				if (id_file_array.size() >= MOD_SIZE) {
					bool force_name = Equals(id_file_array[MOD_FORCENAME],L"true") || Equals(id_file_array[MOD_FORCENAME],L"1");

					if (force_name  &&  !Equals(input.mods[i].folder_name,id_file_array[MOD_NAME])) {
						if (RenameWithBackup(input.mods[i].folder_name, id_file_array[MOD_NAME])) {
							input.mods[i].folder_name = id_file_array[MOD_NAME];
						} else {
							global.logfile.close();
							return 7;
						}
					}
				}

				input.user_arguments     += (i==0 ? L"-mod=" : L";") + input.mods[i].folder_name;
				input.user_arguments_log += (i==0 ? L"-mod=" : L";") + input.mods[i].folder_name;
			}
		}
		
		input.user_arguments     += L" ";
		input.user_arguments_log += L" ";
	}

	// Get user's custom launch parameters for this server
	if (!input.server_uniqueid.empty()) {
		std::string params = ReadStartupParams(utf8(input.server_uniqueid));
		
		if (input.server_equalmodreq) {
			std::vector<std::string> params_array;
			Tokenize(params, " ", params_array);
			
			for (size_t i=0; i<params_array.size(); i++) {
				if (Equals(params_array[i].substr(0,5),"-mod="))
					continue;
					
				input.user_arguments     += utf16(params_array[i]) + L" ";
				input.user_arguments_log += utf16(params_array[i]) + L" ";
			}
		} else {
			input.user_arguments     += utf16(params) + L" ";
			input.user_arguments_log += utf16(params) + L" ";
		}
	}

	// Self-update
	if (input.self_update != SELF_UPDATE_DISABLED) {
		wchar_t url[]           = L"http://ofp-faguss.com/fwatch/116test";
		std::wstring error_text = L"";
		DWORD result            = 0;
		
		if (nolaunch) {
			if (!fwatch_pid)
				fwatch_pid = GetProcessID(L"fwatch.exe");
			
			HANDLE fwatch_handle = OpenProcess(PROCESS_ALL_ACCESS, 0, fwatch_pid);
			
			if (!fwatch_handle) {
				std::wstring recommendation = L"You have to download and update manually";

				if (GetLastError() == ERROR_ACCESS_DENIED)
					recommendation = L"Set gameRestart.exe to run as admin and try again";

				LogMessage(
					L"Failed to access Fwatch process " + 
					Int2StrW(GetLastError()) + 
					FormatError(GetLastError()) + 
					recommendation
				);

				MessageBox(NULL, recommendation.c_str(), L"Fwatch self-update ERROR", MB_OK | MB_ICONSTOP);
				
				if (GetLastError() == ERROR_ACCESS_DENIED)
					SelectFileInExplorer(L"\\fwatch\\data\\gameRestart.exe");
				else
					ShellExecute(NULL, L"open", url, NULL, NULL, SW_SHOWNORMAL);
				
				global.logfile.close();
				return 1;
			}
			
			BOOL terminated = TerminateProcess(fwatch_handle, 0);
			CloseHandle(fwatch_handle);
			
			if (!terminated) {
				LogMessage(
					L"Failed to close Fwatch " + 
					Int2StrW(GetLastError()) + 
					FormatError(GetLastError()) + 
					L"You have to download and update manually",
					OPTION_CLOSELOG
				);
				MessageBox(NULL, L"You have to download and update manually", L"Fwatch self-update ERROR", MB_OK | MB_ICONSTOP);
				ShellExecute(NULL, L"open", url, NULL, NULL, SW_SHOWNORMAL);
				return 1;
			}
		}
			
		Sleep(1000);

		DeleteFile(L"fwatch\\data\\libeay32.dll");
		DeleteFile(L"fwatch\\data\\libiconv2.dll");
		DeleteFile(L"fwatch\\data\\libintl3.dll");
		DeleteFile(L"fwatch\\data\\libssl32.dll");
		DeleteFile(L"fwatch\\data\\sortMissions.exe");

		std::wstring file_name = L"fwatch\\tmp\\schedule\\schedule.bin";
		if (!DeleteFile(file_name.c_str()))
			LogMessage(L"Failed to delete " + file_name + FormatError(GetLastError()));
		
		std::wstring rename_src = L"fwatch\\data\\gameRestart.exe";
		std::wstring rename_dst = L"fwatch\\data\\gameRestart_old.exe";
		DeleteFile(rename_dst.c_str());
		if (!MoveFileEx(rename_src.c_str(), rename_dst.c_str(), MOVEFILE_REPLACE_EXISTING)) {
			LogMessage(L"Failed to rename " + rename_src + L" to " + rename_dst + FormatError(GetLastError()) + L"You have to download and update manually", OPTION_CLOSELOG);
			MessageBox(NULL, L"You have to download and update manually", L"Fwatch self-update ERROR", MB_OK | MB_ICONSTOP);
			ShellExecute(NULL, L"open", url, NULL, NULL, SW_SHOWNORMAL);
			return GetLastError();
		}
		
		std::vector<std::wstring> download_mirrors;
		download_mirrors.push_back(L"http://ofp-faguss.com/fwatch/download/fwatch_self_update.7z");
		download_mirrors.push_back(L"http://faguss.paradoxstudio.uk/fwatch/download/fwatch_self_update.7z");
		
		for (size_t i=0; i<download_mirrors.size(); i++) {
			result = Download(download_mirrors[i]);
			
			if (result != ERROR_SUCCESS && i == download_mirrors.size()-1) {
				LogMessage(L"Download failed\r\nYou have to download and update manually", OPTION_CLOSELOG);
				MessageBox(NULL, L"You have to download and update manually", L"Fwatch self-update ERROR", MB_OK | MB_ICONSTOP);
				ShellExecute(NULL, L"open", url, NULL, NULL, SW_SHOWNORMAL);
				return result;
			}
			
			// Delete files manually to make sure they're not being accessed
			result    = 0;
			int tries = 4;
			
			do {
				if (!DeleteFile(L"fwatch.dll")) {
					result = GetLastError();

					if (result == ERROR_FILE_NOT_FOUND)
						result = ERROR_SUCCESS;

					if (result != ERROR_SUCCESS) {
						Sleep(500);
						tries--;
					}

					LogMessage(L"Delete fwatch.dll " + FormatError(GetLastError()));
				} else {
					LogMessage(L"Delete fwatch.dll success");
				}
			} while (result != ERROR_SUCCESS && tries>=0);
			
			if (result != ERROR_SUCCESS) {
				LogMessage(L"Delete failed", OPTION_CLOSELOG);
				MessageBox(NULL, L"You have to unpack the archive manually (password is \"fwatch\")", L"Fwatch self-update ERROR", MB_OK | MB_ICONSTOP);
				SelectFileInExplorer(global.downloaded_filename);
				return result;
			}
			
			result = Unpack(global.downloaded_filename, L"fwatch");
			
			if (result == ERROR_SUCCESS)
				break;
			
			if (result != ERROR_SUCCESS && i==download_mirrors.size()-1) {
				LogMessage(L"Unpacking failed", OPTION_CLOSELOG);

				std::wstring recommendation = L"";
				bool corrupted_archive      = error_text.find(L"Can not open the file as") != std::wstring::npos;
				
				if (corrupted_archive) {
					recommendation += L"You have to download and update manually";
					DeleteFile(global.downloaded_filename.c_str());
				} else
					recommendation += L"You have to unpack the archive manually (password is \"fwatch\")";
				
				MessageBox(NULL, recommendation.c_str(), L"Fwatch self-update ERROR", MB_OK | MB_ICONSTOP);
				
				if (corrupted_archive)
					ShellExecute(NULL, L"open", url, NULL, NULL, SW_SHOWNORMAL);
				else
					SelectFileInExplorer(global.downloaded_filename);
				
				return result;			
			}
		}

		if (input.self_update == SELF_UPDATE_AND_START_ITSELF) {
			LogMessage(L"self update and start itself");
			Sleep(500);
			PROCESS_INFORMATION pi;
		    STARTUPINFO si; 
			ZeroMemory(&si, sizeof(si));
			ZeroMemory(&pi, sizeof(pi));
			si.cb 		   = sizeof(si);
			si.dwFlags 	   = STARTF_USESHOWWINDOW;
			si.wShowWindow = SW_SHOW;
			
			std::wstring launch_exe = global.working_directory + L"\\fwatch\\data\\gameRestart.exe";
			std::wstring launch_arg = L" " + global.working_directory + L" " + (std::wstring)global.program_arguments;

			BOOL run = CreateProcess(&launch_exe[0], &launch_arg[0], NULL, NULL, false, 0, NULL, NULL, &si, &pi);
			if (!run) {
				LogMessage(L"error starting: " + FormatError(GetLastError()));
			}
			CloseHandle(pi.hProcess);
		    CloseHandle(pi.hThread);
			LogMessage(L"closing " + FormatError(GetLastError()));
			PostMessage(global.window, WM_CLOSE, 0, 0);
			return 0;
		}
		
		if (steam) {
			LogMessage(L"Operation successful", OPTION_CLOSELOG);
			MessageBox(NULL, L"Update complete. Restart Fwatch", L"Fwatch self-update", MB_OK | MB_ICONINFORMATION);
			PostMessage(global.window, WM_CLOSE, 0, 0);
			return 0;
		}
				
		if (nolaunch) {
			Sleep(500);
			PROCESS_INFORMATION pi;
		    STARTUPINFO si; 
			ZeroMemory(&si, sizeof(si));
			ZeroMemory(&pi, sizeof(pi));
			si.cb 		   = sizeof(si);
			si.dwFlags 	   = STARTF_USESHOWWINDOW;
			si.wShowWindow = SW_SHOW;
		
			std::wstring launch_exe = global.working_directory + L"\\fwatch.exe";
			std::wstring launch_arg = L" " + global.working_directory + L" -nolaunch " + fwatch_arguments;
			
			CreateProcess(&launch_exe[0], &launch_arg[0], NULL, NULL, false, 0, NULL, NULL, &si, &pi);
			CloseHandle(pi.hProcess);
		    CloseHandle(pi.hThread);
		    Sleep(500);
		}
	} else 
		DeleteFile(L"fwatch\\data\\gameRestart_old.exe");
		
	// Update resource.cpp file
	if (!input.update_resource.empty()) {
		wchar_t url[]           = L"http://ofp-faguss.com/fwatch/116test";
		DWORD result            = 0;
		
		std::vector<std::wstring> download_mirrors2;
		download_mirrors2.push_back(L"http://ofp-faguss.com/fwatch/download/ofp_aspect_ratio207.7z");
		download_mirrors2.push_back(L"http://faguss.paradoxstudio.uk/fwatch/download/ofp_aspect_ratio207.7z");
		
		for (size_t i=0; i<download_mirrors2.size(); i++) {
			result = Download(download_mirrors2[i]);
			if (result == ERROR_SUCCESS)
				break;
		}
		
		if (result != ERROR_SUCCESS) {
			MessageBox(NULL, L"You have to download and update manually", L"Resource update", MB_OK | MB_ICONSTOP);
			ShellExecute(NULL, L"open", url, NULL, NULL, SW_SHOWNORMAL);
			return result;
		}
				
		result = Unpack(global.downloaded_filename, L"", true);
		DeleteFile(global.downloaded_filename.c_str());
		
		if (result != ERROR_SUCCESS) {
			MessageBox(NULL, L"You have to download and update manually", L"Resource update", MB_OK | MB_ICONSTOP);
			ShellExecute(NULL, L"open", url, NULL, NULL, SW_SHOWNORMAL);
			return result;
		}
		
		std::wstring source      = L"fwatch\\tmp\\_extracted\\Files\\";
		std::wstring destination = L"bin\\";
		
		if (input.update_resource == L"1.96") {
			source     += L"OFP Resistance 1.96";
			destination = L"Res\\bin\\";
		}
			
		if (input.update_resource == L"1.99")
			source += L"ArmA Cold War Assault 1.99";
			
		if (input.update_resource == L"2.01")
			source += L"ArmA Resistance 2.01";

		std::wstring destination_folder = destination;
		source                         += L"\\Resource.cpp";
		destination                    += L"Resource.cpp";
		std::wstring destination_backup = L"";
		
		if (!RenameWithBackup(source, destination)) {
			LogMessage(L"Rename failed", OPTION_CLOSELOG);
			MessageBox(NULL, L"You have to download and update manually", L"Resource update", MB_OK | MB_ICONSTOP);
			ShellExecute(NULL, L"open", url, NULL, NULL, SW_SHOWNORMAL);
			return 7;
		}
		
		if (steam) {
			MessageBox(NULL, L"Update complete. Start Fwatch again", L"Fwatch self-update", MB_OK | MB_ICONINFORMATION);
			LogMessage(L"Operation successful", OPTION_CLOSELOG);
			PostMessage(global.window, WM_CLOSE, 0, 0);
			return 0;
		}
				
		if (nolaunch) {
			Sleep(500);
			PROCESS_INFORMATION pi;
		    STARTUPINFO si; 
			ZeroMemory(&si, sizeof(si));
			ZeroMemory(&pi, sizeof(pi));
			si.cb 		   = sizeof(si);
			si.dwFlags 	   = STARTF_USESHOWWINDOW;
			si.wShowWindow = SW_SHOW;

			std::wstring launch_exe = global.working_directory + L"\\fwatch.exe";
			std::wstring launch_arg = L" " + global.working_directory + L" -nolaunch " + fwatch_arguments;
			
			CreateProcess(&launch_exe[0], &launch_arg[0], NULL, NULL, false, 0, NULL, NULL, &si, &pi);
			CloseHandle(pi.hProcess);
		    CloseHandle(pi.hThread);
		    Sleep(500);
		}
	}

	// Check for face textures in the modfolders
	{
		std::vector<std::wstring> mod_list;
		std::vector<std::wstring> arguments_split;
		Tokenize(input.user_arguments, L" ", arguments_split);
		std::wstring player_name = L"";
		
		// Get list of selected mods
		for (size_t i=0; i<arguments_split.size(); i++) {
			std::wstring namevalue = arguments_split[i];
			size_t separator       = namevalue.find_first_of(L'=');
		
			if (separator != std::wstring::npos) {
				std::wstring name  = namevalue.substr(0,separator);
				std::wstring value = namevalue.substr(separator+1);
			
				if (Equals(name,L"-mod")) {
					std::vector<std::wstring> temp_array;
					Tokenize(value, L";", temp_array);
				
					for (size_t j=0; j<temp_array.size(); j++)
						mod_list.push_back(temp_array[j]);
				}
			
				if (Equals(name,L"-name")) {
					player_name = value;
				}
			}
		}
	
		// Before launching the game do the face texture replacement
		if (mod_list.size() > 0) {
			std::wstring mod_face;
			std::wstring user_face;
			std::wstring user_face_backup;
			std::wstring changelog;

			enum FACE_TYPES {
				NONE,
				PAA,
				JPG
			};

			size_t face_type               = NONE;
			std::wstring face_extensions[] = {L"paa", L"jpg"};
			size_t face_extensions_length  = sizeof(face_extensions) / sizeof(face_extensions[0]);

			// Find mod with custom face file starting from the last
			for (std::vector<std::wstring>::reverse_iterator current_mod=mod_list.rbegin(); current_mod!=mod_list.rend(); ++current_mod) {
				for (size_t j=0; j<face_extensions_length; j++) {
					mod_face = *current_mod + L"\\face." + face_extensions[j];

					if (GetFileAttributes(mod_face.c_str()) != INVALID_FILE_ATTRIBUTES) {
						face_type = j + 1;

						// Need player name from the registry
						if (player_name.empty()) {
							HKEY key_handle     = 0;
							wchar_t value[1024] = L"";
							DWORD value_size    = sizeof(value);
							LONG result         = RegOpenKeyEx(HKEY_CURRENT_USER, game_version==VER_199 ? L"SOFTWARE\\Bohemia Interactive Studio\\ColdWarAssault" : L"SOFTWARE\\Codemasters\\Operation Flashpoint", 0, KEY_READ, &key_handle);

							if (result == ERROR_SUCCESS) {
								DWORD data_type = REG_SZ;

								if (RegQueryValueEx(key_handle, L"Player Name", 0, &data_type, (BYTE*)value, &value_size) == ERROR_SUCCESS)
									player_name = (std::wstring)value;
							}

							RegCloseKey(key_handle);

							if (player_name.length() == 0)
								break;
						}

						// Check if backup already exists
						bool backup_exists = false;
						for (size_t k=0; k<face_extensions_length && !backup_exists; k++) {
							user_face = L"Users\\" + player_name + L"\\face." + face_extensions[k] + L"backup";

							if (GetFileAttributes(user_face.c_str()) != INVALID_FILE_ATTRIBUTES)
								backup_exists = true;
						}

						if (backup_exists)
							break;

						// Backup current face
						for (size_t k=0; k<face_extensions_length; k++) {
							user_face        = L"Users\\" + player_name + L"\\face." + face_extensions[k];
							user_face_backup = user_face + L"backup";

							if (MoveFileEx(user_face.c_str(), user_face_backup.c_str(), 0)) {
								changelog    += user_face + L"?" + user_face_backup + L"\r\n";
								backup_exists = true;
							}
						}

						if (!backup_exists)
							break;

						// Move face from the modfolder
						user_face = L"Users\\" + player_name + L"\\face." + face_extensions[j];

						std::ofstream out;
						out.open("fwatch\\data\\user_rename.txt", std::ios::out | std::ios::app);

						if (MoveFileEx(mod_face.c_str(), user_face.c_str(), 0))
							out << utf8(mod_face) << "?" << utf8(user_face) << "\r\n";

						out << utf8(changelog);
						out.close();
						break;
					}
				}
			}
		}
	}
	
	// Check if custom files are above size limit
	if (!input.maxcustom.empty() && !input.username.empty()) {
		std::wstring changelog   = L"";
		std::wstring path_to_cfg = L"Users\\" + input.username + L"\\UserInfo.cfg";
		
		std::ifstream config(path_to_cfg.c_str(), std::ios::in | std::ios::binary);
		std::string config_buffer((std::istreambuf_iterator<char>(config)), std::istreambuf_iterator<char>());

		if (config.is_open())
			config.close();

		std::string face_property = "face=\"Custom\"";
		size_t face_property_pos  = config_buffer.find(face_property);
		bool rewrite              = false;
		double size_limit         = wcstod(input.maxcustom.c_str(), NULL);
		HANDLE hFind;
		WIN32_FIND_DATA fd;
		
		// Get custom face size
		if (face_property_pos != std::string::npos) {
			std::wstring path_to_face = L"Users\\" + input.username + L"\\face.paa";
			double face_size          = 0;

			hFind = FindFirstFile(path_to_face.c_str(), &fd);
			if (hFind != INVALID_HANDLE_VALUE) {
				face_size = fd.nFileSizeLow;
				FindClose(hFind);
			} else {
				path_to_face = L"Users\\" + input.username + L"\\face.jpg";
				hFind        = FindFirstFile(path_to_face.c_str(), &fd);
				
				if (hFind != INVALID_HANDLE_VALUE) {
					face_size = fd.nFileSizeLow;
					FindClose(hFind);
				}
			}
			
			if (face_size > size_limit && face_size <= 102400) {
				config_buffer.replace(face_property_pos, face_property.length(), "face=\"Face52\"");
				rewrite   = true;
			}			
		}
		
		if (rewrite) {
			std::ofstream config_new(path_to_cfg.c_str(), std::ios::out | std::ios::trunc | std::ios::binary);
			
			if (config_new.is_open()) {
				changelog = path_to_cfg + L"\r\n";
				config_new << config_buffer;
				LogMessage(L"Changed custom face to a default one due to server file limit");
				config_new.close();
			} else {
				wchar_t error_text[256] = L"";
				_wcserror_s(error_text, 256, errno);
				LogMessage(L"Failed to modify config " + Int2StrW(errno) + L" - " + error_text);
			}
		}
			
		// Get custom sound size
		std::wstring path_to_sound = L"Users\\" + input.username + L"\\sound\\*.*";
		hFind                      = FindFirstFile(path_to_sound.c_str(), &fd);
		
		if (hFind != INVALID_HANDLE_VALUE) {
			do {
				if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					continue;

				// If above limit then move it out of the folder
				if (fd.nFileSizeLow > size_limit && fd.nFileSizeLow <= 51200) {
					std::wstring rename_src  = L"Users\\" + input.username + L"\\sound\\" + (std::wstring)fd.cFileName;
					std::wstring rename_base = L"Users\\" + input.username + L"\\" + (std::wstring)fd.cFileName;
					std::wstring rename_dst  = L"";
					int tries                = 1;
					DWORD last_error         = ERROR_SUCCESS;
					
					do {
						std::string number_suffix = tries>1 ? Int2Str(tries) : "";
						rename_dst = rename_base + utf16(number_suffix);

						if (MoveFileEx(rename_src.c_str(), rename_dst.c_str(), 0)) {
							last_error = ERROR_SUCCESS;
							changelog += rename_src + L"?" + rename_dst + L"\r\n";
						} else {
							tries++;
							last_error = GetLastError();
							if (last_error != ERROR_ALREADY_EXISTS) {
								LogMessage(L"Failed to rename " + rename_src + L" to " + rename_dst + L" - " + FormatError(last_error), OPTION_CLOSELOG);
								return 7;
							}
						}
					} while (last_error == ERROR_ALREADY_EXISTS);
					
					LogMessage(L"Renamed " + rename_src + L" to " + rename_dst);
				}
			} while (FindNextFile(hFind, &fd) != 0);
			
			FindClose(hFind);
		}
		
		if (!changelog.empty()) {
			std::ofstream out("fwatch\\data\\user_rename.txt", std::ios::out | std::ios::app);
			if (out.is_open()) {
				out << utf8(changelog);
				out.close();
			}
		}
	}

	// Start a new game instance
	{
		std::wstring launch_exe     = global.working_directory + L"\\" + (nolaunch ? game_exe : L"fwatch.exe");
		std::wstring launch_arg     = L" \"" + global.working_directory + L"\" ";
		std::wstring launch_arg_log = L"";

		launch_arg     += filtered_game_arguments + input.user_arguments + L" " + (nolaunch ? L"" : fwatch_arguments);
		launch_arg_log += filtered_game_arguments + input.user_arguments_log + L" " + (nolaunch ? L"" : fwatch_arguments);
	
		// Sort arguments to deal with the OFP arguments bug (should be -nosplash -x=1280 and not -x=1280 -nosplash)
		std::vector<std::wstring> temp_array;
		Tokenize(launch_arg, L" ", temp_array);
		std::wstring with_equality    = L"";
		std::wstring without_equality = L"";

		for (size_t i=0; i<temp_array.size(); i++)
			if (temp_array[i].find_first_of(L"=") != std::wstring::npos)
				with_equality += temp_array[i] + L" ";
			else
				without_equality += temp_array[i] + L" ";
	
		launch_arg = without_equality + with_equality;
	
		temp_array.clear();
		Tokenize(launch_arg_log, L" ", temp_array);
		with_equality    = L"";
		without_equality = L"";

		for (size_t i=0; i<temp_array.size(); i++)
			if (temp_array[i].find_first_of(L"=") != std::wstring::npos)
				with_equality += temp_array[i] + L" ";
			else
				without_equality += temp_array[i] + L" ";
	
		launch_arg_log = without_equality + with_equality;
	
		if (steam) {
			HKEY hKey			        = 0;
			wchar_t SteamPath[MAX_PATH] = {0};
			wchar_t SteamExe[MAX_PATH]  = {0};
			DWORD dwType		        = 0;
			DWORD SteamPathSize	        = sizeof(SteamPath);
			DWORD SteamExeSize	        = sizeof(SteamExe);

			if (RegOpenKey(HKEY_CURRENT_USER,L"Software\\Valve\\Steam",&hKey) == ERROR_SUCCESS) {
				dwType    = REG_SZ;
				bool key1 = RegQueryValueEx(hKey, L"SteamPath", 0, &dwType, (BYTE*)SteamPath, &SteamPathSize) == ERROR_SUCCESS;
				bool key2 = RegQueryValueEx(hKey, L"SteamExe",  0, &dwType, (BYTE*)SteamExe,  &SteamExeSize)  == ERROR_SUCCESS;

				if (key1 && key2) {
					launch_exe = (std::wstring)SteamExe;
					launch_arg = L" -applaunch 65790 ";
				
					if (filtered_game_arguments.find(L"-nomap")==std::wstring::npos && input.user_arguments.find(L"-nomap")==std::wstring::npos)
						launch_arg += L" -nomap ";
					
					launch_arg += filtered_game_arguments + input.user_arguments;
				} else {
					LogMessage(L"Couldn't read registry key" + FormatError(GetLastError()));
				}
			} else {
				LogMessage(L"Couldn't open registry" + FormatError(GetLastError()));
			}

			RegCloseKey(hKey);
		}

		LogMessage(launch_exe + L"\r\n" + launch_arg_log);
	
		PROCESS_INFORMATION pi;
		STARTUPINFO si; 
		ZeroMemory( &si, sizeof(si) );
		ZeroMemory( &pi, sizeof(pi) );
		si.cb 		   = sizeof(si);
		si.dwFlags 	   = STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_SHOW;

		if (!CreateProcess(&launch_exe[0], &launch_arg[0], NULL, NULL, false, 0, NULL, NULL, &si, &pi)) {
			LogMessage(L"Couldn't start the game" + FormatError(GetLastError()), OPTION_CLOSELOG);
			MessageBox(NULL, L"Failed to launch the game.\r\nPlease refer to the fwatch\\data\\gameRestartLog.txt.", L"gameRestart", MB_OK|MB_ICONERROR );
			return 8;
		}
	
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread); 

		LogMessage(L"Operation successful", OPTION_CLOSELOG);
	
		if (!input.voice_server.empty())
			ShellExecute(NULL, L"open", input.voice_server.c_str(), NULL, NULL, SW_SHOWNOACTIVATE);
	}
	
	global.logfile.close();
	PostMessage(global.window, WM_CLOSE, 0, 0);
	return 0;
}
