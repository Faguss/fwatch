/* PROPRIETARY CODE WAS REPLACED WITH DOTS */
#include <.................>
#include <..........................>
#include <.......................>
#include <.....................>
#include "errno.h"
#include "windows.h"
#include <string>

	// http://stackoverflow.com/a/3418285
std::string ReplaceAll(std::string str, const std::string& from, const std::string& to) 
{
    if (from.empty())
        return str;
        
    size_t start_pos = 0;
    
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
    
    return str;
}


class FilePreprocessor : public .......
{
protected:
	........ ...............(const char *filename)
	{
		std::string filename_adjusted, new_full_path;
		bool found = false;

		if (include_count++ > 0) {
			filename_adjusted = ReplaceAll(filename, "*", "z");
			filename          = filename_adjusted.c_str();

			// Simulate context of an addon or mission description
			if (addondir_size>0  ||  gamedir) {

				// Check if the file exists in the selected addon folder
				for (int i=0; i<addondir_size && !found; i++) {
					new_full_path = addondir[i];
					new_full_path += "\\";
					new_full_path += filename_adjusted;

					if (..........::.........(new_full_path.c_str())) {
						filename = new_full_path.c_str();
						found    = true;
					}
				}

				// Check if the file exists in the game dir root
				if (!found && gamedir) {
					new_full_path = gamedir;
					new_full_path += "\\";
					new_full_path += filename_adjusted;

					if (..........::.........(new_full_path.c_str())) {
						filename = new_full_path.c_str();
						found    = true;
					}
				}
			}
		}

		if (!found && !..........::.........(filename))
			return NULL;

		.......... ....... = new ..........();
		......->........(filename);
		return ......;
	}

	void .............(........ *......)
	{
		if (......) delete (.......... *)......;
	}
};


void fwatch_error(std::string &container_main, std::string &container, std::string &input_filename) {
	std::string error_message(strerror(errno));
	error_message = ReplaceAll(error_message, "\"", "\"\"");
	
	container += "]+[false,7,";
	container += errno;
	container += ",\"";
	container += error_message;
	container += " - ";
	container += input_filename;
	container += "\"";

	container_main = "false,7,";
	container_main += errno;
	container_main += ",\"";
	container_main += error_message;
	container_main += " - ";
	container_main += input_filename;
	container_main += "\"";
}


