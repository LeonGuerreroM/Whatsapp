#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<sys/select.h>
#include<stdio.h>

struct usuario{
    int id;
    char nombre[30];
    long telefono;
    char estado[80];
    long contactos[100];
    int num_contactos;
};

struct grupo{
    int id;
    char nombre[30];
    int id_admin;
    long integrantes[50];
    int num_integrantes;
};

struct usuario *usuarios = NULL;
int num_usuarios = 0;
int id_usuario_sesion[] = {-1,-1,-1,-1,-1,-1,-1,-1}; //arreglos de ids de usuarios conectados
int usuarios_conectados = 0; 
FILE *archivo = NULL; 
char despliegue[1000];
char texto[70];
char emisor[30];
char usuario_con_sesion[30];

struct grupo *grupos = NULL;
int num_grupos = 0;
FILE *archivo2 = NULL; 
char muestra_grupos[2000];
char whatsGrupal[100];

#define PUERTO 6020
#define COLA_CLIENTES 8
#define TAM_BUFFER 100
//#define maximo(a,b) ((a) > (b) ? (a) : (b))
int maximo(int a, int b, int c, int d, int e, int f, int g, int h);

//FUNCIONES DE ESTRUCTURA usuario
int alta(char nombre[], long telefono);
void guardar();
void llenar_estructura();
void imprimir_estructura();

//FUNCIONES DE ESTRUCTURA grupo
void altaGrupos(char nombre[], int id_admin, long telefono_admin);
void guardarGrupos();
void llenar_estructura_grupos();
void imprimir_estructura_grupos();



//FUNCIONES DE APLICACION
int agregarTelefono(char leer_mensaje[]);
int iniciar_sesion(char leer_mensaje[], int indice_cliente);
int agregarContacto(char leer_mensaje[], int indice_cliente);
int listar_contactos(char leer_mensaje[], int indice_cliente);
int enviarMensaje(char leer_mensaje[], int indice_cliente);
void atenderCliente(char mensaje[], int indice_cliente);

int crearGrupo(char leer_mensaje[], int indice_cliente);
int aniadirUsuarioAGrupo(char leer_mensaje[], int indice_cliente);
int borrarUsuarioDeGrupo(char leer_mensaje[], int indice_cliente);
void listarGruposPorUsuario(int indice_cliente);
int enviarMensajeGrupo(char leer_mensaje[], int indice_cliente);
void enviarMsj(int indice_cliente);
void salir();


