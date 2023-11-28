#include "stdafx.h"
#include "resource.h"
#include "common.h"
#include "functions.h"
#include "installer.h"

DWORD WINAPI addonInstallerWrapper(__in LPVOID lpParameter)
{
	UNREFERENCED_PARAMETER(lpParameter);
	DWORD threadID1 = 0;
	DWORD threadID2 = 0;

	global.thread_installer = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)addonInstallerMain, 0, 0,&threadID2);

	if (global.thread_installer) {
		if (!global.test_mode)
			global.thread_receiver = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ReceiveInstructions, 0, 0,&threadID1);
		WaitForSingleObject(global.thread_installer, INFINITE);
	}

	DeleteDirectory(L"fwatch\\tmp\\_backup");
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

	LogMessage(L"", CLOSE_LOG);

	DWORD thread_return = 0;
	if (global.thread_installer != 0)
		GetExitCodeThread(global.thread_installer, &thread_return);

	if (global.restart_game || (thread_return==ERROR_USER_ABORTED && global.test_mode))
		SendMessage(global.window, WM_CLOSE, 0, 0);

	return 0;
}

DWORD WINAPI addonInstallerMain(__in LPVOID lpParameter)
{
	UNREFERENCED_PARAMETER(lpParameter);

	WriteProgressFile(INSTALL_PROGRESS, global.lang[STR_ACTION_INIT]);

	// Create log file
	{
		global.logfile.open("fwatch\\data\\addonInstallerLog.txt", std::ios::out | std::ios::app | std::ios::binary);

		if (!global.logfile.is_open()) {
			WriteProgressFile(INSTALL_ERROR, (global.lang[STR_ERROR]+L"\r\n"+global.lang[STR_ERROR_LOGFILE]));
			return ERROR_LOGFILE;
		}

		if (!global.test_mode)
			LogMessageDate(DATE_START);
	}

	// Open test mode config
	if (global.test_mode) {
		//std::wstring GetFileContents(L"fwatch\\data\\addonInstaller_test.cfg");
		HANDLE hFile = CreateFile(PATH_TO_TEST_CFG, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile != INVALID_HANDLE_VALUE) {
			int id_to_write[] = {INPUT_MOD_NAME, INPUT_DIR_NAME, INPUT_GAME_VER};
			std::wstring *ptr[] = {&global.current_mod, &global.current_mod_new_name, &global.game_version};
			DWORD bytesRead;
			BOOL result;

			for(;;) {
				int signature = 0;
				result = ReadFile(hFile, &signature, sizeof(signature), &bytesRead, NULL);
				if (!result || bytesRead == 0)
					break;

				int index = -1;
				for (int i=0; i<sizeof(id_to_write)/sizeof(id_to_write[0]) && index==-1; i++)
					if (signature == id_to_write[i])
						index = i;

				DWORD length = 0;
				result = ReadFile(hFile, &length, sizeof(length), &bytesRead, NULL);
				if (!result)
					break;

				if (index >= 0) {
					ptr[index]->reserve(length/sizeof(wchar_t));
					ptr[index]->resize((length-2)/sizeof(wchar_t));

					result = ReadFile(hFile, (LPVOID)ptr[index]->c_str(), length, &bytesRead, NULL);
					if (!result)
						break;

					SetWindowText(global.controls[signature], ptr[index]->c_str());
				} else {
					SetFilePointer(hFile, length, NULL, FILE_CURRENT);
				}
			}

			CloseHandle(hFile);

			if (global.current_mod_new_name.empty())
				global.current_mod_new_name = global.current_mod;
		}

		global.current_mod_version      = L"1";
		global.current_mod_version_date = time(0);
	}
			
	// Download installation script
	if (!global.arguments_table[L"downloadscript"].empty()) {
		WriteProgressFile(INSTALL_PROGRESS, global.lang[STR_ACTION_GETSCRIPT]);

		std::wstring url = GetFileContents(global.arguments_table[L"downloadscript"]) + L" --verbose \"--output-document=fwatch\\tmp\\installation script\"";
		int result       = Download(url, FLAG_OVERWRITE | FLAG_SILENT_MODE);

		if (result > 0) {
			LogMessage(L"", CLOSE_LOG);
			return ERROR_NO_SCRIPT;
		}
	}
	
	// Open installation script
	std::wstring script_file_content;
	{
		WriteProgressFile(INSTALL_PROGRESS, global.lang[STR_ACTION_READSCRIPT]);
		std::wstring script_file_name = global.test_mode ? PATH_TO_TEST_SCRIPT : L"fwatch\\tmp\\installation script";
		script_file_content           = GetFileContents(script_file_name);
	
		if (script_file_content.empty()) {
			if (global.test_mode) {
				LogMessage(L"Failed to open " + script_file_name, CLOSE_LOG);
				WriteProgressFile(INSTALL_ERROR, (global.lang[STR_ERROR]+L"\r\n"+global.lang[STR_ERROR_READSCRIPT]));
				return ERROR_NO_SCRIPT;
			}
		}

		SetWindowText(global.controls[EDIT_SCRIPT], script_file_content.c_str());
	}
	
	ParseInstallationScript(script_file_content, global.commands);
	FillCommandsList();

	if (global.commands.size() == 0) {
		if (global.test_mode) {
			SwitchTab(INSTALLER_TAB_SCRIPT);
		} else {
			LogMessage(L"Script is empty", CLOSE_LOG);
			WriteProgressFile(INSTALL_ERROR, (global.lang[STR_ERROR]+L"\r\n"+global.lang[STR_ERROR_READSCRIPT]));
			return ERROR_WRONG_SCRIPT;
		}
	}

	// If wrong version
	if (!global.test_mode && (global.script_version == 0 || global.installer_version < global.script_version)) {
		LogMessage(L"Version mismatch. Script version: " + Double2StrW(global.script_version) + L"  Program version: " + Double2StrW(global.installer_version), CLOSE_LOG);
		WriteProgressFile(INSTALL_ERROR, (global.lang[STR_ERROR_WRONG_VERSION] + L"\r\n" + Double2StrW(global.script_version) + L" vs " + Double2StrW(global.installer_version)));
		return ERROR_WRONG_SCRIPT;
	}

	// Install for Real
	bool play_automatically             = !global.test_mode;
	global.instruction_index            = 0;
	global.order                        = ORDER_NONE;
	INSTALLER_ERROR_CODE command_result = ERROR_NONE;
	std::vector<LARGE_INTEGER> download_sizes;
	download_sizes.resize(global.commands.size());

	for (;;) {
		global.installation_phase = PHASE_WAITING;

		InvalidateRect(global.controls[LIST_COMMANDS], NULL, false); //trigger WM_DRAWITEM for the listbox

		// Scroll listbox to show current command
		{
			size_t top = (size_t)SendMessage(global.controls[LIST_COMMANDS], LB_GETTOPINDEX, 0, 0);
			LRESULT item_height = SendMessage(global.controls[LIST_COMMANDS], LB_GETITEMHEIGHT, 0, 0);
			RECT listbox;
			GetWindowRect(global.controls[LIST_COMMANDS], &listbox);
			int visible_items = (listbox.bottom - listbox.top) / item_height;

			if (global.instruction_index < global.commands.size()) {
				if (global.instruction_index < top)
					SendMessage(global.controls[LIST_COMMANDS], LB_SETTOPINDEX, (WPARAM)global.instruction_index, 0);
				else
					if (global.instruction_index >= (top+visible_items))
						SendMessage(global.controls[LIST_COMMANDS], LB_SETTOPINDEX, (WPARAM)(global.instruction_index-visible_items+1), 0);
			}
		}

		if (command_result == ERROR_NONE) {
			if (global.instruction_index >= global.commands.size())
				play_automatically = false;
			else
				if (global.test_mode) {
					WriteProgressFile(INSTALL_PROGRESS, L"");
					Sleep(500); // I slow down the test mode to show that the arrow is moving in the command list
				}
		}

		// Stop and wait for user's instruction
		if (global.order!=ORDER_NONE || command_result!=ERROR_NONE || !play_automatically) {
			EnableWindow(global.controls[BUTTON_REWIND], command_result!=ERROR_NONE || global.instruction_index>0);
			EnableWindow(global.controls[BUTTON_BACK], global.instruction_index>0);
			EnableWindow(global.controls[BUTTON_NEXT], command_result==ERROR_NONE && global.instruction_index<global.commands.size());
			EnableWindow(global.controls[BUTTON_PLAY], global.instruction_index<global.commands.size());
			EnableWindow(global.controls[BUTTON_SAVETEST], command_result==ERROR_NONE);
			EnableWindow(global.controls[INPUT_MOD_NAME], command_result==ERROR_NONE && global.instruction_index==0);
			EnableWindow(global.controls[INPUT_DIR_NAME], command_result==ERROR_NONE && global.instruction_index==0);
			EnableWindow(global.controls[INPUT_GAME_VER], command_result==ERROR_NONE && global.instruction_index==0);
			EnableWindow(global.controls[BUTTON_JUMP_TO_STEP], command_result==ERROR_NONE && global.instruction_index<global.commands.size() && !global.commands[global.instruction_index].disable);
			SetWindowText(global.controls[BUTTON_PLAY], play_automatically && command_result==ERROR_NONE ? L"||" : L">");
			SumDownloadSizes(download_sizes, global.instruction_index);

			if (command_result == ERROR_COMMAND_FAILED) {
				WriteProgressFile(INSTALL_RETRYORABORT, global.last_log_message + L"\r\n\r\n" + global.lang[STR_ASK_RETRYORABORT]);
				EnableMenuItem(global.window_menu, ID_PROCESS_RETRY, MF_BYCOMMAND);
			} else
				// Installation finished
				if (global.instruction_index >= global.commands.size()) {
					// Clean up after the last mod
					EndMod();

					if (global.missing_modfolders.empty()) {
						WriteProgressFile(INSTALL_DONE, global.lang[STR_ACTION_DONE]);
						LogMessageDate(DATE_END);
					} else {
						std::wstring message = ReplaceAll(global.lang[STR_ACTION_DONEWARNING], L"%MOD%", global.missing_modfolders);
						WriteProgressFile(INSTALL_WARNING, message);
						LogMessage(L"WARNING: Installation completed but modfolders " + global.missing_modfolders + L" are still missing");
						global.restart_game = false;
					}

					if (global.test_mode)
						global.missing_modfolders.clear();
					else
						break;
				} else 
					if (global.order==ORDER_PAUSE || command_result==ERROR_PAUSED)
						WriteProgressFile(INSTALL_PAUSED, L"Installation paused");
					else
						if (global.order==ORDER_ABORT || command_result==ERROR_USER_ABORTED) {
							RollBackInstallation();
							WriteProgressFile(INSTALL_ABORTED, global.lang[STR_ACTION_ABORTED]);
							LogMessage(L"Installation aborted by user", CLOSE_LOG);
							return ERROR_USER_ABORTED;
						} else 
							if (global.order == ORDER_PLAY) {
								play_automatically = !play_automatically;
								SetWindowText(global.controls[BUTTON_PLAY], play_automatically ? L"||" : L">");
							}

			global.order = ORDER_NONE;

			while (global.order == ORDER_NONE)
				Sleep(100);

			// Reload settings when beginning a new installation process in test mode
			if (global.test_mode && global.instruction_index==0 && command_result==ERROR_NONE && (global.order==ORDER_PLAY || global.order==ORDER_NEXT)) {
				std::wstring new_mod = L"";
				WindowTextToString(global.controls[INPUT_MOD_NAME], new_mod);
				Trim(new_mod);

				if (new_mod.empty()) {
					MessageBox(global.window, L"Mod name cannot be empty!", L"Addon Installer", MB_OK | MB_ICONEXCLAMATION);
					continue;
				}

				wchar_t is_forbidden = VerifyWindowsFileName(new_mod);
				if (is_forbidden != NULL) {
					std::wstring msg = L"Mod name cannot contain |";
					msg[msg.length()-1] = is_forbidden;
					MessageBox(global.window, msg.c_str(), L"Addon Installer", MB_OK | MB_ICONEXCLAMATION);
					continue;
				}

				std::wstring new_dir_name = L"";
				WindowTextToString(global.controls[INPUT_DIR_NAME], new_dir_name);
				Trim(new_dir_name);

				if (!new_dir_name.empty()) {
					is_forbidden = VerifyWindowsFileName(new_dir_name);
					if (is_forbidden != NULL) {
						std::wstring msg = L"Dir name cannot contain |";
						msg[msg.length()-1] = is_forbidden;
						MessageBox(global.window, msg.c_str(), L"Addon Installer", MB_OK | MB_ICONEXCLAMATION);
						global.order = ORDER_NONE;
						continue;
					}
				}

				std::wstring new_game_version = L"";
				WindowTextToString(global.controls[INPUT_GAME_VER], new_game_version);
				Trim(new_game_version);

				if (new_game_version != global.game_version) {
					if (new_game_version.empty()) {
						MessageBox(global.window, L"Game version cannot be empty!", L"Addon Installer", MB_OK | MB_ICONEXCLAMATION);
						global.order = ORDER_NONE;
						continue;
					}

					bool is_number = true;
					for (size_t i=0; i<new_game_version.length(); i++) {
						if (!iswdigit(new_game_version[i]) && new_game_version[i]!=L'.') {
							is_number = false;
							break;
						}
					}

					if (!is_number) {
						MessageBox(global.window, L"Game version must be a number", L"Addon Installer", MB_OK | MB_ICONEXCLAMATION);
						global.order = ORDER_NONE;
						continue;
					}

					ParseInstallationScript(script_file_content, global.commands);
				}

				global.current_mod  = new_mod;
				global.game_version = new_game_version;

				if (new_dir_name.empty())
					global.current_mod_new_name = global.current_mod;
				else
					global.current_mod_new_name = new_dir_name;

				// Save config
				HANDLE hFile = CreateFile(PATH_TO_TEST_CFG, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

				if (hFile != INVALID_HANDLE_VALUE) {
					int id_to_write[] = {INPUT_MOD_NAME, INPUT_DIR_NAME, INPUT_GAME_VER};
					DWORD bytesWritten;

					for (int i=0; i<sizeof(id_to_write)/sizeof(id_to_write[0]); i++) {
						std::wstring text = L"";
						WindowTextToString(global.controls[id_to_write[i]], text);

						WriteFile(hFile, &id_to_write[i], sizeof(id_to_write[0]), &bytesWritten, NULL);
						DWORD length = (DWORD)(text.length()+1) * sizeof(wchar_t);
						WriteFile(hFile, &length, sizeof(length), &bytesWritten, NULL);
						WriteFile(hFile, text.c_str(), length, &bytesWritten, NULL);
					}

					CloseHandle(hFile);
				}

				for (size_t i=0; i<download_sizes.size(); i++)
					download_sizes[i].QuadPart = 0;

				SumDownloadSizes(download_sizes, global.instruction_index);
				LogMessageDate(DATE_START);
			}

			EnableWindow(global.controls[BUTTON_REWIND], 0);
			EnableWindow(global.controls[BUTTON_BACK], 0);
			EnableWindow(global.controls[BUTTON_NEXT], 0);
			EnableWindow(global.controls[BUTTON_PLAY], 1);
			EnableWindow(global.controls[BUTTON_SAVETEST], 0);
			EnableWindow(global.controls[INPUT_MOD_NAME], 0);
			EnableWindow(global.controls[INPUT_DIR_NAME], 0);
			EnableWindow(global.controls[INPUT_GAME_VER], 0);
			EnableWindow(global.controls[BUTTON_JUMP_TO_STEP], 0);
			EnableMenuItem(global.window_menu, ID_PROCESS_RETRY, MF_BYCOMMAND | MF_GRAYED);

			INSTALLER_ORDER order = global.order;
			global.order = ORDER_NONE;

			switch(order) {
				case ORDER_RETRY : {
					RollBackInstallation(global.instruction_index);
				} break;

				case ORDER_ABORT : {
					if (global.test_mode) {
						EnableWindow(global.controls[BUTTON_PLAY], 0);
					} else {
						RollBackInstallation();
						WriteProgressFile(INSTALL_ABORTED, global.lang[STR_ACTION_ABORTED]);
						LogMessage(L"Installation aborted by user", CLOSE_LOG);
					}

					return ERROR_USER_ABORTED;
				} break;

				case ORDER_PREV : {
					if (command_result == ERROR_NONE) {
						if (global.instruction_index > 0) {
							do {
								global.instruction_index--;
							} while (global.instruction_index>0 && (global.instruction_index>=global.commands.size() || (global.instruction_index<global.commands.size() && global.commands[global.instruction_index].disable)));
						}
					} else
						command_result = ERROR_NONE;
					
					play_automatically = false;
					RollBackInstallation(global.instruction_index);
					continue;
				} break;

				case ORDER_NEXT : {
					if (global.instruction_index >= global.commands.size())
						continue;
				} break;

				case ORDER_REWIND : {
					RollBackInstallation();
					global.instruction_index = 0;
					command_result = ERROR_NONE;
					play_automatically = false;
					continue;
				} break;

				case ORDER_PLAY : {
					if (command_result == ERROR_NONE) {
						play_automatically = !play_automatically;
						SetWindowText(global.controls[BUTTON_PLAY], play_automatically ? L"||" : L">");
						if (global.instruction_index >= global.commands.size())
							continue;
					} else {
						RollBackInstallation(global.instruction_index);
					}
				} break;

				case ORDER_PAUSE : command_result=ERROR_NONE;continue;

				case ORDER_RELOAD : {
					script_file_content.clear();
					WindowTextToString(global.controls[EDIT_SCRIPT], script_file_content);
					
					std::ofstream out(PATH_TO_TEST_SCRIPT, std::ios::out | std::ios::binary | std::ios::trunc);
					if (out.is_open()) {
						out << utf8(script_file_content);
						out.close();
					}

					global.instruction_index = ParseInstallationScript(script_file_content, global.commands, COMPARE_OLD_WITH_NEW);
					FillCommandsList();
					RollBackInstallation(global.instruction_index);
					SwitchTab(INSTALLER_TAB_INSTRUCTIONS);
					download_sizes.resize(global.commands.size());
					SumDownloadSizes(download_sizes, global.instruction_index);
					command_result = ERROR_NONE;
					play_automatically = false;
					continue;
				} break;

				case ORDER_JUMP : {
					size_t selected = (size_t)SendMessage(global.controls[LIST_COMMANDS], LB_GETCURSEL, 0, 0);
					if (selected<global.commands.size() && !global.commands[selected].disable && command_result==ERROR_NONE && selected!=global.instruction_index) {
						global.instruction_index = selected;
						RollBackInstallation(global.instruction_index);
					}
					continue;
				};
			}
		}

		global.command_line_num      = global.commands[global.instruction_index].line_num;
		global.download_iterator     = 0;
		global.last_download_attempt = true;

		if (
			// if modfolder wasn't formally started OR skipping this mod
			((global.current_mod.empty() || global.skip_modfolder) && global.commands[global.instruction_index].id != COMMAND_BEGIN_MOD)
			||
			// if version wasn't formally started
			(!global.current_mod.empty() && global.current_mod_version.empty() && global.commands[global.instruction_index].id != COMMAND_BEGIN_VERSION)
			||
			// if inside condition block
			(global.condition_index >= 0 && !global.condition[global.condition_index] && !global.commands[global.instruction_index].ctrl_flow)
			||
			global.commands[global.instruction_index].disable
		) {
			do {
				global.instruction_index++;
			} while (global.instruction_index < global.commands.size() && global.commands[global.instruction_index].disable);
			continue;
		}
		
		command_result          = ERROR_NONE;
		size_t failed_downloads = 0;

		// Check if there's an URL list for this command
		if (global.commands[global.instruction_index].downloads.size() > 0) {
			Download_Phase:
			global.installation_phase = PHASE_DOWNLOADING;
			
			// For each url
			for (;  global.download_iterator<global.commands[global.instruction_index].downloads.size();  global.download_iterator++) {
				size_t j                     = global.download_iterator;
				int download_flags           = FLAG_CONTINUE | (global.commands[global.instruction_index].id == COMMAND_DOWNLOAD ? FLAG_CLEAN_DL_LATER : FLAG_CLEAN_DL_NOW);
				global.last_download_attempt = j == global.commands[global.instruction_index].downloads.size() - 1;
				global.command_line_num      = global.commands[j].line_num;
				
				// Check how many url arguments
				if (global.commands[global.instruction_index].downloads[j].arguments.size() == 0)
					command_result = Download(global.commands[global.instruction_index].downloads[j].url, download_flags);
				else 
				if (global.commands[global.instruction_index].downloads[j].arguments.size() == 1)
					command_result = Download(global.commands[global.instruction_index].downloads[j].url + L" \"--output-document=" + global.commands[global.instruction_index].downloads[j].arguments[0] + L"\"", download_flags);
				else {
					std::wstring original_url     = global.commands[global.instruction_index].downloads[j].url;
					std::wstring cookie_file_name = L"fwatch\\tmp\\__cookies.txt";
					std::wstring token_file_name  = L"fwatch\\tmp\\__downloadtoken";
					std::wstring wget_arguments   = L"";
					std::wstring new_url          = original_url;
					std::wstring POST             = L"";
					bool found_phrase             = false;
				
					DeleteFile(cookie_file_name.c_str());
					DeleteFile(token_file_name.c_str());
				
					for (size_t k=0; k<global.commands[global.instruction_index].downloads[j].arguments.size()-1; k++) {
						wget_arguments = L"";
						
						if (!POST.empty()) {
							wget_arguments += L"--post-data=" + POST + L" ";
							POST            = L"";
						}
					
						wget_arguments += (k==0 ? L"--keep-session-cookies --save-cookies " : L"--load-cookies ") + cookie_file_name + L" --output-document=" + token_file_name + L" " + new_url;
						command_result  = Download(wget_arguments, FLAG_OVERWRITE, new_url);
				
						if (command_result != ERROR_NONE)
							goto Finished_downloading;
				
						// Parse downloaded file and find link
						std::wstring token_file_buffer = GetFileContents(token_file_name);
					    bool is_href                   = Equals(global.commands[global.instruction_index].downloads[j].arguments[k].substr(0,6),L"href=\"") || Equals(global.commands[global.instruction_index].downloads[j].arguments[k].substr(0,6),L"href=''");
						size_t find                    = token_file_buffer.find(global.commands[global.instruction_index].downloads[j].arguments[k]);
			
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
											
											POST += (POST.empty() ? L"" : L"&") + UrlEncode(name) + L"=" + UrlEncode(value);
										}
										
										input = GetTextBetween(form, L"<input", L">", offset);
									}
								}
								
								new_url = found_url;
							}
						}

						if (!found_phrase) {
							command_result = ErrorMessage(STR_DOWNLOAD_FIND_ERROR, L"%STR% " + global.commands[global.instruction_index].downloads[j].arguments[k]);
							goto Finished_downloading;
						}
							
						token_file_name += UInt2StrW(k);
					}

					wget_arguments = L"--load-cookies " + cookie_file_name;
					
					if (!POST.empty())
						wget_arguments += L" --post-data=\"" + POST + L"\" ";
					
					size_t last_url_arg = global.commands[global.instruction_index].downloads[j].arguments.size() - 1;
					wget_arguments     +=  L" \"--output-document=" + global.commands[global.instruction_index].downloads[j].arguments[last_url_arg] + L"\" " + new_url;
					command_result      = Download(wget_arguments, download_flags, new_url);
				
					if (!global.test_mode) {
						DeleteFile(cookie_file_name.c_str());
						DeleteFile(token_file_name.c_str());
					}
				}
		
				Finished_downloading:
				if (global.order==ORDER_PAUSE || global.order==ORDER_ABORT)
					break;
				else
					if (command_result == ERROR_NONE) {
						std::wstring path = L"fwatch\\tmp\\" + global.downloaded_filename;
						HANDLE hFile = CreateFile(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
						if (hFile != INVALID_HANDLE_VALUE) {
							LARGE_INTEGER lFileSize = {0};
							if (GetFileSizeEx(hFile, &lFileSize)) {
								download_sizes[global.instruction_index].QuadPart = lFileSize.QuadPart;
								SumDownloadSizes(download_sizes, global.instruction_index);
							}
							CloseHandle(hFile);
						}
						break;
					} else
						failed_downloads++;
			}
		}
		
		
		
		// If download was successful then execute command
		if ((
				command_result != ERROR_USER_ABORTED && 
				command_result != ERROR_PAUSED
			) && 
			(
				global.commands[global.instruction_index].downloads.size() == 0 || 
				(
					global.commands[global.instruction_index].downloads.size() > 0 && 
					failed_downloads < global.commands[global.instruction_index].downloads.size()
				)
			)
		) {
			global.installation_phase = PHASE_EXECUTING;
			
			switch(global.commands[global.instruction_index].id) {
				case COMMAND_BEGIN_MOD : {
					if (global.commands[global.instruction_index].arguments.size() < 4) {
						command_result = ErrorMessage(STR_ERROR_INVALID_SCRIPT, L"%STR%", ERROR_WRONG_SCRIPT);
						break;
					}
					
					if (!global.current_mod.empty())
						EndMod();
						
					WriteProgressFile(INSTALL_PROGRESS, global.lang[STR_ACTION_PREPARING]);
				
					global.current_mod          = global.commands[global.instruction_index].arguments[0];
					global.current_mod_id       = global.commands[global.instruction_index].arguments[1];
					global.current_mod_keepname = global.commands[global.instruction_index].arguments[2];
					global.command_line_num     = 0;
					global.current_mod_version  = L"";
					
					// Make a list of mod aliases for the entire installation
					std::vector<std::wstring> aliases;
					Tokenize(global.commands[global.instruction_index].arguments[3], L" ", aliases);
					
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
					if (global.commands[global.instruction_index].arguments.size() >= 2) {
						if (!global.current_mod_version.empty())
							EndModVersion();
						
						global.current_mod_version      = global.commands[global.instruction_index].arguments[0];
						global.current_mod_version_date = _wtoi(global.commands[global.instruction_index].arguments[1].c_str());
						global.command_line_num         = 0;
					} else
						command_result = ErrorMessage(STR_ERROR_INVALID_SCRIPT, L"%STR%", ERROR_WRONG_SCRIPT);

					break;
				}
				
				case COMMAND_ALIAS : {
					INSTALLER_OPERATION_LOG backup = {0};
					backup.instruction_index       = global.instruction_index;
					backup.operation_type          = OPERATION_ALIAS;
					backup.source                  = L"";

					for (size_t i=0; i<global.current_mod_alias.size(); i++)
						backup.source += L" " + global.current_mod_alias[i];

					global.rollback.push_back(backup);

					if (global.commands[global.instruction_index].arguments.size() == 0)
						global.current_mod_alias.clear();
					else 
						for (size_t j=0; j<global.commands[global.instruction_index].arguments.size(); j++)
							global.current_mod_alias.push_back(global.commands[global.instruction_index].arguments[j]);
					break;
				}

				case COMMAND_IF_VERSION : command_result=Condition_If_version(global.commands[global.instruction_index].arguments); break;
				case COMMAND_ELSE       : command_result=Condition_Else(); break;
				case COMMAND_ENDIF      : command_result=Condition_Endif(); break;
				case COMMAND_EXIT       : global.instruction_index=global.commands.size(); break;

				case COMMAND_AUTO_INSTALL :  {
					LogMessage(L"Auto installation"); 
					std::wstring file = global.downloaded_filename;
					
					if (global.commands[global.instruction_index].arguments.size() > 0)
						file = global.commands[global.instruction_index].arguments[0];
					
					std::wstring file_with_path = L"fwatch\\tmp\\" + file;
					DWORD attributes            = GetFileAttributes(file_with_path.c_str());
					
					if (attributes != INVALID_FILE_ATTRIBUTES) {
						command_result = Auto_Install(file, attributes, FLAG_RUN_EXE, global.commands[global.instruction_index].password);
						
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
					std::wstring file_name = global.commands[global.instruction_index].arguments[0];

					if (Equals(file_name,L"<download>") || Equals(file_name,L"<dl>") || (file_name).empty())
						file_name = global.downloaded_filename;

					if (!(file_name).empty()) {
						command_result = Unpack(file_name, global.commands[global.instruction_index].password);
						
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
					std::wstring source      = global.commands[global.instruction_index].arguments[0];
					std::wstring destination = global.commands[global.instruction_index].arguments[1];
					std::wstring new_name    = global.commands[global.instruction_index].arguments[2];

					if (source.empty()) {
						command_result = ErrorMessage(STR_ERROR_NO_FILE);
						break;
					}

					bool is_download_dir = true;
					int options          = FLAG_OVERWRITE | (global.commands[global.instruction_index].id==COMMAND_MOVE ? FLAG_MOVE_FILES : FLAG_NONE);

					if (global.commands[global.instruction_index].switches & SWITCH_NO_OVERWRITE)
						options &= ~FLAG_OVERWRITE;

					if (global.commands[global.instruction_index].switches & SWITCH_MATCH_DIR)
						options |= FLAG_MATCH_DIRS;
						
					if (global.commands[global.instruction_index].switches & SWITCH_MATCH_DIR_ONLY)
						options |= FLAG_MATCH_DIRS | FLAG_MATCH_DIRS_ONLY;

					for (int j=0; j<=2; j++)
						if (!VerifyPath(global.commands[global.instruction_index].arguments[j])) {
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
					std::wstring *path = &global.commands[global.instruction_index].arguments[0];
					
					if (VerifyPath(*path))
						command_result = MakeDir(global.current_mod_new_name + L"\\" + *path);
					else
						command_result = ErrorMessage(STR_ERROR_PATH);
				
					break;
				}
				
				case COMMAND_DELETE : {
					WriteProgressFile(INSTALL_PROGRESS, global.lang[STR_ACTION_DELETING]+L"...");
					
					std::wstring file_name = global.commands[global.instruction_index].arguments[0];
					int options            = FLAG_MOVE_FILES | FLAG_ALLOW_ERROR;
					
					if (global.commands[global.instruction_index].switches & SWITCH_MATCH_DIR)
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
						if (global.order == ORDER_ABORT) {
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
								INSTALLER_OPERATION_LOG backup = {0};
								backup.instruction_index = global.instruction_index;
								backup.operation_type    = OPERATION_MOVE;
								backup.source            = backup_path;
								backup.destination       = source_list[j];
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
				
					std::wstring source      = global.commands[global.instruction_index].arguments[0];
					std::wstring destination = global.commands[global.instruction_index].arguments[1];
					int options              = FLAG_MOVE_FILES;
					
					if (global.commands[global.instruction_index].switches & SWITCH_MATCH_DIR)
						options |= FLAG_MATCH_DIRS;
						
					for (int j=0; j<2; j++)
						if (!VerifyPath(global.commands[global.instruction_index].arguments[j])) {
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
						if (global.order == ORDER_ABORT) {
							command_result = ERROR_USER_ABORTED;
							break;
						}

						// Format path for logging
						LogMessage(L"Renaming  " + ReplaceAll(source_list[j], L"fwatch\\tmp\\_extracted\\", L"") + L"  to  " + PathLastItem(destination_list[j]));

						// Rename
					    if (MoveFileEx(source_list[j].c_str(), destination_list[j].c_str(), 0)) {
							INSTALLER_OPERATION_LOG backup = {0};
							backup.instruction_index = global.instruction_index;
							backup.operation_type    = OPERATION_MOVE;
							backup.source            = destination_list[j];
							backup.destination       = source_list[j];
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
					std::wstring file_name = global.commands[global.instruction_index].arguments[0];
					
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
					if (global.commands[global.instruction_index].arguments.size() < 2) {
						command_result = ErrorMessage(STR_ERROR_ARG_COUNT);
						break;
					}
					
					std::wstring *file_name   = &global.commands[global.instruction_index].arguments[0];
					std::wstring *url         = &global.commands[global.instruction_index].arguments[1];
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
					
					std::wstring file_name = global.commands[global.instruction_index].arguments[0];
										
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
					INSTALLER_OPERATION_LOG backup = {0};
					backup.instruction_index       = global.instruction_index;

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
							if (global.order == ORDER_ABORT) {
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
				
							if (global.commands[global.instruction_index].switches & SWITCH_TIMESTAMP)
								command_result = ChangeFileDate(pbo_name, global.commands[global.instruction_index].timestamp);
							else
								command_result = ChangeFileDate(pbo_name, global.current_mod_version_date);

							if (command_result == ERROR_NONE && ~global.commands[global.instruction_index].switches & SWITCH_KEEP_SOURCE) {
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
					std::wstring source      = global.commands[global.instruction_index].arguments[0];
					std::wstring destination = global.commands[global.instruction_index].arguments[1];
					
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
				
					if (global.commands[global.instruction_index].arguments.size() < 3) {
						command_result = ErrorMessage(STR_ERROR_ARG_COUNT);
						break;
					}
					
					std::wstring file_name  = global.commands[global.instruction_index].arguments[0];
					std::string wanted_text = utf8(global.commands[global.instruction_index].arguments[2]);
					size_t wanted_line      = wcstoul(global.commands[global.instruction_index].arguments[1].c_str(), NULL, 10);
				
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
					INSTALLER_OPERATION_LOG backup = {0};
					backup.instruction_index       = global.instruction_index;
					backup.operation_type          = OPERATION_NONE;

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

							if (CopyFile(file_name.c_str(), backup_path.c_str(), 1)) {
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
				    
				    if (~global.commands[global.instruction_index].switches & SWITCH_NEWFILE) {
				    	LogMessage(L"Editing line " + UInt2StrW(wanted_line) + L" in " + file_name);
				    	
				    	file.open(file_name.c_str(), std::ios::in);
				    	
						if (file.is_open()) {
							std::string line;
						
							while (getline(file, line)) {
								line_number++;
								
								if (file.eof())
									ends_with_newline = false;
								
								if (line_number == wanted_line) {
									std::string new_line = global.commands[global.instruction_index].switches & SWITCH_APPEND ? line+wanted_text : wanted_text;
								
									contents.push_back(new_line);
									
									if (global.commands[global.instruction_index].switches & SWITCH_INSERT) {
										contents.push_back(line);
										line_number++;
									}
								} else 
									contents.push_back(line);
							}
							
							if (global.commands[global.instruction_index].switches & SWITCH_INSERT  &&  (wanted_line==0 || wanted_line > line_number)) {
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
						
						if (global.commands[global.instruction_index].switches & SWITCH_TIMESTAMP)
							command_result = ChangeFileDate(file_name, global.commands[global.instruction_index].timestamp);
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
					if (global.commands[global.instruction_index].arguments.size() < 2) {
						command_result = ErrorMessage(STR_ERROR_ARG_COUNT);
						break;
					}
					
					std::wstring file_name = global.commands[global.instruction_index].arguments[0];
					std::wstring date_text = global.commands[global.instruction_index].arguments[1];
					
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
								INSTALLER_OPERATION_LOG backup = {0};
								backup.instruction_index       = global.instruction_index;
								backup.operation_type          = OPERATION_FILEDATE;
								backup.source                  = file_name;
								backup.modif_time              = modification_time;
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
		if (command_result == ERROR_WRONG_ARCHIVE) {
			std::wstring file_name = L"fwatch\\tmp\\" + global.downloaded_filename;
			DeleteFile(file_name.c_str());
			global.downloads.pop_back();
			command_result = ERROR_COMMAND_FAILED;
		}

		if (command_result == ERROR_NONE) {
			do {
				global.instruction_index++;
			} while (global.instruction_index < global.commands.size() && global.commands[global.instruction_index].disable);
		}
	}

    return ERROR_NONE;
}

	// Separate thread for checking user feedback
DWORD WINAPI ReceiveInstructions(__in LPVOID lpParameter)
{
	UNREFERENCED_PARAMETER(lpParameter);
	
	while (WaitForSingleObject(global.thread_installer, 0) != WAIT_OBJECT_0) {
		std::wstring file_name = L"fwatch\\tmp\\schedule\\InstallerInstruction.txt";
		std::wstring contents  = GetFileContents(file_name);

		if (!contents.empty()) {
			if (contents == L"abort") {
				if (global.order == ORDER_NONE)
					global.order = ORDER_ABORT;
				DisableMenu();
			}

			if (contents == L"restart") {
				global.restart_game = !global.restart_game;
				CheckMenuItem(global.window_menu, ID_OPTIONS_RESTARTGAME, global.restart_game ? MF_CHECKED : MF_UNCHECKED);
			}
				
			if (contents == L"voice")
				global.run_voice_program = !global.run_voice_program;

			if (contents == L"retry")
				if (global.order == ORDER_NONE)
					global.order = ORDER_RETRY;

			if (contents == L"pause") {
				if (global.order == ORDER_NONE)
					global.order = ORDER_PAUSE;
				CheckMenuItem(global.window_menu, ID_PROCESS_PAUSE, MF_CHECKED);
			}

			if (contents == L"resume") {
				if (global.order == ORDER_NONE)
					global.order = ORDER_PLAY;
				CheckMenuItem(global.window_menu, ID_PROCESS_PAUSE, MF_UNCHECKED);
			}

			DeleteFile(L"fwatch\\tmp\\schedule\\InstallerInstruction.txt");
		}

		Sleep(100);
	}
	return 0;
}