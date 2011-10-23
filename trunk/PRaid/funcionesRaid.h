

#ifndef FUNCIONESRAID_H_
#define FUNCIONESRAID_H_

//TRABAJO CON DISCOS
lDiscos    	*BuscarDiscoPorId(char *id);
lDiscos 	*buscarDiscoPorDesc(int socket);
int 		 removerDiscoPorDesc(int sock);
int 		 posDisco(char *id);
int 		 removerDisco(char *id);
lDiscos    	*AgregarDisco(int desc, char* id, int idlen, char est);
int 		 hayDiscosHabilitados();
lDiscos    	*discoHabilitado();
int 		 sincronizarDisco(int sock);
int 		 listarPedidosDe(char *id);
int 		 caidaDeDisco(char *id, lPeticiones *pedidoActual);
int 		 imprimirDiscoHabilitado();

//TRABAJO CON PEDIDOS
lPeticiones *Agregar(t_list *p, int desc, uint32_t sector, char *data, int pedlen, uint8_t tPet);
void 		 distribuirPedidoLectura(int desc,pedidoStr *msg, int msglen);
void 		 distribuirPedidoEscritura(int desc,pedidoStr *ped, int pedlen);
lPeticiones *pedidoSiguiente(t_list* p);
int 		 transferirPedidos(t_list *p);

#endif /* FUNCIONESRAID_H_ */
