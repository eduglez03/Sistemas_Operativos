#include "shell.h"

//FUNCION QUE IMPRIME EL PROMPT DE LA SHELL
void print_prompt(int last_command_status){
  std::string prompt; // string del prompt que será impresa
  char buf[LINE_MAX];

  prompt = getlogin(); //obtener el nombre del usuario actual
  gethostname(buf, LINE_MAX - 1); // obtener nombre de la maquina

  prompt = prompt + buf;

  getcwd(buf, LINE_MAX - 1); // obtener ruta actual de trabajo del proceso

  prompt = prompt + buf;

  if(last_command_status == 0) {
    prompt = prompt + "> "; // en caso de que el ultimo comando tuviese exito
  } else {
    prompt = prompt + "< "; // en caso de que el último comando no haya tenido exito
  }

  int write_prompt = write(STDOUT_FILENO, prompt.c_str(), prompt.size()); //imprimir por la salida estandar el prompt

  if(write_prompt == -1) {
    std::cout << "Error al impirmir el prompt de la shell" << std::endl;
  }
}

// FUNCION QUE LEE DE LA ENTRADA ESTANDAR EL COMANDO INTRODUCIDO POR EL USUARIO 
std::error_code read_line(int fd, std::string& line){
  std::vector<uint8_t> pending_input;  
  std::vector<uint8_t> buffer(16ul * 1024 * 1024);

  while(true){

    for(auto& i: pending_input) { // Buscamos en pending_input el primer salto de linea y almacenamos el valor de pending_input en line
      line.push_back(i);
      if (i == '\n') {
        pending_input.clear();
        return std::error_code(0, std::system_category());
      } 
    }

    ssize_t bytes{}; // valor arbitario

    bytes = read(fd, buffer.data(), buffer.size()); //leemos en el buffer

    if (bytes < 0) {
      return std::error_code(errno, std::system_category()); // if error{return errno}
    }
        
    if(buffer.empty()) { // si el buffer está vacio
      if (!pending_input.empty()) { // y pending_input no lo está
        for(int i = 0; i <= pending_input.size();) { //sustituir el contenido de line con el de pending_input
          line[i] = pending_input[i];
          i++;
        }
        line.push_back('\n'); // añadir a line un salto de linea
        pending_input.clear(); // vaciar pending_input
      }
      return std::error_code(0, std::system_category()); 
    } else {
        pending_input.insert(pending_input.end(), buffer.begin(), buffer.end()); // pending_input + buffer; 
    }
  }
}


// FUNCION QUE SEPARA LA LINEA DE COMANDOS INTRODUCIDA POR EL USUARIO, EN COMANDOS SEPARADOS QUE SE ALMACENAN EN UN VECTOR DE COMANDOS (LISTA)
std::vector<shell::command> parse_line(const std::string& line) { // dividir line (que contiene todos los comandos), en una lista de comandos separados
  std::istringstream command_stream(line);
  std::vector<shell::command> commands(16ul * 1024 * 1024);

  int contador_posicion{0};

  while(!command_stream.eof()) {
    std::string word;
    command_stream >> word;

    if (word == "|" || word == ";" || word == "&") // si la palabra es '|', ';'o '&'
      contador_posicion++;
    else { // si la palabra termina en '|', ';'o '&'
      if (word.back() == '|' || word.back() == ';' || word.back() == '&') {
        word.pop_back(); // se elmina el ultimo elemento de la palabra y se almacena en command
        commands[contador_posicion].push_back(word);
        contador_posicion++;
      } 
      else if (word.front() == '#' ) { // si el comienzo de la palabra es '#'
        break;
      } 
      else {
        commands[contador_posicion].push_back(word); // si no es nada de lo anterior, se almacena en commands
      }
    }
  }
  commands.resize(++contador_posicion);
  return commands;
}

