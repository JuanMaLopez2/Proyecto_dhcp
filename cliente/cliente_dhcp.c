#include "cliente_dhcp.h"

unsigned char macCliente[16];  
uint32_t idTransaccionCliente;  
uint32_t ipSolicitada;

// Función para crear un socket UDP
int crearSocket() {                         
    int socketFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socketFd == -1) {
        perror("Error al crear el socket");
        exit(1);
    }
    return socketFd;
}

// Configura la dirección del servidor DHCP
void configurarDireccionServidor(struct sockaddr_in *direccionServidor, int socketFd) {
    direccionServidor->sin_family = AF_INET;
    direccionServidor->sin_port = htons(PUERTO_RESPUESTA);
    direccionServidor->sin_addr.s_addr = inet_addr("255.255.255.255");
    memset(&(direccionServidor->sin_zero), 0, 8);

    int permisoBroadcast = 1;
    if (setsockopt(socketFd, SOL_SOCKET, SO_BROADCAST, &permisoBroadcast, sizeof(permisoBroadcast)) < 0) {
        perror("Error al habilitar la opción de broadcast");
        exit(1);
    }
}

// Configura la dirección del cliente para recibir respuestas
void configurarDireccionCliente(struct sockaddr_in *direccionCliente, int socketFd) {
    direccionCliente->sin_family = AF_INET;
    direccionCliente->sin_port = htons(PUERTO_ESCUCHA);
    direccionCliente->sin_addr.s_addr = INADDR_ANY;
    memset(&(direccionCliente->sin_zero), 0, 8);

    if (bind(socketFd, (struct sockaddr *)direccionCliente, sizeof(struct sockaddr_in)) == -1) {
        perror("Error al enlazar");
        close(socketFd);
        exit(1);
    }     
}

// Envía el mensaje DHCP Discover
void enviarDhcpDiscover(int socketFd, struct sockaddr_in *direccionServidor, int longitudDireccion) {
    struct mensajeDhcp mensaje;
    memset(&mensaje, 0, sizeof(mensaje));
    mensaje.op = 1; // Solicitud (cliente -> servidor)
    mensaje.htype = 1; // Tipo de hardware (Ethernet)
    mensaje.hlen = 6; // Longitud de la dirección de hardware
    mensaje.hops = 0;
    mensaje.xid = htonl(rand()); // Genera un ID de transacción aleatorio
    mensaje.flags = htons(0x8000); // Solicitar IP de difusión

    obtenerMacCliente(mensaje.macCliente); // Obtener la MAC del cliente
    idTransaccionCliente = mensaje.xid; 
    memcpy(macCliente, mensaje.macCliente, 6);

    // Configuración de las opciones DHCP
    unsigned char *opciones = mensaje.opciones;
    opciones[0] = 53; // Opción DHCP Message Type
    opciones[1] = 1;  // Longitud
    opciones[2] = 1;  // DHCP Discover
    opciones[3] = 255; // Fin de las opciones

    if (sendto(socketFd, &mensaje, sizeof(mensaje), 0, (struct sockaddr *)direccionServidor, longitudDireccion) < 0) {
        perror("Error al enviar DHCP Discover");
    } else {
        printf("DHCP Discover enviado\n");
    }
}

// Recibe una oferta DHCP del servidor
int recibirDhcpOffer(int socketFd, struct sockaddr_in *direccionServidor, int *longitudDireccion, char *buffer) {
    if ((recvfrom(socketFd, buffer, BUFFER_LEN, 0, (struct sockaddr *)direccionServidor, (socklen_t *)longitudDireccion)) >= 0) {
        printf("Oferta DHCP recibida de: %s\n", inet_ntoa(direccionServidor->sin_addr));
        struct mensajeDhcp *mensaje = (struct mensajeDhcp *)buffer;
        if (mensaje->op == 2 && mensaje->xid == idTransaccionCliente && memcmp(mensaje->macCliente, macCliente, 6) == 0) {
            ipSolicitada = ntohl(mensaje->ipAsignada); // Almacena la IP ofrecida
            return 1;
        }
    }
    perror("Error al recibir la oferta DHCP");
    return 0;  
}

