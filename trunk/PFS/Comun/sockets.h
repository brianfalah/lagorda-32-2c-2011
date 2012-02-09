#ifndef SOCKET_H
#define SOCKET_H


#define MAX_CONEXIONES_SHELL 1
#define MAX_CONEXIONES 10
//#define ARCH_UNIX



int inicializaSocketInet(int *descriptor, int puerto);
int conectarSocketInet(int *descriptor, char *ip, int puerto);
int inicializaSocketUnix(int *descriptor, char *archUnix);
int	conectarSocketUnix(int *descriptor, char *archUnix);


#endif
