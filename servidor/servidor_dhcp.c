#include "servidor_dhcp.h"

// Función para crear el socket
int crearSocket() {
    int socketFd;
    if ((socketFd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("Error al crear el socket");
        exit(1);
    }
    return socketFd;
}

// Configurar el servidor para escuchar
void configurarServidor(int socketFd, struct sockaddr_in *servidor) {
    servidor->sin_family = AF_INET;
    servidor->sin_port = htons(PUERTO_ESCUCHA); // Puerto para recibir mensajes
    servidor->sin_addr.s_addr = INADDR_ANY;
    memset(&(servidor->sin_zero), 0, 8);

    if (bind(socketFd, (struct sockaddr *)servidor, sizeof(struct sockaddr)) == -1) {
        perror("Error al asociar (bind) el socket");
        exit(1);
    }
}

// Configurar el envío en modo broadcast
void configurarBroadcast(int socketFd, struct sockaddr_in *broadcast) {
    broadcast->sin_family = AF_INET;
    broadcast->sin_port = htons(PUERTO_RESPUESTA); // Puerto para enviar respuestas
    broadcast->sin_addr.s_addr = inet_addr("255.255.255.255");
    memset(&(broadcast->sin_zero), 0, 8);

    int habilitarBroadcast = 1;
    if (setsockopt(socketFd, SOL_SOCKET, SO_BROADCAST, &habilitarBroadcast, sizeof(habilitarBroadcast)) < 0) {
        perror("Error al habilitar la opción de broadcast");
    }
}

// Recibir un mensaje DHCP Discover
int recibirDhcpDiscover(int socketFd, struct sockaddr_in *cliente, int *longitudDir, char *buffer) {
    if (recvfrom(socketFd, buffer, TAMANO_BUFFER, 0, (struct sockaddr *)cliente, (socklen_t *)longitudDir) >= 0) {
        printf("DHCP Discover recibido de: %s\n", inet_ntoa(cliente->sin_addr));
        struct mensajeDhcp *mensaje = (struct mensajeDhcp *)buffer;
        if (mensaje->op == 1 && mensaje->tipoHardware == 1 && mensaje->longitudHardware == 6 && mensaje->ipCliente == 0) {
            idTransaccionCliente = mensaje->idTransaccion;
            memcpy(direccionMacCliente, mensaje->direccionHardwareCliente, 6);
            return 1;
        }
        return 0;
    }
    perror("recvfrom");
    return 0;
}

// Asignar dirección IP al cliente
char* asignarDireccionIp() {
    static char ip[16];
    if (direccionIpDisponible < 253) {
        sprintf(ip, "192.168.1.200.%d", direccionIpDisponible);
        direccionIpDisponible++;
        return ip;  // Ejemplo de IP estática, ajustable según tu red
    }
    return NULL;
}

char*obtenerIpServidor(){
    FILE *fp;
    char buffer[16];
    char *direccionIp = malloc(16);

    fp = popen("ifconfig", "r");
    while (fgets(buffer, sizeof(buffer), fp) != NULL){
        if(strstr(buffer, "inet ") != NULL){
            sscanf(buffer, " inet %s", direccionIp);
            break;
        }
    }
    pclose(fp);

    return direccionIp;
}


// Enviar una oferta DHCP
void enviarDhcpOferta(int socketFd, struct sockaddr_in *cliente, int longitudDir) {
    struct mensajeDhcp mensajeOferta;
    memset(&mensajeOferta, 0, sizeof(mensajeOferta));
    mensajeOferta.op = 2;  // Respuesta
    mensajeOferta.tipoHardware = 1; 
    mensajeOferta.longitudHardware = 6;
    mensajeOferta.idTransaccion = idTransaccionCliente;
    mensajeOferta.flags = htons(0x8000);

    ipAsignada = asignarDireccionIp();
    mensajeOferta.yourIpAddress = htonl(inet_addr(ipAsignada));
    mensajeOferta.ipServidor = htonl(inet_addr(obtenerIpServidor()));
    
    memcpy(mensajeOferta.direccionHardwareCliente, direccionMacCliente, 16);

    unsigned char *opciones = mensajeOferta.opciones;
    int indice = 0;

    // Tipo de mensaje DHCP - DHCPOFFER
    opciones[indice++] = 53; // Tipo de mensaje (Opción)
    opciones[indice++] = 1;  // Longitud
    opciones[indice++] = 2;  // Oferta DHCP
    
    // Mascara Subnet
    opciones[indice++] = 1;
    opciones[indice++] = 4;
    uint32_t mascaraSubnet = htonl(0xFFFFFF00);
    memcpy(&opciones[indice], &mascaraSubnet, 4);
    indice += 4;

    // DNS    
    opciones[indice++] = 6;
    opciones[indice++] = 4;
    uint32_t dns = htonl(134744072);
    memcpy(&opciones[indice], &dns, 4);
    indice += 4;

    opciones[indice++] = 255;  // Fin de opciones

    if (sendto(socketFd, &mensajeOferta, sizeof(mensajeOferta), 0, (struct sockaddr *)cliente, longitudDir) == -1) {
        perror("Error al enviar la oferta DHCP");
        close(socketFd);
        exit(1);
    }
    printf("Oferta DHCP enviada a: %s\n", inet_ntoa(cliente->sin_addr));
}

// Recibir solicitud de DHCP Request
int recibirDhcpSolicitud(int socketFd, struct sockaddr_in *cliente, int *longitudDir, char *buffer) {
    if (recvfrom(socketFd, buffer, TAMANO_BUFFER, 0, (struct sockaddr *)cliente,(socklen_t *)longitudDir) >= 0){
        struct mensajeDhcp *mensaje = (struct mensajeDhcp *)buffer;
        if (mensaje->op == 1 && idTransaccionCliente == mensaje->idTransaccion && mensaje->tipoHardware == 1 && mensaje->longitudHardware == 6){
            printf("Solicitud DHCP recibida de: %s\n", inet_ntoa(cliente->sin_addr));
            return 1;
        };
        return 0;
    }
    perror("recvfrom");
    return 0;
}

// Enviar reconocimiento DHCP (DHCP ACK)
void enviarDhcpReconocimiento(int socketFd, struct sockaddr_in *cliente, int longitudDir) {
    struct mensajeDhcp mensajeAck;
    memset(&mensajeAck, 0, sizeof(mensajeAck));
    mensajeAck.op = 2;  
    mensajeAck.tipoHardware = 1;
    mensajeAck.longitudHardware = 6;
    mensajeAck.idTransaccion = idTransaccionCliente;
    mensajeAck.flags = htons(0x8000);
    mensajeAck.yourIpAddress = htonl(inet_addr(ipAsignada));
    mensajeAck.ipServidor = htonl(inet_addr(obtenerIpServidor()));
    
    memcpy(mensajeAck.direccionHardwareCliente, direccionMacCliente, 16);

    unsigned char *opciones = mensajeAck.opciones;
    int indice = 0;

    opciones[indice++] = 53;  
    opciones[indice++] = 1;  
    opciones[indice++] = 5;  // Reconocimiento (ACK)

    opciones[indice++] = 1;
    opciones[indice++] = 4;
    uint32_t mascaraSubnet = htonl(0xFFFFFF00);
    memcpy(&opciones[indice], &mascaraSubnet, 4);
    indice +=4;

    opciones[indice++] = 6;
    opciones[indice++] = 4;
    uint32_t dns = htonl(134744042);
    memcpy(&opciones[indice], &dns, 4);
    indice += 4;

    opciones[indice++] = 255;  // Fin de opciones

    if (sendto(socketFd, &mensajeAck, sizeof(mensajeAck), 0, (struct sockaddr *)cliente, longitudDir) == -1) {
        perror("Error al enviar el reconocimiento DHCP");
        close(socketFd);
        exit(1);
    }
    printf("Reconocimiento DHCP enviado a: %s\n", inet_ntoa(cliente->sin_addr));
}
