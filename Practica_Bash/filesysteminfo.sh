#!/bin/bash

# Alummno: Eduardo González Gutiérrez
# Correo Institucional: alu0101461588@ull.edu.es
# Asignatura: Sistemas Operativos
# filesysteminfo - Un script que informa del estado del sistema

# -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------

# CONSTANTES
TITLE="Información de los sistemas montados en $HOSTNAME"
RIGHT_NOW=$(date +"%x %r%Z")
TIME_STAMP="Actualizada el $RIGHT_NOW por $USER"

# ESTILO DEL TEXTO
TEXT_BOLD=$(tput bold)
TEXT_RED=$(tput setaf 1)
TEXT_BLUE=$(tput setaf 32)
TEXT_GREEN=$(tput setaf 2)
TEXT_ULINE=$(tput sgr 0 1)
TEXT_RESET=$(tput sgr0)

# VARIABLES
mount_devices=$(mount|cut -d ' ' -f 5|sort|uniq)
header_text=${TEXT_BOLD}"DEVICE           TYPE                               USED            MOUNT_POINT                                        NºDEVICES T_USED     MAJOR MINOR"${TEXT_RESET}
header_devices=${TEXT_BOLD}"DEVICE         TYPE       USED   MOUNT_POINT  NºDEVICES  MAJOR MiNOR ESPACE_SUM OPENED"${TEXT_RESET}
header_user=${TEXT_BOLD}"DEVICE         TYPE       USED   MOUNT_POINT    NºDEVICES MAJOR MINOR ESPACE_SUM OPENED OPENED_USER"${TEXT_RESET}
header=0
filesdevice=
inv=
s_open=
s_device=
ordenar="sort -n -k1,1"
iteraciones=
useroption=


# FUNCIÓN DE USO DEL SISTEMA
function Usage() {
cat << _EOF_
  ${TEXT_BOLD}${TEXT_BLUE}Script que muestra información acerca del sistema de archivos montado en el sistema ${TEXT_RESET}
  Para ejecutarlo debe introducir: ${TEXT_BOLD}${TEXT_GREEN}./filesysteminfo [-h|--help] [-inv] para invertir${TEXT_RESET}
  ${TEXT_ULINE}Parametros${TEXT_RESET}
   --help - Muestra la ventana de ayuda.
    -inv - Invierte la salida del comando.
    -u [usuario/s] - Obtiene la tabla de aquellos usuarios que han sido introducidos
    -devicefiles - Obtiene la tabla correspondiente a los dispositivos representados por el S.O como archivos
    -sopen - Se hace la ordenación por el número de archivos abiertos (esta función solo se puede emplear con -devicefiles o -u)
    -sdevice - La ordenación se realizará por el número total de dispositivos considerados para cada sistema de archivos.
_EOF_
}

#FUNCIÓN PARA LA GESTIÓN DE ERRORES EN FUNCIÓN DE LOS PARÁMETROS INTRODUCIDOS POR EL USUARIO
function ControlErrores() {
  if [ "$s_device" == "1" ] && [ "$s_open" == "1" ]; then
    echo "${TEXT_BOLD}[-]OPCIÓN NO VÁLIDA${TEXT_RESET}"
    echo "NO SE PUEDE EMPLEAR LA FUNCIÓN -sopen Y -sdevice AL MISMO TIEMPO"
    exit 1
  elif [ "$s_open" == "1" ] && [ "$filesdevice" != "1" ] && [ "$useroption" != "1" ]; then
    echo "${TEXT_BOLD}[-]OPCIÓN NO VÁLIDA${TEXT_RESET}"
    echo "-sopen SOLO SE PUEDE EMPLEAR CON -devicefiles O -u"
    exit 1
  fi
}


# FUNCIÓN PARA QUITAR EL CABECERA CUANDO SE INTRODUCE EL PARÁMETRO -noheader
function Header() {
  if [ "$header" == "0" ]; then
    if [ "$useroption" == "1" ]; then
      echo "$header_user"
    
    elif [ "$filesdevice" == "1" ]; then
      echo "$header_devices"
  
    else
      echo  "$header_text"
    fi
  fi
}


# FUNCIÓN MONTAJE DE LOS DISPOSITIVOS DEL SISTEMA
function MountDevices() {
  echo -n ""
  Header
  for device in $mount_devices; do
    total_devices=$((df -at $device |tail -n +2|wc -l) 2>/dev/null)
    info_devices=$((df --all --human-readable -t $device | tail -n +2 | sort -nr -k3,3 | head -n +1 | awk '{ print $1"\t" $3"\t" $6"\t" }') 2>/dev/null)
    sum_used_space=$((df -at $device | tr -s ' ' | cut -d ' ' -f3 | tail -n+2 | awk '{s+=$1} END {printf "%.0f", s}') 2>/dev/null)
    minor=$(stat --format="%T" $(df --all --human-readable -t $device --output=source| tail -n +2 | sort -k'2' --human-numeric-sort -r | head -n +1) 2>/dev/null || echo "*")
    major=$(stat --format="%t" $(df --all --human-readable -t $device --output=source| tail -n +2 | sort -k'2' --human-numeric-sort -r | head -n +1) 2>/dev/null || echo "*")

    print="$device $info_devices $total_devices $sum_used_space $major $minor"

    if [ "$filesdevice" == "1" ]; then 
      device_files=$(stat --format="%t %T" $(df --all --human-readable -t $device --output=source| tail -n +2 | sort -k'2' --human-numeric-sort -r | head -n +1) 2>/dev/null)
      if [ "$?" == "0" ];then
        mounted_names=$(df -at $device --output=source,used,target 2>/dev/null | tail -n +2 | cut -d ' ' -f1 | head -n +1)
        lsof_var=$(lsof $mounted_names 2>/dev/null | wc -l) 
        print_device="$device $info_devices $total_devices $device_files $lsof_var"

        if [ "$useroption" == "1" ]; then
          for user in $usuario; do
            lsof_user=
            wc_user=
            
            lsof_user="$(lsof -au $user $mounted_names 2>/dev/null | wc -l)"
            wc_user=$((wc_user + lsof_user))
          done 

          echo -n "$print_device "
          echo "$wc_user"
        else
          echo "$print_device "
        fi
      fi
    else
      echo "$print"
    fi

  done | $ordenar $inv | column -t 
}



# PROGRAMA PRINCIPAL
while [ "$1" != "" ];do
  case "$1" in
    -h | --help)
		  Usage
        exit 0
        ;;
    -inv)
		  inv="-r"
      ;;
    -devicefiles)
      filesdevice=1
      ;;
    -u)
      iteraciones=1
      filesdevice=1
      useroption=1
      ;;
    -noheader)
      header=1
      ;;
    -sopen)
      s_open=1
      ordenar="sort -n -k8,8"
      ;;
    -sdevice)
      s_device=1
      ordenar="sort -n -k5,5"
      ;;
    *)

    if [ "$iteraciones" == "1" ]; then
      if [ ${1:0:1} != "-" ]; then
        usuario="$usuario $1"
      fi
    else
      ${TEXT_BOLD}[-]OPCIÓN NO VÁLIDA${TEXT_RESET}
      cat << _EOF_
_EOF_
      Usage
      exit 1
    fi
    ;;
  esac
  shift
done

ControlErrores  # LLAMADA A LA FUNCIÓN PARA EL CONTROL DE ERRORES

MountDevices  # LLAMADA A LA FUNCIÓN PRINCIPAL DEL PROGRAMA MountDevices