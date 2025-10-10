#ifndef JUEGO_H
#define JUEGO_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <stdbool.h>

#define MSG_SIZE 200
#define MAX_CLIENTS 20
#define MAX_PARTIDAS 10

// ESTADOS JUGADOR
#define INICIO 0
#define USUARIO_VALIDO 1
#define PASSWORD_VALIDO 2
#define BUSCANDO_PARTIDA 3
#define EN_PARTIDA 4


// ESTADOS PARTIDA
//  Nadie en partida
#define VACIA 0
//  Partida con 2 jugadores juegan o van a jugar
#define EN_CURSO 1
//  Partida con un jugador esperando
#define JUGADOR_EN_ESPERA 2


// ESTADOS TURNO
//  Turno Jugador 1
#define UNO 1
//  Turno Jugador 2
#define DOS 2

struct Jugador
{
    //  INICIO 0, USUARIO_VALIDO 1, PASSWORD_VALIDO 2, BUSCANDO_PARTIDA 3, EN_PARTIDA 4
    int estado;             
    //  Nombre del usuario
    char usuario[MSG_SIZE]; 
    //  Socket del cliente
    int socket;
    //  Dado 1
    int dado1;
    //  Dado 2
    int dado2;
};

struct Partida
{
    //  pos1=-1 si no hay jugador 1, si no pos1=pos
    int pos1;    
    //  pos2=-1 si no hay jugador 2, si no pos2=pos
    int pos2;   
    //  UNO si es el turno del jugador 1, DOS si es el turno del jugador 2 
    int turno;
    //  Valor al que los jugadores deben llegar con su puntuación
    int valorObjetivo;
    //  VACIA 0, EN_CURSO 1, JUGADOR_EN_ESPERA 2
    int estado;
    //  Puntuacion jugador 1
    int puntuacion1;
    //  Puntuacion jugador 2
    int puntuacion2;
};

typedef struct Jugador Jugador; // definimos el tipo Jugador
typedef struct Partida Partida; // definimos el tipo Partida

// FUNCIONES

int buscarSocket(Jugador *arrayClientes, int numClientes, int socket)
{
    for (size_t i = 0; i < numClientes; i++)
    {
        if (arrayClientes[i].socket == socket)
        {
            return i;
        }
    }
    return -1;
}

int buscarUsuario(char *usuario)
{
    // Busca en el fichero users.txt si el usuario ya existe
    FILE *fich = fopen("users.txt", "r");
    if (fich == NULL)
    {
        // error al abir el fichero
        printf("-Err. Error de conexión con la base de datos. \n");
        return 0;
    }

    char user[MSG_SIZE];
    char pass[MSG_SIZE];
    while (fscanf(fich, "%s %s", user, pass) == 2) // MIentras pueda leer dos cadenas (usuario y contraseña) vamos recorriendo el fichero
    {
        if (strcmp(user, usuario) == 0) // si el usuario del fichero es igual al usuario que queremos registrar
        {
            fclose(fich); // cerramos el fichero
            return 1;     // devolvemos 1 si hemos encontrado el usuario (no se puede registrar)
        }
    }

    fclose(fich); // cerramos el fichero
    return 0;     // devolvemos 0 si no hemos encontrado el usuario (se puede registrar)
}

int buscarPassword(char *usuario, char *password)
{
    // Busca en el fichero users.txt si el usuario ya existe
    FILE *fich = fopen("users.txt", "r");
    if (fich == NULL)
    {
        // error al abir el fichero
        printf("-Err. Error de conexión con la base de datos. \n");
        return 0;
    }

    char user[MSG_SIZE];
    char pass[MSG_SIZE];
    while (fscanf(fich, "%s %s", user, pass) == 2) // Mientras pueda leer dos cadenas (usuario y contraseña) vamos recorriendo el fichero
    {
        if (strcmp(user, usuario) == 0 && strcmp(pass, password) == 0) // Si usuario Y  contraseña coinciden
        {
            fclose(fich); // cerramos el fichero
            return 1;     // devolvemos 1 si hemos encontrado el usuario (no se puede registrar)
        }
    }

    fclose(fich); // cerramos el fichero
    return 0;     // devolvemos 0 si no hemos encontrado el usuario (se puede registrar)
}

void registrarUsuario(char *usuario, char *password) // almacena el usuario y la contraseña en el fichero users.txt
{
    FILE *fich = fopen("users.txt", "a");
    if (fich == NULL)
    {
        // error al abir el fichero
        printf("-Err. Error de conexión con la base de datos. \n");
        return;
    }

    fprintf(fich, "%s %s\n", usuario, password); // escribimos el usuario y la contraseña en el fichero
    // de la forma: USUARIO CONTRASEÑA separados por un espacio y cada usuario en una línea diferente

    fclose(fich);
}

int tiradaDado()
{
    // Generar un número aleatorio entre 1 y 6 para simular la tirada de un dado
    return (rand() % 6) + 1;
}

int valorObjetivo()
{
    // numero aleatorio entre 60 y 200
    // return (rand() % 141) + 60;

    // numero aleatorio entre 30 y 40 para pruebas
    return (rand() % 11) + 30;
}

#endif