int sockfd;//, cliente_sockfd1, cliente_sockfd2, cliente_sockfd3;
int cliente_sockfd[COLA_CLIENTES];
void main(int argc, char **argv){
    int tamano_direccion;
    struct sockaddr_in direccion_servidor;
    fd_set conjunto_leer;
    int maximo_fd;
    char mensaje[TAM_BUFFER];

    //int comando, retorno;
    int i;

    archivo = fopen("usuarios.db","rb");
    if(archivo != NULL){
        fread(&num_usuarios,sizeof(int),1,archivo); //inicializacion de la variable global num_usuarios
        printf("Hay %d usuarios registrados\n",num_usuarios);
    }else{
        printf("Problema al abrir el archivo\n");
    }
    llenar_estructura(); //llenado de la estructura

    archivo2 = fopen("grupos.db","rb");
    if(archivo2 != NULL){
        fread(&num_grupos,sizeof(int),1,archivo2); //inicializacion de la variable global num_usuarios
        printf("Hay %d grupos registrados\n",num_grupos);
    }else{
        printf("Problema al abrir el archivo\n");
    }
    llenar_estructura_grupos();

    //Se configura la dirección IPv4 para establecer el socket
    memset (&direccion_servidor, 0, sizeof (direccion_servidor));
    direccion_servidor.sin_family = AF_INET;
    direccion_servidor.sin_port = htons(PUERTO);
    direccion_servidor.sin_addr.s_addr = INADDR_ANY;

    //Limpia el conjunto de lectura
    FD_ZERO(&conjunto_leer);
    //Creación de las estructuras necesarias para el manejo de un socket
    printf("Creando Socket ....\n");
    if ( (sockfd = socket (AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Ocurrió un problema en la creación del socket");
        exit(-1);
    }

    printf("Configurando socket ...\n");
    if (bind(sockfd, (struct sockaddr *) &direccion_servidor,sizeof(direccion_servidor)) < 0){
        perror ("Ocurrión un problema al configurar el socket");
        exit(-1);
    }

    printf ("Estableciendo la aceptación de clientes...\n");
    if ( listen(sockfd, COLA_CLIENTES) < 0){
        perror("Ocurrión un problema al crear la cola de aceptar peticiones de los clientes");
        exit(-1);
    }

    for(i=0;i<COLA_CLIENTES;i++){
        printf ("En espera de ingreso cliente %d ...\n",i+1);
        if ( (cliente_sockfd[i] = accept(sockfd, NULL, NULL)) < 0){
            perror("Ocurrió algún problema al atender a un cliente");
            exit(-1);
        }
    }

    for(i=0;i<COLA_CLIENTES;i++){
        FD_SET(cliente_sockfd[i], &conjunto_leer);
    }

    maximo_fd = maximo (cliente_sockfd[0], cliente_sockfd[1], cliente_sockfd[2], cliente_sockfd[3], cliente_sockfd[4], cliente_sockfd[5], cliente_sockfd[6], cliente_sockfd[7]);
    maximo_fd = maximo_fd + 1;
    while (1){

        for(i=0;i<COLA_CLIENTES;i++){
            FD_SET(cliente_sockfd[i], &conjunto_leer);
        }

        maximo_fd = maximo (cliente_sockfd[0], cliente_sockfd[1], cliente_sockfd[2], cliente_sockfd[3], cliente_sockfd[4], cliente_sockfd[5], cliente_sockfd[6], cliente_sockfd[7]);
        maximo_fd = maximo_fd + 1;
        
        if ( select (maximo_fd, &conjunto_leer, NULL, NULL, NULL) < 0){
            perror ("Ocurrió un problema dentro de la función select");
            for(i=0;i<COLA_CLIENTES;i++){
                close (cliente_sockfd[i]);
            }
            close (sockfd);
        }

        if ( FD_ISSET(cliente_sockfd[0], &conjunto_leer)) {
            if (read(cliente_sockfd[0], &mensaje, TAM_BUFFER) < 0) {
                perror("Ocurrio algun problema al recibir datos del cliente");
                exit(-1);
            }
            atenderCliente(mensaje, 0);
        }

        if ( FD_ISSET(cliente_sockfd[1], &conjunto_leer)) {
            if (read(cliente_sockfd[1], &mensaje, TAM_BUFFER) < 0) {
                perror("Ocurrio algun problema al recibir datos del cliente");
                exit(-1);
            }
            atenderCliente(mensaje, 1);
        }

        if ( FD_ISSET(cliente_sockfd[2], &conjunto_leer)) {
            if (read(cliente_sockfd[2], &mensaje, TAM_BUFFER) < 0) {
                perror("Ocurrio algun problema al recibir datos del cliente");
                exit(-1);
            }
            atenderCliente(mensaje, 2);
        }

        if ( FD_ISSET(cliente_sockfd[3], &conjunto_leer)) {
            if (read(cliente_sockfd[3], &mensaje, TAM_BUFFER) < 0) {
                perror("Ocurrio algun problema al recibir datos del cliente");
                exit(-1);
            }
            atenderCliente(mensaje, 3);
        }

        if ( FD_ISSET(cliente_sockfd[4], &conjunto_leer)) {
            if (read(cliente_sockfd[4], &mensaje, TAM_BUFFER) < 0) {
                perror("Ocurrio algun problema al recibir datos del cliente");
                exit(-1);
            }
            atenderCliente(mensaje, 4);
        }

        if ( FD_ISSET(cliente_sockfd[5], &conjunto_leer)) {
            if (read(cliente_sockfd[5], &mensaje, TAM_BUFFER) < 0) {
                perror("Ocurrio algun problema al recibir datos del cliente");
                exit(-1);
            }
            atenderCliente(mensaje, 5);
        }

        if ( FD_ISSET(cliente_sockfd[6], &conjunto_leer)) {
            if (read(cliente_sockfd[6], &mensaje, TAM_BUFFER) < 0) {
                perror("Ocurrio algun problema al recibir datos del cliente");
                exit(-1);
            }
            atenderCliente(mensaje, 6);
        }

        if ( FD_ISSET(cliente_sockfd[7], &conjunto_leer)) {
            if (read(cliente_sockfd[7], &mensaje, TAM_BUFFER) < 0) {
                perror("Ocurrio algun problema al recibir datos del cliente");
                exit(-1);
            }
            atenderCliente(mensaje, 7);
        }
    }
    close (sockfd);
}



int alta(char nombre[], long telefono){
    int i = 0;
    int encontrado = 0;


    usuarios = realloc(usuarios,sizeof(struct usuario)*(num_usuarios+1));
    memset(&usuarios[num_usuarios],0,sizeof(struct usuario));
    

    for(i=0;i<num_usuarios;i++){
        if(usuarios[i].telefono == telefono){
            encontrado = 1;
        }
    }
    if(encontrado == 1){
        printf("El número %ld ya está registrado\n",telefono);
        return 0;
    }else{
        printf("Hay %d usuarios ya\n",num_usuarios);
        usuarios[num_usuarios].id = num_usuarios;
        strcpy(usuarios[num_usuarios].nombre,nombre);
        usuarios[num_usuarios].telefono = telefono;
        strcpy(usuarios[num_usuarios].estado,"Desconectado");
        usuarios[num_usuarios].num_contactos = 0;
        num_usuarios = num_usuarios + 1;
        guardar();
        return 1;
    }
    
}

void guardar(){
    archivo = fopen("usuarios1.db","wb");
    if(archivo != NULL){
        fwrite(&num_usuarios,sizeof(int),1,archivo);
        fwrite(usuarios,sizeof(struct usuario),num_usuarios,archivo);
        fclose(archivo);
        //printf("se supone que está guardado\n");
    }else{
        printf("Error al abrir el archivo\n");
    }
}

void llenar_estructura(){
    archivo = fopen("usuarios1.db","rb");
    if(archivo != NULL){
        fread(&num_usuarios,sizeof(int),1,archivo);
        usuarios = calloc(sizeof(struct usuario),num_usuarios);
        fread(usuarios,sizeof(struct usuario),num_usuarios,archivo);
        fclose(archivo);
    }else{
        printf("Error al abrir el archivo\n");
    }
}

void imprimir_estructura(){
    int i = 0;
    int j = 0;

    //printf("EL valor de num_usr %d\n",num_usuarios);
    while(i<num_usuarios){
        printf("\n");
        printf("Registro: %d",usuarios[i].id);
        printf("\n");
        printf("Nombre: %s", usuarios[i].nombre);
        //printf("\n");
        printf("telefono: %ld",usuarios[i].telefono);
        printf("\n");
        printf("Estado: %s",usuarios[i].estado);
        printf("\n");
        for(j=0;j<100;j++){
            if(usuarios[i].contactos[j] == '\0'){
                continue;
            }
            printf("Contacto %d: %ld\n",j,usuarios[i].contactos[j]);
        }
            
        i++;
    }

    printf("----------------------------\n");
    for(i=0;i<COLA_CLIENTES;i++){
        printf("El registro %d es el id de usuario %d\n",i,id_usuario_sesion[i]);
    }
}

int agregarTelefono(char leer_mensaje[]){
    char telefonoA[10];
    long telefono;
    char nombre[30];
    int i = 0;
    int isRegistered;
    
    for(i = 1; i<11;i++){
        /*if(leer_mensaje[i] == '\0'){
            return 0;
        }*/
        telefonoA[i-1] = leer_mensaje[i]; //recupera los caracteres del telefono
    }
    printf("El telefono recibido es: "); //imprime el telefono
    for(i = 0; i<10;i++){
        printf("%c",telefonoA[i]);
    }
    printf("\n");
    telefono = atol(telefonoA);
    for(i = 11; i<41;i++){
        nombre[i-11] = leer_mensaje[i]; //recupera los caracteres del telefono
    }
    printf("El numero de telefono recibido es: %ld",telefono); //imprime el telefono
    printf("\n");
    printf("El nombre recibido es: "); //imprime el telefono
    for(i = 0; i<30;i++){
        if(nombre[i] != '\0'){
            printf("%c",nombre[i]);
        }
    }
    printf("\n");
    isRegistered = alta(nombre,telefono);

    return isRegistered;

}

int iniciar_sesion(char leer_mensaje[], int indice_cliente){
    long telefono;
    char telefonoA[10];
    int i, indice_registro;

    for(i=0;i<30;i++){
        usuario_con_sesion[i] = '\0';
    }

    for(i = 1; i<11;i++){
        //printf("leer_mensaje[%d]: %d\n",i,leer_mensaje[i]-'0');
        if(leer_mensaje[i] == '\0'){
            printf("Número de teléfono no valido\n");
            return -1;
        }
        telefonoA[i-1] = leer_mensaje[i]; //recupera los caracteres del telefono
    }
    printf("El telefono recibido es: "); //imprime el telefono
    for(i = 0; i<10;i++){
        printf("%c",telefonoA[i]);
    }
    printf("\n");
    telefono = atol(telefonoA);

    for(i=0;i<num_usuarios;i++){  //comprueba que el telefono esté registrado
        if(usuarios[i].telefono == telefono){
            strcpy(usuarios[i].estado,"Activo");
            id_usuario_sesion[indice_cliente] = i;
            usuarios_conectados++;
            indice_registro = id_usuario_sesion[indice_cliente];
            strcpy(usuario_con_sesion,usuarios[indice_registro].nombre);
            printf("Usuario con sesion: %s",usuario_con_sesion);
            return 1;
        }
    }
    return -10; //solo llega aqui si no lo encontró, asi que no está registrado

}

int agregarContacto(char leer_mensaje[], int indice_cliente){
    int indice_registro;
    char telefonoA[10];
    long telefono;
    int i, encontrado = -1;

    indice_registro = id_usuario_sesion[indice_cliente];

    //recupera qué usuario es el que trata de agregar (Mejora a recuperación automática con socket)
    for(i = 1; i<11;i++){
        if(leer_mensaje[i] == '\0'){
            return 0;
        }
        telefonoA[i-1] = leer_mensaje[i]; //recupera los caracteres del telefono
    }
    printf("El telefono recibido es: "); //imprime el telefono
    for(i = 0; i<10;i++){
        printf("%c",telefonoA[i]);
    }
    printf("\n");
    telefono = atol(telefonoA);

    //revisa que el teléfono esté registrado en la lista general antes de añadirlo a los contactos de un usuario
    for(i=0;i<num_usuarios;i++){
        if(usuarios[i].telefono == telefono){
            encontrado = 1;
        }
    }

    if(encontrado!=1){
        printf("Número proporcionado no registrado\n");
        return -10;
    }

    usuarios[indice_registro].contactos[usuarios[indice_registro].num_contactos] = telefono;
    usuarios[indice_registro].num_contactos = usuarios[indice_registro].num_contactos + 1;
    guardar();
    printf("Usuario registrado en contactos\n");

    return 1;
}

int listar_contactos(char leer_mensaje[], int indice_cliente){
    //int indice_usuario, i, j, indice_registro;
    int i, j, indice_registro;

    for(i=0;i<1000;i++){
        despliegue[i] = '\0';
    }

    indice_registro = id_usuario_sesion[indice_cliente];

    printf("Desplegando contactos de %s\n",usuarios[indice_registro].nombre);

    for(i=0;i<usuarios[indice_registro].num_contactos;i++){
        char sContacto[] = "***";
        char enter[] = "\n";
        //char puntitos[] = ": ";
        //char numero[2];
        char numero2[2];
        char nombre[30];
        char guion[] = " - ";
        char guion2[] = " - ";
        char estado[] = " estado: ";
        char estado_descripcion[80];

        strcat(despliegue,sContacto);
        //sprintf(numero, "%d", i+1);
        //strcat(despliegue, numero);
        //for(j=0;j<2;j++){
          //  numero[j] = '\0';
        //}
        //strcat(despliegue, puntitos);
        for(j=0;j<num_usuarios;j++){
            if(usuarios[indice_registro].contactos[i] == usuarios[j].telefono){
                strcpy(nombre,usuarios[j].nombre);
            }
        }
        strcat(despliegue,nombre);
        strcat(despliegue,guion);
        sprintf(numero2, "%ld", usuarios[indice_registro].contactos[i]);
        strcat(despliegue, numero2);
        for(j=0;j<2;j++){
            numero2[j] = '\0';
        }
        strcat(despliegue,guion2);
        strcat(despliegue,estado);
        for(j=0;j<num_usuarios;j++){
            if(usuarios[indice_registro].contactos[i] == usuarios[j].telefono){
                strcpy(estado_descripcion,usuarios[j].estado);
            }
        }
        strcat(despliegue,estado_descripcion);
        strcat(despliegue,enter);
        printf("Contacto %d: %ld\n",i,usuarios[indice_registro].contactos[i]);
    }

}

int enviarMensaje(char leer_mensaje[], int indice_cliente){
    //int indice_usuario, indice_registro;
    int indice_registro, indice_receptor_en_estructura;
    int indice_receptor_en_sockets;
    char telefonoA[10];
    long telefono;
    int i, encontrado = -1; 

    for(i=0;i<70;i++){ //limpiamos los buffers generales de texto y emisor
        if(i<30){
            emisor[i] = '\0';
        }
        texto[i] = '\0';
    }

    indice_registro = id_usuario_sesion[indice_cliente];

    strcpy(emisor,usuarios[indice_registro].nombre);

    //recupera qué usuario es el que trata de agregar (Mejora a recuperación automática con socket)
    for(i = 1; i<11;i++){
        if(leer_mensaje[i] == '\0'){
            return -2;
        }
        telefonoA[i-1] = leer_mensaje[i]; //recupera los caracteres del telefono
    }
    printf("El telefono recibido es: "); //imprime el telefono
    for(i = 0; i<10;i++){
        printf("%c",telefonoA[i]);
    }
    printf("\n");
    telefono = atol(telefonoA);

    for(i=0;i<usuarios[indice_registro].num_contactos;i++){ //revisa que el teléfono esté en contactos
        if(usuarios[indice_registro].contactos[i] == telefono){
            encontrado = 1;
        }
    }

    for(i=0;i<num_usuarios;i++){ //
        if(usuarios[i].telefono == telefono){
            indice_receptor_en_estructura = i; //obtiene cual es el indice de registro del receptor
            printf("indice_receptor_en_estructura = %d\n",indice_receptor_en_estructura);
            if(strcmp(usuarios[i].estado,"Activo") != 0){
                return -5;
            }
        }
    }

    if(encontrado != 1){
        printf("Telefono no agregado a contactos\n");
        return -3;
    }

    if(leer_mensaje[11]=='\0'){
        return -4;
    }

    for(i = 11; i<81;i++){
        if(leer_mensaje[i]=='\0'){ //comprueba si aún hay algo ahi
            break;
        } 
        texto[i-11] = leer_mensaje[i]; //recupera los caracteres del mensaje a enviar 
    } 

    for(i=0;i<COLA_CLIENTES;i++){
        if(id_usuario_sesion[i] == indice_receptor_en_estructura){ //a partir del indice de registro, obtiene el indice de su socket o pos en sesion iniciada porque son lo mismo
            indice_receptor_en_sockets = i;
            printf("indice_receptor_en_socket = %d\n",indice_receptor_en_sockets);
            return indice_receptor_en_sockets;
        }
    }

    return -10; //si llega aqui es que el indice de registro no se encontró en los usuarios consesion iniciada

}

void atenderCliente(char mensaje[], int indice_cliente){
    int comando, retorno, i, alta_baja, outa;

    comando = mensaje[0]-'0';
    if(comando == 1){
        retorno = agregarTelefono(mensaje);
        if(retorno == 1){
            if(write(cliente_sockfd[indice_cliente], "Registro Realizado", 19) < 0){
                perror("Ocurrio un problema en el envio de un mensaje al cliente");
                exit(1);
            }
        }else if(retorno == 0){
            if(write(cliente_sockfd[indice_cliente], "Registro fallido, numero duplicado", 40) < 0){
                perror("Ocurrio un problema en el envio de un mensaje al cliente");
                exit(1);
            }
        }
    }else if(comando == 0){
        outa = mensaje[1] - '0';
        if(outa == 0){
            salir();
        }else{
            imprimir_estructura();
            printf("*******GRUPOS********\n");
            imprimir_estructura_grupos();
            if(write(cliente_sockfd[indice_cliente], "Lista Desplegada", 19) < 0){
                perror("Ocurrio un problema en el envio de un mensaje al cliente");
                exit(1);
            }
        }  
    }else if(comando == 2){
        printf("Cliente va a iniciar sesión\n");
        retorno = iniciar_sesion(mensaje, indice_cliente);
        if(retorno == -10){
            if(write(cliente_sockfd[indice_cliente], "El número no está dado de alta", 40) < 0){
                perror("Ocurrio un problema en el envio de un mensaje al cliente");
                exit(1);
            }
        }else if(retorno == -1){
            if(write(cliente_sockfd[indice_cliente], "Registro fallido, número no válido", 40) < 0){
                perror("Ocurrio un problema en el envio de un mensaje al cliente");
                exit(1);
            }
        }else if(retorno == 1){
            //char frase[] = "Sesion iniciada, tu identificador es: ";
            char frase[] = "Sesion iniciada como ";
            //char frase2[2];
            //sprintf(frase2, "%d", indice_cliente);
            strcat(frase,usuario_con_sesion);
            if(write(cliente_sockfd[indice_cliente], frase, strlen(frase)+1) < 0){
                perror("Ocurrio un problema en el envi0o de un mensaje al cliente");
                exit(1);
            }
        }
    }else if(comando == 3){
        retorno = agregarContacto(mensaje, indice_cliente);
        if(retorno == 0){
            if(write(cliente_sockfd[indice_cliente], "Número de telefono no valido", 40) < 0){
                perror("Ocurrio un problema en el envio de un mensaje al cliente");
                exit(1);
            }
        }else if(retorno == -1){
            if(write(cliente_sockfd[indice_cliente], "Indice de usuario no valido", 40) < 0){
                perror("Ocurrio un problema en el envio de un mensaje al cliente");
                exit(1);
            }
        }else if(retorno == -10){
            if(write(cliente_sockfd[indice_cliente], "Telefono no registrado", 40) < 0){
                perror("Ocurrio un problema en el envio de un mensaje al cliente");
                exit(1);
            }
        }else if(retorno == 1){
            if(write(cliente_sockfd[indice_cliente], "Contacto agregado\n", 40) < 0){
                perror("Ocurrio un problema en el envio de un mensaje al cliente");
                exit(1);
            }
        }
    }else if(comando == 4){
        listar_contactos(mensaje, indice_cliente);
        if(write(cliente_sockfd[indice_cliente], despliegue, strlen(despliegue)+1) < 0){
            perror("Ocurrio un problema en el envio de un mensaje al cliente");
            exit(1);
        }
    }else if(comando == 5){
        retorno = enviarMensaje(mensaje, indice_cliente);
        if(retorno == -2){
            if(write(cliente_sockfd[indice_cliente], "Número de telefono no valido", 40) < 0){
                perror("Ocurrio un problema en el envio de un mensaje al cliente");
                exit(1);
            }
        }else if(retorno == -1){
            if(write(cliente_sockfd[indice_cliente], "Indice de usuario no valido", 40) < 0){
                perror("Ocurrio un problema en el envio de un mensaje al cliente");
                exit(1);
            }
        }else if(retorno == -3){
            if(write(cliente_sockfd[indice_cliente], "Telefono no registrado en contactos", 40) < 0){
                perror("Ocurrio un problema en el envio de un mensaje al cliente");
                exit(1);
            }
        }else if(retorno == -4){
            if(write(cliente_sockfd[indice_cliente], "No se añadió un mensaje para enviar", 40) < 0){
                perror("Ocurrio un problema en el envio de un mensaje al cliente");
                exit(1);
            }
        }else if(retorno == -5){
            if(write(cliente_sockfd[indice_cliente], "El contacto no está conectado", 40) < 0){
                perror("Ocurrio un problema en el envio de un mensaje al cliente");
                exit(1);
            }
        }else if(retorno == -10){
            if(write(cliente_sockfd[indice_cliente], "Indice no encontrado en sockets", 40) < 0){
                perror("Ocurrio un problema en el envio de un mensaje al cliente");
                exit(1);
            }
        }else{
            char whats[100];
            for(i=0;i<100;i++){
                whats[i] = '\0';
            }
            strcat(whats,emisor);
            strcat(whats,texto);
            if(write(cliente_sockfd[retorno], whats, strlen(whats)+1) < 0){
                perror("Ocurrio un problema en el envio de un mensaje al cliente");
                exit(1);
            }
        }
    }else if(comando == 6){
        retorno = crearGrupo(mensaje, indice_cliente);
        if(retorno == -1){
            if(write(cliente_sockfd[indice_cliente], "No se ha ingresado nombre", 40) < 0){
                perror("Ocurrio un problema en el envio de un mensaje al cliente");
                exit(1);
            }
        }else{
            char frase3[] = "Grupo creado, su identificador es: ";
            char frase4[5];
            sprintf(frase4, "%d", retorno);
            strcat(frase3,frase4);
            
            if(write(cliente_sockfd[indice_cliente], frase3, strlen(frase3)+1) < 0){
                perror("Ocurrio un problema en el envio de un mensaje al cliente");
                exit(1);
            }
        }
    }else if(comando == 7){
        alta_baja = mensaje[1] -'0';
        if(alta_baja == 1){
            retorno = aniadirUsuarioAGrupo(mensaje, indice_cliente);
            if(retorno == -1){
                if(write(cliente_sockfd[indice_cliente], "No eres el administrado del grupo", 40) < 0){
                    perror("Ocurrio un problema en el envio de un mensaje al cliente");
                    exit(1);
                }
            }else if(retorno == -2){
                if(write(cliente_sockfd[indice_cliente], "Teléfono no válido", 40) < 0){
                    perror("Ocurrio un problema en el envio de un mensaje al cliente");
                    exit(1);
                }
            }else if(retorno == -3){
                if(write(cliente_sockfd[indice_cliente], "Teléfono no agregado a contactos", 40) < 0){
                    perror("Ocurrio un problema en el envio de un mensaje al cliente");
                    exit(1);
                }
            }else if(retorno == -4){
                if(write(cliente_sockfd[indice_cliente], "Usuario previamente agregado al grupo", 40) < 0){
                    perror("Ocurrio un problema en el envio de un mensaje al cliente");
                    exit(1);
                }
            }else if(retorno == 1){
                if(write(cliente_sockfd[indice_cliente], "Contacto añadido al grupo :)", 40) < 0){
                    perror("Ocurrio un problema en el envio de un mensaje al cliente");
                    exit(1);
                }
            }
        }else if(alta_baja == 0){
            retorno = borrarUsuarioDeGrupo(mensaje, indice_cliente);
            if(retorno == -1){
                if(write(cliente_sockfd[indice_cliente], "No eres el administrado del grupo", 40) < 0){
                    perror("Ocurrio un problema en el envio de un mensaje al cliente");
                    exit(1);
                }
            }else if(retorno == -2){
                if(write(cliente_sockfd[indice_cliente], "Teléfono no válido", 40) < 0){
                    perror("Ocurrio un problema en el envio de un mensaje al cliente");
                    exit(1);
                }
            }else if(retorno == -3){
                if(write(cliente_sockfd[indice_cliente], "Contacto no agregado al grupo", 40) < 0){
                    perror("Ocurrio un problema en el envio de un mensaje al cliente");
                    exit(1);
                }
            }else if(retorno == 1){
                if(write(cliente_sockfd[indice_cliente], "Contacto eliminado del grupo 2B)", 40) < 0){
                    perror("Ocurrio un problema en el envio de un mensaje al cliente");
                    exit(1);
                }
            }
        }
    }else if(comando == 8){
        listarGruposPorUsuario(indice_cliente);
        if(write(cliente_sockfd[indice_cliente], muestra_grupos, strlen(muestra_grupos)+1) < 0){
            perror("Ocurrio un problema en el envio de un mensaje al cliente");
            exit(1);
        }
    }else if(comando == 9){
        retorno = enviarMensajeGrupo(mensaje, indice_cliente);
        if(retorno == -1){
            if(write(cliente_sockfd[indice_cliente], "No perteneces a ese grupo", 40) < 0){
                perror("Ocurrio un problema en el envio de un mensaje al cliente");
                exit(1);
            }
        } 
    }
}

int maximo(int a, int b, int c, int d, int e, int f, int g, int h){
    int mayor;
    
    mayor = a;
    if(mayor<b){
        mayor = b;
    }
    if(mayor<c){
        mayor = c;
    }
    if(mayor<d){
        mayor = d;
    }
    if(mayor<e){
        mayor = e;
    }
    if(mayor<f){
        mayor = f;
    }
    if(mayor<g){
        mayor = g;
    }
    if(mayor<h){
        mayor = h;
    }

    return mayor;
}

void altaGrupos(char nombre[], int id_admin, long telefono_admin){

    grupos = realloc(grupos,sizeof(struct grupo)*(num_grupos+1));
    memset(&grupos[num_grupos],0,sizeof(struct grupo));
    
    printf("Hay %d grupos ya\n",num_grupos);
    grupos[num_grupos].id = num_grupos;
    strcpy(grupos[num_grupos].nombre,nombre);
    grupos[num_grupos].id_admin = id_admin;
    grupos[num_grupos].num_integrantes = 0;
    grupos[num_grupos].integrantes[grupos[num_grupos].num_integrantes] = telefono_admin;
    grupos[num_grupos].num_integrantes = grupos[num_grupos].num_integrantes + 1;
    num_grupos = num_grupos + 1;
    guardarGrupos();
    
}

void guardarGrupos(){
    archivo2 = fopen("grupos.db","wb");
    if(archivo2 != NULL){
        fwrite(&num_grupos,sizeof(int),1,archivo2);
        fwrite(grupos,sizeof(struct grupo),num_grupos,archivo2);
        fclose(archivo2);
    }else{
        printf("Error al abrir el archivo\n");
    }
}

void llenar_estructura_grupos(){
    archivo2 = fopen("grupos.db","rb");
    if(archivo2 != NULL){
        fread(&num_grupos,sizeof(int),1,archivo2);
        grupos = calloc(sizeof(struct grupo),num_grupos);
        fread(grupos,sizeof(struct grupo),num_grupos,archivo2);
        fclose(archivo2);
    }else{
        printf("Error al abrir el archivo\n");
    }
}

void imprimir_estructura_grupos(){
    int i = 0;
    int j = 0;

    while(i<num_grupos){
        printf("\n");
        printf("ID: %d",grupos[i].id);
        printf("\n");
        printf("Nombre: %s", grupos[i].nombre);
        printf("ID_Admin: %d",grupos[i].id_admin);
        printf("\n");
        printf("Num Integrantes: %d",grupos[i].num_integrantes);
        printf("\n");
        for(j=0;j<50;j++){
            if(grupos[i].integrantes[j] == '\0'){
                continue;
            }
            printf("Integrante %d: %ld\n",j,grupos[i].integrantes[j]);
        }
            
        i++;
    }
}

int crearGrupo(char leer_mensaje[], int indice_cliente){
    int i = 0;
    int indice_registro;
    char nombre[30];

    indice_registro = id_usuario_sesion[indice_cliente]; //indice_cliente es el socket
    
    if(leer_mensaje[1] == '\0'){
        return -1; //Si no se agregó nombre de grupo
    }

    for(i = 1; i<31;i++){
        nombre[i-1] = leer_mensaje[i]; //recupera el nombre
    }

    printf("El nombre recibido es: "); //imprime el telefono
    for(i = 0; i<30;i++){
        if(nombre[i] != '\0'){
            printf("%c",nombre[i]);
        }
    }
    printf("\n");
    altaGrupos(nombre,indice_registro,usuarios[indice_registro].telefono);

    return num_grupos-1;

}


int aniadirUsuarioAGrupo(char leer_mensaje[], int indice_cliente){
    int indice_registro, id_grupo, i, encontrado = -1, encontradoG = -1;
    char telefonoA[10];
    long telefono;

    indice_registro = id_usuario_sesion[indice_cliente]; //indice_cliente es el socket

    id_grupo = leer_mensaje[2] - '0';
    if(indice_registro != grupos[id_grupo].id_admin){
        return -1; //No eres el administrador de ese grupo
    }

    for(i = 3; i<13;i++){
        if(leer_mensaje[i] == '\0'){ //por si el telefono está incompleto 
            return -2;
        }
        telefonoA[i-3] = leer_mensaje[i]; //recupera los caracteres del telefono
    }
    printf("El telefono recibido es: "); //imprime el telefono
    for(i = 0; i<10;i++){
        printf("%c",telefonoA[i]);
    }
    printf("\n");
    telefono = atol(telefonoA);

    for(i=0;i<usuarios[indice_registro].num_contactos;i++){ //revisa que el teléfono esté en contactos
        if(usuarios[indice_registro].contactos[i] == telefono){
            encontrado = 1;
        }
    }

    if(encontrado != 1){
        printf("Telefono no agregado a contactos\n");
        return -3; //el telefono no esta en los contactos 
    }

    for(i=0;i<grupos[id_grupo].num_integrantes;i++){
        if(grupos[id_grupo].integrantes[i] == telefono){
            encontradoG = 1;
        }
    }

    if(encontradoG == 1){
        return -4; //ya está en el grupo
    }

    grupos[id_grupo].integrantes[grupos[id_grupo].num_integrantes] = telefono;
    grupos[id_grupo].num_integrantes = grupos[id_grupo].num_integrantes + 1;
    guardarGrupos();
    printf("Contacto añadido al grupo\n");
    return 1;

}

int borrarUsuarioDeGrupo(char leer_mensaje[], int indice_cliente){
    int indice_registro, id_grupo, i, encontrado = -1, encontradoG = -1;
    char telefonoA[10];
    long telefono;

    indice_registro = id_usuario_sesion[indice_cliente]; //indice_cliente es el socket

    id_grupo = leer_mensaje[2] - '0';
    if(indice_registro != grupos[id_grupo].id_admin){
        return -1; //No eres el administrador de ese grupo
    }

    for(i = 3; i<13;i++){
        if(leer_mensaje[i] == '\0'){ //por si el telefono está incompleto 
            return -2;
        }
        telefonoA[i-3] = leer_mensaje[i]; //recupera los caracteres del telefono
    }
    printf("El telefono recibido es: "); //imprime el telefono
    for(i = 0; i<10;i++){
        printf("%c",telefonoA[i]);
    }
    printf("\n");
    telefono = atol(telefonoA);

    for(i=0;i<grupos[id_grupo].num_integrantes;i++){ //recorrerá los integrantes del grupo
        if(grupos[id_grupo].integrantes[i] == telefono){ //cuando lo encuentra...
            grupos[id_grupo].integrantes[i] = 0; //pone a o su número
            if(i+1 == grupos[id_grupo].num_integrantes){
                grupos[id_grupo].num_integrantes--;
            }
            guardarGrupos();
            return 1;
        }
    }

    return -3; //Si llegamos aquí, es que no lo encontró

}

void listarGruposPorUsuario(int indice_cliente){
    int indice_registro,i,j,k,l;
    long telefono_usuario;
    int grupos_verificados[20];
    int contador_gv = 0, destacado, id_grupo_enCurso;
    long integranteTemp;

    char jump[] = "\n\n";


    for(i=0;i<2000;i++){
        muestra_grupos[i] = '\0';
    }

    indice_registro = id_usuario_sesion[indice_cliente];
    telefono_usuario = usuarios[indice_registro].telefono;

    printf("Desplegando grupos de %s\n",usuarios[indice_registro].nombre);

    for(i=0;i<num_grupos;i++){
        for(j=0;j<grupos[i].num_integrantes;j++){
            if(grupos[i].integrantes[j] == telefono_usuario){
                //printf("akitoy\n");
                grupos_verificados[contador_gv] = grupos[i].id; //obtenemos una lista de los grupos en los que aparece la persona
                contador_gv++;
            }
        }
    }

    /*printf("Grupos destacados");
    for(i=0;i<contador_gv;i++){
        printf("%d ", grupos_verificados[i]);
    }
    printf("\n");*/

    for(i=0;i<contador_gv;i++){
        destacado = grupos_verificados[i];
        //printf("destacado %d\n",destacado);
        for(j=0;j<num_grupos;j++){
            id_grupo_enCurso = grupos[j].id;
            if(destacado == id_grupo_enCurso){
                //printf("id_grupos en curso coincidente %d\n",id_grupo_enCurso);
                char titulo_idgrup[] = "ID Grupo: ";
                char enter[] = "\n";
                char idgrup[5];
                sprintf(idgrup, "%d", grupos[id_grupo_enCurso].id);
                strcat(muestra_grupos,titulo_idgrup);
                strcat(muestra_grupos,idgrup);
                strcat(muestra_grupos,enter);
                char titulo_grupo[] = "Grupo: ";
                strcat(muestra_grupos,titulo_grupo);
                char nombre_grup[30];
                strcpy(nombre_grup,grupos[id_grupo_enCurso].nombre);
                strcat(muestra_grupos,nombre_grup);
                char tipo[] = "Tipo de Usuario: ";
                strcat(muestra_grupos,tipo);
                char titulo_integrantes[] = "INTEGRANTES \n";
                //verifica si es admin o miembro;
                if(grupos[id_grupo_enCurso].id_admin == indice_registro){
                    char titulo_admin[] = "Administrador\n";
                    strcat(muestra_grupos,titulo_admin);
                    strcat(muestra_grupos,titulo_integrantes);
                }else{
                    char titulo_miembro[] = "Miembro\n";
                    strcat(muestra_grupos,titulo_miembro);
                    strcat(muestra_grupos,titulo_integrantes);
                }

                for(k=0;k<grupos[id_grupo_enCurso].num_integrantes;k++){
                    if(grupos[id_grupo_enCurso].integrantes[k] == telefono_usuario){
                        continue;
                    }else{
                        char telInteg[20];
                        char integ[] = "*";
                        char guion[] = " - ";
                        integranteTemp = grupos[id_grupo_enCurso].integrantes[k];
                        sprintf(telInteg, "%ld", integranteTemp);
                        strcat(muestra_grupos,integ);
                        for(l=0;l<num_usuarios;l++){
                            if(usuarios[l].telefono == integranteTemp){
                                //recupera nombre
                                char nombreInteg[30];
                                strcpy(nombreInteg, usuarios[l].nombre);    
                                strcat(muestra_grupos,telInteg);
                                strcat(muestra_grupos,guion);
                                strcat(muestra_grupos,nombreInteg);
                            }
                        }
                    }
                }

            }
        }
        strcat(muestra_grupos,jump);
    }
}

int enviarMensajeGrupo(char leer_mensaje[], int indice_cliente){
    int grupoReceptor, indice_registro;
    long telefono_emisor, telefonoTemporal, idIntegranteTemporal;
    int encontrado = -1, i, j, k;
    char remitente[30];
    char grupoOrigen[30];
    char textoGrupal[70];
    

    for(i=0;i<100;i++){
        whatsGrupal[i] = '\0';
    }


    grupoReceptor = leer_mensaje[1]-'0';

    strcpy(grupoOrigen,grupos[grupoReceptor].nombre);

    indice_registro = id_usuario_sesion[indice_cliente];

    telefono_emisor = usuarios[indice_registro].telefono;

    strcpy(remitente,usuarios[indice_registro].nombre);

    for(i=0;i<grupos[grupoReceptor].num_integrantes;i++){ //comprueba que se encuentre en el grupo
        if(telefono_emisor == grupos[grupoReceptor].integrantes[i]){
            encontrado = 1;
        }
    }

    if(encontrado != 1){
        return -1; //No pertenece al grupo
    }

    for(i = 2; i<72;i++){
        if(leer_mensaje[i] =='\0'){ //comprueba si aún hay algo ahi
            break;
        } 
        textoGrupal[i-2] = leer_mensaje[i]; //recupera los caracteres del mensaje a enviar 
    } 

    strcat(whatsGrupal,grupoOrigen);
    strcat(whatsGrupal,remitente);
    strcat(whatsGrupal,textoGrupal);

    for(i=0;i<grupos[grupoReceptor].num_integrantes;i++){ //revisaremos los integrantes del grupo receptor
        telefonoTemporal = grupos[grupoReceptor].integrantes[i]; //guardamos el telefono de uno de los integrantes del grupo
        //printf("Repito\n");
        for(j=0;j<num_usuarios;j++){ //recorremos los usuarios
            if(telefonoTemporal == usuarios[j].telefono){ //vemos que el telefono es igual a un telefono de un usuario
                idIntegranteTemporal = usuarios[j].id; //Asi obtenemos su id
                for(k=0;k<COLA_CLIENTES;k++){ //vamos a recorrer el de sesiones
                    if(id_usuario_sesion[k]==idIntegranteTemporal){ //si lo encuentra, es que está activo y k es el num de socket
                        if(idIntegranteTemporal == indice_registro){
                            continue;
                        }else{
                            enviarMsj(k);
                        }
                    }
                }
            }
        }
    }

    return 1;

}

void enviarMsj(int indice_cliente){
    if(write(cliente_sockfd[indice_cliente],  whatsGrupal, strlen(whatsGrupal)+1) < 0){
                perror("Ocurrio un problema en el envio de un mensaje al cliente");
                exit(1);
    }
}

void salir(){
    int i;

    for(i=0;i<num_usuarios;i++){
        strcpy(usuarios[i].estado,"Desconectado");
        guardar();
    }
}