int main (int argc, char* argv[]) {
	if (argc <= 1) {
		printf("preproc.exe\npreprocess given file according to OFP syntax\n\n\tUsage:\n\tpreproc [-fwatch] [-merge] [-includepath=] <inputfilename1> ...\n\n");
		return 1;
	}

	bool arg_fwatch        = false;
	bool arg_merge         = false;
	bool arg_addon         = false;
	const int addondir_max = 32;
	char *arg_addondir[addondir_max];
	char *arg_gamedir      = NULL;
	char *arg_output       = NULL;
	int arg_addondir_size  = 0;
	int file_count         = 0;
	size_t files_done      = 0;
	std::string fwatch_output;
	std::string fwatch_output_main;

	// Process options
	for (int i=1; i<argc; i++) {
		if (strncmp(argv[i],"-addondir=",10) == 0) {
			if (arg_addondir_size < addondir_max)
				arg_addondir[arg_addondir_size++] = argv[i] + 10;
			
			arg_addon = true;
			continue;
		}

		if (strncmp(argv[i],"-gamedir=",9) == 0) {
			arg_gamedir = argv[i] + 9;
			continue;
		}
		
		if (strcmp(argv[i],"-fwatch") == 0) {
			arg_fwatch = true;
			arg_gamedir = argv[0];
			continue;
		}

		if (strcmp(argv[i],"-merge") == 0) {
			arg_merge = true;
			continue;
		}

		if (strncmp(argv[i],"-out=",5) == 0) {
			arg_output = argv[i] + 5;
			continue;
		}
		
		file_count++;
	}


	// Do file operations
	for (int i=1; i<argc; i++) {
		if (strncmp(argv[i],"-addondir=",10)!=0 && strncmp(argv[i],"-gamedir=",9)!=0  && strncmp(argv[i],"-out=",5)!=0&& strcmp(argv[i],"-fwatch")!=0 && strcmp(argv[i],"-merge")!=0) {
			FilePreprocessor preproc;
			........ .........;
			preproc.addondir      = arg_addondir;
			preproc.addondir_size = arg_addondir_size;
			preproc.gamedir       = arg_gamedir;
			preproc.text[0]       = '\0';
			preproc.include_count = 0;

			std::string input_filename(argv[i]);

			if (!arg_addon) {
				size_t last_slash = input_filename.find_last_of('\\');

				// Change current directory to the input file location (so that #include works from there)
				if (last_slash != std::string::npos) {
					std::string input_path = input_filename.substr(0, last_slash);
					SetCurrentDirectory(input_path.c_str());
					input_filename = input_filename.substr(last_slash + 1);
				}
			}

			int result = preproc........(&........., input_filename.c_str());

			if (result) {
				std::string output_filename(input_filename);
				std::string file_extension = "";
				size_t last_dot            = output_filename.find_last_of(".");

				if (last_dot != std::string::npos) {
					file_extension  = output_filename.substr(last_dot);
					output_filename = output_filename.substr(0, last_dot);
				}

				output_filename += "_processed" + file_extension;

				if (arg_fwatch && arg_output)
					SetCurrentDirectory(argv[0]);

				FILE *file = fopen(arg_output ? arg_output : output_filename.c_str(), arg_merge ? "ab" : "wb");

				if (file) {	
					int bytes_written = fwrite(.......(..........str(),................()), 1, ................(), file);

					if (bytes_written == ................()) {
						files_done++;

						if (arg_fwatch) {
							fwatch_output += "]+[[true,0,0,\"\",";
							char temp[64];
							snprintf(temp, sizeof(temp), "%d", (...............+1));
							fwatch_output += temp;
							fwatch_output += ",\"";
							fwatch_output += ReplaceAll(............, "\"", "\"\"");
							fwatch_output += "\"]";
						}
					} else {
						if (arg_fwatch)
							fwatch_error(fwatch_output_main, fwatch_output, input_filename);
						else {
							printf("Error writing %s - %d/%d bytes written - ", output_filename, bytes_written, ................());
							perror("");
						}
					}

					fclose(file);
				} else {
					if (arg_fwatch)
						fwatch_error(fwatch_output_main, fwatch_output, input_filename);
					else {
						printf("Error opening %s - ", output_filename);
						perror("");
					}
				}
			} else {
				char error_description_list[][64] = {
					"Missing #endif after condition statement",
					"Failed to #include file",
					"Incorrect #include syntax",
					"Maximal number of recursions was reached",
					"Incorrect macro argument",
					"Incorrect parameter syntax",
					"Missing condition statement before #endif",
					"Unknown directive",
					"Unexpected end of file",
					"Too many parameters",
					"Too few parameters",
					"Incorrect #ifdef/#ifndef/#undef argument",
					"Missing #endif after #else"
				};

				if (arg_fwatch) {
					fwatch_output += "]+[[false,8,";
					char temp[64];
					snprintf(temp, sizeof(temp), "%d", .............);
					fwatch_output += temp;
					fwatch_output += ",\"";
					fwatch_output += error_description_list[.............];
					fwatch_output += "\",";
					snprintf(temp, sizeof(temp), "%d", (...............+1));
					fwatch_output += temp;
					fwatch_output += ",\"";
					fwatch_output += ReplaceAll(............, "\"", "\"\"");
					fwatch_output += "\"]";

					fwatch_output_main = "false,8,";
					snprintf(temp, sizeof(temp), "%d", .............);
					fwatch_output_main += temp;
					fwatch_output_main += ",\"";
					fwatch_output_main += error_description_list[.............];
					fwatch_output_main += "\"";
				} else
					printf("Preprocessor error %d\n%s\nLine: %d\nText: %s", .............,error_description_list[.............],...............+1,............); break;
			}
		}
	}

	if (arg_fwatch) {
		if (files_done > 0)
			fwatch_output_main = "true,0,0,\"\"";

		if (arg_fwatch)
			if (file_count > 1)
				printf("[%s,[%s]]", fwatch_output_main.c_str(), fwatch_output.c_str());
			else
				printf("%s", fwatch_output.c_str()+3);
	}

	return files_done ? 0 : 2;
};