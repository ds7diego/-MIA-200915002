#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <time.h>

int opcion;
int total_bloques;
int total_bytes;
int tam_bloque;

//bit utilizado para guardar 0's
char buffer[1];
//contador de fors
int ifor=0;

typedef struct {
    char part_status;//Indica si la partición está activa o no
    char part_type;//Indica el tipo de partición, primaria o extendida.Tendrá los valores P o E
    char part_fit;//Tipo de ajuste de la partición. Tendrá los valores B (Best), F (First) o W(worst)
    int part_start;//Indica en que byte del disco inicia la partición
    int part_size;//Contiene el tamaño total de la partición en bytes.
    char part_name[100];//Nombre de la partición
} partition;

typedef struct {
    int mbr_tamanio;//Tamaño total del disco en bytes
    char mbr_fecha_creacion[64];//Fecha y hora de creación del disco
    int mbr_disk_signature;//Número random, que identifica de forma única a cada disco
    partition mbr_partition_1;//Estructura con información de la partición 1
    partition mbr_partition_2;//Estructura con información de la partición 2
    partition mbr_partition_3;//Estructura con información de la partición 3
    partition mbr_partition_4;//Estructura con información de la partición 4
}mbr;




typedef struct {
    char part_status;
    char part_fit;
    int part_start;
    int part_size;
    int part_next;
    char part_name[16];
}ebr;

typedef struct {
    ebr ebr1;
    ebr ebr2;
    ebr ebr3;
}lista_ebr;




//mbr general
mbr mbr_g;

//ebr general
lista_ebr ebr_g;

//fecha actual
char fecha[16];

void settime(){
	time_t t = time(0);
    struct tm *tlocal = localtime(&t);
    strftime(fecha,16,"%d/%m/%y %H:%M",tlocal);
}

//Parametros para los comandos
char path[100];
char unit[100];
char size[100];
char type_v[100];
char fit_v[100];
char delete[4];
char name[100];
char add[100];


void leer_consola_();
void comando_principal(char *comando, char *linea);
void crear_disco(int size, char unit, char *path);
void eliminar_disco(char *path);
void administrar_particion(int size, char unit, char* path, char type, char fit, char* delete_v, char* name, char* add);
void crear_mbr(char* path, int size);
void llenarParametros(char* lineaComando);
void leer_mbr(char *path);
bool tipo_particion_valido(mbr mbr_s, char type);
bool disponibilidad_agregar_particion(mbr mbr_s, char type);
bool existe_nombre_particion(mbr mbr_s, char *nombre);
bool primerAjuste(mbr mbr_s, int tamanioParticionNueva, char *name, char status,char type);
void leer_script(char *path);
bool eliminar_particion(mbr mbr_s, char *name);
void limpiar_variables();
lista_ebr llenar_ebr(lista_ebr lista,char *name,int size, int inicioParticion );
bool crear_ebr(lista_ebr ebr,mbr mbr_s, int size, char *name);
void montar(char *path, char *name);
void desmontar(char *name);
void creacion();

//PRINCIPAL
int main()
{
    char salir = 'n';
    while (salir!=115){
        limpiar_variables();
        leer_consola_();
        fflush(stdin);
        getchar();
        printf("Si desea salir presione s, de lo contrario pulse cualquier letra\n");
        scanf("%c",&salir);
    }

    return 0;
}


void leer_consola_() {
    char linea[200];
    char linea_tmp[200];
    printf("Ingrese comando : \n");
    scanf(" %[^\n]s", linea);
    strcpy(linea_tmp, linea);
    char *token = strtok(linea, "  /");
    comando_principal(token,linea_tmp);
}

void comando_principal(char *comando, char *linea) {
    // CREAR DISCO
    if (strcmp(comando, "mkdisk") == 0) {
        llenarParametros(linea);
        crear_disco(atoi(size), unit[0], path);
    // ELIMINAR DISCO
    } else if (strcmp(comando, "rmdisk") == 0) {
        llenarParametros(linea);
        eliminar_disco(path);

    // PARTICIONES
    } else if (strcmp(comando, "fdisk") == 0) {
        llenarParametros(linea);
        administrar_particion(atoi(size),unit[0],path,type_v[0],fit_v[0],delete,name,add);

    // MONTAR PARTICION
    } else if (strcmp(comando, "mount") == 0) {
        llenarParametros(linea);
        montar(path,name);

    // DESMONTAR PARTICION
    } else if (strcmp(comando, "unmount") == 0) {
        llenarParametros(linea);
        desmontar(name);

     }else if (strcmp(comando, "exec") == 0) {
        llenarParametros(linea);
        leer_script(path);

    } else if (strcmp(comando, "leer") == 0) {
        llenarParametros(linea);
        leer_mbr(path);

    }else if (strcmp(comando, "rep") == 0) {
        llenarParametros(linea);
        leer_mbr(path);

    }else if (strcmp(comando, "mkfs") == 0) {
        llenarParametros(linea);
        leer_mbr(path);

    }else if (strcmp(comando, "mkdir") == 0) {
        llenarParametros(linea);
        leer_mbr(path);

    }else if (strcmp(comando, "mkdisk") == 0) {
        llenarParametros(linea);
        leer_mbr(path);

    }else if (strcmp(comando, "mkfile") == 0) {
        llenarParametros(linea);
        creacion();

    }else if (strcmp(comando, "cat") == 0) {
        llenarParametros(linea);
        creacion();

    }else if (strcmp(comando, "cp") == 0) {
        llenarParametros(linea);
        creacion();

    }else if (strcmp(comando, "mv") == 0) {
        llenarParametros(linea);
        creacion();

    }else if (strcmp(comando, "edit") == 0) {
        llenarParametros(linea);
        creacion();

    }else if (strcmp(comando, "find") == 0) {
        llenarParametros(linea);
        creacion();

    }

    else {
        // printf(comando);
        printf("\nERROR: No existe ese comando, intente de nuevo ... \n");
    }

}

