# Documentación: Cliente DHCP
## Descripción general

El cliente DHCP es responsable de solicitar una dirección IP y otros parámetros de red desde un servidor DHCP. Utiliza los mensajes DHCP Discover, DHCP Request y espera recibir DHCP Offer y DHCP Acknowledge del servidor.

# Archivos del Cliente DHCP
El cliente DHCP está compuesto por los siguientes tres archivos:

**1. cliente_dhcp.h**: Archivo de cabecera que define estructuras, constantes y prototipos de funciones.

**2. cliente_dhcp.c**: Archivo de implementación que contiene la lógica de las funciones para la comunicación DHCP.

**3. main.c**: Archivo principal que controla el flujo del cliente, donde se gestionan los intentos de solicitud DHCP.

## cliente_dhcp.h
Este archivo contiene las definiciones clave para el cliente DHCP.

### Estructura: struct mensajeDhcp
Esta estructura define un mensaje DHCP, que es utilizado para las solicitudes y respuestas entre el cliente y el servidor.

- **Campos principales**:
  - op: Tipo de mensaje (1 = Solicitud, 2 = Respuesta).
  - htype: Tipo de hardware (Ethernet).
  - hlen: Longitud de la dirección de hardware.
  - hops: Contador de saltos (normalmente 0).
  - xid: ID de transacción, utilizado para identificar las comunicaciones.

### Constantes Definidas:
 - **Puertos**:
   - PUERTO_RESPUESTA: Puerto por donde el servidor envía respuestas.
   - PUERTO_ESCUCHA: Puerto por donde el cliente escucha las respuestas del servidor.

- **Otros Parámetros**:
   - BUFFER_LEN: Tamaño máximo del buffer para los mensajes.
   - MAX_INTENTOS: Número máximo de intentos de solicitud DHCP.
   - TIMEOUT: Tiempo de espera entre intentos.

### Funciones Prototipadas:
- **int crearSocket()**: Crea un socket UDP para la comunicación.
- **void configurarDireccionServidor(struct sockaddr_in *direccionServidor, int socketFd)**: Configura la dirección del servidor DHCP.
- **void configurarDireccionCliente(struct sockaddr_in *direccionCliente, int socketFd)**: Configura la dirección del cliente para escuchar respuestas.
- **void enviarDhcpDiscover(int socketFd, struct sockaddr_in *direccionServidor, int longitudDireccion)**: Envía un mensaje DHCP Discover.
- **int recibirDhcpOffer(int socketFd, struct sockaddr_in *direccionServidor, int *longitudDireccion, char *buffer)**: Recibe la oferta DHCP del servidor.
- **void enviarDhcpRequest(int socketFd, struct sockaddr_in *direccionServidor)**: Envía una solicitud DHCP Request para la IP ofrecida.
- **int recibirDhcpAck(int socketFd, struct sockaddr_in *direccionCliente, int *longitudDireccion, char *buffer)**: Recibe el reconocimiento DHCP Acknowledge.
- **void obtenerMacCliente(uint8_t *macAddress)**: Obtiene la dirección MAC del cliente desde el sistema.
- **void imprimirDetallesDhcp(struct mensajeDhcp *mensaje)**: Imprime los detalles de la respuesta DHCP, como la IP asignada y la máscara de subred.

## cliente_dhcp.c
Este archivo implementa las funciones del cliente DHCP.

### Funciones Implementadas:
 1.**int crearSocket()**:
   - Crea un socket UDP para la comunicación entre el cliente y el servidor.
   - Si falla, imprime un error y sale del programa.

 2. **void configurarDireccionServidor(struct sockaddr_in *direccionServidor, int socketFd)**:
   - Configura los detalles de la dirección del servidor DHCP.
   - Habilita el modo de difusión (broadcast) para enviar el mensaje DHCP Discover.

 3. **void configurarDireccionCliente(struct sockaddr_in *direccionCliente, int socketFd)**:
   - Configura la dirección del cliente para recibir mensajes del servidor.
   - Enlaza el socket a una dirección local para escuchar en el puerto designado.
 
 4. **void enviarDhcpDiscover(int socketFd, struct sockaddr_in *direccionServidor, int longitudDireccion)**:
   - Crea y envía un mensaje DHCP Discover para buscar un servidor DHCP en la red.
   - El mensaje contiene la dirección MAC del cliente y otros detalles.
 
 5. **int recibirDhcpOffer(int socketFd, struct sockaddr_in *direccionServidor, int *longitudDireccion, char *buffer)**:
   - Escucha las respuestas del servidor DHCP.
   - Valida que la oferta recibida corresponde al cliente mediante la comparación del ID de transacción y la dirección MAC.

 6. **void enviarDhcpRequest(int socketFd, struct sockaddr_in *direccionServidor)**:
   - Envía un mensaje DHCP Request para solicitar formalmente la IP ofrecida por el servidor.
   - Este mensaje incluye las opciones necesarias para solicitar una dirección IP.

 7. **int recibirDhcpAck(int socketFd, struct sockaddr_in *direccionCliente, int *longitudDireccion, char *buffer)**:
   - Recibe el mensaje DHCP Acknowledge, confirmando la asignación de la dirección IP por parte del servidor.

 8.**void obtenerMacCliente(uint8_t *macAddress)**:
   - Utiliza el comando del sistema ifconfig para obtener la dirección MAC del cliente.
   - Esta MAC se usa en los mensajes DHCP Discover y Request.

 9. **void imprimirDetallesDhcp(struct mensajeDhcp *mensaje)**:
   - Imprime los detalles de la configuración IP asignada, incluyendo:
     - Dirección IP asignada.
     - Máscara de subred.
     - Servidor DNS (si está presente en las opciones).

## main.c
Este es el archivo principal que controla el flujo del cliente DHCP.

### Proceso General:

1. **Creación del socket**: Se crea un socket para la comunicación con el servidor DHCP.
   
2. **Configuración de direcciones**: Se configuran las direcciones del servidor y del cliente.

3. **Bucle de intentos**: El cliente intenta obtener una oferta DHCP hasta MAX_INTENTOS:
    - Se envía un mensaje DHCP Discover.
    - Si no se recibe una oferta, espera TIMEOUT segundos antes de reintentar.

4. **Solicitar IP**: Si se recibe una oferta, se envía una solicitud DHCP Request.

5. **Confirmación**: Se espera un mensaje DHCP Acknowledge para confirmar la asignación de la IP.

6. **Impresión de detalles**: Si se recibe el mensaje DHCP Acknowledge, se imprimen los detalles de la configuración de red obtenida.

### Flujo del programa:
- Inicializa el socket y las direcciones.
- Envía el mensaje DHCP Discover hasta obtener una oferta.
- Si se recibe una oferta, solicita formalmente la IP.
- Si la solicitud es reconocida, muestra los detalles de la asignación.
- Si no se recibe una oferta tras varios intentos, el programa termina.
