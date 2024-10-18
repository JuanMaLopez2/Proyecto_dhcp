#include "cliente_dhcp.h"

int main() {
    int socketFd = crearSocket();
    struct sockaddr_in direccionServidor, direccionCliente;
    int longitudDireccion = sizeof(struct sockaddr);
    char buffer[BUFFER_LEN];

    configurarDireccionServidor(&direccionServidor, socketFd);
    configurarDireccionCliente(&direccionCliente, socketFd);

    int intentos = 0;
    int ofertaRecibida = 0;

    while (intentos < MAX_INTENTOS && !ofertaRecibida) {
        printf("Enviando DHCP Discover (intento %d de %d)...\n", intentos + 1, MAX_INTENTOS);
        enviarDhcpDiscover(socketFd, &direccionServidor, longitudDireccion);

        if (recibirDhcpOffer(socketFd, &direccionCliente, &longitudDireccion, buffer)) {
            ofertaRecibida = 1;
        }

        if (!ofertaRecibida) {
            printf("No se recibió oferta, esperando %d segundos...\n", TIEMPO_ESPERA);
            sleep(TIEMPO_ESPERA);
        }

        intentos++;
    }

    if (!ofertaRecibida) {
        printf("No se recibió una oferta DHCP después de %d intentos.\n", MAX_INTENTOS);
        close(socketFd);
        return 1;
    }

    enviarDhcpRequest(socketFd, &direccionServidor);

    if (recibirDhcpAck(socketFd, &direccionCliente, &longitudDireccion, buffer)) {
        struct mensajeDhcp *mensajeAck = (struct mensajeDhcp *)buffer;
        imprimirDetallesDhcp(mensajeAck);
    } else {
        printf("No se recibió un reconocimiento DHCP.\n");
    }

    close(socketFd);
    return 0;
}