// Funcion que ejecuta programas 
int execute_program(const std::vector<std::string> &args, bool has_wait) {
 int contador;

 for(int i = 0; i <= args.size();) {
   contador++;
    i++;
  }
  
 if(args[contador] == "&") {
   has_wait = false;
 }

 if(has_wait == true) {

 }

  std::vector<const char*> argv;
  
  for(int i = 0; i <= args.size();) {
    argv.push_back(args[i].c_str());
    i++;
  }

  argv.push_back(nullptr);

  execvp(argv[0], const_cast<char* const*>(argv.data()));
  
  pid_t pid = getpid(); 

  // Crear un proceso hijo
  pid_t child = fork(); 

  if (child == 0) {
    int status = execvp(argv[0], const_cast<char* const*>(argv.data()));
    return status; 
  }
  
  else if (child > 0) {
    int status;
    wait( &status );   
    return EXIT_SUCCESS;
  }
  
  else { 
    // Aquí solo entra el padre si no pudo crear el hijo
    return EXIT_FAILURE;
  }

  return 0;
}

// Funcion que ejecuta el comando interno de echo
int echo_command(const std::vector<std::string>& argv){
  std::string echo; // string que imprime el resultado

  for(auto& i: argv) {
    echo = echo + " " + i;
  } 
  echo = echo + "\n"; // se añade un salto de linea al final

  for(int i = 0; i <= 5;) {  // eliminar la palabra echo para que no la imprima
    echo.erase(echo.begin());
    i++;
  }

  int written = write(STDOUT_FILENO, echo.c_str(), echo.size()); // se imprime por la salida estandar la string con todas las palabras introducidas por el usuario
  
  if (written == -1){  // se comprueba si se ha producido algun error durante la escritura en la salida estandar
    return 1;
  }

  return 0;
}

// Funcion que ejecuta el comando cd para cambiar de directorio
int cd_command(const std::vector<std::string>& argv) {
  if(argv.size() > 3){ // Error por introducir demasiados argumentos
    return 1;
  }

  int change = chdir(argv[1].c_str()); // cambia de directorio al indicado
  
  if(change){ //si ocurre error devuleve valor distinto de 0
    return 1;
  }

  return 0;
}

// Funcion que copia el contenido de un archivo en otro
int cp_command(const std::vector<std::string>& argv) {
  std::string src_path = argv[1];
  std::string dst_path = argv[2];
  bool preserve_all{false};
  
  if (argv.size() > 3) {
    if (argv[3] == "-a") {
      preserve_all = true;
    }
  } else if (argv.size() < 3) {
    return 1;
  }

  //raise EACCES if src_path no existe.
  int origen_check = open(src_path.c_str(), O_RDONLY );
  if (origen_check == -1){
    return 1;
  }

  close(origen_check);

  struct stat buf_origen;
  stat(src_path.c_str() ,&buf_origen);

  //raise EACCES if src_path no es un archivo regular.    
  if(!S_ISREG(buf_origen.st_mode)){
    return 1;
  }

  struct stat buf_destino;
  stat(dst_path.c_str() ,&buf_destino);

  //if dst_path existe
  if(!S_ISDIR(buf_destino.st_mode)){
    return 1;
  }
  
  //assert src_path y dst_path no son el mismo archivo.
  if( (buf_origen.st_dev == buf_destino.st_dev) && (buf_origen.st_ino == buf_destino.st_ino) ){
    return 1;
  }

  std::string filename = src_path.substr(src_path.find_last_of("/") + 1);
  std::string new_dst_path = dst_path;

  //if dst_path es un directorio then    
  if(S_ISDIR(buf_destino.st_mode)){
    //Guardar en dst_path una nueva ruta con el nombre de archivo de src_path en el directorio dst_path
    new_dst_path = new_dst_path + filename;
  }
    
  int origen = open(src_path.c_str(), O_RDONLY);
  int destino = open(dst_path.c_str(), O_WRONLY || O_TRUNC ||O_CREAT, 00700);
  
  if (destino == -1) {
    return 1;
  }

  if (origen == -1) {
    return 1;
  }

  std::vector<uint8_t> buffer(16ul * 1024 * 1024);
  ssize_t bytes_read; //valor arbitrario

  while (bytes_read != 0) {
    bytes_read = read(origen, buffer.data(), buffer.size());
        
    if (bytes_read > 0) {
      buffer.resize(bytes_read);

      if ((write(destino, buffer.data(), buffer.size())) == -1) {
        return 1;
      }
    }

    if (bytes_read <= 0) {
      return 1;
      break;
    }
  }

  close(destino);
  close(origen); 

  if (preserve_all == true) {
    chmod(dst_path.c_str(), buf_origen.st_mode);
    chown(dst_path.c_str(), buf_origen.st_uid, buf_origen.st_gid);
    struct utimbuf buftime; 
    buftime.actime = buf_origen.st_atime; 
    buftime.modtime = buf_origen.st_mtime;
    utime(dst_path.c_str(), &buftime);
    remove(src_path.c_str());
  }

  return 0;
}

