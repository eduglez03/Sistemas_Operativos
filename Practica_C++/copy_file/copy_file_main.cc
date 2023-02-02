#include<iostream>

#include "copy_file.h"

void Usage(int argc, char *argv) {
  
}  


int main(int argc, char *argv[]) {
 
  std::string ruta_origen = argv[2];
  std::string ruta_destino = argv[3];

  if(argv[1] == "cp") {
    if (argc != 4) {
      if (argc == 5) {
        if (argv[4] == "-a") {
          copy_file(ruta_origen, ruta_destino, true);
        }
      }
    }
    else if (argc == 4) {
      std::error_code error = copy_file(ruta_origen, ruta_destino, false);
    } 
  }

  else if (argv[1] == "mv") {
    move(ruta_origen, ruta_destino);
  }
  
  else {
    std::cout << "Modo de empleo: ./a.out fd_src fd_dst\n"
              << "Pruebe './a.out --help' para más información.\n";
    std::string help = "--help";
    if (argv[1] == help) 
      std::cout << "Programa para copiar un archivo en otro\n";
    return 1;
  }
  
  return 0;
}