// Envía una solicitud DHCP para la IP ofrecida
void enviarDhcpRequest(int socketFd, struct sockaddr_in *direccionServidor) {
    struct mensajeDhcp mensajeRequest;
    memset(&mensajeRequest, 0, sizeof(mensajeRequest));
    mensajeRequest.op = 1; // Solicitud
    mensajeRequest.htype = 1;
    mensajeRequest.hlen = 6;
    mensajeRequest.xid = idTransaccionCliente;
    mensajeRequest.flags = htons(0x8000);
    mensajeRequest.ipAsignada = htonl(ipSolicitada); // IP solicitada
    memcpy(mensajeRequest.macCliente, macCliente, 6);

    // Configuración de las opciones DHCP
    unsigned char *opciones = mensajeRequest.opciones;
    opciones[0] = 53; // DHCP Message Type
    opciones[1] = 1;  // Longitud
    opciones[2] = 3;  // DHCP Request
    opciones[3] = 50; // IP solicitada
    opciones[4] = 4;  // Longitud de la IP solicitada
    memcpy(&opciones[5], &mensajeRequest.ipAsignada, 4); // Copiar IP
    opciones[9] = 255; // Fin de opciones

    if (sendto(socketFd, &mensajeRequest, sizeof(mensajeRequest), 0, (struct sockaddr *)direccionServidor, sizeof(*direccionServidor)) < 0) {
        perror("Error al enviar DHCP Request");
    } else {
        printf("DHCP Request enviado\n");
    }
}

// Recibe el reconocimiento DHCP del servidor
int recibirDhcpAck(int socketFd, struct sockaddr_in *direccionCliente, int *longitudDireccion, char *buffer) {
    ssize_t bytesRecibidos = recvfrom(socketFd, buffer, BUFFER_LEN, 0, (struct sockaddr *)direccionCliente, (socklen_t *)longitudDireccion);
    if (bytesRecibidos >= 0) {
        printf("DHCP Acknowledge recibido de: %s\n", inet_ntoa(direccionCliente->sin_addr));
        return 1;
    } else {
        perror("Error al recibir DHCP Acknowledge");
        return 0;
    }
}

// Obtiene la dirección MAC del cliente usando el comando ifconfig
void obtenerMacCliente(uint8_t *macAddress) {
    char buffer[256];
    FILE *fp = popen("ifconfig -a | grep -i ether", "r");
    if (fp == NULL) {
        perror("Error al ejecutar popen");
        return;
    }

    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        if (sscanf(buffer, "%*s %02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx", 
                   &macAddress[0], &macAddress[1], &macAddress[2], 
                   &macAddress[3], &macAddress[4], &macAddress[5]) == 6) {
            break;
        }
    }
    fclose(fp);
}

// Imprime los detalles de la respuesta DHCP
void imprimirDetallesDhcp(struct mensajeDhcp *mensaje) {
    struct in_addr ipAddr;
    ipAddr.s_addr = ntohl(mensaje->ipAsignada); // Dirección IP asignada
    printf("Dirección IP asignada: %s\n", inet_ntoa(ipAddr));

    unsigned char *opciones = mensaje->opciones;
    int i = 0;

    // Procesa las opciones DHCP
    while (i < 312) {
        switch (opciones[i]) {
            case 1:  // Máscara de subred
                if (opciones[i + 1] == 4) {
                    struct in_addr mascaraSubred;
                    memcpy(&mascaraSubred.s_addr, &opciones[i + 2], 4);
                    printf("Máscara de subred: %s\n", inet_ntoa(mascaraSubred));
                }
                i += 6; 
                break;
            case 6:  // Servidor DNS
                if (opciones[i + 1] == 4) {
                    struct in_addr servidorDns;
                    memcpy(&servidorDns.s_addr, &opciones[i + 2], 4);
                    printf("Servidor DNS: %s\n", inet_ntoa(servidorDns));
                }
                i += 6;
                break;
            case 255:  // Fin de opciones
                return;
            default:
                i += opciones[i + 1] + 2;
                break;
        }
    }
}
