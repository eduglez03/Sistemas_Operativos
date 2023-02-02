#include <iostream>
#include <vector>
#include <system_error>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <utime.h>
#include <fcntl.h>
#include <string>
#include <sstream>
#include <cstdlib>
#include <limits.h>
#include <libgen.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>


namespace shell {
  using command = std::vector<std::string>;
  
  struct command_result {
    int return_value;
    bool is_quit_requested;
    
    // Constructor
    command_result(int return_value, bool request_quit=false) : return_value{return_value}, is_quit_requested{request_quit} {} 
    

    static command_result quit(int return_value=0) {
      return command_result{return_value, true};
    }
  };
}

void print_prompt(int last_command_status);
std::error_code read_line(int fd, std::string& line);
std::vector<shell::command> parse_line(const std::string& line);
shell::command_result execute_commands(const std::vector<shell::command>& commands);
int execute_program(const std::vector<std::string> &args, bool has_wait=true);
int echo_command(const std::vector<std::string>& argv);
int cd_command(const std::vector<std::string>& argv);
int cp_command(const std::vector<std::string>& argv);
int mv_command(const std::vector<std::string>& argv);
