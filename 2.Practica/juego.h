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

// Defino los estados de los clientes
#define INICIO 0
#define USUARIO_VALIDO 1
#define PASSWORD_VALIDO 2
#define BUSCANDO_PARTIDA 3
#define EN_PARTIDA 4

// Defino estados para la partida
#define LIBRE 0;
#define EN_CURSO 1;
#define JUGADOR_EN_ESPERA 2;

struct Jugador
{
    int estado;             // estado del jugador
    char usuario[MSG_SIZE]; // nombre del usuario
    int socket;             // guardamos el socket del cliente
    int puntuacion;         // puntuación del jugador
    int dado1;
    int dado2;
};

struct Partida
{
    int pos1;    // si en en la partida no hay jugador, pos1 = -1
    int pos2;    // si en en la partida no hay jugador, pos2 = -1
    bool turno1; // true si es el turno del jugador 1, false si es el turno del jugador 2
    bool turno2; // true si es el turno del jugador 2, false si es el turno del jugador 1
    int valorObjetivo;
    int estado;
    // int puntuacion1;                        ¿Preguntar cual es mejor implementación?
    // int puntuacion2;
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
        printf("-Err. Error de conexión con la base de datos.");
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
        printf("-Err. Error de conexión con la base de datos.");
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
        printf("-Err. Error de conexión con la base de datos.");
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