// Comando move
int mv_command(const std::vector<std::string>& argv){
  std::string src_path = argv[1];
  std::string dst_path = argv[2];

  // raise EACCES if src_path no existe.
  int origen = open(src_path.c_str(), O_RDONLY );
  if (origen == -1){
    return 1;
  }
  
  close(origen);
  
  // if dst_path es un directorio then
  struct stat buf_destino;
  stat(dst_path.c_str() ,&buf_destino);
  std::string new_dst_path = dst_path;

  if(S_ISDIR(buf_destino.st_mode)){
    std::string filename = src_path.substr(src_path.find_last_of("/") + 1);
    new_dst_path = new_dst_path + filename;
  }

  struct stat buf_origen;
  stat(src_path.c_str() ,&buf_origen);

  if(buf_origen.st_dev == buf_destino.st_dev){
    rename(src_path.c_str(), new_dst_path.c_str());
  } else{
      std::vector<std::string> auxiliar;
      auxiliar.push_back(src_path);
      auxiliar.push_back(dst_path);
      auxiliar.push_back("-a");
      int return_copy = cp_command(auxiliar);

      if(return_copy != 0){
        return 1;
      }
      unlink(src_path.c_str()); // Borrar src_path
    } 
    return 0;
}


// FUNCION QUE EJECUTA LOS COMANDOS PASADOS DESDE LA LISTA DE COMANDOS OBTENDA EN PARSE_LINE
shell::command_result execute_commands(const std::vector<shell::command>& commands) {

  shell::command_result return_function {0};
  int return_value;

  for (auto i : commands) {
    if (i[0] == "exit") { // si el primer valor de la lista de comandos es "exit", se retorna a main el valor de retorno
      return shell::command_result::quit(return_value);
    }
    
    else if (i[0] == "echo") { // si el primer valor de la lista de comandos es "echo", se llama a la funcion echo_command(), para ejecutar el comando interno
      return_value = echo_command(i);
    }

    else if(i.at(0) == "mv") { // si el primer valor de la lista de comandos es "mv", se llama a la funcion mv_command(), para ejecutar el comando interno
      return_value = mv_command(i);
    }

    else if(i[0] == "cp") { // si el primer valor de la lista de comandos es "cp", se llama a la funcion cp_command(), para ejecutar el comando interno
      return_value = cp_command(i);
    }

    else if(i[0] == "cd") { // si el primer valor de la lista de comandos es "cd", se llama a la funcion cd_command(), para ejecutar el comando interno
      return_value = cd_command(i);
    }
        
    else { // en caso de no ser ninguno de los anteriores, se entiende que es un comando externo, y se llama a la funcion execute_program()
      return_value = execute_program(i, 0);
    }
   // return_value = return_function.return_value;
  }
  return shell::command_result (return_value);
}