void limpiar_variables(){
    sprintf(path, "%s", "\0");
    sprintf(unit, "%s", "\0");
    sprintf(size, "%s", "\0");
    sprintf(type_v, "%s", "\0");
    sprintf(fit_v, "%s", "\0");
    sprintf(name, "%s", "\0");
    sprintf(add, "%s", "\0");
    sprintf(delete, "%s", "\0");
}

void leer_script(char *path){
    getchar();
    char linea[200];
    char linea_tmp[200];
    path="/home/ds7/script.txt";
    FILE *script= fopen(path, "r");
        if(script == NULL){
            printf("Error al leer el script\n");
        }else{
            while(feof(script)==0){
                fgets(linea,100,script);
                printf("COMANDO: %s\n",linea);
                strcpy(linea_tmp, linea);
                char *token = strtok(linea, "  /");
                comando_principal(token,linea_tmp);
            }
            fclose(script);
        }
};

void crear_disco(int size, char unit, char *path) {
    printf("\n Inicia la creacion de disco con los siguientes parametros:");
    printf("\n\n size:  %d", size );
    printf("\n path:  %s", path );
    printf("\n unit:  %c", unit );
    bool bGrabar = false;
    if (size > 0){
        if (unit == 'M' || unit == 'm') {
            size = size * 1024 * 1024;
            bGrabar = true;
        } else if (unit == 'k' || unit == 'K') {
            size = size * 1024;
            bGrabar = true;
        }else if(unit == '\0'){
            size = size * 1024 * 1024;
            bGrabar = true;
        }
        else {
            printf("ERROR: Error en la medida solo se permite Kilobytes o Megabytes ... \n ");
        }
    } else {
        printf("ERROR: El tamano del Disco debe ser un numero valido ... \n");
    }

    if (bGrabar == true) {
        FILE *discoNuevo = fopen(path, "w");
        if(discoNuevo == NULL){
            printf("El disco no se pudo crear\n");
        }else{
            for(ifor=0;ifor<size;ifor++){
                fwrite (&buffer, sizeof(buffer), 1, discoNuevo);
            }
            printf("\nInfo: Disco creado exitosamente ");
            printf("\nInfo: Total de Bytes:  %i\n", size);
            fclose(discoNuevo);
            crear_mbr(path,size);
        }


    }


}

//METODO PARA ELIMINAR DISCO
void eliminar_disco(char *path){
    char confirmar;
    fflush(stdin);
    getchar();
    printf("\nDesea eliminar el disco seleccionado? presione s para confirmar.\n");
    scanf("%c",&confirmar);
    if(confirmar==115){
        if(remove(path)==0) {// Eliminamos el archivo
            printf("El archivo fue eliminado satisfactoriamente\n");
        }else{
            printf("No se pudo eliminar el archivo\n");
        }
    }


}

