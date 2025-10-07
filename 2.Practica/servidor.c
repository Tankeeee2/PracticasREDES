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
#include <arpa/inet.h>
#include "juego.h"

/*
 * El servidor ofrece el servicio de un chat
 */

void manejador(int signum);
void salirCliente(int socket, fd_set *readfds, int *numClientes, int arrayClientes[]);

int main()
{

    /*----------------------------------------------------
        Descriptor del socket y buffer de datos
    -----------------------------------------------------*/
    int sd, new_sd;
    struct sockaddr_in sockname, from;
    char buffer[MSG_SIZE];
    socklen_t from_len;
    fd_set readfds, auxfds;
    int salida;
    Jugador arrayClientes[MAX_CLIENTS]; // array de struct Jugador donde guardaremos los clientes conectados
    int numClientes = 0;
    int userFlag = 0; // bandera para saber si el usuario ha sido validado
    // contadores
    int i, j, k;
    int recibidos;
    char identificador[MSG_SIZE];

    int on, ret;

    /* --------------------------------------------------
        Se abre el socket
    ---------------------------------------------------*/
    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd == -1)
    {
        perror("-Err. No se puede abrir el socket cliente\n");
        exit(1);
    }

    // Activaremos una propiedad del socket para permitir· que otros
    // sockets puedan reutilizar cualquier puerto al que nos enlacemos.
    // Esto permite· en protocolos como el TCP, poder ejecutar un
    // mismo programa varias veces seguidas y enlazarlo siempre al
    // mismo puerto. De lo contrario habrÌa que esperar a que el puerto
    // quedase disponible (TIME_WAIT en el caso de TCP)
    on = 1;
    ret = setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    sockname.sin_family = AF_INET;
    sockname.sin_port = htons(1865); // asignamos el puerto 1865
    sockname.sin_addr.s_addr = INADDR_ANY;

    if (bind(sd, (struct sockaddr *)&sockname, sizeof(sockname)) == -1)
    {
        perror("-Err.  Error en la operación bind");
        exit(1);
    }

    /*---------------------------------------------------------------------
        Del las peticiones que vamos a aceptar sólo necesitamos el
        tamaño de su estructura, el resto de información (familia, puerto,
        ip), nos la proporcionará el método que recibe las peticiones.
    ----------------------------------------------------------------------*/
    from_len = sizeof(from);

    if (listen(sd, 1) == -1)
    {
        perror("Error en la operación de listen");
        exit(1);
    }

    printf("El servidor está esperando conexiones...\n"); // Inicializar los conjuntos fd_set

    FD_ZERO(&readfds);
    FD_ZERO(&auxfds);
    FD_SET(sd, &readfds);
    FD_SET(0, &readfds);

    // Capturamos la señal SIGINT (Ctrl+c)
    signal(SIGINT, manejador);

    /*-----------------------------------------------------------------------
        El servidor acepta una petición
    ------------------------------------------------------------------------ */
    while (1)
    {

        // Esperamos recibir mensajes de los clientes (nuevas conexiones o mensajes de los clientes ya conectados)

        auxfds = readfds;

        salida = select(FD_SETSIZE, &auxfds, NULL, NULL, NULL);

        if (salida > 0)
        {

            for (i = 0; i < FD_SETSIZE; i++)
            {
                // Buscamos el socket por el que se ha establecido la comunicación
                if (FD_ISSET(i, &auxfds))
                {
                    if (i == sd)
                    {
                        if ((new_sd = accept(sd, (struct sockaddr *)&from, &from_len)) == -1)//comprueba si ha ocurrido un error al aceptar una nueva conexión entrante.          
                        {
                            perror("-Err. Se ha producido un error al aceptar una nueva conexión entrante");
                        }
                        else
                        {
                            /*
                                AQUÍ ENTRAMOS CUANDO UN CLIENTE SE HA CONECTADO, ES DECIR:
                                    ./cliente
                            */
                            if (numClientes < MAX_CLIENTS)// Si no se ha superado el número máximo de clientes conectados -> acepto la conexión
                            {
                                Jugador jugadorNuevo;//creamos un nuevo jugador
                                jugadorNuevo.socket = new_sd;//guardamos el socket del nuevo cliente
                                jugadorNuevo.estado = INICIO;//inicializamos el estado del jugador

                                arrayClientes[numClientes] = jugadorNuevo;//guardamos el nuevo jugador en el array de clientes
                                numClientes++;                      // Incrementamos el número de clientes conectados

                                FD_SET(new_sd, &readfds);
                                strcpy(buffer, "+Ok. Usuario conectado \n");
                                send(new_sd, buffer, sizeof(buffer), 0);
                            }
                            else // Si se ha superado el número máximo de clientes conectados -> mensaje de error y cierro el socket
                            {
                                bzero(buffer, sizeof(buffer));
                                strcpy(buffer, "-Err hay demasiados clientes conectados al servidor\n");
                                send(new_sd, buffer, sizeof(buffer), 0);
                                close(new_sd);
                            }
                        }
                    }
                    /*
                    AQUÍ ENTRO SI ESCRIBO ALGO DESDE EL PROPIO SERVIDOR, ES DECIR,
                    EN LA TERMINAL DEL SERVIDOR
                    */
                    else if (i == 0)
                    {
                        // Se ha introducido información de teclado
                        bzero(buffer, sizeof(buffer));
                        fgets(buffer, sizeof(buffer), stdin);

                        // Controlar si se ha introducido "SALIR", cerrando todos los sockets y finalmente saliendo del servidor. (implementar)
                        if (strcmp(buffer, "SALIR\n") == 0)
                        {

                            for (j = 0; j < numClientes; j++)
                            {
                                bzero(buffer, sizeof(buffer));
                                strcpy(buffer, "SALIR recibido, desconectando el servidor...\n");
                                send(arrayClientes[j].socket, buffer, sizeof(buffer), 0);
                                close(arrayClientes[j].socket);
                                FD_CLR(arrayClientes[j].socket, &readfds);
                            }
                            close(sd);
                            exit(-1);
                        }
                        // Mensajes que se quieran mandar a los clientes (implementar)


                        //Enviar al mensaje con el valor para ganar la partida a los clientes
                        int valorGanar = valorObjetivo();
                        // Enviar el valor objetivo a todos los clientes conectados
                        bzero(buffer, sizeof(buffer));
                        sprintf(buffer, "El valor objetivo para esta partida es: %d\n", valorGanar);
                        for (j = 0; j < numClientes; j++) {
                            send(arrayClientes[j].socket, buffer, sizeof(buffer), 0);
                        }
                    }
                    else
                    {
                        /*
                        AQUÍ ENTRO SI EL MENSAJE VIENE DE UN CLIENTE
                        */
                        bzero(buffer, sizeof(buffer));// Limpiamos el buffer

                        recibidos = recv(i, buffer, sizeof(buffer), 0); // Recibimos el mensaje del cliente desde el socket i 

                        if (recibidos > 0)// si hemos recibido algo
                        {
                            /*
                                LA VARIABLE i CONTIENE EL SOCKET DEL CLIENTE QUE HA ENVIADO UN MENSAJE
                            */
                            if (strcmp(buffer, "SALIR\n") == 0)
                            {
                                salirCliente(i, &readfds, &numClientes, arrayClientes);
                                // al recibir SALIR, el cliente se desconecta
                            }
                            else if (strncmp(buffer, "REGISTRO", 8) == 0)//funcionalidad de registro
                            {
                                int pos = buscarSocket(arrayClientes, numClientes, i);//Esta función busca en el array de clientes el socket que ha enviado el mensaje y devuelve su posición en el array en la variable pos
                                if (arrayClientes[pos].estado != INICIO)
                                {
                                    //si el estado del cliente no es INICIO 
                                    // No puede enviar el paquete REGISTRO
                                    bzero(buffer, sizeof(buffer));
                                    strcpy(buffer, "-Err. No se permite enviar REGISTRO en estos momentos.");
                                    send(i, buffer, sizeof(buffer), 0);
                                }
                                else
                                {
                                    //el estado del cliente es INICIO
                                    // Si puede enviar el paguete registro
                                    char usuario[MSG_SIZE];//para guardar el usuario
                                    char password[MSG_SIZE];//para guardar la contraseña
                                    if (sscanf(buffer, "REGISTRO -u %s -p %s", usuario, password) == 2)//comprobamos que el foramato del registro es correcto
                                    {
                                        /*
                                        Buscamos los usuaios insertados en el fichero  para comprobar que no hay dos usuarios con el mismo nombre
                                        --> 1 si lo hemos encontrado
                                        --> 0 si no lo hemos encontrado
                                        */
                                        int found = buscarUsuario(usuario);
                                        if (found == 1)// si hemos encontrado el usuario en el fichero, no se puede registrar
                                        {
                                            bzero(buffer, sizeof(buffer));//limpiamos el buffer
                                            strcpy(buffer, "-Err. Ya hay un usuario con el mismo nick.");//mensaje de error
                                            send(i, buffer, sizeof(buffer), 0);//enviamos el mensaje al cliente
                                        }
                                        else//si no hemos encontrado el usuario en el fichero, se puede registrar
                                        {
                                            registrarUsuario(usuario, password);//registramos el usuario
                                            bzero(buffer, sizeof(buffer));//limpiamos el buffer
                                            strcpy(buffer, "+OK. Usuario registrado.");//enviamos mensaje de exito
                                            send(i, buffer, sizeof(buffer), 0);//enviamos el mensaje al cliente
                                        }
                                    }
                                    else //En caso de que el formato del registro no sea correcto
                                    {
                                        bzero(buffer, sizeof(buffer));
                                        strcpy(buffer, "-Err. El registro tiene que tener el formato: REGISTRO -u usuario -p password.");
                                        send(i, buffer, sizeof(buffer), 0);
                                    }
                                }
                            }
                            else if (strncmp(buffer, "USUARIO", 7) == 0)
                            {
                                int pos = buscarSocket(arrayClientes, numClientes, i);//Esta función busca en el array de clientes el socket que ha enviado el mensaje y devuelve su posición en el array en la variable pos
                                if (arrayClientes[pos].estado != INICIO)
                                {
                                    //si el estado del cliente no es INICIO 
                                    // No puede enviar el paquete USUARIO
                                    bzero(buffer, sizeof(buffer));
                                    strcpy(buffer, "-Err. No se permite enviar USUARIO en estos momentos.");
                                    send(i, buffer, sizeof(buffer), 0);
                                }
                                else
                                {
                                    //el estado del cliente es INICIO
                                    // Si puede enviar el paguete USUARIO
                                    char usuario[MSG_SIZE];//para guardar el usuario
                                    if (sscanf(buffer, "USUARIO %s", usuario) == 1)//comprobamos que el foramato del registro es correcto
                                    {
                                        /*
                                        Buscamos los usuaios insertados en el fichero  para comprobar que existe ese usuario
                                        --> 1 si lo hemos encontrado
                                        --> 0 si no lo hemos encontrado
                                        */
                                        int found = buscarUsuario(usuario);//buscamos el usuario en el fichero
                                        if (found == 0)// si no hemos encontrado el usuario en el fichero, no existe ese usuario
                                        {
                                            bzero(buffer, sizeof(buffer));//limpiamos el buffer
                                            strcpy(buffer, "-Err. No existe ningún usuario con ese nick.");//mensaje de error
                                            send(i, buffer, sizeof(buffer), 0);//enviamos el mensaje al cliente
                                        }
                                        else//si hemos encontrado el usuario en el fichero, existe ese usuario
                                        {
                                            //cambiamos el estado del jugador a USUARIO_CORRECTO
                                            arrayClientes[pos].estado = USUARIO_CORRECTO;
                                            //strcpy(arrayClientes[pos].usuario, usuario);//guardamos el nombre del usuario en la estructura del jugador ¿preguntar si es necesario?
                                            bzero(buffer, sizeof(buffer));//limpiamos el buffer
                                            strcpy(buffer, "+OK. Usuario correcto.");//enviamos mensaje de exito
                                            send(i, buffer, sizeof(buffer), 0);//enviamos el mensaje al cliente
                                        }
                                    }
                                    else //En caso de que el formato del registro no sea correcto
                                    {
                                        bzero(buffer, sizeof(buffer));
                                        strcpy(buffer, "-Err. El mensaje tiene que tener el formato: USUARIO usuario.");
                                        send(i, buffer, sizeof(buffer), 0);
                                    }

                            }
                            else if (strncmp(buffer, "PASSWORD", 8) == 0)
                            {
                                int pos = buscarSocket(arrayClientes, numClientes, i);//Esta función busca en el array de clientes el socket que ha enviado el mensaje y devuelve su posición en el array en la variable pos
                                if (arrayClientes[pos].estado != USUARIO_CORRECTO;)
                                {
                                    //si el usuario no ha sido validado
                                    // No puede enviar el paquete PASSWORD
                                    bzero(buffer, sizeof(buffer));
                                    strcpy(buffer, "-Err. No se permite enviar PASSWORD en estos momentos.");
                                    send(i, buffer, sizeof(buffer), 0);
                                }
                                else
                                {
                                    //el usuario ha sido validado
                                    // Si puede enviar el paguete PASSWORD
                                    char password[MSG_SIZE];//para guardar la contraseña
                                    if (sscanf(buffer, "PASSWORD %s", password) == 1)//comprobamos que el foramato del registro es correcto
                                    {
                                        /*
                                        Buscamos los usuarios y contraseñas insertados en el fichero  para comprobar que la contraseña es correcta
                                        --> 1 si lo hemos encontrado
                                        --> 0 si no lo hemos encontrado
                                        */
                                        int found = buscarPassword(arrayClientes[pos].usuario, password);
                                        if (found == 0)// si no hemos encontrado la contraseña en el fichero, la contraseña es incorrecta
                                        {
                                            bzero(buffer, sizeof(buffer));//limpiamos el buffer
                                            strcpy(buffer, "-Err. Contraseña incorrecta.");//mensaje de error
                                            send(i, buffer, sizeof(buffer), 0);//enviamos el mensaje al cliente
                                        }
                                        else//si hemos encontrado la contraseña en el fichero, la contraseña es correcta
                                        {
                                            //cambiamos el estado del jugador a USUARIO_VALIDADO
                                            arrayClientes[pos].estado = USUARIO_VALIDADO;
                                            bzero(buffer, sizeof(buffer));//limpiamos el buffer
                                            strcpy(buffer, "+OK. Contraseña correcta. Bienvenido a la partida.");//enviamos mensaje de exito
                                            send(i, buffer, sizeof(buffer), 0);//enviamos el mensaje al cliente
                                        }
                                    }
                                }
                            }
                            else if (strncmp(buffer, "INICIAR-PARTIDA", 16) == 0)
                            {
                            }
                            else if (strncmp(buffer, "TIRAR-DADOS", 11) == 0)
                            {
                            }
                            else if (strncmp(buffer, "NO-TIRAR-DADOS", 14) == 0)
                            {
                            }
                            else if (strncmp(buffer, "PLANTARME", 9) == 0)
                            {
                            }
                            else
                            {
                                bzero(buffer, sizeof(buffer));
                                strcpy(buffer, "-Err. Mensaje no reconocido.");
                                send(i, buffer, sizeof(buffer), 0);
                            }
                        }
                        // Si el cliente introdujo ctrl+c
                        if (recibidos == 0)
                        {
                            printf("El socket %d, ha introducido ctrl+c. Servidor cerrando socket...\n", i);
                            // Eliminar ese socket
                            salirCliente(i, &readfds, &numClientes, arrayClientes);
                        }
                    }
                }
            }
        }
    }

    close(sd);
    return 0;
}
}

void salirCliente(int socket, fd_set *readfds, int *numClientes, int arrayClientes[])
{

    char buffer[250];
    int j;

    close(socket);
    FD_CLR(socket, readfds);

    // Re-estructurar el array de clientes
    for (j = 0; j < (*numClientes) - 1; j++)
        if (arrayClientes[j] == socket)
            break;
    for (; j < (*numClientes) - 1; j++)
        (arrayClientes[j] = arrayClientes[j + 1]);

    (*numClientes)--;

    bzero(buffer, sizeof(buffer));
    sprintf(buffer, "Desconexión del cliente <%d>", socket);

    for (j = 0; j < (*numClientes); j++)
        if (arrayClientes[j] != socket)
            send(arrayClientes[j], buffer, sizeof(buffer), 0);
}

void manejador(int signum)
{
    printf("\nSe ha recibido la señal sigint\n");
    signal(SIGINT, manejador);

    // Implementar lo que se desee realizar cuando ocurra la excepción de ctrl+c en el servidor
}
