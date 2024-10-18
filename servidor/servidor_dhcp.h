#ifndef SERVIDOR_DHCP_H
#define SERVIDOR_DHCP_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

// Definiciones de constantes
#define PUERTO_ESCUCHA 1068    // Puerto donde el servidor escuchará las solicitudes
#define PUERTO_RESPUESTA 1067  // Puerto para enviar respuestas a los clientes
#define TAMANO_BUFFER 2048     // Tamaño máximo del buffer para recibir mensajes

// Estructura del mensaje DHCP
struct mensajeDhcp {
    uint8_t op;                    // Tipo de mensaje (1: solicitud, 2: respuesta)
    uint8_t tipoHardware;          // Tipo de hardware (Ethernet es 1)
    uint8_t longitudHardware;      // Longitud de la dirección de hardware (generalmente 6 para Ethernet)
    uint8_t saltos;                // Número de saltos (normalmente 0)
    uint32_t idTransaccion;        // ID de transacción
    uint16_t segundos;             // Tiempo transcurrido desde que el cliente inició el proceso
    uint16_t flags;                // Banderas (0x8000 para broadcast)
    uint32_t ipCliente;            // Dirección IP del cliente
    uint32_t yourIpAddress;        // Dirección IP asignada al cliente
    uint32_t ipServidor;           // Dirección IP del servidor
    uint32_t ipPasarela;           // Dirección IP de la pasarela (gateway)
    uint8_t direccionHardwareCliente[16];  // Dirección MAC del cliente
    char nombreServidor[64];       // Nombre del servidor DHCP
    char archivoArranque[128];     // Archivo de arranque
    uint8_t opciones[312];         // Opciones adicionales
};

// Declaraciones de variables globales
extern unsigned char direccionMacCliente[16];  
extern uint32_t idTransaccionCliente;  
extern int direccionIpDisponible;  // Contador de direcciones IP disponibles

// Funciones definidas en el servidor DHCP
int crearSocket();
void configurarServidor(int socketFd, struct sockaddr_in *servidor);
void configurarBroadcast(int socketFd, struct sockaddr_in *broadcast);
int recibirDhcpDiscover(int socketFd, struct sockaddr_in *cliente, int *longitudDir, char *buffer);
char* asignarDireccionIp();
void enviarDhcpOferta(int socketFd, struct sockaddr_in *cliente, int longitudDir);
int recibirDhcpSolicitud(int socketFd, struct sockaddr_in *cliente, int *longitudDir, char *buffer);
void enviarDhcpReconocimiento(int socketFd, struct sockaddr_in *cliente, int longitudDir);

#endif