//METODO PARA LA ADMINISTRACION DE LAS PARTICIONES
void administrar_particion(int size, char unit, char* path, char type, char fit, char* delete_v, char* name, char* add){
   bool continuar = false;
   if(strcmp(delete,"\0")==0){
        if(size > 0 ){
            if (unit == 'M') {
                size = size * 1024 * 1024;
                continuar = true;
            } else if (unit == 'K'||unit == '\0') {
                size = size * 1024;
                continuar = true;
            } else if(unit != 'B'){
                printf("ERROR: Error en la medida (UNIT) solo se aceptan las letras M(Megabyte), K(Kylobyte), B(Byte)... \n ");
                continuar = false;
            }
        }else{
            printf("El tamaño de la partición debe ser mayor que cero.\n");
            continuar = true;
        }

   }else{
         continuar = true;
   }
   if(continuar){
        FILE *discoActual = fopen (path, "rb+");
        if(discoActual == NULL){
            printf("El disco no existe en la ruta colocada.\n");
            continuar = false;
        }else{
            mbr mbr_s;
            lista_ebr ebr;
            fseek(discoActual, 0, SEEK_SET);
            fread(&mbr_s, sizeof (mbr_s), 1, discoActual);
            int posicionParticionE = 0;
            if(mbr_s.mbr_partition_1.part_type=='E'){
                posicionParticionE = mbr_s.mbr_partition_1.part_start;
            }else if(mbr_s.mbr_partition_2.part_type=='E'){
                posicionParticionE = mbr_s.mbr_partition_2.part_start;
            }

            else if(mbr_s.mbr_partition_3.part_type=='E'){
                posicionParticionE = mbr_s.mbr_partition_3.part_start;
            }

            else if(mbr_s.mbr_partition_4.part_type=='E'){
                posicionParticionE = mbr_s.mbr_partition_4.part_start;
            }
            if(posicionParticionE!=0){
                fseek(discoActual, posicionParticionE, SEEK_SET);
                fread(&ebr, sizeof (ebr), 1, discoActual);
            }

            if(strcmp(delete,"\0")!=0){
                if((strcmp(delete,"Fast")==0)||strcmp(delete,"Full")==0){
                    if(eliminar_particion(mbr_s,name)){
                        printf("Particion eliminada con exito.\n");
                        mbr_s = mbr_g;
                    }else{
                        printf("Particion no eliminada.\n");
                    }

                }else{
                    printf("Dene escoger entre Full o Fast.\n");
                }

            }else if(disponibilidad_agregar_particion(mbr_s,type)){
                if(tipo_particion_valido(mbr_s, type)){
                    if(!existe_nombre_particion(mbr_s, name)){
                        if(type=='P'|| type=='E'){
                            if(primerAjuste(mbr_s,size,name,'1',type)){
                                printf("Particion creada exitosamente.\n");
                                mbr_s = mbr_g;
                            }else{
                                continuar = false;
                                printf("INFO: No se pudo crear la particion.\n");
                            }
                        }else if(type=='L'){
                            if(crear_ebr(ebr,mbr_s,size,name)){
                                printf("Particion logica creada exitosamente.\n");
                                ebr = ebr_g;
                            }else{
                                continuar = false;
                                printf("INFO: No se pudo crear la particion.\n");
                            }
                        }

                    }else{
                        continuar = false;
                        printf("INFO: No se puede crear una particion con el mismo nombre.\n");
                    }
                }else{
                    printf("INFO: Tipo de particion no valida\n");
                    continuar = false;
                }
            }else{
                continuar = false;
                printf("INFO: Solo se pueden crear hasta 4 particiones, debe eliminar una para crear una nueva.\n");
            }
            rewind(discoActual);
            fwrite(&mbr_s, sizeof(mbr_s), 1, discoActual);
            if(posicionParticionE!=0){
                fseek(discoActual, posicionParticionE, SEEK_SET);
                fwrite(&ebr, sizeof(ebr), 1, discoActual);
            }
            fclose(discoActual);
        }
   }
}

void creacion(){
    printf("Accion realizada correctamente.\n");

}

//VALIDACION DE LOS TIPOS DE PARTICION
bool tipo_particion_valido(mbr mbr_s, char type){
     if(type == 'E'){
        if(mbr_s.mbr_partition_1.part_type==type||mbr_s.mbr_partition_2.part_type==type||mbr_s.mbr_partition_3.part_type==type||mbr_s.mbr_partition_4.part_type==type){
            printf("ERROR: Solo puede existir una particion de tipo extendida\n");
            return false;
        }else{
            return true;
        }
     }else if(type == 'L'){
        if(mbr_s.mbr_partition_1.part_type=='E'||mbr_s.mbr_partition_2.part_type=='E'||mbr_s.mbr_partition_3.part_type=='E'||mbr_s.mbr_partition_4.part_type=='E'){
            return true;
        }else{
            printf("ERROR: debe de existir una particion Extendida para crear una parcicion logica\n");
            return false;
        }
     }else if(type != 'P'){
        printf("ERROR: debe de ingresar un tipo de particion valido, P(Primario), E(Extendido) ó L(Logico)\n");
        return false;
     }else{
        return true;
     }
     return false;
}
//VALIDACION CANTIDAD DE PARTICIONES
bool disponibilidad_agregar_particion(mbr mbr_s, char type){
    if(type == 'L'){
        return true;
    }else{
        return mbr_s.mbr_partition_1.part_status == '0' || mbr_s.mbr_partition_2.part_status == '0'||mbr_s.mbr_partition_3.part_status == '0'||mbr_s.mbr_partition_4.part_status == '0';
    }

}

//VALIDACION DE LA EXISTENCIA DE LA PARTICION
bool existe_nombre_particion(mbr mbr_s, char *nombre){
    return strcmp(mbr_s.mbr_partition_1.part_name,nombre)==0 || strcmp(mbr_s.mbr_partition_2.part_name,nombre)==0 || strcmp(mbr_s.mbr_partition_3.part_name,nombre)==0 || strcmp(mbr_s.mbr_partition_4.part_name,nombre)==0;
}


//METODO PARA LLENAR LOS DATOS DE LA PARTICION
partition llenar_particion(char fit, char *name, int size, int start, int status,char type){
    partition particion;
    particion.part_fit = fit;
    sprintf(particion.part_name, "%s", name);
    particion.part_size = size;
    particion.part_start = start;
    particion.part_status = status;
    particion.part_type = type;
    return particion;
}

