#include "servidor_dhcp.h"

uint8_t direccionMacCliente[16];  
uint32_t idTransaccionCliente;
char* ipAsignada;  
int direccionIpDisponible = 2;  // Inicialmente, tenemos una IP disponible para asignar

int main() {
    int socketFd = crearSocket();  // Descriptor del socket
    struct sockaddr_in servidor, cliente;  // Estructuras para las direcciones del servidor y cliente
    int longitudDir = sizeof(struct sockaddr);  // Tamaño de la dirección
    char buffer[TAMANO_BUFFER];  // Buffer para almacenar los mensajes recibidos


    // Configurar el servidor para escuchar en el puerto especificado
    configurarServidor(socketFd, &servidor);

    // Configurar la dirección de broadcast para enviar mensajes DHCP
    configurarBroadcast(socketFd, &cliente);

    while (1) {
        printf("Esperando un mensaje DHCP Discover...\n");
        // Esperar a recibir un mensaje DHCP Discover
        if (!recibirDhcpDiscover(socketFd, &servidor, &longitudDir, buffer)) {
            printf("Mensaje DHCP Discover recibido.\n");
            continue;
            }
            // Enviar una oferta DHCP al cliente
            enviarDhcpOferta(socketFd, &cliente, longitudDir);
            
            // Esperar a recibir un DHCP Request del cliente
            if (recibirDhcpSolicitud(socketFd, &cliente, &longitudDir, buffer)) {
                printf("Solicitud DHCP recibida.\n");
                // Enviar el reconocimiento (ACK) al cliente
                enviarDhcpReconocimiento(socketFd, &cliente, longitudDir);
            }
    }

    // Cerrar el socket (en este caso nunca se alcanzará porque el servidor está en un bucle infinito)
    close(socketFd);
    return 0;
}
