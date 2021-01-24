#include<stdio.h>
#include<sys/select.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<string.h>
#include<unistd.h>
#include<stdlib.h>
#include <arpa/inet.h>

#define maximo(a,b) ((a) > (b) ? (a) : (b))
#define PUERTO 6020
#define TAM_BUFFER 100
#define DIR_IP "127.0.0.1"

void main(int argc, char **argv){
    int tamano_direccion, sockfd, id_cliente;
    struct sockaddr_in direccion_servidor;
    char mensaje[TAM_BUFFER];
    fd_set conjunto_leer;
    int maximo_fd, n;

    memset (&direccion_servidor, 0, sizeof (direccion_servidor));
    direccion_servidor.sin_family = AF_INET;
    direccion_servidor.sin_port = htons(PUERTO);
    if ( inet_pton(AF_INET, DIR_IP, &direccion_servidor.sin_addr) <= 0){
        perror("Ocurrio un error al momento de asignar la direccion IP");
        exit(-1);
    }

    FD_ZERO(&conjunto_leer);
    printf("Creando Socket ....\n");
    if ((sockfd = socket (AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Ocurrió un problema en la creación del socket");
        exit(-1);
    }

    printf ("Estableciendo conexión ...\n");
    if ( connect(sockfd, (struct sockaddr *)&direccion_servidor,sizeof(direccion_servidor) ) < 0) {
        perror ("Ocurrió un problema al establecer la conexión");
        exit(-1);
    }
    printf ("Conectandose al servidor ...\n");
    printf("----Lista de comandos generales----\n");
    printf("Registrar Usuario: 1TeléfonoNombre\n");
    printf("Iniciar Sesión: 2Teléfono\n");
    printf("-----Comandos después de sesión-----\n");
    printf("Agregar Contacto: 3Contacto\n");
    printf("Ver Contactos: 4\n");
    printf("Enviar Mensaje: 5ContactoMensaje\n");
    printf("Crear Grupo: 6Nombre\n");
    printf("Añadir Usuario a Grupo: 71Grupo(ID)Teléfono\n");
    printf("Revisar Grupos en los que estoy: 8\n");
    printf("Enviar mensaje a un grupo: 9Grupo(ID)Mensaje\n\n\n");
    while (1){

        FD_SET(fileno(stdin), &conjunto_leer);
        FD_SET(sockfd, &conjunto_leer);
        maximo_fd = maximo(fileno(stdin), sockfd) + 1;

        if ( select (maximo_fd, &conjunto_leer, NULL, NULL, NULL) < 0){
            perror ("Ocurrió un problema dentro de la función select");
            close (sockfd);
        }

        if ( FD_ISSET(sockfd, &conjunto_leer)){
            if (read (sockfd, &mensaje, TAM_BUFFER) < 0){
                perror ("Ocurrió algun problema al recibir datos del cliente");
                exit(-1);
            }
            printf ("===> %s\n", mensaje);
        }

        if ( FD_ISSET(fileno(stdin), &conjunto_leer)){
            if (fgets (mensaje, TAM_BUFFER, stdin) < 0){
                perror ("Ocurrió algun problema al recibir datos del cliente");
                exit(-1);
            }
            if ( write (sockfd, mensaje, strlen(mensaje) + 1) < 0){
                perror("Ocurrió un problema en el envió de un mensaje al cliente");
                exit(-1);
            }
        }
    }
    printf ("El servidor envió el siguiente mensaje: \n%s\n", mensaje);
    close(sockfd);
}