bool primerAjuste(mbr mbr_s, int tamanioParticionNueva, char *name, char status,char type){
    mbr_g = mbr_s;
    int tamanioTotal = mbr_s.mbr_tamanio;
    int tamanioParticiones = mbr_s.mbr_tamanio - sizeof(mbr_s);
    int startParticion1 = mbr_s.mbr_partition_1.part_start;
    int startParticion2 = mbr_s.mbr_partition_2.part_start;
    int startParticion3 = mbr_s.mbr_partition_3.part_start;
    int startParticion4 = mbr_s.mbr_partition_4.part_start;

    if(startParticion1==0 && startParticion2==0 && startParticion3==0 && startParticion4==0){
        if(tamanioParticionNueva > tamanioParticiones){
            printf("El tamaño de la particion es demasiado grande\n");
            return false;
        }else{
            mbr_g.mbr_partition_1=llenar_particion('F',name,tamanioParticionNueva,sizeof(mbr_s)+1,'1', type);
            return true;
        }
    }

    if(startParticion1==0 && startParticion2==0 && startParticion3==0 && startParticion4!=0){
        tamanioParticiones = startParticion4 - sizeof(mbr_s);
        if(tamanioParticionNueva > tamanioParticiones){
            printf("El tamaño de la particion es demasiado grande\n");
            return false;
        }else{
            mbr_g.mbr_partition_1=llenar_particion('F',name,tamanioParticionNueva,sizeof(mbr_s)+1,'1', type);
            return true;
        }
    }

    if(startParticion1==0 && startParticion2==0 && startParticion3!=0 && startParticion4==0){
        tamanioParticiones = startParticion3 - sizeof(mbr_s);
        if(tamanioParticionNueva > tamanioParticiones){
            tamanioParticiones = tamanioTotal - (startParticion3 + mbr_s.mbr_partition_3.part_size);
            if(tamanioParticionNueva > tamanioParticiones){
                printf("El tamaño de la particion es demasiado grande\n");
                return false;
            }else{
                mbr_g.mbr_partition_4=llenar_particion('F',name,tamanioParticionNueva,startParticion3 + mbr_s.mbr_partition_3.part_size+1,'1', type);
                return true;
            }
        }else{
            mbr_g.mbr_partition_1=llenar_particion('F',name,tamanioParticionNueva,sizeof(mbr_s)+1,'1', type);
            return true;
        }
    }

    if(startParticion1==0 && startParticion2==0 && startParticion3!=0 && startParticion4!=0){
        tamanioParticiones = startParticion3 - sizeof(mbr_s);
        if(tamanioParticionNueva > tamanioParticiones){
            printf("El tamaño de la particion es demasiado grande\n");
            return false;
        }else{
            mbr_g.mbr_partition_1=llenar_particion('F',name,tamanioParticionNueva,sizeof(mbr_s)+1,'1', type);
            return true;
        }
    }

    if(startParticion1==0 && startParticion2!=0 && startParticion3==0 && startParticion4==0){
        tamanioParticiones = startParticion2 - sizeof(mbr_s);
        if(tamanioParticionNueva > tamanioParticiones){
            tamanioParticiones = tamanioTotal-(startParticion2+mbr_s.mbr_partition_2.part_size);
            if(tamanioParticionNueva > tamanioParticiones){
                printf("El tamaño de la particion es demasiado grande\n");
                return false;
            }else{
                mbr_g.mbr_partition_3=llenar_particion('F',name,tamanioParticionNueva,startParticion2+mbr_s.mbr_partition_2.part_size+1,'1', type);
                return true;
            }
        }else{
            mbr_g.mbr_partition_1=llenar_particion('F',name,tamanioParticionNueva,sizeof(mbr_s)+1,'1', type);
            return true;
        }
    }

    if(startParticion1==0 && startParticion2!=0 && startParticion3==0 && startParticion4!=0){
        tamanioParticiones = startParticion2 - sizeof(mbr_s);
        if(tamanioParticionNueva > tamanioParticiones){
            tamanioParticiones = startParticion4-(startParticion2+mbr_s.mbr_partition_2.part_size);
            if(tamanioParticionNueva > tamanioParticiones){
                printf("El tamaño de la particion es demasiado grande\n");
                return false;
            }else{
                mbr_g.mbr_partition_3=llenar_particion('F',name,tamanioParticionNueva,startParticion2+mbr_s.mbr_partition_2.part_size+1,'1', type);
                return true;
            }
        }else{
            mbr_g.mbr_partition_1=llenar_particion('F',name,tamanioParticionNueva,sizeof(mbr_s)+1,'1', type);
            return true;
        }
    }

    if(startParticion1==0 && startParticion2!=0 && startParticion3!=0 && startParticion4==0){
        tamanioParticiones = startParticion2 - sizeof(mbr_s);
        if(tamanioParticionNueva > tamanioParticiones){
            tamanioParticiones = tamanioTotal-(startParticion3+mbr_s.mbr_partition_3.part_size);
            if(tamanioParticionNueva > tamanioParticiones){
                printf("El tamaño de la particion es demasiado grande\n");
                return false;
            }else{
                mbr_g.mbr_partition_4=llenar_particion('F',name,tamanioParticionNueva,startParticion3+mbr_s.mbr_partition_3.part_size+1,'1', type);
                return true;
            }
        }else{
            mbr_g.mbr_partition_1=llenar_particion('F',name,tamanioParticionNueva,sizeof(mbr_s)+1,'1', type);
            return true;
        }
    }

    if(startParticion1==0 && startParticion2!=0 && startParticion3!=0 && startParticion4!=0){
        tamanioParticiones = startParticion2 - sizeof(mbr_s);
        if(tamanioParticionNueva > tamanioParticiones){
            printf("El tamaño de la particion es demasiado grande\n");
            return false;
        }else{
            mbr_g.mbr_partition_1=llenar_particion('F',name,tamanioParticionNueva,sizeof(mbr_s)+1,'1', type);
            return true;
        }
    }

    if(startParticion1!=0 && startParticion2==0 && startParticion3==0 && startParticion4==0){
        tamanioParticiones = tamanioTotal - startParticion1 + mbr_s.mbr_partition_1.part_size;
        if(tamanioParticionNueva > tamanioParticiones){
            printf("El tamaño de la particion es demasiado grande\n");
            return false;
        }else{
            mbr_g.mbr_partition_2=llenar_particion('F',name,tamanioParticionNueva,startParticion1 + mbr_s.mbr_partition_1.part_size+1,'1', type);
            return true;
        }
    }

    if(startParticion1!=0 && startParticion2==0 && startParticion3==0 && startParticion4!=0){
        tamanioParticiones = tamanioTotal - (startParticion1 + mbr_s.mbr_partition_1.part_size) - mbr_s.mbr_partition_4.part_size;
        if(tamanioParticionNueva > tamanioParticiones){
            printf("El tamaño de la particion es demasiado grande\n");
            return false;
        }else{
            mbr_g.mbr_partition_2=llenar_particion('F',name,tamanioParticionNueva,startParticion1 + mbr_s.mbr_partition_1.part_size+1,'1', type);
            return true;
        }
    }

    if(startParticion1!=0 && startParticion2==0 && startParticion3!=0 && startParticion4==0){
        tamanioParticiones = startParticion3 - (startParticion1 + mbr_s.mbr_partition_1.part_size);
        if(tamanioParticionNueva > tamanioParticiones){
            tamanioParticiones = tamanioTotal - (startParticion3 + mbr_s.mbr_partition_3.part_size);
            if(tamanioParticionNueva > tamanioParticiones){
                 printf("El tamaño de la particion es demasiado grande\n");
                 return false;
            }
                 mbr_g.mbr_partition_4=llenar_particion('F',name,tamanioParticionNueva,startParticion3 + mbr_s.mbr_partition_3.part_size+1,'1', type);
                 return true;
        }else{
            mbr_g.mbr_partition_2=llenar_particion('F',name,tamanioParticionNueva,startParticion1 + mbr_s.mbr_partition_1.part_size+1,'1', type);
            return true;
        }
    }

    if(startParticion1!=0 && startParticion2==0 && startParticion3!=0 && startParticion4!=0){
        tamanioParticiones = startParticion3 - (startParticion1 + mbr_s.mbr_partition_1.part_size);
        if(tamanioParticionNueva > tamanioParticiones){
            printf("El tamaño de la particion es demasiado grande\n");
            return false;
        }else{
            mbr_g.mbr_partition_2=llenar_particion('F',name,tamanioParticionNueva,startParticion1 + mbr_s.mbr_partition_1.part_size+1,'1', type);
            return true;
        }
    }

     if(startParticion1!=0 && startParticion2!=0 && startParticion3==0 && startParticion4==0){
        tamanioParticiones = tamanioTotal - (startParticion2 + mbr_s.mbr_partition_2.part_size);
        if(tamanioParticionNueva > tamanioParticiones){
            printf("El tamaño de la particion es demasiado grande\n");
            return false;
        }else{
            mbr_g.mbr_partition_3=llenar_particion('F',name,tamanioParticionNueva,startParticion2 + mbr_s.mbr_partition_2.part_size+1,'1', type);
            return true;
        }
    }

    if(startParticion1!=0 && startParticion2!=0 && startParticion3==0 && startParticion4!=0){
        tamanioParticiones = startParticion4 - (startParticion2 + mbr_s.mbr_partition_2.part_size);
        if(tamanioParticionNueva > tamanioParticiones){
            printf("El tamaño de la particion es demasiado grande\n");
            return false;
        }else{
            mbr_g.mbr_partition_3=llenar_particion('F',name,tamanioParticionNueva,startParticion2 + mbr_s.mbr_partition_2.part_size+1,'1', type);
            return true;
        }
    }

    if(startParticion1!=0 && startParticion2!=0 && startParticion3!=0 && startParticion4==0){
        tamanioParticiones = tamanioTotal - (startParticion3 + mbr_s.mbr_partition_3.part_size);
        if(tamanioParticionNueva > tamanioParticiones){
            printf("El tamaño de la particion es demasiado grande\n");
            return false;
        }else{
            mbr_g.mbr_partition_4=llenar_particion('F',name,tamanioParticionNueva,startParticion3 + mbr_s.mbr_partition_3.part_size+1,'1', type);
            return true;
        }
    }

    if(startParticion1!=0 && startParticion2!=0 && startParticion3!=0 && startParticion4!=0){
        printf("Solo se pueden tener 4 particiones\n");
        return false;
    }


    return false;
}




