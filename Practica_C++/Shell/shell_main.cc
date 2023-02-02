#include "shell.h"

int main(int argc, char* argv[]) {
  int exit_value{0};
  int last_command_status{0};
  
  while (true) {
    if(isatty(STDIN_FILENO)) {
      print_prompt(last_command_status);
      
      std::string linea;
      read_line(0, linea);

      std::vector<shell::command> comandos;
      if (!linea.empty()) {
        comandos = parse_line(linea);
      }

      if (!comandos.empty()) {
        auto [return_value, is_quit_requested] = execute_commands(comandos);
        last_command_status = return_value;
        if (is_quit_requested == true ) {
          exit_value = return_value;
          break;
        }
      }


    }
  }

  return exit_value; 
}