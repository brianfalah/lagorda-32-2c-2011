#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "sockets.h"



int inicializaSocketInet(int *socketEscucha, int puerto){
	int len;
	int yes = 1;
	struct sockaddr_in dirLocal;


	if ((*socketEscucha=socket(AF_INET, SOCK_STREAM, 0))==-1) {
		perror("no se pudo crear el socket");
		return 1;
	}

	bzero((char *) &dirLocal, sizeof(dirLocal));
	dirLocal.sin_family = AF_INET;
	dirLocal.sin_addr.s_addr = INADDR_ANY;
	dirLocal.sin_port = htons(puerto);
	len = sizeof(struct sockaddr_in);

	if (setsockopt(*socketEscucha, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		perror("fallo en setsock");
		return 1;
	}

	if (bind(*socketEscucha, (struct sockaddr_un *) &dirLocal, len)==-1) {
		perror("No pudo Bindear el socket");
		return 1;
	}

	if (listen(*socketEscucha, MAX_CONEXIONES) ==  -1) {
		perror("No pudo escuchar conexiones el Listen");
		return 1;
	}

	return 0;

}

int conectarSocketInet(int *descriptor, char *ip, int puerto) {
	struct sockaddr_in dirLocal;
	struct sockaddr_in dirCliente;
	int len;

	if ((*descriptor = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("no se pudo crear el socket");
		return 1;
	}



	bzero((char *) &dirCliente, sizeof(dirCliente));
	dirCliente.sin_family = AF_INET;
	dirCliente.sin_addr.s_addr = inet_addr(ip);
	dirCliente.sin_port = htons(puerto);

	if (connect(*descriptor, (struct sockaddr *) &dirCliente , len)==-1){
		perror("no se pudo conectar");
		return 1;
	}

	return 0;
}






int inicializaSocketUnix(int *socketListen, char *archUnix) {
	int len;
	int yes = 1;
	struct sockaddr_un dirLocal;
	unlink(archUnix);

	if ((*socketListen=socket(AF_UNIX, SOCK_STREAM, 0))==-1) {
		perror("no se pudo crear el socket");
		return 1;
	}

	bzero((char *) &dirLocal, sizeof(dirLocal));
	dirLocal.sun_family = AF_UNIX;
	strcpy(dirLocal.sun_path, archUnix);
	len = strlen(dirLocal.sun_path) + sizeof(dirLocal.sun_family);

	if (setsockopt(*socketListen, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		perror("fallo en setsock");
		return 1;
	}

	if (bind(*socketListen, (struct sockaddr_un *) &dirLocal, len)==-1) {
		perror("No pudo Bindear el socket");
		return 1;
	}

	if (listen(*socketListen, MAX_CONEXIONES_SHELL) ==  -1) {
		perror("No pudo escuchar conexiones el Listen");
		return 1;
	}

	return 0;

}




int	conectarSocketUnix(int *socketConnect, char *archUnix){
	int len;
	struct sockaddr_un dirLocal;


	if ((*socketConnect = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		perror("No se puedo crear el socket.\n");
		return 1;
	}

	bzero((char *) &dirLocal, sizeof(dirLocal));
	dirLocal.sun_family = AF_UNIX;
	strcpy(dirLocal.sun_path, archUnix);
	len = strlen(dirLocal.sun_path) + sizeof(dirLocal.sun_family);

	if (connect(*socketConnect, (struct sockaddr *)&dirLocal, len) == -1){
		perror("No se pudo conectar");
		return 1;
	}


	return 0;

}

int aceptarUnix(int sock){

	struct sockaddr_un	cliente;
	int addrlen = sizeof(struct sockaddr_un);
	int sockAccept = 0;

	bzero((char *) &cliente, sizeof(struct sockaddr_un));
	sockAccept = accept(sock, (struct sockaddr *)&cliente, &addrlen);
	if (sockAccept == -1) {
		perror("accept");
		exit(EXIT_FAILURE);
	}

	return sockAccept;
}

int aceptarInet(int sock){

	struct sockaddr_in	cliente;
	int addrlen = sizeof(struct sockaddr_in);
	int sockAccept = 0;

	bzero((char *) &cliente, sizeof(cliente));
	sockAccept = accept(sock, (struct sockaddr *)&cliente, &addrlen);
	if (sockAccept == -1) {
		perror("accept");
		exit(EXIT_FAILURE);
	}

	return sockAccept;
}