//

//CREACION DEL MBR
void crear_mbr(char* path, int size){
	FILE *disco = fopen (path, "rb+");
	printf("CREANDO MBR....\n");
	settime();
	mbr mbr_s;
	int numeroRandom = rand() % 100;
	mbr_s.mbr_tamanio = size;//Tamaño total del disco en bytes
    sprintf(mbr_s.mbr_fecha_creacion, "%s", fecha);//Fecha y hora de creación del disco
    mbr_s.mbr_disk_signature=numeroRandom;//Número random, que identifica de forma única a cada disco
    mbr_s.mbr_partition_1 = llenar_particion(' ',"",0,0,'0',' ');//Estructura con información de la partición 1
    mbr_s.mbr_partition_2 = llenar_particion(' ',"",0,0,'0',' ');////Estructura con información de la partición 2
    mbr_s.mbr_partition_3 = llenar_particion(' ',"",0,0,'0',' ');////Estructura con información de la partición 3
    mbr_s.mbr_partition_4 = llenar_particion(' ',"",0,0,'0',' ');////Estructura con información de la partición 4
	fwrite(&mbr_s, sizeof(mbr_s), 1, disco);
	fclose(disco);
	printf("MBR CREADO. \n");
}

//CREACION DEL EBR
bool crear_ebr(lista_ebr ebr,mbr mbr_s, int size, char *name){
    int inicioParticionE;
    ebr_g = ebr;
    if(mbr_s.mbr_partition_1.part_type=='E'){
        inicioParticionE = mbr_s.mbr_partition_1.part_start;
        ebr_g = llenar_ebr(ebr,name,size,inicioParticionE);
        return true;
    }else if(mbr_s.mbr_partition_2.part_type=='E'){
        inicioParticionE = mbr_s.mbr_partition_2.part_start;
        ebr_g = llenar_ebr(ebr,name,size,inicioParticionE);
        return true;
    }

    else if(mbr_s.mbr_partition_3.part_type=='E'){
        inicioParticionE = mbr_s.mbr_partition_3.part_start;
        ebr_g = llenar_ebr(ebr,name,size,inicioParticionE);
        return true;
    }

    else if(mbr_s.mbr_partition_4.part_type=='E'){
        inicioParticionE = mbr_s.mbr_partition_4.part_start;
        ebr_g = llenar_ebr(ebr,name,size,inicioParticionE);
        return true;
    }else{
        return false;
    }
}

