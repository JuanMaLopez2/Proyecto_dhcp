#ifndef CLIENTE_DHCP_H
#define CLIENTE_DHCP_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <time.h>

#define PUERTO_RESPUESTA 1067   
#define PUERTO_ESCUCHA  1068
#define BUFFER_LEN 2048
#define MAX_INTENTOS 5
#define TIEMPO_ESPERA 2

struct mensajeDhcp {
    uint8_t op;
    uint8_t htype;
    uint8_t hlen;
    uint8_t hops;
    uint32_t xid;
    uint16_t secs;
    uint16_t flags;
    uint32_t ipCliente;
    uint32_t ipAsignada;
    uint32_t ipServidor;
    uint32_t ipGateway;
    uint8_t macCliente[16];
    char nombreServidor[64];
    char nombreArchivoArranque[128];
    uint8_t opciones[312];
};

int crearSocket();
void configurarDireccionServidor(struct sockaddr_in *direccionServidor, int socketFd);
void configurarDireccionCliente(struct sockaddr_in *direccionCliente, int socketFd);
void enviarDhcpDiscover(int socketFd, struct sockaddr_in *direccionServidor, int longitudDireccion);
int recibirDhcpOffer(int socketFd, struct sockaddr_in *direccionCliente, int *longitudDireccion, char *buffer);
void enviarDhcpRequest(int socketFd, struct sockaddr_in *direccionServidor);
int recibirDhcpAck(int socketFd, struct sockaddr_in *direccionCliente, int *longitudDireccion, char *buffer);
void obtenerMacCliente(uint8_t *macAddress);
void imprimirDetallesDhcp(struct mensajeDhcp *mensaje);

extern unsigned char macCliente[16];  
extern uint32_t idTransaccionCliente;  
extern uint32_t ipSolicitada;

#endif