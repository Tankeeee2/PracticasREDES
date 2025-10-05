#ifndef JUEGO_H
#define JUEGO_H

#define MSG_SIZE 200
#define MAX_CLIENTS 20


// Defino los estados de los clientes
#define INICIO 0
#define USUARIO_CORRECTO 1

struct Jugador
{
    int estado; //estado del jugador
    char usuario[MSG_SIZE]; //nombre del usuario
    int socket;//guardamos el socket del cliente
};

struct Partida
{

};

typedef struct Jugador Jugador; //definimos el tipo Jugador

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
    FILE *fich = fopen("users.txt", "r");
    if (fich == NULL)
    {
        printf("-Err. Error de conexión con la base de datos.");
        return 0;
    }

    char user[MSG_SIZE];
    char pass[MSG_SIZE];
    while (fscanf(fich, "%s %s", user, pass) == 2)
    {
        if (strcmp(user, usuario) == 0)
        {
            fclose(fich);
            return 1;
        }
    }

    fclose(fich);
    return 0;
}

void registrarUsuario(char *usuario, char *password)
{
    FILE *fich = fopen("usuarios.txt", "a");
    if (fich == NULL)
    {
        printf("-Err. Error de conexión con la base de datos.");
        return;
    }

    fprintf(fich, "%s %s\n", usuario, password);

    fclose(fich);
}

int tiradaDado()
{
    // Generar un número aleatorio entre 1 y 6 para simular la tirada de un dado
    return (rand() % 6) + 1;
}

int valorObjetivo()
{
    //numero aleatorio entre 60 y 200
    //return (rand() % 141) + 60;

    //numero aleatorio entre 30 y 40 para pruebas
    return (rand() % 11) + 30;
}

#endif