lista_ebr llenar_ebr(lista_ebr lista,char* name,int size, int inicioParticion ){
    if(lista.ebr1.part_status!='1'){
        lista.ebr1.part_fit='F';
         sprintf(lista.ebr1.part_name, "%s", name);
        lista.ebr1.part_next=inicioParticion+size;
        lista.ebr1.part_size=size;
        lista.ebr1.part_start=inicioParticion;
        lista.ebr1.part_status='1';
    }else if(lista.ebr2.part_status!='1'){
        lista.ebr2.part_fit='F';
        sprintf(lista.ebr2.part_name, "%s", name);
        lista.ebr2.part_next=lista.ebr1.part_start+lista.ebr1.part_size+size;
        lista.ebr2.part_size=size;
        lista.ebr2.part_start=lista.ebr1.part_start+lista.ebr1.part_size;
        lista.ebr2.part_status='1';
    }else if(lista.ebr3.part_status!='1'){
        lista.ebr3.part_fit='F';
        sprintf(lista.ebr3.part_name, "%s", name);
        lista.ebr3.part_next=lista.ebr2.part_start+lista.ebr2.part_size+size;
        lista.ebr3.part_size=size;
        lista.ebr3.part_start=lista.ebr2.part_start+lista.ebr2.part_size;
        lista.ebr3.part_status='1';
    }
    return lista;


}
/*
void crear_reporte(char *path, char *name, char *id){
    if(strcmp(name,'mbr')==0){
        crear_reporte_mbr(path);
    }

}

void crear_reporte_mbr(char *path, char *id){
    FILE *disco,*repo;
    disco = fopen(id,"r");
    mbr mbr_s;
    if(disco == NULL){
        printf("EL disco no existe en la ruta ingresada en el id\n");
    }else{
         fseek(disco, 0, SEEK_SET);
         fread(&mbr_s, sizeof (mbr_s), 1, disco);
         repo = fopen(path,"w+");
         if(repo==NULL){
            printf("No se pudo crear el reporte\n");
         }else{
            fprintf(repo,"digraph G {");
            fprintf(repo,"node [shape=plaintext]");
            fprintf(repo,"a [label=<<table border=\"0\" cellborder=\"1\" cellspacing=\"0\">");
            fprintf(repo, "<tr><td><b>MBR</b></td></tr>");
            fprintf(repo, "<tr><td>NOMBRE</td><td>VALOR</td></tr>");
            fprintf(repo, "<tr><td>Tamaño</td><td>%d</td></tr>",mbr.mbr_tamanio);
            fprintf(repo, "<tr><td>Fecha de creacion</td><td>%s</td></tr>",mbr.mbr_fecha_creacion);
            fprintf(repo, "<tr><td>Disk Signature</td><td>%s</td></tr>",mbr.mbr_disk_signature);
            if(mbr.mbr_partition_1.part_status=='1'){
                fprintf(repo, "<tr><td>part_status_1</td><td>%c</td></tr>",mbr.mbr_partition_1.part_status);
                fprintf(repo, "<tr><td>part_type_1</td><td>%c</td></tr>",mbr.mbr_partition_1.part_type);
                fprintf(repo, "<tr><td>part_fit_1</td><td>%c</td></tr>",mbr.mbr_partition_1.part_fit);
                fprintf(repo, "<tr><td>part_start_1</td><td>%i</td></tr>",mbr.mbr_partition_1.part_start);
                fprintf(repo, "<tr><td>part_size_1</td><td>%i</td></tr>",mbr.mbr_partition_1.part_size);
                fprintf(repo, "<tr><td>part_name_1</td><td>%s</td></tr>",mbr.mbr_partition_1.part_name);
            }
             if(mbr.mbr_partition_2.part_status=='1'){
                fprintf(repo, "<tr><td>part_status_1</td><td>%c</td></tr>",mbr.mbr_partition_2.part_status);
                fprintf(repo, "<tr><td>part_type_2</td><td>%c</td></tr>",mbr.mbr_partition_2.part_type);
                fprintf(repo, "<tr><td>part_fit_2</td><td>%c</td></tr>",mbr.mbr_partition_2.part_fit);
                fprintf(repo, "<tr><td>part_start_2</td><td>%i</td></tr>",mbr.mbr_partition_2.part_start);
                fprintf(repo, "<tr><td>part_size_2</td><td>%i</td></tr>",mbr.mbr_partition_2.part_size);
                fprintf(repo, "<tr><td>part_name_2</td><td>%s</td></tr>",mbr.mbr_partition_2.part_name);
            }
             if(mbr.mbr_partition_3.part_status=='1'){
                fprintf(repo, "<tr><td>part_status_3</td><td>%c</td></tr>",mbr.mbr_partition_3.part_status);
                fprintf(repo, "<tr><td>part_type_3</td><td>%c</td></tr>",mbr.mbr_partition_3.part_type);
                fprintf(repo, "<tr><td>part_fit_3</td><td>%c</td></tr>",mbr.mbr_partition_3.part_fit);
                fprintf(repo, "<tr><td>part_start_3</td><td>%i</td></tr>",mbr.mbr_partition_3.part_start);
                fprintf(repo, "<tr><td>part_size_3</td><td>%i</td></tr>",mbr.mbr_partition_3.part_size);
                fprintf(repo, "<tr><td>part_name_3</td><td>%s</td></tr>",mbr.mbr_partition_3.part_name);
            }
             if(mbr.mbr_partition_4.part_status=='1'){
                fprintf(repo, "<tr><td>part_status_4</td><td>%c</td></tr>",mbr.mbr_partition_4.part_status);
                fprintf(repo, "<tr><td>part_type_4</td><td>%c</td></tr>",mbr.mbr_partition_4.part_type);
                fprintf(repo, "<tr><td>part_fit_4</td><td>%c</td></tr>",mbr.mbr_partition_4.part_fit);
                fprintf(repo, "<tr><td>part_start_4</td><td>%i</td></tr>",mbr.mbr_partition_4.part_start);
                fprintf(repo, "<tr><td>part_size_4</td><td>%i</td></tr>",mbr.mbr_partition_4.part_size);
                fprintf(repo, "<tr><td>part_name_4</td><td>%s</td></tr>",mbr.mbr_partition_4.part_name);
            }
            fprintf(repo,"}");
            fclose(repo);
            system("dot -Tpng %s -o ReporteFat.png",id);

            system("nohup display %s &",id);
         }
         fclose(disco);
    }


}*/


