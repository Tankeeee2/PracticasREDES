#ifndef JUEGO_H
#define JUEGO_H

#define MSG_SIZE 250
#define MAX_CLIENTS 50

// Defino los estados de los clientes
#define INICIO 0
#define USUARIO_CORRECTO 1

struct Jugador
{
    int estado;
    char usuario[MSG_SIZE];
    int socket;
};

typedef struct Jugador Jugador;

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

int buscarUsuarioFichero(char *usuario)
{
    FILE *fich = fopen("usuarios.txt", "r");
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

#endif