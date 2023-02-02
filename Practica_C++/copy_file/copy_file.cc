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
#include <string>
#include <libgen.h>


#include "copy_file.h"

// Funcion que copia el contenido de un archivo en otro
std::error_code copy_file(const std::string& src_path, const std::string& dst_path, bool preserve_all) {

  //raise EACCES if src_path no existe.
  int origen_check = open(src_path.c_str(), O_RDONLY );
  if (origen_check < 0){
    return std::error_code(1, std::system_category());
  }
  close(origen_check);

  struct stat buf_origen;
  stat(src_path.c_str() ,&buf_origen);

  //raise EACCES if src_path no es un archivo regular.    
  if(!S_ISREG(buf_origen.st_mode)){
    return std::error_code(1, std::system_category());
  }

  struct stat buf_destino;
  stat(dst_path.c_str() ,&buf_destino);

  //if dst_path existe
  if(!S_ISDIR(buf_destino.st_mode)){
    return std::error_code(1, std::system_category());
  }
  
  //assert src_path y dst_path no son el mismo archivo.
  if( (buf_origen.st_dev == buf_destino.st_dev) && (buf_origen.st_ino == buf_destino.st_ino) ){
    return std::error_code(1, std::system_category());
  }

  std::string new_dst_path = dst_path;
  std::string filename = src_path.substr(src_path.find_last_of("/") + 1);

  //if dst_path es un directorio then    
  if(S_ISDIR(buf_destino.st_mode)){
    //Guardar en dst_path una nueva ruta con el nombre de archivo de src_path en el directorio dst_path
    new_dst_path = new_dst_path + filename;
  }

  int origen = open(src_path.c_str(), O_RDONLY);
  int destino = open(dst_path.c_str(), O_WRONLY | O_CREAT, 0666);
  
  if (destino < 0) {
    return std::error_code(1, std::system_category());
  }

  if (origen < 0) {
    return std::error_code(1, std::system_category());
  }

  std::vector<uint8_t> buffer(16ul * 1024 * 1024);
  ssize_t bytes_read; //valor arbitrario

  while (true) {
    bytes_read = read(origen, buffer.data(), buffer.size());
        
    if (bytes_read > 0) {
      buffer.resize(bytes_read);
      write(destino, buffer.data(), buffer.size());

      if (write(destino, buffer.data(), buffer.size()) != 0) {
        return std::error_code(0, std::system_category());
      }
    }

    if (bytes_read <= 0) {
      return std::error_code(1, std::system_category());
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
    unlink(src_path.c_str());
  }

  return std::error_code(0, std::system_category()); // Todo bien
}




std::error_code move(const std::string& src_path, const std::string& dst_path){

  // raise EACCES if src_path no existe.
  int origen = open(src_path.c_str(), O_RDONLY );
  if (origen == -1){
    return std::error_code(1, std::system_category());
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
      std::error_code error = copy_file(src_path, dst_path, true);

      if(error){
        return std::error_code(1, std::system_category());;
      }
      // Borrar src_path.
      unlink(src_path.c_str());
    } 
  return std::error_code(0, std::system_category());;
}