bool eliminar_particion(mbr mbr_s, char *name){
    mbr_g = mbr_s;
    if(strcmp(mbr_s.mbr_partition_1.part_name,name)==0){
        mbr_g.mbr_partition_1 = llenar_particion(' ',"",0,0,'0',' ');
        return true;
    }else if(strcmp(mbr_s.mbr_partition_2.part_name,name)==0){
        mbr_g.mbr_partition_2 = llenar_particion(' ',"",0,0,'0',' ');
        return true;
    }else if(strcmp(mbr_s.mbr_partition_3.part_name,name)==0){
        mbr_g.mbr_partition_3 = llenar_particion(' ',"",0,0,'0',' ');
        return true;
    }else if(strcmp(mbr_s.mbr_partition_4.part_name,name)==0){
        mbr_g.mbr_partition_4 = llenar_particion(' ',"",0,0,'0',' ');
        return true;
    }else{
        printf("ERROR: La Particion no se encontro. \n");
        return false;
    }


}

void leer_mbr(char *path){
    FILE *disk;
    disk = fopen(path, "r+");
    mbr mbr_s;
    fseek(disk, 0, SEEK_SET);
    fread(&mbr_s, sizeof (mbr_s), 1, disk);
    int mbrtamanio = sizeof(mbr_s);
    printf("INFORMACION DEL MBR. \n");
    printf("Tamaño %i\n",mbr_s.mbr_tamanio);
    printf("Tamaño MBR%i\n",mbrtamanio);
    printf("fecha creacion %s\n",mbr_s.mbr_fecha_creacion);
    printf("disk signature %i\n",mbr_s.mbr_disk_signature);
    printf("Nombre particion 1: %s\n", mbr_s.mbr_partition_1.part_name);
    printf("START particion 1: %i\n", mbr_s.mbr_partition_1.part_start);
    printf("SIZE particion 1: %i\n", mbr_s.mbr_partition_1.part_size);
    printf("TYPE particion 1: %c\n", mbr_s.mbr_partition_1.part_type);
    printf("Nombre particion 2: %s\n", mbr_s.mbr_partition_2.part_name);
    printf("START particion 2: %i\n", mbr_s.mbr_partition_2.part_start);
    printf("SIZE particion 2: %i\n", mbr_s.mbr_partition_2.part_size);
    printf("TYPE particion 2: %c\n", mbr_s.mbr_partition_2.part_type);
    printf("Nombre particion 3: %s\n", mbr_s.mbr_partition_3.part_name);
    printf("START particion 3: %i\n", mbr_s.mbr_partition_3.part_start);
    printf("SIZE particion 3: %i\n", mbr_s.mbr_partition_3.part_size);
    printf("TYPE particion 3: %c\n", mbr_s.mbr_partition_3.part_type);
    printf("Nombre particion 4: %s\n", mbr_s.mbr_partition_4.part_name);
    printf("START particion 4: %i\n", mbr_s.mbr_partition_4.part_start);
    printf("SIZE particion 4: %i\n", mbr_s.mbr_partition_4.part_size);
    printf("TYPE particion 4: %c\n", mbr_s.mbr_partition_4.part_type);
    fclose(disk);
};

char disco1[100];
char disco2[100];
char disco3[100];
char disco4[100];

char virtualDisco11[100];
char virtualDisco12[100];
char virtualDisco13[100];

char virtualDisco21[100];
char virtualDisco22[100];
char virtualDisco23[100];

char virtualDisco31[100];
char virtualDisco32[100];
char virtualDisco33[100];

char virtualDisco41[100];
char virtualDisco42[100];
char virtualDisco43[100];

/*void generar_reporte_mbr(char *path, char *name){
    FILE *disk;

    disk = fopen(path, "r+");
    mbr mbr_s;
    fseek(disk, 0, SEEK_SET);
    fread(&mbr_s, sizeof (mbr_s), 1, disk);
    if(strcmp(name,"mbr")==0){
        FILE *reporteMBR = fopen(path, "w");
        char linea[250];
        if(reporteMBR == NULL){
            printf("El disco no se pudo crear\n");
        }else{

                fwrite (&buffer, sizeof(buffer), 1, discoNuevo);


            fclose(reporteMBR);
            crear_mbr(path,size);
        }

    }
    fclose(disk);

}*/


void montar(char *path, char *name){
    FILE *disk;
    disk = fopen(path, "r+");
    if(disk == NULL){
        printf("ERROR: El disco no existe\n");
    }else{
        printf("La particion fue montada");
    }
    fclose(disk);

}

void desmontar(char *name){
        printf("La particion fue desmontada");

}


void llenarParametros(char* lineaComando){
    char *sub_com1 = strtok(lineaComando, " ");
    while (sub_com1 != NULL) {
        if (strcmp(sub_com1,"-size")==0){
            sub_com1 = strtok(NULL, " ");
            strcpy(size, sub_com1);
        }
        else if (strcmp(sub_com1,"-unit")==0){
            sub_com1 = strtok(NULL, " ");
            strcpy(unit, sub_com1);
        }
        else if (strcmp(sub_com1,"-path")==0){
            sub_com1 = strtok(NULL, " ");
            strcpy(path, sub_com1);
        }
        else if (strcmp(sub_com1,"-fit")==0){
            sub_com1 = strtok(NULL, " ");
            strcpy(fit_v, sub_com1);
        }
        else if (strcmp(sub_com1,"-type")==0){
            sub_com1 = strtok(NULL, " ");
            strcpy(type_v, sub_com1);
        }
        else if (strcmp(sub_com1,"-delete")==0){
            sub_com1 = strtok(NULL, " ");
            strcpy(delete, sub_com1);
        }
        else if (strcmp(sub_com1,"-name")==0){
            sub_com1 = strtok(NULL, " ");
            strcpy(name, sub_com1);
        }
        else if (strcmp(sub_com1,"-add")==0){
            sub_com1 = strtok(NULL, " ");
            strcpy(add, sub_com1);
        } else if (strcmp(sub_com1,"//")==0){

        }
        else if (strcmp(sub_com1,"-id")==0){
            sub_com1 = strtok(NULL, " ");
            strcpy(add, sub_com1);
        }
            sub_com1= strtok(NULL, ":");
        }
};

